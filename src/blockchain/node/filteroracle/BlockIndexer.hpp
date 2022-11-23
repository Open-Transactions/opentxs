// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type
// IWYU pragma: no_forward_declare opentxs::blockchain::node::filteroracle::BlockIndexerJob
// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <cs_shared_guarded.h>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <shared_mutex>
#include <span>
#include <string_view>

#include "internal/blockchain/node/filteroracle/BlockIndexer.hpp"
#include "internal/blockchain/node/filteroracle/Types.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Header.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "util/Actor.hpp"
#include "util/JobCounter.hpp"

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
class Position;
}  // namespace block

namespace node
{
namespace filteroracle
{
class Shared;
}  // namespace filteroracle

class Manager;
}  // namespace node
}  // namespace blockchain

namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::filteroracle
{
class BlockIndexer::Imp final : public Actor<Imp, BlockIndexerJob>
{
public:
    auto Init(boost::shared_ptr<Imp> me) noexcept -> void;

    Imp(std::shared_ptr<const api::Session> api,
        std::shared_ptr<const node::Manager> node,
        std::shared_ptr<Shared> shared,
        const network::zeromq::BatchID batch,
        allocator_type alloc) noexcept;
    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;

    ~Imp() final;

private:
    friend Actor<Imp, BlockIndexerJob>;

    struct BlockQueue {
        using Pending = Set<block::Position>;
        using Index = Map<block::Hash, block::Position>;
        using Ready =
            Map<block::Position, std::shared_ptr<bitcoin::block::Block>>;

        Pending requested_;
        Index index_;
        Ready ready_;

        auto Reorg(const block::Position& parent) noexcept -> void;

        BlockQueue(allocator_type alloc) noexcept;
        BlockQueue() = delete;
        BlockQueue(const BlockQueue&) = delete;
        BlockQueue(BlockQueue&&) = delete;
        auto operator=(const BlockQueue&) -> BlockQueue& = delete;
        auto operator=(BlockQueue&&) -> BlockQueue& = delete;
    };
    struct WorkQueue {
        block::Position position_;
        cfilter::Header cfheader_;
        block::Position previous_position_;
        cfilter::Header previous_cfheader_;

        auto Reorg(const Shared& shared, const block::Position& parent) noexcept
            -> void;
        auto Reset(const Shared& shared, block::Position tip) noexcept -> void;

        WorkQueue() noexcept;
        WorkQueue(const WorkQueue&) = delete;
        WorkQueue(WorkQueue&&) = delete;
        auto operator=(const WorkQueue&) -> WorkQueue& = delete;
        auto operator=(WorkQueue&&) -> WorkQueue& = delete;
    };
    using GuardedBlocks =
        libguarded::shared_guarded<BlockQueue, std::shared_mutex>;
    using GuardedWork =
        libguarded::shared_guarded<WorkQueue, std::shared_mutex>;
    using GuardedPosition =
        libguarded::shared_guarded<block::Position, std::shared_mutex>;

    std::shared_ptr<const api::Session> api_p_;
    std::shared_ptr<const node::Manager> node_p_;
    std::shared_ptr<Shared> shared_p_;
    const api::Session& api_;
    const node::Manager& node_;
    const blockchain::Type chain_;
    const std::filesystem::path checkpoints_;
    Shared& shared_;
    block::Position best_position_;
    Deque<block::Position> queue_;
    GuardedBlocks blocks_;
    GuardedWork work_;
    GuardedPosition tip_;
    JobCounter counter_;
    Outstanding running_;

    auto background() noexcept -> void;
    auto do_shutdown() noexcept -> void;
    auto do_startup(allocator_type monotonic) noexcept -> bool;
    auto drain_queue(allocator_type monotonic) noexcept -> std::size_t;
    auto fill_queue() noexcept -> void;
    auto pipeline(const Work work, Message&& msg, allocator_type) noexcept
        -> void;
    auto process_block(Message&& in) noexcept -> void;
    auto process_block(block::Position&& position) noexcept -> void;
    auto process_block_ready(Message&& in) noexcept -> void;
    auto process_reindex(Message&& in) noexcept -> void;
    auto process_reorg(Message&& in) noexcept -> void;
    auto process_reorg(block::Position&& parent) noexcept -> void;
    auto process_report(Message&& in) noexcept -> void;
    auto update_best_position(block::Position&& position) noexcept -> void;
    auto update_checkpoint() noexcept -> void;
    auto work(allocator_type monotonic) noexcept -> bool;
    auto write_checkpoint(block::Height target) noexcept -> void;
    auto write_last_checkpoint(const block::Position& tip) noexcept -> void;
};
}  // namespace opentxs::blockchain::node::filteroracle
