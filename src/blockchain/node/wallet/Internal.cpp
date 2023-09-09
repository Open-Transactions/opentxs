// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/wallet/Shared.hpp"  // IWYU pragma: associated

#include <future>

#include "opentxs/blockchain/block/Outpoint.hpp"         // IWYU pragma: keep
#include "opentxs/blockchain/block/TransactionHash.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/node/Spend.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Output.hpp"  // IWYU pragma: keep
#include "opentxs/core/Amount.hpp"

namespace opentxs::blockchain::node::internal
{
auto Wallet::Shared::ConstructTransaction(
    const node::Spend&,
    std::promise<SendOutcome>&&) const noexcept -> void
{
}

auto Wallet::Shared::CreateSpend(const identifier::Nym& spender) const noexcept
    -> node::Spend
{
    return {};
}

auto Wallet::Shared::Execute(node::Spend&) const noexcept -> PendingOutgoing
{
    return {};
}

auto Wallet::Shared::FeeEstimate() const noexcept -> std::optional<Amount>
{
    return {};
}

auto Wallet::Shared::GetBalance() const noexcept -> Balance { return {}; }

auto Wallet::Shared::GetBalance(const identifier::Nym&) const noexcept
    -> Balance
{
    return {};
}

auto Wallet::Shared::GetBalance(
    const identifier::Nym&,
    const identifier::Account&) const noexcept -> Balance
{
    return {};
}

auto Wallet::Shared::GetBalance(const crypto::Key&) const noexcept -> Balance
{
    return {};
}

auto Wallet::Shared::GetOutputs(alloc::Default) const noexcept -> Vector<UTXO>
{
    return {};
}

auto Wallet::Shared::GetOutputs(TxoState, alloc::Default) const noexcept
    -> Vector<UTXO>
{
    return {};
}

auto Wallet::Shared::GetOutputs(const identifier::Nym&, alloc::Default)
    const noexcept -> Vector<UTXO>
{
    return {};
}

auto Wallet::Shared::GetOutputs(
    const identifier::Nym&,
    TxoState,
    alloc::Default) const noexcept -> Vector<UTXO>
{
    return {};
}

auto Wallet::Shared::GetOutputs(
    const identifier::Nym&,
    const identifier::Account&,
    alloc::Default) const noexcept -> Vector<UTXO>
{
    return {};
}

auto Wallet::Shared::GetOutputs(
    const identifier::Nym&,
    const identifier::Account&,
    TxoState,
    alloc::Default) const noexcept -> Vector<UTXO>
{
    return {};
}

auto Wallet::Shared::GetOutputs(const crypto::Key&, TxoState, alloc::Default)
    const noexcept -> Vector<UTXO>
{
    return {};
}

auto Wallet::Shared::GetTags(const block::Outpoint&) const noexcept
    -> UnallocatedSet<TxoTag>
{
    return {};
}

auto Wallet::Shared::Height() const noexcept -> block::Height { return {}; }

auto Wallet::Shared::Run() noexcept -> bool { return {}; }
}  // namespace opentxs::blockchain::node::internal
