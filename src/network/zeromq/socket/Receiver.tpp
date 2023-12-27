// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/network/zeromq/socket/SocketType.hpp"

#pragma once

#include "network/zeromq/socket/Receiver.hpp"  // IWYU pragma: associated

#include <zmq.h>
#include <array>
#include <iostream>
#include <memory>
#include <mutex>
#include <thread>

#include "internal/util/Flag.hpp"
#include "internal/util/Signals.hpp"
#include "internal/util/Thread.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Types.internal.hpp"

namespace opentxs::network::zeromq::socket::implementation
{
template <typename InterfaceType, typename MessageType>
Receiver<InterfaceType, MessageType>::Receiver(
    const zeromq::Context& context,
    const socket::Type type,
    const Direction direction,
    const bool startThread,
    const std::string_view threadname) noexcept
    : Socket(context, type, direction)
    , start_thread_(startThread)
    , receiver_thread_()
    , thread_name_(threadname)
    , next_task_(0)
    , task_lock_()
    , socket_tasks_()
    , task_result_()
{
}

template <typename InterfaceType, typename MessageType>
auto Receiver<InterfaceType, MessageType>::add_task(
    SocketCallback&& cb) const noexcept -> int
{
    const auto lock = Lock{task_lock_};
    auto [it, success] = socket_tasks_.emplace(++next_task_, std::move(cb));

    assert_true(success);

    return it->first;
}

template <typename InterfaceType, typename MessageType>
auto Receiver<InterfaceType, MessageType>::apply_socket(
    SocketCallback&& cb) const noexcept -> bool
{
    const auto id = add_task(std::move(cb));

    while (task_running(id)) {
        sleep(std::chrono::milliseconds(receiver_poll_milliseconds_));
    }

    return task_result(id);
}

template <typename InterfaceType, typename MessageType>
auto Receiver<InterfaceType, MessageType>::Close() const noexcept -> bool
{
    running_->Off();

    if (receiver_thread_.joinable()) { receiver_thread_.join(); }

    return Socket::Close();
}

template <typename InterfaceType, typename MessageType>
void Receiver<InterfaceType, MessageType>::init() noexcept
{
    Socket::init();

    if (start_thread_) {
        receiver_thread_ = std::thread(&Receiver::thread, this);
    }
}

template <typename InterfaceType, typename MessageType>
void Receiver<InterfaceType, MessageType>::run_tasks(
    const Lock& lock) const noexcept
{
    const auto task_lock = Lock{task_lock_};
    auto i = socket_tasks_.begin();

    while (i != socket_tasks_.end()) {
        const auto& [id, cb] = *i;
        task_result_.emplace(id, cb(lock));
        i = socket_tasks_.erase(i);
    }
}

template <typename InterfaceType, typename MessageType>
void Receiver<InterfaceType, MessageType>::shutdown(const Lock& lock) noexcept
{
    if (receiver_thread_.joinable()) { receiver_thread_.join(); }

    Socket::shutdown(lock);
}

template <typename InterfaceType, typename MessageType>
auto Receiver<InterfaceType, MessageType>::task_result(
    const int id) const noexcept -> bool
{
    const auto lock = Lock{task_lock_};
    const auto it = task_result_.find(id);

    assert_true(task_result_.end() != it);

    auto output = it->second;
    task_result_.erase(it);

    return output;
}

template <typename InterfaceType, typename MessageType>
auto Receiver<InterfaceType, MessageType>::task_running(
    const int id) const noexcept -> bool
{
    const auto lock = Lock{task_lock_};

    return (1 == socket_tasks_.count(id));
}

template <typename InterfaceType, typename MessageType>
void Receiver<InterfaceType, MessageType>::thread() noexcept
{
    Signals::Block();

    if (!thread_name_.empty()) { SetThisThreadsName(thread_name_); }

    while (running_.get()) {
        if (have_callback()) { break; }

        sleep(std::chrono::milliseconds(callback_wait_milliseconds_));
    }

    auto poll = std::array<::zmq_pollitem_t, 1>{};
    poll[0].socket = socket_;
    poll[0].events = ZMQ_POLLIN;

    while (running_.get()) {
        auto newEndpoints = endpoint_queue_.pop();
        auto lock = Lock{lock_, std::try_to_lock};

        if (false == lock.owns_lock()) { continue; }

        for (const auto& endpoint : newEndpoints) { start(lock, endpoint); }

        run_tasks(lock);
        const auto events =
            zmq_poll(poll.data(), 1, receiver_poll_milliseconds_);

        if (0 == events) { continue; }

        if (-1 == events) {
            const auto error = zmq_errno();
            std::cerr << std::source_location::current().function_name() << ": "
                      << "poll error: " << zmq_strerror(error) << std::endl;

            continue;
        }

        if (false == running_.get()) { return; }

        auto reply = MessageType{};
        const auto received = Socket::receive_message(lock, socket_, reply);

        if (false == received) {
            std::cerr << std::source_location::current().function_name() << ": "
                      << "failed to receive incoming message." << std::endl;

            continue;
        }

        process_incoming(lock, std::move(reply));
        lock.unlock();
        std::this_thread::yield();
    }
}

template <typename InterfaceType, typename MessageType>
Receiver<InterfaceType, MessageType>::~Receiver()
{
    if (receiver_thread_.joinable()) { receiver_thread_.join(); }
}
}  // namespace opentxs::network::zeromq::socket::implementation
