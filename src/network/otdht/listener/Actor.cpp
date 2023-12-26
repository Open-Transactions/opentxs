// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/otdht/listener/Actor.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <chrono>
#include <functional>
#include <iterator>
#include <memory>
#include <span>
#include <stdexcept>
#include <utility>

#include "internal/api/network/Asio.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/network/otdht/Factory.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/WorkType.internal.hpp"
#include "opentxs/api/Network.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/Session.internal.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/network/blockchain/Types.internal.hpp"
#include "opentxs/network/otdht/Acknowledgement.hpp"
#include "opentxs/network/otdht/Base.hpp"
#include "opentxs/network/otdht/Data.hpp"
#include "opentxs/network/otdht/MessageType.hpp"  // IWYU pragma: keep
#include "opentxs/network/otdht/PushTransaction.hpp"
#include "opentxs/network/otdht/Request.hpp"
#include "opentxs/network/otdht/State.hpp"
#include "opentxs/network/otdht/Types.hpp"
#include "opentxs/network/otdht/Types.internal.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Direction.hpp"   // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Policy.hpp"      // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::network::otdht
{
using namespace std::literals;
using enum opentxs::network::zeromq::socket::Direction;
using enum opentxs::network::zeromq::socket::Policy;
using opentxs::network::zeromq::socket::Type;

Listener::Actor::Actor(
    std::shared_ptr<const api::internal::Session> api,
    std::shared_ptr<Node::Shared> shared,
    std::string_view routerBind,
    std::string_view routerAdvertise,
    std::string_view publishBind,
    std::string_view publishAdvertise,
    std::string_view routingID,
    std::string_view fromNode,
    zeromq::BatchID batchID,
    Vector<zeromq::socket::SocketRequest> extra,
    allocator_type alloc) noexcept
    : opentxs::Actor<Listener::Actor, ListenerJob>(
          api->Self(),
          LogTrace(),
          [&] {
              return CString{"OTDHT listener ", alloc}.append(routerAdvertise);
          }(),
          0ms,
          batchID,
          alloc,
          {
              {api->Endpoints().Shutdown(), Connect},
          },
          {
              {fromNode, Bind},
          },
          {
              {api->Endpoints().Internal().OTDHTWallet(), Connect},
          },
          {extra})
    , api_p_(std::move(api))
    , shared_p_(std::move(shared))
    , api_(api_p_->Self())
    , data_(shared_p_->data_)
    , external_router_(pipeline_.Internal().ExtraSocket(0_uz))
    , external_pub_(pipeline_.Internal().ExtraSocket(1_uz))
    , router_public_endpoint_(routerAdvertise, alloc)
    , publish_public_endpoint_(publishAdvertise, alloc)
    , routing_id_(routingID, alloc)
    , blockchain_([&] {
        auto out = BlockchainSockets{alloc};
        auto index = 1_uz;

        for (const auto chain : Node::Shared::Chains()) {
            auto& socket = pipeline_.Internal().ExtraSocket(++index);
            auto rc = socket.SetRoutingID(routing_id_);

            assert_true(rc);

            rc = socket.Connect(
                api_.Endpoints().Internal().OTDHTBlockchain(chain).data());

            assert_true(rc);

            out.emplace(chain, socket);
        }

        return out;
    }())
    , active_chains_(alloc)
    , registered_chains_(alloc)
    , queue_(alloc)
    , registration_timer_(api_.Network().Asio().Internal().GetTimer())
{
}

Listener::Actor::Actor(
    std::shared_ptr<const api::internal::Session> api,
    std::shared_ptr<Node::Shared> shared,
    std::string_view routerBind,
    std::string_view routerAdvertise,
    std::string_view publishBind,
    std::string_view publishAdvertise,
    std::string_view routingID,
    std::string_view fromNode,
    zeromq::BatchID batchID,
    allocator_type alloc) noexcept
    : Actor(
          api,
          shared,
          routerBind,
          routerAdvertise,
          publishBind,
          publishAdvertise,
          routingID,
          fromNode,
          batchID,
          [&] {
              const auto& chains = Node::Shared::Chains();
              const auto count = chains.size();
              auto out = Vector<zeromq::socket::SocketRequest>{alloc};
              out.reserve(2_uz + count);
              out.clear();
              out.emplace_back(
                  Type::Router,
                  Internal,
                  zeromq::socket::EndpointRequests{
                      {routerBind, Bind},
                  });
              out.emplace_back(
                  Type::Publish,
                  Internal,
                  zeromq::socket::EndpointRequests{
                      {publishBind, Bind},
                  });

              for (auto i = 0_uz, s = count; i < s; ++i) {
                  out.emplace_back(
                      Type::Dealer,
                      Internal,
                      zeromq::socket::EndpointRequests{});
              }

              return out;
          }(),
          alloc)
{
}
auto Listener::Actor::check_registration() noexcept -> void
{
    const auto unregistered = [&] {
        auto out = Chains{get_allocator()};
        std::ranges::set_difference(
            active_chains_, registered_chains_, std::inserter(out, out.end()));

        return out;
    }();

    for (const auto& chain : unregistered) {
        using DHTJob = opentxs::network::blockchain::DHTJob;
        blockchain_.at(chain).SendDeferred(
            [] {
                auto out = MakeWork(DHTJob::registration);
                out.AddFrame(incoming_peer_);

                return out;
            }(),
            true);
    }

    if (unregistered.empty()) {
        registration_timer_.Cancel();
    } else {
        reset_registration_timer(1s);
    }
}

auto Listener::Actor::do_shutdown() noexcept -> void
{
    registration_timer_.Cancel();
    shared_p_.reset();
    api_p_.reset();
}

auto Listener::Actor::do_startup(allocator_type monotonic) noexcept -> bool
{
    if (api_.Internal().ShuttingDown()) { return true; }

    {
        auto& out = active_chains_;
        auto handle = data_.lock_shared();
        const auto& map = handle->state_;
        std::ranges::transform(
            map, std::inserter(out, out.end()), [](const auto& in) {
                return in.first;
            });
    }

    do_work(monotonic);

    return false;
}

auto Listener::Actor::forward_to_chain(
    opentxs::blockchain::Type chain,
    const Message& msg) noexcept -> void
{
    forward_to_chain(chain, Message{msg});
}

auto Listener::Actor::forward_to_chain(
    opentxs::blockchain::Type chain,
    Message&& msg) noexcept -> void
{
    const auto& log = log_;

    if (false == active_chains_.contains(chain)) {
        log()(name_)(": ")(print(chain))(" is not active").Flush();

        return;
    }

    if (false == registered_chains_.contains(chain)) {
        log()(name_)(": adding message to queue until ")(print(chain))(
            " completes registration")
            .Flush();
        queue_[chain].emplace_back(std::move(msg));
    } else {
        log()(name_)(": forwarding message to ")(print(chain)).Flush();
        blockchain_.at(chain).SendDeferred(std::move(msg), true);
    }
}

auto Listener::Actor::pipeline(
    const Work work,
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    if (const auto id = connection_id(msg); external_router_.ID() == id) {
        pipeline_external(work, std::move(msg));
    } else {
        pipeline_internal(work, std::move(msg));
    }

    do_work(monotonic);
}

auto Listener::Actor::pipeline_external(const Work work, Message&& msg) noexcept
    -> void
{
    switch (work) {
        case Work::sync_request:
        case Work::sync_query:
        case Work::publish_contract:
        case Work::query_contract:
        case Work::pushtx: {
            process_external(std::move(msg));
        } break;
        case Work::shutdown:
        case Work::chain_state:
        case Work::sync_reply:
        case Work::sync_push:
        case Work::response:
        case Work::registration:
        case Work::init:
        case Work::statemachine: {
            LogError()()(name_)(": unhandled message type ")(print(work))
                .Flush();
        } break;
        default: {
            LogError()()(name_)(": unhandled message type ")(
                static_cast<OTZMQWorkType>(work))
                .Flush();
        }
    }
}

auto Listener::Actor::pipeline_internal(const Work work, Message&& msg) noexcept
    -> void
{
    switch (work) {
        case Work::chain_state: {
            process_chain_state(std::move(msg));
        } break;
        case Work::sync_reply: {
            process_sync_reply(std::move(msg));
        } break;
        case Work::sync_push: {
            process_sync_push(std::move(msg));
        } break;
        case Work::response: {
            process_response(std::move(msg));
        } break;
        case Work::registration: {
            process_registration(std::move(msg));
        } break;
        case Work::shutdown:
        case Work::sync_request:
        case Work::sync_query:
        case Work::publish_contract:
        case Work::query_contract:
        case Work::pushtx:
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

auto Listener::Actor::process_chain_state(Message&& msg) noexcept -> void
{
    const auto body = msg.Payload();

    if (2 >= body.size()) { LogAbort()()(name_)(": invalid message").Abort(); }

    const auto chain = body[1].as<opentxs::blockchain::Type>();
    const auto enabled = body[2].as<bool>();

    if (enabled) {
        active_chains_.emplace(chain);
    } else {
        active_chains_.erase(chain);
        registered_chains_.erase(chain);
    }
}

auto Listener::Actor::process_external(Message&& msg) noexcept -> void
{
    try {
        const auto pBase = api_.Factory().BlockchainSyncMessage(msg);

        if (false == pBase.operator bool()) {
            throw std::runtime_error{"failed to instantiate message"};
        }

        const auto& base = *pBase;
        using enum opentxs::network::otdht::MessageType;
        const auto type = base.Type();

        switch (type) {
            case query:
            case sync_request: {
                process_sync(std::move(msg), base);
            } break;
            case publish_contract:
            case contract_query: {
                pipeline_.Internal().SendFromThread(std::move(msg));
            } break;
            case pushtx: {
                process_pushtx(std::move(msg), base);
            } break;
            default: {
                throw std::runtime_error{
                    "Unsupported message type "s.append(print(type))};
            }
        }
    } catch (const std::exception& e) {
        LogError()()(name_)(": ")(e.what()).Flush();
    }
}

auto Listener::Actor::process_pushtx(
    Message&& msg,
    const otdht::Base& base) noexcept -> void
{
    const auto& pushtx = base.asPushTransaction();
    forward_to_chain(pushtx.Chain(), std::move(msg));
}

auto Listener::Actor::process_registration(Message&& msg) noexcept -> void
{
    const auto& log = log_;
    const auto body = msg.Payload();

    if (1 >= body.size()) { LogAbort()()(name_)(": invalid message").Abort(); }

    const auto chain = body[1].as<opentxs::blockchain::Type>();
    log()(name_)(": received registration message from ")(print(chain))(
        " worker")
        .Flush();
    registered_chains_.emplace(chain);

    if (auto i = queue_.find(chain); queue_.end() != i) {
        log()(name_)(": flushing ")(queue_.size())(" queued messages for ")(
            print(chain))(" worker")
            .Flush();
        auto post = ScopeGuard{[&] { queue_.erase(i); }};

        for (auto& message : i->second) {
            forward_to_chain(chain, std::move(message));
        }
    }
}

auto Listener::Actor::process_response(Message&& msg) noexcept -> void
{
    external_router_.SendExternal(std::move(msg));
}

auto Listener::Actor::process_sync(
    Message&& msg,
    const otdht::Base& base) noexcept -> void
{
    external_router_.SendDeferred([&] {
        auto out = zeromq::reply_to_message(msg);
        const auto ack = factory::BlockchainSyncAcknowledgement(
            [&] {
                auto state = otdht::StateData{get_allocator()};
                const auto handle = data_.lock_shared();
                const auto& data = *handle;
                const auto& map = data.state_;
                state.reserve(map.size());
                state.clear();

                for (const auto& [chain, position] : map) {
                    state.emplace_back(chain, position);
                }

                return state;
            }(),
            publish_public_endpoint_);
        const auto rc = ack.Serialize(out);

        assert_true(rc);

        return out;
    }());

    switch (base.Type()) {
        case MessageType::sync_request: {
            const auto& request = base.asRequest();

            for (const auto& state : request.State()) {
                forward_to_chain(state.Chain(), msg);
            }
        } break;
        case MessageType::error:
        case MessageType::sync_ack:
        case MessageType::sync_reply:
        case MessageType::new_block_header:
        case MessageType::publish_contract:
        case MessageType::publish_ack:
        case MessageType::contract_query:
        case MessageType::contract:
        case MessageType::pushtx:
        case MessageType::pushtx_reply: {
            LogAbort()()(name_)(": invalid type ")(print(base.Type())).Abort();
        }
        case MessageType::query:
        default: {
        }
    }
}

auto Listener::Actor::process_sync_push(Message&& msg) noexcept -> void
{

    {
        const auto base = api_.Factory().BlockchainSyncMessage(msg);

        assert_false(nullptr == base);

        const auto& data = base->asData();
        const auto& state = data.State();
        auto handle = data_.lock();
        auto& map = handle->state_;

        if (auto i = map.find(state.Chain()); map.end() != i) {
            i->second = state.Position();
        } else {
            const auto [_, added] =
                map.try_emplace(state.Chain(), state.Position());

            assert_true(added);
        }
    }

    external_pub_.SendExternal(std::move(msg));
}

auto Listener::Actor::process_sync_reply(Message&& msg) noexcept -> void
{
    external_router_.SendExternal(std::move(msg));
}

auto Listener::Actor::reset_registration_timer(
    std::chrono::microseconds interval) noexcept -> void
{
    reset_timer(interval, registration_timer_, Work::statemachine);
}

auto Listener::Actor::work(allocator_type monotonic) noexcept -> bool
{
    check_registration();

    return false;
}

Listener::Actor::~Actor() = default;
}  // namespace opentxs::network::otdht
