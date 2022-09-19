// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/blockoracle/blockfetcher/Shared.hpp"  // IWYU pragma: associated

#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

#include "internal/blockchain/database/Block.hpp"
#include "internal/blockchain/node/Job.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/bitcoin/block/Block.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::node::blockoracle
{
BlockFetcher::Shared::Data::Data(
    const api::Session& api,
    database::Block& db,
    blockchain::Type chain,
    allocator_type alloc) noexcept
    : api_(api)
    , db_(db)
    , log_(LogTrace())
    , chain_(chain)
    , blocks_(alloc)
    , job_index_(alloc)
    , unassigned_()
    , tip_()
{
}

auto BlockFetcher::Shared::Data::get_allocator() const noexcept
    -> allocator_type
{
    return job_index_.get_allocator();
}

auto BlockFetcher::Shared::Data::AddBlocks(NewBlocks&& blocks) noexcept -> void
{
    if (blocks.empty()) { return; }

    const auto before{unassigned_};
    const auto& first = blocks.front().first.height_;

    if (auto expect = LastBlock() + 1; first != expect) {
        LogAbort()(OT_PRETTY_CLASS())("expected new blocks to begin at ")(
            expect)(" but they actually start at ")(first)
            .Abort();
    }

    auto height{first};

    for (auto& [pos, status] : blocks) {
        if (pos.height_ != height++) {
            LogAbort()(OT_PRETTY_CLASS())(
                "non-contiguous sequence. Expected height ")(height - 1)(
                " but found ")(pos.height_)
                .Abort();
        }

        if (Status::pending == status) {
            log_(OT_PRETTY_CLASS())("adding block ")(pos)(" to queue").Flush();
            ++unassigned_;

        } else {
            log_(OT_PRETTY_CLASS())("block ")(pos)(" already downloaded")
                .Flush();
        }

        blocks_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(pos.height_),
            std::forward_as_tuple(
                std::move(pos.hash_), std::nullopt, std::move(status)));
    }

    if (unassigned_ > blocks_.size()) {
        LogAbort()(OT_PRETTY_CLASS())("unassigned block count ")(
            unassigned_)(" exceeds total number of blocks in queue ")(
            blocks_.size())
            .Abort();
    }

    log_(OT_PRETTY_CLASS())("added ")(blocks.size())(
        " blocks to queue. Unassigned block count increased from ")(
        before)(" to ")(unassigned_)
        .Flush();
}

auto BlockFetcher::Shared::Data::DownloadBlock(
    const block::Hash& block,
    download::JobID id) noexcept -> void
{
    remove_block_from_job(block, id, Status::success);
}

auto BlockFetcher::Shared::Data::FinishJob(download::JobID id) noexcept -> void
{
    const auto before = unassigned_;

    if (auto i = job_index_.find(id); job_index_.end() != i) {
        auto& blocks = i->second;

        for (auto& [block, j] : blocks) {
            auto& [height, data] = *j;
            auto& [hash, job, status] = data;
            job = std::nullopt;

            if (Status::success == status) {
                log_(OT_PRETTY_CLASS())("block ")
                    .asHex(hash)(" is downloaded")
                    .Flush();
            } else {
                log_(OT_PRETTY_CLASS())("block ")
                    .asHex(hash)(" was not downloaded. Returning to queue.")
                    .Flush();
                status = Status::pending;
                ++unassigned_;
            }
        }

        job_index_.erase(i);
        log_(OT_PRETTY_CLASS())("job ")(id)(" is finished").Flush();

        if (unassigned_ > blocks_.size()) {
            LogAbort()(OT_PRETTY_CLASS())("unassigned block count ")(
                unassigned_)(" exceeds total number of blocks in queue ")(
                blocks_.size())
                .Abort();
        }

        if (before != unassigned_) {
            log_(OT_PRETTY_CLASS())("unassigned block count incremented from ")(
                before)(" to ")(unassigned_)(" due to undownloaded blocks")
                .Flush();
        }
    } else {
        LogAbort()(OT_PRETTY_CLASS())("job ")(id)(" does not exist").Abort();
    }
}

auto BlockFetcher::Shared::Data::GetBatch(
    std::size_t peerTarget,
    allocator_type alloc) noexcept -> Batch
{
    const auto before{unassigned_};

    if (before > blocks_.size()) {
        LogAbort()(OT_PRETTY_CLASS())("unassigned block count ")(
            before)(" exceeds total number of blocks in queue ")(blocks_.size())
            .Abort();
    }

    // TODO define max in Params
    static constexpr auto max = 500_uz;
    static constexpr auto min = 10_uz;
    const auto count = download::batch_size(before, peerTarget, max, min);

    if (before < count) {
        LogAbort()(OT_PRETTY_CLASS())("calculated a batch size of ")(
            count)(" but only ")(before)(" are unassigned")
            .Abort();
    }

    if (0_uz == count) {
        log_(OT_PRETTY_CLASS())("can not create download batch from ")(
            unassigned_)(" of ")(blocks_.size())(" unassigned blocks")
            .Flush();

        return Batch{-1, alloc};
    }

    auto out = Batch{download::next_job(), alloc};
    const auto& id = out.first;
    auto& hashes = out.second;
    hashes.reserve(count);
    auto& index = job_index_[id];
    auto i = blocks_.begin();

    while (hashes.size() < count) {
        auto cur = i++;

        OT_ASSERT(blocks_.end() != cur);

        auto& [height, val] = *cur;
        auto& [hash, job, status] = val;

        if (Status::pending == status) {
            OT_ASSERT(0_uz < unassigned_);

            --unassigned_;
            job = id;
            status = Status::downloading;
            hashes.emplace_back(hash);
            index.emplace(hash, cur);
        }
    }

    log_(OT_PRETTY_CLASS())("assigned job ")(id)(" by allocating ")(
        hashes.size())(" of ")(blocks_.size())(" blocks from queue")
        .Flush();
    log_(OT_PRETTY_CLASS())("unassigned block count reduced from ")(
        before)(" to ")(unassigned_)
        .Flush();

    return out;
}

auto BlockFetcher::Shared::Data::JobAvailable() const noexcept -> bool
{
    return 0_uz < unassigned_;
}

auto BlockFetcher::Shared::Data::LastBlock() const noexcept -> block::Height
{
    if (blocks_.empty()) {

        return tip_.height_;
    } else {

        return blocks_.crbegin()->first;
    }
}

auto BlockFetcher::Shared::Data::PruneStale(
    const block::Position& lastGood) noexcept -> void
{
    log_(OT_PRETTY_CLASS())("erasing blocks after last good position ")(
        lastGood)
        .Flush();

    if (blocks_.empty()) {
        log_(OT_PRETTY_CLASS())("no blocks in queue").Flush();

        return;
    }

    for (auto i = blocks_.lower_bound(lastGood.height_), stop = blocks_.end();
         i != stop;) {
        auto& [height, val] = *i;
        auto& [hash, job, status] = val;

        if ((height == lastGood.height_) && (hash == lastGood.hash_)) {
            log_(OT_PRETTY_CLASS())("last good position is present in queue")
                .Flush();

            ++i;
        } else {
            log_(OT_PRETTY_CLASS())("queued block at height ")(
                height)(" is stale")
                .Flush();

            if (job.has_value()) {
                log_(OT_PRETTY_CLASS())("queued block at height ")(
                    height)(" is assigned to a download job")
                    .Flush();
                remove_block_from_job(hash, *job, Status::stale);
            } else {
                log_(OT_PRETTY_CLASS())("queued block at height ")(
                    height)(" is queued but not yet assigned")
                    .Flush();

                OT_ASSERT(0_uz < unassigned_);

                --unassigned_;
                log_(OT_PRETTY_CLASS())(
                    "unassigned block count decremented to ")(unassigned_)
                    .Flush();
            }

            i = blocks_.erase(i);
        }
    }
}

auto BlockFetcher::Shared::Data::ReceiveBlock(
    download::JobID job,
    std::string_view bytes) noexcept -> void
{
    auto pBlock = api_.Factory().BitcoinBlock(chain_, bytes);

    if (pBlock) {
        if (false == db_.BlockStore(*pBlock)) {
            LogAbort()(OT_PRETTY_CLASS())("database error").Abort();
        }
    } else {
        log_(OT_PRETTY_CLASS())("received invalid block").Flush();

        return;
    }

    const auto& block = *pBlock;
    const auto& hash = block.ID();
    DownloadBlock(hash, job);
}

auto BlockFetcher::Shared::Data::remove_block_from_job(
    const block::Hash& block,
    download::JobID job,
    Status update) noexcept -> void
{
    if (auto i = job_index_.find(job); i != job_index_.end()) {
        auto& batch = i->second;

        if (auto j = batch.find(block); batch.end() != j) {
            {
                const auto& [key, value] = *j;
                auto& [h, job, status] = value->second;
                status = update;
                job = std::nullopt;
            }

            batch.erase(j);
            log_(OT_PRETTY_CLASS())("removed block ")
                .asHex(block)(" from job ")(job)
                .Flush();
        } else {
            log_(OT_PRETTY_CLASS())("block ")
                .asHex(block)(" was not assigned to ")(job)
                .Flush();
        }
    } else {
        log_(OT_PRETTY_CLASS())("job ")(job)(" already finished").Flush();
    }
}

auto BlockFetcher::Shared::Data::ReviseTip(
    const block::Position& newTip) noexcept -> bool
{
    log_(OT_PRETTY_CLASS())("previous tip: ")(tip_).Flush();
    log_(OT_PRETTY_CLASS())(" updated tip: ")(newTip).Flush();

    if (newTip == tip_) {

        return false;
    } else {
        tip_ = newTip;

        return true;
    }
}

auto BlockFetcher::Shared::Data::UpdateTip() noexcept
    -> std::optional<block::Position>
{
    auto newTip = std::optional<block::Position>{std::nullopt};
    auto expected = tip_.height_;
    auto erase = [&] {
        for (auto i = blocks_.begin(), end = blocks_.end(); i != end; ++i) {
            const auto& [height, val] = *i;
            const auto& [hash, job, status] = val;

            if (++expected != height) {
                LogAbort()(OT_PRETTY_CLASS())(
                    "non-contiguous sequence. Expected height ")(expected - 1)(
                    " but found ")(height)
                    .Abort();
            }

            auto canErase{false};

            if (job.has_value()) {
                if (db_.BlockExists(hash)) {
                    remove_block_from_job(hash, *job, Status::success);
                    canErase = true;
                }
            } else if (Status::success == status) {
                canErase = true;
            }

            if (canErase) {
                newTip.emplace(height, hash);
                log_(OT_PRETTY_CLASS())("block ") (*newTip)(
                    " successfully downloaded and can be erased from queue")
                    .Flush();
            } else {
                log_(OT_PRETTY_CLASS())("block ")
                    .asHex(hash)(" at height ")(
                        height)(" still downloading and must remain in queue")
                    .Flush();

                return i;
            }
        }

        return blocks_.end();
    }();
    blocks_.erase(blocks_.begin(), erase);

    if (newTip.has_value()) { tip_ = *newTip; }

    return newTip;
}
}  // namespace opentxs::blockchain::node::blockoracle
