// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/enable_shared_from.hpp>
#include <cs_plain_guarded.h>
#include <cstddef>
#include <memory>
#include <span>
#include <tuple>

#include "blockchain/node/blockoracle/Cache.hpp"
#include "blockchain/node/blockoracle/Futures.hpp"
#include "blockchain/node/blockoracle/Queue.hpp"
#include "blockchain/node/blockoracle/Update.hpp"
#include "internal/blockchain/node/Job.hpp"
#include "internal/blockchain/node/blockoracle/BlockOracle.hpp"
#include "internal/blockchain/node/blockoracle/Types.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace database
{
class Block;
}  // namespace database

namespace block
{
class Block;
class Hash;
class Header;
class Validator;
}  // namespace block

namespace node
{
namespace internal
{
class BlockBatch;
}  // namespace internal

class HeaderOracle;
class Manager;
}  // namespace node
}  // namespace blockchain

class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::internal
{
using namespace blockoracle;

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wnon-virtual-dtor"
class BlockOracle::Shared final : public Allocated,
                                  public boost::enable_shared_from
{
public:
    const Log& log_;
    const api::Session& api_;
    const node::Manager& node_;
    const blockchain::Type chain_;
    const CString name_;
    const block::Position genesis_;

    auto DownloadQueue() const noexcept -> std::size_t;
    auto FinishJob(download::JobID job) const noexcept -> void;
    auto FinishWork() noexcept -> void;
    auto GetBlocks(
        Hashes hashes,
        allocator_type monotonic,
        allocator_type alloc) const noexcept -> Vector<BlockLocation>;
    auto GetWork(alloc::Default alloc) const noexcept -> BlockBatch;
    auto get_allocator() const noexcept -> allocator_type final;
    auto Load(const block::Hash& block, allocator_type monotonic) const noexcept
        -> BlockResult;
    auto Load(Hashes hashes, allocator_type alloc, allocator_type monotonic)
        const noexcept -> BlockResults;
    auto Receive(const ReadView block, allocator_type monotonic) const noexcept
        -> bool;
    auto SubmitBlock(
        const blockchain::block::Block& in,
        allocator_type monotonic) const noexcept -> bool;
    auto Tip() const noexcept -> block::Position;

    auto GetTip(allocator_type monotonic) noexcept -> block::Position;
    auto SetTip(const block::Position& tip) noexcept -> bool;

    Shared(
        const api::Session& api,
        const node::Manager& node,
        allocator_type alloc) noexcept;
    Shared() = delete;
    Shared(const Shared&) = delete;
    Shared(Shared&&) = delete;
    auto operator=(const Shared&) -> Shared& = delete;
    auto operator=(Shared&&) -> Shared& = delete;

    ~Shared() final;

private:
    using GuardedCache = libguarded::plain_guarded<Cache>;
    using GuardedFutures = libguarded::plain_guarded<Futures>;
    using GuardedQueue = libguarded::plain_guarded<Queue>;
    using GuardedUpdate = libguarded::plain_guarded<Update>;
    using GuardedSocket =
        libguarded::plain_guarded<network::zeromq::socket::Raw>;
    using BlockData =
        std::tuple<const block::Hash*, const BlockLocation*, int*>;

    database::Block& db_;
    const bool use_persistent_storage_;
    mutable GuardedCache cache_;
    mutable GuardedFutures futures_;
    mutable GuardedQueue queue_;
    mutable GuardedUpdate update_;
    mutable GuardedSocket to_blockchain_api_;
    mutable GuardedSocket to_header_oracle_;
    mutable GuardedSocket publish_;

    static auto get_validator(
        const blockchain::Type chain,
        const node::HeaderOracle& headers) noexcept
        -> std::unique_ptr<const block::Validator>;

    auto bad_block(const block::Hash& id, const BlockLocation& block)
        const noexcept -> void;
    auto block_is_ready(
        const block::Hash& id,
        const BlockLocation& block,
        allocator_type monotonic) const noexcept -> void;
    auto check_blocks(std::span<BlockData> view) const noexcept -> void;
    auto check_block(BlockData& data) const noexcept -> void;
    auto check_header(const blockchain::block::Header& header) const noexcept
        -> void;
    auto check_header(const block::Hash& id, const ReadView header)
        const noexcept -> void;
    auto load_blocks(
        const Hashes& blocks,
        allocator_type alloc,
        allocator_type monotonic) const noexcept -> Vector<BlockLocation>;
    auto publish_queue(QueueData queue) const noexcept -> void;
    auto receive(
        const block::Hash& id,
        const ReadView block,
        allocator_type monotonic) const noexcept -> bool;
    auto save_block(
        const block::Hash& id,
        const ReadView bytes,
        allocator_type monotonic) const noexcept -> BlockLocation;
    auto save_to_cache(const block::Hash& id, const ReadView bytes)
        const noexcept -> CachedBlock;
    auto save_to_database(
        const block::Hash& id,
        const ReadView bytes,
        allocator_type monotonic) const noexcept -> PersistentBlock;
    auto work_available() const noexcept -> void;
};
#pragma GCC diagnostic pop
}  // namespace opentxs::blockchain::node::internal
