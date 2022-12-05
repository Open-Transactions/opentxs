// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/peermanager/PeerManager.hpp"  // IWYU pragma: associated

#include <BlockchainPeerAddress.pb.h>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <frozen/bits/algorithms.h>
#include <frozen/bits/basic_types.h>
#include <frozen/unordered_map.h>
#include <algorithm>
#include <chrono>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <future>
#include <iterator>
#include <memory>
#include <optional>
#include <ratio>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "BoostAsio.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/database/Peer.hpp"
#include "internal/blockchain/node/Config.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/Types.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/network/blockchain/bitcoin/Factory.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/message/Message.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"
#include "internal/network/zeromq/socket/Types.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/api/crypto/Util.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/blockchain/p2p/Address.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/asio/Endpoint.hpp"
#include "opentxs/network/asio/Socket.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/FrameIterator.hpp"
#include "opentxs/network/zeromq/message/FrameSection.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/util/BlockchainProfile.hpp"
#include "opentxs/util/ConnectionMode.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Options.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node
{
using namespace std::literals;

auto print(PeerManagerJobs in) noexcept -> std::string_view
{
    using enum PeerManagerJobs;
    static constexpr auto map =
        frozen::make_unordered_map<PeerManagerJobs, std::string_view>({
            {shutdown, "shutdown"sv},
            {registration, "registration"sv},
            {resolve, "resolve"sv},
            {p2p, "p2p"sv},
            {disconnect, "disconnect"sv},
            {addpeer, "addpeer"sv},
            {addlistener, "addlistener"sv},
            {verifypeer, "verifypeer"sv},
            {broadcasttx, "broadcasttx"sv},
            {report, "report"sv},
            {init, "init"sv},
            {statemachine, "statemachine"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {
        LogAbort()(__FUNCTION__)(": invalid PeerManagerJobs: ")(
            static_cast<OTZMQWorkType>(in))
            .Abort();
    }
}
}  // namespace opentxs::blockchain::node

namespace opentxs
{
constexpr auto get_next(std::string_view& in, char delim = ',') noexcept
    -> std::string_view
{
    if (in.empty()) { return {}; }

    auto n = in.find(delim);

    switch (n) {
        case 0_uz: {
            auto out = std::string_view{};
            in.remove_prefix(sizeof(delim));

            return out;
        }
        case std::string_view::npos: {
            auto out = in;
            in = {};

            return out;
        }
        default: {
            auto out = in.substr(0_uz, n);
            in.remove_prefix(n + sizeof(delim));

            return out;
        }
    }
}
}  // namespace opentxs

namespace opentxs::blockchain::node::peermanager
{
using namespace std::literals;

Actor::Actor(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const node::Manager> node,
    database::Peer& db,
    std::string_view peers,
    network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : ActorType(
          *api,
          LogTrace(),
          [&] {
              using namespace std::literals;
              auto out = CString{alloc};
              out.append(print(node->Internal().Chain()));
              out.append(" peer manager"sv);

              return out;
          }(),
          0ms,
          std::move(batch),
          alloc,
          [&] {
              using enum network::zeromq::socket::Direction;
              auto sub = network::zeromq::EndpointArgs{alloc};
              sub.emplace_back(
                  api->Endpoints().Internal().BlockchainReportStatus(),
                  Connect);
              sub.emplace_back(api->Endpoints().Shutdown(), Connect);
              sub.emplace_back(
                  node->Internal().Endpoints().shutdown_publish_, Connect);

              return sub;
          }(),
          [&] {
              using enum network::zeromq::socket::Direction;
              auto pull = network::zeromq::EndpointArgs{alloc};
              pull.emplace_back(
                  node->Internal().Endpoints().peer_manager_pull_, Bind);

              return pull;
          }(),
          [&] {
              using api::session::internal::Endpoints;
              using enum network::zeromq::socket::Direction;
              auto dealer = network::zeromq::EndpointArgs{alloc};
              dealer.emplace_back(Endpoints::Asio(), Connect);

              return dealer;
          }(),
          [&] {
              using enum network::zeromq::socket::Direction;
              using enum network::zeromq::socket::Type;
              auto extra = Vector<network::zeromq::SocketData>{alloc};
              extra.emplace_back(
                  Router,
                  Vector<network::zeromq::EndpointArg>{alloc});  // NOTE
                                                                 // external_
              extra.emplace_back(Router, [&] {
                  auto out = Vector<network::zeromq::EndpointArg>{alloc};
                  out.emplace_back(
                      node->Internal().Endpoints().peer_manager_router_, Bind);

                  return out;
              }());  // NOTE internal_
              extra.emplace_back(Push, [&] {
                  auto out = Vector<network::zeromq::EndpointArg>{alloc};
                  out.emplace_back(
                      api->Endpoints()
                          .Internal()
                          .Internal()
                          .BlockchainMessageRouter(),
                      Connect);

                  return out;
              }());  // NOTE to_blockchain_api_
              extra.emplace_back(Push, [&] {
                  auto out = Vector<network::zeromq::EndpointArg>{alloc};
                  out.emplace_back(
                      node->Internal().Endpoints().peer_manager_push_, Bind);

                  return out;
              }());  // NOTE broadcast_tx_

              return extra;
          }())
    , api_p_(std::move(api))
    , node_p_(std::move(node))
    , me_()
    , api_(*api_p_)
    , node_(*node_p_)
    , db_(db)
    , external_(pipeline_.Internal().ExtraSocket(0))
    , internal_(pipeline_.Internal().ExtraSocket(1))
    , to_blockchain_api_(pipeline_.Internal().ExtraSocket(2))
    , broadcast_tx_(pipeline_.Internal().ExtraSocket(3))
    , external_id_(external_.ID())
    , internal_id_(internal_.ID())
    , dealer_id_(pipeline_.ConnectionIDDealer())
    , chain_(node_.Internal().Chain())
    , nonce_([&] {
        auto out = decltype(nonce_){0};
        const auto rc = api_.Crypto().Util().RandomizeMemory(&out, sizeof(out));

        OT_ASSERT(rc);

        return out;
    }())
    , preferred_services_([&] {
        auto out = Set<p2p::Service>{};
        const auto& profile = node_.Internal().GetConfig().profile_;
        using enum BlockchainProfile;

        switch (profile) {
            case desktop_native: {
                out.emplace(p2p::Service::CompactFilters);
            } break;
            case mobile:
            case desktop:
            case server: {
            } break;
            default: {
                LogAbort()(OT_PRETTY_CLASS())(name_)(": invalid profile ")(
                    print(profile))
                    .Abort();
            }
        }

        return out;
    }())
    , command_line_peers_([&] {
        auto out = Addresses{alloc};
        const auto& params = params::get(chain_);
        using enum p2p::Network;

        while (false == peers.empty()) {
            auto host = get_next(peers);

            if (host.empty()) { continue; }

            try {
                const auto bytes = ip::make_address_v4(host).to_bytes();
                const auto& addr = out.emplace_back(factory::BlockchainAddress(
                    api_,
                    params.P2PDefaultProtocol(),
                    ipv4,
                    ReadView{
                        reinterpret_cast<const char*>(bytes.data()),
                        bytes.size()},
                    params.P2PDefaultPort(),
                    chain_,
                    {},
                    {},
                    false));

                if (addr.IsValid()) {
                    LogConsole()("Adding ")(print(chain_))(" peer ")(
                        addr.Display())(" from command line")
                        .Flush();

                    continue;
                } else {
                    LogConsole()("Ignoring invalid ")(print(chain_))(
                        " address ")(addr.Display())
                        .Flush();
                    out.pop_back();
                }
            } catch (...) {
            }

            try {
                const auto bytes = ip::make_address_v6(host).to_bytes();
                const auto& addr = out.emplace_back(factory::BlockchainAddress(
                    api_,
                    params.P2PDefaultProtocol(),
                    ipv6,
                    ReadView{
                        reinterpret_cast<const char*>(bytes.data()),
                        bytes.size()},
                    params.P2PDefaultPort(),
                    chain_,
                    {},
                    {},
                    false));

                if (addr.IsValid()) {
                    LogConsole()("Adding ")(print(chain_))(" peer ")(
                        addr.Display())(" from command line")
                        .Flush();

                    continue;
                } else {
                    LogConsole()("Ignoring invalid ")(print(chain_))(
                        " address ")(addr.Display())
                        .Flush();
                    out.pop_back();
                }
            } catch (...) {
            }

            // TODO resolve default peers passed as dns names
        }

        return out;
    }())
    , peer_target_([&] {
        if (api_.GetOptions().TestMode()) { return 0_uz; }

        const auto standard = node_.Internal().GetConfig().PeerTarget(chain_);

        return std::max(standard, command_line_peers_.size());
    }())
    , dns_(std::nullopt)
    , socket_queue_(alloc)
    , asio_listeners_(alloc)
    , zmq_external_(alloc)
    , zmq_internal_(alloc)
    , next_id_(invalid_peer_)
    , peers_(alloc)
    , index_(alloc)
    , active_(alloc)
    , verified_(alloc)
    , outgoing_(alloc)
    , dns_timer_(api_.Network().Asio().Internal().GetTimer())
{
    constexpr auto output = [](const auto& value) {
        auto copy{value};

        return get_next(copy);
    };
    constexpr auto input = [](const auto& value) {
        auto copy{value};
        get_next(copy);

        return copy;
    };
    static_assert(output(""sv).empty());
    static_assert(input(""sv).empty());
    static_assert(output(","sv).empty());
    static_assert(input(","sv).empty());
    static_assert(output("127.0.0.1"sv) == "127.0.0.1"sv);
    static_assert(input("127.0.0.1"sv).empty());
    static_assert(output("127.0.0.1,"sv) == "127.0.0.1"sv);
    static_assert(input("127.0.0.1,"sv).empty());
    static_assert(output("127.0.0.1,127.0.0.2"sv) == "127.0.0.1"sv);
    static_assert(input("127.0.0.1,127.0.0.2"sv) == "127.0.0.2"sv);
}

auto Actor::accept(
    const p2p::Network type,
    const network::asio::Endpoint& endpoint,
    network::asio::Socket&& socket,
    boost::shared_ptr<Actor> me) noexcept -> void

{
    OT_ASSERT(me);

    auto address = factory::BlockchainAddress(
        me->api_,
        params::get(me->chain_).P2PDefaultProtocol(),
        type,
        endpoint.GetBytes(),
        endpoint.GetPort(),
        me->chain_,
        {},
        {},
        true);

    if (address.IsValid()) {
        me->socket_queue_.lock()->emplace_back(
            std::move(address), std::move(socket));
        me->pipeline_.Push(MakeWork(Work::statemachine));
    } else {
        LogError()(OT_PRETTY_STATIC(Actor))(": invalid address: ")(
            address.Display())
            .Flush();
    }
}

auto Actor::accept_asio() noexcept -> void
{
    auto queue = SocketQueue{get_allocator()};
    socket_queue_.lock()->swap(queue);

    while (false == queue.empty()) {
        auto& [address, socket] = queue.front();
        add_peer(std::move(address), true, std::move(socket));
        queue.pop_front();
    }
}

auto Actor::active_addresses(allocator_type monotonic) const noexcept
    -> Set<AddressID>
{
    auto out = Set<AddressID>{monotonic};
    std::transform(
        index_.begin(),
        index_.end(),
        std::inserter(out, out.end()),
        [](const auto& pair) { return pair.first; });

    return out;
}

auto Actor::add_peer(
    p2p::Address endpoint,
    bool incoming,
    std::optional<network::asio::Socket> asio,
    ReadView connection,
    std::optional<Message> msg) noexcept -> PeerID
{
    OT_ASSERT(endpoint.IsValid());

    endpoint.Internal().SetIncoming(incoming);

    if (is_active(endpoint)) {
        log_(OT_PRETTY_CLASS())(name_)(": address ")(endpoint.Display())(
            " is already active")
            .Flush();

        return invalid_peer_;
    }

    const auto inproc = network::zeromq::MakeArbitraryInproc();
    const auto peerID = [&, this] {
        using enum network::zeromq::socket::Type;
        auto [it, isNew] = peers_.try_emplace(
            ++next_id_,
            endpoint.ID(),
            api_.Network().ZeroMQ().Internal().RawSocket(Push),
            connection,
            get_allocator());

        OT_ASSERT(isNew);

        auto& [address, socket, external, internal, cache] = it->second;
        const auto listen = socket.Bind(inproc.data());

        OT_ASSERT(listen);
        OT_ASSERT(external.Bytes() == connection);
        OT_ASSERT(internal.empty());

        if (msg.has_value()) { cache.emplace_back(std::move(*msg)); }

        return it->first;
    }();
    index_[endpoint.ID()] = peerID;
    using enum p2p::Protocol;
    const auto protocol = params::get(chain_).P2PDefaultProtocol();

    switch (protocol) {
        case bitcoin: {
            factory::BlockchainPeerBitcoin(
                api_p_,
                node_p_,
                nonce_,
                peerID,
                std::move(endpoint),
                inproc,
                std::move(asio));
        } break;
        case opentxs:
        case ethereum:
        default: {
            LogAbort()(OT_PRETTY_CLASS())(
                name_)(": unsupported network protocol ")(print(protocol))
                .Abort();
        }
    }

    active_.emplace(peerID);

    if (false == incoming) { outgoing_.emplace(peerID); }

    broadcast_active();

    return peerID;
}

auto Actor::broadcast_active() noexcept -> void
{
    to_blockchain_api_.SendDeferred(
        [&] {
            using enum WorkType;
            auto work =
                network::zeromq::tagged_message(BlockchainPeerConnected);
            work.AddFrame(chain_);
            work.AddFrame(active_.size());

            return work;
        }(),
        __FILE__,
        __LINE__);
}

auto Actor::broadcast_verified(std::string_view address) noexcept -> void
{
    to_blockchain_api_.SendDeferred(
        [&] {
            using enum WorkType;
            auto work = network::zeromq::tagged_message(BlockchainPeerAdded);
            work.AddFrame(chain_);
            work.AddFrame(address.data(), address.size());
            work.AddFrame(verified_.size());

            return work;
        }(),
        __FILE__,
        __LINE__);
}

auto Actor::check_command_line_peers() noexcept -> void
{
    for (const auto& peer : command_line_peers_) {
        if (is_active(peer)) {

            continue;
        } else {
            add_peer(peer, false);
        }
    }
}

auto Actor::check_peers(allocator_type monotonic) noexcept -> void
{
    if (have_target_peers()) { return; }

    if (auto peer = get_peer(monotonic); peer.IsValid()) {
        add_peer(std::move(peer), false);
    }
}

auto Actor::check_dns() noexcept -> void
{
    if (dns_timed_out()) {
        dns_.reset();
        dns_timer_.Cancel();
    }
}

auto Actor::dns_timed_out() const noexcept -> bool
{
    if (dns_) {

        return (sClock::now() - *dns_) >= dns_timeout_;
    } else {

        return false;
    }
}

auto Actor::do_shutdown() noexcept -> void
{
    dns_timer_.Cancel();

    for (const auto& endpoint : asio_listeners_) {
        api_.Network().Asio().Internal().Close(endpoint);
    }

    for (auto& [id, data] : peers_) {
        using enum WorkType;
        data.socket_.SendDeferred(MakeWork(Shutdown), __FILE__, __LINE__);
    }

    me_.reset();
    node_p_.reset();
    api_p_.reset();
}

auto Actor::do_startup(allocator_type monotonic) noexcept -> bool
{
    if ((api_.Internal().ShuttingDown()) || (node_.Internal().ShuttingDown())) {

        return true;
    }

    const auto& params = params::get(chain_);
    using enum p2p::Network;

    for (const auto& addr : api_.GetOptions().BlockchainBindIpv4()) {
        try {
            const auto boost = boost::asio::ip::make_address(addr);

            if (false == boost.is_v4()) {
                const auto error = UnallocatedCString(addr).append(
                    " is not a valid ipv4 address");

                throw std::runtime_error{error};
            }

            const auto addr = [&] {
                auto out = api_.Factory().Data();
                const auto v4 = boost.to_v4();
                const auto bytes = v4.to_bytes();
                out.Assign(bytes.data(), bytes.size());

                return out;
            }();
            auto address = opentxs::factory::BlockchainAddress(
                api_,
                params.P2PDefaultProtocol(),
                ipv4,
                addr.Bytes(),
                params::get(chain_).P2PDefaultPort(),
                chain_,
                {},
                {},
                false);
            listen(address, monotonic);
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            continue;
        }
    }

    for (const auto& addr : api_.GetOptions().BlockchainBindIpv6()) {
        try {
            const auto boost = boost::asio::ip::make_address(addr);

            if (false == boost.is_v6()) {
                const auto error = UnallocatedCString(addr).append(
                    " is not a valid ipv6 address");

                throw std::runtime_error{error};
            }

            const auto addr = [&] {
                auto out = api_.Factory().Data();
                const auto v6 = boost.to_v6();
                const auto bytes = v6.to_bytes();
                out.Assign(bytes.data(), bytes.size());

                return out;
            }();
            auto address = opentxs::factory::BlockchainAddress(
                api_,
                params.P2PDefaultProtocol(),
                ipv6,
                addr.Bytes(),
                params::get(chain_).P2PDefaultPort(),
                chain_,
                {},
                {},
                false);
            listen(address, monotonic);
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            continue;
        }
    }

    for (const auto& peer : command_line_peers_) { db_.AddOrUpdate(peer); }

    do_work(monotonic);

    return false;
}

auto Actor::forward_message(
    network::zeromq::socket::Raw& socket,
    ReadView id,
    Message&& message) noexcept -> void
{
    socket.SendDeferred(
        [&] {
            auto out = Message{};
            out.AddFrame(id.data(), id.size());
            out.StartBody();

            for (auto& frame : message.Body()) {
                out.AddFrame(std::move(frame));
            }

            return out;
        }(),
        __FILE__,
        __LINE__);
}

auto Actor::get_peer(allocator_type monotonic) noexcept -> p2p::Address
{
    auto peer = db_.Get(
        params::get(chain_).P2PDefaultProtocol(),
        usable_networks(monotonic),
        preferred_services_,
        active_addresses(monotonic));

    if (peer.IsValid()) { return peer; }

    send_dns_query();

    return {};
}

auto Actor::have_target_peers() const noexcept -> bool
{
    return outgoing_.size() >= peer_target_;
}

auto Actor::is_active(const p2p::Address& addr) const noexcept -> bool
{
    return index_.contains(addr.ID());
}

auto Actor::listen(
    const p2p::Address& address,
    allocator_type monotonic) noexcept -> void
{
    if (false == address.IsValid()) {
        LogError()(OT_PRETTY_CLASS())("invalid address: ")(address.Display())
            .Flush();

        return;
    }

    using enum p2p::Network;
    const auto type = address.Type();

    switch (type) {
        case zmq: {
            listen_zmq(address, monotonic);
        } break;
        case ipv6:
        case ipv4: {
            listen_tcp(address);
        } break;
        case onion2:
        case onion3:
        case eep:
        case cjdns:
        default: {
            LogError()(OT_PRETTY_CLASS())("unsupported network type ")(
                print(type))
                .Flush();
        }
    }
}

auto Actor::listen_tcp(const p2p::Address& address) noexcept -> void
{
    try {
        const auto type = address.Type();
        const auto port = address.Port();
        const auto addr = address.Bytes();
        const auto& endpoint = [&]() -> auto&
        {
            using enum p2p::Network;
            using opentxs::network::asio::Endpoint;
            auto network = Endpoint::Type{};

            switch (type) {
                case ipv4: {
                    network = Endpoint::Type::ipv4;
                } break;
                case ipv6: {
                    network = Endpoint::Type::ipv6;
                } break;
                default: {
                    throw std::runtime_error{"invalid address"};
                }
            }

            return asio_listeners_.emplace_back(network, addr.Bytes(), port);
        }
        ();
        const auto accepted = api_.Network().Asio().Internal().Accept(
            endpoint, [=, me = me_](auto&& socket) {
                accept(type, endpoint, std::move(socket), me);
            });

        if (accepted) {
            log_(OT_PRETTY_CLASS())(name_)(
                ": listening for incoming connections on ")(address.Display())
                .Flush();
        } else {
            LogError()(OT_PRETTY_CLASS())(name_)(": unable to bind to ")(
                address.Display())
                .Flush();
            asio_listeners_.pop_back();
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
    }
}

auto Actor::listen_zmq(
    const p2p::Address& address,
    allocator_type monotonic) noexcept -> void
{
    const auto endpoint =
        CString{address.Bytes().Bytes(), monotonic}.append(":").append(
            std::to_string(address.Port()));

    if (external_.Bind(endpoint.c_str())) {
        LogConsole()(name_)(" listening on ")(endpoint).Flush();
    } else {
        LogError()(name_)(" unable to bind to ")(endpoint).Flush();
    }
}

auto Actor::need_peers() const noexcept -> bool
{
    if (have_target_peers()) { return false; }

    if (dns_timer_.IsActive()) { return false; }

    return true;
}

auto Actor::pipeline(
    const Work work,
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    using network::zeromq::SocketID;
    const auto socket = msg.Internal().ExtractFront().as<SocketID>();

    if (socket == external_id_) {
        pipeline_external(work, std::move(msg), monotonic);
    } else if (socket == internal_id_) {
        pipeline_internal(work, std::move(msg), monotonic);
    } else if (socket == dealer_id_) {
        pipeline_dealer(work, std::move(msg), monotonic);
        do_work(monotonic);
    } else {
        pipeline_standard(work, std::move(msg), monotonic);
        do_work(monotonic);
    }
}

auto Actor::pipeline_dealer(
    const Work work,
    Message&& msg,
    allocator_type) noexcept -> void
{
    using enum PeerManagerJobs;

    switch (work) {
        case resolve: {
            process_resolve(std::move(msg));
        } break;
        case shutdown:
        case registration:
        case p2p:
        case disconnect:
        case addpeer:
        case addlistener:
        case verifypeer:
        case broadcasttx:
        case report:
        case init:
        case statemachine: {
            unhandled_type(work, "on dealer socket"sv);
        }
        default: {
            unknown_type(work);
        }
    }
}

auto Actor::pipeline_internal(
    const Work work,
    Message&& msg,
    allocator_type) noexcept -> void
{
    const auto header = msg.Header();

    OT_ASSERT(0 < header.size());

    const auto connection = header.at(0).Bytes();
    using enum PeerManagerJobs;

    switch (work) {
        case registration: {
            process_registration(std::move(msg), connection);
        } break;
        case p2p: {
            process_p2p_internal(std::move(msg), connection);
        } break;
        case shutdown:
        case resolve:
        case disconnect:
        case addpeer:
        case addlistener:
        case verifypeer:
        case broadcasttx:
        case report:
        case init:
        case statemachine: {
            unhandled_type(work, "on internal socket"sv);
        }
        default: {
            unknown_type(work);
        }
    }
}

auto Actor::pipeline_external(
    const Work work,
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    const auto header = msg.Header();
    const auto body = msg.Body();

    if (0_uz == header.size()) {
        log_(OT_PRETTY_CLASS())(name_)(": ignoring message with missing header")
            .Flush();

        return;
    }

    if (0_uz == body.size()) {
        log_(OT_PRETTY_CLASS())(name_)(": ignoring message with missing body")
            .Flush();

        return;
    }

    using enum PeerManagerJobs;

    switch (work) {
        case p2p: {
            process_p2p_external(std::move(msg), monotonic);
        } break;
        case shutdown:
        case registration:
        case resolve:
        case disconnect:
        case addpeer:
        case addlistener:
        case verifypeer:
        case broadcasttx:
        case report:
        case init:
        case statemachine: {
            log_(OT_PRETTY_CLASS())(name_)(": ignoring invalid message type: ")(
                print(work))
                .Flush();
        } break;
        default: {
            log_(OT_PRETTY_CLASS())(name_)(": ignoring unknown message type: ")(
                static_cast<OTZMQWorkType>(work))
                .Flush();
        }
    }
}

auto Actor::pipeline_standard(
    const Work work,
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    using enum PeerManagerJobs;

    switch (work) {
        case disconnect: {
            process_disconnect(std::move(msg));
        } break;
        case addpeer: {
            process_addpeer(std::move(msg));
        } break;
        case addlistener: {
            process_addlistener(std::move(msg), monotonic);
        } break;
        case verifypeer: {
            process_verify(std::move(msg));
        } break;
        case broadcasttx: {
            process_broadcasttx(std::move(msg));
        } break;
        case report: {
            process_report(std::move(msg));
        } break;
        case shutdown:
        case registration:
        case resolve:
        case p2p:
        case init:
        case statemachine: {
            unhandled_type(work);
        }
        default: {
            unknown_type(work);
        }
    }
}

auto Actor::process_addlistener(
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    const auto body = msg.Body();
    OT_ASSERT(1 < body.size());

    auto address = api_.Factory().InternalSession().BlockchainAddress(
        proto::Factory<proto::BlockchainPeerAddress>(body.at(1)));

    if (address.IsValid()) {
        listen(address, monotonic);
    } else {
        LogError()(OT_PRETTY_CLASS())(name_)(": invalid address").Flush();
    }
}

auto Actor::process_addpeer(Message&& msg) noexcept -> void
{
    const auto body = msg.Body();
    OT_ASSERT(1 < body.size());

    auto address = api_.Factory().InternalSession().BlockchainAddress(
        proto::Factory<proto::BlockchainPeerAddress>(body.at(1)));

    if (address.IsValid()) {
        add_peer(std::move(address), false);
    } else {
        LogError()(OT_PRETTY_CLASS())(name_)(": invalid address").Flush();
    }
}

auto Actor::process_broadcasttx(Message&& msg) noexcept -> void
{
    broadcast_tx_.SendDeferred(std::move(msg), __FILE__, __LINE__);
}

auto Actor::process_disconnect(Message&& msg) noexcept -> void
{
    const auto body = msg.Body();

    OT_ASSERT(2_uz < body.size());

    process_disconnect(body.at(1).as<PeerID>(), body.at(2).Bytes());
}

auto Actor::process_disconnect(PeerID id, std::string_view display) noexcept
    -> void
{
    if (auto i = peers_.find(id); peers_.end() != i) {
        log_(OT_PRETTY_CLASS())(name_)(": disconnecting peer ")(id);
        {
            auto& [address, socket, external, internal, _3] = i->second;
            index_.erase(address);
            socket.Stop();
            zmq_internal_.erase(internal);
            zmq_external_.erase(external);
        }
        peers_.erase(i);
        active_.erase(id);
        verified_.erase(id);
        outgoing_.erase(id);
        broadcast_verified(display);
        broadcast_active();
    } else {
        log_(OT_PRETTY_CLASS())(name_)(": peer ")(id)(" not found").Flush();
    }
}

auto Actor::process_p2p_external(
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    const auto header = msg.Header();
    const auto body = msg.Body();
    const auto id = api_.Factory().Data(header.at(0));

    if (auto i = zmq_external_.find(id); zmq_external_.end() != i) {
        auto& [_1, socket, _2, internal, queue] = peers_.at(i->second);

        if (internal.empty()) {
            queue.emplace_back(std::move(msg));
        } else {
            forward_message(internal_, internal.Bytes(), std::move(msg));
        }
    } else {
        const auto peer = add_peer(
            factory::BlockchainAddress(
                api_,
                params::get(chain_).P2PDefaultProtocol(),
                p2p::Network::zmq,
                CString{"zeromq_", monotonic}.append(id.asHex()),
                params::get(chain_).P2PDefaultPort(),
                chain_,
                {},
                {},
                true),
            true,
            std::nullopt,
            id.Bytes(),
            std::move(msg));

        OT_ASSERT(invalid_peer_ != peer);

        zmq_external_[id] = peer;
    }
}

auto Actor::process_p2p_internal(Message&& msg, const ConnectionID& id) noexcept
    -> void
{
    if (auto i = zmq_internal_.find(id); zmq_internal_.end() != i) {
        auto& [_1, _2, external, _3, _4] = peers_.at(i->second);
        forward_message(external_, external.Bytes(), std::move(msg));
    } else {
        LogAbort()(OT_PRETTY_CLASS())(name_)(": peer ")
            .asHex(id)(" is not registered")
            .Abort();
    }
}

auto Actor::process_registration(
    Message&& msg,
    const ConnectionID& connection) noexcept -> void
{
    auto body = msg.Body();

    OT_ASSERT(1 < body.size());

    const auto id = body.at(1).as<PeerID>();

    if (auto i = peers_.find(id); peers_.end() != i) {
        auto& [_1, _2, _3, internal, queue] = i->second;
        internal = connection;
        internal_.SendDeferred(
            network::zeromq::tagged_reply_to_message(
                msg, WorkType::AsioRegister),
            __FILE__,
            __LINE__);

        while (false == queue.empty()) {
            forward_message(
                internal_, connection.Bytes(), std::move(queue.front()));
            queue.pop_front();
        }

        zmq_internal_[connection] = id;
    } else {
        LogAbort()(OT_PRETTY_CLASS())(name_)(": peer ")(id)(" does not exist")
            .Abort();
    }
}

auto Actor::process_report(Message&&) noexcept -> void
{
    broadcast_verified();
    broadcast_active();
}

auto Actor::process_resolve(Message&& msg) noexcept -> void
{
    static constexpr auto success = std::byte{0x01};
    const auto body = msg.Body();

    OT_ASSERT(2 < body.size());

    dns_.reset();
    dns_timer_.Cancel();

    if (body.at(1).as<std::byte>() == success) {
        const auto port = body.at(2).as<std::uint16_t>();

        for (auto i = std::next(body.begin(), 3), end = body.end(); i != end;
             ++i) {
            const auto& frame = *i;
            const auto network = [&] {
                static constexpr auto v4Bytes =
                    sizeof(boost::asio::ip::address_v4::bytes_type);
                static constexpr auto v6Bytes =
                    sizeof(boost::asio::ip::address_v6::bytes_type);
                const auto size = frame.size();
                using enum p2p::Network;

                switch (size) {
                    case v4Bytes: {

                        return ipv4;
                    }
                    case v6Bytes: {

                        return ipv6;
                    }
                    default: {
                        LogAbort()(OT_PRETTY_CLASS())(
                            name_)(": invalid address size (")(size)(")")
                            .Abort();
                    }
                }
            }();
            auto addr = factory::BlockchainAddress(
                api_,
                params::get(chain_).P2PDefaultProtocol(),
                network,
                frame.Bytes(),
                port,
                chain_,
                Time{},
                {},
                false);

            if (addr.IsValid()) { db_.AddOrUpdate(std::move(addr)); }
        }
    } else {
        log_(OT_PRETTY_CLASS())(name_)(": ")(body.at(2).Bytes()).Flush();
    }
}

auto Actor::process_verify(Message&& msg) noexcept -> void
{
    const auto body = msg.Body();

    OT_ASSERT(2_uz < body.size());

    process_verify(body.at(1).as<PeerID>(), body.at(2).Bytes());
}

auto Actor::process_verify(PeerID id, std::string_view display) noexcept -> void
{
    verified_.insert(active_.extract(id));
    log_(OT_PRETTY_CLASS())(name_)(": peer ")(id)(" reached verified state")
        .Flush();
    broadcast_verified(display);
}

auto Actor::reset_dns_timer() noexcept -> void
{
    reset_timer(dns_timeout_, dns_timer_, Work::statemachine);
    dns_ = sClock::now();
}

auto Actor::send_dns_query() noexcept -> void
{
    if (dns_timer_.IsActive()) { return; }

    const auto& data = params::get(chain_);

    for (const auto& host : data.P2PSeeds()) {
        pipeline_.Internal().SendFromThread([&] {
            auto out = MakeWork(WorkType::AsioResolve);
            out.AddFrame(host.data(), host.size());
            out.AddFrame(data.P2PDefaultPort());

            return out;
        }());
    }

    reset_dns_timer();
}

auto Actor::usable_networks(allocator_type monotonic) const noexcept
    -> Set<p2p::Network>
{
    using enum p2p::Network;
    using enum ConnectionMode;
    using enum std::future_status;
    auto output = Set<p2p::Network>{monotonic};
    const auto& asio = api_.Network().Asio();
    constexpr auto limit = 1s;

    switch (api_.GetOptions().Ipv4ConnectionMode()) {
        case off: {
            output.erase(ipv4);
        } break;
        case on: {
            output.insert(ipv4);
        } break;
        case automatic:
        default: {
            if (auto f = asio.GetPublicAddress4(); ready == f.wait_for(limit)) {
                if (auto addr = f.get(); false == addr.empty()) {
                    output.insert(ipv4);
                }
            }
        }
    }

    switch (api_.GetOptions().Ipv6ConnectionMode()) {
        case off: {
            output.erase(ipv6);
        } break;
        case on: {
            output.insert(ipv6);
        } break;
        case automatic:
        default: {
            if (auto f = asio.GetPublicAddress6(); ready == f.wait_for(limit)) {
                if (auto addr = f.get(); false == addr.empty()) {
                    output.insert(ipv6);
                }
            }
        }
    }

    return output;
}

auto Actor::work(allocator_type monotonic) noexcept -> bool
{
    check_command_line_peers();
    check_dns();
    check_peers(monotonic);
    accept_asio();

    return need_peers();
}

Actor::~Actor() = default;
}  // namespace opentxs::blockchain::node::peermanager
