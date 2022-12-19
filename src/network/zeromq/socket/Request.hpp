// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/network/zeromq/socket/Request.hpp"

#include "internal/util/Mutex.hpp"
#include "network/zeromq/curve/Client.hpp"
#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
class Context;
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::socket::implementation
{
class Request final : virtual public zeromq::socket::Request,
                      public Socket,
                      public zeromq::curve::implementation::Client
{
public:
    auto Send(zeromq::Message&& message) const noexcept -> SendResult final;
    auto SetSocksProxy(const UnallocatedCString& proxy) const noexcept
        -> bool final;

    Request(const zeromq::Context& context) noexcept;
    Request() = delete;
    Request(const Request&) = delete;
    Request(Request&&) = delete;
    auto operator=(const Request&) -> Request& = delete;
    auto operator=(Request&&) -> Request& = delete;

    ~Request() final;

private:
    static constexpr auto poll_milliseconds_{10};

    auto clone() const noexcept -> Request* final;
    auto wait(const Lock& lock) const noexcept -> bool;
};
}  // namespace opentxs::network::zeromq::socket::implementation
