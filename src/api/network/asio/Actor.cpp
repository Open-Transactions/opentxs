// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/network/asio/Actor.hpp"  // IWYU pragma: associated

#include <chrono>
#include <cstdint>
#include <memory>
#include <span>
#include <utility>

#include "api/network/asio/Shared.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/Context.internal.hpp"
#include "opentxs/network/zeromq/message/Envelope.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/Direction.hpp"   // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Policy.hpp"      // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Options.hpp"
#include "opentxs/util/WorkType.hpp"
#include "opentxs/util/WorkType.internal.hpp"

namespace opentxs::api::network::asio
{
using enum opentxs::network::zeromq::socket::Direction;
using enum opentxs::network::zeromq::socket::Policy;
using enum opentxs::network::zeromq::socket::Type;

Actor::Actor(
    std::shared_ptr<const api::internal::Context> context,
    std::shared_ptr<Shared> shared,
    allocator_type alloc) noexcept
    : opentxs::Actor<asio::Actor, OTZMQWorkType>(
          context->Self(),
          LogTrace(),
          {"asio", alloc},
          1s,
          shared->batch_id_,
          alloc,
          {
              {session::internal::Endpoints::ContextShutdown(), Connect},
          },
          {
              {shared->endpoint_, Bind},
          },
          {},
          {
              {Router,
               Internal,
               {
                   {session::internal::Endpoints::Asio(), Bind},
               }},
          })
    , context_p_(std::move(context))
    , shared_p_(std::move(shared))
    , context_(context_p_->Self())
    , shared_(*shared_p_)
    , router_(pipeline_.Internal().ExtraSocket(0_uz))
    , test_(context_.Options().TestMode())
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
    if (const auto id = connection_id(msg); router_.ID() == id) {
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
            LogAbort()()(name_)(": unhandled message type ")(
                opentxs::print(work))
                .Abort();
        }
        default: {
            LogAbort()()(name_)(": unhandled message type ")(
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
            LogAbort()()(name_)(": unhandled message type ")(
                opentxs::print(work))
                .Abort();
        }
        case value(WorkType::AsioResolve):
        default: {
            router_.SendDeferred(std::move(msg));
        }
    }
}

auto Actor::process_registration(Message&& in) noexcept -> void
{
    router_.SendDeferred([&] {
        auto work = opentxs::network::zeromq::tagged_reply_to_message(
            in, WorkType::AsioRegister);
        auto envelope = std::move(in).Envelope();

        assert_true(envelope.IsValid());
        work.MoveFrames(envelope.get());

        return work;
    }());
}

auto Actor::process_resolve(Message&& in) noexcept -> void
{
    if (test_) { return; }

    const auto body = in.Payload();
    auto envelope = std::move(in).Envelope();

    assert_true(envelope.IsValid());
    assert_true(2_uz < body.size());

    shared_.Resolve(
        shared_p_, envelope, body[1].Bytes(), body[2].as<std::uint16_t>());
}

auto Actor::work(allocator_type monotonic) noexcept -> bool
{
    if (test_) { return false; }

    return shared_.StateMachine();
}

Actor::~Actor() = default;
}  // namespace opentxs::api::network::asio
