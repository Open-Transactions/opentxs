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
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::wallet
{
Rescan::Imp::Imp(
    const SubchainStateData& parent,
    const network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : Job(LogTrace(),
          parent,
          batch,
          CString{"rescan", alloc},
          alloc,
          {
              {parent.shutdown_endpoint_, Direction::Connect},
              {CString{parent.api_.Endpoints().BlockchainNewFilter()},
               Direction::Connect},
          },
          {
              {parent.to_rescan_endpoint_, Direction::Bind},
          },
          {},
          {
              {SocketType::Push,
               {
                   {parent.to_process_endpoint_, Direction::Connect},
               }},
              {SocketType::Push,
               {
                   {parent.to_progress_endpoint_, Direction::Connect},
               }},
          })
    , to_process_(pipeline_.Internal().ExtraSocket(0))
    , to_progress_(pipeline_.Internal().ExtraSocket(1))
    , active_(false)
    , last_scanned_(std::nullopt)
    , filter_tip_(std::nullopt)
    , dirty_(alloc)
{
}

auto Rescan::Imp::adjust_last_scanned(
    std::optional<block::Position>&& highestClean) noexcept -> void
{
    const auto effective = highestClean.value_or(parent_.null_position_);

    if (active_) {
        log_(OT_PRETTY_CLASS())(parent_.name_)(" ignoring scan position ")(
            opentxs::print(effective))(" due to active rescan in progress")
            .Flush();

        return;
    }

    if (highestClean.has_value()) {
        log_(OT_PRETTY_CLASS())(parent_.name_)(" last scanned updated to ")(
            opentxs::print(effective))
            .Flush();

        last_scanned_ = std::move(highestClean);
    }
}

auto Rescan::Imp::caught_up() const noexcept -> bool
{
    return last_scanned_.value_or(parent_.null_position_) ==
           filter_tip_.value_or(parent_.null_position_);
}

auto Rescan::Imp::do_startup() noexcept -> void
{
    const auto& node = parent_.node_;
    const auto& filters = node.FilterOracleInternal();
    last_scanned_ = parent_.db_.SubchainLastScanned(parent_.db_key_);
    filter_tip_ = filters.FilterTip(parent_.filter_type_);

    OT_ASSERT(last_scanned_.has_value());
    OT_ASSERT(filter_tip_.has_value());

    log_(OT_PRETTY_CLASS())(parent_.name_)(" loaded last scanned value of ")(
        opentxs::print(last_scanned_.value()))(" from database")
        .Flush();
    log_(OT_PRETTY_CLASS())(parent_.name_)(" loaded filter tip value of ")(
        opentxs::print(last_scanned_.value()))(" from filter oracle")
        .Flush();

    if (last_scanned_.value() > filter_tip_.value()) {
        log_(OT_PRETTY_CLASS())(parent_.name_)(" last scanned reset to ")(
            opentxs::print(filter_tip_.value()))
            .Flush();
        last_scanned_ = filter_tip_;
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

auto Rescan::Imp::process(const Set<ScanStatus>& clean) noexcept -> void
{
    for (const auto& [state, position] : clean) {
        if (ScanState::processed == state) {
            log_(OT_PRETTY_CLASS())(parent_.name_)(
                " removing processed block ")(opentxs::print(position))(
                " from dirty list")
                .Flush();
            dirty_.erase(position);
        }
    }
}

auto Rescan::Imp::ProcessReorg(const block::Position& parent) noexcept -> void
{
    if (last_scanned_.has_value() && (last_scanned_.value() > parent)) {
        log_(OT_PRETTY_CLASS())(parent_.name_)(" last scanned reset to ")(
            opentxs::print(parent))
            .Flush();
        last_scanned_ = parent;
    }

    if (filter_tip_.has_value() && (filter_tip_.value() > parent)) {
        log_(OT_PRETTY_CLASS())(parent_.name_)(" filter tip reset to ")(
            opentxs::print(parent))
            .Flush();
        filter_tip_ = parent;
    }

    dirty_.erase(dirty_.upper_bound(parent), dirty_.end());
}

auto Rescan::Imp::process_filter(block::Position&& tip) noexcept -> void
{
    log_(OT_PRETTY_CLASS())(parent_.name_)(" filter tip updated to ")(
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
    adjust_last_scanned(highest_clean([&] {
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
    }()));

    if (0u < dirty.size()) {
        if (false == active_) {
            log_(OT_PRETTY_CLASS())(parent_.name_)(
                " enabling rescan since update contains dirty blocks")
                .Flush();
            active_ = true;
        }

        std::move(
            dirty.begin(), dirty.end(), std::inserter(dirty_, dirty_.end()));
    }

    process(clean);

    if (active_) {
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
            log_(OT_PRETTY_CLASS())(parent_.name_)(
                " pruning re-scanned position ")(opentxs::print(position))
                .Flush();
            i = dirty_.erase(i);
        } else {
            break;
        }
    }
}

auto Rescan::Imp::stop() const noexcept -> block::Height
{
    if (0u == dirty_.size()) {

        return std::numeric_limits<block::Height>::max();
    }

    return std::max<block::Height>(0, dirty_.cbegin()->first - 1);
}

auto Rescan::Imp::work() noexcept -> bool
{
    if (false == active_) {
        log_(OT_PRETTY_CLASS())(parent_.name_)(" rescan is not necessary")
            .Flush();

        return false;
    }

    const auto notify = [&] {
        auto clean = Vector<ScanStatus>{get_allocator()};
        to_progress_.SendDeferred([&] {
            clean.emplace_back(ScanState::rescan_clean, last_scanned_.value());
            auto out = MakeWork(Work::update);
            encode(clean, out);

            return out;
        }());
    };

    if (caught_up()) {
        active_ = false;
        log_(OT_PRETTY_CLASS())(parent_.name_)(
            " rescan has caught up to current filter tip")
            .Flush();

        if (last_scanned_.has_value()) { notify(); }

        return false;
    }

    auto dirty = Vector<ScanStatus>{get_allocator()};
    auto highestTested = last_scanned_.value_or(parent_.null_position_);
    const auto stopHeight = stop();

    if (highestTested.first >= stopHeight) {
        log_(OT_PRETTY_CLASS())(parent_.name_)(
            " waiting for block processing to finish before continuing rescan")
            .Flush();

        return false;
    }

    auto highestClean =
        parent_.Scan(filter_tip_.value(), stop(), highestTested, dirty);

    if (highestClean.has_value()) {
        log_(OT_PRETTY_CLASS())(parent_.name_)(" last scanned updated to ")(
            opentxs::print(highestClean.value()))
            .Flush();
        last_scanned_ = std::move(highestClean);
        prune();
    } else {
        // NOTE either the first tested block was dirty or else the scan was
        // interrupted for a state change
    }

    if (last_scanned_.has_value()) {
        log_(OT_PRETTY_CLASS())(parent_.name_)(" progress updated to ")(
            opentxs::print(last_scanned_.value()))
            .Flush();
        notify();
    } else {
        // NOTE: this should only happen if the genesis block has multiple
        // matching transactions, not all of which were discovered by the
        // original scan. Since that is such an incredibly rare condition it's
        // probably a programming error instead.
        OT_FAIL;
    }

    if (auto count = dirty.size(); 0u < count) {
        log_(OT_PRETTY_CLASS())(parent_.name_)(" re-processing ")(
            count)(" items ")
            .Flush();
        to_process_.SendDeferred([&] {
            auto out = MakeWork(Work::update);
            encode(dirty, out);

            return out;
        }());

        for (auto& [type, position] : dirty) {
            dirty_.emplace(std::move(position));
        }
    }

    if (0u == dirty_.size()) {

        return false == caught_up();
    } else {
        const auto& lowestDirty = *dirty_.cbegin();

        return last_scanned_.value_or(parent_.null_position_) < lowestDirty;
    }
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
Rescan::Rescan(const SubchainStateData& parent) noexcept
    : imp_([&] {
        const auto& asio = parent.api_.Network().ZeroMQ().Internal();
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

auto Rescan::ChangeState(const State state) noexcept -> bool
{
    return imp_->ChangeState(state);
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
