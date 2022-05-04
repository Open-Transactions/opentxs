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
#include <string_view>

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
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/ScopeGuard.hpp"
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
template <typename JobType>
class Actor : virtual public Allocated
{
public:
    using Message = network::zeromq::Message;

    const CString name_;

    auto get_allocator() const noexcept -> allocator_type final
    {
        return pipeline_.get_allocator();
    }

protected:
    //This interface used to be private and accessed through friend relationship.
    using Work = JobType;
    virtual auto do_startup() noexcept -> void = 0;
    virtual auto do_shutdown() noexcept -> void = 0;
    virtual auto pipeline(const Work work, Message&& msg) noexcept -> void = 0;
    virtual auto work() noexcept -> bool = 0;

protected:
    using Direction = network::zeromq::socket::Direction;
    using SocketType = network::zeromq::socket::Type;

    const Log& log_;
    mutable std::timed_mutex reorg_lock_;
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
    auto defer(Message&& message) noexcept -> void
    {
        cache_.emplace(std::move(message));
    }
    auto do_init() noexcept -> void
    {
        log_(name_)(" ")(__FUNCTION__)(": initializing").Flush();
        do_startup();

        try {
            init_promise_.set_value();
            log_(name_)(" ")(__FUNCTION__)(": initialization complete").Flush();
            flush_cache();
        } catch (...) {
            LogError()(name_)(" ")(__FUNCTION__)(
                ": init message received twice")
                .Flush();

            OT_FAIL;
        }
    }
    auto do_work() noexcept -> void
    {
        rate_limit_state_machine();
        state_machine_queued_.store(false);
        repeat(work());
    }
    auto flush_cache() noexcept -> void
    {
        retry_.Cancel();

        const auto count = cache_.size();

        if (0u < count) {
            log_(name_)(" ")(__FUNCTION__)(": flushing ")(cache_.size())(
                " cached messages")
                .Flush();
        }

        while (0u < cache_.size()) {
            auto message = Message{std::move(cache_.front())};
            cache_.pop();
            handle_message(std::move(message));
        }
    }
private:
    // unused - remove?
    auto init_complete() noexcept -> void { init_promise_.set_value(); }
protected:
    auto shutdown_actor() noexcept -> void
    {
        init_future_.get();

        if (auto previous = running_.exchange(false); previous) {
            do_shutdown();
            pipeline_.Close();
        }
    }
protected:

    Actor(
        const api::Session& api,
        const Log& logger,
        const CString&& name,
        const std::chrono::milliseconds rateLimit,
        const network::zeromq::BatchID batch,
        allocator_type alloc,
        const network::zeromq::EndpointArgs& subscribe = {},
        const network::zeromq::EndpointArgs& pull = {},
        const network::zeromq::EndpointArgs& dealer = {},
        const Vector<network::zeromq::SocketData>& extra = {},
        Set<Work>&& neverDrop = {}) noexcept
        : name_(std::move(name))
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
        , init_promise_()
        , init_future_(init_promise_.get_future())
        , running_(true)
        , last_executed_(Clock::now())
        , cache_(alloc)
        , state_machine_queued_(false)
        , rng_(std::random_device{}())
        , delay_(50, 150)
        , retry_(api.Network().Asio().Internal().GetTimer())
    {
        log_(name_)(" ")(__FUNCTION__)(": using ZMQ batch ")(
            pipeline_.BatchID())
            .Flush();
    }

    ~Actor() override { retry_.Cancel(); }

private:
    auto wait_for_init() const noexcept -> void { init_future_.get(); }

private:
    const std::chrono::milliseconds rate_limit_;
    const Set<Work> never_drop_;
    std::promise<void> init_promise_;
    std::shared_future<void> init_future_;
    std::atomic<bool> running_;
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
            log_(name_)(" ")(__FUNCTION__)(": rate limited for ")(wait.count())(
                " microseconds")
                .Flush();
            Sleep(wait);
        }
    }

    auto decode_message_type(const network::zeromq::Message& in) noexcept(false)
    {
        const auto body = in.Body();

        if (1 > body.size()) {

            throw std::runtime_error{"empty message received"};
        }

        const auto work = [&] {
            try {

                return body.at(0).as<Work>();
            } catch (...) {

                throw std::runtime_error{
                    "message does not contain a valid work tag"};
            }
        }();

        const auto type = print(work);
        log_(name_)(" ")(__FUNCTION__)(": message type is: ")(type).Flush();
        const auto isInit =
            OT_ZMQ_INIT_SIGNAL == static_cast<OTZMQWorkType>(work);
        const auto canDrop = (0u == never_drop_.count(work));
        const auto initFinished = IsReady(init_future_);

        return std::make_tuple(work, type, isInit, canDrop, initFinished);
    }
    auto handle_message(network::zeromq::Message&& in) noexcept -> void
    {
        try {
            const auto [work, type, isInit, canDrop, initFinished] =
                decode_message_type(in);

            OT_ASSERT(initFinished);

            handle_message(
                false,
                isInit,
                initFinished,
                canDrop,
                type,
                work,
                std::move(in));
        } catch (const std::exception& e) {
            log_(name_)(" ")(__FUNCTION__)(": ")(e.what()).Flush();
        }
    }
    auto handle_message(
        const bool topLevel,
        const bool isInit,
        const bool initFinished,
        const bool canDrop,
        const std::string_view type,
        const Work work,
        network::zeromq::Message&& in) noexcept -> void
    {
        if (false == initFinished) {
            if (isInit) {
                do_init();
                flush_cache();
            } else if (canDrop) {
                log_(name_)(" ")(__FUNCTION__)(": dropping message of type ")(
                    type)(" until init is processed")
                    .Flush();
            } else {
                log_(name_)(" ")(__FUNCTION__)(": queueing message of type ")(
                    type)(" until init is processed")
                    .Flush();
                defer(std::move(in));
            }
        } else {
            if (disable_automatic_processing_) {
                log_(name_)(" ")(__FUNCTION__)(": processing ")(
                    type)(" in bypass mode")
                    .Flush();
                handle_message(work, std::move(in));

                return;
            } else if (topLevel) {
                flush_cache();
            }

            switch (static_cast<OTZMQWorkType>(work)) {
                case value(WorkType::Shutdown): {
                    log_(name_)(" ")(__FUNCTION__)(": shutting down").Flush();
                    this->shutdown_actor();
                } break;
                case OT_ZMQ_STATE_MACHINE_SIGNAL: {
                    log_(name_)(" ")(__FUNCTION__)(": executing state machine")
                        .Flush();
                    do_work();
                } break;
                default: {
                    log_(name_)(" ")(__FUNCTION__)(": processing ")(type)
                        .Flush();
                    handle_message(work, std::move(in));
                }
            }
        }
    }
    auto handle_message(const Work work, Message&& msg) noexcept -> void
    {
        try {
            pipeline(work, std::move(msg));
        } catch (const std::exception& e) {
            log_(name_)(" ")(__FUNCTION__)(": error processing ")(print(work))(
                " message: ")(e.what())
                .Flush();

            OT_FAIL;
        }
    }
    auto repeat(const bool again) noexcept -> void
    {
        if (again) { trigger(); }

        last_executed_ = Clock::now();
    }
    auto try_lock() noexcept
    {
        auto limit = std::chrono::milliseconds{delay_(rng_)};
        auto lock = std::unique_lock<std::timed_mutex>{reorg_lock_, limit};
        auto attempts{-1};

        while ((false == lock.owns_lock()) && (++attempts < 3)) {
            limit = std::chrono::milliseconds{delay_(rng_)};
            lock.try_lock_for(limit);
        }

        return lock;
    }
    auto worker(network::zeromq::Message&& in) noexcept -> void
    {
        log_(name_)(" ")(__FUNCTION__)(": Message received").Flush();

        try {
            const auto [work, type, isInit, canDrop, initFinished] =
                decode_message_type(in);
            auto lock = try_lock();

            if (false == lock.owns_lock()) {
                auto log{type};
                const auto queue = [&] {
                    log_(name_)(" ")(__FUNCTION__)(
                        ": queueing message of type ")(
                        log)(" until reorg is processed")
                        .Flush();
                    defer(std::move(in));
                    retry_.SetRelative(1s);
                    retry_.Wait([this](const auto& e) {
                        if (!e) { trigger(); }
                    });
                    defer(std::move(in));
                };

                if (false == initFinished) {
                    if (canDrop) {
                        log_(name_)(" ")(__FUNCTION__)(
                            ": dropping message of type ")(
                            type)(" until init is processed")
                            .Flush();

                        return;
                    } else if (false == isInit) {
                        queue();

                        return;
                    }
                } else {
                    queue();

                    return;
                }
            }

            if (false == lock.owns_lock()) {
                // If this branch is reached then a reorg must have been
                // initiated in between when this object was constructed and
                // before the init message was received. Only in this one case
                // we will block the thread until the lock is available.
                lock.lock();
            }

            handle_message(
                true, isInit, initFinished, canDrop, type, work, std::move(in));
        } catch (const std::exception& e) {
            log_(name_)(" ")(__FUNCTION__)(": ")(e.what()).Flush();
        }

        log_(name_)(" ")(__FUNCTION__)(": message processing complete").Flush();
    }
};
}  // namespace opentxs
