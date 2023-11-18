// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/network/asio/Endpoint.hpp"
// IWYU pragma: no_include "opentxs/network/asio/Socket.hpp"

#pragma once

#include <future>
#include <memory>
#include <string_view>

#include "opentxs/api/network/Asio.hpp"
#include "opentxs/network/asio/Endpoint.hpp"
#include "opentxs/network/asio/Socket.hpp"
#include "opentxs/util/Types.hpp"
#include "util/Work.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace boost
{
namespace asio
{
class io_context;
}  // namespace asio

namespace json
{
class value;
}  // namespace json
}  // namespace boost

namespace opentxs
{
namespace api
{
namespace internal
{
class Context;
}  // namespace internal
}  // namespace api

namespace network
{
namespace asio
{
class Endpoint;
class Socket;
}  // namespace asio

namespace zeromq
{
class Envelope;
}  // namespace zeromq
}  // namespace network

class Timer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::network::internal
{
class Asio : virtual public network::Asio
{
public:
    using Endpoint = opentxs::network::asio::Endpoint;
    using Socket = opentxs::network::asio::Socket;
    using AcceptCallback = std::function<void(Socket&&)>;
    using SocketImp = std::shared_ptr<opentxs::network::asio::Socket::Imp>;
    using Callback = std::function<void()>;

    /**  Receive incoming tcp and udp connections
     *
     *   Calling this function will instruct the operating system to monitor a
     *   specified endpoint for incoming connection requests.
     *
     *   Once a connection request has been processed Asio will generate a
     *   socket and deliver it to the caller via the provided callback
     *
     *   @param endpoint the address / port which will be monitored for incoming
     *                   connection requests. \warning The caller must ensure
     *                   the lifetime of the endpoint lasts until Close() has
     *                   been called
     *   @param cb the callback function which will be executed to deliver a
     *             newly created socket once an incoming connection request has
     *             been received
     *
     *   \returns true if the operating system accepts the request to set up
     *            incoming connection handling on the specified socket
     */
    virtual auto Accept(const Endpoint& endpoint, AcceptCallback cb)
        const noexcept -> bool = 0;
    virtual auto Close(const Endpoint& endpoint) const noexcept -> bool = 0;
    virtual auto Connect(
        const opentxs::network::zeromq::Envelope& id,
        SocketImp socket) const noexcept -> bool = 0;
    virtual auto FetchJson(
        const ReadView host,
        const ReadView path,
        const bool https = true,
        const ReadView notify = {}) const noexcept
        -> std::future<boost::json::value> = 0;
    virtual auto GetTimer() const noexcept -> Timer = 0;
    auto Internal() const noexcept -> const internal::Asio& final
    {
        return *this;
    }
    virtual auto IOContext() const noexcept -> boost::asio::io_context& = 0;
    /**  Construct a socket for outgoing tcp and udp connections
     *
     *   @param endpoint the address / port to which an outgoing connection will
     *                   be created \warning The caller must ensure the lifetime
     *                   of the endpoint exceeds the lifetime of the
     *                   socket
     */
    virtual auto MakeSocket(const Endpoint& endpoint) const noexcept
        -> Socket = 0;
    /**  Endpoint for asio to zeromq message routing (null terminated)
     *
     *   This class maintained a zeromq router socket which is bound to the
     *   endpoint specified by this function.
     *
     *   After connecting to this endpoint with a zeromq dealer socket, callers
     *   should send an AsioRegister message as described in util/WorkType.hpp
     *
     *   The sequence of bytes received as the payload of the AsioRegister
     *   response is the value that must be provided to the
     *   asio::Socket::Connect and asio::Socket::Receive functions
     */
    virtual auto NotificationEndpoint() const noexcept -> std::string_view = 0;
    virtual auto Receive(
        const opentxs::network::zeromq::Envelope& id,
        const OTZMQWorkType type,
        const std::size_t bytes,
        SocketImp socket) const noexcept -> bool = 0;
    virtual auto Transmit(
        const opentxs::network::zeromq::Envelope& id,
        const ReadView bytes,
        SocketImp socket) const noexcept -> bool = 0;

    virtual auto Init(
        std::shared_ptr<const api::internal::Context> context) noexcept
        -> void = 0;
    auto Internal() noexcept -> internal::Asio& final { return *this; }
    virtual auto Shutdown() noexcept -> void = 0;

    Asio(const Asio&) = delete;
    Asio(Asio&&) = delete;
    auto operator=(const Asio&) -> Asio& = delete;
    auto operator=(Asio&&) -> Asio& = delete;

    ~Asio() override = default;

protected:
    Asio() = default;
};
}  // namespace opentxs::api::network::internal
