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

auto Ethereum::KnownTransactions(alloc::Strategy alloc) const noexcept
    -> Set<protocol::ethereum::AccountNonce>
{
    if (auto p = imp_.lock(); p) {
        return p->asImported().asEthereum().KnownTransactions(alloc);
    } else {
        return internal::Ethereum::Blank().KnownTransactions(alloc);
    }
}

auto Ethereum::MissingTransactions(alloc::Strategy alloc) const noexcept
    -> Set<protocol::ethereum::AccountNonce>
{
    if (auto p = imp_.lock(); p) {
        return p->asImported().asEthereum().MissingTransactions(alloc);
    } else {
        return internal::Ethereum::Blank().MissingTransactions(alloc);
    }
}

auto Ethereum::NextNonce() const noexcept -> protocol::ethereum::AccountNonce
{
    if (auto p = imp_.lock(); p) {
        return p->asImported().asEthereum().NextNonce();
    } else {
        return internal::Ethereum::Blank().NextNonce();
    }
}

auto Ethereum::UpdateBalance(
    const Amount& balance,
    protocol::ethereum::AccountNonce nonce) const noexcept -> bool
{
    if (auto p = imp_.lock(); p) {
        return p->asImported().asEthereum().UpdateBalance(balance, nonce);
    } else {
        return internal::Ethereum::Blank().UpdateBalance(balance, nonce);
    }
}

Ethereum::~Ethereum() = default;
}  // namespace opentxs::blockchain::crypto
