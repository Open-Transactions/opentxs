// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Socket.hpp"

#include "network/zeromq/socket/Pair.hpp"

#include <zmq.h>

#include "Proxy.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::Proxy>;

//#define OT_METHOD "opentxs::network::zeromq::implementation::Proxy::"

namespace opentxs::network::zeromq
{
OTZMQProxy Proxy::Factory(
    const Context& context,
    Socket& frontend,
    Socket& backend)
{
    return OTZMQProxy(new implementation::Proxy(context, frontend, backend));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
Proxy::Proxy(
    const zeromq::Context& context,
    zeromq::Socket& frontend,
    zeromq::Socket& backend)
    : context_(context)
    , frontend_(frontend)
    , backend_(backend)
    , null_callback_(opentxs::network::zeromq::ListenCallback::Factory(
          [](const zeromq::Message&) -> void {}))
    , control_listener_(new socket::implementation::PairSocket(
          context,
          null_callback_,
          false))
    , control_sender_(new socket::implementation::PairSocket(
          null_callback_,
          control_listener_,
          false))
    , thread_(nullptr)
{
    thread_.reset(new std::thread(&Proxy::proxy, this));

    OT_ASSERT(thread_)
}

Proxy* Proxy::clone() const { return new Proxy(context_, frontend_, backend_); }

void Proxy::proxy() const
{
    zmq_proxy_steerable(frontend_, backend_, nullptr, nullptr);
}

Proxy::~Proxy()
{
    control_sender_->Send("TERMINATE");

    if (thread_ && thread_->joinable()) { thread_->join(); }
}
}  // namespace opentxs::network::zeromq::implementation
