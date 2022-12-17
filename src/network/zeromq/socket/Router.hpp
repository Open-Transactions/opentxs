// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <ostream>
#include <string_view>

#include "internal/network/zeromq/socket/Router.hpp"
#include "internal/network/zeromq/socket/Socket.hpp"
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/util/Mutex.hpp"
#include "network/zeromq/curve/Client.hpp"
#include "network/zeromq/curve/Server.hpp"
#include "network/zeromq/socket/Bidirectional.hpp"
#include "network/zeromq/socket/Receiver.tpp"
#include "network/zeromq/socket/Socket.hpp"
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
class Router;
}  // namespace socket

class Context;
class ListenCallback;
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::socket::implementation
{
class Router final : public Bidirectional<zeromq::socket::Router>,
                     public zeromq::curve::implementation::Client,
                     public zeromq::curve::implementation::Server
{
public:
    auto SetSocksProxy(const UnallocatedCString& proxy) const noexcept
        -> bool final
    {
        return set_socks_proxy(proxy);
    }

    Router(
        const zeromq::Context& context,
        const Direction direction,
        const zeromq::ListenCallback& callback,
        const std::string_view threadname = "Router") noexcept;
    Router() = delete;
    Router(const Router&) = delete;
    Router(Router&&) = delete;
    auto operator=(const Router&) -> Router& = delete;
    auto operator=(Router&&) -> Router& = delete;

    ~Router() final;

private:
    const ListenCallback& callback_;

    auto clone() const noexcept -> Router* final;
    auto have_callback() const noexcept -> bool final { return true; }

    void process_incoming(const Lock& lock, Message&& message) noexcept final;
};
}  // namespace opentxs::network::zeromq::socket::implementation
