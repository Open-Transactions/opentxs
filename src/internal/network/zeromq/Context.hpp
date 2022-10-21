// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <future>
#include <memory>
#include <optional>
#include <string_view>
#include <thread>
#include <tuple>

#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/socket/Dealer.hpp"
#include "internal/network/zeromq/socket/Pair.hpp"
#include "internal/network/zeromq/socket/Publish.hpp"
#include "internal/network/zeromq/socket/Pull.hpp"
#include "internal/network/zeromq/socket/Push.hpp"
#include "internal/network/zeromq/socket/Reply.hpp"
#include "internal/network/zeromq/socket/Request.hpp"
#include "internal/network/zeromq/socket/Router.hpp"
#include "internal/network/zeromq/socket/Socket.hpp"
#include "internal/network/zeromq/socket/Subscribe.hpp"
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace zeromq
{
namespace internal
{
class Batch;
class Handle;
class Thread;
}  // namespace internal

namespace socket
{
class Raw;
}  // namespace socket

class Context;
class Frame;
class ListenCallback;
class Message;
class PairEventCallback;
class Pipeline;
class Proxy;
class ReplyCallback;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::zeromq::internal
{
class Context : virtual public zeromq::Context
{
public:
    virtual auto ActiveBatches(alloc::Default alloc = {}) const noexcept
        -> CString = 0;
    virtual auto Alloc(BatchID id) const noexcept -> alloc::Resource* = 0;
    virtual auto BelongsToThreadPool(
        const std::thread::id = std::this_thread::get_id()) const noexcept
        -> bool = 0;
    virtual auto DealerSocket(
        const ListenCallback& callback,
        const socket::Direction direction,
        const std::string_view threadname = {}) const noexcept
        -> Pimpl<socket::Dealer> = 0;
    auto Internal() const noexcept -> const internal::Context& final
    {
        return *this;
    }
    virtual auto MakeBatch(Vector<socket::Type>&& types, std::string_view name)
        const noexcept -> Handle = 0;
    virtual auto MakeBatch(
        const BatchID preallocated,
        Vector<socket::Type>&& types,
        std::string_view name) const noexcept -> Handle = 0;
    virtual auto Modify(SocketID id, ModifyCallback cb) const noexcept
        -> void = 0;
    virtual auto PairEventListener(
        const PairEventCallback& callback,
        const int instance,
        const std::string_view threadname = {}) const noexcept
        -> Pimpl<socket::Subscribe> = 0;
    virtual auto PairSocket(
        const ListenCallback& callback,
        const std::string_view threadname = {}) const noexcept
        -> Pimpl<socket::Pair> = 0;
    virtual auto PairSocket(
        const ListenCallback& callback,
        const socket::Pair& peer,
        const std::string_view threadname = {}) const noexcept
        -> Pimpl<socket::Pair> = 0;
    virtual auto PairSocket(
        const ListenCallback& callback,
        const std::string_view endpoint,
        const std::string_view threadname = {}) const noexcept
        -> Pimpl<socket::Pair> = 0;
    virtual auto PreallocateBatch() const noexcept -> BatchID = 0;
    virtual auto Pipeline(
        std::function<void(zeromq::Message&&)>&& callback,
        const std::string_view threadname,
        const EndpointArgs& subscribe = {},
        const EndpointArgs& pull = {},
        const EndpointArgs& dealer = {},
        const Vector<SocketData>& extra = {},
        const std::optional<BatchID>& preallocated = std::nullopt,
        alloc::Default pmr = {}) const noexcept -> zeromq::Pipeline = 0;
    virtual auto Proxy(
        socket::Socket& frontend,
        socket::Socket& backend,
        const std::string_view threadname = {}) const noexcept
        -> Pimpl<zeromq::Proxy> = 0;
    virtual auto PublishSocket() const noexcept -> Pimpl<socket::Publish> = 0;
    virtual auto PullSocket(
        const socket::Direction direction,
        const std::string_view threadname = {}) const noexcept
        -> Pimpl<socket::Pull> = 0;
    virtual auto PullSocket(
        const ListenCallback& callback,
        const socket::Direction direction,
        const std::string_view threadname = {}) const noexcept
        -> Pimpl<socket::Pull> = 0;
    virtual auto PushSocket(const socket::Direction direction) const noexcept
        -> Pimpl<socket::Push> = 0;
    virtual auto RawSocket(socket::Type type) const noexcept -> socket::Raw = 0;
    virtual auto ReplySocket(
        const ReplyCallback& callback,
        const socket::Direction direction,
        const std::string_view threadname = {}) const noexcept
        -> Pimpl<socket::Reply> = 0;
    virtual auto RequestSocket() const noexcept -> Pimpl<socket::Request> = 0;
    virtual auto RouterSocket(
        const ListenCallback& callback,
        const socket::Direction direction,
        const std::string_view threadname = {}) const noexcept
        -> Pimpl<socket::Router> = 0;
    virtual auto Start(BatchID id, StartArgs&& sockets) const noexcept
        -> Thread* = 0;
    virtual auto SubscribeSocket(
        const ListenCallback& callback,
        const std::string_view threadname = {}) const noexcept
        -> Pimpl<socket::Subscribe> = 0;
    virtual auto Thread(BatchID id) const noexcept -> Thread* = 0;
    virtual auto ThreadID(BatchID id) const noexcept -> std::thread::id = 0;

    virtual auto Init(std::shared_ptr<const zeromq::Context> me) noexcept
        -> void = 0;
    auto Internal() noexcept -> internal::Context& final { return *this; }
    virtual auto Stop() noexcept -> std::future<void> = 0;

    ~Context() override = default;

private:
    friend Handle;

    virtual auto Stop(BatchID id) const noexcept -> void = 0;
};
}  // namespace opentxs::network::zeromq::internal
