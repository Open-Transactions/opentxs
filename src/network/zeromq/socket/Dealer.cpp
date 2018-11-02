// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/DealerSocket.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include "network/zeromq/curve/Client.hpp"
#include "Bidirectional.hpp"
#include "Send.hpp"
#include "Socket.hpp"

#include <zmq.h>

#include "Dealer.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::DealerSocket>;

#define OT_METHOD                                                              \
    "opentxs::network::zeromq::socket::implementation::DealerSocket::"

namespace opentxs::network::zeromq
{
OTZMQDealerSocket DealerSocket::Factory(
    const class Context& context,
    const Socket::Direction direction,
    const ListenCallback& callback)
{
    return OTZMQDealerSocket(
        new socket::implementation::DealerSocket(context, direction, callback));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::socket::implementation
{
DealerSocket::DealerSocket(
    const zeromq::Context& context,
    const Socket::Direction direction,
    const zeromq::ListenCallback& callback)
    : _Bidirectional(context, SocketType::Dealer, direction, true)
    , Client(this->get())
    , callback_(callback)
{
    init();
}

DealerSocket* DealerSocket::clone() const
{
    return new DealerSocket(context_, direction_, callback_);
}

void DealerSocket::process_incoming(
    const Lock& lock,
    opentxs::network::zeromq::Message& message)
{
    OT_ASSERT(verify_lock(lock))

    LogDetail(OT_METHOD)(__FUNCTION__)(
        ": Incoming messaged received. Triggering callback.")
        .Flush();
    callback_.Process(message);
    LogDetail(OT_METHOD)(__FUNCTION__)(": Done.").Flush();
}

DealerSocket::~DealerSocket() SHUTDOWN
}  // namespace opentxs::network::zeromq::socket::implementation
