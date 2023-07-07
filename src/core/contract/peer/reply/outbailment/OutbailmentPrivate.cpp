// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/peer/reply/outbailment/OutbailmentPrivate.hpp"  // IWYU pragma: associated

#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep

namespace opentxs::contract::peer::reply
{
OutbailmentPrivate::OutbailmentPrivate(allocator_type alloc) noexcept
    : ReplyPrivate(alloc)
{
}

OutbailmentPrivate::OutbailmentPrivate(
    const OutbailmentPrivate& rhs,
    allocator_type alloc) noexcept
    : ReplyPrivate(rhs, alloc)
{
}

auto OutbailmentPrivate::Description() const noexcept -> std::string_view
{
    return {};
}

auto OutbailmentPrivate::Type() const noexcept -> RequestType
{
    return RequestType::OutBailment;
}

OutbailmentPrivate::~OutbailmentPrivate() = default;
}  // namespace opentxs::contract::peer::reply
