// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/zeromq/socket/Publish.hpp"  // IWYU pragma: associated

#include <memory>

#include "internal/network/zeromq/socket/Factory.hpp"
#include "internal/network/zeromq/socket/Publish.hpp"
#include "internal/util/Pimpl.hpp"
#include "network/zeromq/curve/Server.hpp"
#include "network/zeromq/socket/Sender.tpp"
#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/socket/Direction.hpp"   // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Types.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::socket::Publish>;

namespace opentxs::factory
{
auto PublishSocket(const opentxs::network::zeromq::Context& context)
    -> std::unique_ptr<network::zeromq::socket::Publish>
{
    using ReturnType = network::zeromq::socket::implementation::Publish;

    return std::make_unique<ReturnType>(context);
}
}  // namespace opentxs::factory

namespace opentxs::network::zeromq::socket::implementation
{
Publish::Publish(const zeromq::Context& context) noexcept
    : Socket(context, socket::Type::Publish, Direction::Bind)
    , Sender()
    , Server(this->get())
{
    init();
}

Publish::~Publish() SHUTDOWN_SOCKET
}  // namespace opentxs::network::zeromq::socket::implementation
