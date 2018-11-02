// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "stdafx.hpp"

#include "Receiver.hpp"

namespace opentxs::network::zeromq::socket::implementation
{
template <typename T>
Receiver<T>::Receiver(
    const zeromq::Context& context,
    const SocketType type,
    const Socket::Direction direction,
    const bool startThread)
    : Socket(context, type, direction)
    , receiver_thread_()
    , start_()
    , start_thread_(startThread)
{
}

template <typename T>
void Receiver<T>::init()
{
    Socket::init();

    if (start_thread_) {
        receiver_thread_ = std::thread(&Receiver::thread, this);
    }
}

template <typename T>
void Receiver<T>::thread()
{
    while (running_.get()) {
        if (have_callback()) { break; }

        Log::Sleep(std::chrono::milliseconds(CALLBACK_WAIT_MILLISECONDS));
    }

    zmq_pollitem_t poll[1];
    poll[0].socket = socket_;
    poll[0].events = ZMQ_POLLIN;

    while (running_.get()) {
        Lock lock(lock_, std::try_to_lock);

        if (false == lock.owns_lock()) { continue; }

        std::string endpoint{};

        while (start_.Pop(endpoint)) { Socket::start(lock, endpoint); }

        const auto events = zmq_poll(poll, 1, RECEIVER_POLL_MILLISECONDS);

        if (0 == events) { continue; }

        if (-1 == events) {
            const auto error = zmq_errno();
            std::cerr << RECEIVER_METHOD << __FUNCTION__
                      << ": Poll error: " << zmq_strerror(error) << std::endl;

            continue;
        }

        if (false == running_.get()) { return; }

        auto reply = T::Factory();
        const auto received = Socket::receive_message(lock, socket_, reply);

        if (false == received) { return; }

        process_incoming(lock, reply);
    }
}

template <typename T>
void Receiver<T>::shutdown(const Lock& lock)
{
    if (receiver_thread_.joinable()) { receiver_thread_.join(); }

    Socket::shutdown(lock);
}

template <typename T>
bool Receiver<T>::Start(const std::string& endpoint) const
{
    start_.Push(endpoint);

    while (running_.get() && start_.Queued(endpoint)) {
        if (1 == endpoints_.count(endpoint)) { return true; }

        Log::Sleep(std::chrono::milliseconds(RECEIVER_POLL_MILLISECONDS));
    }

    return 1 == endpoints_.count(endpoint);
}

template <typename T>
Receiver<T>::~Receiver()
{
    if (receiver_thread_.joinable()) { receiver_thread_.join(); }
}
}  // namespace opentxs::network::zeromq::socket::implementation
