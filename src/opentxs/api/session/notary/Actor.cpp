// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/session/notary/Actor.hpp"  // IWYU pragma: associated

#include <chrono>
#include <span>
#include <utility>

#include "internal/util/P0330.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/Session.internal.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Notary.hpp"
#include "opentxs/api/session/Notary.internal.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/api/session/notary/Shared.hpp"
#include "opentxs/api/session/notary/Types.internal.hpp"
#include "opentxs/identifier/UnitDefinition.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/socket/Direction.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Policy.hpp"     // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::api::session::notary
{
using enum opentxs::network::zeromq::socket::Direction;
using enum opentxs::network::zeromq::socket::Policy;

Actor::Actor(
    std::shared_ptr<api::session::internal::Notary> api,
    std::shared_ptr<Shared> shared,
    allocator_type alloc) noexcept
    : opentxs::Actor<notary::Actor, Job>(
          api->asNotaryPublic(),
          LogTrace(),
          {"notary api", alloc},
          0s,
          shared->batch_id_,
          alloc,
          {
              {api->Endpoints().Shutdown(), Connect},
          },
          {
              {shared->endpoint_, Connect},
          })
    , api_p_(std::move(api))
    , shared_p_(std::move(shared))
    , api_(api_p_->asNotaryPublic())
    , shared_(*shared_p_)
    , queue_(alloc)
{
}

auto Actor::do_shutdown() noexcept -> void
{
    queue_.clear();
    shared_p_.reset();
    api_p_.reset();
}

auto Actor::do_startup(allocator_type) noexcept -> bool
{
    if ((api_.Internal().ShuttingDown())) { return true; }

    for (const auto& [id, alias] : api_.Wallet().UnitDefinitionList()) {
        queue_.emplace_back(api_.Factory().UnitIDFromBase58(id));
    }

    return false;
}

auto Actor::pipeline(
    const Work work,
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    switch (work) {
        case Work::queue_unitid: {
            process_queue_unitid(std::move(msg), monotonic);
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

auto Actor::process_queue_unitid(
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    const auto body = msg.Payload();

    assert_true(1_uz < body.size());

    auto& id = queue_.emplace_back();
    id.Assign(body[1].Bytes());
    do_work(monotonic);
}

auto Actor::work(allocator_type monotonic) noexcept -> bool
{
    if (false == queue_.empty()) {
        auto out = ScopeGuard{[&] { queue_.pop_front(); }};
        const auto& unitID = queue_.front();
        api_.Internal().asNotary().CheckMint(unitID);
    }

    return false == queue_.empty();
}

Actor::~Actor() = default;
}  // namespace opentxs::api::session::notary
