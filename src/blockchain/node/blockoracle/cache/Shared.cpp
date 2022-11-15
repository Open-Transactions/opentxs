// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "blockchain/node/blockoracle/cache/Shared.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <chrono>
#include <exception>
#include <iterator>
#include <memory>
#include <utility>

#include "blockchain/node/blockoracle/BlockBatch.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/blockchain/database/Block.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/node/Config.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Job.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/blockoracle/BlockBatch.hpp"
#include "internal/blockchain/node/blockoracle/Types.hpp"
#include "internal/blockchain/node/headeroracle/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/bitcoin/block/Block.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameIterator.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/util/BlockchainProfile.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WorkType.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/ScopeGuard.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::blockoracle
{
Cache::Shared::Data::Data(
    const api::Session& api,
    const node::Manager& node,
    network::zeromq::BatchID batchID,
    allocator_type alloc) noexcept
    : api_(api)
    , node_(node)
    , db_(node_.Internal().DB())
    , batch_id_(std::move(batchID))
    , chain_(node_.Internal().Chain())
    , save_blocks_([&] {
        switch (node_.Internal().GetConfig().profile_) {
            case BlockchainProfile::mobile: {

                return false;
            }
            case BlockchainProfile::desktop:
            case BlockchainProfile::desktop_native:
            case BlockchainProfile::server: {

                return true;
            }
            default: {
                LogAbort()(OT_PRETTY_CLASS())("invalid profile").Abort();
            }
        }
    }())
    , peer_target_(node_.Internal().GetConfig().PeerTarget(chain_))
    , to_blockchain_api_([&] {
        using Type = opentxs::network::zeromq::socket::Type;
        auto out = api.Network().ZeroMQ().Internal().RawSocket(Type::Push);
        const auto endpoint = UnallocatedCString{
            api.Endpoints().Internal().Internal().BlockchainMessageRouter()};
        const auto rc = out.Connect(endpoint.c_str());

        OT_ASSERT(rc);

        return out;
    }())
    , job_ready_([&] {
        using Type = opentxs::network::zeromq::socket::Type;
        auto out = api.Network().ZeroMQ().Internal().RawSocket(Type::Publish);
        const auto rc =
            out.Connect(node_.Internal()
                            .Endpoints()
                            .block_cache_job_ready_publish_.c_str());

        OT_ASSERT(rc);

        return out;
    }())
    , to_header_oracle_([&] {
        using Type = opentxs::network::zeromq::socket::Type;
        auto out = api.Network().ZeroMQ().Internal().RawSocket(Type::Push);
        const auto rc = out.Connect(
            node_.Internal().Endpoints().header_oracle_pull_.c_str());

        OT_ASSERT(rc);

        return out;
    }())
    , to_block_fetcher_([&] {
        using Type = opentxs::network::zeromq::socket::Type;
        auto out = api.Network().ZeroMQ().Internal().RawSocket(Type::Push);
        const auto rc = out.Connect(
            node_.Internal().Endpoints().block_fetcher_pull_.c_str());

        OT_ASSERT(rc);

        return out;
    }())
    , to_actor_([&] {
        using Type = opentxs::network::zeromq::socket::Type;
        auto out = api.Network().ZeroMQ().Internal().RawSocket(Type::Push);
        const auto rc =
            out.Connect(node_.Internal().Endpoints().block_cache_pull_.c_str());

        OT_ASSERT(rc);

        return out;
    }())
    , pending_(alloc)
    , queue_(alloc)
    , batch_index_(alloc)
    , block_id_to_batch_id_(alloc)
    , queued_block_index_(alloc)
    , mem_(cache_limit_, alloc)
    , running_(true)
{
}

auto Cache::Shared::Data::check_consistency() const noexcept -> void
{
    check_consistency(
        queued_block_index_.size(),
        block_id_to_batch_id_.size(),
        queue_.size());
}

auto Cache::Shared::Data::check_consistency(
    std::size_t total,
    std::size_t assigned,
    std::size_t waiting) const noexcept -> void
{
    if (total != (waiting + assigned)) {
        LogConsole()(PrintStackTrace()).Flush();
        LogAbort()(OT_PRETTY_CLASS())("instance ")(api_.Instance())(
            " queued block count (")(
            total)(") does not match sum of downloading (")(
            assigned)(") and waiting (")(waiting)(")")
            .Abort();
    }
}

auto Cache::Shared::Data::DownloadQueue() const noexcept -> std::size_t
{
    return queued_block_index_.size();
}

auto Cache::Shared::Data::FinishBatch(const BatchID id) noexcept -> void
{
    if (auto i = batch_index_.find(id); batch_index_.end() != i) {
        const auto& [original, remaining] = i->second;

        if (const auto count = remaining.size(); 0_uz < count) {
            LogTrace()(OT_PRETTY_CLASS())("batch")(id)(" cancelled with ")(
                count)(" of ")(original)(" hashes not downloaded")
                .Flush();
        }

        for (const auto& hash : remaining) {
            OT_ASSERT(0_uz < queued_block_index_.count(hash));

            block_id_to_batch_id_.erase(hash);
            queue_.emplace_front(hash);
        }

        batch_index_.erase(i);
    } else {
        LogError()(OT_PRETTY_CLASS())("batch")(id)(" does not exist").Flush();
    }

    publish_download_queue();
}

auto Cache::Shared::Data::GetBatch(allocator_type alloc) noexcept
    -> std::pair<BatchID, Vector<block::Hash>>
{
    // TODO define max in Params
    static constexpr auto max = 50000_uz;
    static constexpr auto min = 10_uz;
    const auto available = queue_.size();
    using download::batch_size;
    static_assert(batch_size(1, 0, 50000, 10) == 1);
    static_assert(batch_size(1, 4, 50000, 10) == 1);
    static_assert(batch_size(9, 0, 50000, 10) == 9);
    static_assert(batch_size(9, 4, 50000, 10) == 9);
    static_assert(batch_size(11, 4, 50000, 10) == 10);
    static_assert(batch_size(11, 0, 50000, 10) == 11);
    static_assert(batch_size(40, 4, 50000, 10) == 10);
    static_assert(batch_size(40, 0, 50000, 10) == 40);
    static_assert(batch_size(45, 4, 50000, 10) == 11);
    static_assert(batch_size(45, 2, 50000, 10) == 22);
    static_assert(batch_size(45, 0, 50000, 10) == 45);
    static_assert(batch_size(45, 2, 2, 10) == 2);
    static_assert(batch_size(45, 0, 2, 10) == 2);
    static_assert(batch_size(0, 2, 50000, 10) == 0);
    static_assert(batch_size(0, 0, 50000, 10) == 0);
    static_assert(batch_size(1000000, 4, 50000, 10) == 50000);
    static_assert(batch_size(1000000, 0, 50000, 10) == 50000);
    const auto target = batch_size(available, peer_target_, max, min);

    if (0_uz == target) {
        return std::make_pair(-1, Vector<block::Hash>{alloc});
    }

    LogTrace()(OT_PRETTY_CLASS())("creating download batch for ")(
        target)(" block hashes out of ")(available)(" waiting in queue")
        .Flush();
    auto out = std::make_pair(download::next_job(), Vector<block::Hash>{alloc});
    const auto& batchID = out.first;
    auto& hashes = out.second;
    hashes.reserve(target);
    auto [i, rc] = batch_index_.try_emplace(
        batchID, target, Set<block::Hash>{get_allocator()});

    OT_ASSERT(rc);

    auto& [count, index] = i->second;

    while (hashes.size() < target) {
        const auto& hash = queue_.front();

        // TODO c++20 use contains
        OT_ASSERT(0_uz < queued_block_index_.count(hash));

        hashes.emplace_back(hash);
        index.emplace(hash);
        const auto [j, added] =
            block_id_to_batch_id_.try_emplace(hash, batchID);

        if (false == added) {
            LogAbort()(OT_PRETTY_CLASS())("block ")
                .asHex(hash)(" already assigned to batch ")(j->second)
                .Abort();
        }

        queue_.pop_front();
    }

    OT_ASSERT(out.second.size() <= block_id_to_batch_id_.size());

    check_consistency();

    return out;
}

auto Cache::Shared::Data::get_allocator() const noexcept -> allocator_type
{
    return pending_.get_allocator();
}

auto Cache::Shared::Data::notify_batch_available() noexcept -> void
{
    job_ready_.SendDeferred(
        MakeWork(OT_ZMQ_BLOCK_BATCH_JOB_AVAILABLE), __FILE__, __LINE__);
}

auto Cache::Shared::Data::ProcessBlockRequests(
    network::zeromq::Message&& in) noexcept -> void
{
    if (false == running_) { return; }

    const auto body = in.Body();
    LogTrace()(OT_PRETTY_CLASS())("received a request for ")(body.size() - 1u)(
        " block hashes")
        .Flush();

    for (auto f = std::next(body.begin()), end = body.end(); f != end; ++f) {
        try {
            const auto hash = block::Hash{f->Bytes()};
            queue_hash(hash);
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
        }
    }

    publish_download_queue();
}

auto Cache::Shared::Data::publish(const block::Hash& block) noexcept -> void
{
    to_blockchain_api_.SendDeferred(
        [&] {
            auto work = network::zeromq::tagged_message(
                WorkType::BlockchainBlockAvailable);
            work.AddFrame(chain_);
            work.AddFrame(block);

            return work;
        }(),
        __FILE__,
        __LINE__);
}

auto Cache::Shared::Data::publish_download_queue() noexcept -> void
{
    const auto waiting = queue_.size();
    const auto assigned = block_id_to_batch_id_.size();
    const auto total = queued_block_index_.size();
    check_consistency(total, assigned, waiting);
    LogTrace()(OT_PRETTY_CLASS())(total)(" in download queue: ")(
        waiting)(" waiting / ")(assigned)(" downloading")
        .Flush();
    to_blockchain_api_.SendDeferred(
        [&] {
            auto work = network::zeromq::tagged_message(
                WorkType::BlockchainBlockDownloadQueue);
            work.AddFrame(chain_);
            work.AddFrame(total);

            return work;
        }(),
        __FILE__,
        __LINE__);
}

auto Cache::Shared::Data::queue_hash(const block::Hash& hash) noexcept -> void
{
    // TODO c++20 use contains
    if (0_uz < queued_block_index_.count(hash)) { return; }
    if (db_.BlockExists(hash)) { return; }

    queued_block_index_.emplace(hash);
    queue_.emplace_back(hash);
    check_consistency();
}

auto Cache::Shared::Data::ReceiveBlock(const std::string_view in) noexcept
    -> void
{
    ReceiveBlock(api_.Factory().BitcoinBlock(chain_, in));
}

auto Cache::Shared::Data::ReceiveBlock(
    std::shared_ptr<const bitcoin::block::Block> in) noexcept -> bool
{
    if (false == in.operator bool()) {
        LogError()(OT_PRETTY_CLASS())("Invalid block").Flush();

        return false;
    }

    const auto& block = *in;

    if (save_blocks_) {
        const auto saved = db_.BlockStore(block);

        OT_ASSERT(saved);
    }

    auto id = block::Hash{block.ID()};

    if (false == node_.HeaderOracle().Exists(id)) {
        to_header_oracle_.SendDeferred(
            [&] {
                using Task = headeroracle::Job;
                auto work = MakeWork(Task::submit_block_header);
                block.Header().Serialize(work.AppendBytes(), false);

                return work;
            }(),
            __FILE__,
            __LINE__);
    }

    auto future = [&]() -> BitcoinBlockResult {
        if (auto pending = pending_.find(id); pending_.end() == pending) {
            auto promise = Promise{};
            promise.set_value(std::move(in));

            return promise.get_future();
        } else {
            auto& [promise, future] = pending->second;
            promise.set_value(std::move(in));
            auto out{future};
            pending_.erase(pending);

            return out;
        }
    }();

    receive_block(id);
    publish(id);
    publish_download_queue();
    LogVerbose()(OT_PRETTY_CLASS())("Cached block ").asHex(id).Flush();
    mem_.push(std::move(id), std::move(future));
    to_block_fetcher_.SendDeferred(
        MakeWork(BlockFetcherJob::statemachine), __FILE__, __LINE__, true);

    return true;
}

auto Cache::Shared::Data::receive_block(const block::Hash& id) noexcept -> void
{
    auto& index = block_id_to_batch_id_;

    if (auto i = index.find(id); index.end() != i) {
        auto post = ScopeGuard{[&] { index.erase(i); }};
        const auto& batch = i->second;
        LogTrace()(OT_PRETTY_CLASS())(" block ")
            .asHex(id)(" was assigned to batch ")(batch)
            .Flush();
        auto count = batch_index_.at(i->second).second.erase(id);

        OT_ASSERT(1_uz == count);

        count = queued_block_index_.erase(id);

        OT_ASSERT(1_uz == count);
    } else {
        LogTrace()(OT_PRETTY_CLASS())(" block ")
            .asHex(id)(" was not assigned to any batches")
            .Flush();
    }

    if (0_uz < queued_block_index_.count(id)) {
        LogTrace()(OT_PRETTY_CLASS())("somehow received block ")
            .asHex(id)(" before requesting it")
            .Flush();
        queued_block_index_.erase(id);

        for (auto i = queue_.begin(); i != queue_.end();) {
            const auto& hash = *i;

            if (id == hash) {
                i = queue_.erase(i);

                break;
            } else {
                ++i;
            }
        }
    }

    OT_ASSERT(0_uz == queued_block_index_.count(id));

    check_consistency();
}

auto Cache::Shared::Data::Request(const block::Hash& block) noexcept
    -> BitcoinBlockResult
{
    const auto output = Request(Vector<block::Hash>{block});

    OT_ASSERT(1 == output.size());

    return output.at(0);
}

auto Cache::Shared::Data::Request(const Vector<block::Hash>& hashes) noexcept
    -> BitcoinBlockResults
{
    auto alloc = get_allocator();
    auto output = BitcoinBlockResults{};
    output.reserve(hashes.size());
    auto ready = Vector<const block::Hash*>{alloc};
    auto download = Map<block::Hash, BitcoinBlockResults::iterator>{alloc};

    if (false == running_) {
        std::for_each(hashes.begin(), hashes.end(), [&](const auto&) {
            auto promise = Promise{};
            promise.set_value(nullptr);
            output.emplace_back(promise.get_future());
        });

        return output;
    }

    for (const auto& block : hashes) {
        const auto& log = LogTrace();
        const auto start = Clock::now();
        auto found{false};

        if (auto future = mem_.find(block.Bytes()); future.valid()) {
            output.emplace_back(std::move(future));
            ready.emplace_back(&block);
            found = true;
        }

        const auto mem = Clock::now();

        if (found) {
            log(OT_PRETTY_CLASS())(" block is cached in memory. Found in ")(
                std::chrono::nanoseconds{mem - start})
                .Flush();
            receive_block(block);

            continue;
        }

        {
            auto it = pending_.find(block);

            if (pending_.end() != it) {
                const auto& [promise, future] = it->second;
                output.emplace_back(future);
                found = true;
            }
        }

        const auto pending = Clock::now();

        if (found) {
            log(OT_PRETTY_CLASS())(
                " block is already in download queue. Found in ")(
                std::chrono::nanoseconds{pending - mem})
                .Flush();

            continue;
        }

        if (auto pBlock = db_.BlockLoadBitcoin(block); bool(pBlock)) {
            // TODO this should be checked in the block factory function
            OT_ASSERT(pBlock->ID() == block);

            auto promise = Promise{};
            promise.set_value(std::move(pBlock));
            mem_.push(block::Hash{block}, promise.get_future());
            output.emplace_back(mem_.find(block.Bytes()));
            ready.emplace_back(&block);
            found = true;
        }

        const auto disk = Clock::now();

        if (found) {
            log(OT_PRETTY_CLASS())(
                " block is already downloaded. Loaded from storage in ")(
                std::chrono::nanoseconds{disk - pending})
                .Flush();
            receive_block(block);

            continue;
        }

        output.emplace_back();
        auto it = output.begin();
        std::advance(it, output.size() - 1);
        download.emplace(block, it);

        log(OT_PRETTY_CLASS())(" block queued for download in ")(
            std::chrono::nanoseconds{Clock::now() - pending})
            .Flush();
    }

    OT_ASSERT(output.size() == hashes.size());

    if (0 < download.size()) {
        LogVerbose()(OT_PRETTY_CLASS())("Downloading ")(download.size())(
            " blocks from peers")
            .Flush();

        for (auto& [hash, futureOut] : download) {
            queue_hash(hash);
            auto& [promise, future] = pending_[hash];
            future = promise.get_future();
            *futureOut = future;
        }

        notify_batch_available();
        publish_download_queue();
    }

    for (const auto* hash : ready) { publish(*hash); }

    return output;
}

auto Cache::Shared::Data::Shutdown() noexcept -> void
{
    if (running_) {
        running_ = false;
        mem_.clear();

        for (auto& [hash, item] : pending_) {
            auto& [promise, future] = item;
            promise.set_value(nullptr);
        }

        pending_.clear();
        publish_download_queue();
    }
}

auto Cache::Shared::Data::StateMachine() noexcept -> bool
{
    for (const auto& [id, data] : pending_) {
        // TODO c++20 use contains
        if (0_uz < queued_block_index_.count(id)) { continue; }

        queue_hash(id);
    }

    if (0_uz < DownloadQueue()) {
        notify_batch_available();

        return true;
    }

    return false;
}

auto Cache::Shared::Data::trigger() const noexcept -> void
{
    to_actor_.SendDeferred(
        MakeWork(CacheJob::statemachine), __FILE__, __LINE__);
}

Cache::Shared::Data::~Data() { Shutdown(); }
}  // namespace opentxs::blockchain::node::blockoracle

namespace opentxs::blockchain::node::blockoracle
{
Cache::Shared::Shared(
    const api::Session& api,
    const node::Manager& node,
    network::zeromq::BatchID batchID,
    allocator_type alloc) noexcept
    : data_(api, node, std::move(batchID), std::move(alloc))
{
}

auto Cache::Shared::DownloadQueue() const noexcept -> std::size_t
{
    return data_.lock_shared()->DownloadQueue();
}

auto Cache::Shared::get_allocator() const noexcept -> allocator_type
{
    return data_.lock_shared()->get_allocator();
}

auto Cache::Shared::GetBlockBatch(
    boost::shared_ptr<Shared> me,
    alloc::Default alloc) noexcept -> node::internal::BlockBatch
{
    auto pmr = alloc::PMR<node::internal::BlockBatch::Imp>{alloc};
    auto [id, hashes] = data_.lock()->GetBatch(alloc);

    if (hashes.empty()) {

        return {};
    } else {
        const auto batchID{id};  // TODO c++20 lambda capture structured binding
        auto* imp = pmr.allocate(1_uz);
        pmr.construct(
            imp,
            id,
            std::move(hashes),
            [me](const auto bytes) { me->data_.lock()->ReceiveBlock(bytes); },
            [me, batchID] { me->data_.lock()->FinishBatch(batchID); });

        return imp;
    }
}

auto Cache::Shared::ReceiveBlock(
    std::shared_ptr<const bitcoin::block::Block> in) noexcept -> bool
{
    return data_.lock()->ReceiveBlock(std::move(in));
}

auto Cache::Shared::Request(const block::Hash& block) noexcept
    -> BitcoinBlockResult
{
    return data_.lock()->Request(block);
}

auto Cache::Shared::Request(const Vector<block::Hash>& hashes) noexcept
    -> BitcoinBlockResults
{
    return data_.lock()->Request(hashes);
}

Cache::Shared::~Shared() = default;
}  // namespace opentxs::blockchain::node::blockoracle
