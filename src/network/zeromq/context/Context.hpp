// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/network/zeromq/socket/SocketType.hpp"

#pragma once

#include <cs_plain_guarded.h>
#include <functional>
#include <memory>
#include <optional>
#include <string_view>
#include <thread>

#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Handle.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/Proxy.hpp"
#include "internal/network/zeromq/Thread.hpp"
#include "internal/network/zeromq/socket/Dealer.hpp"
#include "internal/network/zeromq/socket/Pair.hpp"
#include "internal/network/zeromq/socket/Publish.hpp"
#include "internal/network/zeromq/socket/Pull.hpp"
#include "internal/network/zeromq/socket/Push.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/network/zeromq/socket/Reply.hpp"
#include "internal/network/zeromq/socket/Request.hpp"
#include "internal/network/zeromq/socket/Router.hpp"
#include "internal/network/zeromq/socket/Subscribe.hpp"
#include "internal/util/alloc/Logging.hpp"
#include "network/zeromq/context/Pool.hpp"
#include "opentxs/api/Log.internal.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/network/zeromq/Types.internal.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "util/ScopeGuard.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace internal
{
class Log;
}  // namespace internal

class Context;
class Session;
}  // namespace api

namespace network
{
namespace zeromq
{
namespace socket
{
class Socket;
}  // namespace socket

class Context;
class ListenCallback;
class Message;
class PairEventCallback;
class ReplyCallback;
}  // namespace zeromq
}  // namespace network

class Options;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::implementation
{
class Context final : virtual public internal::Context
{
public:
    operator void*() const noexcept final;

    auto ActiveBatches(alloc::Default alloc = {}) const noexcept
        -> CString final;
    auto Alloc(BatchID id) const noexcept -> alloc::Logging* final;
    auto BelongsToThreadPool(const std::thread::id) const noexcept
        -> bool final;
    auto DealerSocket(
        const ListenCallback& callback,
        const socket::Direction direction,
        const std::string_view threadname = {}) const noexcept
        -> OTZMQDealerSocket final;
    auto MakeBatch(Vector<socket::Type>&& types, std::string_view name)
        const noexcept -> internal::Handle final;
    auto MakeBatch(
        const BatchID preallocated,
        Vector<socket::Type>&& types,
        std::string_view name) const noexcept -> internal::Handle final;
    auto Modify(SocketID id, ModifyCallback cb) const noexcept -> void final;
    auto PairEventListener(
        const PairEventCallback& callback,
        const int instance,
        const std::string_view threadname = {}) const noexcept
        -> OTZMQSubscribeSocket final;
    auto PairSocket(
        const zeromq::ListenCallback& callback,
        const std::string_view threadname = {}) const noexcept
        -> OTZMQPairSocket final;
    auto PairSocket(
        const zeromq::ListenCallback& callback,
        const socket::Pair& peer,
        const std::string_view threadname = {}) const noexcept
        -> OTZMQPairSocket final;
    auto PairSocket(
        const zeromq::ListenCallback& callback,
        const std::string_view endpoint,
        const std::string_view threadname = {}) const noexcept
        -> OTZMQPairSocket final;
    auto Pipeline(
        std::function<void(zeromq::Message&&)>&& callback,
        const std::string_view threadname,
        socket::EndpointRequests::span subscribe,
        socket::EndpointRequests::span pull,
        socket::EndpointRequests::span dealer,
        socket::SocketRequests::span extra,
        socket::CurveClientRequests::span curveClient,
        socket::CurveServerRequests::span curveServer,
        const std::optional<BatchID>& preallocated,
        alloc::Default pmr) const noexcept -> zeromq::Pipeline final;
    auto PreallocateBatch() const noexcept -> BatchID final;
    auto Proxy(
        socket::Socket& frontend,
        socket::Socket& backend,
        const std::string_view threadname = {}) const noexcept
        -> OTZMQProxy final;
    auto PublishSocket() const noexcept -> OTZMQPublishSocket final;
    auto PullSocket(
        const socket::Direction direction,
        const std::string_view threadname = {}) const noexcept
        -> OTZMQPullSocket final;
    auto PullSocket(
        const ListenCallback& callback,
        const socket::Direction direction,
        const std::string_view threadname = {}) const noexcept
        -> OTZMQPullSocket final;
    auto PushSocket(const socket::Direction direction) const noexcept
        -> OTZMQPushSocket final;
    auto PushToEndpoint(std::string_view endpoint, Message&& message)
        const noexcept -> bool final;
    auto RawSocket(socket::Type type) const noexcept -> socket::Raw final;
    auto ReplySocket(
        const ReplyCallback& callback,
        const socket::Direction direction,
        const std::string_view threadname = {}) const noexcept
        -> OTZMQReplySocket final;
    auto RequestSocket() const noexcept -> OTZMQRequestSocket final;
    auto RouterSocket(
        const ListenCallback& callback,
        const socket::Direction direction,
        const std::string_view threadname = {}) const noexcept
        -> OTZMQRouterSocket final;
    auto SpawnActor(
        const api::Context& context,
        std::string_view name,
        actor::Startup startup,
        actor::Shutdown shutdown,
        actor::Processor processor,
        actor::StateMachine statemachine,
        socket::EndpointRequests subscribe,
        socket::EndpointRequests pull,
        socket::EndpointRequests dealer,
        socket::SocketRequests extra,
        socket::CurveClientRequests curveClient,
        socket::CurveServerRequests curveServer) const noexcept
        -> BatchID final;
    auto SpawnActor(
        const api::Session& context,
        std::string_view name,
        actor::Startup startup,
        actor::Shutdown shutdown,
        actor::Processor processor,
        actor::StateMachine statemachine,
        socket::EndpointRequests subscribe,
        socket::EndpointRequests pull,
        socket::EndpointRequests dealer,
        socket::SocketRequests extra,
        socket::CurveClientRequests curveClient,
        socket::CurveServerRequests curveServer) const noexcept
        -> BatchID final;
    auto Start(BatchID id, StartArgs&& sockets) const noexcept
        -> internal::Thread* final;
    auto Stop(BatchID id) const noexcept -> void final;
    auto SubscribeSocket(
        const ListenCallback& callback,
        const std::string_view threadname = {}) const noexcept
        -> OTZMQSubscribeSocket final;
    auto Thread(BatchID id) const noexcept -> internal::Thread* final;
    auto ThreadID(BatchID id) const noexcept -> std::thread::id final;

    auto Init(
        const opentxs::Options& args,
        std::shared_ptr<const zeromq::Context> me) noexcept -> void final;
    auto Stop() noexcept -> void final;

    Context(const opentxs::Options& args) noexcept;
    Context() = delete;
    Context(const Context&) = delete;
    Context(Context&&) = delete;
    auto operator=(const Context&) -> Context& = delete;
    auto operator=(Context&&) -> Context& = delete;

    ~Context() final;

private:
    using Pool = libguarded::plain_guarded<std::optional<context::Pool>>;
    using GuardedSocket = libguarded::plain_guarded<socket::Raw>;
    using SocketMap = Map<CString, GuardedSocket>;
    using GuardedSocketMap = libguarded::plain_guarded<SocketMap>;

    ScopeGuard post_;
    void* context_;
    api::internal::Log log_;
    mutable Pool pool_;
    mutable GuardedSocketMap push_sockets_;

    static auto max_sockets() noexcept -> int;
};
}  // namespace opentxs::network::zeromq::implementation
