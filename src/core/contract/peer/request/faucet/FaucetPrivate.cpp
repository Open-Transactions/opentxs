// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/peer/request/faucet/FaucetPrivate.hpp"  // IWYU pragma: associated

#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep

namespace opentxs::contract::peer::request
{
FaucetPrivate::FaucetPrivate(allocator_type alloc) noexcept
    : RequestPrivate(alloc)
{
}

FaucetPrivate::FaucetPrivate(
    const FaucetPrivate& rhs,
    allocator_type alloc) noexcept
    : RequestPrivate(rhs, alloc)
{
}

auto FaucetPrivate::Currency() const noexcept -> opentxs::UnitType
{
    return {};
}

auto FaucetPrivate::Instructions() const noexcept -> std::string_view
{
    return {};
}

auto FaucetPrivate::Type() const noexcept -> RequestType
{
    return RequestType::Faucet;
}

FaucetPrivate::~FaucetPrivate() = default;
}  // namespace opentxs::contract::peer::request
