// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <cstdint>
#include <functional>
#include <memory>
#include <string_view>
#include <tuple>

#include "internal/network/zeromq/socket/Types.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
namespace internal
{
class Socket;
}  // namespace internal
}  // namespace socket

class Context;
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::socket
{
class Socket
{
public:
    using SendResult = std::pair<otx::client::SendResult, Message>;

    virtual operator void*() const noexcept = 0;

    virtual auto Close() const noexcept -> bool = 0;
    virtual auto Context() const noexcept -> const zeromq::Context& = 0;
    virtual auto Internal() const noexcept -> const internal::Socket& = 0;
    virtual auto SetTimeouts(
        const std::chrono::milliseconds& linger,
        const std::chrono::milliseconds& send,
        const std::chrono::milliseconds& receive) const noexcept -> bool = 0;
    // Do not call Start during callback execution
    virtual auto Start(const std::string_view endpoint) const noexcept
        -> bool = 0;
    // StartAsync version may be called during callback execution
    virtual auto StartAsync(const std::string_view endpoint) const noexcept
        -> void = 0;
    virtual auto Type() const noexcept -> socket::Type = 0;

    virtual auto Internal() noexcept -> internal::Socket& = 0;

    Socket(const Socket&) = delete;
    Socket(Socket&&) = default;
    auto operator=(const Socket&) -> Socket& = delete;
    auto operator=(Socket&&) -> Socket& = default;

    virtual ~Socket() = default;

protected:
    Socket() noexcept = default;
};
}  // namespace opentxs::network::zeromq::socket
