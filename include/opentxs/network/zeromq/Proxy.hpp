// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/util/Pimpl.hpp"

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
class Proxy;
}  // namespace zeromq
}  // namespace network

using OTZMQProxy = Pimpl<network::zeromq::Proxy>;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq
{
class OPENTXS_EXPORT Proxy
{
public:
    static auto Factory(
        const Context& context,
        socket::Socket& frontend,
        socket::Socket& backend) -> OTZMQProxy;

    virtual ~Proxy() = default;

protected:
    Proxy() = default;

private:
    friend OTZMQProxy;

    virtual auto clone() const -> Proxy* = 0;

    Proxy(const Proxy&) = delete;
    Proxy(Proxy&&) = default;
    auto operator=(const Proxy&) -> Proxy& = delete;
    auto operator=(Proxy&&) -> Proxy& = default;
};
}  // namespace opentxs::network::zeromq
