// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "blockchain/node/blockoracle/Actor.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/shared_ptr.hpp>
#include <algorithm>
#include <chrono>
#include <cstring>
#include <exception>
#include <iterator>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>
#include <variant>

#include "blockchain/node/blockoracle/Shared.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/blockchain/node/Config.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/message/Message.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/util/BlockchainProfile.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/ScopeGuard.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::internal
{
BlockOracle::Actor::Actor(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const node::Manager> node,
    boost::shared_ptr<Shared> shared,
    network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : BlockOracleActor(
          *api,
          shared->log_,
          CString{shared->name_, alloc},
          0ms,
          std::move(batch),
          alloc,
          [&] {
              using enum network::zeromq::socket::Direction;
              auto sub = network::zeromq::EndpointArgs{alloc};
              sub.emplace_back(api->Endpoints().Shutdown(), Connect);
              sub.emplace_back(api->Endpoints().BlockchainReorg(), Connect);
              sub.emplace_back(
                  api->Endpoints().Internal().BlockchainReportStatus(),
                  Connect);
              sub.emplace_back(
                  node->Internal().Endpoints().shutdown_publish_, Connect);

              return sub;
          }(),
          [&] {
              using enum network::zeromq::socket::Direction;
              auto pull = network::zeromq::EndpointArgs{alloc};
              pull.emplace_back(
                  node->Internal().Endpoints().block_oracle_pull_, Bind);

              return pull;
          }(),
          {},
          [&] {
              using enum network::zeromq::socket::Direction;
              using enum network::zeromq::socket::Type;
              auto extra = Vector<network::zeromq::SocketData>{alloc};
              extra.emplace_back(Router, [&] {
                  auto out = Vector<network::zeromq::EndpointArg>{alloc};
                  out.emplace_back(
                      node->Internal().Endpoints().block_oracle_router_, Bind);

                  return out;
              }());  // NOTE router_
              extra.emplace_back(Publish, [&] {
                  auto out = Vector<network::zeromq::EndpointArg>{alloc};
                  out.emplace_back(
                      node->Internal().Endpoints().block_tip_publish_, Bind);

                  return out;
              }());  // NOTE tip_updated_
              extra.emplace_back(Push, [&] {
                  auto out = Vector<network::zeromq::EndpointArg>{alloc};
                  out.emplace_back(
                      api->Endpoints().Internal().BlockchainMessageRouter(),
                      Connect);

                  return out;
              }());  // NOTE to_blockchain_api_

              return extra;
          }())
    , api_p_(std::move(api))
    , node_p_(std::move(node))
    , shared_p_(std::move(shared))
    , api_(*api_p_)
    , node_(*node_p_)
    , shared_(*shared_p_)
    , router_(pipeline_.Internal().ExtraSocket(0))
    , tip_updated_(pipeline_.Internal().ExtraSocket(1))
    , to_blockchain_api_(pipeline_.Internal().ExtraSocket(2))
    , chain_(node_.Internal().Chain())
    , download_blocks_(
          BlockchainProfile::server == node_.Internal().GetConfig().profile_)
    , requests_(alloc)
    , tip_()
    , cache_(alloc)
    , downloading_(alloc)
    , ready_(alloc)
{
}

auto BlockOracle::Actor::adjust_tip(const block::Position& tip) noexcept -> void
{
    const auto& log = log_;

    if (false == downloading_.empty()) {
        const auto best = downloading_.crbegin();

        if (best->second != tip) {
            downloading_.erase(
                downloading_.lower_bound(tip.height_), downloading_.end());
            ready_.erase(ready_.lower_bound(tip.height_), ready_.end());
        }
    }

    if (tip.height_ == tip_.height_) {
        if (tip.hash_ == tip_.hash_) {
            log(OT_PRETTY_CLASS())(name_)(": ancestor block ")(
                tip)(" matches existing tip ")(tip_)
                .Flush();
        } else {
            LogAbort()(OT_PRETTY_CLASS())(name_)(": ancestor block ")(
                tip)(" does not match existing tip ")(tip_)
                .Abort();
        }

    } else if (tip.height_ < tip_.height_) {
        set_tip(tip);
    }
}

auto BlockOracle::Actor::block_is_ready(const block::Hash& id) noexcept -> void
{
    const auto& log = log_;

    if (auto i = cache_.find(id); cache_.end() != i) {
        const auto& height = i->second;

        if (block_is_ready(id, height)) { cache_.erase(i); }
    } else {
        log(OT_PRETTY_CLASS())(name_)(": block ")
            .asHex(id)(" is not in cache")
            .Flush();
    }
}

auto BlockOracle::Actor::block_is_ready(
    const block::Hash& id,
    block::Height height) noexcept -> bool
{
    const auto& log = log_;

    if (auto i = downloading_.find(height); downloading_.end() != i) {
        const auto& position = i->second;

        if (position.hash_ == id) {
            log(OT_PRETTY_CLASS())(name_)(": block ")
                .asHex(id)(" at height ")(height)(" downloaded")
                .Flush();
            ready_.insert(downloading_.extract(i));

            return true;
        } else {
            log(OT_PRETTY_CLASS())(name_)(": block ")
                .asHex(id)(" at height ")(
                    height)(" does not match expected hash for this height (")
                .asHex(position.hash_)(")")
                .Flush();

            return false;
        }
    } else {
        log(OT_PRETTY_CLASS())(name_)(": failed to locate block ")
            .asHex(id)(" in download queue")
            .Flush();

        return false;
    }
}

auto BlockOracle::Actor::broadcast_tip() noexcept -> void
{
    tip_updated_.SendDeferred(
        [this] {
            auto msg = MakeWork(OT_ZMQ_NEW_FULL_BLOCK_SIGNAL);
            msg.AddFrame(tip_.height_);
            msg.AddFrame(tip_.hash_);

            return msg;
        }(),
        __FILE__,
        __LINE__);
    to_blockchain_api_.SendDeferred(
        [this] {
            auto msg = MakeWork(WorkType::BlockchainBlockOracleProgress);
            msg.AddFrame(chain_);
            msg.AddFrame(tip_.height_);
            msg.AddFrame(tip_.hash_);

            return msg;
        }(),
        __FILE__,
        __LINE__);
}

auto BlockOracle::Actor::do_shutdown() noexcept -> void
{
    shared_p_.reset();
    node_p_.reset();
    api_p_.reset();
}

auto BlockOracle::Actor::do_startup(allocator_type monotonic) noexcept -> bool
{
    if ((api_.Internal().ShuttingDown()) || (node_.Internal().ShuttingDown())) {

        return true;
    }

    if (download_blocks_) {
        tip_ = shared_.GetTip(monotonic);
        broadcast_tip();
        do_work(monotonic);
    }

    return false;
}

auto BlockOracle::Actor::get_sender(const Message& msg) noexcept -> ByteArray
{
    return msg.Header_at(0).Bytes();
}

auto BlockOracle::Actor::Init(boost::shared_ptr<Actor> me) noexcept -> void
{
    signal_startup(me);
}

auto BlockOracle::Actor::next_position() const noexcept
    -> const block::Position&
{
    if (downloading_.empty()) {

        return tip_;
    } else {

        return downloading_.crbegin()->second;
    }
}

auto BlockOracle::Actor::notify_requestors(
    const block::Hash& id,
    const blockoracle::BlockLocation& block) noexcept -> void
{
    using blockoracle::CachedBlock;
    using blockoracle::MissingBlock;
    using blockoracle::PersistentBlock;

    struct Visitor {
        const block::Hash& id_;
        Actor& this_;

        auto operator()(const MissingBlock&) noexcept -> void {}
        auto operator()(const PersistentBlock& bytes) noexcept -> void
        {
            this_.notify_requestors(
                id_,
                reinterpret_cast<std::uintptr_t>(bytes.data()),
                bytes.size());
        }
        auto operator()(const CachedBlock& block) noexcept -> void
        {
            this_.notify_requestors(id_, block->Bytes());
        }
    };

    std::visit(Visitor{id, *this}, block);
}

auto BlockOracle::Actor::notify_requestors(
    const block::Hash& hash,
    const ReadView bytes) noexcept -> void
{
    if (auto i = requests_.find(hash); requests_.end() != i) {
        auto post = ScopeGuard{[&] { requests_.erase(i); }};

        for (const auto& id : i->second) {
            router_.SendDeferred(
                [&] {
                    auto msg = network::zeromq::tagged_reply_to_connection(
                        id.Bytes(), OT_ZMQ_BLOCK_ORACLE_BLOCK_READY);
                    msg.AddFrame(hash);
                    msg.AddFrame(bytes.data(), bytes.size());

                    return msg;
                }(),
                __FILE__,
                __LINE__);
        }
    }
}

auto BlockOracle::Actor::notify_requestors(
    const block::Hash& hash,
    const std::uintptr_t pointer,
    const std::size_t size) noexcept -> void
{
    if (auto i = requests_.find(hash); requests_.end() != i) {
        auto post = ScopeGuard{[&] { requests_.erase(i); }};

        for (const auto& id : i->second) {
            router_.SendDeferred(
                [&] {
                    auto msg = network::zeromq::tagged_reply_to_connection(
                        id.Bytes(), OT_ZMQ_BLOCK_ORACLE_BLOCK_READY);
                    msg.AddFrame(hash);
                    msg.AddFrame(pointer);
                    msg.AddFrame(size);

                    return msg;
                }(),
                __FILE__,
                __LINE__);
        }
    }
}

auto BlockOracle::Actor::pipeline(
    const Work work,
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    using network::zeromq::SocketID;
    const auto socket = msg.Internal().ExtractFront().as<SocketID>();

    switch (work) {
        case Work::header: {
            process_header(std::move(msg));
        } break;
        case Work::reorg: {
            process_reorg(std::move(msg));
        } break;
        case Work::request_blocks: {
            if (router_.ID() == socket) {
                process_request_blocks(std::move(msg), monotonic);
            } else {
                LogAbort()(OT_PRETTY_CLASS())(name_)(" received ")(
                    print(work))()(" on pull socket")
                    .Abort();
            }
        } break;
        case Work::block_ready: {
            process_block_ready(std::move(msg));
        } break;
        case Work::report: {
            process_report(std::move(msg));
        } break;
        case Work::submit_block: {
            process_submit_block(std::move(msg));
        } break;
        case Work::shutdown:
        case Work::init:
        case Work::statemachine: {
            unhandled_type(work);
        }
        default: {
            unknown_type(work);
        }
    }

    if (download_blocks_) { do_work(monotonic); }
}

auto BlockOracle::Actor::process_block_ready(Message&& msg) noexcept -> void
{
    const auto body = msg.Body();

    for (auto n = 1_uz, stop = body.size(); n < stop; n += 2_uz) {
        const auto& idFrame = body.at(n);
        const auto& dataFrame = body.at(n + 1_uz);
        const auto hash = block::Hash{idFrame.Bytes()};
        block_is_ready(hash);
        using blockoracle::SerializedReadView;
        const auto view = [&]() -> std::optional<SerializedReadView> {
            auto buf = SerializedReadView{};

            if (dataFrame.size() == sizeof(buf)) {
                auto* p = reinterpret_cast<std::byte*>(std::addressof(buf));
                std::memcpy(p, dataFrame.data(), sizeof(buf));

                return buf;
            } else {

                return std::nullopt;
            }
        }();

        if (view.has_value()) {
            notify_requestors(hash, view->pointer_, view->size_);
        } else {
            notify_requestors(hash, dataFrame.Bytes());
        }
    }

    shared_.FinishWork();
}

auto BlockOracle::Actor::process_header(Message&& msg) noexcept -> void
{
    // NOTE no action required
}

auto BlockOracle::Actor::process_reorg(Message&& msg) noexcept -> void
{
    // NOTE no action required
}

auto BlockOracle::Actor::process_report(Message&& msg) noexcept -> void
{
    broadcast_tip();
}

auto BlockOracle::Actor::process_request_blocks(
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    const auto requestor = get_sender(msg);
    const auto body = msg.Body();
    const auto count = body.size();

    if (1_uz >= count) { return; }

    const auto hashes = [&] {
        auto out = Vector<block::Hash>{monotonic};
        out.reserve(count - 1_uz);

        for (auto n = 1_uz; n < count; ++n) {
            const auto& hash = out.emplace_back(body.at(n).Bytes());
            requests_[hash].emplace(requestor);
        }

        return out;
    }();
    const auto blocks = shared_.GetBlocks(hashes, monotonic, monotonic);
    auto h = hashes.cbegin();
    auto b = blocks.cbegin();

    for (auto end = blocks.cend(); b != end; ++b, ++h) {
        const auto& hash = *h;
        const auto& block = *b;
        notify_requestors(hash, block);
    }
}

auto BlockOracle::Actor::process_submit_block(Message&& msg) noexcept -> void
{
    const auto body = msg.Body();

    OT_ASSERT(1_uz < body.size());

    shared_.Receive(body.at(1).Bytes());
}

auto BlockOracle::Actor::queue_blocks(allocator_type monotonic) noexcept -> bool
{
    try {
        const auto& next = next_position();
        const auto data = node_.HeaderOracle().Ancestors(next, 0_uz, monotonic);

        OT_ASSERT(false == data.empty());

        const auto& ancestor = data.front();
        adjust_tip(ancestor);

        if (1_uz == data.size()) { return false; }

        const auto start = std::next(data.cbegin());
        const auto count = data.size() - 1_uz;
        constexpr auto limit = 250_uz;
        const auto effective = std::min(limit, count);
        auto height = start->height_;
        auto hashes = Vector<block::Hash>{monotonic};
        hashes.reserve(effective);
        auto i{start};

        for (auto n = 0_uz; n < effective; ++n, ++i) {
            const auto& position = *i;
            hashes.emplace_back(position.hash_);
            cache_[position.hash_] = position.height_;
            const auto [_, added] =
                downloading_.try_emplace(position.height_, position);

            OT_ASSERT(added);
        }

        const auto blocks = shared_.GetBlocks(hashes, monotonic, monotonic);

        OT_ASSERT(hashes.size() == blocks.size());

        using blockoracle::CachedBlock;
        using blockoracle::MissingBlock;
        using blockoracle::PersistentBlock;

        struct Exists {
            auto operator()(const MissingBlock&) noexcept -> bool
            {
                return false;
            }
            auto operator()(const PersistentBlock&) noexcept -> bool
            {
                return true;
            }
            auto operator()(const CachedBlock&) noexcept -> bool
            {
                return true;
            }
        };

        auto h = hashes.cbegin();
        auto b = blocks.cbegin();

        for (auto end = blocks.cend(); b != end; ++b, ++h, ++height) {
            const auto& id = *h;
            const auto& block = *b;

            if (std::visit(Exists{}, block) && block_is_ready(id, height)) {
                cache_.erase(id);
            }
        }

        return effective != count;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(name_)(": ")(e.what()).Flush();

        return true;
    }
}

auto BlockOracle::Actor::set_tip(const block::Position& tip) noexcept -> void
{
    if (shared_.SetTip(tip)) {
        tip_ = tip;
        broadcast_tip();
    } else {
        LogAbort()(OT_PRETTY_CLASS())(name_)(": failed to update database")
            .Abort();
    }
}

auto BlockOracle::Actor::update_progress() noexcept -> void
{
    auto tip{tip_};

    for (auto i = ready_.begin(); i != ready_.end();) {
        auto& [height, position] = *i;
        const auto target = tip.height_ + 1;

        if (height != target) { break; }

        tip = std::move(position);
        i = ready_.erase(i);
    }

    if (tip != tip_) { set_tip(tip); }
}

auto BlockOracle::Actor::work(allocator_type monotonic) noexcept -> bool
{
    if (download_blocks_) {
        auto out = queue_blocks(monotonic);
        update_progress();

        if (downloading_.empty() && ready_.empty()) { cache_.clear(); }

        return out;
    } else {

        return false;
    }
}

BlockOracle::Actor::~Actor() = default;
}  // namespace opentxs::blockchain::node::internal
