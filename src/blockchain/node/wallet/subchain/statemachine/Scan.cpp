// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/wallet/subchain/statemachine/Scan.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <atomic>
#include <chrono>
#include <limits>
#include <memory>
#include <mutex>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "blockchain/node/wallet/subchain/statemachine/Batch.hpp"
#include "blockchain/node/wallet/subchain/statemachine/Process.hpp"
#include "blockchain/node/wallet/subchain/statemachine/Rescan.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/GCS.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::blockchain::node::wallet
{
Scan::Scan(SubchainStateData& parent, Process& process, Rescan& rescan) noexcept
    : Job(ThreadPool::General, parent)
    , last_scanned_(parent_.db_.SubchainLastScanned(parent_.db_key_))
    , filter_tip_(std::nullopt)
    , ready_(false)
    , process_(process)
    , rescan_(rescan)
{
}

auto Scan::Do(
    const block::Position best,
    const block::Height stop,
    block::Position highestTested) noexcept -> void
{
    const auto& api = parent_.api_;
    const auto& name = parent_.name_;
    const auto& node = parent_.node_;
    const auto& type = parent_.filter_type_;
    const auto& headers = node.HeaderOracle();
    const auto& filters = node.FilterOracleInternal();
    const auto start = Clock::now();
    const auto startHeight = highestTested.first + 1;
    const auto stopHeight =
        std::min(std::min(startHeight + 999, best.first), stop);
    auto atLeastOnce{false};
    auto highestClean = std::optional<block::Position>{std::nullopt};
    auto batches = UnallocatedVector<std::unique_ptr<Batch>>{};
    auto jobs = UnallocatedVector<Work*>{};
    auto blocks = UnallocatedVector<block::Position>{};
    auto postcondition = ScopeGuard{[&] {
        const auto& log = LogTrace();
        const auto start = Clock::now();
        log(OT_PRETTY_CLASS())(name)(" ")(this->type())(" finalizing ")(
            batches.size())(" batches")
            .Flush();

        for (auto& batch : batches) { batch->Finalize(); }

        const auto finalized = Clock::now();
        auto elapsed = std::chrono::nanoseconds{finalized - start};
        log(OT_PRETTY_CLASS())(name)(" ")(this->type())(" finalized in ")(
            elapsed)
            .Flush();

        auto lock = Lock{lock_, std::defer_lock};

        if (atLeastOnce) {
            process_.Request(
                highestClean, blocks, std::move(batches), std::move(jobs));
            const auto requested = Clock::now();
            elapsed = std::chrono::nanoseconds{requested - finalized};
            log(OT_PRETTY_CLASS())(name)(" ")(this->type())(
                " blocks requested in ")(elapsed)
                .Flush();
            lock.lock();
            last_scanned_ = highestTested;
        } else {
            log(OT_PRETTY_CLASS())(name)(" ")(this->type())(
                " job had no work to do. Start height: ")(
                startHeight)(", stop height: ")(stopHeight)
                .Flush();
            lock.lock();
        }

        finish(lock);
    }};

    if (startHeight > stopHeight) { return; }

    const auto& log = LogVerbose();
    log(OT_PRETTY_CLASS())(name)(" ")(this->type())("ning filters from ")(
        startHeight)(" to ")(stopHeight)
        .Flush();
    const auto [elements, utxos, patterns] = parent_.get_account_targets();
    auto blockHash = api.Factory().Data();

    for (auto i{startHeight}; i <= stopHeight; ++i) {
        if (shutdown_) { return; }

        blockHash = headers.BestHash(i, best);

        if (blockHash->empty()) {
            log(OT_PRETTY_CLASS())(name)(" interrupting ")(this->type())(
                " due to chain reorg")
                .Flush();

            break;
        }

        auto testPosition = block::Position{i, blockHash};
        const auto pFilter = filters.LoadFilterOrResetTip(type, testPosition);

        if (false == bool(pFilter)) {
            log(OT_PRETTY_CLASS())(name)(" filter at height ")(i)(" not found ")
                .Flush();

            break;
        }

        atLeastOnce = true;
        const auto& filter = *pFilter;
        auto matches = filter.Match(patterns);
        auto isClean{true};

        if (0 < matches.size()) {
            const auto [untested, retest] =
                parent_.get_block_targets(blockHash, utxos);
            matches = filter.Match(retest);

            if (0 < matches.size()) {
                log(OT_PRETTY_CLASS())(name)(" GCS ")(this->type())(
                    " for block ")(blockHash->asHex())(" at height ")(
                    i)(" found ")(matches.size())(
                    " new potential matches for the ")(patterns.size())(
                    " target elements for ")(parent_.id_)
                    .Flush();
                isClean = false;
            }
        }

        if (isClean) {
            if (0u == jobs.size()) { highestClean = testPosition; }
        } else {
            if ((0u == batches.size()) || (batches.back()->IsFull())) {
                batches.emplace_back(std::make_unique<Batch>());
            }

            auto& batch = batches.back();
            blocks.emplace_back(testPosition);
            jobs.emplace_back(batch->AddJob(testPosition));
        }

        highestTested = std::move(testPosition);
    }

    if (atLeastOnce) {
        const auto count = jobs.size();
        log(OT_PRETTY_CLASS())(name)(" ")(this->type())(" found ")(
            count)(" new potential matches between blocks ")(
            startHeight)(" and ")(highestTested.first)(" in ")(
            std::chrono::nanoseconds{Clock::now() - start})
            .Flush();
    } else {
        log(OT_PRETTY_CLASS())(name)(" ")(this->type())(
            " interrupted due to missing filter")
            .Flush();
    }
}

auto Scan::finish(Lock& lock) noexcept -> void
{
    if (last_scanned_.has_value()) {
        rescan_.SetCeiling(last_scanned_.value());
    }

    Job::finish(lock);
}

auto Scan::Position() const noexcept -> block::Position
{
    auto lock = Lock{lock_};

    return last_scanned_.value_or(parent_.null_position_);
}

auto Scan::Reorg(const block::Position& parent) noexcept -> void
{
    if (last_scanned_.has_value() && (last_scanned_.value() > parent)) {
        last_scanned_ = parent;
    }

    if (filter_tip_.has_value() && (filter_tip_.value() > parent)) {
        filter_tip_ = parent;
    }
}

auto Scan::Run() noexcept -> bool { return Run(update_tip()); }

auto Scan::Run(const block::Position& filterTip) noexcept -> bool
{
    const auto& name = parent_.name_;
    const auto& null = parent_.null_position_;
    auto needScan{false};

    try {
        const auto current = [&] {
            auto lock = Lock{lock_};
            filter_tip_ = filterTip;

            if (is_running(lock)) {
                throw std::runtime_error{"job is already running"};
            }

            if (false == ready_) {
                throw std::runtime_error{"job is disabled"};
            }

            return last_scanned_;
        }();
        // NOTE below this line we can assume only one Scan thread is running
        if (current.has_value()) {
            if (current == filterTip) {
                LogVerbose()(OT_PRETTY_CLASS())(name)(" has been ")(type())(
                    "ned to the newest downloaded cfilter ")(
                    filterTip.second->asHex())(" at height ")(filterTip.first)
                    .Flush();
                needScan = false;
            } else if (current > filterTip) {
                {
                    auto lock = Lock{lock_};
                    last_scanned_ = parent_.node_.HeaderOracle().GetPosition(
                        std::max<block::Height>(filterTip.first, 0) - 1);
                }
                needScan = true;
            } else {
                needScan = true;
            }
        } else {
            needScan = true;
        }

        if (needScan) {
            static constexpr auto stop =
                std::numeric_limits<block::Height>::max();
            auto highestTested = current.value_or(null);
            needScan = queue_work(
                [=,
                 last = std::move(highestTested),
                 best = std::move(filterTip)] {
                    Do(std::move(best), stop, std::move(last));
                },
                type(),
                false);
        } else {
            process_.Request(filterTip, {}, {}, {});
        }
    } catch (...) {
        needScan = true;
    }

    return needScan;
}

auto Scan::SetReady() noexcept -> void
{
    auto lock = Lock{lock_};
    set_ready(lock);
}

auto Scan::set_ready(const Lock& lock) noexcept -> void { ready_ = true; }

auto Scan::UpdateTip(const block::Position& tip) noexcept -> void
{
    auto lock = Lock{lock_};
    filter_tip_ = tip;
}

auto Scan::update_tip() noexcept -> block::Position
{
    auto lock = Lock{lock_};

    return update_tip(lock);
}

auto Scan::update_tip(const Lock& lock) noexcept -> block::Position
{
    if (false == filter_tip_.has_value()) {
        const auto& node = parent_.node_;
        const auto& filters = node.FilterOracleInternal();
        filter_tip_ = filters.FilterTip(parent_.filter_type_);
    }

    OT_ASSERT(filter_tip_.has_value());

    return filter_tip_.value();
}
}  // namespace opentxs::blockchain::node::wallet
