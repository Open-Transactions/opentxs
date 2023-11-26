// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/blockchain/CfilterListener.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <chrono>
#include <exception>
#include <mutex>
#include <span>
#include <utility>

#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/PMR.hpp"
#include "internal/util/alloc/Logging.hpp"
#include "util/Actor.hpp"
#include "util/Work.hpp"

namespace ottest
{
using namespace std::literals;

enum class CfilterListenerJob : ot::OTZMQWorkType {
    shutdown = value(ot::WorkType::Shutdown),
    filter = value(ot::WorkType::BlockchainNewFilter),
    init = ot::OT_ZMQ_INIT_SIGNAL,
    statemachine = ot::OT_ZMQ_STATE_MACHINE_SIGNAL,
};

static auto print(CfilterListenerJob state) noexcept -> std::string_view
{
    try {
        using Job = CfilterListenerJob;
        static const auto map = ot::Map<Job, std::string_view>{
            {Job::shutdown, "shutdown"sv},
            {Job::filter, "filter"sv},
            {Job::init, "init"sv},
            {Job::statemachine, "statemachine"sv},
        };

        return map.at(state);
    } catch (...) {
        ot::LogAbort()(__FUNCTION__)(": invalid CfilterListenerJob: ")(
            static_cast<ot::OTZMQWorkType>(state))
            .Abort();
    }
}
}  // namespace ottest

namespace ottest
{
using namespace opentxs::literals;
using namespace std::literals;
using enum opentxs::network::zeromq::socket::Direction;

using CfilterListenerActor =
    opentxs::Actor<CfilterListener::Imp, CfilterListenerJob>;

class CfilterListener::Imp final : public CfilterListenerActor
{
public:
    const ot::api::Session& api_;
    mutable std::mutex lock_;
    std::promise<Position> promise_;
    Height target_;

    auto get_deleter() noexcept -> delete_function final
    {
        return opentxs::pmr::make_deleter(this);
    }
    auto Init(std::shared_ptr<Imp> me) noexcept -> void { signal_startup(me); }
    auto Stop() noexcept -> void
    {
        pipeline_.Push(ot::MakeWork(Work::shutdown));
    }

    Imp(const ot::api::Session& api, std::string_view name) noexcept
        : Imp(api,
              name,
              api.Network().ZeroMQ().Context().Internal().PreallocateBatch())
    {
    }

private:
    friend CfilterListenerActor;

    auto do_shutdown() noexcept -> void {}
    auto do_startup(allocator_type) noexcept -> bool { return false; }
    auto pipeline(const Work work, Message&& msg, allocator_type) noexcept
        -> void
    {
        switch (work) {
            case Work::filter: {
                process_filter(std::move(msg));
            } break;
            case Work::shutdown:
            case Work::init:
            case Work::statemachine:
            default: {
                ot::LogAbort()()(name_)(" unhandled message type ")(
                    static_cast<ot::OTZMQWorkType>(work))
                    .Abort();
            }
        }
    }
    auto process_filter(Message&& msg) noexcept -> void
    {
        const auto body = msg.Payload();

        opentxs::assert_true(4_uz < body.size());

        using BlockHeight = ot::blockchain::block::Height;
        process_position({body[3].as<BlockHeight>(), body[4].Bytes()});
    }
    auto process_position(ot::blockchain::block::Position&& position) noexcept
        -> void
    {
        ot::LogVerbose()(name_)(" filter oracle updated to ")(position).Flush();
        auto lock = ot::Lock{lock_};

        if (position.height_ == target_) {
            try {
                promise_.set_value(position);
                log_(name_)(" future for ")(position.height_)(" is ready ")
                    .Flush();
            } catch (const std::exception& e) {
                log_(name_)(": ")(e.what()).Flush();
            }
        } else {
            log_(name_)(" received position ")(
                position)(" but no future is active for this height")
                .Flush();
        }
    }
    auto work(allocator_type) noexcept -> bool { return false; }

    Imp(const ot::api::Session& api,
        std::string_view name,
        ot::network::zeromq::BatchID batch)
        : Imp(api,
              name,
              batch,
              api.Network().ZeroMQ().Context().Internal().Alloc(batch))
    {
    }
    Imp(const ot::api::Session& api,
        std::string_view name,
        ot::network::zeromq::BatchID batch,
        allocator_type alloc)
        : CfilterListenerActor(
              api,
              ot::LogTrace(),
              ot::CString{name, alloc},
              0ms,
              std::move(batch),
              alloc,
              {
                  {api.Endpoints().Shutdown(), Connect},
                  {api.Endpoints().BlockchainNewFilter(), Connect},
              })
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
CfilterListener::CfilterListener(
    const ot::api::Session& api,
    std::string_view name) noexcept
    : imp_(std::make_shared<Imp>(api, name))
{
    imp_->Init(imp_);
}

auto CfilterListener::GetFuture(const Height height) noexcept -> Future
{
    auto lock = ot::Lock{imp_->lock_};
    imp_->target_ = height;

    try {
        imp_->promise_ = {};
    } catch (...) {
    }

    return imp_->promise_.get_future();
}

CfilterListener::~CfilterListener() { imp_->Stop(); }
}  // namespace ottest
