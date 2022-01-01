// Copyright (c) 2010-2021 The Open-Transactions developers
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
#include <vector>

#include "opentxs/core/Data.hpp"

namespace opentxs
{
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
}  // namespace opentxs

namespace opentxs::api::network
{
class OPENTXS_EXPORT Asio
{
public:
    using Endpoint = opentxs::network::asio::Endpoint;
    using Socket = opentxs::network::asio::Socket;
    using Resolved = std::pmr::vector<Endpoint>;
    using AcceptCallback = std::function<void(Socket&&)>;

    // NOTE: endpoint must remain valid until Close is called
    auto Accept(const Endpoint& endpoint, AcceptCallback cb) const noexcept
        -> bool;
    auto Close(const Endpoint& endpoint) const noexcept -> bool;
    auto GetPublicAddress4() const noexcept -> std::shared_future<OTData>;
    auto GetPublicAddress6() const noexcept -> std::shared_future<OTData>;
    OPENTXS_NO_EXPORT auto Internal() const noexcept -> internal::Asio&;
    auto MakeSocket(const Endpoint& endpoint) const noexcept -> Socket;
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
