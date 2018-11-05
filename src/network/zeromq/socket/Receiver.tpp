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
    , start_thread_(startThread)
    , next_task_(0)
    , task_lock_()
    , socket_tasks_()
{
}

template <typename T>
int Receiver<T>::add_task(SocketCallback&& cb) const
{
    Lock lock(task_lock_);
    auto [it, success] = socket_tasks_.emplace(++next_task_, std::move(cb));

    OT_ASSERT(success);

    return it->first;
}

template <typename T>
bool Receiver<T>::apply_socket(SocketCallback&& cb) const
{
    const auto id = add_task(std::move(cb));

    while (task_running(id)) {
        Log::Sleep(std::chrono::milliseconds(RECEIVER_POLL_MILLISECONDS));
    }

    return task_result(id);
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
void Receiver<T>::run_tasks(const Lock& lock) const
{
    Lock task_lock(task_lock_);

    for (auto i = socket_tasks_.begin(); i != socket_tasks_.end(); ++i) {
        const auto& [id, cb] = *i;
        task_result_.emplace(id, cb(lock));
        socket_tasks_.erase(i);
    }
}

template <typename T>
void Receiver<T>::shutdown(const Lock& lock)
{
    if (receiver_thread_.joinable()) { receiver_thread_.join(); }

    Socket::shutdown(lock);
}

template <typename T>
bool Receiver<T>::task_result(const int id) const
{
    Lock lock(task_lock_);
    const auto it = task_result_.find(id);

    OT_ASSERT(task_result_.end() != it);

    auto output = it->second;
    task_result_.erase(it);

    return output;
}

template <typename T>
bool Receiver<T>::task_running(const int id) const
{
    Lock lock(task_lock_);

    return (1 == socket_tasks_.count(id));
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
        std::this_thread::yield();
        Lock lock(lock_, std::try_to_lock);

        if (false == lock.owns_lock()) { continue; }

        run_tasks(lock);
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
Receiver<T>::~Receiver()
{
    if (receiver_thread_.joinable()) { receiver_thread_.join(); }
}
}  // namespace opentxs::network::zeromq::socket::implementation
