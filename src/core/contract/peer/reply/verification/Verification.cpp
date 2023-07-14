// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/core/contract/peer/reply/Verification.hpp"  // IWYU pragma: associated

#include <utility>

#include "core/contract/peer/reply/base/ReplyPrivate.hpp"
#include "core/contract/peer/reply/verification/VerificationPrivate.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::contract::peer::reply
{
Verification::Verification(ReplyPrivate* imp) noexcept
    : Reply(imp)
{
}

Verification::Verification(allocator_type alloc) noexcept
    : Verification(VerificationPrivate::Blank(alloc))
{
}

Verification::Verification(
    const Verification& rhs,
    allocator_type alloc) noexcept
    : Reply(rhs, alloc)
{
}

Verification::Verification(Verification&& rhs) noexcept
    : Reply(std::move(rhs))
{
}

Verification::Verification(Verification&& rhs, allocator_type alloc) noexcept
    : Reply(std::move(rhs), alloc)
{
}

auto Verification::Accepted() const noexcept -> bool
{
    return imp_->asVerificationPrivate()->Accepted();
}

auto Verification::Blank() noexcept -> Verification&
{
    static auto blank = Verification{allocator_type{alloc::Default()}};

    return blank;
}

auto Verification::IsValid() const noexcept -> bool
{
    using enum RequestType;

    return imp_->IsValid() && (Type() == Verification);
}

auto Verification::operator=(const Verification& rhs) noexcept -> Verification&
{
    return copy_assign_child<Reply>(*this, rhs);
}

auto Verification::operator=(Verification&& rhs) noexcept -> Verification&
{
    return move_assign_child<Reply>(*this, std::move(rhs));
}

auto Verification::Response() const noexcept
    -> const std::optional<identity::wot::Verification>&
{
    return imp_->asVerificationPrivate()->Response();
}

Verification::~Verification() = default;
}  // namespace opentxs::contract::peer::reply
