// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "1_Internal.hpp"                         // IWYU pragma: associated
#include "blockchain/node/blockoracle/Cache.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <atomic>
#include <chrono>
#include <exception>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

#include "internal/api/network/Blockchain.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameIterator.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/ByteLiterals.hpp"

namespace opentxs::blockchain::node::blockoracle
{
const std::size_t Cache::cache_limit_{8_MiB};
const std::chrono::seconds Cache::download_timeout_{60};

Cache::Cache(
    const api::Session& api,
    const internal::Network& node,
    internal::BlockDatabase& db,
    const blockchain::Type chain,
    allocator_type alloc) noexcept
    : api_(api)
    , node_(node)
    , db_(db)
    , chain_(chain)
    , block_available_([&] {
        using Type = opentxs::network::zeromq::socket::Type;
        auto out = api.Network().ZeroMQ().Internal().RawSocket(Type::Push);
        const auto endpoint = UnallocatedCString{
            api.Network().Blockchain().Internal().BlockAvailableEndpoint()};
        const auto rc = out.Connect(endpoint.c_str());

        OT_ASSERT(rc);

        return out;
    }())
    , cache_size_publisher_([&] {
        using Type = opentxs::network::zeromq::socket::Type;
        auto out = api.Network().ZeroMQ().Internal().RawSocket(Type::Push);
        const auto endpoint = UnallocatedCString{
            api.Network().Blockchain().Internal().BlockQueueUpdateEndpoint()};
        const auto rc = out.Connect(endpoint.c_str());

        OT_ASSERT(rc);

        return out;
    }())
    , pending_(alloc)
    , queue_(alloc)
    , batch_index_(alloc)
    , hash_index_(alloc)
    , hash_cache_(alloc)
    , mem_(cache_limit_, alloc)
    , peer_target_(std::nullopt)
    , running_(true)
{
}

auto Cache::DownloadQueue() const noexcept -> std::size_t
{
    return queue_.size() + hash_index_.size();
}

auto Cache::FinishBatch(const BatchID id) noexcept -> void
{
    if (auto i = batch_index_.find(id); batch_index_.end() != i) {
        const auto& [original, remaining] = i->second;

        if (const auto count = remaining.size(); 0u < count) {
            LogTrace()(OT_PRETTY_CLASS())("batch")(id)(" cancelled with ")(
                count)(" of ")(original)(" hashes not downloaded")
                .Flush();
        }

        for (const auto& hash : remaining) {
            hash_index_.erase(hash);
            queue_.emplace_front(hash);
        }

        batch_index_.erase(i);
    } else {
        LogError()(OT_PRETTY_CLASS())("batch")(id)(" does not exist").Flush();
    }

    publish_download_queue();
}

auto Cache::GetBatch(allocator_type alloc) noexcept
    -> std::pair<BatchID, Vector<block::Hash>>
{
    // TODO define max in Params
    static constexpr auto max = std::size_t{50000};
    static constexpr auto min = std::size_t{10};
    const auto available = queue_.size();
    const auto peers = get_peer_target();
    // NOTE The batch size should approximate the value appropriate for ideal
    // load balancing across the number of peers which should be active, if the
    // number of blocks which should be downloaded exceeds a minimum threshold.
    // The value is capped at a maximum size to prevent exceeding protocol
    // limits for inv requests.
    static constexpr auto GetTarget =
        [](auto available, auto peers, auto max, auto min) {
            decltype(peers) min_peers{1u};
            return std::min(
                max,
                std::min(
                    available,
                    std::max(min, available / std::max(peers, min_peers))));
        };

    static_assert(GetTarget(1, 0, 50000, 10) == 1);
    static_assert(GetTarget(1, 4, 50000, 10) == 1);
    static_assert(GetTarget(9, 0, 50000, 10) == 9);
    static_assert(GetTarget(9, 4, 50000, 10) == 9);
    static_assert(GetTarget(11, 4, 50000, 10) == 10);
    static_assert(GetTarget(11, 0, 50000, 10) == 11);
    static_assert(GetTarget(40, 4, 50000, 10) == 10);
    static_assert(GetTarget(40, 0, 50000, 10) == 40);
    static_assert(GetTarget(45, 4, 50000, 10) == 11);
    static_assert(GetTarget(45, 2, 50000, 10) == 22);
    static_assert(GetTarget(45, 0, 50000, 10) == 45);
    static_assert(GetTarget(45, 2, 2, 10) == 2);
    static_assert(GetTarget(45, 0, 2, 10) == 2);
    static_assert(GetTarget(0, 2, 50000, 10) == 0);
    static_assert(GetTarget(0, 0, 50000, 10) == 0);
    static_assert(GetTarget(1000000, 4, 50000, 10) == 50000);
    static_assert(GetTarget(1000000, 0, 50000, 10) == 50000);
    const auto target = GetTarget(available, peers, max, min);
    LogTrace()(OT_PRETTY_CLASS())("creating download batch for ")(
        target)(" block hashes out of ")(available)(" waiting in queue")
        .Flush();
    auto out = std::make_pair(next_batch_id(), Vector<block::Hash>{alloc});
    const auto& batchID = out.first;
    auto& hashes = out.second;
    hashes.reserve(target);
    auto& [count, index] = batch_index_[batchID];
    count = target;

    while (hashes.size() < target) {
        const auto& hash = queue_.front();
        hashes.emplace_back(hash);
        index.emplace(hash);
        hash_index_.emplace(hash, batchID);
        queue_.pop_front();
        hash_cache_.erase(hash);
    }

    return out;
}

auto Cache::get_peer_target() noexcept -> std::size_t
{
    if (false == peer_target_.has_value()) {
        peer_target_ = node_.PeerTarget();
    }

    return peer_target_.value();
}

auto Cache::next_batch_id() noexcept -> BatchID
{
    static auto counter = std::atomic<BatchID>{0};

    return ++counter;
}

auto Cache::ProcessBlockRequests(zmq::Message&& in) noexcept -> void
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

auto Cache::publish(const block::Hash& block) noexcept -> void
{
    block_available_.SendDeferred([&] {
        auto work =
            network::zeromq::tagged_message(WorkType::BlockchainBlockAvailable);
        work.AddFrame(chain_);
        work.AddFrame(block);

        return work;
    }());
}

auto Cache::publish_download_queue() noexcept -> void
{
    const auto waiting = queue_.size();
    const auto assigned = hash_index_.size();
    const auto total = waiting + assigned;
    LogTrace()(OT_PRETTY_CLASS())(total)(" in download queue: ")(
        waiting)(" waiting / ")(assigned)(" assigned")
        .Flush();
    cache_size_publisher_.SendDeferred([&] {
        auto work = network::zeromq::tagged_message(
            WorkType::BlockchainBlockDownloadQueue);
        work.AddFrame(chain_);
        work.AddFrame(total);

        return work;
    }());
}

auto Cache::queue_hash(const block::Hash& hash) noexcept -> void
{
    if (0u < hash_cache_.count(hash)) { return; }
    if (0u < hash_index_.count(hash)) { return; }
    if (db_.BlockExists(hash)) { return; }

    queue_.emplace_back(hash);
}

auto Cache::ReceiveBlock(const zmq::Frame& in) noexcept -> void
{
    ReceiveBlock(in.Bytes());
}

auto Cache::ReceiveBlock(const std::string_view in) noexcept -> void
{
    ReceiveBlock(api_.Factory().BitcoinBlock(chain_, in));
}

auto Cache::ReceiveBlock(
    std::shared_ptr<const block::bitcoin::Block> in) noexcept -> void
{
    if (false == bool(in)) {
        LogError()(OT_PRETTY_CLASS())("Invalid block").Flush();

        return;
    }

    auto& block = *in;

    if (database::BlockStorage::None != db_.BlockPolicy()) {
        const auto saved = db_.BlockStore(block);

        OT_ASSERT(saved);
    }

    const auto& id = block.ID();
    receive_block(id);
    auto pending = pending_.find(id);

    if (pending_.end() == pending) {
        LogVerbose()(OT_PRETTY_CLASS())("Received block not in request list")
            .Flush();

        return;
    }

    auto& [time, promise, future, queued] = pending->second;
    promise.set_value(std::move(in));
    publish(id);
    LogVerbose()(OT_PRETTY_CLASS())("Cached block ")(id.asHex()).Flush();
    mem_.push(block::Hash{id}, std::move(future));
    pending_.erase(pending);
    publish_download_queue();
}

auto Cache::receive_block(const block::Hash& id) noexcept -> void
{
    if (auto i = hash_index_.find(id); hash_index_.end() != i) {
        const auto batch = i->second;
        batch_index_.at(batch).second.erase(id);
        hash_index_.erase(i);
    }
}

auto Cache::Request(const block::Hash& block) noexcept -> BitcoinBlockResult
{
    const auto output = Request(Vector<block::Hash>{block});

    OT_ASSERT(1 == output.size());

    return output.at(0);
}

auto Cache::Request(const Vector<block::Hash>& hashes) noexcept
    -> BitcoinBlockResults
{
    auto output = BitcoinBlockResults{};
    output.reserve(hashes.size());
    auto ready = UnallocatedVector<const block::Hash*>{};
    auto download =
        UnallocatedMap<block::Hash, BitcoinBlockResults::iterator>{};

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

            continue;
        }

        {
            auto it = pending_.find(block);

            if (pending_.end() != it) {
                const auto& [time, promise, future, queued] = it->second;
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
        auto blockList = UnallocatedVector<ReadView>{};
        std::transform(
            std::begin(download),
            std::end(download),
            std::back_inserter(blockList),
            [](const auto& in) -> auto {
                const auto& [key, value] = in;

                return key.Bytes();
            });
        LogVerbose()(OT_PRETTY_CLASS())("Downloading ")(blockList.size())(
            " blocks from peers")
            .Flush();
        // TODO broadcast a signal which will notify active Peer objects to
        // check for work

        for (auto& [hash, futureOut] : download) {
            queue_hash(hash);
            auto& [time, promise, future, queued] = pending_[hash];
            time = Clock::now();
            future = promise.get_future();
            *futureOut = future;
            queued = true;
        }

        publish_download_queue();
    }

    for (const auto* hash : ready) { publish(*hash); }

    return output;
}

auto Cache::Shutdown() noexcept -> void
{
    if (running_) {
        running_ = false;
        mem_.clear();

        for (auto& [hash, item] : pending_) {
            auto& [time, promise, future, queued] = item;
            promise.set_value(nullptr);
        }

        pending_.clear();
        publish_download_queue();
    }
}

auto Cache::StateMachine() noexcept -> bool
{
    if (false == running_) { return false; }

    LogVerbose()(OT_PRETTY_CLASS())(print(chain_))(" download queue contains ")(
        pending_.size())(" blocks.")
        .Flush();
    auto blockList = UnallocatedVector<ReadView>{};
    blockList.reserve(pending_.size());

    for (auto& [hash, item] : pending_) {
        auto& [time, promise, future, queued] = item;
        const auto now = Clock::now();
        namespace c = std::chrono;
        const auto elapsed = std::chrono::nanoseconds{now - time};
        const auto timeout = download_timeout_ <= elapsed;

        if (timeout || (false == queued)) {
            LogVerbose()(OT_PRETTY_CLASS())("Requesting ")(print(chain_))(
                " block ")(hash.asHex())(" from peers")
                .Flush();
            blockList.emplace_back(hash.Bytes());
            queued = true;
            time = now;
        } else {
            LogVerbose()(OT_PRETTY_CLASS())(elapsed)(" elapsed waiting for ")(
                print(chain_))(" block ")(hash.asHex())
                .Flush();
        }
    }

    if (0 < blockList.size()) { node_.RequestBlocks(blockList); }

    return 0 < pending_.size();
}
}  // namespace opentxs::blockchain::node::blockoracle
