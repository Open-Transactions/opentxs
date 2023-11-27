// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/wallet/subchain/statemachine/Scan.hpp"  // IWYU pragma: associated

#include <compare>
#include <limits>
#include <memory>
#include <utility>

#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "blockchain/node/wallet/subchain/statemachine/MatchCache.hpp"
#include "internal/blockchain/database/Wallet.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/wallet/Reorg.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/alloc/Logging.hpp"
#include "opentxs/api/Network.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/network/ZeroMQ.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Output.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/socket/Direction.hpp"   // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Policy.hpp"      // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "util/ScopeGuard.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::wallet
{
using enum opentxs::network::zeromq::socket::Direction;
using enum opentxs::network::zeromq::socket::Policy;
using enum opentxs::network::zeromq::socket::Type;

Scan::Imp::Imp(
    const std::shared_ptr<const SubchainStateData>& parent,
    const network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : Job(LogTrace(),
          parent,
          batch,
          JobType::scan,
          alloc,
          {
              {parent->node_.Internal().Endpoints().new_filter_publish_,
               Connect},
          },
          {
              {parent->to_scan_endpoint_, Bind},
          },
          {},
          {
              {Push,
               Internal,
               {
                   {parent->to_process_endpoint_, Connect},
               }},
          })
    , to_process_(pipeline_.Internal().ExtraSocket(1))
    , last_scanned_(std::nullopt)
    , filter_tip_(std::nullopt)
    , index_ready_(false)
{
}

auto Scan::Imp::caught_up() const noexcept -> bool
{
    return current() == filter_tip_.value_or(parent_.null_position_);
}

auto Scan::Imp::current() const noexcept -> const block::Position&
{
    if (last_scanned_.has_value()) {

        return last_scanned_.value();
    } else {

        return parent_.null_position_;
    }
}

auto Scan::Imp::do_reorg(
    const node::HeaderOracle& oracle,
    const node::internal::HeaderOraclePrivate& data,
    Reorg::Params& params) noexcept -> bool
{
    if (false == parent_.need_reorg_) { return true; }

    const auto& [position, tx] = params;

    if (last_scanned_.has_value()) {
        const auto target =
            parent_.ReorgTarget(position, last_scanned_.value());
        log_()(name_)(" last scanned reset to ")(target).Flush();
        last_scanned_ = target;
    }

    if (filter_tip_.has_value() && (filter_tip_->IsReplacedBy(position))) {
        log_()(name_)(" filter tip reset to ")(position).Flush();
        filter_tip_ = position;
    }

    return Job::do_reorg(oracle, data, params);
}

auto Scan::Imp::do_startup_internal(allocator_type monotonic) noexcept -> void
{
    const auto& node = node_;
    const auto& filters = node.FilterOracle();
    const auto& log = log_;
    last_scanned_ = parent_.db_.SubchainLastScanned(parent_.db_key_);
    filter_tip_ = filters.FilterTip(parent_.filter_type_);

    assert_true(last_scanned_.has_value());
    assert_true(filter_tip_.has_value());

    log()(name_)(" loaded last scanned value of ")(last_scanned_.value())(
        " from database")
        .Flush();
    log()(name_)(" loaded filter tip value of ")(last_scanned_.value())(
        " from filter oracle")
        .Flush();

    if (last_scanned_->IsReplacedBy(*filter_tip_)) {
        log()(name_)(" last scanned reset to ")(filter_tip_.value()).Flush();
        last_scanned_ = filter_tip_;
    }

    to_process_.SendDeferred([&] {
        auto out = MakeWork(Work::update);
        add_last_reorg(out);
        auto clean = Vector<ScanStatus>{get_allocator()};
        clean.emplace_back(ScanState::scan_clean, last_scanned_.value());
        encode(clean, out);

        return out;
    }());
}

auto Scan::Imp::forward_to_next(Message&& msg) noexcept -> void
{
    to_process_.SendDeferred(std::move(msg));
}

auto Scan::Imp::process_do_rescan(Message&& in) noexcept -> void
{
    last_scanned_.reset();
    parent_.match_cache_.lock()->Reset();
    to_process_.SendDeferred(std::move(in));
}

auto Scan::Imp::process_filter(
    Message&& in,
    block::Position&& tip,
    allocator_type monotonic) noexcept -> void
{
    if (tip < this->tip()) {
        log_()(name_)(" ignoring stale filter tip ")(tip).Flush();

        return;
    }

    log_()(name_)(" filter tip updated to ")(tip).Flush();
    filter_tip_ = std::move(tip);

    if (auto last = last_reorg(); last.has_value()) {
        in.AddFrame(last.value());
    }

    to_process_.SendDeferred(std::move(in));
    do_work(monotonic);
}

auto Scan::Imp::process_start_scan(Message&&, allocator_type monotonic) noexcept
    -> void
{
    index_ready_ = true;
    log_()(name_)(
        " ready to begin scan now that initial index operation is complete")
        .Flush();
    do_work(monotonic);
}

auto Scan::Imp::tip() const noexcept -> const block::Position&
{
    if (filter_tip_.has_value()) {

        return filter_tip_.value();
    } else {

        return parent_.null_position_;
    }
}

auto Scan::Imp::work(allocator_type monotonic) noexcept -> bool
{
    if ((false == index_ready_) || (State::reorg == state())) { return false; }

    auto post = ScopeGuard{[&] { Job::work(monotonic); }};

    if (false == filter_tip_.has_value()) {
        log_()(name_)(
            " scanning not possible until a filter tip value is received ")
            .Flush();

        return false;
    }

    if (caught_up()) {
        log_()(name_)(" all available filters have been scanned").Flush();

        return false;
    }

    const auto height = current().height_;
    const auto rescan = [&]() -> block::Height {
        auto handle = parent_.progress_position_.lock();

        if (handle->has_value()) {

            return handle->value().height_;
        } else {

            return -1;
        }
    }();
    const auto& threshold = parent_.scan_threshold_;

    if (parent_.scan_dirty_ && ((height - rescan) > threshold)) {
        log_()(name_)(
            " waiting to continue scan until rescan has caught up to block ")(
            height - threshold)(" from current position of ")(rescan)
            .Flush();

        return false;
    }

    auto clean = Vector<ScanStatus>{monotonic};
    clean.clear();
    auto dirty = Vector<ScanStatus>{monotonic};
    dirty.clear();
    auto highestTested = current();
    const auto highestClean = parent_.Scan(
        filter_tip_.value(),
        std::numeric_limits<block::Height>::max(),
        highestTested,
        dirty,
        monotonic);
    last_scanned_ = std::move(highestTested);
    log_()(name_)(" last scanned updated to ")(current()).Flush();

    if (auto count = dirty.size(); 0_uz < count) {
        log_()(name_)(" ")(count)(" blocks queued for processing ").Flush();
        to_process_.SendDeferred([&] {
            auto out = MakeWork(Work::update);
            add_last_reorg(out);
            encode(dirty, out);

            return out;
        }());
    }

    if (highestClean.has_value()) {
        clean.emplace_back(ScanState::scan_clean, highestClean.value());
        to_process_.SendDeferred([&] {
            auto out = MakeWork(Work::update);
            add_last_reorg(out);
            encode(clean, out);

            return out;
        }());
    }

    return (false == caught_up());
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
Scan::Scan(const std::shared_ptr<const SubchainStateData>& parent) noexcept
    : imp_([&] {
        const auto& asio = parent->api_.Network().ZeroMQ().Context().Internal();
        const auto batchID = asio.PreallocateBatch();

        return std::allocate_shared<Imp>(
            alloc::PMR<Imp>{asio.Alloc(batchID)}, parent, batchID);
    }())
{
    assert_false(nullptr == imp_);
}

auto Scan::Init() noexcept -> void
{
    imp_->Init(imp_);
    imp_.reset();
}

Scan::~Scan() = default;
}  // namespace opentxs::blockchain::node::wallet
