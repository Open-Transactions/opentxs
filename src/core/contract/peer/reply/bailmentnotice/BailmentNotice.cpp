// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/core/contract/peer/reply/BailmentNotice.hpp"  // IWYU pragma: associated

#include <utility>

#include "core/contract/peer/reply/bailmentnotice/BailmentNoticePrivate.hpp"
#include "core/contract/peer/reply/base/ReplyPrivate.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::contract::peer::reply
{
BailmentNotice::BailmentNotice(ReplyPrivate* imp) noexcept
    : Reply(imp)
{
}

BailmentNotice::BailmentNotice(allocator_type alloc) noexcept
    : BailmentNotice(BailmentNoticePrivate::Blank(alloc))
{
}

BailmentNotice::BailmentNotice(
    const BailmentNotice& rhs,
    allocator_type alloc) noexcept
    : Reply(rhs, alloc)
{
}

BailmentNotice::BailmentNotice(BailmentNotice&& rhs) noexcept
    : Reply(std::move(rhs))
{
}

BailmentNotice::BailmentNotice(
    BailmentNotice&& rhs,
    allocator_type alloc) noexcept
    : Reply(std::move(rhs), alloc)
{
}

auto BailmentNotice::Blank() noexcept -> BailmentNotice&
{
    static auto blank = BailmentNotice{allocator_type{alloc::Default()}};

    return blank;
}

auto BailmentNotice::IsValid() const noexcept -> bool
{
    using enum RequestType;

    return imp_->IsValid() && (Type() == PendingBailment);
}

auto BailmentNotice::operator=(const BailmentNotice& rhs) noexcept
    -> BailmentNotice&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::copy_assign_child<Reply>(*this, rhs);
}

auto BailmentNotice::operator=(BailmentNotice&& rhs) noexcept -> BailmentNotice&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::move_assign_child<Reply>(*this, std::move(rhs));
}

auto BailmentNotice::Value() const noexcept -> bool
{
    return imp_->asBailmentNoticePrivate()->Value();
}

BailmentNotice::~BailmentNotice() = default;
}  // namespace opentxs::contract::peer::reply
