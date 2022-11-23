// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "blockchain/node/blockoracle/Queue.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <span>
#include <tuple>

#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::blockchain::node::blockoracle
{
Queue::Queue(
    const Log& log,
    std::string_view name,
    std::size_t peerTarget,
    allocator_type alloc) noexcept
    : log_(log)
    , name_(name)
    , peer_target_(peerTarget)
    , queue_(alloc)
    , pending_(alloc)
    , jobs_(alloc)
    , block_to_job_(alloc)
{
}

auto Queue::Add(Hashes blocks) noexcept -> QueueData
{
    for (const auto& hash : blocks) {
        if (false == is_queued(hash)) { queue_hash(hash); }
    }

    return Items();
}

auto Queue::Finish(JobID job) noexcept -> QueueData
{
    const auto& log = log_;

    if (auto i = jobs_.find(job); jobs_.end() != i) {
        auto post = ScopeGuard{[&] { jobs_.erase(i); }};
        const auto& [_, remaining] = i->second;
        log(OT_PRETTY_CLASS())(name_)(": job ")(job)(" finished with ")(
            remaining.size())(" blocks still outstanding")
            .Flush();

        for (const auto& hash : remaining) {
            block_to_job_.erase(hash);
            queue_.emplace_front(hash);
        }
    }

    return Items();
}

auto Queue::get_allocator() const noexcept -> allocator_type
{
    return queue_.get_allocator();
}

auto Queue::GetWork(allocator_type alloc) noexcept -> Work
{
    using namespace download;

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

    // TODO define max in Params
    constexpr auto max = 100_uz;
    constexpr auto min = 10_uz;
    const auto available = queue_.size();
    const auto target = batch_size(available, peer_target_, max, min);

    if (0_uz == target) {
        return std::make_tuple(
            invalidJob, Vector<block::Hash>{alloc}, Available(), Waiting());
    }

    auto out = std::make_tuple(
        next_job(), Vector<block::Hash>{alloc}, Available(), Waiting());
    auto& [jobID, hashes, jobs, downloading] = out;
    hashes.reserve(target);
    // TODO c++20
    auto& [count, index] = [&, this ](const auto& id) -> auto&
    {
        auto [i, rc] = jobs_.try_emplace(id, target, Index{get_allocator()});

        OT_ASSERT(rc);

        return i->second;
    }
    (jobID);

    while (hashes.size() < target) {
        const auto& hash = queue_.front();
        hashes.emplace_back(hash);
        index.emplace(hash);
        auto [_, rc] = block_to_job_.try_emplace(hash, jobID);

        OT_ASSERT(rc);

        queue_.pop_front();
    }

    jobs = queue_.size();

    return out;
}

auto Queue::is_queued(const block::Hash& hash) const noexcept -> bool
{
    return pending_.contains(hash);
}

auto Queue::Items() const noexcept -> QueueData
{
    return std::make_pair(Available(), Waiting());
}

auto Queue::queue_hash(const block::Hash& hash) noexcept -> void
{
    queue_.emplace_back(hash);
    pending_.emplace(hash);
}

auto Queue::Receive(const block::Hash& block) noexcept -> QueueData
{
    remove_from_job(block);
    remove_from_queue(block);

    return Items();
}

auto Queue::remove_from_job(const block::Hash& hash) noexcept -> void
{
    auto& index = block_to_job_;

    if (auto i = index.find(hash); index.end() != i) {
        auto post = ScopeGuard{[&] { index.erase(i); }};
        auto count = jobs_.at(i->second).second.erase(hash);

        OT_ASSERT(1_uz == count);

        count = pending_.erase(hash);

        OT_ASSERT(1_uz == count);
    }
}

auto Queue::remove_from_queue(const block::Hash& hash) noexcept -> void
{
    if (is_queued(hash)) {
        pending_.erase(hash);
        queue_.erase(
            std::remove(queue_.begin(), queue_.end(), hash), queue_.end());
    }
}

Queue::~Queue() = default;
}  // namespace opentxs::blockchain::node::blockoracle
