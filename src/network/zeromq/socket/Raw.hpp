// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <cstddef>
#include <memory>

#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"
#include "util/ByteLiterals.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::socket
{
using namespace std::literals;

class Raw::Imp
{
public:
    const SocketID id_{GetSocketID()};

    auto ID() const noexcept -> SocketID { return id_; }
    virtual auto Type() const noexcept -> socket::Type
    {
        return socket::Type::Error;
    }
    virtual auto Bind(const char*) noexcept -> bool { return {}; }
    virtual auto ClearSubscriptions() noexcept -> bool { return {}; }
    virtual auto Close() noexcept -> void {}
    virtual auto Connect(const char*) noexcept -> bool { return {}; }
    virtual auto Disconnect(const char*) noexcept -> bool { return {}; }
    virtual auto DisconnectAll() noexcept -> bool { return {}; }
    virtual auto EnableCurveClient(
        const ReadView,
        const ReadView,
        const ReadView) noexcept -> bool
    {
        return {};
    }
    virtual auto EnableCurveServer(const ReadView) noexcept -> bool
    {
        return {};
    }
    virtual auto Native() noexcept -> void* { return nullptr; }
    virtual auto Send(Message&&, const char*, int, bool silent) noexcept -> bool
    {
        return {};
    }
    virtual auto SendDeferred(Message&&, const char*, int, bool) noexcept
        -> bool
    {
        return {};
    }
    virtual auto SendExternal(Message&&, const char*, int, bool) noexcept
        -> bool
    {
        return {};
    }
    virtual auto SetExposedUntrusted() noexcept -> bool { return false; }
    virtual auto SetIncomingHWM(int) noexcept -> bool { return {}; }
    virtual auto SetLinger(int) noexcept -> bool { return {}; }
    virtual auto SetMaxMessageSize(std::size_t) noexcept -> bool { return {}; }
    virtual auto SetMonitor(const char*, int) noexcept -> bool { return {}; }
    virtual auto SetOutgoingHWM(int) noexcept -> bool { return {}; }
    virtual auto SetPrivateKey(ReadView) noexcept -> bool { return {}; }
    virtual auto SetRouterHandover(bool) noexcept -> bool { return {}; }
    virtual auto SetRoutingID(ReadView) noexcept -> bool { return {}; }
    virtual auto SetSendTimeout(std::chrono::milliseconds) noexcept -> bool
    {
        return {};
    }
    virtual auto SetZAPDomain(ReadView) noexcept -> bool { return {}; }
    virtual auto Stop() noexcept -> void {}
    virtual auto Unbind(const char*) noexcept -> bool { return {}; }
    virtual auto UnbindAll() noexcept -> bool { return {}; }
    virtual auto WaitForSend() noexcept -> bool { return false; }

    Imp() = default;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;

    virtual ~Imp() = default;
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
    auto EnableCurveClient(
        const ReadView serverKey,
        const ReadView publicKey,
        const ReadView secretKey) noexcept -> bool final;
    auto EnableCurveServer(const ReadView secretKey) noexcept -> bool final;
    auto Native() noexcept -> void* final { return socket_.get(); }
    auto Send(Message&& msg, const char* file, int line, bool silent) noexcept
        -> bool final;
    auto SendDeferred(
        Message&& msg,
        const char* file,
        int line,
        bool silent) noexcept -> bool final;
    auto SendExternal(
        Message&& msg,
        const char* file,
        int line,
        bool silent) noexcept -> bool final;
    auto SetExposedUntrusted() noexcept -> bool final;
    auto SetIncomingHWM(int value) noexcept -> bool final;
    auto SetLinger(int value) noexcept -> bool final;
    auto SetMaxMessageSize(std::size_t bytes) noexcept -> bool final;
    auto SetMonitor(const char* endpoint, int events) noexcept -> bool final;
    auto SetOutgoingHWM(int value) noexcept -> bool final;
    auto SetPrivateKey(ReadView key) noexcept -> bool final;
    auto SetRouterHandover(bool value) noexcept -> bool final;
    auto SetRoutingID(ReadView key) noexcept -> bool final;
    auto SetSendTimeout(std::chrono::milliseconds value) noexcept -> bool final;
    auto SetZAPDomain(ReadView domain) noexcept -> bool final;
    auto Stop() noexcept -> void final;
    auto Unbind(const char* endpoint) noexcept -> bool final;
    auto UnbindAll() noexcept -> bool final;
    auto WaitForSend() noexcept -> bool final;

    Raw(const Context& context, const socket::Type type) noexcept;
    Raw() = delete;
    Raw(const Raw&) = delete;
    Raw(Raw&&) = delete;
    auto operator=(const Raw&) -> Raw& = delete;
    auto operator=(Raw&&) -> Raw& = delete;

    ~Raw() final;

private:
    using Socket = std::unique_ptr<void, decltype(&zmq_close_wrapper)>;
    using Endpoints = UnallocatedSet<UnallocatedCString>;

    static constexpr auto default_hwm_ = int{0};
    static constexpr auto untrusted_hwm_ = int{1024};
    static constexpr auto untrusted_max_message_size_ = std::size_t{32_mib};
    static constexpr auto default_send_timeout_ = 0ms;

    const socket::Type type_;
    Socket socket_;
    Endpoints bound_endpoints_;
    Endpoints connected_endpoints_;

    auto record_endpoint(Endpoints& out) noexcept -> void;
    auto send(
        Message&& msg,
        const int flags,
        const char* file,
        int line,
        bool silent) noexcept -> bool;
    auto wait(int flags) noexcept -> bool;
};
}  // namespace opentxs::network::zeromq::socket::implementation
