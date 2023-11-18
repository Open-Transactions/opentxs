// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/peermanager/PeerManager.hpp"  // IWYU pragma: associated

#include <BlockchainPeerAddress.pb.h>
#include <boost/json.hpp>
#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <algorithm>
#include <chrono>
#include <compare>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <future>
#include <iterator>
#include <limits>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "BoostAsio.hpp"
#include "internal/api/network/Asio.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/blockchain/database/Peer.hpp"
#include "internal/blockchain/node/Config.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/Types.hpp"
#include "internal/blockchain/params/ChainData.hpp"
#include "internal/network/asio/Types.hpp"
#include "internal/network/blockchain/Address.hpp"
#include "internal/network/blockchain/Types.hpp"
#include "internal/network/blockchain/bitcoin/Factory.hpp"
#include "internal/network/otdht/Types.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/socket/Pipeline.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Timer.hpp"
#include "network/blockchain/Seednodes.hpp"
#include "opentxs/api/Factory.internal.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/Session.internal.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/crypto/Util.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/network/OTDHT.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Endpoints.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Factory.internal.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/asio/Endpoint.hpp"
#include "opentxs/network/asio/Socket.hpp"
#include "opentxs/network/blockchain/Address.hpp"
#include "opentxs/network/blockchain/Protocol.hpp"   // IWYU pragma: keep
#include "opentxs/network/blockchain/Transport.hpp"  // IWYU pragma: keep
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/Service.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/network/zeromq/message/Envelope.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/network/zeromq/socket/Direction.hpp"   // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Policy.hpp"      // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/SocketType.hpp"  // IWYU pragma: keep
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/BlockchainProfile.hpp"
#include "opentxs/util/Bytes.hpp"
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
            {resolve, "resolve"sv},
            {disconnect, "disconnect"sv},
            {addpeer, "addpeer"sv},
            {addlistener, "addlistener"sv},
            {verifypeer, "verifypeer"sv},
            {spawn_peer, "spawn_peer"sv},
            {register_ack, "register_ack"sv},
            {gossip_address, "gossip_address"sv},
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
auto Actor::DNS::GotResponse() noexcept -> void
{
    last_sent_query_.reset();
    last_received_response_ = sClock::now();
}

auto Actor::DNS::NeedQuery() noexcept -> bool
{
    const auto now = sClock::now();

    if (auto& last = last_sent_query_; last) {
        const auto duration = now - *last;

        if (duration > timeout_) {
            last = now;

            return true;
        } else {

            return false;
        }
    } else if (auto& prior = last_received_response_; prior) {
        const auto duration = now - *prior;

        if (duration > repeat_) {
            prior.reset();
            last = now;

            return true;
        } else {

            return false;
        }
    } else {
        last = now;

        return true;
    }
}
}  // namespace opentxs::blockchain::node::peermanager

namespace opentxs::blockchain::node::peermanager
{
auto Actor::Seed::RetryDNS(const sTime& now) const noexcept -> bool
{
    if (address_.IsValid()) { return false; }

    if (last_dns_query_.has_value()) {
        using namespace std::chrono;
        const auto duration = duration_cast<seconds>(now - *last_dns_query_);

        return duration >= dns_timeout_;
    } else {

        return true;
    }
}
}  // namespace opentxs::blockchain::node::peermanager

namespace opentxs::blockchain::node::peermanager
{
using namespace std::literals;
using enum opentxs::network::zeromq::socket::Direction;
using enum opentxs::network::zeromq::socket::Policy;
using enum opentxs::network::zeromq::socket::Type;

Actor::Actor(
    std::shared_ptr<const api::internal::Session> api,
    std::shared_ptr<const node::Manager> node,
    database::Peer& db,
    std::string_view peers,
    network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : ActorType(
          api->Self(),
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
          {
              {api->Endpoints().Internal().BlockchainReportStatus(), Connect},
              {api->Endpoints().Shutdown(), Connect},
              {node->Internal().Endpoints().shutdown_publish_, Connect},
          },
          {
              {node->Internal().Endpoints().peer_manager_pull_, Bind},
          },
          {
              {api::session::internal::Endpoints::Asio(), Connect},
          },
          {
              {Push,
               Internal,
               {
                   {api->Endpoints().Internal().BlockchainMessageRouter(),
                    Connect},
               }},
              {Push,
               Internal,
               {
                   {node->Internal().Endpoints().peer_manager_push_, Bind},
               }},
              {Dealer,
               Internal,
               {
                   {api->Endpoints().Internal().OTDHTNodeRouter(), Connect},
               }},
              {Publish,
               Internal,
               {
                   {node->Internal().Endpoints().peer_manager_publish_, Bind},
               }},
          })
    , api_p_(std::move(api))
    , node_p_(std::move(node))
    , me_()
    , api_(api_p_->Self())
    , node_(*node_p_)
    , db_(db)
    , to_blockchain_api_(pipeline_.Internal().ExtraSocket(0))
    , broadcast_tx_(pipeline_.Internal().ExtraSocket(1))
    , to_otdht_(pipeline_.Internal().ExtraSocket(2))
    , to_peers_(pipeline_.Internal().ExtraSocket(3))
    , dealer_id_(pipeline_.ConnectionIDDealer())
    , otdht_id_(to_otdht_.ID())
    , chain_(node_.Internal().Chain())
    , nonce_([&] {
        auto out = decltype(nonce_){0};
        const auto rc = api_.Crypto().Util().RandomizeMemory(&out, sizeof(out));

        assert_true(rc);

        return out;
    }())
    , preferred_services_([&] {
        auto out = Set<network::blockchain::bitcoin::Service>{};
        const auto& profile = node_.Internal().GetConfig().profile_;
        using enum BlockchainProfile;

        switch (profile) {
            case desktop_native: {
                out.emplace(
                    network::blockchain::bitcoin::Service::CompactFilters);
            } break;
            case mobile:
            case desktop:
            case server: {
            } break;
            default: {
                LogAbort()()(name_)(": invalid profile ")(print(profile))
                    .Abort();
            }
        }

        return out;
    }())
    , command_line_peers_([&] {
        auto out = Addresses{alloc};
        const auto& params = params::get(chain_);
        using enum network::blockchain::Transport;

        while (false == peers.empty()) {
            auto host = get_next(peers);

            if (host.empty()) { continue; }

            using namespace network::asio;

            if (const auto a = address_from_string(host); a.has_value()) {
                const auto serialized = serialize(*a);
                const auto& addr =
                    out.emplace_back(api_.Factory().BlockchainAddress(
                        params.P2PDefaultProtocol(),
                        type(*a),
                        serialized.Bytes(),
                        params.P2PDefaultPort(),
                        chain_,
                        {},
                        {}));

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
    , seed_nodes_([&] {
        auto out = decltype(seed_nodes_){alloc};
        const auto json = [] {
            const auto in = network::blockchain::seednodes_json();
            auto parser = boost::json::stream_parser{};
            parser.write(in.data(), in.size());

            return parser.release();
        }();
        const auto& nodes = json.get_object().at("nodes").get_array();
        out.reserve(nodes.size());
        out.clear();

        for (const auto& seed : nodes) {
            const auto& hostname =
                seed.get_object().at("hostname").get_string();
            const auto& key = seed.get_object().at("key").get_string();
            log_()(name_)(": loaded seed node ")(hostname.c_str())(
                " with pubkey ")(key.c_str())
                .Flush();
            auto pubkey = [&] {
                auto pk = CString{alloc};
                const auto view = std::string_view{key.c_str(), key.size()};
                const auto rc =
                    api_.Crypto().Encode().Z85Decode(view, writer(pk));

                assert_true(rc);

                return pk;
            }();

            if (pubkey == api_.Network().OTDHT().CurvePublicKey()) {
                log_()(name_)(": skipping connection to self").Flush();

                continue;
            }

            out.emplace_back(hostname.c_str(), std::move(pubkey));
        }

        return out;
    }())
    , seeds_([&] {
        auto out = decltype(seeds_){alloc};
        out.clear();

        for (const auto& [host, key] : seed_nodes_) {
            auto& seed = out.try_emplace(host, Seed{}).first->second;
            auto address = network::asio::address_from_string(host);

            if (address.has_value()) {
                seed.address_ = api_.Factory().BlockchainAddressZMQ(
                    params::get(chain_).P2PDefaultProtocol(),
                    *address,
                    chain_,
                    Time{},
                    {},
                    key);
            }

            seed.last_dns_query_.emplace(std::numeric_limits<sTime>::max());
        }

        return out;
    }())
    , dns_()
    , socket_queue_(alloc)
    , asio_listeners_(alloc)
    , next_id_(invalid_peer_)
    , peers_(alloc)
    , index_(alloc)
    , active_(alloc)
    , verified_(alloc)
    , outgoing_(alloc)
    , registered_(false)
    , external_addresses_(alloc)
    , transports_(alloc)
    , database_is_ready_(false)
    , queue_(alloc)
    , dns_timer_(api_.Network().Asio().Internal().GetTimer())
    , registration_timer_(api_.Network().Asio().Internal().GetTimer())
    , startup_timer_(api_.Network().Asio().Internal().GetTimer())
{
    external_addresses_.clear();
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
    const network::blockchain::Transport type,
    const network::asio::Endpoint& endpoint,
    network::asio::Socket&& socket,
    std::shared_ptr<Actor> me) noexcept -> void

{
    assert_false(nullptr == me);

    auto address = me->api_.Factory().Internal().BlockchainAddressIncoming(
        params::get(me->chain_).P2PDefaultProtocol(),
        type,
        network::blockchain::Transport::invalid,
        endpoint.GetBytes(),
        endpoint.GetPort(),
        me->chain_,
        {},
        {},
        {});

    if (address.IsValid()) {
        me->socket_queue_.lock()->emplace_back(
            std::move(address), std::move(socket));
        me->pipeline_.Push(MakeWork(Work::statemachine));
    } else {
        LogError()()(": invalid address: ")(address.Display()).Flush();
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
    std::ranges::transform(
        index_, std::inserter(out, out.end()), [](const auto& pair) {
            return pair.first;
        });

    return out;
}

auto Actor::add_peer(
    network::blockchain::Address endpoint,
    bool incoming,
    std::optional<network::asio::Socket> asio,
    ConnectionID connection,
    std::optional<Message> msg) noexcept -> PeerID
{
    const auto& log = log_;

    assert_true(endpoint.IsValid());

    endpoint.Internal().SetIncoming(incoming);

    if (is_active(endpoint)) {
        log()(name_)(": address ")(endpoint.Display())(" is already active")
            .Flush();

        return invalid_peer_;
    } else {
        log()(name_)(": adding peer ")(endpoint.Display()).Flush();
    }

    const auto inproc = network::zeromq::MakeArbitraryInproc();
    const auto peerID = [&, this] {
        using enum network::zeromq::socket::Type;
        auto [it, isNew] = peers_.try_emplace(
            ++next_id_,
            endpoint.ID(),
            api_.Network().ZeroMQ().Internal().RawSocket(Push),
            connection,
            endpoint.Type(),
            incoming,
            get_allocator());

        assert_true(isNew);

        auto& [_1, socket, external, internal, cache, _2, _3] = it->second;
        const auto listen = socket.Bind(inproc.data());

        assert_true(listen);
        assert_true(external == connection);
        assert_false(internal.IsValid());

        if (msg.has_value()) { cache.emplace_back(std::move(*msg)); }

        return it->first;
    }();
    index_[endpoint.ID()] = peerID;

    if (false == incoming) { ++transports_[endpoint.Type()]; }

    using enum network::blockchain::Protocol;
    const auto protocol = params::get(chain_).P2PDefaultProtocol();

    switch (protocol) {
        case bitcoin: {
            factory::BlockchainPeerBitcoin(
                api_p_,
                node_p_,
                nonce_,
                peerID,
                std::move(endpoint),
                external_addresses_,
                inproc,
                std::move(asio));
        } break;
        case opentxs:
        case ethereum:
        default: {
            LogAbort()()(name_)(": unsupported network protocol ")(
                print(protocol))
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
    to_blockchain_api_.SendDeferred([&] {
        using enum WorkType;
        auto work =
            network::zeromq::tagged_message(BlockchainPeerConnected, true);
        work.AddFrame(chain_);
        work.AddFrame(active_.size());

        return work;
    }());
}

auto Actor::broadcast_verified(std::string_view address) noexcept -> void
{
    to_blockchain_api_.SendDeferred([&] {
        using enum WorkType;
        auto work = network::zeromq::tagged_message(BlockchainPeerAdded, true);
        work.AddFrame(chain_);
        work.AddFrame(address.data(), address.size());
        work.AddFrame(verified_.size());

        return work;
    }());
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

auto Actor::check_database(allocator_type monotonic) noexcept -> bool
{
    if (database_is_ready_) { return true; }

    database_is_ready_ = db_.PeerIsReady();

    if (database_is_ready_) {
        first_time_init(monotonic);

        while (false == queue_.empty()) {
            auto& [work, msg] = queue_.front();
            pipeline_otdht(work, std::move(msg), monotonic);
            queue_.pop_front();
        }

        return true;
    } else {

        return false;
    }
}

auto Actor::check_dns() noexcept -> void
{
    if (dns_.NeedQuery()) { send_dns_query(); }
}

auto Actor::check_peers(allocator_type monotonic) noexcept -> void
{
    const auto& log = log_;

    if (false == have_target_zmq_peers()) {
        log()(name_)(": attempting to add a zmq peer").Flush();

        if (auto peer = get_peer(true, monotonic); peer.IsValid()) {
            add_peer(std::move(peer), false);
        } else {
            log()(name_)(": no valid zmq peer not found").Flush();
        }
    }

    if (false == have_target_peers()) {
        log()(name_)(": attempting to add a peer on any network").Flush();

        if (auto peer = get_peer(false, monotonic); peer.IsValid()) {
            add_peer(std::move(peer), false);
        } else {
            log()(name_)(": no valid peer not found").Flush();
        }
    }
}

auto Actor::check_registration() noexcept -> void
{
    if (false == registered_) {
        using enum network::otdht::NodeJob;
        to_otdht_.SendDeferred([this] {
            auto out = MakeWork(connect_peer_manager);
            out.AddFrame(chain_);

            return out;
        }());
        reset_registration_timer();
    }
}

auto Actor::check_seeds() noexcept -> void
{
    const auto now = sClock::now();

    for (auto& [host, seed] : seeds_) {
        if (seed.RetryDNS(now)) {
            log_()(name_)(": resolving seed peer ")(host).Flush();
            pipeline_.Internal().SendFromThread([&]() {
                auto out = MakeWork(WorkType::AsioResolve);
                out.AddFrame(host.data(), host.size());
                out.AddFrame(network::blockchain::otdht_listen_port_);

                return out;
            }());
            seed.last_dns_query_.emplace(now);
        }
    }
}

auto Actor::do_shutdown() noexcept -> void
{
    using enum network::otdht::NodeJob;
    to_otdht_.SendDeferred([this] {
        auto out = MakeWork(disconnect_peer_manager);
        out.AddFrame(chain_);

        return out;
    }());

    dns_timer_.Cancel();

    for (const auto& endpoint : asio_listeners_) {
        api_.Network().Asio().Internal().Close(endpoint);
    }

    for (auto& [id, data] : peers_) {
        using enum WorkType;
        data.socket_.SendDeferred(MakeWork(Shutdown));
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

    do_work(monotonic);

    return false;
}

auto Actor::first_time_init(allocator_type monotonic) noexcept -> void
{
    const auto& params = params::get(chain_);
    using enum network::blockchain::Transport;
    using namespace network::asio;

    for (const auto& v4addr : api_.GetOptions().BlockchainBindIpv4()) {
        try {
            const auto boost = address_from_string(v4addr);

            if (false == boost.has_value()) {
                const auto error = UnallocatedCString(v4addr).append(
                    " is not a valid address");

                throw std::runtime_error{error};
            }

            if (false == boost->is_v4()) {
                const auto error = UnallocatedCString(v4addr).append(
                    " is not a valid ipv4 address");

                throw std::runtime_error{error};
            }

            const auto addr = serialize(*boost);
            auto address = api_.Factory().BlockchainAddress(
                params.P2PDefaultProtocol(),
                type(*boost),
                addr.Bytes(),
                params::get(chain_).P2PDefaultPort(),
                chain_,
                {},
                {});
            listen(address, monotonic);
        } catch (const std::exception& e) {
            LogError()()(e.what()).Flush();

            continue;
        }
    }

    for (const auto& v6addr : api_.GetOptions().BlockchainBindIpv6()) {
        try {
            const auto boost = address_from_string(v6addr);

            if (false == boost.has_value()) {
                const auto error = UnallocatedCString(v6addr).append(
                    " is not a valid address");

                throw std::runtime_error{error};
            }

            if (false == boost->is_v6()) {
                const auto error = UnallocatedCString(v6addr).append(
                    " is not a valid ipv6 address");

                throw std::runtime_error{error};
            }

            const auto addr = serialize(*boost);
            auto address = api_.Factory().BlockchainAddress(
                params.P2PDefaultProtocol(),
                type(*boost),
                addr.Bytes(),
                params::get(chain_).P2PDefaultPort(),
                chain_,
                {},
                {});
            listen(address, monotonic);
        } catch (const std::exception& e) {
            LogError()()(e.what()).Flush();

            continue;
        }
    }

    for (const auto& peer : command_line_peers_) { db_.AddOrUpdate(peer); }

    if (false == api_.GetOptions().TestMode()) {
        const auto now = sClock::now();

        for (const auto& [host, key] : seed_nodes_) {
            auto boost = address_from_string(host);
            auto& seed = seeds_[host];

            if (boost.has_value()) {
                auto& addr = seed.address_;
                addr = api_.Factory().BlockchainAddressZMQ(
                    params::get(chain_).P2PDefaultProtocol(),
                    type(*boost),
                    serialize(*boost).Bytes(),
                    chain_,
                    Time{},
                    {},
                    key);

                if (addr.IsValid()) {
                    log_()(name_)(": added peer from hardcoded seed list: ")(
                        addr.Display())
                        .Flush();
                    db_.AddOrUpdate(addr);
                    add_peer(addr, false);
                }
            } else {
                log_()(name_)(": resolving seed peer ")(host).Flush();
                pipeline_.Internal().SendFromThread([&]() {
                    auto out = MakeWork(WorkType::AsioResolve);
                    out.AddFrame(host.data(), host.size());
                    out.AddFrame(network::blockchain::otdht_listen_port_);

                    return out;
                }());
                seed.last_dns_query_.emplace(now);
            }
        }
    }
}

auto Actor::get_peer(bool zmqOnly, allocator_type monotonic) noexcept
    -> network::blockchain::Address
{
    const auto& log = log_;
    auto peer = opentxs::network::blockchain::Address{};
    using enum opentxs::network::blockchain::Transport;
    const auto& count = transports_[zmq];
    const auto protocol = params::get(chain_).P2PDefaultProtocol();
    const auto transports = usable_networks(monotonic);
    const auto selfKey = api_.Network().OTDHT().CurvePublicKey();
    auto exclude = active_addresses(monotonic);

    // attempt to maintain at least one zmq connection at all times
    while ((count < zmq_peer_target_) && (false == peer.IsValid())) {
        static const auto z = Set<network::blockchain::Transport>{zmq};
        static const auto noServices = decltype(preferred_services_){};
        peer = db_.Get(protocol, z, noServices, exclude);

        if (peer.IsValid()) {
            if (peer.Key() == selfKey) {
                log()(name_)(": avoiding connection to self").Flush();
                exclude.emplace(peer.ID());
            } else if (
                (zmq == peer.Type()) &&
                (false == transports.contains(peer.Subtype()))) {
                log()(name_)(
                    ": ignoring zmq peer due to unsupported transport ")(
                    print(peer.Subtype()))
                    .Flush();
                exclude.emplace(peer.ID());
            } else {
                log()(name_)(": attempting to connect to zmq peer ")(
                    peer.Display())
                    .Flush();

                return peer;
            }
        } else {

            break;
        }
    }

    if (0_uz == count) {
        log()(name_)(": did not find a suitable zmq peer in database").Flush();
    }

    if (zmqOnly) { return {}; }

    while (false == peer.IsValid()) {
        peer = db_.Get(protocol, transports, preferred_services_, exclude);

        if (peer.IsValid()) {
            if (peer.Key() == selfKey) {
                exclude.emplace(peer.ID());
            } else {

                return peer;
            }
        } else {

            break;
        }
    }

    return {};
}

auto Actor::have_target_peers() const noexcept -> bool
{
    return outgoing_.size() >= peer_target_;
}

auto Actor::have_target_zmq_peers() const noexcept -> bool
{
    using enum network::blockchain::Transport;

    if (auto i = transports_.find(zmq); transports_.end() != i) {

        return i->second >= zmq_peer_target_;
    } else {

        return false;
    }
}

auto Actor::is_active(const network::blockchain::Address& addr) const noexcept
    -> bool
{
    return index_.contains(addr.ID());
}

auto Actor::listen(
    const network::blockchain::Address& address,
    allocator_type monotonic) noexcept -> void
{
    if (false == address.IsValid()) {
        LogError()()("invalid address: ")(address.Display()).Flush();

        return;
    }

    using enum network::blockchain::Transport;
    const auto type = address.Type();

    switch (type) {
        case ipv6:
        case ipv4: {
            listen_tcp(address);
        } break;
        case onion2:
        case onion3:
        case eep:
        case cjdns:
        case zmq:
        default: {
            LogError()()(name_)(": unsupported network type ")(print(type))
                .Flush();
        }
    }
}

auto Actor::listen_tcp(const network::blockchain::Address& address) noexcept
    -> void
{
    try {
        const auto type = address.Type();
        const auto port = address.Port();
        const auto addr = address.Bytes();
        const auto& endpoint = [&]() -> auto& {
            using enum network::blockchain::Transport;
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
        }();
        const auto accepted = api_.Network().Asio().Internal().Accept(
            endpoint, [=, me = me_](auto&& socket) {
                accept(type, endpoint, std::move(socket), me);
            });

        if (accepted) {
            log_()(name_)(": listening for incoming connections on ")(
                address.Display())
                .Flush();
            external_addresses_.emplace(address);
            to_peers_.SendDeferred([&] {
                using enum network::blockchain::PeerJob;
                auto out = MakeWork(gossip_address);
                const auto proto = [&] {
                    auto p = proto::BlockchainPeerAddress{};
                    address.Internal().Serialize(p);

                    return p;
                }();
                proto::write(proto, out.AppendBytes());

                return out;
            }());
        } else {
            LogError()()(name_)(": unable to bind to ")(address.Display())
                .Flush();
            asio_listeners_.pop_back();
        }
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();
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
    const auto socket = connection_id(msg);

    if (socket == otdht_id_) {
        pipeline_otdht(work, std::move(msg), monotonic);
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
        case disconnect:
        case addpeer:
        case addlistener:
        case verifypeer:
        case spawn_peer:
        case register_ack:
        case gossip_address:
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

auto Actor::pipeline_otdht(
    const Work work,
    Message&& msg,
    allocator_type monotonic) noexcept -> void
{
    using enum PeerManagerJobs;

    switch (work) {
        case spawn_peer: {
            process_spawn_peer(std::move(msg), monotonic);
        } break;
        case register_ack: {
            process_register_ack(std::move(msg));
        } break;
        case gossip_address: {
            process_gossip_address(std::move(msg));
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
            unhandled_type(work);
        }
        default: {
            unknown_type(work);
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
        case gossip_address: {
            process_gossip_address(std::move(msg));
        } break;
        case broadcasttx: {
            process_broadcasttx(std::move(msg));
        } break;
        case report: {
            process_report(std::move(msg));
        } break;
        case shutdown:
        case resolve:
        case spawn_peer:
        case register_ack:
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
    const auto body = msg.Payload();
    assert_true(1 < body.size());

    auto address = api_.Factory().Internal().Session().BlockchainAddress(
        proto::Factory<proto::BlockchainPeerAddress>(body[1]));

    if (address.IsValid()) {
        listen(address, monotonic);
    } else {
        LogError()()(name_)(": invalid address").Flush();
    }
}

auto Actor::process_addpeer(Message&& msg) noexcept -> void
{
    const auto body = msg.Payload();
    assert_true(1 < body.size());

    auto address = api_.Factory().Internal().Session().BlockchainAddress(
        proto::Factory<proto::BlockchainPeerAddress>(body[1]));

    if (address.IsValid()) {
        add_peer(std::move(address), false);
    } else {
        LogError()()(name_)(": invalid address").Flush();
    }
}

auto Actor::process_broadcasttx(Message&& msg) noexcept -> void
{
    broadcast_tx_.SendDeferred(std::move(msg));
}

auto Actor::process_disconnect(Message&& msg) noexcept -> void
{
    const auto body = msg.Payload();

    assert_true(2_uz < body.size());

    process_disconnect(body[1].as<PeerID>(), body[2].Bytes());
}

auto Actor::process_disconnect(PeerID id, std::string_view display) noexcept
    -> void
{
    if (auto i = peers_.find(id); peers_.end() != i) {
        log_()(name_)(": disconnecting peer ")(id);
        {
            auto& [address, socket, external, internal, _, transport, incoming] =
                i->second;

            if (false == incoming) { --transports_[transport]; }

            index_.erase(address);
            socket.Stop();
        }
        peers_.erase(i);
        active_.erase(id);
        verified_.erase(id);
        outgoing_.erase(id);
        broadcast_verified(display);
        broadcast_active();
    } else {
        log_()(name_)(": peer ")(id)(" not found").Flush();
    }
}

auto Actor::process_gossip_address(Message&& msg) noexcept -> void
{
    const auto& log = log_;
    const auto payload = msg.Payload();

    if (1_uz == payload.size()) { return; }

    auto addresses = payload.subspan(1_uz);
    using enum network::blockchain::PeerJob;
    auto message = MakeWork(gossip_address);
    auto good = 0_uz;

    for (auto& frame : addresses) {
        auto addr = api_.Factory().Internal().Session().BlockchainAddress(
            proto::Factory<proto::BlockchainPeerAddress>(frame));

        if (addr.IsValid()) {
            log()(name_)(": adding ")(addr.Display())(" on ")(
                print(addr.Type()))(" to list of external endpoints to gossip")
                .Flush();
            external_addresses_.emplace(std::move(addr));
            message.AddFrame(std::move(frame));
            ++good;
        } else {
            LogError()()(name_)(": ignoring invalid address").Flush();
        }
    }

    if (0_uz < good) {
        to_peers_.SendDeferred(std::move(message));
        log()(name_)(": broadcasting ")(good)(" external endpoints to peers")
            .Flush();
    }
}

auto Actor::process_register_ack(Message&& msg) noexcept -> void
{
    registered_ = true;
    process_gossip_address(std::move(msg));
}

auto Actor::process_report(Message&&) noexcept -> void
{
    broadcast_verified();
    broadcast_active();
}

auto Actor::process_resolve(Message&& msg) noexcept -> void
{
    const auto& log = log_;
    static constexpr auto success = std::byte{0x01};
    const auto body = msg.Payload();

    assert_true(3 < body.size());

    dns_.GotResponse();
    const auto query = CString{body[2].Bytes(), get_allocator()};
    const auto port = body[3].as<std::uint16_t>();

    if (body[1].as<std::byte>() == success) {
        for (auto i = std::next(body.begin(), 4), end = body.end(); i != end;
             ++i) {
            const auto& frame = *i;
            const auto network = [&] {
                static constexpr auto v4Bytes =
                    sizeof(boost::asio::ip::address_v4::bytes_type);
                static constexpr auto v6Bytes =
                    sizeof(boost::asio::ip::address_v6::bytes_type);
                const auto size = frame.size();
                using enum network::blockchain::Transport;

                switch (size) {
                    case v4Bytes: {

                        return ipv4;
                    }
                    case v6Bytes: {

                        return ipv6;
                    }
                    default: {
                        LogAbort()()(name_)(": invalid address size (")(
                            size)(")")
                            .Abort();
                    }
                }
            }();

            if (auto j = seeds_.find(query); seeds_.end() != j) {
                const auto& key = [&]() -> const auto& {
                    for (const auto& [host, pubkey] : seed_nodes_) {
                        if (host == query) { return pubkey; }
                    }

                    LogAbort()().Abort();
                }();
                auto& seed = j->second;
                seed.last_dns_query_.reset();
                auto& addr = seed.address_;
                addr = api_.Factory().BlockchainAddressZMQ(
                    params::get(chain_).P2PDefaultProtocol(),
                    network,
                    frame.Bytes(),
                    chain_,
                    Time{},
                    {},
                    key);

                if (addr.IsValid()) {
                    log()(name_)(": resolved zmq seed ")(query)(":")(
                        port)(" to ")(addr.Display())
                        .Flush();
                    db_.AddOrUpdate(addr);
                    add_peer(addr, false);
                }
            } else {
                auto addr = api_.Factory().BlockchainAddress(
                    params::get(chain_).P2PDefaultProtocol(),
                    network,
                    frame.Bytes(),
                    port,
                    chain_,
                    Time{},
                    {});

                if (addr.IsValid()) {
                    log()(name_)(": resolved dns seed ")(query)(":")(
                        port)(": to ")(addr.Display())
                        .Flush();

                    db_.AddOrUpdate(std::move(addr));
                }
            }
        }
    } else {
        assert_true(4 < body.size());

        log()(name_)(": error while resolving ")(query)(":")(port)(": ")(
            body[4].Bytes())
            .Flush();
    }
}

auto Actor::process_spawn_peer(Message&& msg, allocator_type monotonic) noexcept
    -> void
{
    if (database_is_ready_) {
        const auto payload = msg.Payload();

        assert_true(3_uz < payload.size());

        const auto cookie = payload[1].Bytes();
        const auto subtype = payload[2].as<network::blockchain::Transport>();
        const auto endpoint = payload[3].Bytes();
        const auto name =
            CString{monotonic}.append(endpoint).append(" #").append(
                std::to_string(payload[1].as<std::uint64_t>()));
        const auto peer = add_peer(
            api_.Factory().Internal().BlockchainAddressIncoming(
                params::get(chain_).P2PDefaultProtocol(),
                network::blockchain::Transport::zmq,
                subtype,
                name,
                network::blockchain::otdht_listen_port_,
                chain_,
                {},
                {},
                cookie),
            true,
            std::nullopt,
            std::move(msg).Envelope(),
            std::nullopt);

        assert_true(invalid_peer_ != peer);
    } else {
        queue_.emplace_back(Work::spawn_peer, std::move(msg));
    }
}

auto Actor::process_verify(Message&& msg) noexcept -> void
{
    const auto body = msg.Payload();

    assert_true(2_uz < body.size());

    process_verify(body[1].as<PeerID>(), body[2].Bytes());
}

auto Actor::process_verify(PeerID id, std::string_view display) noexcept -> void
{
    verified_.insert(active_.extract(id));
    log_()(name_)(": peer ")(id)(" reached verified state").Flush();
    broadcast_verified(display);
}

auto Actor::reset_dns_timer() noexcept -> void
{
    reset_timer(dns_.timeout_, dns_timer_, Work::statemachine);
}

auto Actor::reset_registration_timer() noexcept -> void
{
    reset_timer(registration_timeout_, registration_timer_, Work::statemachine);
}

auto Actor::reset_startup_timer() noexcept -> void
{
    reset_timer(1s, startup_timer_, Work::statemachine);
}

auto Actor::send_dns_query() noexcept -> void
{
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
    -> Set<network::blockchain::Transport>
{
    using enum network::blockchain::Transport;
    using enum ConnectionMode;
    using enum std::future_status;
    auto output = Set<network::blockchain::Transport>{monotonic};
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
    if (check_database(monotonic)) {
        check_registration();
        check_command_line_peers();
        check_dns();

        if (false == api_.GetOptions().TestMode()) { check_seeds(); }

        check_peers(monotonic);
        accept_asio();

        return need_peers();
    } else {
        reset_startup_timer();

        return false;
    }
}

Actor::~Actor() = default;
}  // namespace opentxs::blockchain::node::peermanager
