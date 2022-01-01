// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/peermanager/PeerManager.hpp"  // IWYU pragma: associated

#include <boost/asio.hpp>
#include <boost/system/system_error.hpp>
#include <algorithm>
#include <array>
#include <atomic>
#include <chrono>
#include <iterator>
#include <map>
#include <memory>
#include <random>
#include <stdexcept>
#include <utility>
#include <vector>

#include "IncomingConnectionManager.hpp"
#include "internal/api/network/Blockchain.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "internal/blockchain/p2p/bitcoin/Factory.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/network/Asio.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/p2p/Address.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/network/asio/Endpoint.hpp"
#include "opentxs/network/zeromq/message/Message.tpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Options.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs::blockchain::node::implementation
{
PeerManager::Peers::Peers(
    const api::Session& api,
    const internal::Config& config,
    const node::internal::Mempool& mempool,
    const internal::Network& node,
    const HeaderOracle& headers,
    const internal::FilterOracle& filter,
    const internal::BlockOracle& block,
    const internal::PeerDatabase& database,
    const internal::PeerManager& parent,
    const database::BlockStorage policy,
    const std::atomic<bool>& running,
    const std::string& shutdown,
    const Type chain,
    const std::string& seednode,
    const std::size_t peerTarget) noexcept
    : chain_(chain)
    , api_(api)
    , config_(config)
    , mempool_(mempool)
    , node_(node)
    , headers_(headers)
    , filter_(filter)
    , block_(block)
    , database_(database)
    , parent_(parent)
    , connected_peers_(api_.Network().Blockchain().Internal().PeerUpdate())
    , policy_(policy)
    , running_(running)
    , shutdown_endpoint_(shutdown)
    , invalid_peer_(false)
    , localhost_peer_(api_.Factory().Data("0x7f000001", StringStyle::Hex))
    , default_peer_(set_default_peer(
          seednode,
          localhost_peer_,
          const_cast<bool&>(invalid_peer_)))
    , preferred_services_(get_preferred_services(config_))
    , next_id_(0)
    , minimum_peers_([&]() -> std::size_t {
        static const auto test =
            api.Factory().Data("0x7f000002", StringStyle::Hex);

        if (default_peer_ == test) { return 1u; }

        return peerTarget;
    }())
    , peers_()
    , active_()
    , count_()
    , connected_()
    , incoming_zmq_()
    , incoming_tcp_()
    , attempt_()
{
    const auto& data = params::Data::Chains().at(chain_);
    database_.AddOrUpdate(Endpoint{factory::BlockchainAddress(
        api_,
        data.p2p_protocol_,
        p2p::Network::ipv4,
        default_peer_,
        data.default_port_,
        chain_,
        Time{},
        {},
        false)});
}

auto PeerManager::Peers::add_peer(Endpoint endpoint) noexcept -> int
{
    return add_peer(++next_id_, std::move(endpoint));
}

auto PeerManager::Peers::add_peer(const int id, Endpoint endpoint) noexcept
    -> int
{
    OT_ASSERT(endpoint);

    const auto address = OTIdentifier{endpoint->ID()};
    auto& count = active_[address];

    if (0 == count) {
        auto addressID = OTIdentifier{endpoint->ID()};

        if (auto peer = peer_factory(std::move(endpoint), id); peer) {
            const auto [it, added] = peers_.emplace(id, std::move(peer));

            if (added) {
                ++count;
                adjust_count(1);
                attempt_[addressID] = Clock::now();
                connected_.emplace(std::move(addressID));

                return id;
            }
        }
    }

    return -1;
}

auto PeerManager::Peers::adjust_count(int adjustment) noexcept -> void
{
    if (0 < adjustment) {
        ++count_;
    } else if (0 > adjustment) {
        --count_;
    } else {
        count_.store(0);
    }

    connected_peers_.Send([&] {
        auto work =
            network::zeromq::tagged_message(WorkType::BlockchainPeerConnected);
        work.AddFrame(chain_);
        work.AddFrame(count_.load());

        return work;
    }());
}

auto PeerManager::Peers::AddListener(
    const p2p::Address& address,
    std::promise<bool>& promise) noexcept -> void
{
    switch (address.Type()) {
        case p2p::Network::zmq: {
            auto& manager = incoming_zmq_;

            if (false == bool(manager)) {
                manager = IncomingConnectionManager::ZMQ(api_, *this);
            }

            OT_ASSERT(manager);

            promise.set_value(manager->Listen(address));
        } break;
        case p2p::Network::ipv6:
        case p2p::Network::ipv4: {
            auto& manager = incoming_tcp_;

            if (false == bool(manager)) {
                manager = IncomingConnectionManager::TCP(api_, *this);
            }

            OT_ASSERT(manager);

            promise.set_value(manager->Listen(address));
        } break;
        case p2p::Network::onion2:
        case p2p::Network::onion3:
        case p2p::Network::eep:
        case p2p::Network::cjdns:
        default: {
            promise.set_value(false);
        }
    }
}

auto PeerManager::Peers::AddPeer(
    const p2p::Address& address,
    std::promise<bool>& promise) noexcept -> void
{
    if (false == running_) {
        promise.set_value(false);

        return;
    }

    if (address.Chain() != chain_) {
        promise.set_value(false);

        return;
    }

    auto endpoint = Endpoint{factory::BlockchainAddress(
        api_,
        address.Style(),
        address.Type(),
        address.Bytes(),
        address.Port(),
        address.Chain(),
        address.LastConnected(),
        address.Services(),
        false)};

    OT_ASSERT(endpoint);

    add_peer(std::move(endpoint));
    promise.set_value(true);
}

auto PeerManager::Peers::ConstructPeer(Endpoint endpoint) noexcept -> int
{
    auto id = ++next_id_;
    parent_.AddIncomingPeer(
        id, reinterpret_cast<std::uintptr_t>(endpoint.release()));

    return id;
}

auto PeerManager::Peers::Disconnect(const int id) noexcept -> void
{
    if (auto it = peers_.find(id); peers_.end() != it) {
        if (incoming_zmq_) { incoming_zmq_->Disconnect(id); }

        if (incoming_tcp_) { incoming_tcp_->Disconnect(id); }

        const auto address = [&] {
            auto& peer = *it->second;
            auto out = peer.AddressID();
            peer.Shutdown();
            peers_.erase(it);

            return out;
        }();
        --active_.at(address);
        adjust_count(-1);
        connected_.erase(address);
    }
}

auto PeerManager::Peers::get_default_peer() const noexcept -> Endpoint
{
    if (localhost_peer_.get() == default_peer_) { return {}; }

    const auto& data = params::Data::Chains().at(chain_);

    return Endpoint{factory::BlockchainAddress(
        api_,
        data.p2p_protocol_,
        p2p::Network::ipv4,
        default_peer_,
        data.default_port_,
        chain_,
        Time{},
        {},
        false)};
}

auto PeerManager::Peers::get_dns_peer() const noexcept -> Endpoint
{
    try {
        const auto& data = params::Data::Chains().at(chain_);
        const auto& dns = data.dns_seeds_;

        if (0 == dns.size()) {
            LogVerbose()(OT_PRETTY_CLASS())("No dns seeds available").Flush();

            return {};
        }

        auto seeds = std::pmr::vector<std::string>{};
        const auto count = std::size_t{1};
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

        const auto port = data.default_port_;
        LogVerbose()(OT_PRETTY_CLASS())("Using DNS seed: ")(seed).Flush();

        for (const auto& endpoint : api_.Network().Asio().Resolve(seed, port)) {
            LogVerbose()(OT_PRETTY_CLASS())("Found address: ")(
                endpoint.GetAddress())
                .Flush();
            auto output = Endpoint{};
            auto network = p2p::Network{};

            switch (endpoint.GetType()) {
                case network::asio::Endpoint::Type::ipv4: {
                    network = p2p::Network::ipv4;
                } break;
                case network::asio::Endpoint::Type::ipv6: {
                    network = p2p::Network::ipv6;
                } break;
                default: {
                    LogVerbose()(OT_PRETTY_CLASS())("unknown endpoint type")
                        .Flush();

                    continue;
                }
            }

            output = factory::BlockchainAddress(
                api_,
                data.p2p_protocol_,
                network,
                api_.Factory().Data(endpoint.GetBytes()),
                port,
                chain_,
                Time{},
                {},
                false);

            if (output) {
                database_.AddOrUpdate(output->clone_internal());

                if (previous_failure_timeout(output->ID())) {
                    LogVerbose()(OT_PRETTY_CLASS())("Skipping ")(DisplayString(
                        chain_))(" peer ")(output->Display())(" due to retry "
                                                              "timeout")
                        .Flush();

                    continue;
                }

                return output;
            }
        }

        LogVerbose()(OT_PRETTY_CLASS())("No addresses found").Flush();

        return {};
    } catch (const boost::system::system_error& e) {
        LogDebug()(OT_PRETTY_CLASS())(e.what()).Flush();

        return {};
    } catch (...) {
        LogError()(OT_PRETTY_CLASS())("No dns seeds defined").Flush();

        return {};
    }
}

auto PeerManager::Peers::get_fallback_peer(
    const p2p::Protocol protocol) const noexcept -> Endpoint
{
    return database_.Get(protocol, get_types(), {});
}

auto PeerManager::Peers::get_peer() const noexcept -> Endpoint
{
    const auto protocol = params::Data::Chains().at(chain_).p2p_protocol_;
    auto pAddress = get_default_peer();

    if (pAddress) {
        LogVerbose()(OT_PRETTY_CLASS())("Default peer is: ")(
            pAddress->Display())
            .Flush();

        if (is_not_connected(*pAddress)) {
            LogVerbose()(OT_PRETTY_CLASS())(
                "Attempting to connect to default peer ")(pAddress->Display())
                .Flush();

            return pAddress;
        } else {
            LogVerbose()(OT_PRETTY_CLASS())(
                "Already connected / connecting to default "
                "peer ")(pAddress->Display())
                .Flush();
        }
    } else {
        LogVerbose()(OT_PRETTY_CLASS())("No default peer").Flush();
    }

    pAddress = get_preferred_peer(protocol);

    if (pAddress && is_not_connected(*pAddress)) {
        LogVerbose()(OT_PRETTY_CLASS())(
            "Attempting to connect to preferred peer: ")(pAddress->Display())
            .Flush();

        return pAddress;
    }

    pAddress = get_dns_peer();

    if (pAddress && is_not_connected(*pAddress)) {
        LogVerbose()(OT_PRETTY_CLASS())("Attempting to connect to dns peer: ")(
            pAddress->Display())
            .Flush();

        return pAddress;
    }

    pAddress = get_fallback_peer(protocol);

    if (pAddress) {
        LogVerbose()(OT_PRETTY_CLASS())(
            "Attempting to connect to fallback peer: ")(pAddress->Display())
            .Flush();
    }

    return pAddress;
}

auto PeerManager::Peers::get_preferred_peer(
    const p2p::Protocol protocol) const noexcept -> Endpoint
{
    auto output = database_.Get(protocol, get_types(), preferred_services_);

    if (output && (output->Bytes() == localhost_peer_)) {
        LogVerbose()(OT_PRETTY_CLASS())("Skipping localhost as preferred peer")
            .Flush();

        return {};
    }

    if (output && previous_failure_timeout(output->ID())) {
        LogVerbose()(OT_PRETTY_CLASS())("Skipping ")(DisplayString(chain_))(
            " peer ")(output->Display())(" due to retry timeout")
            .Flush();

        return {};
    }

    return output;
}

auto PeerManager::Peers::get_preferred_services(
    const internal::Config& config) noexcept -> std::pmr::set<p2p::Service>
{
    if (config.download_cfilters_) {

        return {p2p::Service::CompactFilters};
    } else {

        return {};
    }
}

auto PeerManager::Peers::get_types() const noexcept
    -> std::pmr::set<p2p::Network>
{
    using Type = blockchain::p2p::Network;
    using Mode = Options::ConnectionMode;
    auto output = std::pmr::set<p2p::Network>{};

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

            if (!ipv4data->empty()) { output.insert(Type::ipv4); }
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

            if (!ipv6data->empty()) { output.insert(Type::ipv6); }
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

auto PeerManager::Peers::is_not_connected(
    const p2p::Address& endpoint) const noexcept -> bool
{
    return 0 == connected_.count(endpoint.ID());
}

auto PeerManager::Peers::LookupIncomingSocket(const int id) noexcept(false)
    -> opentxs::network::asio::Socket
{
    if (!incoming_tcp_) {
        throw std::runtime_error{"TCP connection manager not instantiated"};
    }

    return incoming_tcp_->LookupIncomingSocket(id);
}

auto PeerManager::Peers::peer_factory(Endpoint endpoint, const int id) noexcept
    -> std::unique_ptr<p2p::internal::Peer>
{
    switch (params::Data::Chains().at(chain_).p2p_protocol_) {
        case p2p::Protocol::bitcoin: {
            return factory::BitcoinP2PPeerLegacy(
                api_,
                config_,
                mempool_,
                node_,
                headers_,
                filter_,
                block_,
                parent_,
                policy_,
                id,
                std::move(endpoint),
                shutdown_endpoint_);
        }
        case p2p::Protocol::opentxs:
        case p2p::Protocol::ethereum:
        default: {
            OT_FAIL;
        }
    }
}

auto PeerManager::Peers::previous_failure_timeout(
    const Identifier& addressID) const noexcept -> bool
{
    static constexpr auto timeout = std::chrono::minutes{10};

    if (const auto it = attempt_.find(addressID); attempt_.end() == it) {

        return false;
    } else {
        const auto& last = it->second;

        return (Clock::now() - last) < timeout;
    }
}

auto PeerManager::Peers::set_default_peer(
    const std::string node,
    const Data& localhost,
    bool& invalidPeer) noexcept -> OTData
{
    if (false == node.empty()) {
        try {
            const auto bytes = ip::make_address_v4(node).to_bytes();

            return Data::Factory(bytes.data(), bytes.size());
        } catch (...) {
            invalidPeer = true;
        }
    }

    return localhost;
}

auto PeerManager::Peers::Run() noexcept -> bool
{
    if ((false == running_) || invalid_peer_) { return false; }

    const auto target = minimum_peers_.load();

    if (target > peers_.size()) {
        LogVerbose()(OT_PRETTY_CLASS())("Fewer peers (")(peers_.size())(
            ") than desired (")(target)(")")
            .Flush();
        auto peer = get_peer();

        if (peer) { add_peer(std::move(peer)); }
    }

    return target > peers_.size();
}

auto PeerManager::Peers::Shutdown() noexcept -> void
{
    OT_ASSERT(false == running_);

    if (incoming_zmq_) { incoming_zmq_->Shutdown(); }
    if (incoming_tcp_) { incoming_tcp_->Shutdown(); }

    for (auto& [id, peer] : peers_) {
        peer->Shutdown();
        peer.reset();
    }

    peers_.clear();
    adjust_count(0);
    active_.clear();
}

PeerManager::Peers::~Peers() = default;
}  // namespace opentxs::blockchain::node::implementation
