// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "internal/blockchain/database/Database.hpp"

#pragma once

#include <memory>
#include <span>
#include <string_view>

#include "internal/blockchain/node/Config.hpp"
#include "internal/blockchain/node/Endpoints.hpp"
#include "internal/blockchain/node/Manager.hpp"
#include "internal/blockchain/node/Mempool.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/BlockOracle.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Wallet.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace blockchain
{
namespace block
{
class Block;
class TransactionHash;
}  // namespace block

namespace database
{
class Database;  // IWYU pragma: keep
}  // namespace database

namespace node
{
namespace manager
{
class Shared;
}  // namespace manager

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
}  // namespace network
class PaymentCode;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::node::implementation
{
class Base : virtual public node::internal::Manager
{
public:
    auto AddBlock(const block::Block& block) const noexcept -> bool final;
    auto AddPeer(const network::blockchain::Address& address) const noexcept
        -> bool final;
    auto BlockOracle() const noexcept -> const node::BlockOracle& final;
    auto BroadcastTransaction(const block::Transaction& tx, const bool pushtx)
        const noexcept -> bool final;
    auto Chain() const noexcept -> Type final;
    auto DB() const noexcept -> database::Database& final;
    auto Endpoints() const noexcept -> const node::Endpoints& final;
    auto FeeRate() const noexcept -> Amount final;
    auto FilterOracle() const noexcept -> const node::FilterOracle& final;
    auto GetBalance() const noexcept -> Balance final;
    auto GetBalance(const identifier::Nym& owner) const noexcept
        -> Balance final;
    auto GetConfig() const noexcept -> const internal::Config& final;
    auto GetShared() const noexcept
        -> std::shared_ptr<const node::Manager> final;
    auto GetTransactions() const noexcept
        -> UnallocatedVector<block::TransactionHash> final;
    auto GetTransactions(const identifier::Nym& account) const noexcept
        -> UnallocatedVector<block::TransactionHash> final;
    auto GetType() const noexcept -> Type final;
    auto HeaderOracle() const noexcept -> const node::HeaderOracle& final;
    auto Internal() const noexcept -> const Manager& final { return *this; }
    auto Listen(const network::blockchain::Address& address) const noexcept
        -> bool final;
    auto Mempool() const noexcept -> const internal::Mempool& final;
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
    auto Sweep(
        const identifier::Nym& account,
        std::string_view toAddress = {},
        std::span<const PaymentCode> notify = {}) const noexcept
        -> PendingOutgoing final;
    auto Sweep(
        const identifier::Nym& account,
        const identifier::Account& subaccount,
        std::string_view toAddress = {},
        std::span<const PaymentCode> notify = {}) const noexcept
        -> PendingOutgoing final;
    auto Sweep(
        const crypto::Key& key,
        std::string_view toAddress = {},
        std::span<const PaymentCode> notify = {}) const noexcept
        -> PendingOutgoing final;

    auto Internal() noexcept -> Manager& final { return *this; }
    auto Shutdown() noexcept -> void final;
    auto Start(
        std::shared_ptr<const api::session::Client> api,
        std::shared_ptr<node::Manager>) noexcept -> void final;
    auto StartWallet() noexcept -> void final;
    auto Wallet() const noexcept -> const node::Wallet& final;

    Base() = delete;
    Base(const Base&) = delete;
    Base(Base&&) = delete;
    auto operator=(const Base&) -> Base& = delete;
    auto operator=(Base&&) -> Base& = delete;

    ~Base() override;

protected:
    Base(
        const api::session::Client& api,
        const Type type,
        const node::internal::Config& config,
        std::string_view seednode) noexcept;

private:
    std::shared_ptr<manager::Shared> shared_;

    Base(
        const api::session::Client& api,
        const Type type,
        const node::internal::Config& config,
        std::string_view seednode,
        node::Endpoints endpoints) noexcept;
};
}  // namespace opentxs::blockchain::node::implementation
