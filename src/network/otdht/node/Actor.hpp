// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <ankerl/unordered_dense.h>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <frozen/unordered_set.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <memory>
#include <optional>
#include <span>
#include <string_view>
#include <tuple>
#include <utility>

#include "internal/network/otdht/Node.hpp"
#include "internal/network/otdht/Types.hpp"
#include "internal/util/Options.hpp"
#include "internal/util/P0330.hpp"
#include "network/otdht/node/Shared.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/network/blockchain/Address.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/zeromq/Types.hpp"
#include "opentxs/network/zeromq/message/Envelope.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Types.hpp"
#include "opentxs/util/Container.hpp"
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
namespace block
{
class Position;
}  // namespace block
}  // namespace blockchain

namespace network
{
namespace zeromq
{
namespace socket
{
class Raw;
}  // namespace socket

class Frame;
}  // namespace zeromq
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::otdht
{
class Node::Actor final : public opentxs::Actor<Node::Actor, NodeJob>
{
public:
    auto Init(boost::shared_ptr<Actor> self) noexcept -> void
    {
        signal_startup(self);
    }

    Actor(
        std::shared_ptr<const api::Session> api,
        boost::shared_ptr<Shared> shared,
        zeromq::BatchID batchID,
        allocator_type alloc) noexcept;
    Actor() = delete;
    Actor(const Actor&) = delete;
    Actor(Actor&&) = delete;
    auto operator=(const Actor&) -> Actor& = delete;
    auto operator=(Actor&&) -> Actor& = delete;

    ~Actor() final;

private:
    friend opentxs::Actor<Node::Actor, NodeJob>;

    static constexpr auto external_router_limit_ = 8_uz;

    using PeerData = std::tuple<CString, CString, zeromq::socket::Raw>;
    using Peers = Map<CString, PeerData>;
    using ExternalEndpoint = std::pair<blockchain::Transport, ByteArray>;
    using ParsedListener = std::pair<CString, ExternalEndpoint>;
    using Cookie = std::uint64_t;
    using RemoteID = zeromq::Envelope;
    using LocalID = zeromq::Envelope;
    using PeerManagerID = zeromq::Envelope;
    using Queue = Deque<zeromq::Message>;
    using Connections = Set<RemoteID>;
    using ExternalSocketIndex = std::size_t;
    using BlockchainPeerData = std::tuple<Cookie, LocalID, Queue>;
    using IncomingBlockchainIndex =
        ankerl::unordered_dense::pmr::map<RemoteID, BlockchainPeerData>;
    using PeerManagerIndex = ankerl::unordered_dense::pmr::
        map<opentxs::blockchain::Type, std::pair<PeerManagerID, Connections>>;
    using IncomingBlockchainReverseIndex = ankerl::unordered_dense::pmr::
        map<Cookie, std::pair<RemoteID, ExternalSocketIndex>>;
    using OutgoingIndex = ankerl::unordered_dense::pmr::
        map<LocalID, std::tuple<RemoteID, Cookie, ExternalSocketIndex>>;
    using ExternalSockets =
        frozen::unordered_set<zeromq::SocketID, external_router_limit_>;
    using ExternalIndex = ankerl::unordered_dense::pmr::
        map<zeromq::SocketID, ExternalSocketIndex>;

    std::shared_ptr<const api::Session> api_p_;
    boost::shared_ptr<Shared> shared_p_;
    const Shared& shared_;
    const api::Session& api_;
    Shared::Guarded& data_;
    zeromq::socket::Raw& publish_;
    zeromq::socket::Raw& router_;
    std::array<zeromq::socket::Raw*, external_router_limit_> external_;
    const ExternalSockets external_sockets_;
    const ExternalIndex external_index_;
    Peers peers_;
    Peers listeners_;
    Set<CString> listen_endpoints_;
    Vector<blockchain::Address> external_endpoints_;
    IncomingBlockchainIndex blockchain_index_;
    IncomingBlockchainReverseIndex blockchain_reverse_index_;
    OutgoingIndex outgoing_blockchain_index_;
    PeerManagerIndex peer_manager_index_;
    Cookie next_cookie_;

    auto forward(
        const zeromq::Envelope& recipient,
        std::span<zeromq::Frame> payload,
        zeromq::socket::Raw& socket) const noexcept -> void;
    auto get_peers() const noexcept -> Set<CString>;
    auto get_peers(Message& out) const noexcept -> void;
    auto parse(const opentxs::internal::Options::Listener& params)
        const noexcept -> std::optional<ParsedListener>;

    auto do_shutdown() noexcept -> void;
    auto do_startup(allocator_type monotonic) noexcept -> bool;
    auto listen(allocator_type monotonic) noexcept -> void;
    auto load_peers() noexcept -> void;
    auto load_positions() noexcept -> void;
    auto pipeline(const Work work, Message&& msg, allocator_type) noexcept
        -> void;
    auto pipeline_external(
        const Work work,
        Message&& msg,
        ExternalSocketIndex index,
        allocator_type monotonic) noexcept -> void;
    auto pipeline_internal(const Work work, Message&& msg) noexcept -> void;
    auto pipeline_router(
        const Work work,
        Message&& msg,
        allocator_type monotonic) noexcept -> void;
    auto process_add_listener(Message&& msg) noexcept -> void;
    auto process_blockchain_external(
        Message&& msg,
        ExternalSocketIndex index,
        allocator_type monotonic) noexcept -> void;
    auto process_blockchain_internal(
        Message&& msg,
        allocator_type monotonic) noexcept -> void;
    auto process_cfilter(
        opentxs::blockchain::Type chain,
        opentxs::blockchain::block::Position&& tip) noexcept -> void;
    auto process_chain_state(Message&& msg) noexcept -> void;
    auto process_connect_peer(Message&& msg, allocator_type monotonic) noexcept
        -> void;
    auto process_connect_peer_manager(Message&& msg) noexcept -> void;
    auto process_disconnect_peer(Message&& msg) noexcept -> void;
    auto process_disconnect_peer_manager(Message&& msg) noexcept -> void;
    auto process_new_cfilter(Message&& msg) noexcept -> void;
    auto process_new_peer(Message&& msg) noexcept -> void;
    auto process_peer(std::string_view endpoint) noexcept -> void;
    auto process_registration(Message&& msg) noexcept -> void;
    auto publish_peers() noexcept -> void;
    auto send_to_peers(Message&& msg) noexcept -> void;
    auto work(allocator_type monotonic) noexcept -> bool;

    Actor(
        std::shared_ptr<const api::Session> api,
        boost::shared_ptr<Shared> shared,
        zeromq::BatchID batchID,
        Vector<network::zeromq::socket::SocketRequest> extra,
        allocator_type alloc) noexcept;
};
}  // namespace opentxs::network::otdht
