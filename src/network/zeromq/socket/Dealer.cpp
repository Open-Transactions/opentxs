// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "1_Internal.hpp"                    // IWYU pragma: associated
#include "network/zeromq/socket/Dealer.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "internal/network/zeromq/socket/Factory.hpp"
#include "internal/util/LogMacros.hpp"
#include "network/zeromq/curve/Client.hpp"
#include "network/zeromq/socket/Bidirectional.tpp"
#include "network/zeromq/socket/Receiver.hpp"
#include "network/zeromq/socket/Receiver.tpp"
#include "network/zeromq/socket/Sender.tpp"
#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Dealer.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::socket::Dealer>;

namespace opentxs::factory
{
auto DealerSocket(
    const network::zeromq::Context& context,
    const bool direction,
    const network::zeromq::ListenCallback& callback)
    -> std::unique_ptr<network::zeromq::socket::Dealer>
{
    using ReturnType = network::zeromq::socket::implementation::Dealer;

    return std::make_unique<ReturnType>(
        context,
        static_cast<network::zeromq::socket::Direction>(direction),
        callback);
}
}  // namespace opentxs::factory

namespace opentxs::network::zeromq::socket::implementation
{
Dealer::Dealer(
    const zeromq::Context& context,
    const Direction direction,
    const zeromq::ListenCallback& callback) noexcept
    : Receiver(context, socket::Type::Dealer, direction, false)
    , Bidirectional(context, true)
    , Client(this->get())
    , callback_(callback)
{
    init();
}

auto Dealer::clone() const noexcept -> Dealer*
{
    return new Dealer(context_, direction_, callback_);
}

void Dealer::process_incoming(
    const Lock& lock,
    opentxs::network::zeromq::Message&& message) noexcept
{
    OT_ASSERT(verify_lock(lock))

    LogTrace()(OT_PRETTY_CLASS())(
        "Incoming messaged received. Triggering callback.")
        .Flush();
    callback_.Process(std::move(message));
    LogTrace()(OT_PRETTY_CLASS())("Done.").Flush();
}

Dealer::~Dealer() SHUTDOWN_SOCKET
}  // namespace opentxs::network::zeromq::socket::implementation
