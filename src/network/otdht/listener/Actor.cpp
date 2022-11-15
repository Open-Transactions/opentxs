// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "0_stdafx.hpp"                      // IWYU pragma: associated
#include "network/otdht/listener/Actor.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <chrono>
#include <iterator>
#include <memory>
#include <ratio>
#include <stdexcept>
#include <utility>

#include "internal/api/network/Asio.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/network/blockchain/Types.hpp"
#include "internal/network/otdht/Factory.hpp"
#include "internal/network/otdht/Types.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/message/Message.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/network/otdht/Acknowledgement.hpp"
#include "opentxs/network/otdht/Base.hpp"
#include "opentxs/network/otdht/Data.hpp"
#include "opentxs/network/otdht/MessageType.hpp"
#include "opentxs/network/otdht/PushTransaction.hpp"
#include "opentxs/network/otdht/Request.hpp"
#include "opentxs/network/otdht/State.hpp"
#include "opentxs/network/otdht/Types.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/ScopeGuard.hpp"
#include "util/Work.hpp"

namespace opentxs::network::otdht
{
using namespace std::literals;

Listener::Actor::Actor(
    std::shared_ptr<const api::Session> api,
    boost::shared_ptr<Node::Shared> shared,
    std::string_view routerBind,
    std::string_view routerAdvertise,
    std::string_view publishBind,
    std::string_view publishAdvertise,
    std::string_view routingID,
    std::string_view fromNode,
    zeromq::BatchID batchID,
    allocator_type alloc) noexcept
    : opentxs::Actor<Listener::Actor, ListenerJob>(
          *api,
          LogTrace(),
          [&] {
              return CString{"OTDHT listener ", alloc}.append(routerAdvertise);
          }(),
          0ms,
          batchID,
          alloc,
          [&] {
              using Dir = zeromq::socket::Direction;
              auto sub = zeromq::EndpointArgs{alloc};
              sub.emplace_back(
                  CString{api->Endpoints().Shutdown(), alloc}, Dir::Connect);

              return sub;
          }(),
          [&] {
              using Dir = zeromq::socket::Direction;
              auto pull = zeromq::EndpointArgs{alloc};
              pull.emplace_back(CString{fromNode, alloc}, Dir::Bind);

              return pull;
          }(),
          [&] {
              using Dir = zeromq::socket::Direction;
              auto dealer = zeromq::EndpointArgs{alloc};
              dealer.emplace_back(
                  CString{api->Endpoints().Internal().OTDHTWallet(), alloc},
                  Dir::Connect);

              return dealer;
          }(),
          [&] {
              auto out = Vector<zeromq::SocketData>{alloc};
              using Socket = zeromq::socket::Type;
              using Args = zeromq::EndpointArgs;
              using Dir = zeromq::socket::Direction;
              out.emplace_back(std::make_pair<Socket, Args>(
                  Socket::Router,
                  {
                      {CString{routerBind, alloc}, Dir::Bind},
                  }));
              out.emplace_back(std::make_pair<Socket, Args>(
                  Socket::Publish,
                  {
                      {CString{publishBind, alloc}, Dir::Bind},
                  }));
              const auto& chains = Node::Shared::Chains();

              for (auto i = 0_uz, s = chains.size(); i < s; ++i) {
                  out.emplace_back(
                      std::make_pair<Socket, Args>(Socket::Dealer, {}));
              }

              return out;
          }())
    , api_p_(std::move(api))
    , shared_p_(std::move(shared))
    , api_(*api_p_)
    , data_(shared_p_->data_)
    , external_router_([&]() -> auto& {
        auto& socket = pipeline_.Internal().ExtraSocket(0_uz);
        const auto rc = socket.SetExposedUntrusted();

        OT_ASSERT(rc);

        return socket;
    }())
    , external_pub_([&]() -> auto& {
        auto& socket = pipeline_.Internal().ExtraSocket(1_uz);
        const auto rc = socket.SetExposedUntrusted();

        OT_ASSERT(rc);

        return socket;
    }())
    , router_public_endpoint_(routerAdvertise, alloc)
    , publish_public_endpoint_(publishAdvertise, alloc)
    , routing_id_(routingID, alloc)
    , blockchain_([&] {
        auto out = BlockchainSockets{alloc};
        auto index = 1_uz;

        for (const auto chain : Node::Shared::Chains()) {
            auto& socket = pipeline_.Internal().ExtraSocket(++index);
            auto rc = socket.SetRoutingID(routing_id_);

            OT_ASSERT(rc);

            rc = socket.Connect(
                api_.Endpoints().Internal().OTDHTBlockchain(chain).data());

            OT_ASSERT(rc);

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

auto Listener::Actor::check_registration() noexcept -> void
{
    const auto unregistered = [&] {
        auto out = Chains{get_allocator()};
        std::set_difference(
            active_chains_.begin(),
            active_chains_.end(),
            registered_chains_.begin(),
            registered_chains_.end(),
            std::inserter(out, out.end()));

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
            __FILE__,
            __LINE__,
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
        std::transform(
            map.begin(),
            map.end(),
            std::inserter(out, out.end()),
            [](const auto& in) { return in.first; });
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

    // TODO c++20 use contains
    if (0_uz == active_chains_.count(chain)) {
        log(OT_PRETTY_CLASS())(name_)(": ")(print(chain))(" is not active")
            .Flush();

        return;
    }

    // TODO c++20 use contains
    if (0_uz == registered_chains_.count(chain)) {
        log(OT_PRETTY_CLASS())(name_)(": adding message to queue until ")(
            print(chain))(" completes registration")
            .Flush();
        queue_[chain].emplace_back(std::move(msg));
    } else {
        log(OT_PRETTY_CLASS())(name_)(": forwarding message to ")(print(chain))
            .Flush();
        blockchain_.at(chain).SendDeferred(
            std::move(msg), __FILE__, __LINE__, true);
    }
}

auto Listener::Actor::pipeline(
    const Work work,
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    const auto id = msg.Internal().ExtractFront().as<zeromq::SocketID>();

    if (external_router_.ID() == id) {
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
            LogError()(OT_PRETTY_CLASS())(name_)(": unhandled message type ")(
                print(work))
                .Flush();
        } break;
        default: {
            LogError()(OT_PRETTY_CLASS())(name_)(": unhandled message type ")(
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

auto Listener::Actor::process_chain_state(Message&& msg) noexcept -> void
{
    const auto body = msg.Body();

    if (2 >= body.size()) {
        LogAbort()(OT_PRETTY_CLASS())(name_)(": invalid message").Abort();
    }

    const auto chain = body.at(1).as<opentxs::blockchain::Type>();
    const auto enabled = body.at(2).as<bool>();

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
        using Type = opentxs::network::otdht::MessageType;
        const auto type = base.Type();

        switch (type) {
            case Type::query:
            case Type::sync_request: {
                process_sync(std::move(msg), base);
            } break;
            case Type::publish_contract:
            case Type::contract_query: {
                pipeline_.Internal().SendFromThread(std::move(msg));
            } break;
            case Type::pushtx: {
                process_pushtx(std::move(msg), base);
            } break;
            default: {
                throw std::runtime_error{
                    UnallocatedCString{"Unsupported message type "}.append(
                        print(type))};
            }
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
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
    const auto body = msg.Body();

    if (1 >= body.size()) {
        LogAbort()(OT_PRETTY_CLASS())(name_)(": invalid message").Abort();
    }

    const auto chain = body.at(1).as<opentxs::blockchain::Type>();
    log(OT_PRETTY_CLASS())(name_)(": received registration message from ")(
        print(chain))(" worker")
        .Flush();
    registered_chains_.emplace(chain);

    if (auto i = queue_.find(chain); queue_.end() != i) {
        log(OT_PRETTY_CLASS())(name_)(": flushing ")(queue_.size())(
            " queued messages for ")(print(chain))(" worker")
            .Flush();
        auto post = ScopeGuard{[&] { queue_.erase(i); }};

        for (auto& msg : i->second) { forward_to_chain(chain, std::move(msg)); }
    }
}

auto Listener::Actor::process_response(Message&& msg) noexcept -> void
{
    external_router_.SendExternal(std::move(msg), __FILE__, __LINE__);
}

auto Listener::Actor::process_sync(
    Message&& msg,
    const otdht::Base& base) noexcept -> void
{
    external_router_.SendDeferred(
        [&] {
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

            OT_ASSERT(rc);

            return out;
        }(),
        __FILE__,
        __LINE__);

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
            LogAbort()(OT_PRETTY_CLASS())(name_)(": invalid type ")(
                print(base.Type()))
                .Abort();
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

        OT_ASSERT(base);

        const auto& data = base->asData();
        const auto& state = data.State();
        auto handle = data_.lock();
        auto& map = handle->state_;

        if (auto i = map.find(state.Chain()); map.end() != i) {
            i->second = state.Position();
        } else {
            const auto [_, added] =
                map.try_emplace(state.Chain(), state.Position());

            OT_ASSERT(added);
        }
    }

    external_pub_.SendExternal(std::move(msg), __FILE__, __LINE__);
}

auto Listener::Actor::process_sync_reply(Message&& msg) noexcept -> void
{
    external_router_.SendExternal(std::move(msg), __FILE__, __LINE__);
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
