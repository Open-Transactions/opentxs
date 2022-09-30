// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <cs_plain_guarded.h>
#include <cstddef>
#include <optional>
#include <string_view>
#include <tuple>
#include <utility>

#include "internal/blockchain/node/Job.hpp"
#include "internal/blockchain/node/blockoracle/BlockFetcher.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
class Hash;
class Position;
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
namespace socket
{
class Raw;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::blockoracle
{
class BlockFetcher::Shared final : public Allocated
{
public:
    enum class Status { pending, downloading, success, stale };
    using Index =
        Map<block::Height,
            std::tuple<block::Hash, std::optional<download::JobID>, Status>>;

    class Data : public Allocated
    {
    public:
        const api::Session& api_;
        database::Block& db_;
        const Log& log_;
        const blockchain::Type chain_;

        using Batch = std::pair<download::JobID, Vector<block::Hash>>;
        using NewBlocks = Vector<std::pair<block::Position, Status>>;

        auto get_allocator() const noexcept -> allocator_type final;
        auto JobAvailable() const noexcept -> bool;
        auto LastBlock() const noexcept -> block::Height;
        auto Tip() const noexcept -> block::Position { return tip_; }

        auto AddBlocks(NewBlocks&& blocks) noexcept -> void;
        auto DownloadBlock(
            const block::Hash& block,
            download::JobID job) noexcept -> void;
        auto FinishJob(download::JobID id) noexcept -> void;
        auto GetBatch(std::size_t peerTarget, allocator_type alloc) noexcept
            -> Batch;
        auto PruneStale(const block::Position& lastGood) noexcept -> void;
        auto ReceiveBlock(download::JobID job, std::string_view block) noexcept
            -> void;
        auto ReviseTip(const block::Position& newTip) noexcept -> bool;
        auto UpdateTip() noexcept -> std::optional<block::Position>;

        Data(
            const api::Session& api,
            database::Block& db,
            blockchain::Type chain,
            allocator_type alloc) noexcept;
        Data() = delete;
        Data(const Data&) = delete;
        Data(Data&&) = delete;
        auto operator=(const Data&) -> Data& = delete;
        auto operator=(Data&&) -> Data& = delete;

    private:
        Index blocks_;
        Map<download::JobID, Map<block::Hash, Index::iterator>> job_index_;
        std::size_t unassigned_;
        block::Position tip_;

        auto remove_block_from_job(
            const block::Hash& block,
            download::JobID job,
            Status update) noexcept -> void;
    };

    using Guarded = libguarded::plain_guarded<Data>;
    using Socket = libguarded::plain_guarded<network::zeromq::socket::Raw>;

    const network::zeromq::BatchID batch_id_;
    const std::size_t peer_target_;
    mutable Guarded data_;
    mutable Socket to_actor_;

    auto GetJob(boost::shared_ptr<Shared> self, allocator_type alloc)
        const noexcept -> internal::BlockBatch;
    auto get_allocator() const noexcept -> allocator_type final;

    Shared(
        const api::Session& api,
        const node::Manager& node,
        network::zeromq::BatchID batchID,
        allocator_type alloc) noexcept;

    ~Shared() final;
};
}  // namespace opentxs::blockchain::node::blockoracle
