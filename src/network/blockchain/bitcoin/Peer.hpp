// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <span>
#include <string_view>

#include "blockchain/bitcoin/Inventory.hpp"
#include "internal/blockchain/node/headeroracle/HeaderJob.hpp"
#include "internal/network/blockchain/Peer.hpp"
#include "internal/network/blockchain/bitcoin/message/Types.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/PMR.hpp"
#include "network/blockchain/peer/Imp.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/GCS.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Header.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/network/blockchain/Address.hpp"
#include "opentxs/network/blockchain/bitcoin/Types.hpp"
#include "opentxs/network/zeromq/Types.hpp"
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
}  // namespace blockchain

namespace network
{
namespace asio
{
class Socket;
}  // namespace asio

namespace blockchain
{
namespace bitcoin
{
namespace message
{
namespace internal
{
class Addr2;
class Addr;
class Block;
class Cfcheckpt;
class Cfheaders;
class Cfilter;
class Getaddr;
class Getblocks;
class Getcfcheckpt;
class Getcfheaders;
class Getcfilters;
class Getdata;
class Getheaders;
class Header;
class Headers;
class Inv;
class Mempool;
class Message;
class Notfound;
class Ping;
class Pong;
class Reject;
class Sendaddr2;
class Tx;
class Verack;
class Version;
}  // namespace internal
}  // namespace message
}  // namespace bitcoin
}  // namespace blockchain
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::blockchain::bitcoin
{
class Peer final : public blockchain::internal::Peer::Imp
{
public:
    auto get_deleter() noexcept -> delete_function final
    {
        return pmr::make_deleter(this);
    }

    Peer(
        std::shared_ptr<const api::Session> api,
        std::shared_ptr<const opentxs::blockchain::node::Manager> network,
        message::Nonce nonce,
        int peerID,
        blockchain::Address address,
        Set<network::blockchain::Address> gossip,
        message::ProtocolVersion protocol,
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
    using MessageType = message::internal::Message;
    using HeaderType = message::internal::Header;

    struct Handshake {
        bool got_version_{false};
        bool got_verack_{false};
    };
    struct Verification {
        bool got_block_header_{false};
        bool got_cfheader_{false};
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
        message::ProtocolVersion{70015};
    static constexpr auto max_inv_ = 50000_uz;

    const opentxs::blockchain::node::internal::Mempool& mempool_;
    const CString user_agent_;
    const bool peer_cfilter_;
    const message::Nonce nonce_;
    const opentxs::blockchain::bitcoin::Inventory::Type inv_block_;
    const opentxs::blockchain::bitcoin::Inventory::Type inv_tx_;
    const blockchain::Address local_address_;
    message::ProtocolVersion protocol_;
    bool bip37_;
    bool addr_v2_;
    bool can_gossip_zmq_;
    Handshake handshake_;
    Verification verification_;

    static auto get_local_services(
        const message::ProtocolVersion version,
        const opentxs::blockchain::Type network,
        const opentxs::blockchain::node::internal::Config& config,
        allocator_type alloc) noexcept
        -> Set<opentxs::network::blockchain::bitcoin::Service>;
    static auto is_implemented(message::Command) noexcept -> bool;

    auto can_gossip(const blockchain::Address& address) const noexcept -> bool;
    auto ignore_message(message::Command type) const noexcept -> bool;

    auto check_handshake(allocator_type monotonic) noexcept -> void final;
    auto check_verification(allocator_type monotonic) noexcept -> void;
    auto extract_body_size(const zeromq::Frame& header) const noexcept
        -> std::size_t final;
    auto process_addresses(
        std::span<Address> data,
        allocator_type monotonic) noexcept -> void;
    auto process_block_hash(
        const opentxs::blockchain::bitcoin::Inventory& inv,
        allocator_type monotonic) noexcept -> bool;
    auto process_block_hashes(
        std::span<opentxs::blockchain::bitcoin::Inventory> hashes,
        allocator_type monotonic) noexcept -> void;
    auto process_broadcasttx(Message&& msg, allocator_type monotonic) noexcept
        -> void final;
    auto process_protocol(Message&& message, allocator_type monotonic) noexcept
        -> void final;
    auto process_protocol(
        message::internal::Addr& message,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol(
        message::internal::Addr2& message,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol(
        message::internal::Block& message,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol(
        message::internal::Cfcheckpt& message,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol(
        message::internal::Cfheaders& message,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_verify(
        message::internal::Cfheaders& message,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol(
        message::internal::Cfilter& message,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol(
        message::internal::Getaddr& message,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol(
        message::internal::Getblocks& message,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol(
        message::internal::Getcfcheckpt& message,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol(
        message::internal::Getcfheaders& message,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol(
        message::internal::Getcfilters& message,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol(
        message::internal::Getdata& message,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol(
        message::internal::Getheaders& message,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol(
        message::internal::Headers& message,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_verify(
        message::internal::Headers& message,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol_run(
        message::internal::Headers& message,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol(
        message::internal::Inv& message,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol(
        message::internal::Mempool& message,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol(
        message::internal::Notfound& message,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol(
        message::internal::Ping& message,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol(
        message::internal::Pong& message,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol(
        message::internal::Reject& message,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol(
        message::internal::Sendaddr2& message,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol(
        message::internal::Tx& message,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol(
        message::internal::Verack& message,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_protocol(
        message::internal::Version& message,
        allocator_type monotonic) noexcept(false) -> void;
    auto process_transaction_hashes(
        std::span<opentxs::blockchain::bitcoin::Inventory> hashes,
        allocator_type monotonic) noexcept -> void;
    auto reconcile_mempool(allocator_type monotonic) noexcept -> void;
    auto request_checkpoint_block_header(allocator_type monotonic) noexcept
        -> void;
    auto request_checkpoint_cfheader(allocator_type monotonic) noexcept -> void;
    auto transition_state_handshake(allocator_type monotonic) noexcept
        -> void final;
    auto transition_state_verify(allocator_type monotonic) noexcept
        -> void final;
    auto transmit_addresses(
        std::span<network::blockchain::Address> addresses,
        allocator_type monotonic) noexcept -> void final;
    auto transmit_block_hash(
        opentxs::blockchain::block::Hash&& hash,
        allocator_type monotonic) noexcept -> void final;
    auto transmit_ping(allocator_type monotonic) noexcept -> void final;
    template <typename Outgoing, typename... Args>
    auto transmit_protocol(allocator_type monotonic, Args&&... args) noexcept
        -> void;
    auto transmit_protocol_addr(
        std::span<network::blockchain::Address> addresses,
        allocator_type monotonic) noexcept -> void;
    auto transmit_protocol_addr2(
        std::span<network::blockchain::Address> addresses,
        allocator_type monotonic) noexcept -> void;
    auto transmit_protocol_block(
        const ReadView serialized,
        allocator_type monotonic) noexcept -> void;
    auto transmit_protocol_cfheaders(
        opentxs::blockchain::cfilter::Type type,
        const opentxs::blockchain::block::Hash& stop,
        const opentxs::blockchain::cfilter::Header& previous,
        std::span<opentxs::blockchain::cfilter::Hash> hashes,
        allocator_type monotonic) noexcept -> void;
    auto transmit_protocol_cfilter(
        opentxs::blockchain::cfilter::Type type,
        const opentxs::blockchain::block::Hash& hash,
        const opentxs::blockchain::GCS& filter,
        allocator_type monotonic) noexcept -> void;
    auto transmit_protocol_getaddr(allocator_type monotonic) noexcept -> void;
    auto transmit_protocol_getcfheaders(
        const opentxs::blockchain::block::Height start,
        const opentxs::blockchain::block::Hash& stop,
        allocator_type monotonic) noexcept -> void;
    auto transmit_protocol_getcfilters(
        const opentxs::blockchain::block::Height start,
        const opentxs::blockchain::block::Hash& stop,
        allocator_type monotonic) noexcept -> void;
    auto transmit_protocol_getdata(
        opentxs::blockchain::bitcoin::Inventory&& item,
        allocator_type monotonic) noexcept -> void;
    auto transmit_protocol_getdata(
        std::span<opentxs::blockchain::bitcoin::Inventory> items,
        allocator_type monotonic) noexcept -> void;
    auto transmit_protocol_getheaders(allocator_type monotonic) noexcept
        -> void;
    auto transmit_protocol_getheaders(
        const opentxs::blockchain::block::Hash& stop,
        allocator_type monotonic) noexcept -> void;
    auto transmit_protocol_getheaders(
        opentxs::blockchain::block::Hash&& parent,
        const opentxs::blockchain::block::Hash& stop,
        allocator_type monotonic) noexcept -> void;
    auto transmit_protocol_getheaders(
        std::span<opentxs::blockchain::block::Hash> history,
        const opentxs::blockchain::block::Hash& stop,
        allocator_type monotonic) noexcept -> void;
    auto transmit_protocol_getheaders(
        std::span<opentxs::blockchain::block::Hash> history,
        allocator_type monotonic) noexcept -> void;
    auto transmit_protocol_headers(
        std::span<opentxs::blockchain::block::Header> headers,
        allocator_type monotonic) noexcept -> void;
    auto transmit_protocol_inv(
        opentxs::blockchain::bitcoin::Inventory&& inv,
        allocator_type monotonic) noexcept -> void;
    auto transmit_protocol_inv(
        std::span<opentxs::blockchain::bitcoin::Inventory> inv,
        allocator_type monotonic) noexcept -> void;
    auto transmit_protocol_mempool(allocator_type monotonic) noexcept -> void;
    auto transmit_protocol_notfound(
        std::span<opentxs::blockchain::bitcoin::Inventory> payload,
        allocator_type monotonic) noexcept -> void;
    auto transmit_protocol_ping(allocator_type monotonic) noexcept -> void;
    auto transmit_protocol_pong(
        const message::Nonce& nonce,
        allocator_type monotonic) noexcept -> void;
    auto transmit_protocol_sendaddr2(allocator_type monotonic) noexcept -> void;
    auto transmit_protocol_tx(
        ReadView serialized,
        allocator_type monotonic) noexcept -> void;
    auto transmit_protocol_verack(allocator_type monotonic) noexcept -> void;
    auto transmit_protocol_version(allocator_type monotonic) noexcept -> void;
    auto transmit_request_block_headers(allocator_type monotonic) noexcept
        -> void final;
    auto transmit_request_block_headers(
        const opentxs::blockchain::node::internal::HeaderJob& job,
        allocator_type monotonic) noexcept -> void final;
    auto transmit_request_blocks(
        opentxs::blockchain::node::internal::BlockBatch& job,
        allocator_type monotonic) noexcept -> void final;
    auto transmit_request_mempool(allocator_type monotonic) noexcept
        -> void final;
    auto transmit_request_peers(allocator_type monotonic) noexcept
        -> void final;
    auto transmit_txid(const Txid& txid, allocator_type monotonic) noexcept
        -> void final;
};
}  // namespace opentxs::network::blockchain::bitcoin
