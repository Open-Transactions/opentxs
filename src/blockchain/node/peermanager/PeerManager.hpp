// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_plain_guarded.h>
#include <chrono>
#include <cstddef>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>

#include "internal/blockchain/node/Types.hpp"
#include "internal/network/blockchain/bitcoin/message/Types.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/PMR.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/network/asio/Endpoint.hpp"
#include "opentxs/network/asio/Socket.hpp"
#include "opentxs/network/blockchain/Address.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/Types.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/network/zeromq/message/Envelope.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Writer.hpp"
#include "util/Actor.hpp"

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
namespace peermanager
{
class Actor;
}  // namespace peermanager

class Manager;
}  // namespace node
}  // namespace blockchain
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::peermanager
{
using ActorType = opentxs::Actor<peermanager::Actor, PeerManagerJobs>;

class Actor final : public ActorType
{
public:
    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }
    auto Init(std::shared_ptr<Actor> me) noexcept -> void
    {
        me_ = me;
        signal_startup(me_);
    }

    Actor(
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<const node::Manager> node,
        database::Peer& db,
        std::string_view peers,
        network::zeromq::BatchID batch,
        allocator_type alloc) noexcept;
    Actor() = delete;
    Actor(const Actor&) = delete;
    Actor(Actor&&) = delete;
    auto operator=(const Actor&) -> Actor& = delete;
    auto operator=(Actor&&) -> Actor& = delete;

    ~Actor() final;

private:
    friend ActorType;

    using ConnectionID = network::zeromq::Envelope;

    struct PeerData {
        const identifier::Generic address_id_;
        network::zeromq::socket::Raw socket_;
        const ConnectionID external_id_;
        ConnectionID internal_id_;
        Deque<Message> queue_;
        const opentxs::network::blockchain::Transport transport_;
        const bool incoming_;

        PeerData(
            const identifier::Generic& address,
            network::zeromq::socket::Raw&& socket,
            ConnectionID external,
            opentxs::network::blockchain::Transport transport,
            bool incoming,
            allocator_type alloc)
            : address_id_(address)
            , socket_(std::move(socket))
            , external_id_(std::move(external), alloc)
            , internal_id_(alloc)
            , queue_(alloc)
            , transport_(transport)
            , incoming_(incoming)
        {
        }
    };

    struct DNS {
        static constexpr auto timeout_ = 30s;
        static constexpr auto repeat_ = 5min;

        auto GotResponse() noexcept -> void;
        auto NeedQuery() noexcept -> bool;

    private:
        std::optional<sTime> last_sent_query_{std::nullopt};
        std::optional<sTime> last_received_response_{std::nullopt};
    };

    struct Seed {
        network::blockchain::Address address_{};
        std::optional<sTime> last_dns_query_{};

        auto RetryDNS(const sTime& now) const noexcept -> bool;
    };

    using SocketQueue =
        Deque<std::pair<network::blockchain::Address, network::asio::Socket>>;
    using GuardedSocketQueue = libguarded::plain_guarded<SocketQueue>;
    using AsioListeners = List<network::asio::Endpoint>;
    using AddressID = identifier::Generic;
    using PeerID = int;
    using AddressIndex = Map<AddressID, PeerID>;
    using PeerIndex = Map<PeerID, PeerData>;
    using Addresses = Vector<network::blockchain::Address>;
    using SeedNode = std::pair<CString, CString>;
    using SeedNodes = Vector<SeedNode>;
    using ResolvedSeedNodes = Map<CString, Seed>;
    using TransportIndex =
        Map<opentxs::network::blockchain::Transport, std::size_t>;

    static constexpr auto invalid_peer_ = PeerID{-1};
    static constexpr auto connect_timeout_ = 2min;
    static constexpr auto registration_timeout_ = 1s;
    static constexpr auto zmq_peer_target_ = 2_uz;
    static constexpr auto dns_timeout_ = 30s;

    std::shared_ptr<const api::Session> api_p_;
    std::shared_ptr<const node::Manager> node_p_;
    std::shared_ptr<Actor> me_;
    const api::Session& api_;
    const node::Manager& node_;
    database::Peer& db_;
    network::zeromq::socket::Raw& to_blockchain_api_;
    network::zeromq::socket::Raw& broadcast_tx_;
    network::zeromq::socket::Raw& to_otdht_;
    network::zeromq::socket::Raw& to_peers_;
    const network::zeromq::SocketID dealer_id_;
    const network::zeromq::SocketID otdht_id_;
    const Type chain_;
    const network::blockchain::bitcoin::message::Nonce nonce_;
    const Set<network::blockchain::bitcoin::Service> preferred_services_;
    const Addresses command_line_peers_;
    const std::size_t peer_target_;
    const SeedNodes seed_nodes_;
    ResolvedSeedNodes seeds_;
    DNS dns_;
    GuardedSocketQueue socket_queue_;
    AsioListeners asio_listeners_;
    PeerID next_id_;
    PeerIndex peers_;
    AddressIndex index_;
    Set<PeerID> active_;
    Set<PeerID> verified_;
    Set<PeerID> outgoing_;
    bool registered_;
    Set<network::blockchain::Address> external_addresses_;
    TransportIndex transports_;
    bool database_is_ready_;
    Deque<std::pair<Work, Message>> queue_;
    Timer dns_timer_;
    Timer registration_timer_;
    Timer startup_timer_;

    static auto accept(
        const network::blockchain::Transport type,
        const network::asio::Endpoint& endpoint,
        network::asio::Socket&& socket,
        std::shared_ptr<Actor> me) noexcept -> void;

    auto active_addresses(allocator_type monotonic) const noexcept
        -> Set<AddressID>;
    auto have_target_peers() const noexcept -> bool;
    auto have_target_zmq_peers() const noexcept -> bool;
    auto is_active(const network::blockchain::Address& addr) const noexcept
        -> bool;
    auto need_peers() const noexcept -> bool;
    auto usable_networks(allocator_type monotonic) const noexcept
        -> Set<network::blockchain::Transport>;

    auto accept_asio() noexcept -> void;
    auto add_peer(
        network::blockchain::Address endpoint,
        bool incoming,
        std::optional<network::asio::Socket> socket = std::nullopt,
        ConnectionID connection = {},
        std::optional<Message> = std::nullopt) noexcept -> PeerID;
    auto broadcast_active() noexcept -> void;
    auto broadcast_verified(std::string_view address = {}) noexcept -> void;
    auto check_command_line_peers() noexcept -> void;
    auto check_database(allocator_type monotonic) noexcept -> bool;
    auto check_dns() noexcept -> void;
    auto check_peers(allocator_type monotonic) noexcept -> void;
    auto check_registration() noexcept -> void;
    auto check_seeds() noexcept -> void;
    auto do_shutdown() noexcept -> void;
    auto do_startup(allocator_type monotonic) noexcept -> bool;
    auto first_time_init(allocator_type monotonic) noexcept -> void;
    auto get_peer(bool zmqOnly, allocator_type monotonic) noexcept
        -> network::blockchain::Address;
    auto listen(
        const network::blockchain::Address& address,
        allocator_type monotonic) noexcept -> void;
    auto listen_tcp(const network::blockchain::Address& address) noexcept
        -> void;
    auto pipeline(const Work work, Message&& msg, allocator_type) noexcept
        -> void;
    auto pipeline_dealer(
        const Work work,
        Message&& msg,
        allocator_type) noexcept -> void;
    auto pipeline_otdht(const Work work, Message&& msg, allocator_type) noexcept
        -> void;
    auto pipeline_standard(
        const Work work,
        Message&& msg,
        allocator_type) noexcept -> void;
    auto process_addlistener(Message&& msg, allocator_type monotonic) noexcept
        -> void;
    auto process_addpeer(Message&& msg) noexcept -> void;
    auto process_broadcasttx(Message&& msg) noexcept -> void;
    auto process_disconnect(Message&& msg) noexcept -> void;
    auto process_disconnect(PeerID id, std::string_view display) noexcept
        -> void;
    auto process_gossip_address(Message&& msg) noexcept -> void;
    auto process_register_ack(Message&& msg) noexcept -> void;
    auto process_report(Message&& msg) noexcept -> void;
    auto process_resolve(Message&& msg) noexcept -> void;
    auto process_spawn_peer(Message&& msg, allocator_type monotonic) noexcept
        -> void;
    auto process_verify(Message&& msg) noexcept -> void;
    auto process_verify(PeerID id, std::string_view display) noexcept -> void;
    auto reset_dns_timer() noexcept -> void;
    auto reset_registration_timer() noexcept -> void;
    auto reset_startup_timer() noexcept -> void;
    auto send_dns_query() noexcept -> void;
    auto work(allocator_type monotonic) noexcept -> bool;
};
}  // namespace opentxs::blockchain::node::peermanager
