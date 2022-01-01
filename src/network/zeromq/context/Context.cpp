// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "network/zeromq/context/Context.hpp"  // IWYU pragma: associated

#include <zmq.h>
#include <atomic>
#include <cassert>
#include <memory>
#include <utility>

#include "internal/network/zeromq/Factory.hpp"
#include "internal/network/zeromq/socket/Factory.hpp"
#include "network/zeromq/PairEventListener.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/Proxy.hpp"
#include "opentxs/network/zeromq/socket/Dealer.hpp"
#include "opentxs/network/zeromq/socket/Pair.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Pull.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/network/zeromq/socket/Reply.hpp"
#include "opentxs/network/zeromq/socket/Request.hpp"
#include "opentxs/network/zeromq/socket/Router.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"

namespace opentxs::factory
{
auto ZMQContext() noexcept -> std::unique_ptr<network::zeromq::Context>
{
    using ReturnType = network::zeromq::implementation::Context;

    return std::make_unique<ReturnType>();
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
Context::Context() noexcept
    : context_(::zmq_ctx_new())
    , pool_(*this)
{
    assert(nullptr != context_);
    assert(1 == ::zmq_has("curve"));

    constexpr auto sockets
    {
#if defined _WIN32
        10240
#elif defined __APPLE__
        1024
#else
        16384
#endif
    };
    [[maybe_unused]] const auto init =
        ::zmq_ctx_set(context_, ZMQ_MAX_SOCKETS, sockets);

    assert(0 == init);
}

Context::operator void*() const noexcept
{
    assert(nullptr != context_);

    return context_;
}

auto Context::DealerSocket(
    const ListenCallback& callback,
    const socket::Socket::Direction direction) const noexcept
    -> OTZMQDealerSocket
{
    return OTZMQDealerSocket{
        factory::DealerSocket(*this, static_cast<bool>(direction), callback)};
}

auto Context::MakeBatch(std::pmr::vector<socket::Type>&& types) const noexcept
    -> internal::Batch&
{
    return pool_.MakeBatch(std::move(types));
}

auto Context::Modify(SocketID id, ModifyCallback cb) const noexcept
    -> AsyncResult
{
    return pool_.Modify(id, std::move(cb));
}

auto Context::PairEventListener(
    const PairEventCallback& callback,
    const int instance) const noexcept -> OTZMQSubscribeSocket
{
    return OTZMQSubscribeSocket(
        new class PairEventListener(*this, callback, instance));
}

auto Context::PairSocket(const zeromq::ListenCallback& callback) const noexcept
    -> OTZMQPairSocket
{
    return OTZMQPairSocket{factory::PairSocket(*this, callback, true)};
}

auto Context::PairSocket(
    const zeromq::ListenCallback& callback,
    const socket::Pair& peer) const noexcept -> OTZMQPairSocket
{
    return OTZMQPairSocket{factory::PairSocket(callback, peer, true)};
}

auto Context::PairSocket(
    const zeromq::ListenCallback& callback,
    const std::string& endpoint) const noexcept -> OTZMQPairSocket
{
    return OTZMQPairSocket{factory::PairSocket(*this, callback, endpoint)};
}

auto Context::Pipeline(
    const api::Session& api,
    std::function<void(zeromq::Message&&)> callback) const noexcept
    -> zeromq::Pipeline
{
    return opentxs::factory::Pipeline(api, *this, callback);
}

auto Context::Proxy(socket::Socket& frontend, socket::Socket& backend)
    const noexcept -> OTZMQProxy
{
    return zeromq::Proxy::Factory(*this, frontend, backend);
}

auto Context::PublishSocket() const noexcept -> OTZMQPublishSocket
{
    return OTZMQPublishSocket{factory::PublishSocket(*this)};
}

auto Context::PullSocket(
    const socket::Socket::Direction direction) const noexcept -> OTZMQPullSocket
{
    return OTZMQPullSocket{
        factory::PullSocket(*this, static_cast<bool>(direction))};
}

auto Context::PullSocket(
    const ListenCallback& callback,
    const socket::Socket::Direction direction) const noexcept -> OTZMQPullSocket
{
    return OTZMQPullSocket{
        factory::PullSocket(*this, static_cast<bool>(direction), callback)};
}

auto Context::PushSocket(
    const socket::Socket::Direction direction) const noexcept -> OTZMQPushSocket
{
    return OTZMQPushSocket{
        factory::PushSocket(*this, static_cast<bool>(direction))};
}

auto Context::ReplySocket(
    const ReplyCallback& callback,
    const socket::Socket::Direction direction) const noexcept
    -> OTZMQReplySocket
{
    return OTZMQReplySocket{
        factory::ReplySocket(*this, static_cast<bool>(direction), callback)};
}

auto Context::RequestSocket() const noexcept -> OTZMQRequestSocket
{
    return OTZMQRequestSocket{factory::RequestSocket(*this)};
}

auto Context::RouterSocket(
    const ListenCallback& callback,
    const socket::Socket::Direction direction) const noexcept
    -> OTZMQRouterSocket
{
    return OTZMQRouterSocket{
        factory::RouterSocket(*this, static_cast<bool>(direction), callback)};
}

auto Context::Start(BatchID id, StartArgs&& sockets) const noexcept
    -> internal::Thread*
{
    return pool_.Start(id, std::move(sockets));
}

auto Context::Stop(BatchID id) const noexcept -> std::future<bool>
{
    return pool_.Stop(id);
}

auto Context::SubscribeSocket(const ListenCallback& callback) const noexcept
    -> OTZMQSubscribeSocket
{
    return OTZMQSubscribeSocket{factory::SubscribeSocket(*this, callback)};
}

Context::~Context()
{
    pool_.Shutdown();

    if (nullptr != context_) {
        zmq_ctx_shutdown(context_);
        zmq_ctx_term(context_);
        context_ = nullptr;
    }
}
}  // namespace opentxs::network::zeromq::implementation
