// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/peer/reply/faucet/FaucetPrivate.hpp"  // IWYU pragma: associated

#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep

namespace opentxs::contract::peer::reply
{
FaucetPrivate::FaucetPrivate(allocator_type alloc) noexcept
    : ReplyPrivate(alloc)
{
}

FaucetPrivate::FaucetPrivate(
    const FaucetPrivate& rhs,
    allocator_type alloc) noexcept
    : ReplyPrivate(rhs, alloc)
{
}

auto FaucetPrivate::Accepted() const noexcept -> bool { return {}; }

auto FaucetPrivate::Transaction() const noexcept
    -> const blockchain::block::Transaction&
{
    static const auto blank = blockchain::block::Transaction{alloc::System()};

    return blank;
}

auto FaucetPrivate::Type() const noexcept -> RequestType
{
    return RequestType::Faucet;
}

FaucetPrivate::~FaucetPrivate() = default;
}  // namespace opentxs::contract::peer::reply
