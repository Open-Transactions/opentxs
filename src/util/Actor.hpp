// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "internal/network/zeromq/socket/SocketType.hpp"

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
#include <sstream>
#include <string_view>

#include "internal/api/network/Asio.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/util/BoostPMR.hpp"
#include "internal/util/Future.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Thread.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/ScopeGuard.hpp"
#include "util/Work.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Context;
class Session;
}  // namespace api

class Log;
class Timer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
template <
    typename CRTP,
    typename JobType,
    OTZMQWorkType Terminate = value(WorkType::Shutdown)>
class Actor : virtual public Allocated
{
public:
    using Work = JobType;

    const CString name_;

    auto get_allocator() const noexcept -> allocator_type final
    {
        return pipeline_.get_allocator();
    }

protected:
    using Direction = network::zeromq::socket::Direction;
    using SocketType = network::zeromq::socket::Type;
    using Message = network::zeromq::Message;

    const Log& log_;
    network::zeromq::Pipeline pipeline_;

    auto trigger() const noexcept -> void
    {
        const auto running = state_machine_queued_.exchange(true);

        if (false == running) {
            pipeline_.Push(MakeWork(state_machine_signal_));
        }
    }
    template <typename SharedPtr>
    auto signal_startup(SharedPtr me) const noexcept -> void
    {
        if (running_) {
            pipeline_.Internal().SetCallback(
                [=](auto&& m) { me->worker(std::move(m)); });
            pipeline_.Push(MakeWork(init_signal_));
        }
    }

    auto defer(Message&& message) noexcept -> void
    {
        cache_.emplace(std::move(message));
    }
    auto do_init(allocator_type monotonic) noexcept -> void
    {
        if (init_complete_) {
            LogAbort()(OT_PRETTY_CLASS())(
                name_)(": init message received twice")
                .Abort();
        } else {
            log_(OT_PRETTY_CLASS())(name_)(": initializing").Flush();
        }

        const auto shutdown = downcast().do_startup(monotonic);
        init_complete_ = true;
        log_(OT_PRETTY_CLASS())(name_)(": initialization complete").Flush();

        if (shutdown) {
            shutdown_actor();
        } else {
            flush_cache(monotonic);
        }
    }
    auto do_work(allocator_type monotonic) noexcept -> void
    {
        const auto now = sClock::now();

        if (now < next_state_machine_) {
            const auto wait =
                std::chrono::duration_cast<std::chrono::nanoseconds>(
                    next_state_machine_ - now);
            log_(OT_PRETTY_CLASS())(name_)(": rate limited for ")(wait).Flush();
            reset_timer(
                std::chrono::duration_cast<std::chrono::microseconds>(wait),
                rate_limit_timer_,
                state_machine_signal_);
        } else {
            state_machine_queued_.store(false);
            repeat(downcast().work(monotonic));
            next_state_machine_ = now + rate_limit_;
        }
    }
    auto reset_timer(
        const std::chrono::microseconds& value,
        Timer& timer,
        Work work) noexcept -> void
    {
        timer.Cancel();
        timer.SetRelative(value);
        reset_timer(timer, work);
    }
    auto reset_timer(const Time& value, Timer& timer, Work work) noexcept
        -> void
    {
        timer.Cancel();
        timer.SetAbsolute(value);
        reset_timer(timer, work);
    }
    auto shutdown_actor() noexcept -> void
    {
        if (auto previous = running_.exchange(false); previous) {
            rate_limit_timer_.Cancel();
            downcast().do_shutdown();
            pipeline_.Close();
        }
    }

    Actor(
        const api::network::Asio& asio,
        const network::zeromq::Context& zmq,
        const Log& logger,
        int instance,
        CString&& name,
        std::chrono::milliseconds rateLimit,
        network::zeromq::BatchID batch,
        allocator_type alloc,
        const network::zeromq::EndpointArgs& subscribe = {},
        const network::zeromq::EndpointArgs& pull = {},
        const network::zeromq::EndpointArgs& dealer = {},
        const Vector<network::zeromq::SocketData>& extra = {},
        Set<Work>&& neverDrop = {}) noexcept
        : name_([&] {
            // TODO c++20 allocator
            auto ss = std::stringstream{};

            if (0 > instance) {
                ss << "global";
            } else {
                ss << "instance ";
                ss << std::to_string(instance);
            }

            ss << " ";
            ss << name;

            return CString{ss.str().c_str(), alloc};
        }())
        , log_(logger)
        , pipeline_(zmq.Internal().Pipeline(
              {},
              name,
              subscribe,
              pull,
              dealer,
              extra,
              batch,
              alloc.resource()))
        , rate_limit_(std::move(rateLimit))
        , never_drop_(std::move(neverDrop))
        , init_complete_(false)
        , running_(true)
        , next_state_machine_()
        , cache_(alloc)
        , state_machine_queued_(false)
        , rate_limit_timer_(asio.Internal().GetTimer())
    {
        log_(OT_PRETTY_CLASS())(name_)(": using ZMQ batch ")(
            pipeline_.BatchID())
            .Flush();
    }
    Actor(
        const api::Session& api,
        const Log& logger,
        CString&& name,
        std::chrono::milliseconds rateLimit,
        network::zeromq::BatchID batch,
        allocator_type alloc,
        const network::zeromq::EndpointArgs& subscribe = {},
        const network::zeromq::EndpointArgs& pull = {},
        const network::zeromq::EndpointArgs& dealer = {},
        const Vector<network::zeromq::SocketData>& extra = {},
        Set<Work>&& neverDrop = {}) noexcept
        : Actor(
              api.Network().Asio(),
              api.Network().ZeroMQ(),
              logger,
              api.Instance(),
              std::move(name),
              std::move(rateLimit),
              std::move(batch),
              std::move(alloc),
              subscribe,
              pull,
              dealer,
              extra,
              std::move(neverDrop))
    {
    }
    Actor(
        const api::Context& context,
        const Log& logger,
        CString&& name,
        std::chrono::milliseconds rateLimit,
        network::zeromq::BatchID batch,
        allocator_type alloc,
        const network::zeromq::EndpointArgs& subscribe = {},
        const network::zeromq::EndpointArgs& pull = {},
        const network::zeromq::EndpointArgs& dealer = {},
        const Vector<network::zeromq::SocketData>& extra = {},
        Set<Work>&& neverDrop = {}) noexcept
        : Actor(
              context.Asio(),
              context.ZMQ(),
              logger,
              -1,
              std::move(name),
              std::move(rateLimit),
              std::move(batch),
              std::move(alloc),
              subscribe,
              pull,
              dealer,
              extra,
              std::move(neverDrop))
    {
    }

    ~Actor() override = default;

private:
    static constexpr auto terminate_signal_ = static_cast<Work>(Terminate);
    static constexpr auto init_signal_ = static_cast<Work>(OT_ZMQ_INIT_SIGNAL);
    static constexpr auto state_machine_signal_ =
        static_cast<Work>(OT_ZMQ_STATE_MACHINE_SIGNAL);

    const std::chrono::milliseconds rate_limit_;
    const Set<Work> never_drop_;
    bool init_complete_;
    std::atomic<bool> running_;
    sTime next_state_machine_;
    std::queue<Message, Deque<Message>> cache_;
    mutable std::atomic<bool> state_machine_queued_;
    Timer rate_limit_timer_;

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
        log_(OT_PRETTY_CLASS())(name_)(": message type is: ")(type).Flush();
        const auto isInit = (init_signal_ == work);
        const auto canDrop = (0u == never_drop_.count(work));

        return std::make_tuple(work, type, isInit, canDrop);
    }
    inline auto downcast() noexcept -> CRTP&
    {
        return static_cast<CRTP&>(*this);
    }
    auto flush_cache(allocator_type monotonic) noexcept -> void
    {
        if (false == cache_.empty()) {
            log_(OT_PRETTY_CLASS())(name_)(": flushing ")(cache_.size())(
                " cached messages")
                .Flush();
        }

        for (auto n{0_uz}, stop = cache_.size(); n < stop; ++n) {
            auto message = Message{std::move(cache_.front())};
            cache_.pop();
            handle_message(std::move(message), monotonic);
        }
    }
    auto handle_message(
        network::zeromq::Message&& in,
        allocator_type monotonic) noexcept -> void
    {
        try {
            const auto [work, type, isInit, canDrop] = decode_message_type(in);

            OT_ASSERT(init_complete_);

            handle_message(
                false, isInit, canDrop, type, work, std::move(in), monotonic);
        } catch (const std::exception& e) {
            log_(OT_PRETTY_CLASS())(name_)(": ")(e.what()).Flush();
        }
    }
    auto handle_message(
        const bool topLevel,
        const bool isInit,
        const bool canDrop,
        const std::string_view type,
        const Work work,
        network::zeromq::Message&& in,
        allocator_type monotonic) noexcept -> void
    {
        if (false == init_complete_) {
            if (isInit) {
                do_init(monotonic);
                flush_cache(monotonic);
            } else if (canDrop) {
                log_(OT_PRETTY_CLASS())(name_)(": dropping message of type ")(
                    type)(" until init is processed")
                    .Flush();
            } else {
                log_(OT_PRETTY_CLASS())(name_)(": queueing message of type ")(
                    type)(" until init is processed")
                    .Flush();
                defer(std::move(in));
            }
        } else {
            if (topLevel) { flush_cache(monotonic); }

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wswitch-enum"
            switch (work) {
                case terminate_signal_: {
                    log_(OT_PRETTY_CLASS())(name_)(": shutting down").Flush();
                    shutdown_actor();
                } break;
                case state_machine_signal_: {
                    log_(OT_PRETTY_CLASS())(name_)(": executing state machine")
                        .Flush();
                    do_work(monotonic);
                } break;
                default: {
                    log_(OT_PRETTY_CLASS())(name_)(": processing ")(type)
                        .Flush();
                    handle_message(work, std::move(in), monotonic);
                }
            }
        }
#pragma GCC diagnostic pop
    }
    auto handle_message(
        const Work work,
        Message&& msg,
        allocator_type monotonic) noexcept -> void
    {
        try {
            downcast().pipeline(work, std::move(msg), monotonic);
        } catch (const std::exception& e) {
            log_(OT_PRETTY_CLASS())(name_)(": error processing ")(print(work))(
                " message: ")(e.what())
                .Abort();
        }
    }
    auto repeat(const bool again) noexcept -> void
    {
        if (again) { trigger(); }
    }
    auto reset_timer(Timer& timer, Work work) noexcept -> void
    {
        timer.Wait([this, work](const auto& ec) {
            if (ec) {
                if (boost::system::errc::operation_canceled != ec.value()) {
                    LogError()(OT_PRETTY_CLASS())(name_)(": ")(ec).Flush();
                }
            } else {
                pipeline_.Push(MakeWork(work));
            }
        });
    }
    auto worker(network::zeromq::Message&& in) noexcept -> void
    {
        log_(OT_PRETTY_CLASS())(name_)(": Message received").Flush();
        // NOLINTNEXTLINE(modernize-avoid-c-arrays)
        std::byte buf[thread_pool_monotonic_];

        try {
            auto upstream = alloc::StandardToBoost(get_allocator().resource());
            auto alloc = alloc::BoostMonotonic(
                buf, sizeof(buf), std::addressof(upstream));
            const auto [work, type, isInit, canDrop] = decode_message_type(in);
            handle_message(
                true,
                isInit,
                canDrop,
                type,
                work,
                std::move(in),
                std::addressof(alloc));
        } catch (const std::exception& e) {
            log_(OT_PRETTY_CLASS())(name_)(": ")(e.what()).Flush();
        }

        log_(OT_PRETTY_CLASS())(name_)(": message processing complete").Flush();
    }
};
}  // namespace opentxs
