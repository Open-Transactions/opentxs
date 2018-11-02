// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include <chrono>

#include <zmq.h>

#include "Subscribe.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::SubscribeSocket>;

#define OT_METHOD                                                              \
    "opentxs::network::zeromq::socket::implementation::SubscribeSocket::"

namespace opentxs::network::zeromq
{
OTZMQSubscribeSocket SubscribeSocket::Factory(
    const class Context& context,
    const ListenCallback& callback)
{
    return OTZMQSubscribeSocket(
        new socket::implementation::SubscribeSocket(context, callback));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::socket::implementation
{
SubscribeSocket::SubscribeSocket(
    const zeromq::Context& context,
    const zeromq::ListenCallback& callback)
    : Receiver(context, SocketType::Subscribe, Socket::Direction::Connect, true)
    , Client(this->get())
    , callback_(callback)
{
    init();
}

SubscribeSocket* SubscribeSocket::clone() const
{
    return new SubscribeSocket(context_, callback_);
}

bool SubscribeSocket::have_callback() const { return true; }

void SubscribeSocket::init()
{
    // subscribe to all messages until filtering is implemented
    const auto set = zmq_setsockopt(socket_, ZMQ_SUBSCRIBE, "", 0);
    Receiver::init();

    OT_ASSERT(0 == set);
}

void SubscribeSocket::process_incoming(const Lock& lock, Message& message)
{
    OT_ASSERT(verify_lock(lock))

    LogDetail(OT_METHOD)(__FUNCTION__)(
        ": Incoming messaged received. Triggering callback.")
        .Flush();
    callback_.Process(message);
    LogDetail(OT_METHOD)(__FUNCTION__)(" Done.").Flush();
}

bool SubscribeSocket::SetSocksProxy(const std::string& proxy) const
{
    return set_socks_proxy(proxy);
}

SubscribeSocket::~SubscribeSocket() SHUTDOWN
}  // namespace opentxs::network::zeromq::socket::implementation
