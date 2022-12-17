// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/zeromq/socket/Reply.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "internal/network/zeromq/ReplyCallback.hpp"
#include "internal/network/zeromq/socket/Factory.hpp"
#include "internal/network/zeromq/socket/Reply.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/util/Pimpl.hpp"
#include "network/zeromq/curve/Server.hpp"
#include "network/zeromq/socket/Receiver.tpp"
#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::socket::Reply>;
template class opentxs::network::zeromq::socket::implementation::Receiver<
    opentxs::network::zeromq::Message>;

namespace opentxs::factory
{
auto ReplySocket(
    const network::zeromq::Context& context,
    const bool direction,
    const network::zeromq::ReplyCallback& callback,
    const std::string_view threadname)
    -> std::unique_ptr<network::zeromq::socket::Reply>
{
    using ReturnType = network::zeromq::socket::implementation::Reply;

    return std::make_unique<ReturnType>(
        context,
        static_cast<network::zeromq::socket::Direction>(direction),
        callback,
        threadname);
}
}  // namespace opentxs::factory

namespace opentxs::network::zeromq::socket::implementation
{
Reply::Reply(
    const zeromq::Context& context,
    const Direction direction,
    const ReplyCallback& callback,
    const std::string_view threadname) noexcept
    : Receiver(
          context,
          socket::Type::Reply,
          direction,
          true,
          threadname.empty() ? "Reply" : CString(threadname) + " reply")
    , Server(this->get())
    , callback_(callback)
{
    init();
}

auto Reply::clone() const noexcept -> Reply*
{
    return new Reply(context_, direction_, callback_);
}

auto Reply::have_callback() const noexcept -> bool { return true; }

auto Reply::process_incoming(const Lock& lock, Message&& message) noexcept
    -> void
{
    auto output = callback_.Process(std::move(message));
    Message& reply = output;
    send_message(lock, std::move(reply));
}

Reply::~Reply() SHUTDOWN_SOCKET
}  // namespace opentxs::network::zeromq::socket::implementation
