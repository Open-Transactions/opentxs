// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/network/zeromq/RequestSocket.hpp"
#include "opentxs/OT.hpp"

#include "network/zeromq/curve/Client.hpp"
#include "network/zeromq/Message.hpp"
#include "Sender.hpp"

#include <zmq.h>

#include "Request.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::RequestSocket>;

#define POLL_MILLISECONDS 10

#define OT_METHOD                                                              \
    "opentxs::network::zeromq::socket::implementation::RequestSocket::"

namespace opentxs::network::zeromq
{
OTZMQRequestSocket RequestSocket::Factory(const class Context& context)
{
    return OTZMQRequestSocket(
        new socket::implementation::RequestSocket(context));
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::socket::implementation
{
RequestSocket::RequestSocket(const zeromq::Context& context)
    : Sender(context, SocketType::Request, Socket::Direction::Connect)
    , Client(this->get())
{
    init();
}

RequestSocket* RequestSocket::clone() const
{
    return new RequestSocket(context_);
}

Socket::SendResult RequestSocket::SendRequest(opentxs::Data& input) const
{
    return SendRequest(Message::Factory(input));
}

Socket::SendResult RequestSocket::SendRequest(const std::string& input) const
{
    auto copy = input;

    return SendRequest(Message::Factory(copy));
}

Socket::SendResult RequestSocket::SendRequest(zeromq::Message& request) const
{
    OT_ASSERT(nullptr != socket_);

    Lock lock(lock_);
    SendResult output{opentxs::SendResult::ERROR, Message::Factory()};
    auto& status = output.first;
    auto& reply = output.second;

    if (false == send_message(lock, request)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to deliver message.")
            .Flush();

        return output;
    }

    if (false == wait(lock)) {
        LogVerbose(OT_METHOD)(__FUNCTION__)(": Receive timeout.").Flush();
        status = opentxs::SendResult::TIMEOUT;

        return output;
    }

    if (receive_message(lock, reply)) {
        status = opentxs::SendResult::VALID_REPLY;
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to receive reply.")
            .Flush();
    }

    return output;
}

bool RequestSocket::SetSocksProxy(const std::string& proxy) const
{
    return set_socks_proxy(proxy);
}

bool RequestSocket::wait(const Lock& lock) const
{
    OT_ASSERT(verify_lock(lock))

    const auto start = std::chrono::system_clock::now();
    zmq_pollitem_t poll[1];
    poll[0].socket = socket_;
    poll[0].events = ZMQ_POLLIN;

    while (running_.get()) {
        std::this_thread::yield();
        const auto events = zmq_poll(poll, 1, POLL_MILLISECONDS);

        if (0 == events) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(": No messages.").Flush();

            const auto now = std::chrono::system_clock::now();

            if ((now - start) > std::chrono::milliseconds(receive_timeout_)) {

                return false;
            } else {
                continue;
            }
        }

        if (0 > events) {
            const auto error = zmq_errno();
            LogOutput(OT_METHOD)(__FUNCTION__)(": Poll error: ")(
                zmq_strerror(error))(".")
                .Flush();

            return false;
        }

        return true;
    }

    return false;
}

RequestSocket::~RequestSocket() SHUTDOWN
}  // namespace opentxs::network::zeromq::socket::implementation
