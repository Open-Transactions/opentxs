// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/asio.hpp>
#include <boost/asio/ip/tcp.hpp>
#include <boost/container/flat_set.hpp>
#include <atomic>
#include <cstddef>
#include <cstdint>
#include <future>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "1_Internal.hpp"
#include "core/Worker.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/blockchain/p2p/P2P.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/network/asio/Socket.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Push.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
namespace bitcoin
{
class Transaction;
}  // namespace bitcoin

class Block;
}  // namespace block

namespace node
{
namespace internal
{
struct Mempool;
}  // namespace internal

class HeaderOracle;
}  // namespace node

namespace p2p
{
namespace internal
{
struct Address;
struct Peer;
}  // namespace internal

class Address;
}  // namespace p2p
}  // namespace blockchain

namespace network
{
namespace asio
{
class Socket;
}  // namespace asio

namespace zeromq
{
namespace socket
{
class Publish;
class Sender;
}  // namespace socket

class Context;
}  // namespace zeromq
}  // namespace network

class Flag;
}  // namespace opentxs

namespace zmq = opentxs::network::zeromq;

namespace opentxs::blockchain::node::implementation
{
class PeerManager final : virtual public node::internal::PeerManager,
                          public Worker<PeerManager, api::Session>
{
public:
    class IncomingConnectionManager;

    struct Peers {
        using Endpoint = std::unique_ptr<p2p::internal::Address>;

        const Type chain_;

        auto Count() const noexcept -> std::size_t { return count_.load(); }

        auto AddIncoming(const int id, Endpoint endpoint) noexcept -> void
        {
            add_peer(id, std::move(endpoint));
        }
        auto AddListener(
            const p2p::Address& address,
            std::promise<bool>& promise) noexcept -> void;
        auto AddPeer(
            const p2p::Address& address,
            std::promise<bool>& promise) noexcept -> void;
        auto ConstructPeer(Endpoint endpoint) noexcept -> int;
        auto LookupIncomingSocket(const int id) noexcept(false)
            -> opentxs::network::asio::Socket;
        auto Disconnect(const int id) noexcept -> void;
        auto Run() noexcept -> bool;
        auto Shutdown() noexcept -> void;

        Peers(
            const api::Session& api,
            const node::internal::Config& config,
            const node::internal::Mempool& mempool,
            const node::internal::Network& node,
            const node::HeaderOracle& headers,
            const node::internal::FilterOracle& filter,
            const node::internal::BlockOracle& block,
            const node::internal::PeerDatabase& database,
            const node::internal::PeerManager& parent,
            const database::BlockStorage policy,
            const std::atomic<bool>& running,
            const std::string& shutdown,
            const Type chain,
            const std::string& seednode,
            const std::size_t peerTarget) noexcept;

        ~Peers();

    private:
        using Peer = std::unique_ptr<p2p::internal::Peer>;
        using Resolver = boost::asio::ip::tcp::resolver;
        using Addresses = boost::container::flat_set<OTIdentifier>;

        const api::Session& api_;
        const node::internal::Config& config_;
        const node::internal::Mempool& mempool_;
        const node::internal::Network& node_;
        const node::HeaderOracle& headers_;
        const node::internal::FilterOracle& filter_;
        const node::internal::BlockOracle& block_;
        const node::internal::PeerDatabase& database_;
        const node::internal::PeerManager& parent_;
        const network::zeromq::socket::Publish& connected_peers_;
        const database::BlockStorage policy_;
        const std::atomic<bool>& running_;
        const std::string& shutdown_endpoint_;
        const bool invalid_peer_;
        const OTData localhost_peer_;
        const OTData default_peer_;
        const std::pmr::set<p2p::Service> preferred_services_;
        std::atomic<int> next_id_;
        std::atomic<std::size_t> minimum_peers_;
        std::pmr::map<int, Peer> peers_;
        std::pmr::map<OTIdentifier, int> active_;
        std::atomic<std::size_t> count_;
        Addresses connected_;
        std::unique_ptr<IncomingConnectionManager> incoming_zmq_;
        std::unique_ptr<IncomingConnectionManager> incoming_tcp_;
        std::pmr::map<OTIdentifier, Time> attempt_;

        static auto get_preferred_services(
            const node::internal::Config& config) noexcept
            -> std::pmr::set<p2p::Service>;
        static auto set_default_peer(
            const std::string node,
            const Data& localhost,
            bool& invalidPeer) noexcept -> OTData;

        auto get_default_peer() const noexcept -> Endpoint;
        auto get_dns_peer() const noexcept -> Endpoint;
        auto get_fallback_peer(const p2p::Protocol protocol) const noexcept
            -> Endpoint;
        auto get_peer() const noexcept -> Endpoint;
        auto get_preferred_peer(const p2p::Protocol protocol) const noexcept
            -> Endpoint;
        auto get_types() const noexcept -> std::pmr::set<p2p::Network>;
        auto is_not_connected(const p2p::Address& endpoint) const noexcept
            -> bool;

        auto add_peer(Endpoint endpoint) noexcept -> int;
        auto add_peer(const int id, Endpoint endpoint) noexcept -> int;
        auto adjust_count(int adjustment) noexcept -> void;
        auto previous_failure_timeout(
            const Identifier& addressID) const noexcept -> bool;
        auto peer_factory(Endpoint endpoint, const int id) noexcept
            -> std::unique_ptr<p2p::internal::Peer>;
    };

    enum class Work : OTZMQWorkType {
        Disconnect = OT_ZMQ_INTERNAL_SIGNAL + 0,
        AddPeer = OT_ZMQ_INTERNAL_SIGNAL + 1,
        AddListener = OT_ZMQ_INTERNAL_SIGNAL + 2,
        IncomingPeer = OT_ZMQ_INTERNAL_SIGNAL + 3,
        StateMachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
        Shutdown = value(WorkType::Shutdown),
    };

    auto AddIncomingPeer(const int id, std::uintptr_t endpoint) const noexcept
        -> void final;
    auto AddPeer(const p2p::Address& address) const noexcept -> bool final;
    auto BroadcastBlock(const block::Block& block) const noexcept -> bool final;
    auto BroadcastTransaction(
        const block::bitcoin::Transaction& tx) const noexcept -> bool final;
    auto Database() const noexcept -> const node::internal::PeerDatabase& final
    {
        return database_;
    }
    auto Connect() noexcept -> bool final;
    auto Disconnect(const int id) const noexcept -> void final;
    auto Endpoint(const Task type) const noexcept -> std::string final
    {
        return jobs_.Endpoint(type);
    }
    auto GetPeerCount() const noexcept -> std::size_t final
    {
        return peers_.Count();
    }
    auto GetVerifiedPeerCount() const noexcept -> std::size_t final;
    auto Heartbeat() const noexcept -> void final
    {
        jobs_.Dispatch(Task::Heartbeat);
    }
    auto JobReady(const Task type) const noexcept -> void final;
    auto Listen(const p2p::Address& address) const noexcept -> bool final;
    auto LookupIncomingSocket(const int id) const noexcept(false)
        -> opentxs::network::asio::Socket final;
    auto RequestBlock(const block::Hash& block) const noexcept -> bool final;
    auto RequestBlocks(const std::pmr::vector<ReadView>& hashes) const noexcept
        -> bool final;
    auto RequestHeaders() const noexcept -> bool final;
    auto VerifyPeer(const int id, const std::string& address) const noexcept
        -> void final;

    auto Shutdown() noexcept -> std::shared_future<void> final
    {
        return signal_shutdown();
    }

    auto init() noexcept -> void final;

    PeerManager(
        const api::Session& api,
        const node::internal::Config& config,
        const node::internal::Mempool& mempool,
        const node::internal::Network& node,
        const node::HeaderOracle& headers,
        const node::internal::FilterOracle& filter,
        const node::internal::BlockOracle& block,
        const node::internal::PeerDatabase& database,
        const Type chain,
        const database::BlockStorage policy,
        const std::string& seednode,
        const std::string& shutdown) noexcept;

    ~PeerManager() final;

private:
    friend Worker<PeerManager, api::Session>;

    struct Jobs {
        auto Endpoint(const Task type) const noexcept -> std::string;
        auto Work(const Task task) const noexcept -> network::zeromq::Message;

        auto Dispatch(const Task type) noexcept -> void;
        auto Dispatch(zmq::Message&& work) noexcept -> void;
        auto Shutdown() noexcept -> void;

        Jobs(const api::Session& api) noexcept;

    private:
        using EndpointMap = std::pmr::map<Task, std::string>;
        using SocketMap = std::pmr::map<Task, zmq::socket::Sender*>;

        const zmq::Context& zmq_;
        OTZMQPushSocket getheaders_;
        OTZMQPublishSocket getcfheaders_;
        OTZMQPublishSocket getcfilters_;
        OTZMQPublishSocket getblocks_;
        OTZMQPublishSocket heartbeat_;
        OTZMQPushSocket getblock_;
        OTZMQPushSocket broadcast_transaction_;
        OTZMQPublishSocket broadcast_block_;
        const EndpointMap endpoint_map_;
        const SocketMap socket_map_;

        static auto listen(
            EndpointMap& map,
            const Task type,
            const zmq::socket::Sender& socket) noexcept -> void;

        Jobs() = delete;
    };

    const node::internal::Network& node_;
    const node::internal::PeerDatabase& database_;
    const Type chain_;
    mutable Jobs jobs_;
    mutable Peers peers_;
    mutable std::mutex verified_lock_;
    mutable std::pmr::set<int> verified_peers_;
    std::promise<void> init_promise_;
    std::shared_future<void> init_;

    static auto peer_target(
        const Type chain,
        const database::BlockStorage policy) noexcept -> std::size_t;

    auto pipeline(zmq::Message&& message) noexcept -> void;
    auto shutdown(std::promise<void>& promise) noexcept -> void;
    auto state_machine() noexcept -> bool;

    PeerManager() = delete;
    PeerManager(const PeerManager&) = delete;
    PeerManager(PeerManager&&) = delete;
    auto operator=(const PeerManager&) -> PeerManager& = delete;
    auto operator=(PeerManager&&) -> PeerManager& = delete;
};
}  // namespace opentxs::blockchain::node::implementation
