// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "network/otdht/node/Actor.hpp"  // IWYU pragma: associated

#include <BlockchainPeerAddress.pb.h>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <algorithm>
#include <chrono>
#include <iterator>
#include <memory>
#include <span>
#include <stdexcept>
#include <utility>

#include "BoostAsio.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/blockchain/node/Types.hpp"
#include "internal/network/blockchain/Address.hpp"
#include "internal/network/blockchain/Types.hpp"
#include "internal/network/otdht/Listener.hpp"
#include "internal/network/otdht/Peer.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Options.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/BlockchainHandle.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/network/OTDHT.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/network/blockchain/Protocol.hpp"   // IWYU pragma: keep
#include "opentxs/network/blockchain/Transport.hpp"  // IWYU pragma: keep
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/network/zeromq/message/Envelope.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Options.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WorkType.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/ScopeGuard.hpp"
#include "util/Work.hpp"

namespace opentxs::network::otdht
{
Node::Actor::Actor(
    std::shared_ptr<const api::Session> api,
    boost::shared_ptr<Shared> shared,
    zeromq::BatchID batchID,
    allocator_type alloc) noexcept
    : opentxs::Actor<Node::Actor, NodeJob>(
          *api,
          LogTrace(),
          {"OTDHT node", alloc},
          0ms,
          batchID,
          alloc,
          [&] {
              using enum zeromq::socket::Direction;
              auto sub = zeromq::EndpointArgs{alloc};
              sub.emplace_back(
                  CString{api->Endpoints().Shutdown(), alloc}, Connect);
              sub.emplace_back(
                  CString{
                      api->Endpoints().BlockchainSyncServerUpdated(), alloc},
                  Connect);
              sub.emplace_back(
                  CString{api->Endpoints().BlockchainNewFilter(), alloc},
                  Connect);
              sub.emplace_back(
                  CString{api->Endpoints().BlockchainStateChange(), alloc},
                  Connect);

              return sub;
          }(),
          [&] {
              using enum zeromq::socket::Direction;
              auto pull = zeromq::EndpointArgs{alloc};
              pull.emplace_back(
                  CString{api->Endpoints().Internal().OTDHTNodePull(), alloc},
                  Bind);

              return pull;
          }(),
          {},
          [&] {
              auto out = Vector<network::zeromq::SocketData>{alloc};
              using enum network::zeromq::socket::Direction;
              using enum network::zeromq::socket::Type;
              using Args = network::zeromq::EndpointArgs;
              using Socket = network::zeromq::socket::Type;
              out.emplace_back(std::make_tuple<Socket, Args, bool>(
                  Publish,
                  {
                      {CString{
                           api->Endpoints().Internal().OTDHTNodePublish(),
                           alloc},
                       Bind},
                  },
                  false));  // NOTE publish_
              out.emplace_back(std::make_tuple<Socket, Args, bool>(
                  Router,
                  {
                      {CString{
                           api->Endpoints().Internal().OTDHTNodeRouter(),
                           alloc},
                       Bind},
                  },
                  false));  // NOTE router_
              out.emplace_back(std::make_tuple<Socket, Args, bool>(
                  Router, {}, true));  // NOTE external_

              return out;
          }())
    , api_p_(std::move(api))
    , shared_p_(std::move(shared))
    , shared_(*shared_p_)
    , api_(*api_p_)
    , data_(shared_.data_)
    , publish_(pipeline_.Internal().ExtraSocket(0))
    , router_(pipeline_.Internal().ExtraSocket(1))
    , external_([&]() -> auto& {
        auto& out = pipeline_.Internal().ExtraSocket(2);
        const auto rc = out.EnableCurveServer(shared_.private_key_.Bytes());

        OT_ASSERT(rc);

        return out;
    }())
    , peers_(alloc)
    , listeners_(alloc)
    , listen_endpoints_(alloc)
    , external_endpoints_(alloc)
    , blockchain_index_(alloc)
    , blockchain_reverse_index_(alloc)
    , outgoing_blockchain_index_(alloc)
    , peer_manager_index_(alloc)
    , next_cookie_(0u)
{
}

auto Node::Actor::do_shutdown() noexcept -> void
{
    shared_p_.reset();
    api_p_.reset();
}

auto Node::Actor::do_startup(allocator_type monotonic) noexcept -> bool
{
    if (api_.Internal().ShuttingDown()) { return true; }

    load_positions();
    load_peers();
    listen(monotonic);

    return false;
}

auto Node::Actor::forward(
    const zeromq::Envelope& recipient,
    std::span<zeromq::Frame> payload,
    zeromq::socket::Raw& socket) const noexcept -> void
{
    OT_ASSERT(recipient.IsValid());

    socket.SendDeferred(
        [&] {
            auto out = zeromq::reply_to_message(recipient, true);
            out.MoveFrames(payload);

            return out;
        }(),
        __FILE__,
        __LINE__);
}

auto Node::Actor::get_peers() const noexcept -> Set<CString>
{
    auto out = Set<CString>{get_allocator()};
    std::transform(
        peers_.begin(),
        peers_.end(),
        std::inserter(out, out.end()),
        [](const auto& data) { return std::get<0>(data.second); });
    std::transform(
        listeners_.begin(),
        listeners_.end(),
        std::inserter(out, out.end()),
        [](const auto& data) { return std::get<0>(data.second); });

    return out;
}

auto Node::Actor::get_peers(Message& out) const noexcept -> void
{
    for (const auto& endpoint : get_peers()) { out.AddFrame(endpoint); }
}

auto Node::Actor::listen(allocator_type monotonic) noexcept -> void
{
    const auto listeners = api_.GetOptions().Internal().OTDHTListeners();
    auto endpoints = Set<ParsedListener>{monotonic};
    endpoints.clear();

    for (const auto& params : listeners) {
        if (auto val = parse(params); val.has_value()) {
            endpoints.emplace(*val);
        }
    }

    for (const auto& [endpoint, external] : endpoints) {
        if (external_.Bind(endpoint.c_str())) {
            LogConsole()("OTDHT listening on ")(endpoint).Flush();
            const auto& [transport, bytes] = external;
            external_endpoints_.emplace_back(
                api_.Factory().BlockchainAddressZMQ(
                    blockchain::Protocol::opentxs,
                    transport,
                    bytes.Bytes(),
                    opentxs::blockchain::Type::Unknown,
                    {},
                    {},
                    shared_.public_key_.Bytes()));
        } else {
            LogConsole()("OTDHT unable to bind to ")(endpoint).Flush();
        }
    }
}

auto Node::Actor::load_peers() noexcept -> void
{
    const auto peers = api_.Network().OTDHT().KnownPeers(get_allocator());
    log_(OT_PRETTY_CLASS())(name_)(": loading ")(peers.size())(" peers")
        .Flush();

    for (const auto& peer : peers) { process_peer(peer); }
}

auto Node::Actor::load_positions() noexcept -> void
{
    for (const auto& chain : Shared::Chains()) {
        try {
            const auto handle = api_.Network().Blockchain().GetChain(chain);

            if (false == handle.IsValid()) { continue; }

            const auto& filter = handle.get().FilterOracle();
            process_cfilter(chain, filter.FilterTip(filter.DefaultType()));
        } catch (...) {
        }
    }
}

auto Node::Actor::parse(const opentxs::internal::Options::Listener& val)
    const noexcept -> std::optional<ParsedListener>
{
    try {
        auto alloc = get_allocator();
        using namespace boost::asio::ip;
        using enum blockchain::Transport;

        switch (val.external_type_) {
            case zmq: {
                if (false == api_.GetOptions().TestMode()) {

                    throw std::runtime_error{
                        "inproc listeners are only allowed in test mode"};
                }

                if (val.local_type_ != zmq) {
                    const auto error =
                        CString{
                            "incorrect local type for inproc listener: ", alloc}
                            .append(print(val.local_type_));

                    throw std::runtime_error{error.c_str()};
                }

                if (val.local_address_ != val.external_address_) {

                    throw std::runtime_error{
                        "local address must match external address for inproc "
                        "listeners"};
                }

                return ParsedListener{
                    CString{"inproc://", alloc}
                        .append(val.local_address_.Bytes())
                        .append(":")
                        .append(std::to_string(blockchain::otdht_listen_port_)),
                    std::make_pair(val.external_type_, val.external_address_)};
            }
            case ipv4: {
                if (val.local_type_ != ipv4) {
                    const auto error =
                        CString{
                            "incorrect local type for ipv4 listener: ", alloc}
                            .append(print(val.local_type_));

                    throw std::runtime_error{error.c_str()};
                }

                auto external = make_address(val.external_address_.Bytes());
                const auto local = make_address(val.local_address_.Bytes());

                if (false == external.is_v4()) {

                    throw std::runtime_error{
                        "external address is not a valid ipv4 address"};
                }

                if (false == local.is_v4()) {

                    throw std::runtime_error{
                        "local address is not a valid ipv4 address"};
                }

                return ParsedListener{
                    CString{"tcp://", alloc}
                        .append(val.local_address_.Bytes())
                        .append(":")
                        .append(std::to_string(blockchain::otdht_listen_port_)),
                    std::make_pair(val.external_type_, val.external_address_)};
            }
            case ipv6: {
                if (val.local_type_ != ipv6) {
                    const auto error =
                        CString{
                            "incorrect local type for ipv6 listener: ", alloc}
                            .append(print(val.local_type_));

                    throw std::runtime_error{error.c_str()};
                }

                auto external = make_address(val.external_address_.Bytes());
                const auto local = make_address(val.local_address_.Bytes());

                if (false == external.is_v6()) {

                    throw std::runtime_error{
                        "external address is not a valid ipv6 address"};
                }

                if (false == local.is_v6()) {

                    throw std::runtime_error{
                        "local address is not a valid ipv6 address"};
                }

                return ParsedListener{
                    CString{"tcp://[", alloc}
                        .append(val.local_address_.Bytes())
                        .append("]:")
                        .append(std::to_string(blockchain::otdht_listen_port_)),
                    std::make_pair(val.external_type_, val.external_address_)};
            }
            default: {
                LogError()(OT_PRETTY_CLASS())(
                    name_)(": unsupported listener type: ")(
                    print(val.external_type_))
                    .Flush();

                return std::nullopt;
            }
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(name_)(": ")(e.what()).Flush();

        return std::nullopt;
    }
}

auto Node::Actor::pipeline(
    const Work work,
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    const auto id = connection_id(msg);

    if (router_.ID() == id) {
        pipeline_router(work, std::move(msg), monotonic);
    } else if (external_.ID() == id) {
        pipeline_external(work, std::move(msg), monotonic);
    } else {
        pipeline_internal(work, std::move(msg));
    }
}

auto Node::Actor::pipeline_external(
    const Work work,
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    switch (work) {
        case Work::blockchain: {
            process_blockchain_external(std::move(msg), monotonic);
        } break;
        case Work::shutdown:
        case Work::chain_state:
        case Work::new_cfilter:
        case Work::new_peer:
        case Work::add_listener:
        case Work::connect_peer_manager:
        case Work::disconnect_peer_manager:
        case Work::connect_peer:
        case Work::disconnect_peer:
        case Work::registration:
        case Work::init:
        case Work::statemachine: {
            log_(OT_PRETTY_CLASS())(name_)(
                ": received unhandled message type ")(print(work))(
                " on external socket")
                .Flush();
        } break;
        default: {
            log_(OT_PRETTY_CLASS())(name_)(
                ": received unhandled message type ")(
                static_cast<OTZMQWorkType>(work))(" on external socket")
                .Flush();
        }
    }
}

auto Node::Actor::pipeline_internal(const Work work, Message&& msg) noexcept
    -> void
{
    switch (work) {
        case Work::chain_state: {
            process_chain_state(std::move(msg));
        } break;
        case Work::new_cfilter: {
            process_new_cfilter(std::move(msg));
        } break;
        case Work::new_peer: {
            process_new_peer(std::move(msg));
        } break;
        case Work::add_listener: {
            process_add_listener(std::move(msg));
        } break;
        case Work::shutdown:
        case Work::blockchain:
        case Work::connect_peer_manager:
        case Work::disconnect_peer_manager:
        case Work::connect_peer:
        case Work::disconnect_peer:
        case Work::registration:
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

auto Node::Actor::pipeline_router(
    const Work work,
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    switch (work) {
        case Work::blockchain: {
            process_blockchain_internal(std::move(msg), monotonic);
        } break;
        case Work::connect_peer_manager: {
            process_connect_peer_manager(std::move(msg));
        } break;
        case Work::disconnect_peer_manager: {
            process_disconnect_peer_manager(std::move(msg));
        } break;
        case Work::connect_peer: {
            process_connect_peer(std::move(msg), monotonic);
        } break;
        case Work::disconnect_peer: {
            process_disconnect_peer(std::move(msg));
        } break;
        case Work::registration: {
            process_registration(std::move(msg));
        } break;
        case Work::shutdown:
        case Work::chain_state:
        case Work::new_cfilter:
        case Work::new_peer:
        case Work::add_listener:
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

auto Node::Actor::process_add_listener(Message&& msg) noexcept -> void
{
    const auto body = msg.Payload();

    if (4 >= body.size()) {
        LogAbort()(OT_PRETTY_CLASS())(name_)(": invalid message").Abort();
    }

    const auto routerBind = body[1].Bytes();
    const auto routerAdvertise = body[2].Bytes();
    const auto publishBind = body[3].Bytes();
    const auto publishAdvertise = body[4].Bytes();

    // TODO c++20 use contains
    if (0_uz < listen_endpoints_.count(routerBind)) { return; }
    if (0_uz < listen_endpoints_.count(routerAdvertise)) { return; }
    if (0_uz < listen_endpoints_.count(publishBind)) { return; }
    if (0_uz < listen_endpoints_.count(publishAdvertise)) { return; }

    listen_endpoints_.emplace(routerBind);
    listen_endpoints_.emplace(routerAdvertise);
    listen_endpoints_.emplace(publishBind);
    listen_endpoints_.emplace(publishAdvertise);

    auto alloc = get_allocator();
    using Socket = zeromq::socket::Type;
    auto [it, added] = peers_.try_emplace(
        CString{routerAdvertise, alloc},
        Listener::NextID(alloc),
        zeromq::MakeArbitraryInproc(alloc),
        api_.Network().ZeroMQ().Internal().RawSocket(Socket::Push));

    if (added) {
        auto& [routingID, pushEndpoint, socket] = it->second;
        const auto rc = socket.Connect(pushEndpoint.data());

        OT_ASSERT(rc);

        Listener{
            api_p_,
            shared_p_,
            routerBind,
            routerAdvertise,
            publishBind,
            publishAdvertise,
            routingID,
            pushEndpoint}
            .Init();
    }

    publish_peers();
}

auto Node::Actor::process_blockchain_external(
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    const auto& log = log_;
    auto payload = msg.Payload();
    const auto chain = blockchain::decode(msg);
    const auto type = CString{payload[3].Bytes(), monotonic};
    auto remoteID = std::move(msg).Envelope();
    auto& map = blockchain_index_;

    if (auto i = map.find(remoteID); map.end() != i) {
        auto& [cookie, localID, queue] = i->second;

        if (localID.IsValid()) {
            log(OT_PRETTY_CLASS())(name_)(": forwarding incoming ")(
                print(chain))(" ")(type.c_str())(" message to peer ")(cookie)
                .Flush();
            forward(localID, payload, router_);
        } else {
            log(OT_PRETTY_CLASS())(name_)(": queueing incoming ")(print(chain))(
                " ")(type.c_str())(" message for peer ")(cookie)
                .Flush();
            auto& queued = queue.emplace_back();
            queued.StartBody();
            queued.MoveFrames(payload);
        }
    } else {
        auto& index = peer_manager_index_;

        if (auto j = index.find(chain); index.end() != j) {
            // NOTE the appropriate peer manager is available so we can tell it
            // to span a peer to handle this connection
            auto& [peerManager, connections] = j->second;
            connections.emplace(remoteID);
            auto& [cookie, localID, queue] = [&]() -> auto&
            {
                auto alloc = get_allocator();
                auto [k, _] = map.try_emplace(
                    remoteID, next_cookie_++, LocalID{alloc}, Queue{alloc});

                return k->second;
            }
            ();
            blockchain_reverse_index_.try_emplace(cookie, std::move(remoteID));
            log(OT_PRETTY_CLASS())(name_)(": notifying ")(print(chain))(
                " peer manager to spawn peer ")(cookie)
                .Flush();
            log(OT_PRETTY_CLASS())(name_)(": queueing incoming ")(print(chain))(
                " ")(type.c_str())(" message for peer ")(cookie)
                .Flush();
            {
                queue.clear();
                auto& queued = queue.emplace_back();
                queued.StartBody();
                queued.MoveFrames(payload);
            }
            // TODO c++20 capture structured binding
            router_.SendDeferred(
                [&](const auto& p, const auto& c) {
                    using enum opentxs::blockchain::node::PeerManagerJobs;
                    auto out =
                        zeromq::tagged_reply_to_message(p, spawn_peer, true);
                    out.AddFrame(c);

                    return out;
                }(peerManager, cookie),
                __FILE__,
                __LINE__);
        } else {
            // NOTE there is no peer manager for this chain so drop the message
        }
    }
}

auto Node::Actor::process_blockchain_internal(
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    const auto& log = log_;
    auto payload = msg.Payload();
    const auto chain = blockchain::decode(msg);
    const auto type = CString{payload[3].Bytes(), monotonic};
    const auto localID = std::move(msg).Envelope();
    const auto& map = outgoing_blockchain_index_;

    OT_ASSERT(localID.IsValid());

    if (auto i = map.find(localID); map.end() != i) {
        const auto& [remoteID, cookie] = i->second;
        log(OT_PRETTY_CLASS())(name_)(": forwarding outgoing ")(print(chain))(
            " ")(type.c_str())(" message from peer ")(cookie)
            .Flush();
        forward(remoteID, payload, external_);
    } else {
        log(OT_PRETTY_CLASS())(name_)(
            ": can not determine outgoing connection for ")(print(chain))(" ")(
            type.c_str())(" message")
            .Flush();
    }
}

auto Node::Actor::process_cfilter(
    opentxs::blockchain::Type chain,
    opentxs::blockchain::block::Position&& tip) noexcept -> void
{
    auto handle = data_.lock();
    auto& map = handle->state_;

    if (auto it = map.find(chain); map.end() != it) {
        it->second = std::move(tip);
    } else {
        map.try_emplace(chain, std::move(tip));
    }

    send_to_peers([&] {
        auto work = MakeWork(WorkType::BlockchainStateChange);
        work.AddFrame(chain);
        work.AddFrame(true);

        return work;
    }());
}

auto Node::Actor::process_chain_state(Message&& msg) noexcept -> void
{
    const auto body = msg.Payload();

    if (2 >= body.size()) {
        LogAbort()(OT_PRETTY_CLASS())(name_)(": invalid message").Abort();
    }

    const auto chain = body[1].as<opentxs::blockchain::Type>();
    auto enabled = body[2].as<bool>();
    auto tip = [&]() -> opentxs::blockchain::block::Position {
        try {
            const auto& network = api_.Network().Blockchain().GetChain(chain);
            const auto& filter = network.get().FilterOracle();

            return filter.FilterTip(filter.DefaultType());
        } catch (...) {
            enabled = false;

            return {};
        }
    }();

    auto handle = data_.lock();
    auto& map = handle->state_;

    if (enabled) {
        if (auto it = map.find(chain); map.end() != it) {
            it->second = std::move(tip);
        } else {
            const auto [i, rc] = map.try_emplace(chain, std::move(tip));

            OT_ASSERT(rc);
        }
    } else {
        map.erase(chain);
    }

    send_to_peers(std::move(msg));
}

auto Node::Actor::process_connect_peer(
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    const auto& log = log_;
    const auto payload = msg.Payload();
    auto envelope = std::move(msg).Envelope();

    OT_ASSERT(2_uz < payload.size());
    OT_ASSERT(envelope.IsValid());

    const auto chain = payload[1].as<opentxs::blockchain::Type>();
    const auto cookie = payload[2].as<Cookie>();
    auto& map = blockchain_index_;
    auto& reverse = blockchain_reverse_index_;
    auto& outgoing = outgoing_blockchain_index_;

    if (auto i = reverse.find(cookie); reverse.end() != i) {
        const auto& remoteID = i->second;

        if (auto j = map.find(remoteID); map.end() != j) {
            auto& [_, localID, queue] = j->second;
            localID = std::move(envelope);
            outgoing[localID] = std::make_pair(remoteID, cookie);
            router_.SendDeferred(
                zeromq::tagged_reply_to_message(
                    localID, WorkType::AsioRegister, true),
                __FILE__,
                __LINE__);
            log(OT_PRETTY_CLASS())(name_)(": ")(print(chain))(" peer ")(
                cookie)(" registered")
                .Flush();

            while (false == queue.empty()) {
                auto& m = queue.front();
                auto original = m.Payload();
                const auto type = CString{original[3].Bytes(), monotonic};
                log(OT_PRETTY_CLASS())(name_)(": delivering queued ")(print(
                    chain))(" ")(type.c_str())(" message to peer ")(cookie)
                    .Flush();
                forward(localID, original, router_);
                queue.pop_front();
            }
        }
    }
}

auto Node::Actor::process_connect_peer_manager(Message&& msg) noexcept -> void
{
    const auto& log = log_;
    const auto payload = msg.Payload();
    auto envelope = std::move(msg).Envelope();

    OT_ASSERT(1_uz < payload.size());
    OT_ASSERT(envelope.IsValid());

    const auto chain = payload[1].as<opentxs::blockchain::Type>();
    auto& map = peer_manager_index_;
    auto alloc = get_allocator();

    if (auto i = map.find(chain); map.end() == i) {
        map.try_emplace(
            chain, PeerManagerID{envelope, alloc}, Connections{alloc});
    } else {
        i->second.first = PeerManagerID{envelope, alloc};
    }

    using enum opentxs::blockchain::node::PeerManagerJobs;
    router_.SendDeferred(
        [&] {
            auto out = zeromq::tagged_reply_to_message(
                std::move(envelope), register_ack, true);

            for (const auto& addr : external_endpoints_) {
                auto proto = proto::BlockchainPeerAddress{};

                if (addr.Internal().Serialize(proto)) {
                    proto::write(proto, out.AppendBytes());
                } else {
                    LogError()(OT_PRETTY_CLASS())(
                        name_)(": failed to serialize address")
                        .Flush();
                }
            }

            return out;
        }(),
        __FILE__,
        __LINE__);
    log(OT_PRETTY_CLASS())(name_)(": ")(print(chain))(
        " peer manager registered")
        .Flush();
}

auto Node::Actor::process_disconnect_peer(Message&& msg) noexcept -> void
{
    const auto& log = log_;
    const auto payload = msg.Payload();
    auto localID = std::move(msg).Envelope();

    OT_ASSERT(2_uz < payload.size());
    OT_ASSERT(localID.IsValid());

    const auto chain = payload[1].as<opentxs::blockchain::Type>();
    const auto cookie = payload[2].as<Cookie>();
    auto& peer = peer_manager_index_;
    auto& reverse = blockchain_reverse_index_;
    outgoing_blockchain_index_.erase(localID);

    if (const auto i = reverse.find(cookie); reverse.end() != i) {
        const auto& remoteID = i->second;

        if (auto j = peer.find(chain); peer.end() != j) {
            auto& [_, connections] = j->second;
            connections.erase(remoteID);
        }

        blockchain_index_.erase(remoteID);
        log(OT_PRETTY_CLASS())(name_)(": ")(print(chain))(" peer ")(
            cookie)(" unregistered")
            .Flush();
    }
}

auto Node::Actor::process_disconnect_peer_manager(Message&& msg) noexcept
    -> void
{
    const auto& log = log_;
    const auto payload = msg.Payload();

    OT_ASSERT(1_uz < payload.size());

    const auto chain = payload[1].as<opentxs::blockchain::Type>();
    auto& peer = peer_manager_index_;

    if (auto i = peer.find(chain); peer.end() != i) {
        const auto post1 = ScopeGuard{[&] {
            peer.erase(i);
            log(OT_PRETTY_CLASS())(name_)(": ")(print(chain))(
                " peer manager unregistered")
                .Flush();
        }};
        const auto& [_1, connections] = i->second;

        for (const auto& remoteID : connections) {
            auto& map = blockchain_index_;

            if (auto j = map.find(remoteID); map.end() != j) {
                const auto post2 = ScopeGuard{[&] { map.erase(j); }};
                const auto& [cookie, localID, _2] = j->second;
                blockchain_reverse_index_.erase(cookie);
                outgoing_blockchain_index_.erase(localID);
            }
        }
    }
}

auto Node::Actor::process_new_cfilter(Message&& msg) noexcept -> void
{
    const auto body = msg.Payload();

    if (4 >= body.size()) {
        LogAbort()(OT_PRETTY_CLASS())(name_)(": invalid message").Abort();
    }

    process_cfilter(
        body[1].as<opentxs::blockchain::Type>(),
        {body[3].as<opentxs::blockchain::block::Height>(), body[4].Bytes()});
}

auto Node::Actor::process_new_peer(Message&& msg) noexcept -> void
{
    const auto body = msg.Payload();

    if (2 >= body.size()) {
        LogAbort()(OT_PRETTY_CLASS())(name_)(": invalid message").Abort();
    }

    const auto active = body[2].as<bool>();

    if (active) { process_peer(body[1].Bytes()); }
}

auto Node::Actor::process_peer(std::string_view endpoint) noexcept -> void
{
    // TODO c++20 use contains
    if (0_uz < peers_.count(endpoint)) {
        log_(OT_PRETTY_CLASS())(name_)("already connected to ")(endpoint)
            .Flush();

        return;
    } else {
        log_(OT_PRETTY_CLASS())(name_)("connecting to ")(endpoint).Flush();
    }

    auto alloc = get_allocator();
    using Socket = zeromq::socket::Type;
    auto [it, rc] = peers_.try_emplace(
        CString{endpoint, alloc},
        Peer::NextID(alloc),
        zeromq::MakeArbitraryInproc(alloc),
        api_.Network().ZeroMQ().Internal().RawSocket(Socket::Push));

    OT_ASSERT(rc);

    auto& [routingID, pushEndpoint, socket] = it->second;
    rc = socket.Connect(pushEndpoint.data());

    OT_ASSERT(rc);

    Peer{api_p_, shared_p_, routingID, endpoint, pushEndpoint}.Init();
    publish_peers();
}

auto Node::Actor::process_registration(Message&& msg) noexcept -> void
{
    const auto body = msg.Payload();

    if (3 >= body.size()) {
        LogAbort()(OT_PRETTY_CLASS())(name_)(": invalid message").Abort();
    }

    process_cfilter(
        body[1].as<opentxs::blockchain::Type>(),
        {body[2].as<opentxs::blockchain::block::Height>(), body[3].Bytes()});
    const auto local = get_peers();
    const auto remote = [&] {
        auto out = Set<CString>{get_allocator()};

        for (auto i = std::next(body.begin(), 4_z), stop = body.end();
             i != stop;
             ++i) {
            out.emplace(i->Bytes());
        }

        return out;
    }();
    const auto missing = [&] {
        auto out = Vector<CString>{get_allocator()};
        std::set_difference(
            local.begin(),
            local.end(),
            remote.begin(),
            remote.end(),
            std::back_inserter(out));

        return out;
    }();
    router_.SendDeferred(
        [&] {
            auto out = zeromq::tagged_reply_to_message(
                msg, blockchain::DHTJob::registration);

            for (const auto& endpoint : missing) { out.AddFrame(endpoint); }

            return out;
        }(),
        __FILE__,
        __LINE__);
    publish_peers();
}

auto Node::Actor::publish_peers() noexcept -> void
{
    publish_.SendDeferred(
        [&] {
            auto out = MakeWork(OT_ZMQ_OTDHT_PEER_LIST);
            get_peers(out);
            return out;
        }(),
        __FILE__,
        __LINE__);
}

auto Node::Actor::send_to_peers(Message&& msg) noexcept -> void
{
    for (auto& [key, value] : peers_) {
        auto& [routingID, endpoint, socket] = value;
        socket.SendDeferred(Message{msg}, __FILE__, __LINE__);
    }
}

auto Node::Actor::work(allocator_type monotonic) noexcept -> bool
{
    return false;
}

Node::Actor::~Actor() = default;
}  // namespace opentxs::network::otdht
