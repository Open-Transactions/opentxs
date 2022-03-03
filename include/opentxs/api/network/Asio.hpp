// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/network/asio/Endpoint.hpp"
// IWYU pragma: no_include "opentxs/network/asio/Socket.hpp"

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <functional>
#include <future>
#include <string_view>

#include "opentxs/core/Data.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace network
{
namespace internal
{
class Asio;
}  // namespace internal
}  // namespace network
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
class Context;
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::api::network
{
class OPENTXS_EXPORT Asio
{
public:
    using Endpoint = opentxs::network::asio::Endpoint;
    using Socket = opentxs::network::asio::Socket;
    using Resolved = UnallocatedVector<Endpoint>;
    using AcceptCallback = std::function<void(Socket&&)>;

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
    auto Accept(const Endpoint& endpoint, AcceptCallback cb) const noexcept
        -> bool;
    auto Close(const Endpoint& endpoint) const noexcept -> bool;
    auto GetPublicAddress4() const noexcept -> std::shared_future<OTData>;
    auto GetPublicAddress6() const noexcept -> std::shared_future<OTData>;
    OPENTXS_NO_EXPORT auto Internal() const noexcept -> internal::Asio&;

    /**  Construct a socket for outgoing tcp and udp connections
     *
     *   @param endpoint the address / port to which an outgoing connection will
     *                   be created \warning The caller must ensure the lifetime
     *                   of the endpoint exceeds the lifetime of the
     *                   socket
     */
    auto MakeSocket(const Endpoint& endpoint) const noexcept -> Socket;

    /**  Endpoint for asio to zeromq message routing
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
    auto NotificationEndpoint() const noexcept -> const char*;
    auto Resolve(std::string_view server, std::uint16_t port) const noexcept
        -> Resolved;

    OPENTXS_NO_EXPORT auto Init() noexcept -> void;
    OPENTXS_NO_EXPORT auto Shutdown() noexcept -> void;

    OPENTXS_NO_EXPORT Asio(
        const opentxs::network::zeromq::Context& zmq) noexcept;

    OPENTXS_NO_EXPORT ~Asio();

private:
    struct Imp;

    Imp* imp_;

    Asio() = delete;
    Asio(const Asio&) = delete;
    Asio(Asio&&) = delete;
    auto operator=(const Asio&) -> Asio& = delete;
    auto operator=(Asio&&) -> Asio& = delete;
};
}  // namespace opentxs::api::network
