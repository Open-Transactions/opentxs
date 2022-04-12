// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <chrono>
#include <cstddef>
#include <future>
#include <memory>
#include <mutex>
#include <optional>
#include <string_view>
#include <tuple>
#include <utility>

#include "blockchain/node/blockoracle/MemDB.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Block;
}  // namespace bitcoin
}  // namespace block

namespace node
{
namespace internal
{
struct BlockDatabase;
struct Network;
}  // namespace internal
}  // namespace node
}  // namespace blockchain

namespace network
{
namespace zeromq
{
namespace socket
{
class Raw;
}  // namespace socket

class Message;
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::blockoracle
{
class Cache final : public Allocated
{
public:
    using BatchID = std::size_t;

    auto DownloadQueue() const noexcept -> std::size_t;
    auto get_allocator() const noexcept -> allocator_type final
    {
        return pending_.get_allocator();
    }

    auto FinishBatch(const BatchID id) noexcept -> void;
    auto GetBatch(allocator_type alloc) noexcept
        -> std::pair<BatchID, Vector<block::Hash>>;
    auto ProcessBlockRequests(zmq::Message&& in) noexcept -> void;
    auto ReceiveBlock(const zmq::Frame& in) noexcept -> void;
    auto ReceiveBlock(const std::string_view in) noexcept -> void;
    auto ReceiveBlock(std::shared_ptr<const block::bitcoin::Block> in) noexcept
        -> void;
    auto Request(const block::Hash& block) noexcept -> BitcoinBlockResult;
    auto Request(const Vector<block::Hash>& hashes) noexcept
        -> BitcoinBlockResults;
    auto Shutdown() noexcept -> void;
    auto StateMachine() noexcept -> bool;

    Cache(
        const api::Session& api_,
        const internal::Network& node,
        internal::BlockDatabase& db,
        const blockchain::Type chain,
        allocator_type alloc) noexcept;

    ~Cache() final { Shutdown(); }

private:
    using Promise = std::promise<std::shared_ptr<const block::bitcoin::Block>>;
    using PendingData = std::tuple<Time, Promise, BitcoinBlockResult, bool>;
    using Pending = Map<block::Hash, PendingData>;
    using RequestQueue = Deque<block::Hash>;
    using BatchIndex = Map<BatchID, std::pair<std::size_t, Set<block::Hash>>>;
    using HashIndex = Map<block::Hash, BatchID>;
    using HashCache = Set<block::Hash>;

    static const std::size_t cache_limit_;
    static const std::chrono::seconds download_timeout_;

    const api::Session& api_;
    const internal::Network& node_;
    internal::BlockDatabase& db_;
    const blockchain::Type chain_;
    opentxs::network::zeromq::socket::Raw block_available_;
    opentxs::network::zeromq::socket::Raw cache_size_publisher_;
    Pending pending_;
    RequestQueue queue_;
    BatchIndex batch_index_;
    HashIndex hash_index_;
    HashCache hash_cache_;
    MemDB mem_;
    std::optional<std::size_t> peer_target_;
    bool running_;

    static auto next_batch_id() noexcept -> BatchID;

    auto download(const block::Hash& block) const noexcept -> bool;

    auto get_peer_target() noexcept -> std::size_t;
    auto publish(const block::Hash& block) noexcept -> void;
    auto publish_download_queue() noexcept -> void;
    auto queue_hash(const block::Hash& id) noexcept -> void;
    auto receive_block(const block::Hash& id) noexcept -> void;
};
}  // namespace opentxs::blockchain::node::blockoracle
