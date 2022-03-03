// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <zmq.h>
#include <memory>
#include <mutex>
#include <string_view>

#include "network/zeromq/socket/Receiver.hpp"
#include "network/zeromq/socket/Receiver.tpp"
#include "network/zeromq/socket/Sender.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace network
{
namespace zeromq
{
class Context;
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::socket::implementation
{
template <typename InterfaceType, typename MessageType = zeromq::Message>
class Bidirectional
    : virtual public Receiver<InterfaceType, MessageType>,
      virtual public Sender<InterfaceType, Receiver<InterfaceType, MessageType>>
{
protected:
    Bidirectional(
        const zeromq::Context& context,
        const bool startThread) noexcept;

    void init() noexcept override;
    void shutdown(const Lock& lock) noexcept final;

    ~Bidirectional() override = default;

private:
    using RawSocket = std::unique_ptr<void, decltype(&::zmq_close)>;

    static constexpr auto poll_milliseconds_{100};
    static constexpr auto callback_wait_milliseconds_{50};

    const bool bidirectional_start_thread_;
    const CString endpoint_;
    RawSocket push_socket_;
    RawSocket pull_socket_;
    mutable int linger_;
    mutable int send_timeout_;
    mutable int receive_timeout_;
    mutable std::mutex send_lock_;

    auto apply_timeouts(void* socket, std::mutex& socket_mutex) const noexcept
        -> bool;
    auto bind(
        void* socket,
        std::mutex& socket_mutex,
        const std::string_view endpoint) const noexcept -> bool;
    auto connect(
        void* socket,
        std::mutex& socket_mutex,
        const std::string_view endpoint) const noexcept -> bool;

    auto process_pull_socket(const Lock& lock) noexcept -> bool;
    auto process_receiver_socket(const Lock& lock) noexcept -> bool;
    auto Send(zeromq::Message&& message) const noexcept -> bool final;
    auto send(const Lock& lock, zeromq::Message&& message) noexcept -> bool;
    auto thread() noexcept -> void final;

    Bidirectional() = delete;
    Bidirectional(const Bidirectional&) = delete;
    Bidirectional(Bidirectional&&) = delete;
    auto operator=(const Bidirectional&) -> Bidirectional& = delete;
    auto operator=(Bidirectional&&) -> Bidirectional& = delete;
};
}  // namespace opentxs::network::zeromq::socket::implementation
