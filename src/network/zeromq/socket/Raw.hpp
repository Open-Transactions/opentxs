// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <zmq.h>
#include <set>
#include <string>

#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"

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

class Context;
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

namespace opentxs::network::zeromq::socket
{
class Raw::Imp
{
public:
    const SocketID id_{GetSocketID()};

    auto ID() const noexcept -> SocketID { return id_; }
    virtual auto Type() const noexcept -> socket::Type
    {
        return socket::Type::Error;
    }
    virtual auto Bind(const char* endpoint) noexcept -> bool { return {}; }
    virtual auto ClearSubscriptions() noexcept -> bool { return {}; }
    virtual auto Close() noexcept -> void {}
    virtual auto Connect(const char* endpoint) noexcept -> bool { return {}; }
    virtual auto Disconnect(const char* endpoint) noexcept -> bool
    {
        return {};
    }
    virtual auto DisconnectAll() noexcept -> bool { return {}; }
    virtual auto Native() noexcept -> void* { return nullptr; }
    virtual auto Send(Message&& msg) noexcept -> bool { return {}; }
    virtual auto SetPrivateKey(ReadView) noexcept -> bool { return {}; }
    virtual auto SetRoutingID(ReadView) noexcept -> bool { return {}; }
    virtual auto SetZAPDomain(ReadView) noexcept -> bool { return {}; }
    virtual auto Stop() noexcept -> void {}
    virtual auto Unbind(const char* endpoint) noexcept -> bool { return {}; }
    virtual auto UnbindAll() noexcept -> bool { return {}; }

    Imp() = default;

    virtual ~Imp() = default;

private:
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;
};
}  // namespace opentxs::network::zeromq::socket

namespace opentxs::network::zeromq::socket::implementation
{
class Raw final : public socket::Raw::Imp
{
public:
    auto Type() const noexcept -> socket::Type final { return type_; }

    auto Bind(const char* endpoint) noexcept -> bool final;
    auto ClearSubscriptions() noexcept -> bool final;
    auto Close() noexcept -> void final;
    auto Connect(const char* endpoint) noexcept -> bool final;
    auto Disconnect(const char* endpoint) noexcept -> bool final;
    auto DisconnectAll() noexcept -> bool final;
    auto Native() noexcept -> void* final { return socket_.get(); }
    auto Send(Message&& msg) noexcept -> bool final;
    auto SetPrivateKey(ReadView key) noexcept -> bool final;
    auto SetRoutingID(ReadView key) noexcept -> bool final;
    auto SetZAPDomain(ReadView domain) noexcept -> bool final;
    auto Stop() noexcept -> void final;
    auto Unbind(const char* endpoint) noexcept -> bool final;
    auto UnbindAll() noexcept -> bool final;

    Raw(const Context& context, const socket::Type type) noexcept;

    ~Raw() final;

private:
    using Socket = std::unique_ptr<void, decltype(&::zmq_close)>;
    using Endpoints = std::pmr::set<std::string>;

    const socket::Type type_;
    Socket socket_;
    Endpoints bound_endpoints_;
    Endpoints connected_endpoints_;

    auto record_endpoint(Endpoints& out) noexcept -> void;

    Raw() = delete;
    Raw(const Raw&) = delete;
    Raw(Raw&&) = delete;
    auto operator=(const Raw&) -> Raw& = delete;
    auto operator=(Raw&&) -> Raw& = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation
