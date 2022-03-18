// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>
#include <cstddef>
#include <future>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <utility>

#include "blockchain/node/Mempool.hpp"
#include "core/Shutdown.hpp"
#include "core/Worker.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/node/HeaderOracle.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/blockchain/node/Wallet.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Pipeline.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/Pair.hpp"
#include "opentxs/network/zeromq/socket/Publish.hpp"
#include "opentxs/network/zeromq/socket/Subscribe.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/WorkType.hpp"
#include "util/Work.hpp"

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
namespace block
{
namespace bitcoin
{
class Block;
class Transaction;
}  // namespace bitcoin

class Header;
}  // namespace block

namespace database
{
namespace common
{
class Database;
}  // namespace common
}  // namespace database

namespace node
{
namespace base
{
class SyncServer;
}  // namespace base

namespace p2p
{
class Requestor;
}  // namespace p2p
}  // namespace node

namespace p2p
{
class Address;
}  // namespace p2p
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket

class Message;
}  // namespace zeromq
}  // namespace network

class PaymentCode;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace zmq = opentxs::network::zeromq;

namespace opentxs::blockchain::node::implementation
{
class Base : virtual public node::internal::Network,
             public Worker<Base, api::Session>
{
public:
    enum class Work : OTZMQWorkType {
        shutdown = value(WorkType::Shutdown),
        heartbeat = OT_ZMQ_HEARTBEAT_SIGNAL,
        filter = OT_ZMQ_NEW_FILTER_SIGNAL,
        statemachine = OT_ZMQ_STATE_MACHINE_SIGNAL,
    };

    const Type chain_;

    auto AddBlock(const std::shared_ptr<const block::bitcoin::Block> block)
        const noexcept -> bool final;
    auto AddPeer(const blockchain::p2p::Address& address) const noexcept
        -> bool final;
    auto BlockOracle() const noexcept
        -> const node::internal::BlockOracle& final
    {
        return *block_p_;
    }
    auto BroadcastTransaction(
        const block::bitcoin::Transaction& tx,
        const bool pushtx) const noexcept -> bool final;
    auto Chain() const noexcept -> Type final { return chain_; }
    auto FeeRate() const noexcept -> Amount final;
    auto FilterOracleInternal() const noexcept
        -> const node::internal::FilterOracle& final
    {
        return *filter_p_;
    }
    auto GetBalance() const noexcept -> Balance final
    {
        return database_.GetBalance();
    }
    auto GetBalance(const identifier::Nym& owner) const noexcept
        -> Balance final
    {
        return database_.GetBalance(owner);
    }
    auto GetConfirmations(const UnallocatedCString& txid) const noexcept
        -> ChainHeight final;
    auto GetHeight() const noexcept -> ChainHeight final
    {
        return local_chain_height_.load();
    }
    auto GetPeerCount() const noexcept -> std::size_t final;
    auto GetTransactions() const noexcept
        -> UnallocatedVector<block::pTxid> final;
    auto GetTransactions(const identifier::Nym& account) const noexcept
        -> UnallocatedVector<block::pTxid> final;
    auto GetType() const noexcept -> Type final { return chain_; }
    auto GetVerifiedPeerCount() const noexcept -> std::size_t final;
    auto HeaderOracle() const noexcept -> const node::HeaderOracle& final
    {
        return header_;
    }
    auto IsSynchronized() const noexcept -> bool final
    {
        return is_synchronized_headers();
    }
    auto JobReady(const node::internal::PeerManager::Task type) const noexcept
        -> void final;
    auto Internal() const noexcept -> const internal::Network& final
    {
        return *this;
    }
    auto Listen(const blockchain::p2p::Address& address) const noexcept
        -> bool final;
    auto Mempool() const noexcept -> const internal::Mempool& final
    {
        return mempool_;
    }
    auto Reorg() const noexcept
        -> const network::zeromq::socket::Publish& final;
    auto RequestBlock(const block::Hash& block) const noexcept -> bool final;
    auto RequestBlocks(const UnallocatedVector<ReadView>& hashes) const noexcept
        -> bool final;
    auto SendToAddress(
        const opentxs::identifier::Nym& sender,
        const UnallocatedCString& address,
        const Amount amount,
        const UnallocatedCString& memo) const noexcept -> PendingOutgoing final;
    auto SendToPaymentCode(
        const opentxs::identifier::Nym& sender,
        const UnallocatedCString& recipient,
        const Amount amount,
        const UnallocatedCString& memo) const noexcept -> PendingOutgoing final;
    auto SendToPaymentCode(
        const opentxs::identifier::Nym& sender,
        const PaymentCode& recipient,
        const Amount amount,
        const UnallocatedCString& memo) const noexcept -> PendingOutgoing final;
    auto Submit(network::zeromq::Message&& work) const noexcept -> void final;
    auto SyncTip() const noexcept -> block::Position final;
    auto Track(network::zeromq::Message&& work) const noexcept
        -> std::future<void> final;
    auto UpdateHeight(const block::Height height) const noexcept -> void final;
    auto UpdateLocalHeight(const block::Position position) const noexcept
        -> void final;

    auto Connect() noexcept -> bool final;
    auto Disconnect() noexcept -> bool final;
    auto FilterOracleInternal() noexcept -> node::internal::FilterOracle& final
    {
        return filters_;
    }
    auto Shutdown() noexcept -> std::shared_future<void> final
    {
        return signal_shutdown();
    }
    auto StartWallet() noexcept -> void final;
    auto Wallet() const noexcept -> const node::Wallet& final
    {
        return wallet_;
    }

    ~Base() override;

private:
    const cfilter::Type filter_type_;
    opentxs::internal::ShutdownSender shutdown_sender_;
    std::unique_ptr<blockchain::internal::Database> database_p_;
    const node::internal::Config& config_;
    node::Mempool mempool_;
    std::unique_ptr<node::HeaderOracle> header_p_;
    std::unique_ptr<node::internal::BlockOracle> block_p_;
    std::unique_ptr<node::internal::FilterOracle> filter_p_;
    std::unique_ptr<node::internal::PeerManager> peer_p_;
    std::unique_ptr<node::internal::Wallet> wallet_p_;

protected:
    blockchain::internal::Database& database_;
    node::internal::FilterOracle& filters_;
    node::HeaderOracle& header_;
    node::internal::PeerManager& peer_;
    node::internal::BlockOracle& block_;
    node::internal::Wallet& wallet_;

    // NOTE call init in every final constructor body
    auto init() noexcept -> void;

    Base(
        const api::Session& api,
        const Type type,
        const node::internal::Config& config,
        const UnallocatedCString& seednode,
        const UnallocatedCString& syncEndpoint) noexcept;

private:
    friend Worker<Base, api::Session>;

    enum class State : int {
        UpdatingHeaders,
        UpdatingBlocks,
        UpdatingFilters,
        UpdatingSyncData,
        Normal
    };

    struct WorkPromises {
        auto clear(int index) noexcept -> void
        {
            auto lock = Lock{lock_};
            auto it = map_.find(index);

            if (map_.end() == it) { return; }

            it->second.set_value();
            map_.erase(it);
        }
        auto get() noexcept -> std::pair<int, std::future<void>>
        {
            auto lock = Lock{lock_};
            const auto counter = ++counter_;
            auto& promise = map_[counter];

            return std::make_pair(counter, promise.get_future());
        }

    private:
        std::mutex lock_{};
        int counter_{-1};
        UnallocatedMap<int, std::promise<void>> map_{};
    };
    struct SendPromises {
        auto finish(int index) noexcept -> std::promise<SendOutcome>
        {
            auto lock = Lock{lock_};
            auto it = map_.find(index);

            OT_ASSERT(map_.end() != it);

            auto output{std::move(it->second)};
            map_.erase(it);

            return output;
        }
        auto get() noexcept -> std::pair<int, PendingOutgoing>
        {
            auto lock = Lock{lock_};
            const auto counter = ++counter_;
            auto& promise = map_[counter];

            return std::make_pair(counter, promise.get_future());
        }

    private:
        std::mutex lock_{};
        int counter_{-1};
        UnallocatedMap<int, std::promise<SendOutcome>> map_{};
    };

    const Time start_;
    const UnallocatedCString sync_endpoint_;
    const UnallocatedCString requestor_endpoint_;
    std::unique_ptr<base::SyncServer> sync_server_;
    std::unique_ptr<p2p::Requestor> p2p_requestor_;
    OTZMQListenCallback sync_cb_;
    OTZMQPairSocket sync_socket_;
    mutable std::atomic<block::Height> local_chain_height_;
    mutable std::atomic<block::Height> remote_chain_height_;
    OTFlag waiting_for_headers_;
    Time headers_requested_;
    Time headers_received_;
    mutable WorkPromises work_promises_;
    mutable SendPromises send_promises_;
    Timer heartbeat_;
    std::atomic<State> state_;
    std::promise<void> init_promise_;
    std::shared_future<void> init_;

    virtual auto instantiate_header(const ReadView payload) const noexcept
        -> std::unique_ptr<block::Header> = 0;
    auto is_synchronized_blocks() const noexcept -> bool;
    auto is_synchronized_filters() const noexcept -> bool;
    auto is_synchronized_headers() const noexcept -> bool;
    auto is_synchronized_sync_server() const noexcept -> bool;
    auto notify_sync_client() const noexcept -> void;
    auto target() const noexcept -> block::Height;

    auto pipeline(zmq::Message&& in) noexcept -> void;
    auto process_block(zmq::Message&& in) noexcept -> void;
    auto process_filter_update(zmq::Message&& in) noexcept -> void;
    auto process_header(zmq::Message&& in) noexcept -> void;
    auto process_send_to_address(zmq::Message&& in) noexcept -> void;
    auto process_send_to_payment_code(zmq::Message&& in) noexcept -> void;
    auto process_sync_data(zmq::Message&& in) noexcept -> void;
    auto reset_heartbeat() noexcept -> void;
    auto shutdown(std::promise<void>& promise) noexcept -> void;
    auto state_machine() noexcept -> bool;
    auto state_machine_headers() noexcept -> void;
    auto state_transition_blocks() noexcept -> void;
    auto state_transition_filters() noexcept -> void;
    auto state_transition_normal() noexcept -> void;
    auto state_transition_sync() noexcept -> void;

    Base() = delete;
    Base(const Base&) = delete;
    Base(Base&&) = delete;
    auto operator=(const Base&) -> Base& = delete;
    auto operator=(Base&&) -> Base& = delete;
};
}  // namespace opentxs::blockchain::node::implementation
