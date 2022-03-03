// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "1_Internal.hpp"                  // IWYU pragma: associated
#include "network/zeromq/zap/Handler.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/util/LogMacros.hpp"
#include "network/zeromq/curve/Server.hpp"
#include "network/zeromq/socket/Receiver.tpp"
#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/network/zeromq/zap/Callback.hpp"
#include "opentxs/network/zeromq/zap/Handler.hpp"
#include "opentxs/network/zeromq/zap/Reply.hpp"
#include "opentxs/network/zeromq/zap/Request.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::zap::Handler>;
template class opentxs::network::zeromq::socket::implementation::Receiver<
    opentxs::network::zeromq::zap::Request>;

namespace opentxs::network::zeromq::zap
{
auto Handler::Factory(
    const zeromq::Context& context,
    const zap::Callback& callback) -> OTZMQZAPHandler
{
    return OTZMQZAPHandler(new implementation::Handler(context, callback));
}
}  // namespace opentxs::network::zeromq::zap

namespace opentxs::network::zeromq::zap::implementation
{
Handler::Handler(
    const zeromq::Context& context,
    const zap::Callback& callback) noexcept
    : Receiver(context, socket::Type::Router, socket::Direction::Bind, true)
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

    OT_ASSERT(running);

    LogDetail()(OT_PRETTY_CLASS())("Listening on ")(endpoint).Flush();
}

auto Handler::process_incoming(
    const Lock& lock,
    zap::Request&& message) noexcept -> void
{
    send_message(lock, callback_.Process(std::move(message)));
}

Handler::~Handler() SHUTDOWN_SOCKET
}  // namespace opentxs::network::zeromq::zap::implementation
