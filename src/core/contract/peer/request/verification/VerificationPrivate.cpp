// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/peer/request/verification/VerificationPrivate.hpp"  // IWYU pragma: associated

#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep
#include "opentxs/identity/wot/Claim.hpp"

namespace opentxs::contract::peer::request
{
VerificationPrivate::VerificationPrivate(allocator_type alloc) noexcept
    : RequestPrivate(alloc)
{
}

VerificationPrivate::VerificationPrivate(
    const VerificationPrivate& rhs,
    allocator_type alloc) noexcept
    : RequestPrivate(rhs, alloc)
{
}

auto VerificationPrivate::Claim() const noexcept -> const identity::wot::Claim&
{
    static const auto claim = identity::wot::Claim{};

    return claim;
}

auto VerificationPrivate::Type() const noexcept -> RequestType
{
    return RequestType::Verification;
}

VerificationPrivate::~VerificationPrivate() = default;
}  // namespace opentxs::contract::peer::request
