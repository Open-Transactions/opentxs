// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include <zmq.h>

#include "Pair.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::PairSocket>;

// #define OT_METHOD
// "opentxs::network::zeromq::socket::implementation::PairSocket::"

namespace opentxs::network::zeromq
{
OTZMQPairSocket PairSocket::Factory(
    const class Context& context,
    const ListenCallback& callback)
{
    return OTZMQPairSocket(
        new socket::implementation::PairSocket(context, callback));
}

OTZMQPairSocket PairSocket::Factory(
    const ListenCallback& callback,
    const PairSocket& peer)
{
    return OTZMQPairSocket(
        new socket::implementation::PairSocket(callback, peer));
}

OTZMQPairSocket PairSocket::Factory(
    const class Context& context,
    const ListenCallback& callback,
    const std::string& endpoint)
{
    return OTZMQPairSocket(
        new socket::implementation::PairSocket(context, callback, endpoint));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::socket::implementation
{
PairSocket::PairSocket(
    const zeromq::Context& context,
    const zeromq::ListenCallback& callback,
    const std::string& endpoint,
    const Socket::Direction direction,
    const bool startThread)
    : Bidirectional(context, SocketType::Pair, direction, startThread)
    , callback_(callback)
    , endpoint_(endpoint)
{
    init();
}

PairSocket::PairSocket(
    const zeromq::Context& context,
    const zeromq::ListenCallback& callback,
    const bool startThread)
    : PairSocket(
          context,
          callback,
          Socket::random_inproc_endpoint(),
          Socket::Direction::Bind,
          startThread)
{
}

PairSocket::PairSocket(
    const zeromq::ListenCallback& callback,
    const zeromq::PairSocket& peer,
    const bool startThread)
    : PairSocket(
          peer.Context(),
          callback,
          peer.Endpoint(),
          Socket::Direction::Connect,
          startThread)
{
}

PairSocket::PairSocket(
    const zeromq::Context& context,
    const zeromq::ListenCallback& callback,
    const std::string& endpoint)
    : PairSocket(context, callback, endpoint, Socket::Direction::Connect, true)
{
}

PairSocket* PairSocket::clone() const
{
    return new PairSocket(context_, callback_, endpoint_, direction_, false);
}

const std::string& PairSocket::Endpoint() const { return endpoint_; }

bool PairSocket::have_callback() const { return true; }

void PairSocket::init()
{
    Bidirectional::init();

    OT_ASSERT(false == endpoint_.empty())

    const auto init = Bidirectional::Start(endpoint_);

    OT_ASSERT(init)
}

void PairSocket::process_incoming(const Lock& lock, Message& message)
{
    OT_ASSERT(verify_lock(lock))

    callback_.Process(message);
}

bool PairSocket::Send(const std::string& data) const
{
    return Send(Message::Factory(data));
}

bool PairSocket::Send(const opentxs::Data& data) const
{
    return Send(Message::Factory(data));
}

bool PairSocket::Send(zeromq::Message& data) const
{
    return queue_message(data);
}

PairSocket::~PairSocket() SHUTDOWN
}  // namespace opentxs::network::zeromq::socket::implementation
