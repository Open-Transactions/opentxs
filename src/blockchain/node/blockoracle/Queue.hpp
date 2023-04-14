// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <functional>
#include <string_view>
#include <utility>

#include "internal/blockchain/node/Job.hpp"
#include "internal/blockchain/node/blockoracle/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::blockoracle
{
class Queue final : public opentxs::Allocated
{
public:
    auto Available() const noexcept { return queue_.size(); }
    auto get_allocator() const noexcept -> allocator_type final;
    auto Items() const noexcept -> QueueData;
    auto Waiting() const noexcept { return pending_.size(); }

    auto Add(Hashes blocks) noexcept -> QueueData;
    auto Finish(JobID job) noexcept -> QueueData;
    auto GetWork(allocator_type alloc) noexcept -> Work;
    auto Receive(const block::Hash& block) noexcept -> QueueData;

    Queue(
        const Log& log,
        std::string_view name,
        std::size_t peerTarget,
        allocator_type alloc) noexcept;
    Queue() = delete;
    Queue(const Queue&) = delete;
    Queue(Queue&&) = delete;
    auto operator=(const Queue&) -> Queue& = delete;
    auto operator=(Queue&&) -> Queue& = delete;

    ~Queue() final;

private:
    using Blocks = Deque<block::Hash>;
    using Index = Set<block::Hash>;
    using Jobs = Map<JobID, std::pair<const std::size_t, Index>>;
    using BlockToJob = Map<block::Hash, JobID>;

    const Log& log_;
    const std::string_view name_;
    const std::size_t peer_target_;
    Blocks queue_;
    Index pending_;
    Jobs jobs_;
    BlockToJob block_to_job_;

    auto is_queued(const block::Hash& hash) const noexcept -> bool;

    auto queue_hash(const block::Hash& hash) noexcept -> void;
    auto remove_from_job(const block::Hash& hash) noexcept -> void;
    auto remove_from_queue(const block::Hash& hash) noexcept -> void;
};
}  // namespace opentxs::blockchain::node::blockoracle
