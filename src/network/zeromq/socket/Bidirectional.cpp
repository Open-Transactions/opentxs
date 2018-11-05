// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/FrameIterator.hpp"
#include "opentxs/network/zeromq/Message.hpp"

#include <zmq.h>

#include "network/zeromq/Message.hpp"
#include "Socket.hpp"

#include "Bidirectional.hpp"

#define CALLBACK_WAIT_MILLISECONDS 50
#define POLL_MILLISECONDS 100

#define OT_METHOD "opentxs::network::zeromq::implementation::Bidirectional::"

namespace opentxs::network::zeromq::socket::implementation
{
Bidirectional::Bidirectional(
    const zeromq::Context& context,
    const SocketType type,
    const zeromq::Socket::Direction direction,
    const bool startThread)
    : Receiver(context, type, direction, false)
    , push_socket_{zmq_socket(context, ZMQ_PUSH)}
    , bidirectional_start_thread_(startThread)
    , endpoint_{Socket::random_inproc_endpoint()}
    , pull_socket_{zmq_socket(context, ZMQ_PULL)}
{
}

bool Bidirectional::apply_timeouts(void* socket, std::mutex& socket_mutex) const
{
    OT_ASSERT(nullptr != socket);
    Lock lock(socket_mutex);

    auto set = zmq_setsockopt(socket, ZMQ_LINGER, &linger_, sizeof(linger_));

    if (0 != set) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to set ZMQ_LINGER"
              << std::endl;

        return false;
    }

    set = zmq_setsockopt(
        socket, ZMQ_SNDTIMEO, &send_timeout_, sizeof(send_timeout_));

    if (0 != set) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to set ZMQ_SNDTIMEO"
              << std::endl;

        return false;
    }

    set = zmq_setsockopt(
        socket, ZMQ_RCVTIMEO, &receive_timeout_, sizeof(receive_timeout_));

    if (0 != set) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to set ZMQ_RCVTIMEO"
              << std::endl;

        return false;
    }

    return true;
}

bool Bidirectional::bind(
    void* socket,
    std::mutex& socket_mutex,
    const std::string& endpoint) const
{
    apply_timeouts(socket, socket_mutex);

    return (0 == zmq_bind(socket, endpoint.c_str()));
}

bool Bidirectional::connect(
    void* socket,
    std::mutex& socket_mutex,
    const std::string& endpoint) const
{
    apply_timeouts(socket, socket_mutex);

    return (0 == zmq_connect(socket, endpoint.c_str()));
}

void Bidirectional::init()
{
    OT_ASSERT(nullptr != pull_socket_);
    OT_ASSERT(nullptr != push_socket_);

    auto bound = bind(pull_socket_, lock_, endpoint_);

    OT_ASSERT(false != bound);

    auto connected = connect(push_socket_, lock_, endpoint_);

    OT_ASSERT(false != connected);

    Socket::init();

    if (bidirectional_start_thread_) {
        receiver_thread_ = std::thread(&Bidirectional::thread, this);
    }
}

bool Bidirectional::process_pull_socket(const Lock& lock)
{
    auto msg = Message::Factory();
    const auto received = Socket::receive_message(lock, pull_socket_, msg);

    if (false == received) { return false; }

    const auto sent = send(lock, msg);

    return sent;
}

bool Bidirectional::process_receiver_socket(const Lock& lock)
{
    auto reply = Message::Factory();
    const auto received = Socket::receive_message(lock, socket_, reply);

    if (false == received) { return false; }

    process_incoming(lock, reply);

    return true;
}

bool Bidirectional::queue_message(zeromq::Message& message) const
{
    Lock lock(send_lock_);

    if (false == running_.get()) { return false; }

    OT_ASSERT(nullptr != push_socket_);

    return Socket::send_message(lock, push_socket_, message);
}

bool Bidirectional::send(const Lock& lock, zeromq::Message& message)
{
    return Socket::send_message(lock, socket_, message);
}

void Bidirectional::shutdown(const Lock& lock)
{
    Lock send(send_lock_);
    Socket::shutdown(lock);

    if (running_.get()) {
        zmq_disconnect(push_socket_, endpoint_.c_str());
        zmq_unbind(pull_socket_, endpoint_.c_str());
    }
}

void Bidirectional::thread()
{
    LogVerbose(OT_METHOD)(__FUNCTION__)(": Starting listener").Flush();

    while (running_.get()) {
        if (have_callback()) { break; }

        Log::Sleep(std::chrono::milliseconds(CALLBACK_WAIT_MILLISECONDS));
    }

    LogVerbose(OT_METHOD)(__FUNCTION__)(": Callback ready").Flush();
    zmq_pollitem_t poll[2];
    poll[0].socket = socket_;
    poll[0].events = ZMQ_POLLIN;
    poll[1].socket = pull_socket_;
    poll[1].events = ZMQ_POLLIN;

    while (running_.get()) {
        std::this_thread::yield();
        Lock lock(lock_, std::try_to_lock);

        if (false == lock.owns_lock()) { continue; }

        run_tasks(lock);
        const auto events = zmq_poll(poll, 2, POLL_MILLISECONDS);

        if (0 == events) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(": No messages.").Flush();

            continue;
        }

        if (-1 == events) {
            const auto error = zmq_errno();
            otErr << OT_METHOD << __FUNCTION__
                  << ": Poll error: " << zmq_strerror(error) << std::endl;

            continue;
        }

        bool processed = true;

        if (ZMQ_POLLIN == poll[0].revents) {
            processed = process_receiver_socket(lock);
        }

        if (processed && ZMQ_POLLIN == poll[1].revents) {
            processed = process_pull_socket(lock);
        }
    }

    LogVerbose(OT_METHOD)(__FUNCTION__)(": Shutting down").Flush();
}
}  // namespace opentxs::network::zeromq::socket::implementation
