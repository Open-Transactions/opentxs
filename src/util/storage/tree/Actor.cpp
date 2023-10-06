// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/tree/Actor.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <algorithm>
#include <chrono>
#include <compare>
#include <cstdlib>
#include <optional>
#include <ratio>
#include <string_view>
#include <type_traits>
#include <utility>

#include "api/session/Storage.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/socket/Direction.hpp"   // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/ScopeGuard.hpp"
#include "util/Work.hpp"

namespace opentxs::storage::tree
{
using namespace std::literals;

auto print(Job state) noexcept -> std::string_view
{
    using enum Job;
    static constexpr auto map =
        frozen::make_unordered_map<Job, std::string_view>({
            {shutdown, "shutdown"sv},
            {finished, "finished"sv},
            {init, "init"sv},
            {statemachine, "statemachine"sv},
        });

    if (const auto* i = map.find(state); map.end() != i) {

        return i->second;
    } else {

        return "unknown job type"sv;
    }
}
}  // namespace opentxs::storage::tree

namespace opentxs::storage::tree
{
using enum opentxs::network::zeromq::socket::Direction;
using enum opentxs::network::zeromq::socket::Type;
using namespace std::literals;

Actor::Actor(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<api::session::imp::Storage> parent,
    opentxs::network::zeromq::BatchID batchID,
    std::chrono::seconds interval,
    CString endpoint,
    allocator_type alloc) noexcept
    : opentxs::Actor<tree::Actor, Job>(
          *api,
          LogTrace(),
          {"storage garbage collector", alloc},
          0ms,
          batchID,
          alloc,
          {
              {api->Endpoints().Shutdown(), Connect},
          },
          {
              {endpoint, Bind},
          })
    , api_p_(std::move(api))
    , parent_p_(std::move(parent))
    , self_()
    , api_(*api_p_)
    , parent_(*parent_p_)
    , interval_(std::max<decltype(interval_)>(interval, 1min))
    , push_([&] {
        auto out = api_.Network().ZeroMQ().Internal().RawSocket(Push);
        const auto rc = out.Connect(endpoint.c_str());

        OT_ASSERT(rc);

        return out;
    }())
    , timer_(api_.Network().Asio().Internal().GetTimer())
{
}

auto Actor::do_shutdown() noexcept -> void
{
    timer_.Cancel();
    api_p_.reset();
    parent_p_.reset();
    self_.reset();
}

auto Actor::do_startup(allocator_type monotonic) noexcept -> bool
{
    if (api_.Internal().ShuttingDown()) { return true; }

    reset_gc_timer(0s);

    return false;
}

auto Actor::need_gc(std::chrono::microseconds elapsed) const noexcept -> bool
{
    return elapsed >= interval_;
}

auto Actor::pipeline(
    const Work work,
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    using enum Job;

    switch (work) {
        case finished: {
            do_work(monotonic);
        } break;
        case shutdown:
        case init:
        case statemachine: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(": unhandled message type ")(
                print(work))
                .Abort();
        }
        default: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(": unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Abort();
        }
    }
}

auto Actor::reset_gc_timer(std::chrono::microseconds wait) noexcept -> void
{
    reset_timer(wait, timer_, Work::statemachine);
}

auto Actor::run_gc(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<api::session::imp::Storage> parent,
    std::shared_ptr<Actor> self,
    const GCParams& params) noexcept -> void
{
    OT_ASSERT(api);
    OT_ASSERT(parent);
    OT_ASSERT(self);

    using enum Job;
    auto success{false};
    const auto post = ScopeGuard{[parent, self, &success] {
        parent->FinishGC(success);
        self->push_.lock()->Send(MakeWork(finished), __FILE__, __LINE__);
    }};
    const auto& me = *self;
    const auto& log = me.log_;
    log(OT_PRETTY_STATIC(Actor))(me.name_)(": garbage collection running")
        .Flush();
    success = parent->DoGC(params);

    if (success) {
        log(OT_PRETTY_STATIC(Actor))(me.name_)(": garbage collection finished")
            .Flush();
    } else {
        LogError()(OT_PRETTY_STATIC(Actor))(me.name_)(
            ": garbage collection failed")
            .Flush();
    }
}

auto Actor::schedule_gc(std::chrono::microseconds elapsed) noexcept -> void
{
    using namespace std::chrono;
    const auto wait = duration_cast<seconds>(interval_ - elapsed);
    log_(OT_PRETTY_CLASS())(name_)(": scheduling garbage collection in ")(
        wait.count())(" seconds")
        .Flush();
    reset_gc_timer(wait);
}

auto Actor::start_gc() noexcept -> void
{
    log_(OT_PRETTY_CLASS())(name_)(": beginning garbage collection").Flush();

    if (auto gc = parent_.StartGC(); gc) {
        RunJob([api = api_p_,
                parent = parent_p_,
                self = self_,
                params = std::move(gc.value())] {
            run_gc(api, parent, self, params);
        });
    } else {
        log_(OT_PRETTY_CLASS())(name_)(": failed to start garbage collection")
            .Flush();
        reset_gc_timer(10s);
    }
}

auto Actor::work(allocator_type) noexcept -> bool
{
    const auto& log = log_;
    const auto status = parent_.GCStatus();

    if (status.running_) {
        log(OT_PRETTY_CLASS())(name_)(": garbage collection already running")
            .Flush();
    } else {
        using namespace std::chrono;
        const auto elapsed =
            duration_cast<seconds>(Clock::now() - status.last_);
        const auto seconds = std::chrono::seconds{std::llabs(elapsed.count())};
        log(OT_PRETTY_CLASS())(name_)(": ")(seconds.count())(
            " seconds since last garbage collection run")
            .Flush();

        if (need_gc(seconds)) {
            start_gc();
        } else {
            schedule_gc(seconds);
        }
    }

    return false;
}

Actor::~Actor() = default;
}  // namespace opentxs::storage::tree
