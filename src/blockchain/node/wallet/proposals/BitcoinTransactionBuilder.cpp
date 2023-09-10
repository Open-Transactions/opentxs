// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/wallet/proposals/BitcoinTransactionBuilder.hpp"  // IWYU pragma: associated

#include "blockchain/node/wallet/proposals/BitcoinTransactionBuilderPrivate.hpp"
#include "internal/blockchain/node/wallet/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"

namespace opentxs::blockchain::node::wallet
{
BitcoinTransactionBuilder::BitcoinTransactionBuilder(
    const api::Session& api,
    const node::Manager& node,
    const identifier::Generic& id,
    const Type chain,
    database::Wallet& db,
    node::Spend& proposal,
    std::promise<SendOutcome>& promise) noexcept
    : imp_(std::make_unique<BitcoinTransactionBuilderPrivate>(
          api,
          node,
          id,
          chain,
          db,
          proposal,
          promise))
{
    OT_ASSERT(imp_);
}

auto BitcoinTransactionBuilder::operator()() noexcept -> BuildResult
{
    return imp_->operator()();
}

BitcoinTransactionBuilder::~BitcoinTransactionBuilder() = default;
}  // namespace opentxs::blockchain::node::wallet
