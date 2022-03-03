// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <memory>
#include <string_view>

#include "opentxs/network/zeromq/socket/Dealer.hpp"
#include "opentxs/network/zeromq/socket/Pair.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Pull.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/network/zeromq/socket/Reply.hpp"
#include "opentxs/network/zeromq/socket/Request.hpp"
#include "opentxs/network/zeromq/socket/Router.hpp"
#include "opentxs/network/zeromq/socket/Socket.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace google
{
namespace protobuf
{
class MessageLite;
}  // namespace protobuf
}  // namespace google

namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace network
{
namespace zeromq
{
namespace internal
{
class Context;
}  // namespace internal

class Context;
class Frame;
class ListenCallback;
class Message;
class PairEventCallback;
class Proxy;
class ReplyCallback;
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq
{
class OPENTXS_EXPORT Context
{
public:
    virtual operator void*() const noexcept = 0;

    virtual auto DealerSocket(
        const ListenCallback& callback,
        const socket::Direction direction) const noexcept
        -> Pimpl<socket::Dealer> = 0;
    OPENTXS_NO_EXPORT virtual auto Internal() const noexcept
        -> const internal::Context& = 0;
    virtual auto PairEventListener(
        const PairEventCallback& callback,
        const int instance) const noexcept -> Pimpl<socket::Subscribe> = 0;
    virtual auto PairSocket(const ListenCallback& callback) const noexcept
        -> Pimpl<socket::Pair> = 0;
    virtual auto PairSocket(
        const ListenCallback& callback,
        const socket::Pair& peer) const noexcept -> Pimpl<socket::Pair> = 0;
    virtual auto PairSocket(
        const ListenCallback& callback,
        const std::string_view endpoint) const noexcept
        -> Pimpl<socket::Pair> = 0;
    virtual auto Proxy(socket::Socket& frontend, socket::Socket& backend)
        const noexcept -> Pimpl<zeromq::Proxy> = 0;
    virtual auto PublishSocket() const noexcept -> Pimpl<socket::Publish> = 0;
    virtual auto PullSocket(const socket::Direction direction) const noexcept
        -> Pimpl<socket::Pull> = 0;
    virtual auto PullSocket(
        const ListenCallback& callback,
        const socket::Direction direction) const noexcept
        -> Pimpl<socket::Pull> = 0;
    virtual auto PushSocket(const socket::Direction direction) const noexcept
        -> Pimpl<socket::Push> = 0;
    virtual auto ReplySocket(
        const ReplyCallback& callback,
        const socket::Direction direction) const noexcept
        -> Pimpl<socket::Reply> = 0;
    virtual auto RequestSocket() const noexcept -> Pimpl<socket::Request> = 0;
    virtual auto RouterSocket(
        const ListenCallback& callback,
        const socket::Direction direction) const noexcept
        -> Pimpl<socket::Router> = 0;
    virtual auto SubscribeSocket(const ListenCallback& callback) const noexcept
        -> Pimpl<socket::Subscribe> = 0;

    OPENTXS_NO_EXPORT virtual auto Internal() noexcept
        -> internal::Context& = 0;

    virtual ~Context() = default;

protected:
    Context() noexcept = default;

private:
    Context(const Context&) = delete;
    Context(Context&&) = delete;
    auto operator=(const Context&) -> Context& = delete;
    auto operator=(Context&&) -> Context& = delete;
};
}  // namespace opentxs::network::zeromq
