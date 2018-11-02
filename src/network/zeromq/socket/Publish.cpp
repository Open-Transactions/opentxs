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
#include "opentxs/network/zeromq/PublishSocket.hpp"

#include "network/zeromq/curve/Server.hpp"
#include "Sender.hpp"

#include "Publish.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::PublishSocket>;

//#define OT_METHOD
//"opentxs::network::zeromq::socket::implementation::PublishSocket::"

namespace opentxs::network::zeromq
{
OTZMQPublishSocket PublishSocket::Factory(const class Context& context)
{
    return OTZMQPublishSocket(
        new socket::implementation::PublishSocket(context));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::socket::implementation
{
PublishSocket::PublishSocket(const zeromq::Context& context)
    : Sender(context, SocketType::Publish, Socket::Direction::Bind)
    , Server(this->get())
{
    init();
}

PublishSocket::~PublishSocket() SHUTDOWN
}  // namespace opentxs::network::zeromq::socket::implementation
