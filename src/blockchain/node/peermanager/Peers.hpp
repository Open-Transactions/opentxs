// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/container/flat_set.hpp>
#include <boost/container/vector.hpp>
#include <atomic>
#include <cstddef>
#include <functional>
#include <memory>
#include <string_view>
#include <utility>

#include "internal/blockchain/p2p/P2P.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/p2p/Address.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "util/Gatekeeper.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace database
{
class Peer;
}  // namespace database

namespace node
{
namespace internal
{
class PeerManager;
struct Config;
}  // namespace internal

namespace peermanager
{
class IncomingConnectionManager;
}  // namespace peermanager

class Manager;
struct Endpoints;
}  // namespace node

namespace p2p
{
class Address;
}  // namespace p2p
}  // namespace blockchain

namespace identifier
{
class Generic;
}  // namespace identifier

namespace network
{
namespace asio
{
class Socket;
}  // namespace asio

namespace blockchain
{
namespace internal
{
class Peer;
}  // namespace internal
}  // namespace blockchain
}  // namespace network

class Data;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::peermanager
{
class Peers
{
public:
    const Type chain_;

    auto Count() const noexcept -> std::size_t { return count_.load(); }
    auto Nonce() const noexcept -> const p2p::bitcoin::Nonce& { return nonce_; }

    auto AddIncoming(const int id, p2p::Address&& endpoint) noexcept -> void;
    auto AddListener(p2p::Address&& address) noexcept -> void;
    auto AddPeer(p2p::Address&& address) noexcept -> void;
    auto AddResolvedDNS(Vector<p2p::Address> address) noexcept -> void;
    auto ConstructPeer(const p2p::Address& endpoint) noexcept -> int;
    auto LookupIncomingSocket(const int id) noexcept(false)
        -> opentxs::network::asio::Socket;
    auto Disconnect(const int id) noexcept -> void;
    auto Run() noexcept -> bool;
    auto Shutdown() noexcept -> void;

    Peers(
        const api::Session& api,
        const node::internal::Config& config,
        const node::Manager& node,
        database::Peer& database,
        node::internal::PeerManager& parent,
        const node::Endpoints& endpoints,
        const Type chain,
        const std::string_view seednode) noexcept;

    ~Peers();

private:
    using Peer = network::blockchain::internal::Peer;
    using Resolver = boost::asio::ip::tcp::resolver;
    using Addresses = boost::container::flat_set<identifier::Generic>;

    struct PeerData {
        identifier::Generic address_id_;
        network::zeromq::socket::Raw socket_;

        PeerData(
            const identifier::Generic& address,
            network::zeromq::socket::Raw&& socket)
            : address_id_(address)
            , socket_(std::move(socket))
        {
        }
    };

    static std::atomic<int> next_id_;

    const api::Session& api_;
    const node::internal::Config& config_;
    const node::Manager& node_;
    database::Peer& database_;
    node::internal::PeerManager& parent_;
    opentxs::network::zeromq::socket::Raw to_blockchain_api_;
    const node::Endpoints& endpoints_;
    const bool invalid_peer_;
    const ByteArray localhost_peer_;
    const ByteArray default_peer_;
    const UnallocatedSet<p2p::Service> preferred_services_;
    const p2p::bitcoin::Nonce nonce_;
    std::atomic<std::size_t> minimum_peers_;
    Map<int, PeerData> peers_;
    Map<identifier::Generic, int> active_;
    std::atomic<std::size_t> count_;
    Addresses connected_;
    std::unique_ptr<IncomingConnectionManager> incoming_zmq_;
    std::unique_ptr<IncomingConnectionManager> incoming_tcp_;
    Map<identifier::Generic, Time> attempt_;
    Deque<p2p::Address> resolved_dns_;
    Gatekeeper gatekeeper_;

    static auto get_preferred_services(
        const node::internal::Config& config) noexcept
        -> UnallocatedSet<p2p::Service>;
    static auto set_default_peer(
        const std::string_view node,
        const Data& localhost,
        bool& invalidPeer) noexcept -> ByteArray;

    auto get_default_peer() const noexcept -> p2p::Address;
    auto get_fallback_peer(const p2p::Protocol protocol) const noexcept
        -> p2p::Address;
    auto get_preferred_peer(const p2p::Protocol protocol) const noexcept
        -> p2p::Address;
    auto get_types() const noexcept -> UnallocatedSet<p2p::Network>;
    auto is_not_connected(const p2p::Address& endpoint) const noexcept -> bool;

    auto add_peer(p2p::Address&& endpoint) noexcept -> int;
    auto add_peer(const int id, p2p::Address&& endpoint) noexcept -> int;
    auto adjust_count(int adjustment) noexcept -> void;
    auto get_dns_peer() noexcept -> p2p::Address;
    auto get_peer() noexcept -> p2p::Address;
    auto previous_failure_timeout(
        const identifier::Generic& addressID) const noexcept -> bool;
    auto peer_factory(
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<const opentxs::blockchain::node::Manager> network,
        const int id,
        std::string_view inproc,
        p2p::Address endpoint) noexcept -> Peer;
};
}  // namespace opentxs::blockchain::node::peermanager
