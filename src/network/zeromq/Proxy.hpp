// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <thread>

#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Proxy.hpp"
#include "opentxs/network/zeromq/socket/Pair.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace network
{
namespace zeromq
{
namespace socket
{
class Socket;
}  // namespace socket

class Context;
class Message;
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::implementation
{
class Proxy final : virtual public network::zeromq::Proxy
{
public:
    ~Proxy() final;

private:
    friend network::zeromq::Proxy;

    const zeromq::Context& context_;
    zeromq::socket::Socket& frontend_;
    zeromq::socket::Socket& backend_;
    OTZMQListenCallback null_callback_;
    OTZMQPairSocket control_listener_;
    OTZMQPairSocket control_sender_;
    std::unique_ptr<std::thread> thread_{nullptr};

    auto clone() const -> Proxy* final;
    void proxy() const;

    Proxy(
        const zeromq::Context& context,
        zeromq::socket::Socket& frontend,
        zeromq::socket::Socket& backend);
    Proxy() = delete;
    Proxy(const Proxy&) = delete;
    Proxy(Proxy&&) = delete;
    auto operator=(const Proxy&) -> Proxy& = delete;
    auto operator=(Proxy&&) -> Proxy& = delete;
};
}  // namespace opentxs::network::zeromq::implementation
