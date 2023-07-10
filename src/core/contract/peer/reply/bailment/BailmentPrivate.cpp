// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/peer/reply/bailment/BailmentPrivate.hpp"  // IWYU pragma: associated

#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep

namespace opentxs::contract::peer::reply
{
BailmentPrivate::BailmentPrivate(allocator_type alloc) noexcept
    : ReplyPrivate(alloc)
{
}

BailmentPrivate::BailmentPrivate(
    const BailmentPrivate& rhs,
    allocator_type alloc) noexcept
    : ReplyPrivate(rhs, alloc)
{
}

auto BailmentPrivate::Instructions() const noexcept -> std::string_view
{
    return {};
}

auto BailmentPrivate::Type() const noexcept -> RequestType
{
    return RequestType::Bailment;
}

BailmentPrivate::~BailmentPrivate() = default;
}  // namespace opentxs::contract::peer::reply
