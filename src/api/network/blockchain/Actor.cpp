// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/network/blockchain/Actor.hpp"  // IWYU pragma: associated

#include <chrono>
#include <string_view>
#include <utility>

#include "internal/api/session/Endpoints.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/Session.internal.hpp"
#include "opentxs/api/network/Types.internal.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/network/zeromq/socket/Direction.hpp"   // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Policy.hpp"      // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::api::network
{
using namespace std::literals;

auto print(BlockchainJob state) noexcept -> std::string_view
{
    using Job = BlockchainJob;

    try {
        static const auto map = Map<Job, std::string_view>{
            {Job::shutdown, "shutdown"sv},
            {Job::block_header, "block_header"sv},
            {Job::peer_active, "peer_active"sv},
            {Job::reorg, "reorg"sv},
            {Job::cfilter_progress, "cfilter_progress"sv},
            {Job::cfilter, "cfilter"sv},
            {Job::block_queue, "block_queue"sv},
            {Job::peer_connected, "peer_connected"sv},
            {Job::mempool, "mempool"sv},
            {Job::block_available, "block_available"sv},
            {Job::sync_server, "sync_server"sv},
            {Job::block, "block"sv},
            {Job::report_status, "report_status"sv},
            {Job::init, "init"sv},
            {Job::statemachine, "statemachine"sv},
        };

        return map.at(state);
    } catch (...) {
        LogAbort()(__FUNCTION__)(": invalid BlockchainJob: ")(
            static_cast<OTZMQWorkType>(state))
            .Abort();
    }
}
}  // namespace opentxs::api::network

namespace opentxs::api::network::blockchain
{
using enum opentxs::network::zeromq::socket::Direction;
using enum opentxs::network::zeromq::socket::Policy;
using enum opentxs::network::zeromq::socket::Type;

Actor::Actor(
    std::shared_ptr<const api::internal::Session> api,
    opentxs::network::zeromq::BatchID batchID,
    allocator_type alloc) noexcept
    : opentxs::Actor<blockchain::Actor, Work>(
          api->Self(),
          LogTrace(),
          {"blockchain status router", alloc},
          0ms,
          batchID,
          alloc,
          {
              {api->Endpoints().Shutdown(), Connect},
          },
          {
              {api->Endpoints().Internal().BlockchainMessageRouter(), Bind},
          },
          {},
          {
              {Publish,
               Internal,
               {
                   {api->Endpoints().BlockchainPeer(), Bind},
               }},
              {Publish,
               Internal,
               {
                   {api->Endpoints().BlockchainBlockAvailable(), Bind},
               }},
              {Publish,
               Internal,
               {
                   {api->Endpoints().BlockchainBlockDownloadQueue(), Bind},
               }},
              {Publish,
               Internal,
               {
                   {api->Endpoints().BlockchainBlockOracleProgress(), Bind},
               }},
              {Publish,
               Internal,
               {
                   {api->Endpoints().BlockchainSyncProgress(), Bind},
               }},
              {Publish,
               Internal,
               {
                   {api->Endpoints().BlockchainNewFilter(), Bind},
               }},
              {Publish,
               Internal,
               {
                   {api->Endpoints().BlockchainPeerConnection(), Bind},
               }},
              {Publish,
               Internal,
               {
                   {api->Endpoints().BlockchainMempool(), Bind},
               }},
              {Publish,
               Internal,
               {
                   {api->Endpoints().BlockchainReorg(), Bind},
               }},
              {Publish,
               Internal,
               {
                   {api->Endpoints().Internal().BlockchainReportStatus(), Bind},
               }},
              {Publish,
               Internal,
               {
                   {api->Endpoints().BlockchainSyncServerProgress(), Bind},
               }},
          })
    , api_p_(std::move(api))
    , api_(api_p_->Self())
    , active_peers_(pipeline_.Internal().ExtraSocket(0))
    , block_available_(pipeline_.Internal().ExtraSocket(1))
    , block_queue_(pipeline_.Internal().ExtraSocket(2))
    , block_tip_(pipeline_.Internal().ExtraSocket(3))
    , cfilter_progress_(pipeline_.Internal().ExtraSocket(4))
    , cfilter_tip_(pipeline_.Internal().ExtraSocket(5))
    , connected_peers_(pipeline_.Internal().ExtraSocket(6))
    , mempool_(pipeline_.Internal().ExtraSocket(7))
    , reorg_(pipeline_.Internal().ExtraSocket(8))
    , report_status_(pipeline_.Internal().ExtraSocket(9))
    , sync_server_(pipeline_.Internal().ExtraSocket(10))
{
}

auto Actor::do_shutdown() noexcept -> void { api_p_.reset(); }

auto Actor::do_startup(allocator_type) noexcept -> bool
{
    if (api_.Internal().ShuttingDown()) {

        return true;
    } else {

        return false;
    }
}

auto Actor::pipeline(const Work work, Message&& msg, allocator_type) noexcept
    -> void
{
    switch (work) {
        case Work::block_header: {
            reorg_.SendDeferred(std::move(msg));
        } break;
        case Work::peer_active: {
            active_peers_.SendDeferred(std::move(msg));
        } break;
        case Work::reorg: {
            reorg_.SendDeferred(std::move(msg));
        } break;
        case Work::cfilter_progress: {
            cfilter_progress_.SendDeferred(std::move(msg));
        } break;
        case Work::cfilter: {
            cfilter_tip_.SendDeferred(std::move(msg));
        } break;
        case Work::block_queue: {
            block_queue_.SendDeferred(std::move(msg));
        } break;
        case Work::peer_connected: {
            connected_peers_.SendDeferred(std::move(msg));
        } break;
        case Work::mempool: {
            mempool_.SendDeferred(std::move(msg));
        } break;
        case Work::block_available: {
            block_available_.SendDeferred(std::move(msg));
        } break;
        case Work::sync_server: {
            sync_server_.SendDeferred(std::move(msg));
        } break;
        case Work::block: {
            block_tip_.SendDeferred(std::move(msg));
        } break;
        case Work::report_status: {
            report_status_.SendDeferred(std::move(msg));
        } break;
        case Work::shutdown:
        case Work::init:
        case Work::statemachine: {
            LogAbort()()(name_)(": unhandled message type ")(print(work))
                .Abort();
        }
        default: {
            LogAbort()()(name_)(": unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Abort();
        }
    }
}

auto Actor::work(allocator_type monotonic) noexcept -> bool { return false; }

Actor::~Actor() = default;
}  // namespace opentxs::api::network::blockchain
