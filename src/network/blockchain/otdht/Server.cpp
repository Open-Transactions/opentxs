// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/blockchain/otdht/Server.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <compare>
#include <iterator>
#include <span>
#include <stdexcept>
#include <utility>

#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/database/Sync.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/params/ChainData.hpp"
#include "internal/network/blockchain/Types.hpp"
#include "internal/network/otdht/Factory.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/alloc/MonotonicSync.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/cfilter/GCS.hpp"
#include "opentxs/blockchain/cfilter/Header.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Header.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/otdht/Base.hpp"
#include "opentxs/network/otdht/Block.hpp"
#include "opentxs/network/otdht/Data.hpp"
#include "opentxs/network/otdht/Request.hpp"
#include "opentxs/network/otdht/State.hpp"
#include "opentxs/network/otdht/Types.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WorkType.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/ScopeGuard.hpp"
#include "util/Work.hpp"

namespace opentxs::network::blockchain::otdht
{
Server::Shared::Shared(
    const opentxs::blockchain::node::Manager& node,
    allocator_type alloc) noexcept
    : db_(node.Internal().DB())
    , sync_tip_(db_.SyncTip())
    , queue_(alloc)
    , caught_up_(false)
{
}

auto Server::Shared::Best() const noexcept
    -> const opentxs::blockchain::block::Position&
{
    if (queue_.empty()) {

        return sync_tip_;
    } else {

        return queue_.back();
    }
}
}  // namespace opentxs::network::blockchain::otdht

namespace opentxs::network::blockchain::otdht
{
Server::Server(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const opentxs::blockchain::node::Manager> node,
    network::zeromq::BatchID batchID,
    allocator_type alloc) noexcept
    : Actor(api, node, batchID, alloc)
    , checkpoint_(opentxs::blockchain::params::get(chain_).CheckpointPosition())
    , shared_(node_, alloc)
    , counter_()
    , running_(counter_.Allocate(1))
{
}

auto Server::background(
    boost::shared_ptr<Server> me,
    std::shared_ptr<const ScopeGuard> post) noexcept -> void
{
    OT_ASSERT(me);
    OT_ASSERT(post);

    // WARNING this function must not be be called from a zmq thread
    auto alloc = alloc::MonotonicSync{me->get_allocator().resource()};
    auto handle = me->shared_.lock();
    auto& shared = *handle;
    me->fill_queue(shared);
    me->drain_queue(shared, std::addressof(alloc));
    me->check_caught_up(shared);

    if (false == shared.queue_.empty()) {
        me->pipeline_.Push(MakeWork(Work::statemachine));
    }
}

auto Server::check_caught_up(Shared& shared) noexcept -> void
{
    if (shared.caught_up_) { return; }

    if (false == shared.queue_.empty()) { return; }

    if (shared.sync_tip_ < checkpoint_) { return; }

    shared.caught_up_ = (shared.sync_tip_ == node_.HeaderOracle().BestChain());
}

auto Server::do_work() noexcept -> bool
{
    if (false == running_.is_limited()) {
        auto me = boost::shared_from(this);
        auto post = std::make_shared<ScopeGuard>(
            [me] { ++me->running_; }, [me] { --me->running_; });
        RunJob([me, post] { background(me, post); });
    }

    return false;
}

auto Server::drain_queue(Shared& shared, allocator_type monotonic) noexcept
    -> void
{
    if (shared.queue_.empty()) { return; }

    const auto alloc = get_allocator();
    auto data = network::otdht::SyncData{alloc};
    auto push = Vector<Message>{alloc};
    const auto items = std::min(shared.queue_.size(), queue_limit_);
    data.reserve(items);
    data.clear();
    push.reserve(items);
    push.clear();
    auto tip = opentxs::blockchain::block::Position{};

    try {
        auto count = 0_uz;
        const auto& hOracle = node_.HeaderOracle();
        const auto& fOracle = node_.FilterOracle();
        log_(OT_PRETTY_CLASS())(name_)(": processing ")(items)(" blocks")
            .Flush();

        while (count < items) {
            OT_ASSERT(false == shared.queue_.empty());

            const auto& position = shared.queue_.front();
            const auto header = hOracle.LoadHeader(position.hash_);

            if (false == header.IsValid()) {
                throw std::runtime_error(
                    CString{"failed to load block header ", alloc}
                        .append(position.print(alloc))
                        .c_str());
            }

            const auto previousCfheader =
                fOracle.LoadFilterHeader(filter_type_, header.ParentHash());

            if (previousCfheader.empty()) {
                throw std::runtime_error(
                    CString{"failed to previous cfheader for block ", alloc}
                        .append(position.print(alloc))
                        .c_str());
            }

            const auto cfilter = fOracle.LoadFilter(
                filter_type_, position.hash_, {alloc, monotonic});

            if (false == cfilter.IsValid()) {
                throw std::runtime_error(
                    CString{"failed to load cfilter for block ", alloc}
                        .append(position.print(alloc))
                        .c_str());
            }

            const auto headerBytes = header.asBitcoin().Encode();
            const auto filterBytes = [&] {
                auto out = Space{};
                cfilter.Compressed(writer(out));

                return out;
            }();
            const auto& newData = data.emplace_back(
                chain_,
                position.height_,
                filter_type_,
                cfilter.ElementCount(),
                headerBytes.Bytes(),
                reader(filterBytes));
            ++count;
            tip = position;

            if (shared.caught_up_) {
                push.emplace_back([&] {
                    const auto msg = factory::BlockchainSyncData(
                        WorkType::P2PBlockchainNewBlock,
                        {chain_, tip},
                        [&] {
                            auto out = network::otdht::SyncData{alloc};
                            out.emplace_back(newData);

                            return out;
                        }(),
                        previousCfheader.Bytes());
                    auto out = Message{};
                    out.StartBody();
                    const auto serialized = msg.Serialize(out);

                    OT_ASSERT(serialized);

                    return out;
                }());
            }

            shared.queue_.pop_front();
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(name_)(": ")(e.what()).Flush();
        shared.queue_.clear();
    }

    if (data.empty()) {
        log_(OT_PRETTY_CLASS())(name_)("no data to process").Flush();

        return;
    }

    const auto stored = shared.db_.StoreSync(tip, data);

    if (false == stored) {
        LogAbort()(OT_PRETTY_CLASS())(name_)(": database error").Abort();
    }

    update_tip(shared, false, tip);

    for (auto& msg : push) { send_to_listeners(std::move(msg)); }
}

auto Server::fill_queue(Shared& shared) noexcept -> void
{
    const auto& log = log_;
    const auto& current = shared.Best();
    const auto oracle = oracle_position();

    if (0 == oracle.height_) { return; }

    log(OT_PRETTY_CLASS())(name_)(": current position: ")(current).Flush();
    log(OT_PRETTY_CLASS())(name_)(":  oracle position: ")(oracle).Flush();
    auto blocks = node_.HeaderOracle().Ancestors(current, oracle, 0_uz);

    OT_ASSERT(false == blocks.empty());

    log(OT_PRETTY_CLASS())(name_)(": loaded ")(blocks.size())(
        " blocks hashes from oracle")
        .Flush();
    log(OT_PRETTY_CLASS())(name_)(": newest common parent is ")(blocks.front())
        .Flush();
    const auto& best = blocks.front();

    if (best.height_ <= shared.sync_tip_.height_) {
        update_tip(shared, true, best);
    }

    if (1_uz < blocks.size()) {
        log(OT_PRETTY_CLASS())(name_)(": first unqueued block is ")(
            blocks.at(1_uz))
            .Flush();
        log(OT_PRETTY_CLASS())(name_)(":  last unqueued block is ")(
            blocks.back())
            .Flush();
    }

    while ((!shared.queue_.empty()) &&
           (shared.queue_.back().height_ > best.height_)) {
        log(OT_PRETTY_CLASS())(name_)(": removing orphaned block")(
            shared.queue_.back())
            .Flush();
        shared.queue_.pop_back();
    }

    if (false == shared.queue_.empty()) {
        const auto& last = shared.queue_.back();

        OT_ASSERT(last == best);
    }

    if (1_uz < blocks.size()) {
        const auto first = std::next(blocks.begin());
        log(OT_PRETTY_CLASS())(name_)(": adding ")(blocks.size() - 1_uz)(
            " blocks to queue from ") (*first)(" to ")(blocks.back())
            .Flush();
        std::move(
            std::next(blocks.begin()),
            blocks.end(),
            std::back_inserter(shared.queue_));
    } else {
        log(OT_PRETTY_CLASS())(name_)(": no blocks to add to queue").Flush();
    }
}

auto Server::local_position() const noexcept
    -> opentxs::blockchain::block::Position
{
    return shared_.lock_shared()->sync_tip_;
}

auto Server::process_checksum_failure(Message&& msg) noexcept -> void
{
    try {
        const auto body = msg.Payload();

        if (3_uz >= body.size()) {
            throw std::runtime_error{"invalid message"};
        }

        if (body[1].as<decltype(chain_)>() != chain_) { return; }

        using Height = opentxs::blockchain::block::Height;
        const auto height = body[2].as<Height>();
        // TODO const auto version = body[3].as<VersionNumber>();
        auto handle = shared_.lock();
        auto& shared = *handle;
        const auto local = shared.sync_tip_.height_;
        const auto target = std::max<Height>(0, std::min(height, local - 100));
        LogConsole()("Resetting ")(print(chain_))(" sync database to height ")(
            target)(" due to checksum failure");
        update_tip(
            shared, true, {target, node_.HeaderOracle().BestHash(target)});
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(name_)(": ")(e.what()).Flush();
    }
}

auto Server::process_report(Message&& msg) noexcept -> void { report(); }

auto Server::process_sync_request(Message&& msg) noexcept -> void
{
    const auto pBase = api_.Factory().BlockchainSyncMessage(msg);

    OT_ASSERT(pBase);

    const auto& base = *pBase;
    const auto& request = base.asRequest();
    const auto handle = shared_.lock_shared();
    const auto& data = *handle;

    try {
        const auto& incoming = [&]() -> const network::otdht::State& {
            for (const auto& state : request.State()) {
                if (state.Chain() == chain_) { return state; }
            }

            throw std::runtime_error{"No matching chains"};
        }();
        const auto& position = incoming.Position();
        const auto& header = node_.HeaderOracle();
        auto positions = header.Ancestors(position, data.sync_tip_);

        OT_ASSERT(false == positions.empty());

        const auto& parent = positions.front();
        auto& best = positions.back();
        const auto target = [&] {
            static constexpr auto lookback =
                opentxs::blockchain::block::Height{144};

            if ((0 == parent.height_) && (lookback < position.height_)) {

                return std::min(position.height_ - lookback, best.height_);
            } else {

                return parent.height_;
            }
        }();

        const auto needSync = position != best;
        auto reply = factory::BlockchainSyncData(
            WorkType::P2PBlockchainSyncReply,
            network::otdht::State{chain_, std::move(best)},
            {},
            {});

        if (needSync) { data.db_.LoadSync(target, reply); }

        to_dht().SendDeferred(
            [&] {
                auto out = zeromq::reply_to_message(std::move(msg));
                reply.Serialize(out);

                return out;
            }(),
            __FILE__,
            __LINE__);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(name_)(": ")(e.what()).Flush();
    }
}

auto Server::report() noexcept -> void
{
    report(shared_.lock_shared()->sync_tip_);
}

auto Server::report(const opentxs::blockchain::block::Position& tip) noexcept
    -> void
{
    to_api().SendDeferred(
        [&] {
            auto out = MakeWork(WorkType::BlockchainSyncServerProgress);
            out.AddFrame(chain_);
            out.AddFrame(tip.height_);
            out.AddFrame(tip.hash_);

            return out;
        }(),
        __FILE__,
        __LINE__);
}

auto Server::reset_to_genesis() noexcept -> void
{
    auto handle = shared_.lock();
    auto& shared = *handle;
    update_tip(shared, true, node_.HeaderOracle().GetPosition(0));
}

auto Server::update_tip(
    Shared& shared,
    bool db,
    opentxs::blockchain::block::Position tip) noexcept -> void
{
    const auto& log = log_;

    if (db) {
        auto saved = shared.db_.SetSyncTip(tip);

        OT_ASSERT(saved);
    }

    shared.sync_tip_ = std::move(tip);
    log(OT_PRETTY_CLASS())(name_)(": sync data updated to ")(shared.sync_tip_)
        .Flush();
    report(shared.sync_tip_);
}

Server::~Server() = default;
}  // namespace opentxs::network::blockchain::otdht
