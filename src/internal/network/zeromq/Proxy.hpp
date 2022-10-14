// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "internal/util/Pimpl.hpp"

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
class Proxy;
}  // namespace zeromq
}  // namespace network

using OTZMQProxy = Pimpl<network::zeromq::Proxy>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq
{
class Proxy
{
public:
    static auto Factory(
        const Context& context,
        socket::Socket& frontend,
        socket::Socket& backend,
        const std::string_view threadname = {}) -> OTZMQProxy;

    Proxy(const Proxy&) = delete;
    Proxy(Proxy&&) = default;
    auto operator=(const Proxy&) -> Proxy& = delete;
    auto operator=(Proxy&&) -> Proxy& = default;

    virtual ~Proxy() = default;

protected:
    Proxy() = default;

private:
    friend OTZMQProxy;

    virtual auto clone() const -> Proxy* = 0;
};
}  // namespace opentxs::network::zeromq
