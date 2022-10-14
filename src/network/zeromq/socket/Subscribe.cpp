// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "network/zeromq/socket/Subscribe.hpp"  // IWYU pragma: associated

#include <zmq.h>
#include <memory>
#include <stdexcept>
#include <utility>

#include "internal/network/zeromq/ListenCallback.hpp"
#include "internal/network/zeromq/socket/Factory.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"
#include "internal/network/zeromq/socket/Subscribe.hpp"
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Pimpl.hpp"
#include "network/zeromq/socket/Receiver.tpp"
#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

template class opentxs::Pimpl<opentxs::network::zeromq::socket::Subscribe>;

namespace opentxs::factory
{
auto SubscribeSocket(
    const network::zeromq::Context& context,
    const network::zeromq::ListenCallback& callback,
    const std::string_view threadname)
    -> std::unique_ptr<network::zeromq::socket::Subscribe>
{
    using ReturnType = network::zeromq::socket::implementation::Subscribe;

    return std::make_unique<ReturnType>(context, callback, threadname);
}
}  // namespace opentxs::factory

namespace opentxs::network::zeromq::socket::implementation
{
Subscribe::Subscribe(
    const zeromq::Context& context,
    const zeromq::ListenCallback& callback,
    const std::string_view threadname) noexcept
    : Receiver(
          context,
          socket::Type::Subscribe,
          Direction::Connect,
          true,
          threadname.empty() ? "Subscribe" : CString{threadname} + " subscribe")
    , Client(this->get())
    , callback_(callback)
{
    init();
}

auto Subscribe::clone() const noexcept -> Subscribe*
{
    return new Subscribe(context_, callback_);
}

auto Subscribe::have_callback() const noexcept -> bool { return true; }

void Subscribe::init() noexcept
{
    // subscribe to all messages until filtering is implemented
    const auto set = zmq_setsockopt(socket_, ZMQ_SUBSCRIBE, "", 0);
    Receiver::init();

    OT_ASSERT(0 == set);
}

void Subscribe::process_incoming(const Lock& lock, Message&& message) noexcept
{
    OT_ASSERT(verify_lock(lock));

    try {
        callback_.Process(std::move(message));
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())("Callback exception: ")(e.what()).Flush();

        for (const auto& endpoint : endpoints_) {
            LogError()(OT_PRETTY_CLASS())("connected endpoint: ")(endpoint)
                .Flush();
        }
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("Callback exception").Flush();
    }
}

auto Subscribe::SetSocksProxy(const UnallocatedCString& proxy) const noexcept
    -> bool
{
    return set_socks_proxy(proxy);
}

Subscribe::~Subscribe() SHUTDOWN_SOCKET
}  // namespace opentxs::network::zeromq::socket::implementation
