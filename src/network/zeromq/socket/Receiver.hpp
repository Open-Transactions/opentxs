// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/network/zeromq/socket/SocketType.hpp"

#pragma once

#include <mutex>
#include <string_view>
#include <thread>

#include "internal/util/Mutex.hpp"
#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/network/zeromq/zap/Request.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
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
// NOLINTEND(modernize-concat-nested-namespaces)

#define RECEIVER_METHOD "opentxs::network::zeromq::implementation::Receiver::"

namespace opentxs::network::zeromq::socket::implementation
{
template <typename InterfaceType, typename MessageType = zeromq::Message>
class Receiver : virtual public InterfaceType, public Socket
{
public:
    auto apply_socket(SocketCallback&& cb) const noexcept -> bool override;
    auto Close() const noexcept -> bool final;

    Receiver() = delete;
    Receiver(const Receiver&) = delete;
    Receiver(Receiver&&) = delete;
    auto operator=(const Receiver&) -> Receiver& = delete;
    auto operator=(Receiver&&) -> Receiver& = delete;

protected:
    const bool start_thread_;
    mutable std::thread receiver_thread_;
    mutable CString thread_name_;

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
        const Direction direction,
        const bool startThread,
        const std::string_view threadname = "Receiver") noexcept;

    ~Receiver() override;

private:
    static constexpr auto callback_wait_milliseconds_{50};
    static constexpr auto receiver_poll_milliseconds_{100};

    mutable int next_task_;
    mutable std::mutex task_lock_;
    mutable UnallocatedMap<int, SocketCallback> socket_tasks_;
    mutable UnallocatedMap<int, bool> task_result_;

    auto add_task(SocketCallback&& cb) const noexcept -> int;
    auto task_result(const int id) const noexcept -> bool;
    auto task_running(const int id) const noexcept -> bool;
};
}  // namespace opentxs::network::zeromq::socket::implementation
