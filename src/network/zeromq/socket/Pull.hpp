// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "internal/network/zeromq/socket/Pull.hpp"  // IWYU pragma: keep
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/util/Mutex.hpp"
#include "network/zeromq/curve/Server.hpp"
#include "network/zeromq/socket/Receiver.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
class Context;
class ListenCallback;
class Message;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::socket::implementation
{
class Pull final : public Receiver<zeromq::socket::Pull>,
                   public zeromq::curve::implementation::Server
{
public:
    Pull(
        const zeromq::Context& context,
        const Direction direction,
        const zeromq::ListenCallback& callback,
        const bool startThread,
        const std::string_view threadname = "Pull") noexcept;
    Pull(
        const zeromq::Context& context,
        const Direction direction,
        const zeromq::ListenCallback& callback,
        const std::string_view threadname = "Pull") noexcept;
    Pull(
        const zeromq::Context& context,
        const Direction direction,
        const std::string_view threadname = "Pull") noexcept;
    Pull() = delete;
    Pull(const Pull&) = delete;
    Pull(Pull&&) = delete;
    auto operator=(const Pull&) -> Pull& = delete;
    auto operator=(Pull&&) -> Pull& = delete;

    ~Pull() final;

private:
    const ListenCallback& callback_;

    auto clone() const noexcept -> Pull* final;
    auto have_callback() const noexcept -> bool final;

    auto process_incoming(const Lock& lock, Message&& message) noexcept
        -> void final;
};
}  // namespace opentxs::network::zeromq::socket::implementation
