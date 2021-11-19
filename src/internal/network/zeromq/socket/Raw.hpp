// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/network/zeromq/Types.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Bytes.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class Raw;
}  // namespace socket

class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::network::zeromq::socket
{
auto swap(Raw& lhs, Raw& rhs) noexcept -> void;

class Raw
{
public:
    class Imp;

    auto ID() const noexcept -> SocketID;
    auto Type() const noexcept -> socket::Type;

    auto Bind(const char* endpoint) noexcept -> bool;
    auto ClearSubscriptions() noexcept -> bool;
    auto Close() noexcept -> void;
    auto Connect(const char* endpoint) noexcept -> bool;
    auto Disconnect(const char* endpoint) noexcept -> bool;
    auto DisconnectAll() noexcept -> bool;
    auto Native() noexcept -> void*;
    auto Send(Message&& msg) noexcept -> bool;
    auto SetPrivateKey(ReadView key) noexcept -> bool;
    auto SetZAPDomain(ReadView domain) noexcept -> bool;
    auto Stop() noexcept -> void;
    auto swap(Raw& other) noexcept -> void;
    auto Unbind(const char* endpoint) noexcept -> bool;
    auto UnbindAll() noexcept -> bool;

    Raw(Imp* imp) noexcept;
    Raw(Raw&&) noexcept;
    auto operator=(Raw&&) noexcept -> Raw&;

    ~Raw();

private:
    Imp* imp_;

    Raw() = delete;
    Raw(const Raw&) = delete;
    auto operator=(const Raw&) -> Raw& = delete;
};
}  // namespace opentxs::network::zeromq::socket