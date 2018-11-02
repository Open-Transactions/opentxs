// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/ReplyCallback.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/ReplySocket.hpp"

#include "network/zeromq/curve/Server.hpp"
#include "Receiver.tpp"
#include "Socket.hpp"

#include "Reply.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::ReplySocket>;
template class opentxs::network::zeromq::socket::implementation::Receiver<
    opentxs::network::zeromq::Message>;

//#define OT_METHOD
//"opentxs::network::zeromq::socket::implementation::ReplySocket::"

namespace opentxs::network::zeromq
{
OTZMQReplySocket ReplySocket::Factory(
    const class Context& context,
    const Socket::Direction direction,
    const ReplyCallback& callback)
{
    return OTZMQReplySocket(
        new socket::implementation::ReplySocket(context, direction, callback));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::socket::implementation
{
ReplySocket::ReplySocket(
    const zeromq::Context& context,
    const Socket::Direction direction,
    const ReplyCallback& callback)
    : Receiver(context, SocketType::Reply, direction, true)
    , Server(this->get())
    , callback_(callback)
{
    init();
}

ReplySocket* ReplySocket::clone() const
{
    return new ReplySocket(context_, direction_, callback_);
}

bool ReplySocket::have_callback() const { return true; }

void ReplySocket::process_incoming(const Lock& lock, Message& message)
{
    auto output = callback_.Process(message);
    Message& reply = output;
    send_message(lock, reply);
}

ReplySocket::~ReplySocket() SHUTDOWN
}  // namespace opentxs::network::zeromq::socket::implementation
