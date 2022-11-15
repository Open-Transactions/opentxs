// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type
// IWYU pragma: no_forward_declare opentxs::blockchain::cfilter::Type

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "blockchain/node/wallet/subchain/statemachine/Job.hpp"  // IWYU pragma: associated

#include <boost/system/error_code.hpp>  // IWYU pragma: keep
#include <algorithm>
#include <chrono>
#include <iterator>
#include <ratio>
#include <string_view>
#include <utility>

#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/blockchain/node/wallet/Reorg.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Types.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::wallet
{
using namespace std::literals;

auto print(JobState state) noexcept -> std::string_view
{
    try {
        static const auto map = Map<JobState, std::string_view>{
            {JobState::normal, "normal"sv},
            {JobState::reorg, "reorg"sv},
            {JobState::shutdown, "shutdown"sv},
        };

        return map.at(state);
    } catch (...) {
        LogAbort()(__FUNCTION__)(": invalid JobState: ")(
            static_cast<OTZMQWorkType>(state))
            .Abort();
    }
}

auto print(JobType state) noexcept -> std::string_view
{
    try {
        static const auto map = Map<JobType, std::string_view>{
            {JobType::scan, "scan"sv},
            {JobType::process, "process"sv},
            {JobType::index, "index"sv},
            {JobType::rescan, "rescan"sv},
            {JobType::progress, "progress"sv},
        };

        return map.at(state);
    } catch (...) {
        LogAbort()(__FUNCTION__)(": invalid JobType: ")(
            static_cast<OTZMQWorkType>(state))
            .Abort();
    }
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet::statemachine
{
Job::Job(
    const Log& logger,
    const boost::shared_ptr<const SubchainStateData>& parent,
    const network::zeromq::BatchID batch,
    const JobType type,
    allocator_type alloc,
    const network::zeromq::EndpointArgs& subscribe,
    const network::zeromq::EndpointArgs& pull,
    const network::zeromq::EndpointArgs& dealer,
    const Vector<network::zeromq::SocketData>& extra,
    Set<Work>&& neverDrop) noexcept
    : Actor(
          parent->api_,
          logger,
          [&] {
              using namespace std::literals;
              auto out = CString{alloc};
              out.append(print(type));
              out.append(" job for "sv);
              out.append(parent->name_);

              return out;
          }(),
          1ms,
          batch,
          alloc,
          [&] {
              auto out{subscribe};
              out.emplace_back(parent->from_parent_, Direction::Connect);
              out.emplace_back(parent->from_ssd_endpoint_, Direction::Connect);

              return out;
          }(),
          pull,
          dealer,
          [&] {
              auto out = Vector<network::zeromq::SocketData>{
                  {SocketType::Push,
                   {{parent->to_ssd_endpoint_, Direction::Connect}}}};
              std::copy(extra.begin(), extra.end(), std::back_inserter(out));

              return out;
          }(),
          std::move(neverDrop))
    , parent_p_(parent)
    , parent_(*parent_p_)
    , reorg_(parent_.GetReorg().GetSlave(pipeline_, name_, alloc))
    , job_type_(type)
    , to_parent_(pipeline_.Internal().ExtraSocket(0))
    , pending_state_(State::normal)
    , state_(State::normal)
    , reorgs_(alloc)
    , watchdog_(parent_.api_.Network().Asio().Internal().GetTimer())
{
    OT_ASSERT(parent_p_);
}

auto Job::add_last_reorg(Message& out) const noexcept -> void
{
    if (const auto epoc = last_reorg(); epoc.has_value()) {
        out.AddFrame(epoc.value());
    } else {
        out.AddFrame();
    }
}

auto Job::do_process_update(Message&& msg, allocator_type) noexcept -> void
{
    LogAbort()(OT_PRETTY_CLASS())(name_)(" unhandled message type").Abort();
}

auto Job::do_reorg(
    const node::HeaderOracle& oracle,
    const node::internal::HeaderOraclePrivate& data,
    Reorg::Params& params) noexcept -> bool
{
    return true;
}

auto Job::do_shutdown() noexcept -> void
{
    state_ = State::shutdown;
    reorg_.Stop();
    parent_p_.reset();
}

auto Job::do_startup(allocator_type monotonic) noexcept -> bool
{
    if (Reorg::State::shutdown == reorg_.Start()) { return true; }

    do_startup_internal(monotonic);

    return false;
}

auto Job::last_reorg() const noexcept -> std::optional<StateSequence>
{
    if (0_uz == reorgs_.size()) {

        return std::nullopt;
    } else {

        return *reorgs_.crbegin();
    }
}

auto Job::pipeline(
    const Work work,
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    switch (state_) {
        case State::normal: {
            state_normal(work, std::move(msg), monotonic);
        } break;
        case State::reorg: {
            state_reorg(work, std::move(msg));
        } break;
        case State::pre_shutdown: {
            state_pre_shutdown(work, std::move(msg));
        } break;
        case State::shutdown: {
            // NOTE do nothing
        } break;
        default: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(": invalid state").Abort();
        }
    }

    process_watchdog();
}

auto Job::process_block(Message&& in, allocator_type monotonic) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(2_uz < body.size());

    const auto chain = body.at(1).as<blockchain::Type>();

    if (parent_.chain_ != chain) { return; }

    process_block(block::Hash{body.at(2).Bytes()}, monotonic);
}

auto Job::process_block(block::Hash&&, allocator_type) noexcept -> void
{
    LogAbort()(OT_PRETTY_CLASS())(name_)(" unhandled message type").Abort();
}

auto Job::process_filter(Message&& in, allocator_type monotonic) noexcept
    -> void
{
    const auto body = in.Body();

    OT_ASSERT(3_uz < body.size());

    const auto type = body.at(1).as<cfilter::Type>();

    if (type != parent_.node_.FilterOracle().DefaultType()) { return; }

    auto position =
        block::Position{body.at(2).as<block::Height>(), body.at(3).Bytes()};
    process_filter(std::move(in), std::move(position), monotonic);
}

auto Job::process_filter(Message&&, block::Position&&, allocator_type) noexcept
    -> void
{
    LogAbort()(OT_PRETTY_CLASS())(name_)(" unhandled message type").Abort();
}

auto Job::process_key(Message&& in, allocator_type) noexcept -> void
{
    LogAbort()(OT_PRETTY_CLASS())(name_)(" unhandled message type").Abort();
}

auto Job::process_prepare_reorg(Message&& in) noexcept -> void
{
    const auto body = in.Body();

    OT_ASSERT(1u < body.size());

    transition_state_reorg(body.at(1).as<StateSequence>());
}

auto Job::process_process(Message&& in, allocator_type monotonic) noexcept
    -> void
{
    const auto body = in.Body();

    OT_ASSERT(2_uz < body.size());

    process_process(
        block::Position{body.at(1).as<block::Height>(), body.at(2).Bytes()},
        monotonic);
}

auto Job::process_process(block::Position&&, allocator_type) noexcept -> void
{
    LogAbort()(OT_PRETTY_CLASS())(name_)("unhandled message type").Abort();
}

auto Job::process_reprocess(Message&&, allocator_type) noexcept -> void
{
    LogAbort()(OT_PRETTY_CLASS())(name_)(" unhandled message type").Abort();
}

auto Job::process_start_scan(Message&&, allocator_type) noexcept -> void
{
    LogAbort()(OT_PRETTY_CLASS())(name_)(" unhandled message type").Abort();
}

auto Job::process_mempool(Message&&, allocator_type) noexcept -> void
{
    LogAbort()(OT_PRETTY_CLASS())(name_)(" unhandled message type").Abort();
}

auto Job::process_update(Message&& msg, allocator_type monotonic) noexcept
    -> void
{
    const auto body = msg.Body();

    OT_ASSERT(1_uz < body.size());

    const auto& epoc = body.at(1);
    const auto expected = last_reorg();

    if (0_uz == epoc.size()) {
        if (expected.has_value()) {
            log_(OT_PRETTY_CLASS())(name_)(" ignoring stale update").Flush();

            return;
        }
    } else {
        if (expected.has_value()) {
            const auto reorg = epoc.as<StateSequence>();

            if (reorg != expected.value()) {
                log_(OT_PRETTY_CLASS())(name_)(" ignoring stale update")
                    .Flush();

                return;
            }
        } else {
            log_(OT_PRETTY_CLASS())(name_)(" ignoring stale update").Flush();

            return;
        }
    }

    do_process_update(std::move(msg), monotonic);
}

auto Job::process_watchdog() noexcept -> void
{
    to_parent_.SendDeferred(
        [&] {
            auto out = MakeWork(Work::watchdog_ack);
            out.AddFrame(job_type_);

            return out;
        }(),
        __FILE__,
        __LINE__);
    using namespace std::literals;
    reset_timer(10s, watchdog_, Work::watchdog);
}

auto Job::state_normal(
    const Work work,
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    switch (work) {
        case Work::filter: {
            process_filter(std::move(msg), monotonic);
        } break;
        case Work::mempool: {
            process_mempool(std::move(msg), monotonic);
        } break;
        case Work::block: {
            process_block(std::move(msg), monotonic);
        } break;
        case Work::start_scan: {
            process_start_scan(std::move(msg), monotonic);
        } break;
        case Work::prepare_reorg: {
            process_prepare_reorg(std::move(msg));
        } break;
        case Work::update: {
            process_update(std::move(msg), monotonic);
        } break;
        case Work::process: {
            process_process(std::move(msg), monotonic);
        } break;
        case Work::rescan: {
            // NOTE ignore message
        } break;
        case Work::do_rescan: {
            process_do_rescan(std::move(msg));
        } break;
        case Work::watchdog: {
            process_watchdog();
        } break;
        case Work::reprocess: {
            process_reprocess(std::move(msg), monotonic);
        } break;
        case Work::key: {
            process_key(std::move(msg), monotonic);
        } break;
        case Work::prepare_shutdown: {
            transition_state_pre_shutdown();
        } break;
        case Work::finish_reorg: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(" wrong state for ")(
                print(work))(" message")
                .Abort();
        }
        case Work::shutdown:
        case Work::watchdog_ack:
        case Work::init:
        case Work::statemachine: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(": unhandled message type ")(
                print(work))
                .Abort();
        }
        default: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(" unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Abort();
        }
    }
}

auto Job::state_pre_shutdown(const Work work, Message&& msg) noexcept -> void
{
    switch (work) {
        case Work::filter:
        case Work::mempool:
        case Work::block:
        case Work::start_scan:
        case Work::update:
        case Work::process:
        case Work::rescan:
        case Work::do_rescan:
        case Work::watchdog:
        case Work::reprocess:
        case Work::key: {
            // NOTE ignore message
        } break;
        case Work::prepare_reorg:
        case Work::finish_reorg:
        case Work::prepare_shutdown: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(" wrong state for ")(
                print(work))(" message")
                .Abort();
        }
        case Work::shutdown:
        case Work::watchdog_ack:
        case Work::init:
        case Work::statemachine: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(": unhandled message type ")(
                print(work))
                .Abort();
        }
        default: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(" unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Abort();
        }
    }
}

auto Job::state_reorg(const Work work, Message&& msg) noexcept -> void
{
    switch (work) {
        case Work::filter:
        case Work::update: {
            // NOTE ignore message
        } break;
        case Work::mempool:
        case Work::block:
        case Work::start_scan:
        case Work::prepare_reorg:
        case Work::process:
        case Work::reprocess:
        case Work::rescan:
        case Work::do_rescan:
        case Work::key: {
            log_(OT_PRETTY_CLASS())(name_)(" deferring ")(print(work))(
                " message processing until reorg is complete")
                .Flush();
            defer(std::move(msg));
        } break;
        case Work::finish_reorg: {
            transition_state_normal();
        } break;
        case Work::prepare_shutdown: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(" wrong state for ")(
                print(work))(" message")
                .Abort();
        }
        case Work::watchdog: {
            process_watchdog();
        } break;
        case Work::shutdown:
        case Work::watchdog_ack:
        case Work::init:
        case Work::statemachine: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(": unhandled message type ")(
                print(work))
                .Abort();
        }
        default: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(" unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Abort();
        }
    }
}

auto Job::transition_state_normal() noexcept -> void
{
    state_ = State::normal;
    log_(OT_PRETTY_CLASS())(name_)(" transitioned to normal state ").Flush();
    trigger();
}

auto Job::transition_state_pre_shutdown() noexcept -> void
{
    watchdog_.Cancel();
    reorg_.AcknowledgeShutdown();
    state_ = State::pre_shutdown;
    log_(OT_PRETTY_CLASS())(name_)(": transitioned to pre_shutdown state")
        .Flush();
}

auto Job::transition_state_reorg(StateSequence id) noexcept -> void
{
    OT_ASSERT(0_uz < id);

    if (0_uz == reorgs_.count(id)) {
        reorgs_.emplace(id);
        state_ = State::reorg;
        log_(OT_PRETTY_CLASS())(name_)(" ready to process reorg ")(id).Flush();
        reorg_.AcknowledgePrepareReorg(
            [this](const auto& header, const auto& lock, auto& params) {
                return do_reorg(header, lock, params);
            });
    } else {
        LogAbort()(OT_PRETTY_CLASS())(name_)(" reorg ")(id)(" already handled")
            .Abort();
    }
}

auto Job::work(allocator_type monotonic) noexcept -> bool
{
    process_watchdog();

    return false;
}

Job::~Job() = default;
}  // namespace opentxs::blockchain::node::wallet::statemachine
