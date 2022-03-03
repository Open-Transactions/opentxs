// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <chrono>
#include <cstddef>
#include <future>
#include <iosfwd>
#include <memory>
#include <type_traits>

#include "blockchain/p2p/bitcoin/Header.hpp"
#include "blockchain/p2p/bitcoin/Message.hpp"
#include "blockchain/p2p/peer/Peer.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/p2p/bitcoin/message/Message.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace bitcoin
{
class Inventory;
}  // namespace bitcoin

namespace node
{
namespace internal
{
struct BlockOracle;
struct Config;
struct FilterOracle;
struct Mempool;
struct Network;
struct PeerManager;
}  // namespace internal

class HeaderOracle;
}  // namespace node

namespace p2p
{
namespace internal
{
struct Address;
}  // namespace internal
}  // namespace p2p
}  // namespace blockchain

namespace network
{
namespace zeromq
{
class Frame;
class Message;
}  // namespace zeromq
}  // namespace network
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::p2p::bitcoin::implementation
{
class Peer final : public p2p::implementation::Peer
{
public:
    using MessageType = bitcoin::Message;
    using HeaderType = bitcoin::Header;

    Peer(
        const api::Session& api,
        const node::internal::Config& config,
        const node::internal::Mempool& mempool,
        const node::internal::Network& network,
        const node::HeaderOracle& header,
        const node::internal::FilterOracle& filter,
        const node::internal::BlockOracle& block,
        const node::internal::PeerManager& manager,
        const database::BlockStorage policy,
        const UnallocatedCString& shutdown,
        const int id,
        std::unique_ptr<internal::Address> address,
        const bool relay = true,
        const UnallocatedSet<p2p::Service>& localServices = {},
        const ProtocolVersion protocol = 0) noexcept;

    ~Peer() final;

private:
    using CommandFunction =
        void (Peer::*)(std::unique_ptr<HeaderType>, const zmq::Frame&);
    using Nonce = bitcoin::Nonce;

    struct Request {
    public:
        auto Finish() noexcept -> void
        {
            try {
                promise_.set_value();
            } catch (...) {
            }
        }

        auto Running() noexcept -> bool
        {
            static const auto limit = 10s;

            if ((Clock::now() - start_) > limit) { return false; }

            return std::future_status::timeout == future_.wait_for(1ms);
        }

        auto Start() noexcept -> void
        {
            promise_ = {};
            future_ = promise_.get_future();
            start_ = Clock::now();
        }

    private:
        std::promise<void> promise_{};
        std::future<void> future_{};
        Time start_{};
    };

    static const UnallocatedMap<Command, CommandFunction> command_map_;
    static const ProtocolVersion default_protocol_version_{70015};
    static const UnallocatedCString user_agent_;

    const node::HeaderOracle& headers_;
    std::atomic<ProtocolVersion> protocol_;
    const Nonce nonce_;
    const UnallocatedSet<p2p::Service> local_services_;
    std::atomic<bool> relay_;
    Request get_headers_;

    static auto get_local_services(
        const ProtocolVersion version,
        const blockchain::Type network,
        const database::BlockStorage policy,
        const UnallocatedSet<p2p::Service>& input) noexcept
        -> UnallocatedSet<p2p::Service>;
    static auto nonce(const api::Session& api) noexcept -> Nonce;

    auto broadcast_inv(
        UnallocatedVector<blockchain::bitcoin::Inventory>&& inv) noexcept
        -> void;
    auto get_body_size(const zmq::Frame& header) const noexcept
        -> std::size_t final;

    auto broadcast_block(zmq::Message&& message) noexcept -> void final;
    auto broadcast_inv_transaction(ReadView txid) noexcept -> void final;
    auto broadcast_transaction(zmq::Message&& message) noexcept -> void final;
    auto ping() noexcept -> void final;
    auto pong(Nonce) noexcept -> void final;
    auto process_message(zmq::Message&& message) noexcept -> void final;
    auto reconcile_mempool() noexcept -> void;
    auto request_addresses() noexcept -> void final;
    auto request_block(zmq::Message&& message) noexcept -> void final;
    auto request_blocks() noexcept -> void final;
    auto request_cfheaders() noexcept -> void final;
    auto request_cfilter() noexcept -> void final;
    auto request_checkpoint_block_header() noexcept -> void final;
    auto request_checkpoint_filter_header() noexcept -> void final;
    using p2p::implementation::Peer::request_headers;
    auto request_headers() noexcept -> void final;
    auto request_headers(const block::Hash& hash) noexcept -> void;
    auto request_mempool() noexcept -> void final;
    auto request_transactions(
        UnallocatedVector<blockchain::bitcoin::Inventory>&&) noexcept -> void;
    auto start_handshake() noexcept -> void final;

    auto process_addr(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload) -> void;
    auto process_block(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload) -> void;
    auto process_blocktxn(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload) -> void;
    auto process_cfcheckpt(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload) -> void;
    auto process_cfheaders(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload) -> void;
    auto process_cfilter(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload) -> void;
    auto process_cmpctblock(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload) -> void;
    auto process_feefilter(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload) -> void;
    auto process_filteradd(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload) -> void;
    auto process_filterclear(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload) -> void;
    auto process_filterload(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload) -> void;
    auto process_getaddr(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload) -> void;
    auto process_getblocks(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload) -> void;
    auto process_getblocktxn(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload) -> void;
    auto process_getcfcheckpt(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload) -> void;
    auto process_getcfheaders(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload) -> void;
    auto process_getcfilters(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload) -> void;
    auto process_getdata(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload) -> void;
    auto process_getheaders(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload) -> void;
    auto process_headers(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload) -> void;
    auto process_inv(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload) -> void;
    auto process_mempool(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload) -> void;
    auto process_merkleblock(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload) -> void;
    auto process_notfound(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload) -> void;
    auto process_ping(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload) -> void;
    auto process_pong(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload) -> void;
    auto process_reject(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload) -> void;
    auto process_sendcmpct(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload) -> void;
    auto process_sendheaders(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload) -> void;
    auto process_tx(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload) -> void;
    auto process_verack(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload) -> void;
    auto process_version(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload) -> void;

    Peer() = delete;
    Peer(const Peer&) = delete;
    Peer(Peer&&) = delete;
    auto operator=(const Peer&) -> Peer& = delete;
    auto operator=(Peer&&) -> Peer& = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::implementation
