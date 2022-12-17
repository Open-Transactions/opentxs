// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/util/Writer.hpp"

#pragma once

#include <ankerl/unordered_dense.h>
#include <boost/smart_ptr/shared_ptr.hpp>
#include <chrono>
#include <cstddef>
#include <memory>
#include <optional>
#include <string_view>
#include <utility>
#include <variant>

#include "internal/blockchain/node/blockoracle/BlockBatch.hpp"  // IWYU pragma: keep
#include "internal/blockchain/node/headeroracle/HeaderJob.hpp"
#include "internal/network/blockchain/Peer.hpp"
#include "internal/network/blockchain/Types.hpp"
#include "internal/network/zeromq/Types.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/p2p/Address.hpp"
#include "opentxs/blockchain/p2p/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"
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
namespace internal
{
struct Config;
}  // namespace internal

class BlockOracle;
class FilterOracle;
class HeaderOracle;
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
class ConnectionManager;
}  // namespace blockchain

namespace zeromq
{
namespace socket
{
class Raw;
}  // namespace socket

class Frame;
}  // namespace zeromq
}  // namespace network

class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::network::blockchain::internal
{
using namespace std::literals;

class Peer::Imp : public Actor<Imp, PeerJob>
{
public:
    auto Init(boost::shared_ptr<Imp> me) noexcept -> void;

    Imp() = delete;
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;

    ~Imp() override;

private:
    std::shared_ptr<const api::Session> api_p_;
    std::shared_ptr<const opentxs::blockchain::node::Manager> network_p_;

protected:
    enum class Dir : bool { incoming = true, outgoing = false };
    enum class State {
        pre_init,
        init,
        connect,
        handshake,
        verify,
        run,
        shutdown,
    };

    using Txid = opentxs::blockchain::block::TransactionHash;

    const api::Session& api_;
    const opentxs::blockchain::node::Manager& network_;
    const opentxs::blockchain::node::internal::Config& config_;
    const opentxs::blockchain::node::HeaderOracle& header_oracle_;
    const opentxs::blockchain::node::BlockOracle& block_oracle_;
    const opentxs::blockchain::node::FilterOracle& filter_oracle_;
    const opentxs::blockchain::Type chain_;
    const Dir dir_;
    opentxs::blockchain::database::Peer& database_;
    network::zeromq::socket::Raw& to_block_oracle_;
    network::zeromq::socket::Raw& to_header_oracle_;
    network::zeromq::socket::Raw& to_peer_manager_;

    static auto print_state(State) noexcept -> std::string_view;

    auto address() const noexcept -> const opentxs::blockchain::p2p::Address&
    {
        return address_;
    }
    auto connection() const noexcept -> const ConnectionManager&
    {
        return connection_;
    }
    auto get_known_tx(alloc::Default alloc = {}) const noexcept -> Set<Txid>;
    auto state() const noexcept -> State { return state_; }

    auto add_known_block(opentxs::blockchain::block::Hash) noexcept -> bool;
    auto add_known_tx(const Txid& txid) noexcept -> bool;
    auto add_known_tx(Txid&& txid) noexcept -> bool;
    auto cancel_timers() noexcept -> void;
    virtual auto check_handshake() noexcept -> void = 0;
    auto disconnect(std::string_view why) noexcept -> void;
    auto finish_job(bool shutdown = false) noexcept -> void;
    auto reset_peers_timer() noexcept -> void;
    auto run_job() noexcept -> void;
    auto set_block_header_capability(bool value) noexcept -> void;
    auto set_cfilter_capability(bool value) noexcept -> void;
    auto transition_state(
        State state,
        std::optional<std::chrono::microseconds> timeout = {}) noexcept -> void;
    virtual auto transition_state_handshake() noexcept -> void;
    auto transition_state_run() noexcept -> void;
    virtual auto transition_state_verify() noexcept -> void;
    auto transmit(std::pair<zeromq::Frame, zeromq::Frame>&& data) noexcept
        -> void;
    auto update_address(const UnallocatedSet<opentxs::blockchain::p2p::Service>&
                            services) noexcept -> void;
    auto update_block_job(const ReadView block) noexcept -> bool;
    auto update_get_headers_job() noexcept -> void;
    auto update_position(
        opentxs::blockchain::block::Position& target,
        opentxs::blockchain::block::Position pos) noexcept -> void;
    auto update_remote_position(
        opentxs::blockchain::block::Position pos) noexcept -> void;

    Imp(std::shared_ptr<const api::Session> api,
        std::shared_ptr<const opentxs::blockchain::node::Manager> network,
        int peerID,
        opentxs::blockchain::p2p::Address address,
        std::chrono::milliseconds pingInterval,
        std::chrono::milliseconds inactivityInterval,
        std::chrono::milliseconds peersInterval,
        std::size_t headerBytes,
        std::string_view fromParent,
        std::optional<asio::Socket> socket,
        zeromq::BatchID batch,
        allocator_type alloc) noexcept;

private:
    class HasJob;
    class JobType;
    class RunJob;
    class UpdateBlockJob;
    class UpdateGetHeadersJob;

    friend Actor<Imp, PeerJob>;
    friend RunJob;
    friend UpdateBlockJob;
    friend UpdateGetHeadersJob;

    using KnownHashes = ankerl::unordered_dense::set<Txid>;
    using KnownBlocks =
        ankerl::unordered_dense::set<opentxs::blockchain::block::Hash>;
    using Job = std::variant<
        std::monostate,
        opentxs::blockchain::node::internal::HeaderJob,
        opentxs::blockchain::node::internal::BlockBatch>;
    using IsJob = bool;
    using IsFinished = bool;
    using JobUpdate = std::pair<IsJob, IsFinished>;

    static constexpr auto job_timeout_ = 2min;

    const int id_;
    const std::size_t untrusted_connection_id_;
    const std::chrono::milliseconds ping_interval_;
    const std::chrono::milliseconds inactivity_interval_;
    const std::chrono::milliseconds peers_interval_;
    opentxs::blockchain::p2p::Address address_;
    std::unique_ptr<ConnectionManager> connection_p_;
    ConnectionManager& connection_;
    State state_;
    Time last_activity_;
    Timer state_timer_;
    Timer ping_timer_;
    Timer activity_timer_;
    Timer peers_timer_;
    Timer job_timer_;
    KnownHashes known_transactions_;
    KnownBlocks known_blocks_;
    opentxs::blockchain::block::Position local_position_;
    opentxs::blockchain::block::Position remote_position_;
    Job job_;
    bool is_caught_up_;
    bool block_header_capability_;
    bool cfilter_capability_;

    static auto init_connection_manager(
        const api::Session& api,
        const opentxs::blockchain::node::Manager& node,
        const Imp& parent,
        const opentxs::blockchain::p2p::Address& address,
        const Log& log,
        int id,
        std::size_t headerBytes,
        std::optional<asio::Socket> socket) noexcept
        -> std::unique_ptr<ConnectionManager>;
    template <typename J>
    static auto job_name(const J& job) noexcept -> std::string_view;

    auto has_job() const noexcept -> bool;
    auto is_allowed_state(Work work) const noexcept -> bool;
    auto job_name() const noexcept -> std::string_view;

    auto check_jobs() noexcept -> void;
    auto check_positions() noexcept -> void;
    auto connect() noexcept -> void;
    auto connect_dealer(std::string_view endpoint, Work work) noexcept -> void;
    auto do_disconnect() noexcept -> void;
    auto do_shutdown() noexcept -> void;
    auto do_startup(allocator_type monotonic) noexcept -> bool;
    virtual auto extract_body_size(const zeromq::Frame& header) const noexcept
        -> std::size_t = 0;
    auto pipeline(const Work work, Message&& msg, allocator_type) noexcept
        -> void;
    auto pipeline_trusted(
        const Work work,
        Message&& msg,
        allocator_type monotonic) noexcept -> void;
    auto pipeline_untrusted(
        const Work work,
        Message&& msg,
        allocator_type monotonic) noexcept -> void;
    auto process_activitytimeout(Message&& msg) noexcept -> void;
    auto process_block(Message&& msg) noexcept -> void;
    auto process_block(opentxs::blockchain::block::Hash&& hash) noexcept
        -> void;
    auto process_blockheader(Message&& msg) noexcept -> void;
    auto process_body(Message&& msg, allocator_type monotonic) noexcept -> void;
    virtual auto process_broadcasttx(Message&& msg) noexcept -> void = 0;
    auto process_connect() noexcept -> void;
    auto process_connect(bool) noexcept -> void;
    auto process_dealerconnected(Message&& msg) noexcept -> void;
    auto process_disconnect(Message&& msg) noexcept -> void;
    auto process_header(Message&& msg, allocator_type monotonic) noexcept
        -> void;
    auto process_jobavailableblock(Message&& msg) noexcept -> void;
    auto process_jobavailablegetheaders(Message&& msg) noexcept -> void;
    auto process_jobtimeout(Message&& msg) noexcept -> void;
    auto process_mempool(Message&& msg) noexcept -> void;
    auto process_needpeers(Message&& msg) noexcept -> void;
    auto process_needping(Message&& msg) noexcept -> void;
    auto process_p2p(Message&& msg, allocator_type monotonic) noexcept -> void;
    virtual auto process_protocol(
        Message&& message,
        allocator_type monotonic) noexcept -> void = 0;
    auto process_registration(Message&& msg) noexcept -> void;
    auto process_reorg(Message&& msg) noexcept -> void;
    auto process_sendresult(Message&& msg) noexcept -> void;
    auto process_statetimeout(Message&& msg) noexcept -> void;
    auto reset_activity_timer() noexcept -> void;
    auto reset_job_timer() noexcept -> void;
    auto reset_peers_timer(std::chrono::microseconds value) noexcept -> void;
    auto reset_ping_timer() noexcept -> void;
    auto reset_state_timer(std::chrono::microseconds value) noexcept -> void;
    auto transition_state_connect() noexcept -> void;
    auto transition_state_init() noexcept -> void;
    auto transition_state_shutdown() noexcept -> void;
    auto transmit(Message&& message) noexcept -> void;
    virtual auto transmit_block_hash(
        opentxs::blockchain::block::Hash&& hash) noexcept -> void = 0;
    virtual auto transmit_ping() noexcept -> void = 0;
    virtual auto transmit_request_block_headers() noexcept -> void = 0;
    virtual auto transmit_request_block_headers(
        const opentxs::blockchain::node::internal::HeaderJob& job) noexcept
        -> void = 0;
    virtual auto transmit_request_blocks(
        opentxs::blockchain::node::internal::BlockBatch& job) noexcept
        -> void = 0;
    virtual auto transmit_request_mempool() noexcept -> void = 0;
    virtual auto transmit_request_peers() noexcept -> void = 0;
    virtual auto transmit_txid(const Txid& txid) noexcept -> void = 0;
    auto update_activity() noexcept -> void;
    auto update_address() noexcept -> void;
    template <typename Visitor>
    auto update_job(Visitor& visitor) noexcept -> bool;
    auto update_local_position(
        opentxs::blockchain::block::Position pos) noexcept -> void;
    auto work(allocator_type monotonic) noexcept -> bool;
};
}  // namespace opentxs::network::blockchain::internal
