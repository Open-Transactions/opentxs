// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/blockchain/SyncListener.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <chrono>
#include <exception>
#include <mutex>
#include <string_view>
#include <utility>

#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Timer.hpp"
#include "util/Actor.hpp"
#include "util/Work.hpp"

namespace ottest
{
using namespace std::literals;

enum class SyncListenerJob : ot::OTZMQWorkType {
    shutdown = value(ot::WorkType::Shutdown),
    sync = value(ot::WorkType::BlockchainSyncProgress),
    init = ot::OT_ZMQ_INIT_SIGNAL,
    statemachine = ot::OT_ZMQ_STATE_MACHINE_SIGNAL,
};

static auto print(SyncListenerJob state) noexcept -> std::string_view
{
    try {
        using Job = SyncListenerJob;
        static const auto map = ot::Map<Job, std::string_view>{
            {Job::shutdown, "shutdown"sv},
            {Job::sync, "sync"sv},
            {Job::init, "init"sv},
            {Job::statemachine, "statemachine"sv},
        };

        return map.at(state);
    } catch (...) {
        ot::LogAbort()(__FUNCTION__)(": invalid SyncListenerJob: ")(
            static_cast<ot::OTZMQWorkType>(state))
            .Abort();
    }
}
}  // namespace ottest

namespace ottest
{
using namespace opentxs::literals;

using SyncListenerActor = opentxs::Actor<SyncListener::Imp, SyncListenerJob>;

class SyncListener::Imp final : public SyncListenerActor
{
public:
    const ot::api::Session& api_;
    mutable std::mutex lock_;
    std::promise<Height> promise_;
    Height target_;

    auto Init(std::shared_ptr<Imp> me) noexcept -> void { signal_startup(me); }
    auto Stop() noexcept -> void
    {
        pipeline_.Push(ot::MakeWork(Work::shutdown));
    }

    Imp(const ot::api::Session& api, std::string_view name) noexcept
        : Imp(api, name, api.Network().ZeroMQ().Internal().PreallocateBatch())
    {
    }

private:
    friend SyncListenerActor;

    auto do_shutdown() noexcept -> void {}
    auto do_startup() noexcept -> bool { return false; }
    auto pipeline(const Work work, Message&& msg) noexcept -> void
    {
        switch (work) {
            case Work::sync: {
                process_sync(std::move(msg));
            } break;
            case Work::shutdown:
            case Work::init:
            case Work::statemachine:
            default: {
                ot::LogAbort()(OT_PRETTY_CLASS())(
                    name_)(" unhandled message type ")(
                    static_cast<ot::OTZMQWorkType>(work))
                    .Abort();
            }
        }
    }
    auto process_sync(Message&& msg) noexcept -> void
    {
        const auto body = msg.Body();

        OT_ASSERT(2_uz < body.size());

        using Height = ot::blockchain::block::Height;
        process_height(body.at(2).as<Height>());
    }
    auto process_height(ot::blockchain::block::Height&& height) noexcept -> void
    {
        ot::LogConsole()(name_)(" sync client updated to ")(height).Flush();
        auto lock = ot::Lock{lock_};

        if (height == target_) {
            try {
                promise_.set_value(height);
                log_(name_)(" future for ")(height)(" is ready ").Flush();
            } catch (const std::exception& e) {
                log_(name_)(": ")(e.what()).Flush();
            }
        } else {
            log_(name_)(" received height ")(
                height)(" but no future is active for it")
                .Flush();
        }
    }
    auto work() noexcept -> bool { return false; }

    Imp(const ot::api::Session& api,
        std::string_view name,
        ot::network::zeromq::BatchID batch)
        : Imp(api, name, batch, api.Network().ZeroMQ().Internal().Alloc(batch))
    {
    }
    Imp(const ot::api::Session& api,
        std::string_view name,
        ot::network::zeromq::BatchID batch,
        allocator_type alloc)
        : SyncListenerActor(
              api,
              ot::LogTrace(),
              ot::CString{name, alloc},
              0ms,
              std::move(batch),
              alloc,
              [&] {
                  auto sub = ot::network::zeromq::EndpointArgs{alloc};
                  sub.emplace_back(
                      api.Endpoints().Shutdown(), Direction::Connect);
                  sub.emplace_back(
                      api.Endpoints().BlockchainSyncProgress(),
                      Direction::Connect);

                  return sub;
              }())
        , api_(api)
        , lock_()
        , promise_()
        , target_(-1)
    {
    }
};
}  // namespace ottest

namespace ottest
{
SyncListener::SyncListener(
    const ot::api::Session& api,
    std::string_view name) noexcept
    : imp_(std::make_shared<Imp>(api, name))
{
    imp_->Init(imp_);
}

auto SyncListener::GetFuture(const Height height) noexcept -> Future
{
    auto lock = ot::Lock{imp_->lock_};
    imp_->target_ = height;

    try {
        imp_->promise_ = {};
    } catch (...) {
    }

    return imp_->promise_.get_future();
}

SyncListener::~SyncListener() { imp_->Stop(); }
}  // namespace ottest