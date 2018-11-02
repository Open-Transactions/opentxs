// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"

#include "network/zeromq/socket/Socket.hpp"

namespace opentxs::network::zeromq::socket::implementation
{
class Sender : public Socket
{
protected:
    bool deliver(zeromq::Message& message) const;

    Sender(
        const zeromq::Context& context,
        const SocketType type,
        const zeromq::Socket::Direction direction);

    virtual ~Sender() = default;

private:
    Sender() = delete;
    Sender(const Sender&) = delete;
    Sender(Sender&&) = delete;
    Sender& operator=(const Sender&) = delete;
    Sender& operator=(Sender&&) = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation
