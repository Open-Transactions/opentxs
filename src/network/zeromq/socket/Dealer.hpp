// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <ostream>
#include <string_view>

#include "internal/util/Mutex.hpp"
#include "network/zeromq/curve/Client.hpp"
#include "network/zeromq/socket/Bidirectional.hpp"
#include "network/zeromq/socket/Receiver.tpp"
#include "network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/socket/Dealer.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace socket
{
class Dealer;
}  // namespace socket

class Context;
class ListenCallback;
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::socket::implementation
{
class Dealer final : public Bidirectional<zeromq::socket::Dealer>,
                     public zeromq::curve::implementation::Client
{
public:
    auto SetSocksProxy(const UnallocatedCString& proxy) const noexcept
        -> bool final
    {
        return set_socks_proxy(proxy);
    }

    Dealer(
        const zeromq::Context& context,
        const Direction direction,
        const zeromq::ListenCallback& callback,
        const std::string_view threadname = "Dealer") noexcept;
    Dealer() = delete;
    Dealer(const Dealer&) = delete;
    Dealer(Dealer&&) = delete;
    auto operator=(const Dealer&) -> Dealer& = delete;
    auto operator=(Dealer&&) -> Dealer& = delete;

    ~Dealer() final;

private:
    const ListenCallback& callback_;

    auto clone() const noexcept -> Dealer* final;
    auto have_callback() const noexcept -> bool final { return true; }

    void process_incoming(const Lock& lock, Message&& message) noexcept final;
};
}  // namespace opentxs::network::zeromq::socket::implementation
