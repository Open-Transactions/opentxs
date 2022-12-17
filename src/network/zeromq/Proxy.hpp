// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <string_view>
#include <thread>

#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/Proxy.hpp"
#include "internal/network/zeromq/socket/Pair.hpp"
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
class Socket;
}  // namespace socket

class Context;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::implementation
{
class Proxy final : virtual public network::zeromq::Proxy
{
public:
    Proxy() = delete;
    Proxy(const Proxy&) = delete;
    Proxy(Proxy&&) = delete;
    auto operator=(const Proxy&) -> Proxy& = delete;
    auto operator=(Proxy&&) -> Proxy& = delete;

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
    const CString thread_name_;

    auto clone() const -> Proxy* final;
    void proxy() const;

    Proxy(
        const zeromq::Context& context,
        zeromq::socket::Socket& frontend,
        zeromq::socket::Socket& backend,
        const std::string_view threadname = "Proxy");
};
}  // namespace opentxs::network::zeromq::implementation
