// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                // IWYU pragma: associated
#include "api/network/asio/Actor.hpp"  // IWYU pragma: associated

#include <chrono>
#include <cstdint>
#include <memory>
#include <string_view>
#include <utility>

#include "api/network/asio/Shared.hpp"
#include "internal/api/Context.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/message/Message.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

namespace opentxs::api::network::asio
{
Actor::Actor(
    std::shared_ptr<const api::Context> context,
    boost::shared_ptr<Shared> shared,
    allocator_type alloc) noexcept
    : opentxs::Actor<asio::Actor, OTZMQWorkType>(
          *context,
          LogTrace(),
          {"asio", alloc},
          1s,
          shared->batch_id_,
          alloc,
          [&] {
              using Dir = opentxs::network::zeromq::socket::Direction;
              using Endpoints = session::internal::Endpoints;
              auto sub = opentxs::network::zeromq::EndpointArgs{alloc};
              sub.emplace_back(
                  CString{Endpoints::ContextShutdown(), alloc}, Dir::Connect);

              return sub;
          }(),
          [&] {
              using Dir = opentxs::network::zeromq::socket::Direction;
              auto pull = opentxs::network::zeromq::EndpointArgs{alloc};
              pull.emplace_back(shared->endpoint_, Dir::Bind);

              return pull;
          }(),
          {},
          [&] {
              auto out = Vector<opentxs::network::zeromq::SocketData>{alloc};
              using Socket = opentxs::network::zeromq::socket::Type;
              using Args = opentxs::network::zeromq::EndpointArgs;
              using Dir = opentxs::network::zeromq::socket::Direction;
              using Endpoints = session::internal::Endpoints;
              out.emplace_back(std::make_pair<Socket, Args>(
                  Socket::Router,
                  {
                      {CString{Endpoints::Asio(), alloc}, Dir::Bind},
                  }));

              return out;
          }())
    , context_p_(std::move(context))
    , shared_p_(std::move(shared))
    , context_(*context_p_)
    , shared_(*shared_p_)
    , router_(pipeline_.Internal().ExtraSocket(0_uz))
{
}

auto Actor::do_shutdown() noexcept -> void
{
    shared_p_.reset();
    context_p_.reset();
}

auto Actor::do_startup(allocator_type monotonic) noexcept -> bool
{
    if ((context_.Internal().ShuttingDown())) { return true; }

    do_work(monotonic);

    return false;
}

auto Actor::pipeline(const Work work, Message&& msg, allocator_type) noexcept
    -> void
{
    const auto id =
        msg.Internal().ExtractFront().as<opentxs::network::zeromq::SocketID>();

    if (router_.ID() == id) {
        pipeline_external(work, std::move(msg));
    } else {
        pipeline_internal(work, std::move(msg));
    }
}

auto Actor::pipeline_external(const Work work, Message&& msg) noexcept -> void
{
    switch (work) {
        case value(WorkType::AsioRegister): {
            process_registration(std::move(msg));
        } break;
        case value(WorkType::AsioResolve): {
            process_resolve(std::move(msg));
        } break;
        case value(WorkType::Shutdown):
        case OT_ZMQ_INIT_SIGNAL:
        case OT_ZMQ_STATE_MACHINE_SIGNAL: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(": unhandled message type ")(
                opentxs::print(work))
                .Abort();
        }
        default: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(": unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Abort();
        }
    }
}

auto Actor::pipeline_internal(const Work work, Message&& msg) noexcept -> void
{
    switch (work) {
        case value(WorkType::Shutdown):
        case value(WorkType::AsioRegister):
        case OT_ZMQ_INIT_SIGNAL:
        case OT_ZMQ_STATE_MACHINE_SIGNAL: {
            LogAbort()(OT_PRETTY_CLASS())(name_)(": unhandled message type ")(
                opentxs::print(work))
                .Abort();
        }
        case value(WorkType::AsioResolve):
        default: {
            router_.SendDeferred(std::move(msg), __FILE__, __LINE__);
        }
    }
}

auto Actor::process_registration(Message&& in) noexcept -> void
{
    const auto header = in.Header();

    OT_ASSERT(0_uz < header.size());

    const auto& connectionID = header.at(header.size() - 1_uz);

    OT_ASSERT(0_uz < connectionID.size());

    router_.SendDeferred(
        [&] {
            auto work = opentxs::network::zeromq::tagged_reply_to_message(
                in, WorkType::AsioRegister);
            work.AddFrame(connectionID);

            return work;
        }(),
        __FILE__,
        __LINE__);
}

auto Actor::process_resolve(Message&& in) noexcept -> void
{
    const auto header = in.Header();

    OT_ASSERT(0_uz < header.size());

    const auto& connectionID = header.at(header.size() - 1_uz);

    OT_ASSERT(0_uz < connectionID.size());

    const auto body = in.Body();

    OT_ASSERT(2_uz < body.size());

    shared_.Resolve(
        shared_p_,
        connectionID.Bytes(),
        body.at(1).Bytes(),
        body.at(2).as<std::uint16_t>());
}

auto Actor::work(allocator_type monotonic) noexcept -> bool
{
    return shared_.StateMachine();
}

Actor::~Actor() = default;
}  // namespace opentxs::api::network::asio
