// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/network/zeromq/curve/Client.hpp"
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
class Request;
}  // namespace socket

class Message;
}  // namespace zeromq
}  // namespace network

using OTZMQRequestSocket = Pimpl<network::zeromq::socket::Request>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::socket
{
class OPENTXS_EXPORT Request : virtual public curve::Client
{
public:
    virtual auto Send(Message&& message) const noexcept -> SendResult = 0;
    virtual auto SetSocksProxy(const UnallocatedCString& proxy) const noexcept
        -> bool = 0;

    Request(const Request&) = delete;
    Request(Request&&) = delete;
    auto operator=(const Request&) -> Request& = delete;
    auto operator=(Request&&) -> Request& = delete;

    ~Request() override = default;

protected:
    Request() noexcept = default;

private:
    friend OTZMQRequestSocket;

    virtual auto clone() const noexcept -> Request* = 0;
};
}  // namespace opentxs::network::zeromq::socket
