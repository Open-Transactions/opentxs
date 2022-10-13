// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "network/zeromq/context/Context.hpp"  // IWYU pragma: associated

#include <zmq.h>
#include <atomic>
#include <exception>
#include <memory>
#include <thread>
#include <utility>

#include "internal/api/Factory.hpp"
#include "internal/api/Log.hpp"
#include "internal/network/zeromq/Factory.hpp"
#include "internal/network/zeromq/Handle.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/Proxy.hpp"
#include "internal/network/zeromq/socket/Dealer.hpp"
#include "internal/network/zeromq/socket/Factory.hpp"
#include "internal/network/zeromq/socket/Pair.hpp"
#include "internal/network/zeromq/socket/Publish.hpp"
#include "internal/network/zeromq/socket/Pull.hpp"
#include "internal/network/zeromq/socket/Push.hpp"
#include "internal/network/zeromq/socket/Reply.hpp"
#include "internal/network/zeromq/socket/Request.hpp"
#include "internal/network/zeromq/socket/Router.hpp"
#include "internal/network/zeromq/socket/Subscribe.hpp"
#include "network/zeromq/PairEventListener.hpp"
#include "opentxs/util/Options.hpp"

namespace opentxs::factory
{
auto ZMQContext(const opentxs::Options& args) noexcept
    -> std::shared_ptr<network::zeromq::Context>
{
    using ReturnType = network::zeromq::implementation::Context;

    return std::make_shared<ReturnType>(args);
}
}  // namespace opentxs::factory

namespace opentxs::network::zeromq
{
auto GetBatchID() noexcept -> BatchID
{
    static auto counter = std::atomic<BatchID>{};

    return ++counter;
}

auto GetSocketID() noexcept -> SocketID
{
    static auto counter = std::atomic<SocketID>{};

    return ++counter;
}
}  // namespace opentxs::network::zeromq

namespace opentxs::network::zeromq::implementation
{
Context::Context(const opentxs::Options& args) noexcept
    : context_([] {
        auto* context = ::zmq_ctx_new();

        if (nullptr == context) { std::terminate(); }
        if (1 != ::zmq_has("curve")) { std::terminate(); }

        const auto init =
            ::zmq_ctx_set(context, ZMQ_MAX_SOCKETS, max_sockets());

        if (0 != init) { std::terminate(); }

        return context;
    }())
    , log_(factory::Log(*this, args.RemoteLogEndpoint()))
    , pool_(std::nullopt)
    , shutdown_()
{
    if (nullptr == context_) { std::terminate(); }
    if (false == log_.operator bool()) { std::terminate(); }
}

Context::operator void*() const noexcept
{
    if (nullptr == context_) { std::terminate(); }

    return context_;
}

auto Context::ActiveBatches(alloc::Default alloc) const noexcept -> CString
{
    auto handle = pool_.lock();
    auto& pool = *handle;

    if (false == pool.has_value()) { std::terminate(); }

    return pool->ActiveBatches(std::move(alloc));
}

auto Context::Alloc(BatchID id) const noexcept -> alloc::Resource*
{
    auto handle = pool_.lock();
    auto& pool = *handle;

    if (false == pool.has_value()) { std::terminate(); }

    return pool->Alloc(id);
}

auto Context::BelongsToThreadPool(const std::thread::id id) const noexcept
    -> bool
{
    auto handle = pool_.lock();
    auto& pool = *handle;

    if (false == pool.has_value()) { std::terminate(); }

    return pool->BelongsToThreadPool(id);
}

auto Context::DealerSocket(
    const ListenCallback& callback,
    const socket::Direction direction,
    const std::string_view threadname) const noexcept -> OTZMQDealerSocket
{
    return OTZMQDealerSocket{factory::DealerSocket(
        *this, static_cast<bool>(direction), callback, threadname)};
}

auto Context::Init(std::shared_ptr<const zeromq::Context> me) noexcept -> void
{
    auto handle = pool_.lock();
    auto& pool = *handle;

    if (pool.has_value()) { std::terminate(); }

    pool.emplace(std::move(me));

    if (false == pool.has_value()) { std::terminate(); }
}

auto Context::max_sockets() noexcept -> int { return 32768; }

auto Context::MakeBatch(Vector<socket::Type>&& types, std::string_view name)
    const noexcept -> internal::Handle
{
    auto handle = pool_.lock();
    auto& pool = *handle;

    if (false == pool.has_value()) { std::terminate(); }

    return pool->MakeBatch(std::move(types), name);
}

auto Context::MakeBatch(
    const BatchID preallocated,
    Vector<socket::Type>&& types,
    std::string_view name) const noexcept -> internal::Handle
{
    auto handle = pool_.lock();
    auto& pool = *handle;

    if (false == pool.has_value()) { std::terminate(); }

    return pool->MakeBatch(preallocated, std::move(types), name);
}

auto Context::Modify(SocketID id, ModifyCallback cb) const noexcept -> void
{
    auto handle = pool_.lock();
    auto& pool = *handle;

    if (false == pool.has_value()) { std::terminate(); }

    pool->Modify(id, std::move(cb));
}

auto Context::PairEventListener(
    const PairEventCallback& callback,
    const int instance,
    const std::string_view threadname) const noexcept -> OTZMQSubscribeSocket
{
    return OTZMQSubscribeSocket(
        new class PairEventListener(*this, callback, instance, threadname));
}

auto Context::PairSocket(
    const zeromq::ListenCallback& callback,
    const std::string_view threadname) const noexcept -> OTZMQPairSocket
{
    return OTZMQPairSocket{
        factory::PairSocket(*this, callback, true, threadname)};
}

auto Context::PairSocket(
    const zeromq::ListenCallback& callback,
    const socket::Pair& peer,
    const std::string_view threadname) const noexcept -> OTZMQPairSocket
{
    return OTZMQPairSocket{
        factory::PairSocket(callback, peer, true, threadname)};
}

auto Context::PairSocket(
    const zeromq::ListenCallback& callback,
    const std::string_view endpoint,
    const std::string_view threadname) const noexcept -> OTZMQPairSocket
{
    return OTZMQPairSocket{
        factory::PairSocket(*this, callback, endpoint, threadname)};
}

auto Context::Pipeline(
    std::function<void(zeromq::Message&&)>&& callback,
    const std::string_view threadname,
    const EndpointArgs& subscribe,
    const EndpointArgs& pull,
    const EndpointArgs& dealer,
    const Vector<SocketData>& extra,
    const std::optional<BatchID>& preallocated,
    alloc::Default pmr) const noexcept -> zeromq::Pipeline
{
    return opentxs::factory::Pipeline(
        *this,
        std::move(callback),
        subscribe,
        pull,
        dealer,
        extra,
        threadname,
        preallocated,
        pmr);
}

auto Context::PreallocateBatch() const noexcept -> BatchID
{
    auto handle = pool_.lock();
    auto& pool = *handle;

    if (false == pool.has_value()) { std::terminate(); }

    return pool->PreallocateBatch();
}

auto Context::Proxy(
    socket::Socket& frontend,
    socket::Socket& backend,
    const std::string_view threadname) const noexcept -> OTZMQProxy
{
    return zeromq::Proxy::Factory(*this, frontend, backend, threadname);
}

auto Context::PublishSocket() const noexcept -> OTZMQPublishSocket
{
    return OTZMQPublishSocket{factory::PublishSocket(*this)};
}

auto Context::PullSocket(
    const socket::Direction direction,
    const std::string_view threadname) const noexcept -> OTZMQPullSocket
{
    return OTZMQPullSocket{
        factory::PullSocket(*this, static_cast<bool>(direction), threadname)};
}

auto Context::PullSocket(
    const ListenCallback& callback,
    const socket::Direction direction,
    const std::string_view threadname) const noexcept -> OTZMQPullSocket
{
    return OTZMQPullSocket{factory::PullSocket(
        *this, static_cast<bool>(direction), callback, threadname)};
}

auto Context::PushSocket(const socket::Direction direction) const noexcept
    -> OTZMQPushSocket
{
    return OTZMQPushSocket{
        factory::PushSocket(*this, static_cast<bool>(direction))};
}

auto Context::RawSocket(socket::Type type) const noexcept -> socket::Raw
{
    return factory::ZMQSocket(*this, type);
}

auto Context::ReplySocket(
    const ReplyCallback& callback,
    const socket::Direction direction,
    const std::string_view threadname) const noexcept -> OTZMQReplySocket
{
    return OTZMQReplySocket{factory::ReplySocket(
        *this, static_cast<bool>(direction), callback, threadname)};
}

auto Context::RequestSocket() const noexcept -> OTZMQRequestSocket
{
    return OTZMQRequestSocket{factory::RequestSocket(*this)};
}

auto Context::RouterSocket(
    const ListenCallback& callback,
    const socket::Direction direction,
    const std::string_view threadname) const noexcept -> OTZMQRouterSocket
{
    return OTZMQRouterSocket{factory::RouterSocket(
        *this, static_cast<bool>(direction), callback, threadname)};
}

auto Context::Start(BatchID id, StartArgs&& sockets) const noexcept
    -> internal::Thread*
{
    auto handle = pool_.lock();
    auto& pool = *handle;

    if (false == pool.has_value()) { std::terminate(); }

    return pool->Start(id, std::move(sockets));
}

auto Context::Stop(BatchID id) const noexcept -> void
{
    auto handle = pool_.lock();
    auto& pool = *handle;

    if (false == pool.has_value()) { std::terminate(); }

    pool->Stop(id);
}

auto Context::Stop() noexcept -> std::future<void>
{
    auto handle = pool_.lock();
    auto& pool = *handle;

    if (false == pool.has_value()) { std::terminate(); }

    pool->Shutdown();

    return shutdown_.get_future();
}

auto Context::SubscribeSocket(
    const ListenCallback& callback,
    const std::string_view threadname) const noexcept -> OTZMQSubscribeSocket
{
    return OTZMQSubscribeSocket{
        factory::SubscribeSocket(*this, callback, threadname)};
}

auto Context::Thread(BatchID id) const noexcept -> internal::Thread*
{
    auto handle = pool_.lock();
    auto& pool = *handle;

    if (false == pool.has_value()) { std::terminate(); }

    return pool->Thread(id);
}

auto Context::ThreadID(BatchID id) const noexcept -> std::thread::id
{
    auto handle = pool_.lock();
    auto& pool = *handle;

    if (false == pool.has_value()) { std::terminate(); }

    return pool->ThreadID(id);
}

Context::~Context()
{
    log_.reset();

    if (nullptr != context_) {
        std::thread{[context = context_] {
            // NOTE neither of these functions should block forever but
            // sometimes they do anyway
            ::zmq_ctx_shutdown(context);
            ::zmq_ctx_term(context);
        }}.detach();
        context_ = nullptr;
        shutdown_.set_value();
    }
}
}  // namespace opentxs::network::zeromq::implementation
