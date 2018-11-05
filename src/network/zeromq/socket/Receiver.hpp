// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "Internal.hpp"
#include "stdafx.hpp"

#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/Message.hpp"
#include "opentxs/Types.hpp"

#include "network/zeromq/socket/Socket.hpp"

#include <zmq.h>

#include <memory>
#include <mutex>
#include <thread>

#define CALLBACK_WAIT_MILLISECONDS 50
#define RECEIVER_POLL_MILLISECONDS 100

#define RECEIVER_METHOD "opentxs::network::zeromq::implementation::Receiver::"

namespace opentxs::network::zeromq::socket::implementation
{
template <typename T>
class Receiver : public Socket
{
public:
    bool apply_socket(SocketCallback&& cb) const override;

protected:
    std::thread receiver_thread_{};

    virtual bool have_callback() const { return false; }
    void run_tasks(const Lock& lock) const;

    void init() override;
    virtual void process_incoming(const Lock& lock, T& message) = 0;
    void shutdown(const Lock& lock) override;
    virtual void thread();

    Receiver(
        const zeromq::Context& context,
        const SocketType type,
        const Socket::Direction direction,
        const bool startThread);

    virtual ~Receiver();

private:
    const bool start_thread_{true};
    mutable int next_task_;
    mutable std::mutex task_lock_;
    mutable std::map<int, SocketCallback> socket_tasks_;
    mutable std::map<int, bool> task_result_;

    int add_task(SocketCallback&& cb) const;
    bool task_result(const int id) const;
    bool task_running(const int id) const;

    Receiver() = delete;
    Receiver(const Receiver&) = delete;
    Receiver(Receiver&&) = delete;
    Receiver& operator=(const Receiver&) = delete;
    Receiver& operator=(Receiver&&) = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation

extern template class opentxs::network::zeromq::socket::implementation::
    Receiver<opentxs::network::zeromq::Message>;
extern template class opentxs::network::zeromq::socket::implementation::
    Receiver<opentxs::network::zeromq::zap::Request>;
