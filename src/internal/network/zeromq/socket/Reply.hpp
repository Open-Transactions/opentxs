// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/network/zeromq/curve/Server.hpp"
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
class Reply;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

using OTZMQReplySocket = Pimpl<network::zeromq::socket::Reply>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::socket
{
class Reply : virtual public curve::Server
{
public:
    Reply(const Reply&) = delete;
    Reply(Reply&&) = delete;
    auto operator=(const Reply&) -> Reply& = delete;
    auto operator=(Reply&&) -> Reply& = delete;

    ~Reply() override = default;

protected:
    Reply() noexcept = default;

private:
    friend OTZMQReplySocket;

#ifdef _WIN32
public:
#endif
    virtual auto clone() const noexcept -> Reply* = 0;
};
}  // namespace opentxs::network::zeromq::socket
