// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/network/zeromq/curve/Client.hpp"
#include "internal/network/zeromq/curve/Server.hpp"
#include "internal/network/zeromq/socket/Sender.hpp"
#include "opentxs/util/Pimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class Router;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

using OTZMQRouterSocket = Pimpl<network::zeromq::socket::Router>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::socket
{
class Router : virtual public curve::Server,
               virtual public curve::Client,
               virtual public Sender
{
public:
    virtual auto SetSocksProxy(const UnallocatedCString& proxy) const noexcept
        -> bool = 0;

    Router(const Router&) = delete;
    Router(Router&&) = delete;
    auto operator=(const Router&) -> Router& = delete;
    auto operator=(Router&&) -> Router& = delete;

    ~Router() override = default;

protected:
    Router() noexcept = default;

private:
    friend OTZMQRouterSocket;

    virtual auto clone() const noexcept -> Router* = 0;
};
}  // namespace opentxs::network::zeromq::socket
