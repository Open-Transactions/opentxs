// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/zeromq/context/Context.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/make_shared.hpp>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <zmq.h>
#include <atomic>
#include <exception>
#include <memory>
#include <span>
#include <stdexcept>
#include <thread>
#include <utility>

#include "internal/api/Context.hpp"
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
#include "internal/util/LogMacros.hpp"
#include "network/zeromq/Actor.hpp"
#include "network/zeromq/PairEventListener.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "opentxs/util/Log.hpp"
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
    : post_(&zmq_has_terminated)
    , context_([] {
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
    , push_sockets_()
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

auto Context::Alloc(BatchID id) const noexcept -> alloc::Logging*
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

auto Context::Init(
    const opentxs::Options& args,
    std::shared_ptr<const zeromq::Context> me) noexcept -> void
{
    auto handle = pool_.lock();
    auto& pool = *handle;

    if (pool.has_value()) { std::terminate(); }

    pool.emplace(args, std::move(me));

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
    socket::EndpointRequests subscribe,
    socket::EndpointRequests pull,
    socket::EndpointRequests dealer,
    socket::SocketRequests extra,
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

auto Context::PushToEndpoint(std::string_view endpoint, Message&& message)
    const noexcept -> bool
{
    try {
        auto& guarded = [&]() -> GuardedSocket& {
            auto handle = push_sockets_.lock();
            auto& map = *handle;

            if (auto i = map.find(endpoint); map.end() != i) {

                return i->second;
            } else {
                using enum socket::Type;
                auto [j, _] =
                    map.try_emplace(CString{endpoint}, RawSocket(Push));
                auto& [key, out] = *j;
                auto raw = out.lock();

                if (false == raw->Connect(key.c_str())) {

                    throw std::runtime_error{"invalid endpoint"};
                }

                return out;
            }
        }();
        auto handle = guarded.lock();
        auto& socket = *handle;

        return socket.SendDeferred(std::move(message), __FILE__, __LINE__);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
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

auto Context::SpawnActor(
    const api::Context& context,
    std::string_view name,
    actor::Startup startup,
    actor::Shutdown shutdown,
    actor::Processor processor,
    actor::StateMachine statemachine,
    socket::EndpointRequests subscribe,
    socket::EndpointRequests pull,
    socket::EndpointRequests dealer,
    socket::SocketRequests extra) const noexcept -> BatchID
{
    const auto extraCount = extra.get().size();
    const auto batchID = PreallocateBatch();
    auto* alloc = Alloc(batchID);
    // TODO the version of libc++ present in android ndk 23.0.7599858
    // has a broken std::allocate_shared function so we're using
    // boost::shared_ptr instead of std::shared_ptr
    auto actor = boost::allocate_shared<Actor>(
        alloc::PMR<Actor>{alloc},
        context,
        name,
        std::move(startup),
        std::move(shutdown),
        std::move(processor),
        std::move(statemachine),
        std::move(subscribe),
        std::move(pull),
        std::move(dealer),
        std::move(extra),
        batchID,
        extraCount);

    OT_ASSERT(actor);

    actor->Init(actor);

    return batchID;
}

auto Context::SpawnActor(
    const api::Session& session,
    std::string_view name,
    actor::Startup startup,
    actor::Shutdown shutdown,
    actor::Processor processor,
    actor::StateMachine statemachine,
    socket::EndpointRequests subscribe,
    socket::EndpointRequests pull,
    socket::EndpointRequests dealer,
    socket::SocketRequests extra) const noexcept -> BatchID
{
    const auto extraCount = extra.get().size();
    const auto batchID = PreallocateBatch();
    auto* alloc = Alloc(batchID);
    // TODO the version of libc++ present in android ndk 23.0.7599858
    // has a broken std::allocate_shared function so we're using
    // boost::shared_ptr instead of std::shared_ptr
    auto actor = boost::allocate_shared<Actor>(
        alloc::PMR<Actor>{alloc},
        session,
        name,
        std::move(startup),
        std::move(shutdown),
        std::move(processor),
        std::move(statemachine),
        std::move(subscribe),
        std::move(pull),
        std::move(dealer),
        std::move(extra),
        batchID,
        extraCount);

    OT_ASSERT(actor);

    actor->Init(actor);

    return batchID;
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

auto Context::Stop() noexcept -> void
{
    auto handle = pool_.lock();
    auto& pool = *handle;

    if (false == pool.has_value()) { std::terminate(); }

    pool->Shutdown();
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
    }
}
}  // namespace opentxs::network::zeromq::implementation
