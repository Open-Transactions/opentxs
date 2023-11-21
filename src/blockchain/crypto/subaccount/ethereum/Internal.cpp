// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/crypto/Ethereum.hpp"  // IWYU pragma: associated

#include "opentxs/core/Amount.hpp"

namespace opentxs::blockchain::crypto::internal
{
auto Ethereum::AddIncoming(
    const Amount&,
    const block::TransactionHash&,
    bool) noexcept -> bool
{
    return {};
}

auto Ethereum::AddIncoming(const block::TransactionHash&, bool) noexcept -> bool
{
    return {};
}

auto Ethereum::AddOutgoing(
    const Amount&,
    protocol::ethereum::AccountNonce,
    const block::TransactionHash&,
    bool) noexcept -> bool
{
    return {};
}

auto Ethereum::AddOutgoing(
    protocol::ethereum::AccountNonce,
    const block::TransactionHash&,
    bool) noexcept -> bool
{
    return {};
}

auto Ethereum::Balance() const noexcept -> Amount { return {}; }

auto Ethereum::Blank() noexcept -> Ethereum&
{
    static auto blank = Ethereum{};

    return blank;
}

auto Ethereum::KnownIncoming(alloc::Strategy alloc) const noexcept
    -> Set<block::TransactionHash>
{
    return Set<block::TransactionHash>{alloc.result_};
}

auto Ethereum::KnownOutgoing(alloc::Strategy alloc) const noexcept
    -> Set<block::TransactionHash>
{
    return Set<block::TransactionHash>{alloc.result_};
}

auto Ethereum::MissingOutgoing(alloc::Strategy alloc) const noexcept
    -> Set<protocol::ethereum::AccountNonce>
{
    return Set<protocol::ethereum::AccountNonce>{alloc.result_};
}

auto Ethereum::NextOutgoing() const noexcept -> protocol::ethereum::AccountNonce
{
    return {};
}

auto Ethereum::UpdateBalance(const Amount&) noexcept -> bool { return {}; }
}  // namespace opentxs::blockchain::crypto::internal
