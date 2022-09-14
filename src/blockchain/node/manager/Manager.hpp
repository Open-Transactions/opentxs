// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/util/BlockchainProfile.hpp"

#pragma once

#include <cs_plain_guarded.h>
#include <atomic>
#include <cstddef>
#include <future>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <shared_mutex>
#include <string_view>
#include <utility>

#include "blockchain/node/Mempool.hpp"
#include "core/Shutdown.hpp"
#include "core/Worker.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/node/Config.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/Mempool.hpp"
#include "internal/blockchain/node/PeerManager.hpp"
#include "internal/blockchain/node/Types.hpp"
#include "internal/blockchain/node/Wallet.hpp"
#include "internal/blockchain/node/blockoracle/BlockOracle.hpp"
#include "internal/blockchain/node/filteroracle/FilterOracle.hpp"
#include "internal/blockchain/node/headeroracle/HeaderOracle.hpp"
#include "internal/util/Flag.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/node/BlockOracle.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/blockchain/node/Wallet.hpp"
#include "opentxs/core/Amount.hpp"
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
#include "opentxs/util/Types.hpp"
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
namespace bitcoin
{
namespace block
{
class Block;
class Transaction;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Header;
class Position;
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
namespace internal
{
class PeerManager;
struct Config;
}  // namespace internal

namespace p2p
{
class Requestor;
}  // namespace p2p

class Wallet;
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
class Raw;
}  // namespace socket

class Message;
}  // namespace zeromq
}  // namespace network

class PaymentCode;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::implementation
{
class Base : virtual public node::internal::Manager,
             public Worker<Base, api::Session>
{
public:
    const Type chain_;

    auto AddBlock(const std::shared_ptr<const bitcoin::block::Block> block)
        const noexcept -> bool final;
    auto AddPeer(const blockchain::p2p::Address& address) const noexcept
        -> bool final;
    auto BlockOracle() const noexcept -> const node::BlockOracle& final;
    auto BroadcastTransaction(
        const bitcoin::block::Transaction& tx,
        const bool pushtx) const noexcept -> bool final;
    auto Chain() const noexcept -> Type final { return chain_; }
    auto DB() const noexcept -> database::Database& final;
    auto Endpoints() const noexcept -> const node::Endpoints& final
    {
        return endpoints_;
    }
    auto FeeRate() const noexcept -> Amount final;
    auto FilterOracle() const noexcept -> const node::FilterOracle& final;
    auto GetBalance() const noexcept -> Balance final;
    auto GetBalance(const identifier::Nym& owner) const noexcept
        -> Balance final;
    auto GetConfig() const noexcept -> const internal::Config& final
    {
        return config_;
    }
    auto GetConfirmations(const UnallocatedCString& txid) const noexcept
        -> ChainHeight final;
    auto GetPeerCount() const noexcept -> std::size_t final;
    auto GetShared() const noexcept
        -> std::shared_ptr<const node::Manager> final;
    auto GetTransactions() const noexcept
        -> UnallocatedVector<block::pTxid> final;
    auto GetTransactions(const identifier::Nym& account) const noexcept
        -> UnallocatedVector<block::pTxid> final;
    auto GetType() const noexcept -> Type final { return chain_; }
    auto GetVerifiedPeerCount() const noexcept -> std::size_t final;
    auto HeaderOracle() const noexcept -> const node::HeaderOracle& final;
    auto JobReady(const node::PeerManagerJobs type) const noexcept
        -> void final;
    auto Internal() const noexcept -> const Manager& final { return *this; }
    auto Listen(const blockchain::p2p::Address& address) const noexcept
        -> bool final;
    auto Mempool() const noexcept -> const internal::Mempool& final
    {
        return mempool_;
    }
    auto PeerManager() const noexcept -> const internal::PeerManager& final;
    auto Profile() const noexcept -> BlockchainProfile final;
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
    auto ShuttingDown() const noexcept -> bool final;

    auto Connect() noexcept -> bool final;
    auto Disconnect() noexcept -> bool final;
    auto Internal() noexcept -> Manager& final { return *this; }
    auto Shutdown() noexcept -> std::shared_future<void> final
    {
        return signal_shutdown();
    }
    auto Start(std::shared_ptr<const node::Manager>) noexcept -> void final;
    auto StartWallet() noexcept -> void final;
    auto Wallet() const noexcept -> const node::Wallet& final;

    Base() = delete;
    Base(const Base&) = delete;
    Base(Base&&) = delete;
    auto operator=(const Base&) -> Base& = delete;
    auto operator=(Base&&) -> Base& = delete;

    ~Base() override;

private:
    const node::internal::Config& config_;
    const node::Endpoints endpoints_;
    const cfilter::Type filter_type_;
    opentxs::internal::ShutdownSender shutdown_sender_;
    mutable std::shared_ptr<blockchain::database::Database> database_p_;
    node::Mempool mempool_;

protected:
    mutable node::internal::HeaderOracle header_;
    node::internal::BlockOracle block_;

private:
    std::unique_ptr<node::FilterOracle> filter_p_;
    std::unique_ptr<node::internal::PeerManager> peer_p_;

protected:
    blockchain::database::Database& database_;
    node::FilterOracle& filters_;
    node::internal::PeerManager& peer_;
    node::internal::Wallet wallet_;

    // NOTE call init in every final constructor body
    auto init() noexcept -> void;
    auto shutdown_timers() noexcept -> void;

    Base(
        const api::Session& api,
        const Type type,
        const node::internal::Config& config,
        std::string_view seednode) noexcept;

private:
    friend Worker<Base, api::Session>;

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

    // TODO c++20 use atomic weak_ptr
    using GuardedSelf =
        libguarded::plain_guarded<std::weak_ptr<const node::Manager>>;

    network::zeromq::socket::Raw& to_block_oracle_;
    network::zeromq::socket::Raw& to_block_cache_;
    network::zeromq::socket::Raw& to_wallet_;
    network::zeromq::socket::Raw& to_dht_;
    network::zeromq::socket::Raw& to_blockchain_api_;
    mutable SendPromises send_promises_;
    Timer heartbeat_;
    std::promise<void> init_promise_;
    std::shared_future<void> init_;
    mutable GuardedSelf self_;

    auto notify_sync_client() const noexcept -> void;
    auto pipeline(network::zeromq::Message&& in) noexcept -> void;
    auto process_block(network::zeromq::Message&& in) noexcept -> void;
    auto process_filter_update(network::zeromq::Message&& in) noexcept -> void;
    auto process_send_to_address(network::zeromq::Message&& in) noexcept
        -> void;
    auto process_send_to_payment_code(network::zeromq::Message&& in) noexcept
        -> void;
    auto process_sync_data(network::zeromq::Message&& in) noexcept -> void;
    auto reset_heartbeat() noexcept -> void;
    auto shutdown(std::promise<void>& promise) noexcept -> void;
    auto state_machine() noexcept -> bool;

    Base(
        const api::Session& api,
        const Type type,
        const node::internal::Config& config,
        std::string_view seednode,
        node::Endpoints endpoints) noexcept;
};
}  // namespace opentxs::blockchain::node::implementation
