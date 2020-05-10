// Copyright (c) 2010-2020 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: private
// IWYU pragma: friend ".*src/blockchain/p2p/bitcoin/Peer.cpp"

#pragma once

#include <atomic>
#include <chrono>
#include <future>
#include <iosfwd>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <type_traits>

#include "blockchain/p2p/Peer.hpp"
#include "blockchain/p2p/bitcoin/Header.hpp"
#include "blockchain/p2p/bitcoin/Message.hpp"
#include "internal/blockchain/client/Client.hpp"
#include "internal/blockchain/p2p/bitcoin/Bitcoin.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"

namespace opentxs
{
namespace api
{
namespace internal
{
struct Core;
}  // namespace internal
}  // namespace api

namespace blockchain
{
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

class Factory;
}  // namespace opentxs

namespace opentxs::blockchain::p2p::bitcoin::implementation
{
class Peer final : public p2p::implementation::Peer
{
public:
    using MessageType = bitcoin::Message;
    using HeaderType = bitcoin::Header;

    ~Peer() final;

private:
    friend opentxs::Factory;

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
            static const auto limit = std::chrono::seconds(10);

            if ((Clock::now() - start_) > limit) { return false; }

            return std::future_status::timeout ==
                   future_.wait_for(std::chrono::milliseconds(1));
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

    static const std::map<Command, CommandFunction> command_map_;
    static const ProtocolVersion default_protocol_version_{70015};
    static const std::string user_agent_;

    const blockchain::Type chain_;
    std::atomic<ProtocolVersion> protocol_;
    const Nonce nonce_;
    const Magic magic_;
    const std::set<p2p::Service> local_services_;
    std::atomic<bool> relay_;
    Request get_headers_;

    static auto get_local_services(
        const ProtocolVersion version,
        const blockchain::Type network,
        const std::set<p2p::Service>& input) noexcept -> std::set<p2p::Service>;
    static auto nonce(const api::internal::Core& api) noexcept -> Nonce;

    auto get_body_size(const zmq::Frame& header) const noexcept
        -> std::size_t final;

    void ping() noexcept final;
    void pong() noexcept final;
    void process_message(const zmq::Message& message) noexcept final;
    void request_addresses() noexcept final;
    void request_block(zmq::Message& message) noexcept final;
    void request_cfheaders(zmq::Message& message) noexcept final;
    void request_cfilter(zmq::Message& message) noexcept final;
    using p2p::implementation::Peer::request_headers;
    void request_headers() noexcept final;
    void request_headers(const block::Hash& hash) noexcept;
    void start_handshake() noexcept final;
    void start_verify() noexcept final;

    void process_addr(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload);
    void process_block(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload);
    void process_blocktxn(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload);
    void process_cfcheckpt(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload);
    void process_cfheaders(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload);
    void process_cfilter(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload);
    void process_cmpctblock(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload);
    void process_feefilter(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload);
    void process_filteradd(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload);
    void process_filterclear(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload);
    void process_filterload(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload);
    void process_getaddr(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload);
    void process_getblocks(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload);
    void process_getblocktxn(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload);
    void process_getcfcheckpt(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload);
    void process_getcfheaders(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload);
    void process_getcfilters(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload);
    void process_getdata(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload);
    void process_getheaders(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload);
    void process_headers(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload);
    void process_inv(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload);
    void process_mempool(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload);
    void process_merkleblock(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload);
    void process_notfound(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload);
    void process_ping(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload);
    void process_pong(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload);
    void process_reject(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload);
    void process_sendcmpct(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload);
    void process_sendheaders(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload);
    void process_tx(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload);
    void process_verack(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload);
    void process_version(
        std::unique_ptr<HeaderType> header,
        const zmq::Frame& payload);

    Peer(
        const api::internal::Core& api,
        const client::internal::Network& network,
        const client::internal::PeerManager& manager,
        const blockchain::client::internal::IO& io,
        const std::string& shutdown,
        const int id,
        std::unique_ptr<internal::Address> address,
        const bool relay = true,
        const std::set<p2p::Service>& localServices = {},
        const ProtocolVersion protocol = 0) noexcept;

    Peer() = delete;
    Peer(const Peer&) = delete;
    Peer(Peer&&) = delete;
    auto operator=(const Peer&) -> Peer& = delete;
    auto operator=(Peer &&) -> Peer& = delete;
};
}  // namespace opentxs::blockchain::p2p::bitcoin::implementation
