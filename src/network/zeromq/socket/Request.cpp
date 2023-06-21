// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/zeromq/socket/Request.hpp"  // IWYU pragma: associated

#include <zmq.h>
#include <array>
#include <atomic>
#include <chrono>
#include <compare>
#include <memory>
#include <thread>
#include <utility>

#include "internal/network/zeromq/socket/Factory.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Pimpl.hpp"
#include "network/zeromq/curve/Client.hpp"
#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Direction.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::socket::Request>;

namespace opentxs::factory
{
auto RequestSocket(const network::zeromq::Context& context)
    -> std::unique_ptr<network::zeromq::socket::Request>
{
    using ReturnType = network::zeromq::socket::implementation::Request;

    return std::make_unique<ReturnType>(context);
}
}  // namespace opentxs::factory

namespace opentxs::network::zeromq::socket::implementation
{
Request::Request(const zeromq::Context& context) noexcept
    : Socket(context, socket::Type::Request, Direction::Connect)
    , Client(this->get())
{
    init();
}

auto Request::clone() const noexcept -> Request*
{
    return new Request(context_);
}

auto Request::Send(zeromq::Message&& request) const noexcept
    -> Socket::SendResult
{
    OT_ASSERT(nullptr != socket_);

    Lock lock(lock_);
    auto output = SendResult{otx::client::SendResult::Error, {}};
    auto& [status, reply] = output;

    if (false == send_message(lock, std::move(request))) {
        LogError()(OT_PRETTY_CLASS())("Failed to deliver message.").Flush();

        return output;
    }

    if (false == wait(lock)) {
        LogVerbose()(OT_PRETTY_CLASS())("Receive timeout.").Flush();
        status = otx::client::SendResult::TIMEOUT;

        return output;
    }

    if (receive_message(lock, reply)) {
        status = otx::client::SendResult::VALID_REPLY;
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to receive reply.").Flush();
    }

    return output;
}

auto Request::SetSocksProxy(const UnallocatedCString& proxy) const noexcept
    -> bool
{
    return set_socks_proxy(proxy);
}

auto Request::wait(const Lock& lock) const noexcept -> bool
{
    OT_ASSERT(verify_lock(lock));

    const auto start = Clock::now();
    auto poll = std::array<::zmq_pollitem_t, 1>{};
    poll[0].socket = socket_;
    poll[0].events = ZMQ_POLLIN;

    while (running_.get()) {
        std::this_thread::yield();
        const auto events = ::zmq_poll(poll.data(), 1, poll_milliseconds_);

        if (0 == events) {
            LogVerbose()(OT_PRETTY_CLASS())("No messages.").Flush();

            const auto now = Clock::now();

            if ((now - start) > std::chrono::milliseconds(receive_timeout_)) {

                return false;
            } else {
                continue;
            }
        }

        if (0 > events) {
            const auto error = zmq_errno();
            LogError()(OT_PRETTY_CLASS())("Poll error: ")(zmq_strerror(error))(
                ".")
                .Flush();

            continue;  // TODO
        }

        return true;
    }

    return false;
}

Request::~Request() SHUTDOWN_SOCKET
}  // namespace opentxs::network::zeromq::socket::implementation
