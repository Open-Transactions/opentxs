// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/network/zeromq/socket/SocketType.hpp"

#pragma once

#include <map>
#include <mutex>
#include <thread>

#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/network/zeromq/zap/Request.hpp"

namespace opentxs
{
namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs

#define CALLBACK_WAIT_MILLISECONDS 50
#define RECEIVER_POLL_MILLISECONDS 100

#define RECEIVER_METHOD "opentxs::network::zeromq::implementation::Receiver::"

namespace opentxs::network::zeromq::socket::implementation
{
template <typename InterfaceType, typename MessageType = zeromq::Message>
class Receiver : virtual public InterfaceType, public Socket
{
public:
    auto apply_socket(SocketCallback&& cb) const noexcept -> bool override;
    auto Close() const noexcept -> bool final;

protected:
    const bool start_thread_;
    mutable std::thread receiver_thread_;

    virtual auto have_callback() const noexcept -> bool { return false; }
    void run_tasks(const Lock& lock) const noexcept;

    void init() noexcept override;
    virtual void process_incoming(
        const Lock& lock,
        MessageType&& message) noexcept = 0;
    void shutdown(const Lock& lock) noexcept override;
    virtual void thread() noexcept;

    Receiver(
        const zeromq::Context& context,
        const socket::Type type,
        const Socket::Direction direction,
        const bool startThread) noexcept;

    ~Receiver() override;

private:
    mutable int next_task_;
    mutable std::mutex task_lock_;
    mutable std::pmr::map<int, SocketCallback> socket_tasks_;
    mutable std::pmr::map<int, bool> task_result_;

    auto add_task(SocketCallback&& cb) const noexcept -> int;
    auto task_result(const int id) const noexcept -> bool;
    auto task_running(const int id) const noexcept -> bool;

    Receiver() = delete;
    Receiver(const Receiver&) = delete;
    Receiver(Receiver&&) = delete;
    auto operator=(const Receiver&) -> Receiver& = delete;
    auto operator=(Receiver&&) -> Receiver& = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation
