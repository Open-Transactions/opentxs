// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <chrono>
#include <functional>
#include <future>
#include <string_view>

#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "util/Work.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
template <typename Child, typename API = api::session::Client>
class Worker
{
public:
    const API& api_;

protected:
    using Endpoints = UnallocatedVector<UnallocatedCString>;

    const std::chrono::milliseconds rate_limit_;
    std::atomic<bool> running_;
    std::promise<void> shutdown_promise_;
    std::shared_future<void> shutdown_;
    network::zeromq::Pipeline pipeline_;

    auto trigger() const noexcept -> void
    {
        if (false == running_.load()) { return; }

        const auto running = state_machine_queued_.exchange(true);

        if (false == running) {
            pipeline_.Push(MakeWork(OT_ZMQ_STATE_MACHINE_SIGNAL));
        }
    }
    auto signal_shutdown() const noexcept -> std::shared_future<void>
    {
        return const_cast<Worker&>(*this).stop_worker();
    }

    auto do_work() noexcept
    {
        rate_limit_state_machine();
        state_machine_queued_.store(false);
        repeat(downcast().state_machine());
    }
    auto init_executor(const Endpoints endpoints = {}) noexcept -> void
    {
        pipeline_.SubscribeTo(api_.Endpoints().Shutdown());

        for (const auto& endpoint : endpoints) {
            pipeline_.SubscribeTo(endpoint);
        }
    }
    auto stop_worker() noexcept -> std::shared_future<void>
    {
        pipeline_.Close();
        downcast().shutdown(shutdown_promise_);

        return shutdown_;
    }

    Worker(
        const API& api,
        const std::chrono::milliseconds rateLimit,
        const std::string_view name,
        network::zeromq::socket::EndpointRequests subscribe = {},
        network::zeromq::socket::EndpointRequests pull = {},
        network::zeromq::socket::EndpointRequests dealer = {},
        network::zeromq::socket::SocketRequests extra = {}) noexcept
        : api_(api)
        , rate_limit_(rateLimit)
        , running_(true)
        , shutdown_promise_()
        , shutdown_(shutdown_promise_.get_future())
        , pipeline_(api.Network().ZeroMQ().Internal().Pipeline(
              [this](auto&& in) { downcast().pipeline(std::move(in)); },
              name,
              std::move(subscribe),
              std::move(pull),
              std::move(dealer),
              std::move(extra)))
        , last_executed_(Clock::now())
        , state_machine_queued_(false)
    {
        LogTrace()()("using ZMQ batch ")(pipeline_.BatchID()).Flush();
    }

    ~Worker() { stop_worker().get(); }

private:
    Time last_executed_;
    mutable std::atomic<bool> state_machine_queued_;

    auto rate_limit_state_machine() const noexcept
    {
        const auto wait = std::chrono::duration_cast<std::chrono::milliseconds>(
            rate_limit_ - (Clock::now() - last_executed_));

        if (0 < wait.count()) { Sleep(wait); }
    }

    inline auto downcast() noexcept -> Child&
    {
        return static_cast<Child&>(*this);
    }
    auto repeat(const bool again) noexcept -> void
    {
        if (again) { trigger(); }

        last_executed_ = Clock::now();
    }
};
}  // namespace opentxs
