// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/crypto/Ethereum.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/blockchain/crypto/Ethereum.hpp"
#include "internal/blockchain/crypto/Imported.hpp"
#include "internal/blockchain/crypto/Subaccount.hpp"
#include "opentxs/core/Amount.hpp"

namespace opentxs::blockchain::crypto
{
Ethereum::Ethereum(std::shared_ptr<internal::Subaccount> imp) noexcept
    : Imported(std::move(imp))
{
}

Ethereum::Ethereum(const Ethereum& rhs) noexcept
    : Imported(rhs)
{
}

Ethereum::Ethereum(Ethereum&& rhs) noexcept
    : Imported(std::move(rhs))
{
}

auto Ethereum::AddIncoming(
    const Amount& balance,
    const block::TransactionHash& txid,
    bool confirmed) noexcept -> bool
{
    if (auto p = imp_.lock(); p) {
        return p->asImported().asEthereum().AddIncoming(
            balance, txid, confirmed);
    } else {
        return internal::Ethereum::Blank().AddIncoming(
            balance, txid, confirmed);
    }
}

auto Ethereum::AddIncoming(
    const block::TransactionHash& txid,
    bool confirmed) noexcept -> bool
{
    if (auto p = imp_.lock(); p) {
        return p->asImported().asEthereum().AddIncoming(txid, confirmed);
    } else {
        return internal::Ethereum::Blank().AddIncoming(txid, confirmed);
    }
}

auto Ethereum::AddOutgoing(
    const Amount& balance,
    protocol::ethereum::AccountNonce nonce,
    const block::TransactionHash& txid,
    bool confirmed) noexcept -> bool
{
    if (auto p = imp_.lock(); p) {
        return p->asImported().asEthereum().AddOutgoing(
            balance, nonce, txid, confirmed);
    } else {
        return internal::Ethereum::Blank().AddOutgoing(
            balance, nonce, txid, confirmed);
    }
}

auto Ethereum::AddOutgoing(
    protocol::ethereum::AccountNonce nonce,
    const block::TransactionHash& txid,
    bool confirmed) noexcept -> bool
{
    if (auto p = imp_.lock(); p) {
        return p->asImported().asEthereum().AddOutgoing(nonce, txid, confirmed);
    } else {
        return internal::Ethereum::Blank().AddOutgoing(nonce, txid, confirmed);
    }
}

auto Ethereum::Balance() const noexcept -> Amount
{
    if (auto p = imp_.lock(); p) {
        return p->asImported().asEthereum().Balance();
    } else {
        return internal::Ethereum::Blank().Balance();
    }
}

auto Ethereum::Blank() noexcept -> Ethereum&
{
    static auto blank = Ethereum{std::make_shared<internal::Ethereum>()};

    return blank;
}

auto Ethereum::KnownIncoming(alloc::Strategy alloc) const noexcept
    -> Set<block::TransactionHash>
{
    if (auto p = imp_.lock(); p) {
        return p->asImported().asEthereum().KnownIncoming(alloc);
    } else {
        return internal::Ethereum::Blank().KnownIncoming(alloc);
    }
}

auto Ethereum::KnownOutgoing(alloc::Strategy alloc) const noexcept
    -> Set<block::TransactionHash>
{
    if (auto p = imp_.lock(); p) {
        return p->asImported().asEthereum().KnownOutgoing(alloc);
    } else {
        return internal::Ethereum::Blank().KnownOutgoing(alloc);
    }
}

auto Ethereum::MissingOutgoing(alloc::Strategy alloc) const noexcept
    -> Set<protocol::ethereum::AccountNonce>
{
    if (auto p = imp_.lock(); p) {
        return p->asImported().asEthereum().MissingOutgoing(alloc);
    } else {
        return internal::Ethereum::Blank().MissingOutgoing(alloc);
    }
}

auto Ethereum::NextOutgoing() const noexcept -> protocol::ethereum::AccountNonce
{
    if (auto p = imp_.lock(); p) {
        return p->asImported().asEthereum().NextOutgoing();
    } else {
        return internal::Ethereum::Blank().NextOutgoing();
    }
}

auto Ethereum::UpdateBalance(const Amount& balance) noexcept -> bool
{
    if (auto p = imp_.lock(); p) {
        return p->asImported().asEthereum().UpdateBalance(balance);
    } else {
        return internal::Ethereum::Blank().UpdateBalance(balance);
    }
}

Ethereum::~Ethereum() = default;
}  // namespace opentxs::blockchain::crypto
