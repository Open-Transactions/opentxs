// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "blockchain/node/stats/Actor.hpp"  // IWYU pragma: associated

#include <chrono>
#include <cstddef>
#include <string_view>
#include <utility>

#include "blockchain/node/stats/Shared.hpp"
#include "internal/api/network/Types.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/blockchain/node/Types.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node
{
using namespace std::literals;

auto print(StatsJobs state) noexcept -> std::string_view
{
    using Job = StatsJobs;

    try {
        static const auto map = Map<Job, std::string_view>{
            {Job::shutdown, "shutdown"sv},
            {Job::block_header, "block_header"sv},
            {Job::reorg, "reorg"sv},
            {Job::cfilter, "cfilter"sv},
            {Job::peer, "peer"sv},
            {Job::sync_server, "sync_server"sv},
            {Job::block, "block"sv},
            {Job::init, "init"sv},
            {Job::statemachine, "statemachine"sv},
        };

        return map.at(state);
    } catch (...) {
        LogAbort()(__FUNCTION__)(": invalid StatsJobs: ")(
            static_cast<OTZMQWorkType>(state))
            .Abort();
    }
}
}  // namespace opentxs::blockchain::node

namespace opentxs::blockchain::node::stats
{
Actor::Actor(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<Shared> shared,
    network::zeromq::BatchID batchID,
    allocator_type alloc) noexcept
    : opentxs::Actor<stats::Actor, Work>(
          *api,
          LogTrace(),
          {"blockchain stats", alloc},
          0ms,
          batchID,
          alloc,
          [&] {
              using Dir = network::zeromq::socket::Direction;
              auto sub = network::zeromq::EndpointArgs{alloc};
              sub.emplace_back(
                  CString{
                      api->Endpoints().BlockchainBlockOracleProgress(), alloc},
                  Dir::Connect);
              sub.emplace_back(
                  CString{api->Endpoints().BlockchainNewFilter(), alloc},
                  Dir::Connect);
              sub.emplace_back(
                  CString{api->Endpoints().BlockchainPeer(), alloc},
                  Dir::Connect);
              sub.emplace_back(
                  CString{api->Endpoints().BlockchainReorg(), alloc},
                  Dir::Connect);
              sub.emplace_back(
                  CString{
                      api->Endpoints().BlockchainSyncServerProgress(), alloc},
                  Dir::Connect);
              sub.emplace_back(
                  CString{api->Endpoints().Shutdown(), alloc}, Dir::Connect);

              return sub;
          }(),
          [&] {
              using Dir = network::zeromq::socket::Direction;
              auto pull = network::zeromq::EndpointArgs{alloc};
              pull.emplace_back(shared->endpoint_, Dir::Connect);

              return pull;
          }(),
          {},
          [&] {
              auto out = Vector<network::zeromq::SocketData>{alloc};
              using Socket = network::zeromq::socket::Type;
              using Args = network::zeromq::EndpointArgs;
              using Dir = network::zeromq::socket::Direction;
              out.emplace_back(std::make_pair<Socket, Args>(
                  Socket::Push,
                  {
                      {CString{
                           api->Endpoints()
                               .Internal()
                               .BlockchainMessageRouter(),
                           alloc},
                       Dir::Connect},
                  }));  // NOTE to_blockchain_api_

              return out;
          }())
    , api_p_(std::move(api))
    , shared_(std::move(shared))
    , api_(*api_p_)
    , data_(*shared_)
    , to_blockchain_api_(pipeline_.Internal().ExtraSocket(0_uz))
{
}

auto Actor::do_shutdown() noexcept -> void {}

auto Actor::do_startup(allocator_type monotonic) noexcept -> bool
{
    if (api_.Internal().ShuttingDown()) {
        do_work(monotonic);

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
            process_block_header(std::move(msg));
        } break;
        case Work::reorg: {
            process_reorg(std::move(msg));
        } break;
        case Work::cfilter: {
            process_cfilter(std::move(msg));
        } break;
        case Work::peer: {
            process_peer(std::move(msg));
        } break;
        case Work::sync_server: {
            process_sync_server(std::move(msg));
        } break;
        case Work::block: {
            process_block(std::move(msg));
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

auto Actor::process_block(Message&& msg) noexcept -> void
{
    const auto body = msg.Body();

    if (3_uz >= body.size()) {
        LogAbort()(OT_PRETTY_CLASS())(name_)(": invalid message").Abort();
    }

    data_.SetBlockTip(
        body.at(1).as<Type>(),
        {body.at(2).as<block::Height>(), body.at(3).Bytes()});
}

auto Actor::process_block_header(Message&& msg) noexcept -> void
{
    const auto body = msg.Body();

    if (3_uz >= body.size()) {
        LogAbort()(OT_PRETTY_CLASS())(name_)(": invalid message").Abort();
    }

    data_.SetBlockHeaderTip(
        body.at(1).as<Type>(),
        {body.at(3).as<block::Height>(), body.at(2).Bytes()});
}

auto Actor::process_cfilter(Message&& msg) noexcept -> void
{
    const auto body = msg.Body();

    if (4_uz >= body.size()) {
        LogAbort()(OT_PRETTY_CLASS())(name_)(": invalid message").Abort();
    }

    data_.SetCfilterTip(
        body.at(1).as<Type>(),
        {body.at(3).as<block::Height>(), body.at(4).Bytes()});
}

auto Actor::process_peer(Message&& msg) noexcept -> void
{
    const auto body = msg.Body();

    if (3_uz >= body.size()) {
        LogAbort()(OT_PRETTY_CLASS())(name_)(": invalid message").Abort();
    }

    data_.SetPeerCount(body.at(1).as<Type>(), body.at(3).as<std::size_t>());
}

auto Actor::process_reorg(Message&& msg) noexcept -> void
{
    const auto body = msg.Body();

    if (5_uz >= body.size()) {
        LogAbort()(OT_PRETTY_CLASS())(name_)(": invalid message").Abort();
    }

    data_.SetBlockHeaderTip(
        body.at(1).as<Type>(),
        {body.at(5).as<block::Height>(), body.at(4).Bytes()});
}

auto Actor::process_sync_server(Message&& msg) noexcept -> void
{
    const auto body = msg.Body();

    if (3_uz >= body.size()) {
        LogAbort()(OT_PRETTY_CLASS())(name_)(": invalid message").Abort();
    }

    data_.SetSyncTip(
        body.at(1).as<Type>(),
        {body.at(2).as<block::Height>(), body.at(3).Bytes()});
}

auto Actor::work(allocator_type monotonic) noexcept -> bool
{
    using Job = api::network::BlockchainJob;
    to_blockchain_api_.SendDeferred(
        MakeWork(Job::report_status), __FILE__, __LINE__);

    return false;
}

Actor::~Actor() = default;
}  // namespace opentxs::blockchain::node::stats
