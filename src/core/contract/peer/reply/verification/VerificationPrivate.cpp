// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/peer/reply/verification/VerificationPrivate.hpp"  // IWYU pragma: associated

#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep
#include "opentxs/identity/wot/Verification.hpp"

namespace opentxs::contract::peer::reply
{
VerificationPrivate::VerificationPrivate(allocator_type alloc) noexcept
    : ReplyPrivate(alloc)
{
}

VerificationPrivate::VerificationPrivate(
    const VerificationPrivate& rhs,
    allocator_type alloc) noexcept
    : ReplyPrivate(rhs, alloc)
{
}

auto VerificationPrivate::Accepted() const noexcept -> bool { return {}; }

auto VerificationPrivate::Response() const noexcept
    -> const std::optional<identity::wot::Verification>&
{
    static const auto blank = std::optional<identity::wot::Verification>{};

    return blank;
}

auto VerificationPrivate::Type() const noexcept -> RequestType
{
    return RequestType::Verification;
}

VerificationPrivate::~VerificationPrivate() = default;
}  // namespace opentxs::contract::peer::reply
