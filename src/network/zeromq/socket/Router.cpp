// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/RouterSocket.hpp"

#include "network/zeromq/curve/Client.hpp"
#include "network/zeromq/curve/Server.hpp"
#include "Bidirectional.hpp"
#include "Send.hpp"
#include "Socket.hpp"

#include <zmq.h>

#include "Router.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::RouterSocket>;

#define OT_METHOD                                                              \
    "opentxs::network::zeromq::socket::implementation::RouterSocket::"

namespace opentxs::network::zeromq
{
OTZMQRouterSocket RouterSocket::Factory(
    const class Context& context,
    const Socket::Direction direction,
    const ListenCallback& callback)
{
    return OTZMQRouterSocket(
        new socket::implementation::RouterSocket(context, direction, callback));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::socket::implementation
{
RouterSocket::RouterSocket(
    const zeromq::Context& context,
    const Socket::Direction direction,
    const zeromq::ListenCallback& callback)
    : _Bidirectional(context, SocketType::Router, direction, true)
    , Client(this->get())
    , Server(this->get())
    , callback_(callback)
{
    init();
}

RouterSocket* RouterSocket::clone() const
{
    return new RouterSocket(context_, direction_, callback_);
}

void RouterSocket::process_incoming(const Lock& lock, Message& message)
{
    OT_ASSERT(verify_lock(lock))

    LogDetail(OT_METHOD)(__FUNCTION__)(
        ": Incoming messaged received. Triggering callback.")
        .Flush();
    // RouterSocket prepends an identity frame to the message.  This makes sure
    // there is an empty frame between the identity frame(s) and the frames that
    // make up the rest of the message.
    message.EnsureDelimiter();
    callback_.Process(message);
    LogDetail(OT_METHOD)(__FUNCTION__)(": Done.").Flush();
}

RouterSocket::~RouterSocket() SHUTDOWN
}  // namespace opentxs::network::zeromq::socket::implementation
