// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/wallet/subchain/statemachine/Process.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <chrono>
#include <cstddef>
#include <iterator>
#include <memory>
#include <thread>
#include <utility>

#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "blockchain/node/wallet/subchain/statemachine/BlockIndex.hpp"
#include "blockchain/node/wallet/subchain/statemachine/Progress.hpp"
#include "blockchain/node/wallet/subchain/statemachine/Work.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::blockchain::node::wallet
{
Process::Process(SubchainStateData& parent, Progress& progress) noexcept
    : Job(ThreadPool::Blockchain, parent)
    , progress_(progress)
    , cache_(parent_)
    , waiting_()
    , processing_()
{
}

Process::Cache::Cache(const SubchainStateData& parent) noexcept
    : parent_(parent)
    , limit_(
          4u * params::Data::Chains()
                   .at(parent_.node_.Chain())
                   .block_download_batch_)
    , lock_()
    , batches_()
    , pending_()
    , downloading_()
{
}

auto Process::Cache::FinishBatch(BatchMap::iterator batch) noexcept -> void
{
    auto lock = Lock{lock_};
    batches_.erase(batch);
}

auto Process::Cache::Flush() noexcept -> UnallocatedVector<BatchMap::iterator>
{
    auto output = UnallocatedVector<BatchMap::iterator>{};

    if (auto lock = Lock{lock_, std::defer_lock}; lock.try_lock()) {
        for (auto i{batches_.begin()}, end{batches_.end()}; i != end; ++i) {
            if (i->second->IsFinished()) { output.emplace_back(i); }
        }
    }

    return output;
}

auto Process::Cache::Pop(BlockMap& dest) noexcept -> bool
{
    const auto& log = LogInsane();

    if (auto lock = Lock{lock_, std::defer_lock}; lock.try_lock()) {
        auto hashes = UnallocatedVector<block::pHash>{};
        move_nodes(
            downloading_,
            dest,
            [&](auto i) {
                const auto& [cookie, job] = *i;
                const auto& position = job->position_;
                const auto& [height, hash] = position;

                if (job->IsReady()) {
                    log()(OT_PRETTY_CLASS())(parent_.name_)(
                        " ready to process block ")(hash->asHex())
                        .Flush();

                    return true;
                } else {
                    log()(OT_PRETTY_CLASS())(parent_.name_)(
                        " waiting for block ")(hash->asHex())(" to download")
                        .Flush();

                    return false;
                }
            },
            [&](auto move) {
                const auto downloading = dest.size() + move;

                return downloading >= limit_;
            },
            [&](auto i) {
                const auto& [cookie, job] = *i;
                const auto& position = job->position_;
                const auto& [height, hash] = position;
                hashes.emplace_back(hash);
            });

        if (0u < hashes.size()) { parent_.block_index_.Forget(hashes); }

        download(lock);
        log()(OT_PRETTY_CLASS())(parent_.name_)(" staged block count:      ")(
            pending_.size())
            .Flush();
        log()(OT_PRETTY_CLASS())(parent_.name_)(" downloading block count: ")(
            downloading_.size())
            .Flush();
        log()(OT_PRETTY_CLASS())(parent_.name_)(" downloaded block count:  ")(
            dest.size())
            .Flush();

        return false;
    } else {

        return true;
    }
}

auto Process::Cache::download(const Lock& lock) noexcept -> void
{
    const auto& name = parent_.name_;
    const auto& log = LogTrace();
    const auto start = Clock::now();
    auto positions = UnallocatedVector<block::Position>{};
    auto jobs = UnallocatedVector<Work*>{};

    {
        auto count{downloading_.size()};
        log()(OT_PRETTY_CLASS())(name)(" ")(pending_.size())(
            " positions in pending queue")
            .Flush();

        while ((0 < pending_.size()) && (limit_ > count)) {
            ++count;
            auto& job = *jobs.emplace_back(pending_.front());
            positions.emplace_back(job.position_);
            pending_.pop_front();
        }
    }

    const auto count = positions.size();
    const auto selected = Clock::now();
    log()(OT_PRETTY_CLASS())(name)(" selected ")(
        count)(" positions to be requested in ")(
        std::chrono::nanoseconds{selected - start})
        .Flush();

    if (0u < jobs.size()) { parent_.block_index_.Add(positions); }

    const auto indexed = Clock::now();
    log()(OT_PRETTY_CLASS())(name)(" added ")(
        count)(" positions to block index in ")(
        std::chrono::nanoseconds{indexed - selected})
        .Flush();

    for (auto* job : jobs) { request(lock, job); }

    const auto requested = Clock::now();
    log()(OT_PRETTY_CLASS())(name)(" requested blocks for ")(jobs.size())(
        " jobs in ")(std::chrono::nanoseconds{requested - indexed})
        .Flush();
}

auto Process::Cache::Push(
    UnallocatedVector<std::unique_ptr<Batch>>&& batches,
    UnallocatedVector<Work*>&& jobs) noexcept -> void
{
    const auto& name = parent_.name_;
    const auto& log = LogTrace();
    const auto start = Clock::now();
    auto lock = Lock{lock_};
    const auto locked = Clock::now();
    log()(OT_PRETTY_CLASS())(name)(" lock obtained in ")(
        std::chrono::nanoseconds{locked - start})
        .Flush();

    for (auto& batch : batches) {
        auto id = batch->id_;
        batches_.emplace(std::move(id), std::move(batch));
    }

    const auto queuedB = Clock::now();
    log()(OT_PRETTY_CLASS())(name)(" batches queued in ")(
        std::chrono::nanoseconds{queuedB - locked})
        .Flush();
    std::move(jobs.begin(), jobs.end(), std::back_inserter(pending_));
    const auto queuedJ = Clock::now();
    log()(OT_PRETTY_CLASS())(name)(" jobs queued in ")(
        std::chrono::nanoseconds{queuedJ - queuedB})
        .Flush();
    download(lock);
    const auto downloaded = Clock::now();
    log()(OT_PRETTY_CLASS())(name)(" download queues updated in ")(
        std::chrono::nanoseconds{downloaded - queuedJ})
        .Flush();
}

auto Process::Cache::Reorg(const block::Position& parent) noexcept -> void
{
    auto lock = Lock{lock_};

    for (auto i{pending_.begin()}; i != pending_.end();) {
        const auto* job = *i;
        const auto& position = job->position_;

        if (position > parent) {
            i = pending_.erase(i);
        } else {
            ++i;
        }
    }

    flush(parent, downloading_);
}

auto Process::Cache::request(const Lock& lock, Work* job) noexcept -> void
{
    job->DownloadBlock(parent_.node_.BlockOracle());
    downloading_.try_emplace(job->id_, job);
}

auto Process::Cache::ReRequest(Work* job) noexcept -> void
{
    auto lock = Lock{lock_};
    request(lock, job);
}

auto Process::FinishBatches() noexcept -> bool
{
    auto output{false};

    for (auto i : cache_.Flush()) {
        auto& [id, batch] = *i;
        LogTrace()(OT_PRETTY_CLASS())(parent_.name_)(" batch ")(
            id)(" finished ")
            .Flush();
        static constexpr auto job{"process batch"};
        queue_work([=] { ProcessBatch(i); }, job, false);
        output = true;
    }

    return output;
}

auto Process::flush(const block::Position& parent, BlockMap& map) noexcept
    -> void
{
    for (auto i{map.begin()}; i != map.end();) {
        const auto& position = i->second->position_;

        if (position > parent) {
            i = map.erase(i);
        } else {
            ++i;
        }
    }
}

auto Process::limit(const Lock& lock, const std::size_t outstanding)
    const noexcept -> bool
{
    const auto running = processing_.size() + outstanding;
    const auto limit = [&]() -> std::size_t {
        const auto cores = std::max<std::size_t>(
            std::max(std::thread::hardware_concurrency(), 1u) - 1u, 1u);

        return cores;
    }();

    return running >= limit;
}

auto Process::move_nodes(
    BlockMap& from,
    BlockMap& to,
    std::function<bool(BlockMap::iterator)> moveCondition,
    std::function<bool(std::size_t)> breakCondition,
    std::function<void(BlockMap::iterator)> post) noexcept -> void
{
    OT_ASSERT(moveCondition);
    OT_ASSERT(breakCondition);

    auto move = UnallocatedVector<BlockMap::iterator>{};

    for (auto i{from.begin()}; i != from.end(); ++i) {
        if (breakCondition(move.size())) { break; }

        if (moveCondition(i)) { move.emplace_back(i); }
    }

    move_nodes(move, from, to, post);
}

auto Process::move_nodes(
    UnallocatedVector<BlockMap::iterator>& items,
    BlockMap& from,
    BlockMap& to,
    std::function<void(BlockMap::iterator)> post) noexcept -> void
{
    for (auto& i : items) {
        const auto insert = to.insert(from.extract(i));

        OT_ASSERT(insert.inserted);

        if (post) { post(insert.position); }
    }

    items.clear();
}

auto Process::ProcessBatch(Cache::BatchMap::iterator it) noexcept -> void
{
    auto postcondition = ScopeGuard{[&] {
        cache_.FinishBatch(it);
        auto lock = Lock{lock_};
        finish(lock);
    }};
    auto& [id, pBatch] = *it;
    auto& batch = *pBatch;
    batch.Write(parent_.db_key_, parent_.db_);
    batch.UpdateProgress(progress_, parent_.get_index());
}

auto Process::ProcessPosition(const Cookie key, Work* work) noexcept -> void
{
    auto postcondition = ScopeGuard{[&] {
        auto lock = Lock{lock_};
        processing_.erase(key);
        finish(lock);
    }};
    const auto validBlock = work->Do(parent_);

    if (false == validBlock) { cache_.ReRequest(work); }
}

auto Process::Reorg(const block::Position& parent) noexcept -> void
{
    cache_.Reorg(parent);
    flush(parent, waiting_);
    flush(parent, processing_);
}

auto Process::Request(
    const std::optional<block::Position>& highestClean,
    const UnallocatedVector<block::Position>& blocks,
    UnallocatedVector<std::unique_ptr<Batch>>&& batches,
    UnallocatedVector<Work*>&& jobs) noexcept -> void
{
    const auto& name = parent_.name_;
    const auto& log = LogTrace();
    const auto start = Clock::now();
    progress_.UpdateScan(highestClean, blocks);
    const auto progress = Clock::now();
    log()(OT_PRETTY_CLASS())(name)(" progress updated in ")(
        std::chrono::nanoseconds{progress - start})
        .Flush();
    cache_.Push(std::move(batches), std::move(jobs));
    const auto done = Clock::now();
    log()(OT_PRETTY_CLASS())(name)(" batches cached in ")(
        std::chrono::nanoseconds{done - progress})
        .Flush();
}

auto Process::Run() noexcept -> bool
{
    const auto& name = parent_.name_;
    const auto& log = LogTrace();
    FinishBatches();
    auto lock = Lock{lock_};
    auto again = cache_.Pop(waiting_);
    const auto waiting = waiting_.size();

    if (0 < waiting) {
        log()(OT_PRETTY_CLASS())(name)(" ")(
            waiting)(" downloaded blocks ready for processing")
            .Flush();
    }

    move_nodes(
        waiting_,
        processing_,
        [](auto i) { return true; },
        [&](auto move) { return limit(lock, move); },
        [this](auto i) {
            const auto& key = i->first;
            auto* work = i->second;
            static constexpr auto job{"process position"};
            queue_work([=] { ProcessPosition(key, work); }, job, true);
        });
    const auto processing{waiting - waiting_.size()};

    if (0 < waiting) {
        log()(OT_PRETTY_CLASS())(name)(" ")(
            processing)(" blocks added to processing queue. ")(
            processing_.size())(" block total in queue.")
            .Flush();
    }

    return again;
}
}  // namespace opentxs::blockchain::node::wallet
