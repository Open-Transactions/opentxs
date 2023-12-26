// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/core/contract/peer/reply/Connection.hpp"  // IWYU pragma: associated

#include <utility>

#include "core/contract/peer/request/base/RequestPrivate.hpp"
#include "core/contract/peer/request/connection/ConnectionPrivate.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/contract/peer/Request.hpp"
#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/core/contract/peer/request/Connection.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::contract::peer::request
{
Connection::Connection(RequestPrivate* imp) noexcept
    : Request(imp)
{
}

Connection::Connection(allocator_type alloc) noexcept
    : Connection(ConnectionPrivate::Blank(alloc))
{
}

Connection::Connection(const Connection& rhs, allocator_type alloc) noexcept
    : Request(rhs, alloc)
{
}

Connection::Connection(Connection&& rhs) noexcept
    : Request(std::move(rhs))
{
}

Connection::Connection(Connection&& rhs, allocator_type alloc) noexcept
    : Request(std::move(rhs), alloc)
{
}

auto Connection::Blank() noexcept -> Connection&
{
    static auto blank = Connection{allocator_type{alloc::Default()}};

    return blank;
}

auto Connection::IsValid() const noexcept -> bool
{
    using enum RequestType;

    return imp_->IsValid() && (Type() == ConnectionInfo);
}

auto Connection::Kind() const noexcept -> ConnectionInfoType
{
    return imp_->asConnectionPrivate()->Kind();
}

auto Connection::operator=(const Connection& rhs) noexcept -> Connection&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::copy_assign_child<Request>(*this, rhs);
}

auto Connection::operator=(Connection&& rhs) noexcept -> Connection&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::move_assign_child<Request>(*this, std::move(rhs));
}

Connection::~Connection() = default;
}  // namespace opentxs::contract::peer::request
