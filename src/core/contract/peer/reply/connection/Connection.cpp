// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/core/contract/peer/reply/Connection.hpp"  // IWYU pragma: associated

#include <string_view>
#include <utility>

#include "core/contract/peer/reply/base/ReplyPrivate.hpp"
#include "core/contract/peer/reply/connection/ConnectionPrivate.hpp"
#include "opentxs/core/contract/peer/RequestType.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/peer/Types.hpp"
#include "opentxs/util/Allocator.hpp"

namespace opentxs::contract::peer::reply
{
Connection::Connection(ReplyPrivate* imp) noexcept
    : Reply(imp)
{
}

Connection::Connection(allocator_type alloc) noexcept
    : Connection(ConnectionPrivate::Blank(alloc))
{
}

Connection::Connection(const Connection& rhs, allocator_type alloc) noexcept
    : Reply(rhs, alloc)
{
}

Connection::Connection(Connection&& rhs) noexcept
    : Reply(std::move(rhs))
{
}

Connection::Connection(Connection&& rhs, allocator_type alloc) noexcept
    : Reply(std::move(rhs), alloc)
{
}

auto Connection::Accepted() const noexcept -> bool
{
    return imp_->asConnectionPrivate()->Accepted();
}

auto Connection::Blank() noexcept -> Connection&
{
    static auto blank = Connection{allocator_type{alloc::Default()}};

    return blank;
}

auto Connection::Endpoint() const noexcept -> std::string_view
{
    return imp_->asConnectionPrivate()->Endpoint();
}

auto Connection::IsValid() const noexcept -> bool
{
    using enum RequestType;

    return imp_->IsValid() && (Type() == ConnectionInfo);
}

auto Connection::Key() const noexcept -> std::string_view
{
    return imp_->asConnectionPrivate()->Key();
}

auto Connection::Login() const noexcept -> std::string_view
{
    return imp_->asConnectionPrivate()->Login();
}

// NOLINTBEGIN(modernize-use-equals-default)
auto Connection::operator=(const Connection& rhs) noexcept -> Connection&
{
    Reply::operator=(rhs);

    return *this;
}

auto Connection::operator=(Connection&& rhs) noexcept -> Connection&
{
    Reply::operator=(std::move(rhs));

    return *this;
}
// NOLINTEND(modernize-use-equals-default)

auto Connection::Password() const noexcept -> std::string_view
{
    return imp_->asConnectionPrivate()->Password();
}

Connection::~Connection() = default;
}  // namespace opentxs::contract::peer::reply
