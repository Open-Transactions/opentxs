// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "blockchain/node/blockoracle/cache/Actor.hpp"  // IWYU pragma: associated

#include <chrono>
#include <exception>
#include <memory>
#include <utility>

#include "blockchain/node/blockoracle/cache/Shared.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"

namespace opentxs::blockchain::node::blockoracle
{
Cache::Actor::Actor(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const node::Manager> node,
    boost::shared_ptr<Shared> shared,
    network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : opentxs::Actor<Cache::Actor, CacheJob>(
          *api,
          LogTrace(),
          [&] {
              using namespace std::literals;
              auto out = CString{alloc};
              out.append(print(node->Internal().Chain()));
              out.append(" block cache"sv);

              return out;
          }(),
          0ms,
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
              pull.emplace_back(
                  node->Internal().Endpoints().block_cache_pull_,
                  Direction::Bind);

              return pull;
          }())
    , api_p_(std::move(api))
    , node_p_(std::move(node))
    , shared_(std::move(shared))
    , api_(*api_p_)
    , node_(*node_p_)
    , data_(shared_->data_)
    , heartbeat_(api_.Network().Asio().Internal().GetTimer())
{
}

auto Cache::Actor::do_shutdown() noexcept -> void
{
    heartbeat_.Cancel();
    data_.lock()->Shutdown();
    shared_.reset();
    node_p_.reset();
    api_p_.reset();
}

auto Cache::Actor::do_startup() noexcept -> bool
{
    if ((api_.Internal().ShuttingDown()) || (node_.Internal().ShuttingDown())) {
        return true;
    }

    do_work();

    return false;
}

auto Cache::Actor::pipeline(const Work work, Message&& msg) noexcept -> void
{
    switch (work) {
        case Work::shutdown: {
            shutdown_actor();
        } break;
        case Work::request_blocks: {
            data_.lock()->ProcessBlockRequests(std::move(msg));
        } break;
        case Work::process_block: {
            const auto body = msg.Body();

            OT_ASSERT(1_uz < body.size());

            data_.lock()->ReceiveBlock(body.at(1).Bytes());
        } break;
        case Work::init: {
            do_init();
        } break;
        case Work::statemachine: {
            do_work();
        } break;
        default: {
            LogError()(OT_PRETTY_CLASS())(name_)(" unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Flush();

            OT_FAIL;
        }
    }
}

auto Cache::Actor::work() noexcept -> bool
{
    if (data_.lock()->StateMachine()) {
        reset_timer(heartbeat_interval_, heartbeat_, Work::statemachine);
    } else {
        heartbeat_.Cancel();
    }

    return false;
}

Cache::Actor::~Actor() = default;
}  // namespace opentxs::blockchain::node::blockoracle
