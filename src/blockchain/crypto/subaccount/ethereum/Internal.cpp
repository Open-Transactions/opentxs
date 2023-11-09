// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/crypto/Ethereum.hpp"  // IWYU pragma: associated

#include "opentxs/core/Amount.hpp"

namespace opentxs::blockchain::crypto::internal
{
auto Ethereum::Balance() const noexcept -> Amount { return {}; }

auto Ethereum::Blank() noexcept -> Ethereum&
{
    static auto blank = Ethereum{};

    return blank;
}

auto Ethereum::KnownTransactions(alloc::Strategy alloc) const noexcept
    -> Set<protocol::ethereum::AccountNonce>
{
    return Set<protocol::ethereum::AccountNonce>{alloc.result_};
}

auto Ethereum::MissingTransactions(alloc::Strategy alloc) const noexcept
    -> Set<protocol::ethereum::AccountNonce>
{
    return Set<protocol::ethereum::AccountNonce>{alloc.result_};
}

auto Ethereum::NextNonce() const noexcept -> protocol::ethereum::AccountNonce
{
    return {};
}

auto Ethereum::UpdateBalance(const Amount&, protocol::ethereum::AccountNonce)
    const noexcept -> bool
{
    return {};
}
}  // namespace opentxs::blockchain::crypto::internal
