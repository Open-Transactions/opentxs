// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <frozen/bits/basic_types.h>
#include <frozen/unordered_map.h>
#include <cstddef>
#include <memory>
#include <optional>
#include <string_view>

#include "blockchain/bitcoin/Inventory.hpp"
#include "internal/blockchain/node/headeroracle/HeaderJob.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/network/blockchain/Peer.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/util/P0330.hpp"
#include "network/blockchain/peer/Imp.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/GCS.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Header.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

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
class Header;
}  // namespace block

namespace cfilter
{
class Hash;
}  // namespace cfilter

namespace node
{
namespace internal
{
class BlockBatch;
class Mempool;
struct Config;
}  // namespace internal

class Manager;
}  // namespace node

namespace p2p
{
namespace bitcoin
{
namespace message
{
namespace internal
{
struct Cfheaders;
struct Headers;
}  // namespace internal
}  // namespace message

class Header;
struct Message;
}  // namespace bitcoin

class Address;
}  // namespace p2p
}  // namespace blockchain

namespace network
{
namespace asio
{
class Socket;
}  // namespace asio
}  // namespace network

class Data;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::blockchain::bitcoin
{
class Peer final : public blockchain::internal::Peer::Imp
{
public:
    Peer(
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<const opentxs::blockchain::node::Manager> network,
        opentxs::blockchain::p2p::bitcoin::Nonce nonce,
        int peerID,
        opentxs::blockchain::p2p::Address address,
        opentxs::blockchain::p2p::bitcoin::ProtocolVersion protocol,
        std::string_view fromParent,
        std::optional<asio::Socket> socket,
        zeromq::BatchID batch,
        allocator_type alloc) noexcept;
    Peer() = delete;
    Peer(const Peer&) = delete;
    Peer(Peer&&) = delete;
    auto operator=(const Peer&) -> Peer& = delete;
    auto operator=(Peer&&) -> Peer& = delete;

    ~Peer() final;

private:
    using MessageType = opentxs::blockchain::p2p::bitcoin::Message;
    using HeaderType = opentxs::blockchain::p2p::bitcoin::Header;
    using Command = opentxs::blockchain::p2p::bitcoin::Command;
    using CommandFunction = void (
        Peer::*)(std::unique_ptr<HeaderType>, zeromq::Frame&&, allocator_type);
    static constexpr auto CommandMapSize = std::size_t{39};
    using CommandMap =
        frozen::unordered_map<Command, CommandFunction, CommandMapSize>;

    struct Handshake {
        bool got_version_{false};
        bool got_verack_{false};
    };
    struct Verification {
        bool got_block_header_{false};
        bool got_cfheader_{false};
    };
    template <typename Out>
    struct FromWire {
        static auto Name() noexcept -> std::string_view;

        template <typename... Args>
        auto operator()(
            const api::Session& api,
            std::unique_ptr<opentxs::blockchain::p2p::bitcoin::Header> header,
            Args&&... args) const -> Out*;
    };
    template <typename Out>
    struct ToWire {
        static auto Name() noexcept -> std::string_view;

        template <typename... Args>
        auto operator()(
            const api::Session& api,
            opentxs::blockchain::Type chain,
            Args&&... args) const -> std::unique_ptr<Out>;
    };

    static constexpr auto default_protocol_version_ =
        opentxs::blockchain::p2p::bitcoin::ProtocolVersion{70015};
    static constexpr auto max_inv_ = 50000_uz;

    const opentxs::blockchain::node::internal::Mempool& mempool_;
    const CString user_agent_;
    const bool peer_cfilter_;
    const opentxs::blockchain::p2p::bitcoin::Nonce nonce_;
    const opentxs::blockchain::bitcoin::Inventory::Type inv_block_;
    const opentxs::blockchain::bitcoin::Inventory::Type inv_tx_;
    opentxs::blockchain::p2p::bitcoin::ProtocolVersion protocol_;
    UnallocatedSet<opentxs::blockchain::p2p::Service> local_services_;
    bool relay_;
    bool addr_v2_;
    Handshake handshake_;
    Verification verification_;

    static auto commands() noexcept -> const CommandMap&;
    static auto get_local_services(
        const opentxs::blockchain::p2p::bitcoin::ProtocolVersion version,
        const opentxs::blockchain::Type network,
        const opentxs::blockchain::node::internal::Config& config) noexcept
        -> UnallocatedSet<opentxs::blockchain::p2p::Service>;

    template <typename Incoming, typename... Args>
    auto instantiate(std::unique_ptr<HeaderType> header, Args&&... args) const
        noexcept(false) -> std::unique_ptr<Incoming>;

    auto check_handshake() noexcept -> void final;
    auto check_verification() noexcept -> void;
    auto extract_body_size(const zeromq::Frame& header) const noexcept
        -> std::size_t final;
    auto not_implemented(
        std::unique_ptr<HeaderType> header,
        zeromq::Frame&&,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_broadcasttx(Message&& msg) noexcept -> void final;
    auto process_protocol(Message&& message, allocator_type monotonic) noexcept
        -> void final;
    auto process_protocol_addr(
        std::unique_ptr<HeaderType> header,
        zeromq::Frame&& payload,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_addr2(
        std::unique_ptr<HeaderType> header,
        zeromq::Frame&& payload,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_block(
        std::unique_ptr<HeaderType> header,
        zeromq::Frame&& payload,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_blocktxn(
        std::unique_ptr<HeaderType> header,
        zeromq::Frame&& payload,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_cfcheckpt(
        std::unique_ptr<HeaderType> header,
        zeromq::Frame&& payload,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_cfheaders(
        std::unique_ptr<HeaderType> header,
        zeromq::Frame&& payload,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_cfheaders_verify(
        opentxs::blockchain::p2p::bitcoin::message::internal::Cfheaders&
            message) noexcept(false) -> void;
    auto process_protocol_cfilter(
        std::unique_ptr<HeaderType> header,
        zeromq::Frame&& payload,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_cmpctblock(
        std::unique_ptr<HeaderType> header,
        zeromq::Frame&& payload,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_feefilter(
        std::unique_ptr<HeaderType> header,
        zeromq::Frame&& payload,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_filteradd(
        std::unique_ptr<HeaderType> header,
        zeromq::Frame&& payload,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_filterclear(
        std::unique_ptr<HeaderType> header,
        zeromq::Frame&& payload,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_filterload(
        std::unique_ptr<HeaderType> header,
        zeromq::Frame&& payload,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_getaddr(
        std::unique_ptr<HeaderType> header,
        zeromq::Frame&& payload,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_getblocks(
        std::unique_ptr<HeaderType> header,
        zeromq::Frame&& payload,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_getblocktxn(
        std::unique_ptr<HeaderType> header,
        zeromq::Frame&& payload,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_getcfcheckpt(
        std::unique_ptr<HeaderType> header,
        zeromq::Frame&& payload,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_getcfheaders(
        std::unique_ptr<HeaderType> header,
        zeromq::Frame&& payload,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_getcfilters(
        std::unique_ptr<HeaderType> header,
        zeromq::Frame&& payload,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_getdata(
        std::unique_ptr<HeaderType> header,
        zeromq::Frame&& payload,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_getheaders(
        std::unique_ptr<HeaderType> header,
        zeromq::Frame&& payload,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_headers(
        std::unique_ptr<HeaderType> header,
        zeromq::Frame&& payload,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_headers_verify(
        opentxs::blockchain::p2p::bitcoin::message::internal::Headers&
            message) noexcept(false) -> void;
    auto process_protocol_headers_run(
        opentxs::blockchain::p2p::bitcoin::message::internal::Headers&
            message) noexcept(false) -> void;
    auto process_protocol_inv(
        std::unique_ptr<HeaderType> header,
        zeromq::Frame&& payload,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_mempool(
        std::unique_ptr<HeaderType> header,
        zeromq::Frame&& payload,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_merkleblock(
        std::unique_ptr<HeaderType> header,
        zeromq::Frame&& payload,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_notfound(
        std::unique_ptr<HeaderType> header,
        zeromq::Frame&& payload,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_ping(
        std::unique_ptr<HeaderType> header,
        zeromq::Frame&& payload,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_pong(
        std::unique_ptr<HeaderType> header,
        zeromq::Frame&& payload,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_reject(
        std::unique_ptr<HeaderType> header,
        zeromq::Frame&& payload,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_sendaddr2(
        std::unique_ptr<HeaderType> header,
        zeromq::Frame&& payload,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_sendcmpct(
        std::unique_ptr<HeaderType> header,
        zeromq::Frame&& payload,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_sendheaders(
        std::unique_ptr<HeaderType> header,
        zeromq::Frame&& payload,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_tx(
        std::unique_ptr<HeaderType> header,
        zeromq::Frame&& payload,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_verack(
        std::unique_ptr<HeaderType> header,
        zeromq::Frame&& payload,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_version(
        std::unique_ptr<HeaderType> header,
        zeromq::Frame&& payload,
        allocator_type monotonic) noexcept(false) -> void;
    auto reconcile_mempool() noexcept -> void;
    auto request_checkpoint_block_header() noexcept -> void;
    auto request_checkpoint_cfheader() noexcept -> void;
    auto transition_state_handshake() noexcept -> void final;
    auto transition_state_verify() noexcept -> void final;
    auto transmit_block_hash(opentxs::blockchain::block::Hash&& hash) noexcept
        -> void final;
    auto transmit_ping() noexcept -> void final;
    template <typename Outgoing, typename... Args>
    auto transmit_protocol(Args&&... args) noexcept -> void;
    auto transmit_protocol_block(const Data& serialized) noexcept -> void;
    auto transmit_protocol_cfheaders(
        opentxs::blockchain::cfilter::Type type,
        const opentxs::blockchain::block::Hash& stop,
        const opentxs::blockchain::cfilter::Header& previous,
        Vector<opentxs::blockchain::cfilter::Hash>&& hashes) noexcept -> void;
    auto transmit_protocol_cfilter(
        opentxs::blockchain::cfilter::Type type,
        const opentxs::blockchain::block::Hash& hash,
        const opentxs::blockchain::GCS& filter) noexcept -> void;
    auto transmit_protocol_getaddr() noexcept -> void;
    auto transmit_protocol_getcfheaders(
        const opentxs::blockchain::block::Height start,
        const opentxs::blockchain::block::Hash& stop) noexcept -> void;
    auto transmit_protocol_getcfilters(
        const opentxs::blockchain::block::Height start,
        const opentxs::blockchain::block::Hash& stop) noexcept -> void;
    auto transmit_protocol_getdata(
        opentxs::blockchain::bitcoin::Inventory&& item) noexcept -> void;
    auto transmit_protocol_getdata(
        UnallocatedVector<opentxs::blockchain::bitcoin::Inventory>&&
            items) noexcept -> void;
    auto transmit_protocol_getheaders() noexcept -> void;
    auto transmit_protocol_getheaders(
        const opentxs::blockchain::block::Hash& stop) noexcept -> void;
    auto transmit_protocol_getheaders(
        opentxs::blockchain::block::Hash&& parent,
        const opentxs::blockchain::block::Hash& stop) noexcept -> void;
    auto transmit_protocol_getheaders(
        Vector<opentxs::blockchain::block::Hash>&& history,
        const opentxs::blockchain::block::Hash& stop) noexcept -> void;
    auto transmit_protocol_getheaders(
        const Vector<opentxs::blockchain::block::Hash>& history) noexcept
        -> void;
    auto transmit_protocol_headers(
        UnallocatedVector<opentxs::blockchain::block::Header>&&
            headers) noexcept -> void;
    auto transmit_protocol_inv(
        opentxs::blockchain::bitcoin::Inventory&& inv) noexcept -> void;
    auto transmit_protocol_inv(
        UnallocatedVector<opentxs::blockchain::bitcoin::Inventory>&&
            inv) noexcept -> void;
    auto transmit_protocol_mempool() noexcept -> void;
    auto transmit_protocol_notfound(
        UnallocatedVector<opentxs::blockchain::bitcoin::Inventory>&&
            payload) noexcept -> void;
    auto transmit_protocol_ping() noexcept -> void;
    auto transmit_protocol_pong(
        const opentxs::blockchain::p2p::bitcoin::Nonce& nonce) noexcept -> void;
    auto transmit_protocol_sendaddr2() noexcept -> void;
    auto transmit_protocol_tx(ReadView serialized) noexcept -> void;
    auto transmit_protocol_verack() noexcept -> void;
    auto transmit_protocol_version() noexcept -> void;
    auto transmit_request_block_headers() noexcept -> void final;
    auto transmit_request_block_headers(
        const opentxs::blockchain::node::internal::HeaderJob& job) noexcept
        -> void final;
    auto transmit_request_blocks(
        opentxs::blockchain::node::internal::BlockBatch& job) noexcept
        -> void final;
    auto transmit_request_mempool() noexcept -> void final;
    auto transmit_request_peers() noexcept -> void final;
    auto transmit_txid(const Txid& txid) noexcept -> void final;
};
}  // namespace opentxs::network::blockchain::bitcoin
