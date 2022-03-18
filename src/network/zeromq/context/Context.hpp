// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <functional>
#include <future>
#include <iosfwd>
#include <optional>
#include <string_view>
#include <thread>
#include <tuple>

#include "Proto.hpp"
#include "internal/network/zeromq/Batch.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Handle.hpp"
#include "internal/network/zeromq/Thread.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "network/zeromq/context/Pool.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/Proxy.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
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
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
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
class Handle;
}  // namespace internal

namespace socket
{
class Socket;
}  // namespace socket

class Context;
class ListenCallback;
class Message;
class PairEventCallback;
class Pipeline;
class ReplyCallback;
}  // namespace zeromq
}  // namespace network

class Factory;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::implementation
{
class Context final : virtual public internal::Context
{
public:
    operator void*() const noexcept final;

    auto Alloc(BatchID id) const noexcept -> alloc::Resource* final;
    auto BelongsToThreadPool(const std::thread::id) const noexcept
        -> bool final;
    auto DealerSocket(
        const ListenCallback& callback,
        const socket::Direction direction) const noexcept
        -> OTZMQDealerSocket final;
    auto MakeBatch(Vector<socket::Type>&& types) const noexcept
        -> internal::Handle final;
    auto MakeBatch(const BatchID preallocated, Vector<socket::Type>&& types)
        const noexcept -> internal::Handle final;
    auto Modify(SocketID id, ModifyCallback cb) const noexcept
        -> AsyncResult final;
    auto PairEventListener(
        const PairEventCallback& callback,
        const int instance) const noexcept -> OTZMQSubscribeSocket final;
    auto PairSocket(const zeromq::ListenCallback& callback) const noexcept
        -> OTZMQPairSocket final;
    auto PairSocket(
        const zeromq::ListenCallback& callback,
        const socket::Pair& peer) const noexcept -> OTZMQPairSocket final;
    auto PairSocket(
        const zeromq::ListenCallback& callback,
        const std::string_view endpoint) const noexcept
        -> OTZMQPairSocket final;
    auto Pipeline(
        std::function<void(zeromq::Message&&)>&& callback,
        const EndpointArgs& subscribe,
        const EndpointArgs& pull,
        const EndpointArgs& dealer,
        const Vector<SocketData>& extra,
        const std::optional<BatchID>& preallocated,
        alloc::Resource* pmr) const noexcept -> zeromq::Pipeline final;
    auto PreallocateBatch() const noexcept -> BatchID final;
    auto Proxy(socket::Socket& frontend, socket::Socket& backend) const noexcept
        -> OTZMQProxy final;
    auto PublishSocket() const noexcept -> OTZMQPublishSocket final;
    auto PullSocket(const socket::Direction direction) const noexcept
        -> OTZMQPullSocket final;
    auto PullSocket(
        const ListenCallback& callback,
        const socket::Direction direction) const noexcept
        -> OTZMQPullSocket final;
    auto PushSocket(const socket::Direction direction) const noexcept
        -> OTZMQPushSocket final;
    auto RawSocket(socket::Type type) const noexcept -> socket::Raw final;
    auto ReplySocket(
        const ReplyCallback& callback,
        const socket::Direction direction) const noexcept
        -> OTZMQReplySocket final;
    auto RequestSocket() const noexcept -> OTZMQRequestSocket final;
    auto RouterSocket(
        const ListenCallback& callback,
        const socket::Direction direction) const noexcept
        -> OTZMQRouterSocket final;
    auto Start(BatchID id, StartArgs&& sockets) const noexcept
        -> internal::Thread* final;
    auto Stop(BatchID id) const noexcept -> std::future<bool> final;
    auto SubscribeSocket(const ListenCallback& callback) const noexcept
        -> OTZMQSubscribeSocket final;
    auto Thread(BatchID id) const noexcept -> internal::Thread* final;
    auto ThreadID(BatchID id) const noexcept -> std::thread::id final;

    Context() noexcept;

    ~Context() final;

private:
    void* context_;
    mutable context::Pool pool_;

    static auto max_sockets() noexcept -> int;

    Context(const Context&) = delete;
    Context(Context&&) = delete;
    auto operator=(const Context&) -> Context& = delete;
    auto operator=(Context&&) -> Context& = delete;
};
}  // namespace opentxs::network::zeromq::implementation
