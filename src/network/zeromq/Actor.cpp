// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/zeromq/Actor.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <chrono>
#include <functional>
#include <utility>

#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "util/Work.hpp"

namespace opentxs::network::zeromq
{
Actor::Actor(
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
    zeromq::BatchID batchID,
    std::size_t extraCount,
    allocator_type alloc) noexcept
    : opentxs::Actor<zeromq::Actor, OTZMQWorkType>(
          context,
          LogTrace(),
          CString{name, alloc},
          0ms,
          batchID,
          alloc,
          std::move(subscribe),
          std::move(pull),
          std::move(dealer),
          std::move(extra))
    , index_([&] {
        using namespace actor;
        auto out = decltype(index_){alloc};
        out.reserve(fixed_ + extraCount);
        out.try_emplace(pipeline_.ConnectionIDSubscribe(), SubscribeIndex);
        out.try_emplace(pipeline_.ConnectionIDPull(), PullIndex);
        out.try_emplace(pipeline_.ConnectionIDDealer(), DealerIndex);
        out.try_emplace(pipeline_.ConnectionIDInternal(), LoopbackIndex);

        for (auto n = 0_uz; n < extraCount; ++n) {
            out.try_emplace(
                pipeline_.Internal().ExtraSocket(n).ID(), fixed_ + n);
        }

        out.rehash(0_uz);

        return out;
    }())
    , startup_(startup ? std::move(startup) : DefaultStartup())
    , shutdown_(shutdown ? std::move(shutdown) : DefaultShutdown())
    , processor_(processor ? std::move(processor) : DefaultProcessor())
    , state_(statemachine ? std::move(statemachine) : DefaultStateMachine())
{
}

Actor::Actor(
    const api::Session& session,
    std::string_view name,
    actor::Startup startup,
    actor::Shutdown shutdown,
    actor::Processor processor,
    actor::StateMachine statemachine,
    socket::EndpointRequests subscribe,
    socket::EndpointRequests pull,
    socket::EndpointRequests dealer,
    socket::SocketRequests extra,
    zeromq::BatchID batchID,
    std::size_t extraCount,
    allocator_type alloc) noexcept
    : opentxs::Actor<zeromq::Actor, OTZMQWorkType>(
          session,
          LogTrace(),
          CString{name, alloc},
          0ms,
          batchID,
          alloc,
          std::move(subscribe),
          std::move(pull),
          std::move(dealer),
          std::move(extra))
    , index_([&] {
        using namespace actor;
        auto out = decltype(index_){alloc};
        out.reserve(fixed_ + extraCount);
        out.try_emplace(pipeline_.ConnectionIDSubscribe(), SubscribeIndex);
        out.try_emplace(pipeline_.ConnectionIDPull(), PullIndex);
        out.try_emplace(pipeline_.ConnectionIDDealer(), DealerIndex);
        out.try_emplace(pipeline_.ConnectionIDInternal(), LoopbackIndex);

        for (auto n = 0_uz; n < extraCount; ++n) {
            out.try_emplace(
                pipeline_.Internal().ExtraSocket(n).ID(), fixed_ + n);
        }

        out.rehash(0_uz);

        return out;
    }())
    , startup_(startup ? std::move(startup) : DefaultStartup())
    , shutdown_(shutdown ? std::move(shutdown) : DefaultShutdown())
    , processor_(processor ? std::move(processor) : DefaultProcessor())
    , state_(statemachine ? std::move(statemachine) : DefaultStateMachine())
{
}

auto Actor::do_shutdown() noexcept -> void
{
    std::invoke(shutdown_);
    state_ = DefaultStateMachine();
    processor_ = DefaultProcessor();
    shutdown_ = DefaultShutdown();
    startup_ = DefaultStartup();
}

auto Actor::do_startup(allocator_type monotonic) noexcept -> bool
{
    auto out =
        std::invoke(startup_, alloc::Strategy{get_allocator(), monotonic});
    startup_ = DefaultStartup();

    if (false == out) { pipeline_.Push(MakeWork(state_machine_signal_)); }

    return out;
}

auto Actor::get_index(SocketID id) const noexcept -> actor::SocketIndex
{
    if (auto i = index_.find(id); index_.end() != i) {

        return i->second;
    } else {
        OT_FAIL;
    }
}

auto Actor::pipeline(
    const Work work,
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    const auto id = connection_id(msg);
    auto replies = std::invoke(
        processor_,
        get_index(id),
        work,
        std::move(msg),
        alloc::Strategy{get_allocator(), monotonic});

    for (auto& [index, messages] : replies) { send(index, messages); }
}

auto Actor::send(actor::SocketIndex index, std::span<Message> msg) noexcept
    -> void
{
    using namespace actor;

    auto send_via_dealer = [this](auto& m) {
        pipeline_.Internal().SendFromThread(std::move(m));
    };
    auto send_via_loopback = [this](auto& m) { pipeline_.Push(std::move(m)); };
    auto send_via_extra = [this, index](auto& m) {
        pipeline_.Internal()
            .ExtraSocket(index - fixed_)
            .SendDeferred(std::move(m), __FILE__, __LINE__);
    };

    switch (index) {
        case SubscribeIndex:
        case PullIndex: {
            LogAbort()(OT_PRETTY_CLASS())(
                "unable to send message via receive-only socket")
                .Abort();
        }
        case DealerIndex: {
            std::for_each(msg.begin(), msg.end(), send_via_dealer);
        } break;
        case LoopbackIndex: {
            std::for_each(msg.begin(), msg.end(), send_via_loopback);
        } break;
        default: {
            std::for_each(msg.begin(), msg.end(), send_via_extra);
        }
    }
}

auto Actor::work(allocator_type monotonic) noexcept -> bool
{
    return std::invoke(state_, alloc::Strategy{get_allocator(), monotonic});
}

Actor::~Actor() = default;
}  // namespace opentxs::network::zeromq
