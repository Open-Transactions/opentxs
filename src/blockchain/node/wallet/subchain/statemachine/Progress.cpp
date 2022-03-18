// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/smart_ptr/detail/operator_bool.hpp>

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/wallet/subchain/statemachine/Progress.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <iterator>
#include <optional>
#include <utility>

#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Job.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs::blockchain::node::wallet
{
Progress::Imp::Imp(
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
          },
          {
              {parent.to_progress_endpoint_, Direction::Bind},
          })
    , last_reported_(std::nullopt)
{
}

auto Progress::Imp::notify(const block::Position& pos) const noexcept -> void
{
    parent_.ReportScan(pos);
}

auto Progress::Imp::ProcessReorg(const block::Position& parent) noexcept -> void
{
    if (last_reported_.has_value() && last_reported_.value() > parent) {
        last_reported_ = parent;
        notify(parent);
    }
}

auto Progress::Imp::process_update(Message&& msg) noexcept -> void
{
    auto clean = Set<ScanStatus>{get_allocator()};
    auto dirty = Set<block::Position>{get_allocator()};
    decode(parent_.api_, msg, clean, dirty);

    OT_ASSERT(0u == dirty.size());
    OT_ASSERT(0u < clean.size());

    const auto& best = clean.crbegin()->second;
    auto& last = last_reported_;

    if ((false == last.has_value()) || (last.value() != best)) {
        parent_.db_.SubchainSetLastScanned(parent_.db_key_, best);
        last = best;
        notify(best);
    }
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
Progress::Progress(const SubchainStateData& parent) noexcept
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

auto Progress::ChangeState(const State state) noexcept -> bool
{
    return imp_->ChangeState(state);
}

auto Progress::ProcessReorg(const block::Position& parent) noexcept -> void
{
    imp_->ProcessReorg(parent);
}

Progress::~Progress()
{
    if (imp_) { imp_->Shutdown(); }
}
}  // namespace opentxs::blockchain::node::wallet
