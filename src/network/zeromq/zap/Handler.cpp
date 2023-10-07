// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/zeromq/zap/Handler.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/network/zeromq/zap/Callback.hpp"
#include "internal/network/zeromq/zap/Handler.hpp"
#include "internal/network/zeromq/zap/Reply.hpp"
#include "internal/network/zeromq/zap/Request.hpp"
#include "internal/util/Pimpl.hpp"
#include "network/zeromq/curve/Server.hpp"
#include "network/zeromq/socket/Receiver.tpp"
#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/socket/Direction.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::zap::Handler>;
template class opentxs::network::zeromq::socket::implementation::Receiver<
    opentxs::network::zeromq::zap::Request>;

namespace opentxs::network::zeromq::zap
{
auto Handler::Factory(
    const zeromq::Context& context,
    const zap::Callback& callback,
    const std::string_view threadname) -> OTZMQZAPHandler
{
    return OTZMQZAPHandler(
        new implementation::Handler(context, callback, threadname));
}
}  // namespace opentxs::network::zeromq::zap

namespace opentxs::network::zeromq::zap::implementation
{
Handler::Handler(
    const zeromq::Context& context,
    const zap::Callback& callback,
    const std::string_view threadname) noexcept
    : Receiver(
          context,
          socket::Type::Router,
          socket::Direction::Bind,
          true,
          threadname.empty() ? "Handler" : CString{threadname} + " handler")
    , Server(this->get())
    , callback_(callback)
{
    init();
}

auto Handler::init() noexcept -> void
{
    static constexpr auto endpoint{"inproc://zeromq.zap.01"};
    Receiver::init();
    const auto running = Receiver::Start(endpoint);

    assert_true(running);

    LogDetail()()("Listening on ")(endpoint).Flush();
}

auto Handler::process_incoming(
    const Lock& lock,
    zap::Request&& message) noexcept -> void
{
    send_message(lock, callback_.Process(std::move(message)));
}

Handler::~Handler() SHUTDOWN_SOCKET
}  // namespace opentxs::network::zeromq::zap::implementation
