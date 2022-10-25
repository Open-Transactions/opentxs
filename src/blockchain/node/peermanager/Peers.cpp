// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "blockchain/node/peermanager/Peers.hpp"  // IWYU pragma: associated

#include <boost/system/system_error.hpp>
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <compare>
#include <future>
#include <iterator>
#include <memory>
#include <random>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "BoostAsio.hpp"
#include "IncomingConnectionManager.hpp"
#include "internal/api/session/Endpoints.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/database/Peer.hpp"
#include "internal/blockchain/node/Config.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/PeerManager.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "internal/network/blockchain/Peer.hpp"
#include "internal/network/blockchain/bitcoin/Factory.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/SocketType.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
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
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/network/asio/Socket.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ZeroMQ.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/util/BlockchainProfile.hpp"
#include "opentxs/util/ConnectionMode.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Options.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/ScopeGuard.hpp"
#include "util/Work.hpp"

namespace opentxs::blockchain::node::peermanager
{
std::atomic<int> Peers::next_id_{0};

Peers::Peers(
    const api::Session& api,
    const internal::Config& config,
    const node::Manager& node,
    database::Peer& database,
    internal::PeerManager& parent,
    const node::Endpoints& endpoints,
    const Type chain,
    const std::string_view seednode) noexcept
    : chain_(chain)
    , api_(api)
    , config_(config)
    , node_(node)
    , database_(database)
    , parent_(parent)
    , to_blockchain_api_([&] {
        using Type = opentxs::network::zeromq::socket::Type;
        auto out = api.Network().ZeroMQ().Internal().RawSocket(Type::Push);
        const auto endpoint = UnallocatedCString{
            api.Endpoints().Internal().Internal().BlockchainMessageRouter()};
        const auto rc = out.Connect(endpoint.c_str());

        OT_ASSERT(rc);

        return out;
    }())
    , endpoints_(endpoints)
    , invalid_peer_(false)
    , localhost_peer_(api_.Factory().DataFromHex("0x7f000001"))
    , default_peer_(set_default_peer(
          seednode,
          localhost_peer_,
          const_cast<bool&>(invalid_peer_)))
    , preferred_services_(get_preferred_services(config_))
    , nonce_([&] {
        auto out = decltype(nonce_){0};
        const auto rc = api.Crypto().Util().RandomizeMemory(&out, sizeof(out));

        OT_ASSERT(rc);

        return out;
    }())
    , minimum_peers_([&]() -> std::size_t {
        static const auto test = api.Factory().DataFromHex("0x7f000002");

        if (default_peer_ == test) { return 0_uz; }
        if (api_.GetOptions().TestMode()) { return 0_uz; }

        return config_.PeerTarget(chain_);
    }())
    , peers_()
    , active_()
    , count_()
    , connected_()
    , incoming_zmq_()
    , incoming_tcp_()
    , attempt_()
    , resolved_dns_()  // TODO allocator
    , gatekeeper_()
{
    const auto& data = params::get(chain_);
    database_.AddOrUpdate(p2p::Address{factory::BlockchainAddress(
        api_,
        data.P2PDefaultProtocol(),
        p2p::Network::ipv4,
        default_peer_,
        data.P2PDefaultPort(),
        chain_,
        Time{},
        {},
        false)});
}

auto Peers::add_peer(p2p::Address&& endpoint) noexcept -> int
{
    auto ticket = gatekeeper_.get();

    if (ticket) { return -1; }

    return add_peer(++next_id_, std::move(endpoint));
}

auto Peers::add_peer(const int id, p2p::Address&& endpoint) noexcept -> int
{
    auto addressID{endpoint.ID()};
    auto& count = active_[addressID];

    if (0_uz != count) { return -1; }

    const auto inproc = network::zeromq::MakeArbitraryInproc();
    using SocketType = network::zeromq::socket::Type;
    const auto [it, added] = peers_.try_emplace(
        id,
        addressID,
        api_.Network().ZeroMQ().Internal().RawSocket(SocketType::Push));

    OT_ASSERT(added);

    auto& [address, socket] = it->second;
    const auto listen = socket.Bind(inproc.data());

    OT_ASSERT(listen);

    auto api = api_.Internal().GetShared();
    auto node = node_.Internal().GetShared();

    if (api && node) {
        peer_factory(
            std::move(api), std::move(node), id, inproc, std::move(endpoint))
            .Start();
        ++count;
        adjust_count(1);
        attempt_[addressID] = Clock::now();
        connected_.emplace(std::move(addressID));

        return id;
    } else {

        return -1;
    }
}

auto Peers::adjust_count(int adjustment) noexcept -> void
{
    if (0 < adjustment) {
        ++count_;
    } else if (0 > adjustment) {
        --count_;
    } else {
        count_.store(0);
    }

    to_blockchain_api_.SendDeferred(
        [&] {
            auto work = network::zeromq::tagged_message(
                WorkType::BlockchainPeerConnected);
            work.AddFrame(chain_);
            work.AddFrame(count_.load());

            return work;
        }(),
        __FILE__,
        __LINE__);
}

auto Peers::AddIncoming(const int id, p2p::Address&& endpoint) noexcept -> void
{
    endpoint.Internal().SetIncoming(true);
    add_peer(id, std::move(endpoint));
}

auto Peers::AddListener(p2p::Address&& address) noexcept -> void
{
    switch (address.Type()) {
        case p2p::Network::zmq: {
            auto& manager = incoming_zmq_;

            if (false == bool(manager)) {
                manager = IncomingConnectionManager::ZMQ(api_, *this);
            }

            OT_ASSERT(manager);

            manager->Listen(std::move(address));
        } break;
        case p2p::Network::ipv6:
        case p2p::Network::ipv4: {
            auto& manager = incoming_tcp_;

            if (false == bool(manager)) {
                manager = IncomingConnectionManager::TCP(api_, *this);
            }

            OT_ASSERT(manager);

            manager->Listen(std::move(address));
        } break;
        case p2p::Network::onion2:
        case p2p::Network::onion3:
        case p2p::Network::eep:
        case p2p::Network::cjdns:
        default: {
            LogError()(OT_PRETTY_CLASS())("unsupported network type ")(
                print(address.Type()))
                .Flush();
        }
    }
}

auto Peers::AddPeer(p2p::Address&& address) noexcept -> void
{
    auto ticket = gatekeeper_.get();

    if (ticket) { return; }

    if (auto chain = address.Chain(); chain != chain_) {
        LogError()(OT_PRETTY_CLASS())("wrong chain: ")(print(chain)).Flush();

        return;
    }

    address.Internal().SetIncoming(false);
    add_peer(std::move(address));
}

auto Peers::AddResolvedDNS(Vector<p2p::Address> address) noexcept -> void
{
    std::move(
        address.begin(), address.end(), std::back_inserter(resolved_dns_));
}

auto Peers::ConstructPeer(const p2p::Address& endpoint) noexcept -> int
{
    auto id = ++next_id_;
    parent_.AddIncomingPeer(id, endpoint);

    return id;
}

auto Peers::Disconnect(const int id) noexcept -> void
{
    if (auto it = peers_.find(id); peers_.end() != it) {
        if (incoming_zmq_) { incoming_zmq_->Disconnect(id); }

        if (incoming_tcp_) { incoming_tcp_->Disconnect(id); }

        const auto address = [&] {
            auto& [address, socket] = it->second;
            auto out{address};
            socket.SendDeferred(
                MakeWork(WorkType::Shutdown), __FILE__, __LINE__, true);
            peers_.erase(it);

            return out;
        }();
        --active_.at(address);
        adjust_count(-1);
        connected_.erase(address);
    }
}

auto Peers::get_default_peer() const noexcept -> p2p::Address
{
    if (localhost_peer_ == default_peer_) { return {}; }

    const auto& data = params::get(chain_);

    return p2p::Address{factory::BlockchainAddress(
        api_,
        data.P2PDefaultProtocol(),
        p2p::Network::ipv4,
        default_peer_,
        data.P2PDefaultPort(),
        chain_,
        Time{},
        {},
        false)};
}

auto Peers::get_dns_peer() noexcept -> p2p::Address
{
    if (api_.GetOptions().TestMode()) { return {}; }

    try {
        while (false == resolved_dns_.empty()) {
            auto post = ScopeGuard{[&] { resolved_dns_.pop_front(); }};
            auto out = std::move(resolved_dns_.front());

            if (out.IsValid()) {
                database_.AddOrUpdate({out});

                if (previous_failure_timeout(out.ID())) {
                    LogVerbose()(OT_PRETTY_CLASS())("Skipping ")(print(chain_))(
                        " peer ")(out.Display())(" due to retry timeout")
                        .Flush();

                    continue;
                } else {

                    return out;
                }
            }
        }

        const auto& data = params::get(chain_);
        const auto& dns = data.P2PSeeds();

        if (0 == dns.size()) {
            LogVerbose()(OT_PRETTY_CLASS())("No dns seeds available").Flush();

            return {};
        }

        auto seeds = UnallocatedVector<std::string_view>{};
        const auto count = 1_uz;
        std::sample(
            std::begin(dns),
            std::end(dns),
            std::back_inserter(seeds),
            count,
            std::mt19937{std::random_device{}()});

        if (0 == seeds.size()) {
            LogError()(OT_PRETTY_CLASS())("Failed to select a dns seed")
                .Flush();

            return {};
        }

        const auto& seed = *seeds.cbegin();

        if (seed.empty()) {
            LogError()(OT_PRETTY_CLASS())("Invalid dns seed").Flush();

            return {};
        }

        const auto port = data.P2PDefaultPort();
        LogVerbose()(OT_PRETTY_CLASS())("Resolving dns seed: ")(seed).Flush();
        parent_.Resolve(seed, port);

        return {};
    } catch (const boost::system::system_error& e) {
        LogDebug()(OT_PRETTY_CLASS())(e.what()).Flush();

        return {};
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("No dns seeds defined").Flush();

        return {};
    }
}

auto Peers::get_fallback_peer(const p2p::Protocol protocol) const noexcept
    -> p2p::Address
{
    return database_.Get(protocol, get_types(), {});
}

auto Peers::get_peer() noexcept -> p2p::Address
{
    const auto protocol = params::get(chain_).P2PDefaultProtocol();
    auto address = get_default_peer();

    if (address.IsValid()) {
        LogVerbose()(OT_PRETTY_CLASS())("Default peer is: ")(address.Display())
            .Flush();

        if (is_not_connected(address)) {
            LogVerbose()(OT_PRETTY_CLASS())(
                "Attempting to connect to default peer ")(address.Display())
                .Flush();

            return address;
        } else {
            LogVerbose()(OT_PRETTY_CLASS())(
                "Already connected / connecting to default "
                "peer ")(address.Display())
                .Flush();
        }
    } else {
        LogVerbose()(OT_PRETTY_CLASS())("No default peer").Flush();
    }

    address = get_preferred_peer(protocol);

    if (address.IsValid() && is_not_connected(address)) {
        LogVerbose()(OT_PRETTY_CLASS())(
            "Attempting to connect to preferred peer: ")(address.Display())
            .Flush();

        return address;
    }

    address = get_dns_peer();

    if (address.IsValid() && is_not_connected(address)) {
        LogVerbose()(OT_PRETTY_CLASS())("Attempting to connect to dns peer: ")(
            address.Display())
            .Flush();

        return address;
    }

    address = get_fallback_peer(protocol);

    if (address.IsValid()) {
        LogVerbose()(OT_PRETTY_CLASS())(
            "Attempting to connect to fallback peer: ")(address.Display())
            .Flush();
    }

    return address;
}

auto Peers::get_preferred_peer(const p2p::Protocol protocol) const noexcept
    -> p2p::Address
{
    auto output = database_.Get(protocol, get_types(), preferred_services_);

    if (output.IsValid() && (output.Bytes() == localhost_peer_)) {
        LogVerbose()(OT_PRETTY_CLASS())("Skipping localhost as preferred peer")
            .Flush();

        return {};
    }

    if (output.IsValid() && previous_failure_timeout(output.ID())) {
        LogVerbose()(OT_PRETTY_CLASS())("Skipping ")(print(chain_))(" peer ")(
            output.Display())(" due to retry timeout")
            .Flush();

        return {};
    }

    return output;
}

auto Peers::get_preferred_services(const internal::Config& config) noexcept
    -> UnallocatedSet<p2p::Service>
{
    auto out = UnallocatedSet<p2p::Service>{};

    switch (config.profile_) {
        case BlockchainProfile::desktop_native: {
            out.emplace(p2p::Service::CompactFilters);
        } break;
        case BlockchainProfile::mobile:
        case BlockchainProfile::desktop:
        case BlockchainProfile::server: {
        } break;
        default: {
            OT_FAIL;
        }
    }

    return out;
}

auto Peers::get_types() const noexcept -> UnallocatedSet<p2p::Network>
{
    using Type = p2p::Network;
    using Mode = ConnectionMode;
    auto output = UnallocatedSet<p2p::Network>{};

    switch (api_.GetOptions().Ipv4ConnectionMode()) {
        case Mode::off: {
            output.erase(Type::ipv4);
        } break;
        case Mode::on: {
            output.insert(Type::ipv4);
        } break;
        case Mode::automatic:
        default: {
            auto ipv4data = api_.Network().Asio().GetPublicAddress4().get();

            if (!ipv4data.empty()) { output.insert(Type::ipv4); }
        }
    }

    switch (api_.GetOptions().Ipv6ConnectionMode()) {
        case Mode::off: {
            output.erase(Type::ipv6);
        } break;
        case Mode::on: {
            output.insert(Type::ipv6);
        } break;
        case Mode::automatic:
        default: {
            auto ipv6data = api_.Network().Asio().GetPublicAddress6().get();

            if (!ipv6data.empty()) { output.insert(Type::ipv6); }
        }
    }

    static auto first{true};

    if (first && (0u == output.size())) {
        LogError()(OT_PRETTY_CLASS())(
            "No outgoing connection methods available")
            .Flush();
        first = false;
    }

    return output;
}

auto Peers::is_not_connected(const p2p::Address& endpoint) const noexcept
    -> bool
{
    return 0 == connected_.count(endpoint.ID());
}

auto Peers::LookupIncomingSocket(const int id) noexcept(false)
    -> opentxs::network::asio::Socket
{
    if (!incoming_tcp_) {
        throw std::runtime_error{"TCP connection manager not instantiated"};
    }

    return incoming_tcp_->LookupIncomingSocket(id);
}

auto Peers::peer_factory(
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const opentxs::blockchain::node::Manager> network,
    const int id,
    std::string_view inproc,
    p2p::Address endpoint) noexcept -> Peer
{
    switch (params::get(chain_).P2PDefaultProtocol()) {
        case p2p::Protocol::bitcoin: {
            return factory::BlockchainPeerBitcoin(
                std::move(api),
                std::move(network),
                id,
                std::move(endpoint),
                endpoints_,
                inproc);
        }
        case p2p::Protocol::opentxs:
        case p2p::Protocol::ethereum:
        default: {
            OT_FAIL;
        }
    }
}

auto Peers::previous_failure_timeout(
    const identifier::Generic& addressID) const noexcept -> bool
{
    static constexpr auto timeout = std::chrono::minutes{10};

    if (const auto it = attempt_.find(addressID); attempt_.end() == it) {

        return false;
    } else {
        const auto& last = it->second;

        return (Clock::now() - last) < timeout;
    }
}

auto Peers::set_default_peer(
    const std::string_view node,
    const Data& localhost,
    bool& invalidPeer) noexcept -> ByteArray
{
    if (false == node.empty()) {
        try {
            const auto bytes = ip::make_address_v4(node).to_bytes();

            return {bytes.data(), bytes.size()};
        } catch (...) {
            invalidPeer = true;
        }
    }

    return localhost;
}

auto Peers::Run() noexcept -> bool
{
    auto ticket = gatekeeper_.get();

    if (ticket || invalid_peer_) { return false; }

    const auto target = minimum_peers_.load();

    if (target > peers_.size()) {
        LogVerbose()(OT_PRETTY_CLASS())("Fewer peers (")(peers_.size())(
            ") than desired (")(target)(")")
            .Flush();
        auto peer = get_peer();

        if (peer.IsValid()) { add_peer(std::move(peer)); }
    }

    return target > peers_.size();
}

auto Peers::Shutdown() noexcept -> void
{
    gatekeeper_.shutdown();

    if (incoming_zmq_) { incoming_zmq_->Shutdown(); }
    if (incoming_tcp_) { incoming_tcp_->Shutdown(); }

    for (auto& [id, data] : peers_) {
        auto& [address, socket] = data;
        socket.SendDeferred(MakeWork(WorkType::Shutdown), __FILE__, __LINE__);
    }

    peers_.clear();
    adjust_count(0);
    active_.clear();
}

Peers::~Peers() = default;
}  // namespace opentxs::blockchain::node::peermanager
