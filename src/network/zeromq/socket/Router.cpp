// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/zeromq/socket/Router.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/message/Message.hpp"
#include "internal/network/zeromq/socket/Factory.hpp"
#include "internal/network/zeromq/socket/Router.hpp"
#include "internal/util/Pimpl.hpp"
#include "network/zeromq/curve/Client.hpp"
#include "network/zeromq/curve/Server.hpp"
#include "network/zeromq/socket/Bidirectional.tpp"
#include "network/zeromq/socket/Receiver.hpp"
#include "network/zeromq/socket/Sender.tpp"
#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "opentxs/util/Log.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::socket::Router>;

namespace opentxs::factory
{
auto RouterSocket(
    const network::zeromq::Context& context,
    const bool direction,
    const network::zeromq::ListenCallback& callback,
    const std::string_view threadname)
    -> std::unique_ptr<network::zeromq::socket::Router>
{
    using ReturnType = network::zeromq::socket::implementation::Router;

    return std::make_unique<ReturnType>(
        context,
        static_cast<network::zeromq::socket::Direction>(direction),
        callback,
        threadname);
}
}  // namespace opentxs::factory

namespace opentxs::network::zeromq::socket::implementation
{
Router::Router(
    const zeromq::Context& context,
    const Direction direction,
    const zeromq::ListenCallback& callback,
    const std::string_view threadname) noexcept
    : Receiver(
          context,
          socket::Type::Router,
          direction,
          false,
          CString{threadname} + " router")
    , Bidirectional(context, true, CString{threadname} + " router")
    , Client(this->get())
    , Server(this->get())
    , callback_(callback)
{
    init();
}

auto Router::clone() const noexcept -> Router*
{
    return new Router(context_, direction_, callback_);
}

void Router::process_incoming(const Lock& lock, Message&& message) noexcept
{
    assert_true(verify_lock(lock));

    LogTrace()()("Incoming messaged received. Triggering callback.").Flush();
    // Router prepends an identity frame to the message.  This makes sure
    // there is an empty frame between the identity frame(s) and the frames that
    // make up the rest of the message.
    message.Internal().EnsureDelimiter();
    callback_.Process(std::move(message));
    LogTrace()()("Done.").Flush();
}

Router::~Router() SHUTDOWN_SOCKET
}  // namespace opentxs::network::zeromq::socket::implementation
