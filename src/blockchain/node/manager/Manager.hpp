// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "internal/blockchain/database/Database.hpp"

#pragma once

#include <cs_plain_guarded.h>
#include <future>
#include <memory>
#include <mutex>
#include <span>
#include <string_view>
#include <utility>

#include "blockchain/node/Mempool.hpp"
#include "core/Shutdown.hpp"
#include "core/Worker.hpp"
#include "internal/blockchain/node/Config.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/Mempool.hpp"
#include "internal/blockchain/node/Wallet.hpp"
#include "internal/blockchain/node/blockoracle/BlockOracle.hpp"
#include "internal/blockchain/node/headeroracle/HeaderOracle.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/Timer.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/node/BlockOracle.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/blockchain/node/Wallet.hpp"
#include "opentxs/core/Amount.hpp"
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
class Block;
class TransactionHash;
}  // namespace block

namespace crypto
{
class PaymentCode;
}  // namespace crypto

namespace database
{
class Database;  // IWYU pragma: keep
}  // namespace database

namespace node
{
class Manager;
}  // namespace node
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

namespace network
{
namespace blockchain
{
class Address;
}  // namespace blockchain

namespace zeromq
{
namespace socket
{
class Raw;
}  // namespace socket

class Frame;
class Message;
}  // namespace zeromq
}  // namespace network

namespace proto
{
class BlockchainTransactionProposal;
class HDPath;
}  // namespace proto

class PasswordPrompt;
class PaymentCode;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::implementation
{
class Base : virtual public node::internal::Manager,
             public Worker<Base, api::Session>
{
public:
    const Type chain_;

    auto AddBlock(const block::Block& block) const noexcept -> bool final;
    auto AddPeer(const network::blockchain::Address& address) const noexcept
        -> bool final;
    auto BlockOracle() const noexcept -> const node::BlockOracle& final;
    auto BroadcastTransaction(const block::Transaction& tx, const bool pushtx)
        const noexcept -> bool final;
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
    auto GetShared() const noexcept
        -> std::shared_ptr<const node::Manager> final;
    auto GetTransactions() const noexcept
        -> UnallocatedVector<block::TransactionHash> final;
    auto GetTransactions(const identifier::Nym& account) const noexcept
        -> UnallocatedVector<block::TransactionHash> final;
    auto GetType() const noexcept -> Type final { return chain_; }
    auto HeaderOracle() const noexcept -> const node::HeaderOracle& final;
    auto Internal() const noexcept -> const Manager& final { return *this; }
    auto Listen(const network::blockchain::Address& address) const noexcept
        -> bool final;
    auto Mempool() const noexcept -> const internal::Mempool& final
    {
        return mempool_;
    }
    auto Profile() const noexcept -> BlockchainProfile final;
    auto SendToAddress(
        const opentxs::identifier::Nym& sender,
        std::string_view address,
        const Amount amount,
        std::string_view memo,
        std::span<const std::string_view> notify) const noexcept
        -> PendingOutgoing final;
    auto SendToPaymentCode(
        const opentxs::identifier::Nym& sender,
        std::string_view recipient,
        const Amount amount,
        std::string_view memo,
        std::span<const std::string_view> notify) const noexcept
        -> PendingOutgoing final;
    auto SendToPaymentCode(
        const opentxs::identifier::Nym& sender,
        const PaymentCode& recipient,
        const Amount amount,
        std::string_view memo,
        std::span<const PaymentCode> notify) const noexcept
        -> PendingOutgoing final;
    auto ShuttingDown() const noexcept -> bool final;

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
    const CString command_line_peers_;
    opentxs::internal::ShutdownSender shutdown_sender_;
    mutable std::shared_ptr<blockchain::database::Database> database_p_;
    node::Mempool mempool_;

protected:
    mutable node::internal::HeaderOracle header_;
    node::internal::BlockOracle block_;

private:
    std::unique_ptr<node::FilterOracle> filter_p_;

protected:
    blockchain::database::Database& database_;
    node::FilterOracle& filters_;
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

    network::zeromq::socket::Raw& to_peer_manager_;
    network::zeromq::socket::Raw& to_wallet_;
    network::zeromq::socket::Raw& to_dht_;
    network::zeromq::socket::Raw& to_blockchain_api_;
    mutable SendPromises send_promises_;
    Timer heartbeat_;
    std::promise<void> init_promise_;
    std::shared_future<void> init_;
    mutable GuardedSelf self_;

    static auto serialize_notification(
        const PaymentCode& sender,
        const PaymentCode& recipient,
        const proto::HDPath& senderPath,
        proto::BlockchainTransactionProposal& out) noexcept -> void;
    static auto serialize_notifications(
        std::span<const std::string_view> in,
        network::zeromq::Message& out) noexcept -> void;
    static auto serialize_notifications(
        std::span<const PaymentCode> in,
        network::zeromq::Message& out) noexcept -> void;

    auto create_or_load_subaccount(
        const identifier::Nym& senderNym,
        const PaymentCode& senderPC,
        const proto::HDPath& senderPath,
        const PaymentCode& recipient,
        const PasswordPrompt& reason,
        Set<PaymentCode>& notify) const noexcept -> const crypto::PaymentCode&;
    auto extract_notifications(
        const std::span<const network::zeromq::Frame> message,
        const identifier::Nym& senderNym,
        const PaymentCode& senderPC,
        const proto::HDPath& senderPath,
        const PasswordPrompt& reason,
        SendResult& rc) const noexcept(false) -> Set<PaymentCode>;
    auto get_sender(const identifier::Nym& nymID, SendResult& rc) const
        noexcept(false) -> std::pair<opentxs::PaymentCode, proto::HDPath>;
    auto notify_sync_client() const noexcept -> void;
    auto pipeline(network::zeromq::Message&& in) noexcept -> void;
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
