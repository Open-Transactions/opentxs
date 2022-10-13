// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "api/network/blockchain/Actor.hpp"  // IWYU pragma: associated

#include <chrono>
#include <string_view>
#include <utility>

#include "internal/api/session/Endpoints.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"

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
Actor::Actor(
    std::shared_ptr<const api::Session> api,
    opentxs::network::zeromq::BatchID batchID,
    allocator_type alloc) noexcept
    : opentxs::Actor<blockchain::Actor, Work>(
          *api,
          LogTrace(),
          {"blockchain status router", alloc},
          0ms,
          batchID,
          alloc,
          [&] {
              using Dir = opentxs::network::zeromq::socket::Direction;
              auto sub = opentxs::network::zeromq::EndpointArgs{alloc};
              sub.emplace_back(
                  CString{api->Endpoints().Shutdown(), alloc}, Dir::Connect);

              return sub;
          }(),
          [&] {
              using Dir = opentxs::network::zeromq::socket::Direction;
              auto pull = opentxs::network::zeromq::EndpointArgs{alloc};
              pull.emplace_back(
                  CString{
                      api->Endpoints().Internal().BlockchainMessageRouter(),
                      alloc},
                  Dir::Bind);

              return pull;
          }(),
          {},
          [&] {
              auto out = Vector<opentxs::network::zeromq::SocketData>{alloc};
              using Socket = opentxs::network::zeromq::socket::Type;
              using Args = opentxs::network::zeromq::EndpointArgs;
              using Dir = opentxs::network::zeromq::socket::Direction;
              out.emplace_back(std::make_pair<Socket, Args>(
                  Socket::Publish,
                  {
                      {CString{api->Endpoints().BlockchainPeer(), alloc},
                       Dir::Bind},
                  }));  // NOTE active_peers_
              out.emplace_back(std::make_pair<Socket, Args>(
                  Socket::Publish,
                  {
                      {CString{
                           api->Endpoints().BlockchainBlockAvailable(), alloc},
                       Dir::Bind},
                  }));  // NOTE block_available_
              out.emplace_back(std::make_pair<Socket, Args>(
                  Socket::Publish,
                  {
                      {CString{
                           api->Endpoints().BlockchainBlockDownloadQueue(),
                           alloc},
                       Dir::Bind},
                  }));  // NOTE block_queue_
              out.emplace_back(std::make_pair<Socket, Args>(
                  Socket::Publish,
                  {
                      {CString{
                           api->Endpoints().BlockchainBlockOracleProgress(),
                           alloc},
                       Dir::Bind},
                  }));  // NOTE block_tip_
              out.emplace_back(std::make_pair<Socket, Args>(
                  Socket::Publish,
                  {
                      {CString{
                           api->Endpoints().BlockchainSyncProgress(), alloc},
                       Dir::Bind},
                  }));  // NOTE cfilter_progress_
              out.emplace_back(std::make_pair<Socket, Args>(
                  Socket::Publish,
                  {
                      {CString{api->Endpoints().BlockchainNewFilter(), alloc},
                       Dir::Bind},
                  }));  // NOTE cfilter_tip_
              out.emplace_back(std::make_pair<Socket, Args>(
                  Socket::Publish,
                  {
                      {CString{
                           api->Endpoints().BlockchainPeerConnection(), alloc},
                       Dir::Bind},
                  }));  // NOTE connected_peers_
              out.emplace_back(std::make_pair<Socket, Args>(
                  Socket::Publish,
                  {
                      {CString{api->Endpoints().BlockchainMempool(), alloc},
                       Dir::Bind},
                  }));  // NOTE mempool_
              out.emplace_back(std::make_pair<Socket, Args>(
                  Socket::Publish,
                  {
                      {CString{api->Endpoints().BlockchainReorg(), alloc},
                       Dir::Bind},
                  }));  // NOTE reorg_
              out.emplace_back(std::make_pair<Socket, Args>(
                  Socket::Publish,
                  {
                      {CString{
                           api->Endpoints().Internal().BlockchainReportStatus(),
                           alloc},
                       Dir::Bind},
                  }));  // NOTE report_status_
              out.emplace_back(std::make_pair<Socket, Args>(
                  Socket::Publish,
                  {
                      {CString{
                           api->Endpoints().BlockchainSyncServerProgress(),
                           alloc},
                       Dir::Bind},
                  }));  // NOTE sync_server_

              return out;
          }())
    , api_p_(std::move(api))
    , api_(*api_p_)
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

auto Actor::do_shutdown() noexcept -> void {}

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
            reorg_.SendDeferred(std::move(msg), __FILE__, __LINE__);
        } break;
        case Work::peer_active: {
            active_peers_.SendDeferred(std::move(msg), __FILE__, __LINE__);
        } break;
        case Work::reorg: {
            reorg_.SendDeferred(std::move(msg), __FILE__, __LINE__);
        } break;
        case Work::cfilter_progress: {
            cfilter_progress_.SendDeferred(std::move(msg), __FILE__, __LINE__);
        } break;
        case Work::cfilter: {
            cfilter_tip_.SendDeferred(std::move(msg), __FILE__, __LINE__);
        } break;
        case Work::block_queue: {
            block_queue_.SendDeferred(std::move(msg), __FILE__, __LINE__);
        } break;
        case Work::peer_connected: {
            connected_peers_.SendDeferred(std::move(msg), __FILE__, __LINE__);
        } break;
        case Work::mempool: {
            mempool_.SendDeferred(std::move(msg), __FILE__, __LINE__);
        } break;
        case Work::block_available: {
            block_available_.SendDeferred(std::move(msg), __FILE__, __LINE__);
        } break;
        case Work::sync_server: {
            sync_server_.SendDeferred(std::move(msg), __FILE__, __LINE__);
        } break;
        case Work::block: {
            block_tip_.SendDeferred(std::move(msg), __FILE__, __LINE__);
        } break;
        case Work::report_status: {
            report_status_.SendDeferred(std::move(msg), __FILE__, __LINE__);
        } break;
        case Work::shutdown:
        case Work::init:
        case Work::statemachine: {
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

auto Actor::work(allocator_type monotonic) noexcept -> bool { return false; }

Actor::~Actor() = default;
}  // namespace opentxs::api::network::blockchain
