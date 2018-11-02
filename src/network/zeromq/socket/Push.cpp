// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/PushSocket.hpp"

#include "network/zeromq/curve/Client.hpp"
#include "Sender.hpp"

#include "Push.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::PushSocket>;

//#define OT_METHOD
//"opentxs::network::zeromq::socket::implementation::PushSocket::"

namespace opentxs::network::zeromq
{
OTZMQPushSocket PushSocket::Factory(
    const class Context& context,
    const Socket::Direction direction)
{
    return OTZMQPushSocket(
        new socket::implementation::PushSocket(context, direction));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::socket::implementation
{
PushSocket::PushSocket(
    const zeromq::Context& context,
    const Socket::Direction direction)
    : Sender(context, SocketType::Push, direction)
    , Client(this->get())
{
    init();
}

PushSocket::~PushSocket() SHUTDOWN
}  // namespace opentxs::network::zeromq::socket::implementation
