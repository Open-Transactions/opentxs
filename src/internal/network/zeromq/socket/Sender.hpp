// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/network/zeromq/socket/Socket.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::socket
{
class Sender : virtual public Socket
{
public:
    virtual auto Send(Message&& message) const noexcept -> bool = 0;

    Sender(const Sender&) = delete;
    Sender(Sender&&) = delete;
    auto operator=(const Sender&) -> Sender& = delete;
    auto operator=(Sender&&) -> Sender& = delete;

    ~Sender() override = default;

protected:
    Sender() = default;
};
}  // namespace opentxs::network::zeromq::socket
