// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/wallet/proposals/Proposals.hpp"  // IWYU pragma: associated

#include <utility>

#include "blockchain/node/wallet/proposals/ProposalsPrivate.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/node/Spend.hpp"             // IWYU pragma: keep
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::node::wallet
{
Proposals::Proposals(
    const api::session::Client& api,
    const node::Manager& node,
    database::Wallet& db,
    const Type chain) noexcept
    : imp_(std::make_unique<ProposalsPrivate>(api, node, db, chain))
{
    assert_false(nullptr == imp_);
}

auto Proposals::Add(
    const node::Spend& spend,
    std::promise<SendOutcome>&& promise) const noexcept -> bool
{
    return imp_->Add(spend, std::move(promise));
}

auto Proposals::Run() noexcept -> bool { return imp_->Run(); }

Proposals::~Proposals() = default;
}  // namespace opentxs::blockchain::node::wallet
