// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <optional>
#include <span>

#include "blockchain/database/wallet/Output.hpp"
#include "blockchain/database/wallet/Proposal.hpp"
#include "blockchain/database/wallet/Subchain.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/cfilter/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

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
class Transaction;
class TransactionHash;
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
struct HeaderOraclePrivate;
struct SpendPolicy;
}  // namespace internal

class HeaderOracle;
}  // namespace node
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

namespace proto
{
class BlockchainTransactionProposal;
}  // namespace proto

namespace storage
{
namespace lmdb
{
class Database;
class Transaction;
}  // namespace lmdb
}  // namespace storage

class Data;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database::implementation
{
class Wallet
{
public:
    auto AddConfirmedTransactions(
        const SubaccountID& account,
        const SubchainID& index,
        BatchedMatches&& transactions,
        TXOs& txoCreated,
        TXOs& txoConsumed) noexcept -> bool;
    auto AddMempoolTransaction(
        const SubaccountID& balanceNode,
        const crypto::Subchain subchain,
        const Vector<std::uint32_t> outputIndices,
        const block::Transaction& transaction,
        TXOs& txoCreated) const noexcept -> bool;
    auto AddOutgoingTransaction(
        const identifier::Generic& proposalID,
        const proto::BlockchainTransactionProposal& proposal,
        const block::Transaction& transaction) const noexcept -> bool;
    auto AddProposal(
        const identifier::Generic& id,
        const proto::BlockchainTransactionProposal& tx) const noexcept -> bool;
    auto AdvanceTo(const block::Position& pos) const noexcept -> bool;
    auto CancelProposal(const identifier::Generic& id) const noexcept -> bool;
    auto CompletedProposals() const noexcept
        -> UnallocatedSet<identifier::Generic>;
    auto FinalizeReorg(
        storage::lmdb::Transaction& tx,
        const block::Position& pos) const noexcept -> bool;
    auto ForgetProposals(
        const UnallocatedSet<identifier::Generic>& ids) const noexcept -> bool;
    auto GetBalance() const noexcept -> Balance;
    auto GetBalance(const identifier::Nym& owner) const noexcept -> Balance;
    auto GetBalance(const identifier::Nym& owner, const SubaccountID& node)
        const noexcept -> Balance;
    auto GetBalance(const crypto::Key& key) const noexcept -> Balance;
    auto GetOutputs(node::TxoState type, alloc::Default alloc) const noexcept
        -> Vector<UTXO>;
    auto GetOutputs(
        const identifier::Nym& owner,
        node::TxoState type,
        alloc::Default alloc) const noexcept -> Vector<UTXO>;
    auto GetOutputs(
        const identifier::Nym& owner,
        const SubaccountID& node,
        node::TxoState type,
        alloc::Default alloc) const noexcept -> Vector<UTXO>;
    auto GetOutputs(
        const crypto::Key& key,
        node::TxoState type,
        alloc::Default alloc) const noexcept -> Vector<UTXO>;
    auto GetOutputTags(const block::Outpoint& output) const noexcept
        -> UnallocatedSet<node::TxoTag>;
    auto GetPatterns(const SubchainID& index, alloc::Default alloc)
        const noexcept -> Patterns;
    auto GetPosition() const noexcept -> block::Position;
    auto GetSubchainID(
        const SubaccountID& balanceNode,
        const crypto::Subchain subchain) const noexcept -> SubchainID;
    auto GetTransactions() const noexcept
        -> UnallocatedVector<block::TransactionHash>;
    auto GetTransactions(const identifier::Nym& account) const noexcept
        -> UnallocatedVector<block::TransactionHash>;
    auto GetUnconfirmedTransactions() const noexcept
        -> UnallocatedSet<block::TransactionHash>;
    auto GetUnspentOutputs(alloc::Default alloc) const noexcept -> Vector<UTXO>;
    auto GetUnspentOutputs(
        const SubaccountID& balanceNode,
        const crypto::Subchain subchain,
        alloc::Default alloc) const noexcept -> Vector<UTXO>;
    auto GetWalletHeight() const noexcept -> block::Height;
    auto LoadProposal(const identifier::Generic& id) const noexcept
        -> std::optional<proto::BlockchainTransactionProposal>;
    auto LoadProposals() const noexcept
        -> UnallocatedVector<proto::BlockchainTransactionProposal>;
    auto LookupContact(const Data& pubkeyHash) const noexcept
        -> UnallocatedSet<identifier::Generic>;
    auto PublishBalance() const noexcept -> void;
    auto ReorgTo(
        const node::internal::HeaderOraclePrivate& data,
        storage::lmdb::Transaction& tx,
        const node::HeaderOracle& headers,
        const SubaccountID& balanceNode,
        const crypto::Subchain subchain,
        const SubchainID& index,
        std::span<const block::Position> reorg) const noexcept -> bool;
    auto ReserveUTXO(
        const identifier::Nym& spender,
        const identifier::Generic& proposal,
        const node::internal::SpendPolicy& policy) const noexcept
        -> std::optional<UTXO>;
    auto ReserveUTXO(
        const identifier::Nym& spender,
        const identifier::Generic& proposal,
        const block::Outpoint& id) const noexcept -> std::optional<UTXO>;
    auto SubchainAddElements(
        const SubchainID& index,
        const ElementMap& elements) const noexcept -> bool;
    auto SubchainLastIndexed(const SubchainID& index) const noexcept
        -> std::optional<Bip32Index>;
    auto SubchainLastScanned(const SubchainID& index) const noexcept
        -> block::Position;
    auto SubchainSetLastScanned(
        const SubchainID& index,
        const block::Position& position) const noexcept -> bool;

    Wallet(
        const api::Session& api,
        const common::Database& common,
        const storage::lmdb::Database& lmdb,
        const blockchain::Type chain,
        const blockchain::cfilter::Type filter) noexcept;
    Wallet() = delete;
    Wallet(const Wallet&) = delete;
    auto operator=(const Wallet&) -> Wallet& = delete;
    auto operator=(Wallet&&) -> Wallet& = delete;

private:
    const api::Session& api_;
    const common::Database& common_;
    const storage::lmdb::Database& lmdb_;
    mutable wallet::SubchainData subchains_;
    mutable wallet::Proposal proposals_;
    mutable wallet::Output outputs_;
};
}  // namespace opentxs::blockchain::database::implementation
