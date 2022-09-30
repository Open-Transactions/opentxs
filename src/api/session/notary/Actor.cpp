// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "api/session/notary/Actor.hpp"  // IWYU pragma: associated

#include <chrono>
#include <string_view>
#include <type_traits>
#include <utility>

#include "api/session/notary/Shared.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/api/session/notary/Notary.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Notary.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::api::session::notary
{
auto print(Job value) noexcept -> std::string_view
{
    using namespace std::literals;
    static const auto map = Map<Job, std::string_view>{
        {Job::shutdown, "shutdown"sv},
        {Job::queue_unitid, "queue_unitid"sv},
        {Job::init, "init"sv},
        {Job::statemachine, "statemachine"sv},
    };

    try {
        return map.at(value);
    } catch (...) {

        return "Unknown Job type"sv;
    }
}
}  // namespace opentxs::api::session::notary

namespace opentxs::api::session::notary
{
Actor::Actor(
    std::shared_ptr<api::session::Notary> api,
    boost::shared_ptr<Shared> shared,
    allocator_type alloc) noexcept
    : opentxs::Actor<notary::Actor, Job>(
          *api,
          LogTrace(),
          {"notary api", alloc},
          0s,
          shared->batch_id_,
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
              pull.emplace_back(shared->endpoint_, Dir::Connect);

              return pull;
          }())
    , api_p_(std::move(api))
    , shared_p_(std::move(shared))
    , api_(*api_p_)
    , shared_(*shared_p_)
    , queue_(alloc)
{
}

auto Actor::do_shutdown() noexcept -> void { queue_.clear(); }

auto Actor::do_startup() noexcept -> bool
{
    if ((api_.Internal().ShuttingDown())) { return true; }

    for (const auto& [id, alias] : api_.Wallet().UnitDefinitionList()) {
        queue_.emplace_back(api_.Factory().UnitIDFromBase58(id));
    }

    return false;
}

auto Actor::pipeline(const Work work, Message&& msg) noexcept -> void
{
    switch (work) {
        case Work::queue_unitid: {
            process_queue_unitid(std::move(msg));
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

auto Actor::process_queue_unitid(Message&& msg) noexcept -> void
{
    const auto body = msg.Body();

    OT_ASSERT(1_uz < body.size());

    auto& id = queue_.emplace_back();
    id.Assign(body.at(1).Bytes());
    do_work();
}

auto Actor::work() noexcept -> bool
{
    if (false == queue_.empty()) {
        auto out = ScopeGuard{[&] { queue_.pop_front(); }};
        const auto& unitID = queue_.front();
        api_.InternalNotary().CheckMint(unitID);
    }

    return false == queue_.empty();
}

Actor::~Actor() = default;
}  // namespace opentxs::api::session::notary
