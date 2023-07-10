// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "core/contract/peer/request/connection/ConnectionPrivate.hpp"  // IWYU pragma: associated

#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep

namespace opentxs::contract::peer::request
{
ConnectionPrivate::ConnectionPrivate(allocator_type alloc) noexcept
    : RequestPrivate(alloc)
{
}

ConnectionPrivate::ConnectionPrivate(
    const ConnectionPrivate& rhs,
    allocator_type alloc) noexcept
    : RequestPrivate(rhs, alloc)
{
}

auto ConnectionPrivate::Kind() const noexcept -> ConnectionInfoType
{
    return {};
}

auto ConnectionPrivate::Type() const noexcept -> RequestType
{
    return RequestType::ConnectionInfo;
}

ConnectionPrivate::~ConnectionPrivate() = default;
}  // namespace opentxs::contract::peer::request
