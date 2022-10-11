// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "network/zeromq/socket/Pull.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/socket/Factory.hpp"
#include "internal/network/zeromq/socket/Pull.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"
#include "internal/util/LogMacros.hpp"
#include "network/zeromq/curve/Server.hpp"
#include "network/zeromq/socket/Receiver.tpp"
#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Pimpl.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::socket::Pull>;

//"opentxs::network::zeromq::socket::implementation::Pull::"

namespace opentxs::factory
{
auto PullSocket(
    const network::zeromq::Context& context,
    const bool direction,
    const std::string_view threadname)
    -> std::unique_ptr<network::zeromq::socket::Pull>
{
    using ReturnType = network::zeromq::socket::implementation::Pull;

    return std::make_unique<ReturnType>(
        context,
        static_cast<network::zeromq::socket::Direction>(direction),
        threadname);
}

auto PullSocket(
    const network::zeromq::Context& context,
    const bool direction,
    const network::zeromq::ListenCallback& callback,
    const std::string_view threadname)
    -> std::unique_ptr<network::zeromq::socket::Pull>
{
    using ReturnType = network::zeromq::socket::implementation::Pull;

    return std::make_unique<ReturnType>(
        context,
        static_cast<network::zeromq::socket::Direction>(direction),
        callback,
        threadname);
}
}  // namespace opentxs::factory

namespace opentxs::network::zeromq::socket::implementation
{
Pull::Pull(
    const zeromq::Context& context,
    const Direction direction,
    const zeromq::ListenCallback& callback,
    const bool startThread,
    const std::string_view threadname) noexcept
    : Receiver(context, socket::Type::Pull, direction, startThread, threadname)
    , Server(this->get())
    , callback_(callback)
{
    init();
}

Pull::Pull(
    const zeromq::Context& context,
    const Direction direction,
    const zeromq::ListenCallback& callback,
    const std::string_view threadname) noexcept
    : Pull(context, direction, callback, true, threadname)
{
}

Pull::Pull(
    const zeromq::Context& context,
    const Direction direction,
    const std::string_view threadname) noexcept
    : Pull(context, direction, ListenCallback::Factory(), false, threadname)
{
}

auto Pull::clone() const noexcept -> Pull*
{
    return new Pull(context_, direction_, callback_);
}

auto Pull::have_callback() const noexcept -> bool { return true; }

auto Pull::process_incoming(const Lock& lock, Message&& message) noexcept
    -> void
{
    OT_ASSERT(verify_lock(lock));

    callback_.Process(std::move(message));
}

Pull::~Pull() SHUTDOWN_SOCKET
}  // namespace opentxs::network::zeromq::socket::implementation
