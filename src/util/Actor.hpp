// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/network/zeromq/socket/SocketType.hpp"

#pragma once

#include <boost/system/error_code.hpp>
#include <atomic>
#include <chrono>
#include <deque>
#include <functional>
#include <future>
#include <memory>
#include <mutex>
#include <queue>
#include <random>

#include "internal/api/network/Asio.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/util/Future.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

class Log;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
template <typename CRTP, typename JobType>
class Actor : virtual public Allocated
{
private:
    std::promise<void> init_promise_;
    std::shared_future<void> init_future_;
    std::atomic<bool> running_;

public:
    using Message = network::zeromq::Message;

    auto get_allocator() const noexcept -> allocator_type final
    {
        return pipeline_.get_allocator();
    }

protected:
    using Work = JobType;
    using Direction = network::zeromq::socket::Direction;
    using SocketType = network::zeromq::socket::Type;

    const Log& log_;
    mutable std::recursive_timed_mutex reorg_lock_;
    network::zeromq::Pipeline pipeline_;
    bool disable_automatic_processing_;

    auto trigger() const noexcept -> void
    {
        const auto running = state_machine_queued_.exchange(true);

        if (false == running) {
            pipeline_.Push(MakeWork(OT_ZMQ_STATE_MACHINE_SIGNAL));
        }
    }
    auto signal_shutdown() const noexcept -> void
    {
        if (running_) { pipeline_.Push(MakeWork(WorkType::Shutdown)); }
    }
    template <typename SharedPtr>
    auto signal_startup(SharedPtr me) const noexcept -> void
    {
        if (running_) {
            pipeline_.Internal().SetCallback(
                [=](auto&& m) { me->worker(std::move(m)); });
            pipeline_.Push(MakeWork(OT_ZMQ_INIT_SIGNAL));
        }
    }
    auto wait_for_init() const noexcept -> void { init_future_.get(); }

    auto defer(Message&& message) noexcept -> void
    {
        cache_.emplace(std::move(message));
    }
    auto do_init() noexcept -> void
    {
        log_(OT_PRETTY_CLASS())("initializing").Flush();
        downcast().do_startup();

        try {
            init_promise_.set_value();
            log_(OT_PRETTY_CLASS())("initialization complete").Flush();
            flush_cache();
        } catch (...) {
            LogError()(OT_PRETTY_CLASS())("init message received twice")
                .Flush();

            OT_FAIL;
        }
    }
    auto do_work() noexcept -> void
    {
        rate_limit_state_machine();
        state_machine_queued_.store(false);
        repeat(downcast().work());
    }
    auto flush_cache() noexcept -> void
    {
        retry_.Cancel();
        log_(OT_PRETTY_CLASS())("flushing ")(cache_.size())(" cached messages")
            .Flush();

        while (0u < cache_.size()) {
            auto message = Message{std::move(cache_.front())};
            cache_.pop();
            worker(std::move(message));
        }
    }
    auto init_complete() noexcept -> void { init_promise_.set_value(); }
    auto shutdown_actor() noexcept -> void
    {
        init_future_.get();

        if (auto previous = running_.exchange(false); previous) {
            downcast().do_shutdown();
            pipeline_.Close();
        }
    }

    Actor(
        const api::Session& api,
        const Log& logger,
        const std::chrono::milliseconds rateLimit,
        const network::zeromq::BatchID batch,
        allocator_type alloc,
        const network::zeromq::EndpointArgs& subscribe = {},
        const network::zeromq::EndpointArgs& pull = {},
        const network::zeromq::EndpointArgs& dealer = {},
        const Vector<network::zeromq::SocketData>& extra = {},
        Set<Work>&& neverDrop = {}) noexcept
        : init_promise_()
        , init_future_(init_promise_.get_future())
        , running_(true)
        , log_(logger)
        , reorg_lock_()
        , pipeline_(api.Network().ZeroMQ().Internal().Pipeline(
              {},
              subscribe,
              pull,
              dealer,
              extra,
              batch,
              alloc.resource()))
        , disable_automatic_processing_(false)
        , rate_limit_(rateLimit)
        , never_drop_(std::move(neverDrop))
        , last_executed_(Clock::now())
        , cache_(alloc)
        , state_machine_queued_(false)
        , rng_(std::random_device{}())
        , delay_(50, 150)
        , retry_(api.Network().Asio().Internal().GetTimer())
    {
        log_(OT_PRETTY_CLASS())("using ZMQ batch ")(pipeline_.BatchID())
            .Flush();
    }

    ~Actor() override { retry_.Cancel(); }

private:
    const std::chrono::milliseconds rate_limit_;
    const Set<Work> never_drop_;
    Time last_executed_;
    std::queue<Message, Deque<Message>> cache_;
    mutable std::atomic<bool> state_machine_queued_;
    std::mt19937 rng_;
    std::uniform_int_distribution<int> delay_;
    Timer retry_;

    auto rate_limit_state_machine() const noexcept
    {
        const auto wait = std::chrono::duration_cast<std::chrono::microseconds>(
            rate_limit_ - (Clock::now() - last_executed_));

        if (0 < wait.count()) {
            log_(OT_PRETTY_CLASS())("rate limited for ")(wait.count())(
                " microseconds")
                .Flush();
            Sleep(wait);
        }
    }

    inline auto downcast() noexcept -> CRTP&
    {
        return static_cast<CRTP&>(*this);
    }
    auto repeat(const bool again) noexcept -> void
    {
        if (again) { trigger(); }

        last_executed_ = Clock::now();
    }
    auto worker(network::zeromq::Message&& in) noexcept -> void
    {
        log_(OT_PRETTY_CLASS())("Message received").Flush();
        const auto body = in.Body();

        if (1 > body.size()) {
            LogError()(OT_PRETTY_CLASS())("Invalid message").Flush();

            OT_FAIL;
        }

        const auto work = [&] {
            try {

                return body.at(0).as<Work>();
            } catch (...) {

                OT_FAIL;
            }
        }();
        const auto type = CString{print(work)};
        log_(OT_PRETTY_CLASS())("message type is: ")(type).Flush();
        auto limit = std::chrono::milliseconds{delay_(rng_)};
        auto lock =
            std::unique_lock<std::recursive_timed_mutex>{reorg_lock_, limit};
        auto attempts{-1};

        while ((++attempts < 3) && (false == lock.owns_lock())) {
            limit = std::chrono::milliseconds{delay_(rng_)};
            lock.try_lock_for(limit);
        }

        const auto isInit =
            OT_ZMQ_INIT_SIGNAL == static_cast<OTZMQWorkType>(work);
        const auto canDrop = (0u == never_drop_.count(work));
        const auto canDefer =
            (false == lock.owns_lock()) && canDrop && (false == isInit);

        if (canDefer) {
            log_(OT_PRETTY_CLASS())("queueing message of type ")(
                type)(" until reorg is processed")
                .Flush();
            defer(std::move(in));
            retry_.SetRelative(1s);
            retry_.Wait([this](const auto& e) {
                if (!e) { trigger(); }
            });

            return;
        }

        if (false == lock.owns_lock()) { lock.lock(); }

        if (isInit) {
            do_init();
            flush_cache();

            return;
        }

        // NOTE: do not process any messages until init is received
        if (false == IsReady(init_future_)) {
            if (canDrop) {
                log_(OT_PRETTY_CLASS())("dropping message of type ")(
                    type)(" until init is processed")
                    .Flush();
            } else {
                log_(OT_PRETTY_CLASS())("queueing message of type ")(
                    type)(" until init is processed")
                    .Flush();
                defer(std::move(in));
            }

            return;
        }

        if (disable_automatic_processing_) {
            log_(OT_PRETTY_CLASS())("processing ")(type)(" in bypass mode")
                .Flush();
            downcast().pipeline(work, std::move(in));

            return;
        } else {
            flush_cache();
        }

        switch (static_cast<OTZMQWorkType>(work)) {
            case value(WorkType::Shutdown): {
                log_(OT_PRETTY_CLASS())("shutting down").Flush();
                this->shutdown_actor();
            } break;
            case OT_ZMQ_STATE_MACHINE_SIGNAL: {
                log_(OT_PRETTY_CLASS())("executing state machine").Flush();
                do_work();
            } break;
            default: {
                log_(OT_PRETTY_CLASS())("processing ")(type).Flush();
                downcast().pipeline(work, std::move(in));
            }
        }

        log_(OT_PRETTY_CLASS())("message processing complete").Flush();
    }
};
}  // namespace opentxs
