// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/peer/reply/bailmentnotice/BailmentNoticePrivate.hpp"  // IWYU pragma: associated

#include "opentxs/core/Amount.hpp"
#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep

namespace opentxs::contract::peer::reply
{
BailmentNoticePrivate::BailmentNoticePrivate(allocator_type alloc) noexcept
    : ReplyPrivate(alloc)
{
}

BailmentNoticePrivate::BailmentNoticePrivate(
    const BailmentNoticePrivate& rhs,
    allocator_type alloc) noexcept
    : ReplyPrivate(rhs, alloc)
{
}

auto BailmentNoticePrivate::Amount() const noexcept -> opentxs::Amount
{
    return {};
}

auto BailmentNoticePrivate::Type() const noexcept -> RequestType
{
    return RequestType::PendingBailment;
}

auto BailmentNoticePrivate::Value() const noexcept -> bool { return {}; }

BailmentNoticePrivate::~BailmentNoticePrivate() = default;
}  // namespace opentxs::contract::peer::reply
