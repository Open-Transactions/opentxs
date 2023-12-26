// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::identity::wot::Claim

#include "opentxs/core/contract/peer/reply/Verification.hpp"  // IWYU pragma: associated

#include <utility>

#include "core/contract/peer/request/base/RequestPrivate.hpp"
#include "core/contract/peer/request/verification/VerificationPrivate.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/contract/peer/Request.hpp"
#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/contract/peer/request/Verification.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::contract::peer::request
{
Verification::Verification(RequestPrivate* imp) noexcept
    : Request(imp)
{
}

Verification::Verification(allocator_type alloc) noexcept
    : Verification(VerificationPrivate::Blank(alloc))
{
}

Verification::Verification(
    const Verification& rhs,
    allocator_type alloc) noexcept
    : Request(rhs, alloc)
{
}

Verification::Verification(Verification&& rhs) noexcept
    : Request(std::move(rhs))
{
}

Verification::Verification(Verification&& rhs, allocator_type alloc) noexcept
    : Request(std::move(rhs), alloc)
{
}

auto Verification::Blank() noexcept -> Verification&
{
    static auto blank = Verification{allocator_type{alloc::Default()}};

    return blank;
}

auto Verification::Claim() const noexcept -> const identity::wot::Claim&
{
    return imp_->asVerificationPrivate()->Claim();
}

auto Verification::IsValid() const noexcept -> bool
{
    using enum RequestType;

    return imp_->IsValid() && (Type() == Verification);
}

auto Verification::operator=(const Verification& rhs) noexcept -> Verification&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::copy_assign_child<Request>(*this, rhs);
}

auto Verification::operator=(Verification&& rhs) noexcept -> Verification&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::move_assign_child<Request>(*this, std::move(rhs));
}

Verification::~Verification() = default;
}  // namespace opentxs::contract::peer::request
