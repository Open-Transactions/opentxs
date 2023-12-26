// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/wallet/subchain/statemachine/Progress.hpp"  // IWYU pragma: associated

#include <iterator>
#include <memory>
#include <optional>
#include <utility>

#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "blockchain/node/wallet/subchain/statemachine/MatchCache.hpp"
#include "internal/blockchain/database/Wallet.hpp"
#include "internal/blockchain/node/wallet/Reorg.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/alloc/Logging.hpp"
#include "opentxs/WorkType.internal.hpp"
#include "opentxs/api/Network.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/network/ZeroMQ.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/socket/Direction.hpp"   // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Policy.hpp"      // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::node::wallet
{
using enum opentxs::network::zeromq::socket::Direction;
using enum opentxs::network::zeromq::socket::Policy;
using enum opentxs::network::zeromq::socket::Type;

Progress::Imp::Imp(
    const std::shared_ptr<const SubchainStateData>& parent,
    const network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : Job(LogTrace(),
          parent,
          batch,
          JobType::progress,
          alloc,
          {},
          {
              {parent->to_progress_endpoint_, Bind},
          },
          {},
          {
              {Push,
               Internal,
               {
                   {parent->to_scan_endpoint_, Connect},
               }},
          })
    , to_scan_(pipeline_.Internal().ExtraSocket(1))
{
}

auto Progress::Imp::do_process_update(
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    auto alloc = alloc::Strategy{get_allocator(), monotonic};
    const auto& log = log_;
    auto clean = Set<ScanStatus>{alloc.work_};
    auto dirty = Set<block::Position>{alloc.work_};
    decode(api_, msg, clean, dirty);

    assert_true(0u == dirty.size());
    assert_true(0u < clean.size());

    const auto& best = clean.crbegin()->second;
    log()(name_)(" received update: ")(best).Flush();
    auto handle = parent_.progress_position_.lock();
    auto& last = *handle;

    if ((false == last.has_value()) || (last.value() != best)) {
        parent_.db_.SubchainSetLastScanned(log_, parent_.db_key_, best, alloc);
        log()(name_)(" progress saved to database").Flush();
        last = best;
        notify(best);
        to_scan_.SendDeferred(MakeWork(Work::statemachine));
    }

    parent_.match_cache_.lock()->Forget(best);
}

auto Progress::Imp::do_reorg(
    const node::HeaderOracle& oracle,
    const node::internal::HeaderOraclePrivate& data,
    Reorg::Params& params) noexcept -> bool
{
    if (false == parent_.need_reorg_) { return true; }

    const auto& [position, tx] = params;
    auto handle = parent_.progress_position_.lock();
    auto& last = *handle;

    if (last.has_value()) {
        const auto target = parent_.ReorgTarget(position, last.value());
        last = target;
        notify(target);
    }

    return Job::do_reorg(oracle, data, params);
}

auto Progress::Imp::notify(const block::Position& pos) const noexcept -> void
{
    parent_.ReportScan(pos);
}

auto Progress::Imp::process_do_rescan(Message&& in) noexcept -> void
{
    auto alloc = alloc::Strategy{get_allocator()};  // TODO monotonic
    parent_.progress_position_.lock()->reset();
    const auto& best = parent_.null_position_;
    parent_.db_.SubchainSetLastScanned(log_, parent_.db_key_, best, alloc);
    parent_.RescanFinished();
    notify(best);
    to_scan_.SendDeferred(MakeWork(Work::statemachine));
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
Progress::Progress(
    const std::shared_ptr<const SubchainStateData>& parent) noexcept
    : imp_([&] {
        const auto& asio = parent->api_.Network().ZeroMQ().Context().Internal();
        const auto batchID = asio.PreallocateBatch();

        return std::allocate_shared<Imp>(
            alloc::PMR<Imp>{asio.Alloc(batchID)}, parent, batchID);
    }())
{
    assert_false(nullptr == imp_);
}

auto Progress::Init() noexcept -> void
{
    imp_->Init(imp_);
    imp_.reset();
}

Progress::~Progress() = default;
}  // namespace opentxs::blockchain::node::wallet
