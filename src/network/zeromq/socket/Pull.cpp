// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/PullSocket.hpp"

#include "network/zeromq/curve/Server.hpp"
#include "Receiver.hpp"
#include "Socket.hpp"

#include <chrono>

#include <zmq.h>

#include "Pull.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::PullSocket>;

//#define OT_METHOD
//"opentxs::network::zeromq::socket::implementation::PullSocket::"

namespace opentxs::network::zeromq
{
OTZMQPullSocket PullSocket::Factory(
    const class Context& context,
    const Socket::Direction direction,
    const ListenCallback& callback)
{
    return OTZMQPullSocket(
        new socket::implementation::PullSocket(context, direction, callback));
}

OTZMQPullSocket PullSocket::Factory(
    const class Context& context,
    const Socket::Direction direction)
{
    return OTZMQPullSocket(
        new socket::implementation::PullSocket(context, direction));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::socket::implementation
{
PullSocket::PullSocket(
    const zeromq::Context& context,
    const Socket::Direction direction,
    const zeromq::ListenCallback& callback,
    const bool startThread)
    : Receiver(context, SocketType::Pull, direction, startThread)
    , Server(this->get())
    , callback_(callback)
{
    init();
}

PullSocket::PullSocket(
    const zeromq::Context& context,
    const Socket::Direction direction,
    const zeromq::ListenCallback& callback)
    : PullSocket(context, direction, callback, true)
{
}

PullSocket::PullSocket(
    const zeromq::Context& context,
    const Socket::Direction direction)
    : PullSocket(context, direction, ListenCallback::Factory(), false)
{
}

PullSocket* PullSocket::clone() const
{
    return new PullSocket(context_, direction_, callback_);
}

bool PullSocket::have_callback() const { return true; }

void PullSocket::process_incoming(const Lock& lock, Message& message)
{
    OT_ASSERT(verify_lock(lock))

    callback_.Process(message);
}

PullSocket::~PullSocket() SHUTDOWN
}  // namespace opentxs::network::zeromq::socket::implementation
