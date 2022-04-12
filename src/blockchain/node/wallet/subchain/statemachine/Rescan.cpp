// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/smart_ptr/detail/operator_bool.hpp>

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/wallet/subchain/statemachine/Rescan.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <algorithm>
#include <atomic>
#include <iterator>
#include <limits>
#include <utility>

#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Job.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "util/ScopeGuard.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::wallet
{
Rescan::Imp::Imp(
    const boost::shared_ptr<const SubchainStateData>& parent,
    const network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : Job(LogTrace(),
          parent,
          batch,
          JobType::rescan,
          alloc,
          {
              {CString{parent->api_.Endpoints().BlockchainNewFilter()},
               Direction::Connect},
          },
          {
              {parent->to_rescan_endpoint_, Direction::Bind},
          },
          {},
          {
              {SocketType::Push,
               {
                   {parent->to_scan_endpoint_, Direction::Connect},
               }},
              {SocketType::Push,
               {
                   {parent->to_process_endpoint_, Direction::Connect},
               }},
              {SocketType::Push,
               {
                   {parent->to_progress_endpoint_, Direction::Connect},
               }},
          })
    , to_scan_(pipeline_.Internal().ExtraSocket(1))
    , to_process_(pipeline_.Internal().ExtraSocket(2))
    , to_progress_(pipeline_.Internal().ExtraSocket(3))
    , last_scanned_(std::nullopt)
    , filter_tip_(std::nullopt)
    , dirty_(alloc)
{
}

auto Rescan::Imp::adjust_last_scanned(
    const std::optional<block::Position>& highestClean) noexcept -> void
{
    // NOTE before any dirty blocks have been received last_scanned_ simply
    // follows the progress of the Scan operation. After Rescan is enabled
    // last_scanned_ is controlled by work() until rescan catches up and
    // parent_.scan_dirty_ is false.

    if (parent_.scan_dirty_) {
        log_(OT_PRETTY_CLASS())(name_)(
            " ignoring scan position update due to active rescan in progress")
            .Flush();
    } else {
        const auto effective = highestClean.value_or(parent_.null_position_);

        if (highestClean.has_value()) {
            log_(OT_PRETTY_CLASS())(name_)(" last scanned updated to ")(
                opentxs::print(effective))
                .Flush();
            set_last_scanned(highestClean);
        }
    }
}

auto Rescan::Imp::before(const block::Position& position) const noexcept
    -> block::Position
{
    return parent_.node_.HeaderOracle().GetPosition(
        std::max<block::Height>(position.first - 1, 0));
}

auto Rescan::Imp::can_advance() const noexcept -> bool
{
    const auto target = [this] {
        if (0u == dirty_.size()) {

            return filter_tip_.value_or(parent_.null_position_);
        } else {
            const auto& lowestDirty = *dirty_.cbegin();

            return parent_.node_.HeaderOracle().GetPosition(
                std::max<block::Height>(lowestDirty.first - 1, 0));
        }
    }();
    const auto position = current();
    log_(OT_PRETTY_CLASS())(name_)(
        " the highest position available for rescanning is ")(print(target))
        .Flush();
    log_(OT_PRETTY_CLASS())(name_)(" the current position is ")(print(position))
        .Flush();

    return position != target;
}

auto Rescan::Imp::caught_up() const noexcept -> bool
{
    return current() == filter_tip_.value_or(parent_.null_position_);
}

auto Rescan::Imp::current() const noexcept -> const block::Position&
{
    if (last_scanned_.has_value()) {

        return last_scanned_.value();
    } else {

        return parent_.null_position_;
    }
}

auto Rescan::Imp::do_startup() noexcept -> void
{
    const auto& node = parent_.node_;
    const auto& filters = node.FilterOracleInternal();
    set_last_scanned(parent_.db_.SubchainLastScanned(parent_.db_key_));
    filter_tip_ = filters.FilterTip(parent_.filter_type_);

    OT_ASSERT(last_scanned_.has_value());
    OT_ASSERT(filter_tip_.has_value());

    log_(OT_PRETTY_CLASS())(name_)(" loaded last scanned value of ")(
        opentxs::print(last_scanned_.value()))(" from database")
        .Flush();
    log_(OT_PRETTY_CLASS())(name_)(" loaded filter tip value of ")(
        opentxs::print(last_scanned_.value()))(" from filter oracle")
        .Flush();

    if (last_scanned_.value() > filter_tip_.value()) {
        log_(OT_PRETTY_CLASS())(name_)(" last scanned reset to ")(
            opentxs::print(filter_tip_.value()))
            .Flush();
        set_last_scanned(filter_tip_);
    }
}

auto Rescan::Imp::highest_clean(const Set<block::Position>& clean)
    const noexcept -> std::optional<block::Position>
{
    if (clean.empty()) { return std::nullopt; }

    if (dirty_.empty()) { return *clean.crbegin(); }

    if (auto it = clean.lower_bound(*dirty_.cbegin()); clean.begin() != it) {

        return *(--it);
    } else {

        return std::nullopt;
    }
}

auto Rescan::Imp::ProcessReorg(const block::Position& parent) noexcept -> void
{
    if (last_scanned_.has_value() && (last_scanned_.value() > parent)) {
        log_(OT_PRETTY_CLASS())(name_)(" last scanned reset to ")(
            opentxs::print(parent))
            .Flush();
        set_last_scanned(parent);
    }

    if (filter_tip_.has_value() && (filter_tip_.value() > parent)) {
        log_(OT_PRETTY_CLASS())(name_)(" filter tip reset to ")(
            opentxs::print(parent))
            .Flush();
        filter_tip_ = parent;
    }

    dirty_.erase(dirty_.upper_bound(parent), dirty_.end());
}

auto Rescan::Imp::process_clean(const Set<ScanStatus>& clean) noexcept -> void
{
    for (const auto& [state, position] : clean) {
        if (ScanState::processed == state) {
            log_(OT_PRETTY_CLASS())(name_)(" removing processed block ")(
                opentxs::print(position))(" from dirty list")
                .Flush();
            dirty_.erase(position);
        }
    }
}

auto Rescan::Imp::process_dirty(const Set<block::Position>& dirty) noexcept
    -> void
{
    if (0u < dirty.size()) {
        if (auto val = parent_.scan_dirty_.exchange(true); false == val) {
            log_(OT_PRETTY_CLASS())(name_)(
                " enabling rescan since update contains dirty blocks")
                .Flush();
        }

        for (auto& position : dirty) {
            log_(OT_PRETTY_CLASS())(name_)(" block ")(opentxs::print(position))(
                " must be processed due to cfilter matches")
                .Flush();
            dirty_.emplace(std::move(position));
        }
    }

    if (0u < dirty_.size()) {
        const auto& lowestDirty = *dirty_.cbegin();
        const auto limit = before(lowestDirty);
        const auto current = this->current();

        if (current > limit) {
            log_(OT_PRETTY_CLASS())(name_)(" adjusting last scanned to ")(
                opentxs::print(limit))(" based on dirty block ")(
                opentxs::print(lowestDirty))
                .Flush();
            set_last_scanned(limit);
        }
    }
}

auto Rescan::Imp::process_filter(block::Position&& tip) noexcept -> void
{
    log_(OT_PRETTY_CLASS())(name_)(" filter tip updated to ")(
        opentxs::print(tip))
        .Flush();
    filter_tip_ = std::move(tip);
    do_work();
}

auto Rescan::Imp::process_update(Message&& msg) noexcept -> void
{
    auto clean = Set<ScanStatus>{get_allocator()};
    auto dirty = Set<block::Position>{get_allocator()};
    decode(parent_.api_, msg, clean, dirty);
    const auto highestClean = highest_clean([&] {
        auto out = Set<block::Position>{};
        std::transform(
            clean.begin(),
            clean.end(),
            std::inserter(out, out.end()),
            [&](const auto& in) {
                auto& [type, pos] = in;

                return pos;
            });

        return out;
    }());
    adjust_last_scanned(highestClean);
    process_dirty(dirty);
    process_clean(clean);

    if (parent_.scan_dirty_) {
        do_work();
    } else if (0u < clean.size()) {
        to_progress_.SendDeferred(std::move(msg));
    }
}

auto Rescan::Imp::prune() noexcept -> void
{
    OT_ASSERT(last_scanned_.has_value());

    const auto& target = last_scanned_.value();

    for (auto i = dirty_.begin(), end = dirty_.end(); i != end;) {
        const auto& position = *i;

        if (position <= target) {
            log_(OT_PRETTY_CLASS())(name_)(" pruning re-scanned position ")(
                opentxs::print(position))
                .Flush();
            i = dirty_.erase(i);
        } else {
            break;
        }
    }
}

auto Rescan::Imp::set_last_scanned(const block::Position& value) noexcept
    -> void
{
    last_scanned_ = value;
    update_progress();
}

auto Rescan::Imp::set_last_scanned(
    const std::optional<block::Position>& value) noexcept -> void
{
    last_scanned_ = value;
    update_progress();
}

auto Rescan::Imp::set_last_scanned(
    std::optional<block::Position>&& value) noexcept -> void
{
    last_scanned_ = std::move(value);
    update_progress();
}

auto Rescan::Imp::stop() const noexcept -> block::Height
{
    if (0u == dirty_.size()) {
        log_(OT_PRETTY_CLASS())(name_)(" dirty list is empty so rescan "
                                       "may proceed to the end of the "
                                       "chain")
            .Flush();

        return std::numeric_limits<block::Height>::max();
    }

    const auto& lowestDirty = *dirty_.cbegin();
    const auto stopHeight = std::max<block::Height>(0, lowestDirty.first - 1);
    log_(OT_PRETTY_CLASS())(name_)(" first dirty block is ")(
        print(lowestDirty))(" so rescan must stop at height ")(
        stopHeight)(" until this block has been processed")
        .Flush();

    return stopHeight;
}

auto Rescan::Imp::update_progress() noexcept -> void
{
    parent_.rescan_progress_.store(current().first);
    to_scan_.Send(MakeWork(Work::statemachine));
}

auto Rescan::Imp::work() noexcept -> bool
{
    auto post = ScopeGuard{[&] {
        if (last_scanned_.has_value()) {
            log_(OT_PRETTY_CLASS())(name_)(" progress updated to ")(
                opentxs::print(last_scanned_.value()))
                .Flush();
            auto clean = Vector<ScanStatus>{get_allocator()};
            to_progress_.SendDeferred([&] {
                clean.emplace_back(
                    ScanState::rescan_clean, last_scanned_.value());
                auto out = MakeWork(Work::update);
                encode(clean, out);

                return out;
            }());
        }
    }};

    if (false == parent_.scan_dirty_) {
        log_(OT_PRETTY_CLASS())(name_)(" rescan is not necessary").Flush();

        return false;
    }

    if (caught_up()) {
        parent_.scan_dirty_ = false;
        log_(OT_PRETTY_CLASS())(name_)(
            " rescan has caught up to current filter tip")
            .Flush();

        return false;
    }

    auto highestTested = current();
    const auto stopHeight = stop();

    if (highestTested.first >= stopHeight) {
        log_(OT_PRETTY_CLASS())(name_)(" waiting for first of ")(dirty_.size())(
            " blocks to finish processing before continuing rescan "
            "beyond height ")(highestTested.first)
            .Flush();

        return false;
    }

    auto dirty = Vector<ScanStatus>{get_allocator()};
    auto highestClean =
        parent_.Rescan(filter_tip_.value(), stop(), highestTested, dirty);

    if (highestClean.has_value()) {
        log_(OT_PRETTY_CLASS())(name_)(" last scanned updated to ")(
            opentxs::print(highestClean.value()))
            .Flush();
        set_last_scanned(std::move(highestClean));
        // TODO The interval used for rescanning should never include any dirty
        // blocks so is it possible for prune() to ever do anything?
        prune();
    } else {
        // NOTE either the first tested block was dirty or else the scan was
        // interrupted for a state change
    }

    OT_ASSERT(last_scanned_.has_value());

    if (auto count = dirty.size(); 0u < count) {
        log_(OT_PRETTY_CLASS())(name_)(" re-processing ")(count)(" items:")
            .Flush();
        auto work = MakeWork(Work::reprocess);

        for (auto& status : dirty) {
            auto& [type, position] = status;
            log_(" * ")(opentxs::print(position)).Flush();
            encode(status, work);
            dirty_.emplace(std::move(position));
        }

        to_process_.Send(std::move(work));
    } else {
        log_(OT_PRETTY_CLASS())(name_)(" all blocks are clean after rescan")
            .Flush();
    }

    return can_advance();
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
Rescan::Rescan(
    const boost::shared_ptr<const SubchainStateData>& parent) noexcept
    : imp_([&] {
        const auto& asio = parent->api_.Network().ZeroMQ().Internal();
        const auto batchID = asio.PreallocateBatch();
        // TODO the version of libc++ present in android ndk 23.0.7599858
        // has a broken std::allocate_shared function so we're using
        // boost::shared_ptr instead of std::shared_ptr

        return boost::allocate_shared<Imp>(
            alloc::PMR<Imp>{asio.Alloc(batchID)}, parent, batchID);
    }())
{
    imp_->Init(imp_);
}

auto Rescan::ChangeState(const State state, StateSequence reorg) noexcept
    -> bool
{
    return imp_->ChangeState(state, reorg);
}

auto Rescan::ProcessReorg(const block::Position& parent) noexcept -> void
{
    imp_->ProcessReorg(parent);
}

Rescan::~Rescan()
{
    if (imp_) { imp_->Shutdown(); }
}
}  // namespace opentxs::blockchain::node::wallet
