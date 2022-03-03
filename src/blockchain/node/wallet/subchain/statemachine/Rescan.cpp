// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/wallet/subchain/statemachine/Rescan.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <chrono>
#include <limits>
#include <mutex>
#include <optional>
#include <stdexcept>
#include <utility>

#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "blockchain/node/wallet/subchain/statemachine/Job.hpp"
#include "blockchain/node/wallet/subchain/statemachine/Progress.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"

namespace opentxs::blockchain::node::wallet
{
Rescan::Rescan(
    SubchainStateData& parent,
    Process& process,
    Progress& progress) noexcept
    : Scan(parent, process, *this)
    , progress_(progress)
    , rescan_requests_()
    , ceiling_(parent_.null_position_)
{
}

auto Rescan::finish(Lock& lock) noexcept -> void
{
    if (last_scanned_.has_value() && filter_tip_.has_value()) {
        if (last_scanned_.value() == filter_tip_.value()) {
            last_scanned_ = std::nullopt;
        }
    }

    Job::finish(lock);
}

auto Rescan::flush(const Lock& lock) noexcept -> void
{
    const auto have = 0 < rescan_requests_.size();

    while (0 < rescan_requests_.size()) {
        {
            auto& position = rescan_requests_.front();
            process(lock, position);
        }

        rescan_requests_.pop();
    }

    if (have) {
        OT_ASSERT(last_scanned_.has_value());

        LogVerbose()(OT_PRETTY_CLASS())(parent_.name_)(
            " rescanning from block ")(last_scanned_.value().first)
            .Flush();
    }
}

auto Rescan::process(const Lock& lock, const block::Position& position) noexcept
    -> void
{
    LogVerbose()(OT_PRETTY_CLASS())(parent_.name_)(" incoming position: ")(
        position.second->asHex())(" at height ")(position.first)
        .Flush();
    auto& rescan = last_scanned_;

    if (rescan.has_value() && (rescan.value() != parent_.null_position_)) {
        const auto current = rescan.value();
        LogVerbose()(OT_PRETTY_CLASS())(" ")(parent_.name_)(
            " current position: ")(current.second->asHex())(" at height ")(
            current.first)
            .Flush();

        if (current > position) { rescan = position; }
    } else {
        rescan = position;
    }
}

auto Rescan::Queue(const block::Position& position) noexcept -> void
{
    auto lock = Lock{lock_};
    rescan_requests_.push(position);
    set_ready(lock);
}

auto Rescan::Reorg(const block::Position& parent) noexcept -> void
{
    {
        auto lock = Lock{lock_};
        flush(lock);
    }

    if (ceiling_ > parent) { ceiling_ = parent; }

    Scan::Reorg(parent);
}

auto Rescan::Run() noexcept -> bool
{
    auto needScan{false};

    try {
        auto lock = Lock{lock_};
        const auto current = [&] {
            if (is_running(lock)) {
                throw std::runtime_error{"job is already running"};
            }

            if (false == ready_) {
                throw std::runtime_error{"job is disabled"};
            }

            return last_scanned_;
        }();
        auto& log = LogTrace();
        const auto& name = parent_.name_;
        log(OT_PRETTY_CLASS())(name)(" last_scanned_ before flush is ");

        if (last_scanned_.has_value()) {
            const auto& value = last_scanned_.value();
            log(value.second->asHex())(" at height ")(value.first);
        } else {
            log("not set");
        };

        log.Flush();
        flush(lock);
        log(OT_PRETTY_CLASS())(name)(" last_scanned_ after flush is ");
        if (last_scanned_.has_value()) {
            const auto& value = last_scanned_.value();
            log(value.second->asHex())(" at height ")(value.first);
        } else {
            log("not set");
        };

        log.Flush();
        auto caughtUp{false};
        auto wait{false};

        if (last_scanned_.has_value()) {
            const auto& last = last_scanned_.value();

            if (last.first >= (ceiling_.first - 1)) {
                log(OT_PRETTY_CLASS())(name)(
                    " unable to rescan until scan has made more progress")
                    .Flush();
                wait = true;
            } else if (last_scanned_ == ceiling_) {
                log(OT_PRETTY_CLASS())(name)(
                    " rescan has caught up to scan progress")
                    .Flush();
                caughtUp = true;
            } else {
                const auto ld = progress_.Dirty();

                if (ld.has_value() && (last > ld.value())) {
                    last_scanned_ = parent_.node_.HeaderOracle().GetPosition(
                        ld.value().first - 1);
                    wait = true;
                } else {
                    auto ceiling = [&] {
                        const auto dirty = [&] {
                            auto out = ld;

                            if (out.has_value()) {

                                return std::max<block::Height>(
                                           out.value().first, 1u) -
                                       1u;
                            } else {

                                return std::numeric_limits<
                                    block::Height>::max();
                            }
                        }();

                        return std::min(ceiling_.first, dirty);
                    }();
                    log(OT_PRETTY_CLASS())(name)(
                        " highest clean block height is ")(ceiling)
                        .Flush();
                    auto best = update_tip(lock);
                    log(OT_PRETTY_CLASS())(name)(" best cfilter is ")(
                        best.second->asHex())(" at height ")(best.first)
                        .Flush();
                    needScan = queue_work(
                        [=, target = std::move(best)] {
                            Do(std::move(target), ceiling, last);
                        },
                        type(),
                        true);
                }
            }
        } else {
        }

        if (caughtUp) {
            ready_ = false;
            last_scanned_ = std::nullopt;
            needScan = false;
        } else if (wait) {
            needScan = queue_work([] { Sleep(250ms); }, type(), true);
        }
    } catch (...) {
        needScan = true;
    }

    return needScan;
}

auto Rescan::SetCeiling(const block::Position& ceiling) noexcept -> void
{
    auto lock = Lock{lock_};
    ceiling_ = ceiling;
}
}  // namespace opentxs::blockchain::node::wallet
