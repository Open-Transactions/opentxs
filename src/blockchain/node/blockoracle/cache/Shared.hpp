// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <cs_shared_guarded.h>
#include <chrono>
#include <cstddef>
#include <future>
#include <memory>
#include <shared_mutex>
#include <string_view>
#include <tuple>
#include <utility>

#include "blockchain/node/blockoracle/cache/Cache.hpp"
#include "blockchain/node/blockoracle/cache/MemDB.hpp"
#include "internal/blockchain/node/Job.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "util/Allocated.hpp"
#include "util/ByteLiterals.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace bitcoin
{
namespace block
{
class Block;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Hash;
}  // namespace block

namespace database
{
class Block;
}  // namespace database

namespace node
{
namespace internal
{
class BlockBatch;
}  // namespace internal

class Manager;
}  // namespace node
}  // namespace blockchain

namespace network
{
namespace zeromq
{
class Frame;
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::blockoracle
{
using namespace std::literals;

class Cache::Shared final : public Allocated
{
public:
    struct Data final : public Allocated {
        auto DownloadQueue() const noexcept -> std::size_t;
        auto get_allocator() const noexcept -> allocator_type final;

        auto FinishBatch(const BatchID id) noexcept -> void;
        auto GetBatch(allocator_type alloc) noexcept
            -> std::pair<BatchID, Vector<block::Hash>>;
        auto ProcessBlockRequests(network::zeromq::Message&& in) noexcept
            -> void;
        auto ReceiveBlock(const std::string_view in) noexcept -> void;
        auto ReceiveBlock(
            std::shared_ptr<const bitcoin::block::Block> in) noexcept -> bool;
        auto Request(const block::Hash& block) noexcept -> BitcoinBlockResult;
        auto Request(const Vector<block::Hash>& hashes) noexcept
            -> BitcoinBlockResults;
        auto Shutdown() noexcept -> void;
        auto StateMachine() noexcept -> bool;

        Data(
            const api::Session& api,
            const node::Manager& node,
            network::zeromq::BatchID batchID,
            allocator_type alloc) noexcept;
        Data() = delete;
        Data(const Data&) = delete;
        Data(Data&&) = delete;
        auto operator=(const Data&) -> Data& = delete;
        auto operator=(Data&&) -> Data& = delete;

        ~Data() final;

    private:
        using Promise =
            std::promise<std::shared_ptr<const bitcoin::block::Block>>;
        using PendingData = std::pair<Promise, BitcoinBlockResult>;
        using Pending = Map<block::Hash, PendingData>;
        using RequestQueue = Deque<block::Hash>;
        using BatchIndex =
            Map<BatchID, std::pair<const std::size_t, Set<block::Hash>>>;
        using HashIndex = Map<block::Hash, BatchID>;
        using HashCache = Set<block::Hash>;

        static constexpr auto cache_limit_{8_mib};
        static constexpr auto download_timeout_{60s};

        const api::Session& api_;
        const node::Manager& node_;
        database::Block& db_;
        const network::zeromq::BatchID batch_id_;
        const blockchain::Type chain_;
        const bool save_blocks_;
        const std::size_t peer_target_;
        opentxs::network::zeromq::socket::Raw to_blockchain_api_;
        opentxs::network::zeromq::socket::Raw job_ready_;
        opentxs::network::zeromq::socket::Raw to_header_oracle_;
        opentxs::network::zeromq::socket::Raw to_block_fetcher_;
        mutable opentxs::network::zeromq::socket::Raw to_actor_;
        Pending pending_;
        RequestQueue queue_;
        BatchIndex batch_index_;
        HashIndex block_id_to_batch_id_;
        HashCache queued_block_index_;
        MemDB mem_;
        bool running_;

        auto check_consistency() const noexcept -> void;
        auto check_consistency(
            std::size_t total,
            std::size_t assigned,
            std::size_t waiting) const noexcept -> void;
        auto download(const block::Hash& block) const noexcept -> bool;
        auto trigger() const noexcept -> void;

        auto notify_batch_available() noexcept -> void;
        auto publish(const block::Hash& block) noexcept -> void;
        auto publish_download_queue() noexcept -> void;
        auto queue_hash(const block::Hash& id) noexcept -> void;
        auto receive_block(const block::Hash& id) noexcept -> void;
    };

    using GuardedData = libguarded::shared_guarded<Data, std::shared_mutex>;
    GuardedData data_;

    auto DownloadQueue() const noexcept -> std::size_t;
    auto get_allocator() const noexcept -> allocator_type final;

    auto GetBlockBatch(
        boost::shared_ptr<Shared> me,
        alloc::Default alloc) noexcept -> node::internal::BlockBatch;
    auto ReceiveBlock(std::shared_ptr<const bitcoin::block::Block> in) noexcept
        -> bool;
    auto Request(const block::Hash& block) noexcept -> BitcoinBlockResult;
    auto Request(const Vector<block::Hash>& hashes) noexcept
        -> BitcoinBlockResults;

    Shared(
        const api::Session& api,
        const node::Manager& node,
        network::zeromq::BatchID batchID,
        allocator_type alloc) noexcept;

    ~Shared() final;
};
}  // namespace opentxs::blockchain::node::blockoracle
