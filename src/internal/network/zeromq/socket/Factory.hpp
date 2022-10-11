// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <optional>
#include <string_view>

#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Allocator.hpp"
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
class Pair;
class Publish;
class Pull;
class Push;
class Raw;
class Reply;
class Request;
class Router;
class Subscribe;
}  // namespace socket

class Context;
class ListenCallback;
class Message;
class Pipeline;
class ReplyCallback;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto DealerSocket(
    const network::zeromq::Context& context,
    const bool direction,
    const network::zeromq::ListenCallback& callback,
    const std::string_view threadname = {})
    -> std::unique_ptr<network::zeromq::socket::Dealer>;
auto PairSocket(
    const network::zeromq::Context& context,
    const network::zeromq::ListenCallback& callback,
    const bool startThread,
    const std::string_view threadname = {})
    -> std::unique_ptr<network::zeromq::socket::Pair>;
auto PairSocket(
    const network::zeromq::ListenCallback& callback,
    const network::zeromq::socket::Pair& peer,
    const bool startThread,
    const std::string_view threadname = {})
    -> std::unique_ptr<network::zeromq::socket::Pair>;
auto PairSocket(
    const network::zeromq::Context& context,
    const network::zeromq::ListenCallback& callback,
    const std::string_view endpoint,
    const std::string_view threadname = {})
    -> std::unique_ptr<network::zeromq::socket::Pair>;
auto Pipeline(
    const network::zeromq::Context& context,
    std::function<void(network::zeromq::Message&&)>&& callback,
    const network::zeromq::EndpointArgs& subscribe,
    const network::zeromq::EndpointArgs& pull,
    const network::zeromq::EndpointArgs& dealer,
    const Vector<network::zeromq::SocketData>& extra,
    const std::string_view threadname,
    const std::optional<network::zeromq::BatchID>& preallocated,
    alloc::Default pmr) noexcept -> opentxs::network::zeromq::Pipeline;
auto PublishSocket(const network::zeromq::Context& context)
    -> std::unique_ptr<network::zeromq::socket::Publish>;
auto PullSocket(
    const network::zeromq::Context& context,
    const bool direction,
    const std::string_view threadname = {})
    -> std::unique_ptr<network::zeromq::socket::Pull>;
auto PullSocket(
    const network::zeromq::Context& context,
    const bool direction,
    const network::zeromq::ListenCallback& callback,
    const std::string_view threadname = {})
    -> std::unique_ptr<network::zeromq::socket::Pull>;
auto PushSocket(const network::zeromq::Context& context, const bool direction)
    -> std::unique_ptr<network::zeromq::socket::Push>;
auto ReplySocket(
    const network::zeromq::Context& context,
    const bool direction,
    const network::zeromq::ReplyCallback& callback,
    const std::string_view threadname = {})
    -> std::unique_ptr<network::zeromq::socket::Reply>;
auto RequestSocket(const network::zeromq::Context& context)
    -> std::unique_ptr<network::zeromq::socket::Request>;
auto RouterSocket(
    const network::zeromq::Context& context,
    const bool direction,
    const network::zeromq::ListenCallback& callback,
    const std::string_view threadname = {})
    -> std::unique_ptr<network::zeromq::socket::Router>;
auto SubscribeSocket(
    const network::zeromq::Context& context,
    const network::zeromq::ListenCallback& callback,
    const std::string_view threadname = {})
    -> std::unique_ptr<network::zeromq::socket::Subscribe>;
auto ZMQSocket(
    const network::zeromq::Context& context,
    const network::zeromq::socket::Type type) noexcept
    -> network::zeromq::socket::Raw;
auto ZMQSocketNull() noexcept -> network::zeromq::socket::Raw;
}  // namespace opentxs::factory
