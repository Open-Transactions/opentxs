// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/headeroracle/Actor.hpp"  // IWYU pragma: associated

#include <chrono>
#include <iterator>
#include <memory>
#include <span>
#include <string_view>
#include <utility>

#include "blockchain/node/headeroracle/HeaderOraclePrivate.hpp"
#include "blockchain/node/headeroracle/Shared.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/blockchain/database/Header.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/Types.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/api/Network.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/Session.internal.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/socket/Direction.hpp"   // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Policy.hpp"      // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WorkType.internal.hpp"

namespace opentxs::blockchain::node::internal
{
using enum opentxs::network::zeromq::socket::Direction;
using enum opentxs::network::zeromq::socket::Type;

HeaderOracle::Actor::Actor(
    std::shared_ptr<const api::internal::Session> api,
    std::shared_ptr<const node::Manager> node,
    std::shared_ptr<Shared> shared,
    network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : HeaderOracleActor(
          api->Self(),
          LogTrace(),
          [&] {
              using namespace std::literals;
              auto out = CString{alloc};
              out.append(print(node->Internal().Chain()));
              out.append(" header oracle"sv);

              return out;
          }(),
          0ms,
          std::move(batch),
          alloc,
          {
              {api->Endpoints().Shutdown(), Connect},
              {api->Endpoints().Internal().BlockchainReportStatus(), Connect},
              {node->Internal().Endpoints().shutdown_publish_, Connect},
          },
          {
              {node->Internal().Endpoints().header_oracle_pull_, Bind},
          },
          {},
          {
              {Publish,
               network::zeromq::socket::Policy::Internal,
               {
                   {node->Internal().Endpoints().header_oracle_job_ready_,
                    Bind},
               }},
          })
    , api_p_(std::move(api))
    , node_p_(std::move(node))
    , shared_p_(std::move(shared))
    , api_(api_p_->Self())
    , node_(*node_p_)
    , shared_(*shared_p_)
    , job_ready_(pipeline_.Internal().ExtraSocket(0))
    , chain_(node_.Internal().Chain())
    , job_timer_(api_.Network().Asio().Internal().GetTimer())
{
}

auto HeaderOracle::Actor::do_shutdown() noexcept -> void
{
    shared_p_.reset();
    node_p_.reset();
    api_p_.reset();
}

auto HeaderOracle::Actor::do_startup(allocator_type) noexcept -> bool
{
    if ((api_.Internal().ShuttingDown()) || (node_.Internal().ShuttingDown())) {

        return true;
    }

    const auto best = shared_.BestChain();

    assert_true(0 <= best.height_);

    LogVerbose()(print(chain_))(" chain initialized with best position ")(best)
        .Flush();

    return false;
}

auto HeaderOracle::Actor::Init(std::shared_ptr<Actor> me) noexcept -> void
{
    signal_startup(me);
}

auto HeaderOracle::Actor::pipeline(
    const Work work,
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    switch (work) {
        case Work::update_remote_height: {
            process_update_remote_height(std::move(msg), monotonic);
        } break;
        case Work::job_finished: {
            process_job_finished(std::move(msg));
        } break;
        case Work::submit_block_header: {
            process_submit_block_header(std::move(msg));
        } break;
        case Work::submit_block_hash: {
            process_submit_submit_block_hash(std::move(msg), monotonic);
        } break;
        case Work::report: {
            process_report(std::move(msg));
        } break;
        case Work::shutdown:
        case Work::init:
        case Work::statemachine: {
            LogAbort()()(name_)(": unhandled message type ")(print(work))
                .Abort();
        }
        default: {
            LogAbort()()(name_)(" unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Abort();
        }
    }
}

auto HeaderOracle::Actor::process_job_finished(Message&& in) noexcept -> void
{
    auto handle = shared_.data_.lock();
    auto& data = *handle;
    data.have_outstanding_job_ = false;
    reset_job_timer();
}

auto HeaderOracle::Actor::process_report(Message&& msg) noexcept -> void
{
    shared_.Report();
}

auto HeaderOracle::Actor::process_submit_submit_block_hash(
    Message&& in,
    allocator_type monotonic) noexcept -> void
{
    const auto body = in.Payload();

    if (1_uz > body.size()) { LogAbort()()("Invalid message").Abort(); }

    {
        const auto hash = block::Hash{body[1].Bytes()};
        auto handle = shared_.data_.lock();
        auto& data = *handle;

        if (false == data.database_.HeaderExists(hash)) {
            log_()(name_)(": received notification of unknown block hash ")
                .asHex(hash)
                .Flush();
            data.AddUnknownHash(hash);
        }
    }

    do_work(monotonic);
}

auto HeaderOracle::Actor::process_submit_block_header(Message&& in) noexcept
    -> void
{
    const auto body = in.Payload();

    if (2_uz > body.size()) { LogAbort()()("Invalid message").Abort(); }

    auto headers = [&] {
        auto alloc = get_allocator();
        auto out = Vector<block::Header>{alloc};
        out.reserve(body.size() - 1_uz);
        out.clear();

        for (auto i = std::next(body.begin()); i != body.end(); ++i) {
            out.emplace_back(api_.Factory().BlockHeaderFromNative(
                chain_, i->Bytes(), alloc));
        }

        return out;
    }();

    if (false == headers.empty()) { shared_.AddHeaders(headers); }
}

auto HeaderOracle::Actor::process_update_remote_height(
    Message&& in,
    allocator_type monotonic) noexcept -> void
{
    const auto body = in.Payload();

    assert_true(1_uz < body.size());

    {
        auto handle = shared_.data_.lock();
        auto& data = *handle;
        const auto changed =
            data.UpdateRemoteHeight(body[1].as<block::Height>());

        if (changed) {
            data.to_parent_.SendDeferred([&] {
                using Job = ManagerJobs;
                auto out = MakeWork(Job::statemachine);

                return out;
            }());
        }
    }

    do_work(monotonic);
}

auto HeaderOracle::Actor::reset_job_timer() noexcept -> void
{
    reset_timer(10s, job_timer_, Work::statemachine);
}

auto HeaderOracle::Actor::work(allocator_type monotonic) noexcept -> bool
{
    auto handle = shared_.data_.lock_shared();
    const auto& data = *handle;

    if (data.JobIsAvailable()) {
        log_()(name_)(": signaling job availability").Flush();
        job_ready_.SendDeferred(MakeWork(OT_ZMQ_HEADER_ORACLE_JOB_READY));
        reset_job_timer();
    } else {
        log_()(name_)(": no job available").Flush();
    }

    return false;
}

HeaderOracle::Actor::~Actor() = default;
}  // namespace opentxs::blockchain::node::internal
