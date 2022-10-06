// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "blockchain/node/blockoracle/Actor.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/shared_ptr.hpp>
#include <chrono>
#include <exception>
#include <memory>
#include <string_view>
#include <utility>

#include "blockchain/node/blockoracle/Shared.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs::blockchain::node::internal
{
BlockOracle::Actor::Actor(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const node::Manager> node,
    boost::shared_ptr<Shared> shared,
    network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : BlockOracleActor(
          *api,
          LogTrace(),
          [&] {
              using namespace std::literals;
              auto out = CString{alloc};
              out.append(print(node->Internal().Chain()));
              out.append(" block oracle"sv);

              return out;
          }(),
          50ms,
          std::move(batch),
          alloc,
          [&] {
              auto sub = network::zeromq::EndpointArgs{alloc};
              sub.emplace_back(api->Endpoints().Shutdown(), Direction::Connect);
              sub.emplace_back(
                  node->Internal().Endpoints().shutdown_publish_,
                  Direction::Connect);

              return sub;
          }(),
          [&] {
              auto pull = network::zeromq::EndpointArgs{alloc};
              pull.emplace_back(shared->submit_endpoint_, Direction::Bind);
              pull.emplace_back(
                  node->Internal().Endpoints().block_oracle_pull_,
                  Direction::Bind);

              return pull;
          }(),
          {},
          [&] {
              auto out = Vector<network::zeromq::SocketData>{alloc};
              out.emplace_back(SocketType::Push, [&] {
                  auto extra = Vector<network::zeromq::EndpointArg>{alloc};
                  extra.emplace_back(
                      node->Internal().Endpoints().block_cache_pull_,
                      Direction::Connect);

                  return extra;
              }());

              return out;
          }())
    , api_p_(std::move(api))
    , node_p_(std::move(node))
    , shared_(std::move(shared))
    , api_(*api_p_)
    , node_(*node_p_)
    , cache_(shared_->cache_)
    , to_cache_(pipeline_.Internal().ExtraSocket(0))
{
}

auto BlockOracle::Actor::do_shutdown() noexcept -> void
{
    shared_.reset();
    node_p_.reset();
    api_p_.reset();
}

auto BlockOracle::Actor::do_startup(allocator_type) noexcept -> bool
{
    if ((api_.Internal().ShuttingDown()) || (node_.Internal().ShuttingDown())) {
        return true;
    }

    return false;
}

auto BlockOracle::Actor::Init(boost::shared_ptr<Actor> me) noexcept -> void
{
    signal_startup(me);
}

auto BlockOracle::Actor::pipeline(
    const Work work,
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    switch (work) {
        case Work::request_blocks: {
            to_cache_.SendDeferred(std::move(msg), __FILE__, __LINE__);
        } break;
        case Work::process_block: {
            to_cache_.SendDeferred(std::move(msg), __FILE__, __LINE__);
        } break;
        case Work::shutdown:
        case Work::init:
        case Work::statemachine: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(" unhandled message type ")(
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

auto BlockOracle::Actor::work(allocator_type monotonic) noexcept -> bool
{
    return false;
}

BlockOracle::Actor::~Actor() = default;
}  // namespace opentxs::blockchain::node::internal
