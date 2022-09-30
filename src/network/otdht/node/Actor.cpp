// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                  // IWYU pragma: associated
#include "network/otdht/node/Actor.hpp"  // IWYU pragma: associated

#include <boost/smart_ptr/shared_ptr.hpp>
#include <algorithm>
#include <chrono>
#include <iterator>
#include <memory>
#include <utility>

#include "internal/api/session/Endpoints.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/network/blockchain/Types.hpp"
#include "internal/network/otdht/Listener.hpp"
#include "internal/network/otdht/Peer.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/message/Message.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/BlockchainHandle.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/network/OTDHT.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameIterator.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WorkType.hpp"
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
              using Dir = zeromq::socket::Direction;
              auto sub = zeromq::EndpointArgs{alloc};
              sub.emplace_back(
                  CString{api->Endpoints().Shutdown(), alloc}, Dir::Connect);
              sub.emplace_back(
                  CString{
                      api->Endpoints().BlockchainSyncServerUpdated(), alloc},
                  Dir::Connect);
              sub.emplace_back(
                  CString{api->Endpoints().BlockchainNewFilter(), alloc},
                  Dir::Connect);
              sub.emplace_back(
                  CString{api->Endpoints().BlockchainStateChange(), alloc},
                  Dir::Connect);

              return sub;
          }(),
          [&] {
              using Dir = zeromq::socket::Direction;
              auto pull = zeromq::EndpointArgs{alloc};
              pull.emplace_back(
                  CString{api->Endpoints().Internal().OTDHTNodePull(), alloc},
                  Dir::Bind);

              return pull;
          }(),
          {},
          [&] {
              auto out = Vector<network::zeromq::SocketData>{alloc};
              using Socket = network::zeromq::socket::Type;
              using Args = network::zeromq::EndpointArgs;
              using Dir = network::zeromq::socket::Direction;
              out.emplace_back(std::make_pair<Socket, Args>(
                  Socket::Publish,
                  {
                      {CString{
                           api->Endpoints().Internal().OTDHTNodePublish(),
                           alloc},
                       Dir::Bind},
                  }));
              out.emplace_back(std::make_pair<Socket, Args>(
                  Socket::Router,
                  {
                      {CString{
                           api->Endpoints().Internal().OTDHTNodeRouter(),
                           alloc},
                       Dir::Bind},
                  }));

              return out;
          }())
    , api_p_(std::move(api))
    , shared_p_(std::move(shared))
    , api_(*api_p_)
    , data_(shared_p_->data_)
    , publish_(pipeline_.Internal().ExtraSocket(0))
    , router_(pipeline_.Internal().ExtraSocket(1))
    , peers_(alloc)
    , listeners_(alloc)
    , listen_endpoints_(alloc)
{
}

auto Node::Actor::do_shutdown() noexcept -> void
{
    shared_p_.reset();
    api_p_.reset();
}

auto Node::Actor::do_startup() noexcept -> bool
{
    if (api_.Internal().ShuttingDown()) { return true; }

    load_positions();
    load_peers();

    return false;
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

auto Node::Actor::load_peers() noexcept -> void
{
    for (const auto& peer :
         api_.Network().OTDHT().KnownPeers(get_allocator())) {
        process_peer(peer);
    }
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

auto Node::Actor::pipeline(const Work work, Message&& msg) noexcept -> void
{
    const auto id = msg.Internal().ExtractFront().as<zeromq::SocketID>();

    if (router_.ID() == id) {
        pipeline_router(work, std::move(msg));
    } else {
        pipeline_other(work, std::move(msg));
    }
}

auto Node::Actor::pipeline_other(const Work work, Message&& msg) noexcept
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

auto Node::Actor::pipeline_router(const Work work, Message&& msg) noexcept
    -> void
{
    switch (work) {
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
    const auto body = msg.Body();

    if (4 >= body.size()) {
        LogAbort()(OT_PRETTY_CLASS())(name_)(": invalid message").Abort();
    }

    const auto routerBind = body.at(1).Bytes();
    const auto routerAdvertise = body.at(2).Bytes();
    const auto publishBind = body.at(3).Bytes();
    const auto publishAdvertise = body.at(4).Bytes();

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
    const auto body = msg.Body();

    if (2 >= body.size()) {
        LogAbort()(OT_PRETTY_CLASS())(name_)(": invalid message").Abort();
    }

    const auto chain = body.at(1).as<opentxs::blockchain::Type>();
    auto enabled = body.at(2).as<bool>();
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

auto Node::Actor::process_new_cfilter(Message&& msg) noexcept -> void
{
    const auto body = msg.Body();

    if (4 >= body.size()) {
        LogAbort()(OT_PRETTY_CLASS())(name_)(": invalid message").Abort();
    }

    process_cfilter(
        body.at(1).as<opentxs::blockchain::Type>(),
        {body.at(3).as<opentxs::blockchain::block::Height>(),
         body.at(4).Bytes()});
}

auto Node::Actor::process_new_peer(Message&& msg) noexcept -> void
{
    const auto body = msg.Body();

    if (2 >= body.size()) {
        LogAbort()(OT_PRETTY_CLASS())(name_)(": invalid message").Abort();
    }

    const auto active = body.at(2).as<bool>();

    if (active) { process_peer(body.at(1).Bytes()); }
}

auto Node::Actor::process_peer(std::string_view endpoint) noexcept -> void
{
    // TODO c++20 use contains
    if (0_uz < peers_.count(endpoint)) { return; }

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
    const auto body = msg.Body();

    if (3 >= body.size()) {
        LogAbort()(OT_PRETTY_CLASS())(name_)(": invalid message").Abort();
    }

    process_cfilter(
        body.at(1).as<opentxs::blockchain::Type>(),
        {body.at(2).as<opentxs::blockchain::block::Height>(),
         body.at(3).Bytes()});
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

auto Node::Actor::work() noexcept -> bool { return false; }

Node::Actor::~Actor() = default;
}  // namespace opentxs::network::otdht
