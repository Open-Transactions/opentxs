// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/smart_ptr/shared_ptr.hpp>
#include <cs_plain_guarded.h>
#include <chrono>
#include <cstddef>
#include <exception>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>

#include "internal/blockchain/node/Types.hpp"
#include "internal/network/blockchain/bitcoin/message/Types.hpp"
#include "internal/network/zeromq/Pipeline.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/network/asio/Endpoint.hpp"
#include "opentxs/network/asio/Socket.hpp"
#include "opentxs/network/blockchain/Address.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/Types.hpp"
#include "opentxs/network/zeromq/message/Envelope.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
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
    auto Init(boost::shared_ptr<Actor> me) noexcept -> void
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

        PeerData(
            const identifier::Generic& address,
            network::zeromq::socket::Raw&& socket,
            ConnectionID external,
            allocator_type alloc)
            : address_id_(address)
            , socket_(std::move(socket))
            , external_id_(std::move(external), alloc)
            , internal_id_(alloc)
            , queue_(alloc)
        {
        }
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

    static constexpr auto invalid_peer_ = PeerID{-1};
    static constexpr auto connect_timeout_ = 2min;
    static constexpr auto dns_timeout_ = 30s;
    static constexpr auto registration_timeout_ = 1s;

    std::shared_ptr<const api::Session> api_p_;
    std::shared_ptr<const node::Manager> node_p_;
    boost::shared_ptr<Actor> me_;
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
    std::optional<sTime> dns_;
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
    Timer dns_timer_;
    Timer registration_timer_;

    static auto accept(
        const network::blockchain::Transport type,
        const network::asio::Endpoint& endpoint,
        network::asio::Socket&& socket,
        boost::shared_ptr<Actor> me) noexcept -> void;

    auto active_addresses(allocator_type monotonic) const noexcept
        -> Set<AddressID>;
    auto dns_timed_out() const noexcept -> bool;
    auto have_target_peers() const noexcept -> bool;
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
    auto check_dns() noexcept -> void;
    auto check_peers(allocator_type monotonic) noexcept -> void;
    auto check_registration() noexcept -> void;
    auto do_shutdown() noexcept -> void;
    auto do_startup(allocator_type monotonic) noexcept -> bool;
    auto get_peer(allocator_type monotonic) noexcept
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
    auto send_dns_query() noexcept -> void;
    auto work(allocator_type monotonic) noexcept -> bool;
};
}  // namespace opentxs::blockchain::node::peermanager
