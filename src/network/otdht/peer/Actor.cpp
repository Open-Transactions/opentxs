// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type
// IWYU pragma: no_forward_declare opentxs::network::otdht::MessageType

#include "network/otdht/peer/Actor.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <chrono>
#include <compare>
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
#include "opentxs/network/otdht/Acknowledgement.hpp"
#include "opentxs/network/otdht/Base.hpp"
#include "opentxs/network/otdht/Data.hpp"
#include "opentxs/network/otdht/MessageType.hpp"  // IWYU pragma: keep
#include "opentxs/network/otdht/PushTransactionReply.hpp"
#include "opentxs/network/otdht/Query.hpp"
#include "opentxs/network/otdht/State.hpp"
#include "opentxs/network/otdht/Types.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameIterator.hpp"
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

Peer::Actor::Actor(
    std::shared_ptr<const api::Session> api,
    boost::shared_ptr<Node::Shared> shared,
    std::string_view routingID,
    std::string_view toRemote,
    std::string_view fromNode,
    zeromq::BatchID batchID,
    allocator_type alloc) noexcept
    : opentxs::Actor<Peer::Actor, PeerJob>(
          *api,
          LogTrace(),
          [&] {
              return CString{"OTDHT peer ", alloc}.append(toRemote);
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
                  Socket::Dealer,
                  {
                      {CString{toRemote, alloc}, Dir::Connect},
                  }));
              out.emplace_back(
                  std::make_pair<Socket, Args>(Socket::Subscribe, {}));
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
    , external_dealer_([&]() -> auto& {
        auto& socket = pipeline_.Internal().ExtraSocket(0_uz);
        const auto rc = socket.SetExposedUntrusted();

        OT_ASSERT(rc);

        return socket;
    }())
    , external_sub_([&]() -> auto& {
        auto& socket = pipeline_.Internal().ExtraSocket(1_uz);
        auto rc = socket.ClearSubscriptions();

        OT_ASSERT(rc);

        rc = socket.SetExposedUntrusted();

        OT_ASSERT(rc);

        return socket;
    }())
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
    , subscriptions_(alloc)
    , active_chains_(alloc)
    , registered_chains_(alloc)
    , queue_(alloc)
    , last_activity_()
    , last_ack_(std::nullopt)
    , ping_timer_(api_.Network().Asio().Internal().GetTimer())
    , registration_timer_(api_.Network().Asio().Internal().GetTimer())
{
}

auto Peer::Actor::check_ping() noexcept -> void
{
    static constexpr auto interval = 2min;
    const auto elapsed = sClock::now() - last_activity_;

    if (elapsed > interval) {
        ping();
        reset_ping_timer(interval);
    } else {
        reset_ping_timer(std::chrono::duration_cast<std::chrono::microseconds>(
            interval - elapsed));
    }
}

auto Peer::Actor::check_registration() noexcept -> void
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
                out.AddFrame(outgoing_peer_);

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

auto Peer::Actor::do_shutdown() noexcept -> void
{
    registration_timer_.Cancel();
    ping_timer_.Cancel();
    shared_p_.reset();
    api_p_.reset();
}

auto Peer::Actor::do_startup(allocator_type monotonic) noexcept -> bool
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

auto Peer::Actor::forward_to_chain(
    opentxs::blockchain::Type chain,
    const Message& msg) noexcept -> void
{
    forward_to_chain(chain, Message{msg});
}

auto Peer::Actor::forward_to_chain(
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

auto Peer::Actor::forward_to_subscribers(
    const Acknowledgement& ack,
    const Message& msg) noexcept -> void
{
    for (const auto& state : ack.State()) {
        const auto chain = state.Chain();
        forward_to_chain(chain, msg);
    }
}

auto Peer::Actor::ping() noexcept -> void
{
    const auto& log = log_;
    log(OT_PRETTY_CLASS())(name_)(": requesting status").Flush();
    external_dealer_.SendExternal(
        [&] {
            auto msg = zeromq::Message{};
            msg.StartBody();
            const auto query = factory::BlockchainSyncQuery(0);

            if (false == query.Serialize(msg)) { OT_FAIL; }

            return msg;
        }(),
        __FILE__,
        __LINE__);
}

auto Peer::Actor::pipeline(
    const Work work,
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    const auto id = msg.Internal().ExtractFront().as<zeromq::SocketID>();

    if ((external_dealer_.ID() == id) || (external_sub_.ID() == id)) {
        pipeline_external(work, std::move(msg));
    } else {
        pipeline_internal(work, std::move(msg));
    }

    do_work(monotonic);
}

auto Peer::Actor::pipeline_external(const Work work, Message&& msg) noexcept
    -> void
{
    last_activity_ = sClock::now();

    switch (work) {
        case Work::sync_ack:
        case Work::sync_reply:
        case Work::sync_push: {
            process_sync(std::move(msg));
        } break;
        case Work::response: {
            process_response(std::move(msg));
        } break;
        case Work::sync_request:
        case Work::shutdown:
        case Work::chain_state:
        case Work::push_tx:
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

auto Peer::Actor::pipeline_internal(const Work work, Message&& msg) noexcept
    -> void
{
    switch (work) {
        case Work::chain_state: {
            process_chain_state(std::move(msg));
        } break;
        case Work::sync_request: {
            process_sync_request_internal(std::move(msg));
        } break;
        case Work::push_tx: {
            process_pushtx_internal(std::move(msg));
        } break;
        case Work::registration: {
            process_registration(std::move(msg));
        } break;
        case Work::shutdown:
        case Work::sync_ack:
        case Work::sync_reply:
        case Work::sync_push:
        case Work::response:
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

auto Peer::Actor::process_chain_state(Message&& msg) noexcept -> void
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

auto Peer::Actor::process_pushtx_internal(Message&& msg) noexcept -> void
{
    log_(OT_PRETTY_CLASS())(name_)(": forwarding pushtx to remote peer")
        .Flush();
    external_dealer_.SendExternal(
        strip_header(std::move(msg)), __FILE__, __LINE__);
}

auto Peer::Actor::process_registration(Message&& msg) noexcept -> void
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

        for (auto& message : i->second) {
            forward_to_chain(chain, std::move(message));
        }
    } else if (last_ack_.has_value()) {
        const auto& last = *last_ack_;
        const auto sync = api_.Factory().BlockchainSyncMessage(last);
        const auto& ack = sync->asAcknowledgement();

        for (const auto& state : ack.State()) {
            if (state.Chain() == chain) {
                log(OT_PRETTY_CLASS())(name_)(
                    ": sending last acknowledgement message to ")(print(chain))(
                    " worker")
                    .Flush();
                forward_to_chain(chain, last);
                break;
            }
        }
    }
}

auto Peer::Actor::process_response(Message&& msg) noexcept -> void
{
    try {
        const auto base = api_.Factory().BlockchainSyncMessage(msg);

        if (!base) {
            throw std::runtime_error{"failed to instantiate response"};
        }

        using Type = opentxs::network::otdht::MessageType;
        const auto type = base->Type();

        switch (type) {
            case Type::publish_ack:
            case Type::contract: {
                pipeline_.Internal().SendFromThread(std::move(msg));
            } break;
            case Type::pushtx_reply: {
                const auto& reply = base->asPushTransactionReply();
                forward_to_chain(reply.Chain(), std::move(msg));
            } break;
            case Type::error:
            case Type::sync_request:
            case Type::sync_ack:
            case Type::sync_reply:
            case Type::new_block_header:
            case Type::query:
            case Type::publish_contract:
            case Type::contract_query:
            case Type::pushtx:
            default: {
                const auto error =
                    CString{"Unsupported response type "}.append(print(type));

                throw std::runtime_error{error.c_str()};
            }
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(name_)(": ")(e.what()).Flush();
    }
}

auto Peer::Actor::process_sync(Message&& msg) noexcept -> void
{
    const auto& log = log_;

    try {
        const auto sync = api_.Factory().BlockchainSyncMessage(msg);
        const auto type = sync->Type();
        log(OT_PRETTY_CLASS())(name_)(": received ")(print(type)).Flush();
        using Type = opentxs::network::otdht::MessageType;

        switch (type) {
            case Type::sync_ack: {
                const auto& ack = sync->asAcknowledgement();
                subscribe(ack);
                forward_to_subscribers(ack, msg);
                last_ack_ = std::move(msg);
            } break;
            case Type::sync_reply:
            case Type::new_block_header: {
                const auto& data = sync->asData();
                const auto chain = data.State().Chain();
                forward_to_chain(chain, std::move(msg));
            } break;
            case Type::error:
            case Type::sync_request:
            case Type::query:
            case Type::publish_contract:
            case Type::publish_ack:
            case Type::contract_query:
            case Type::contract:
            case Type::pushtx:
            case Type::pushtx_reply: {
                const auto error =
                    CString{}
                        .append("unsupported message type on external socket: ")
                        .append(print(type));

                throw std::runtime_error{error.c_str()};
            }
            default: {
                const auto error =
                    CString{}
                        .append("unknown message type: ")
                        .append(std::to_string(static_cast<TypeEnum>(type)));

                throw std::runtime_error{error.c_str()};
            }
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(name_)(": ")(e.what()).Flush();
    }
}

auto Peer::Actor::process_sync_request_internal(Message&& msg) noexcept -> void
{
    log_(OT_PRETTY_CLASS())(name_)(": forwarding sync request to remote peer")
        .Flush();
    external_dealer_.SendExternal(
        strip_header(std::move(msg)), __FILE__, __LINE__);
}

auto Peer::Actor::reset_ping_timer(std::chrono::microseconds interval) noexcept
    -> void
{
    reset_timer(interval, ping_timer_, Work::statemachine);
}

auto Peer::Actor::reset_registration_timer(
    std::chrono::microseconds interval) noexcept -> void
{
    reset_timer(interval, registration_timer_, Work::statemachine);
}

auto Peer::Actor::strip_header(Message&& in) noexcept -> Message
{
    auto out = Message{};
    out.StartBody();

    for (auto& frame : in.Body()) { out.AddFrame(std::move(frame)); }

    return out;
}

auto Peer::Actor::subscribe(const Acknowledgement& ack) noexcept -> void
{
    const auto& log = log_;
    const auto endpoint = ack.Endpoint();

    if (endpoint.empty()) { return; }

    // TODO c++20 use contains
    if (0_uz < subscriptions_.count(endpoint)) { return; }

    if (external_sub_.Connect(endpoint.data())) {
        log(OT_PRETTY_CLASS())(name_)(": subscribed to endpoint ")(
            endpoint)(" for new block notifications")
            .Flush();
    } else {
        LogError()(OT_PRETTY_CLASS())(
            name_)(": failed to subscribe to endpoint ")(endpoint)
            .Flush();
    }

    subscriptions_.emplace(endpoint);
}

auto Peer::Actor::work(allocator_type monotonic) noexcept -> bool
{
    check_ping();
    check_registration();

    return false;
}

Peer::Actor::~Actor() = default;
}  // namespace opentxs::network::otdht
