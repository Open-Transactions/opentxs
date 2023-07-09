// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <memory>
#include <optional>
#include <span>
#include <tuple>

#include "internal/blockchain/database/Types.hpp"
#include "internal/blockchain/node/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace blockchain
{
namespace bitcoin
{
namespace block
{
class Output;
class Transaction;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Outpoint;
class TransactionHash;
}  // namespace block

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
class Generic;
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
class Transaction;
}  // namespace lmdb
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database
{
class Wallet
{
public:
    virtual auto CompletedProposals() const noexcept
        -> UnallocatedSet<identifier::Generic> = 0;
    virtual auto GetBalance() const noexcept -> Balance = 0;
    virtual auto GetBalance(const identifier::Nym& owner) const noexcept
        -> Balance = 0;
    virtual auto GetBalance(
        const identifier::Nym& owner,
        const SubaccountID& node) const noexcept -> Balance = 0;
    virtual auto GetBalance(const crypto::Key& key) const noexcept
        -> Balance = 0;
    virtual auto GetOutputs(node::TxoState type, alloc::Strategy alloc = {})
        const noexcept -> Vector<UTXO> = 0;
    virtual auto GetOutputs(
        const identifier::Nym& owner,
        node::TxoState type,
        alloc::Strategy alloc = {}) const noexcept -> Vector<UTXO> = 0;
    virtual auto GetOutputs(
        const identifier::Nym& owner,
        const SubaccountID& node,
        node::TxoState type,
        alloc::Strategy alloc = {}) const noexcept -> Vector<UTXO> = 0;
    virtual auto GetOutputs(
        const crypto::Key& key,
        node::TxoState type,
        alloc::Strategy alloc = {}) const noexcept -> Vector<UTXO> = 0;
    virtual auto GetOutputTags(const block::Outpoint& output) const noexcept
        -> UnallocatedSet<node::TxoTag> = 0;
    virtual auto GetPatterns(
        const SubchainID& index,
        alloc::Strategy alloc = {}) const noexcept -> Patterns = 0;
    virtual auto GetPosition() const noexcept -> block::Position = 0;
    virtual auto GetSubchainID(
        const SubaccountID& account,
        const crypto::Subchain subchain) const noexcept -> SubchainID = 0;
    virtual auto GetTransactions() const noexcept
        -> UnallocatedVector<block::TransactionHash> = 0;
    virtual auto GetTransactions(const identifier::Nym& account) const noexcept
        -> UnallocatedVector<block::TransactionHash> = 0;
    virtual auto GetUnconfirmedTransactions() const noexcept
        -> UnallocatedSet<block::TransactionHash> = 0;
    virtual auto GetUnspentOutputs(alloc::Strategy alloc = {}) const noexcept
        -> Vector<UTXO> = 0;
    virtual auto GetUnspentOutputs(
        const SubaccountID& account,
        const crypto::Subchain subchain,
        alloc::Strategy alloc = {}) const noexcept -> Vector<UTXO> = 0;
    virtual auto GetWalletHeight() const noexcept -> block::Height = 0;
    virtual auto LoadProposal(const identifier::Generic& id) const noexcept
        -> std::optional<proto::BlockchainTransactionProposal> = 0;
    virtual auto LoadProposals() const noexcept
        -> UnallocatedVector<proto::BlockchainTransactionProposal> = 0;
    virtual auto LookupContact(const Data& pubkeyHash) const noexcept
        -> UnallocatedSet<identifier::Generic> = 0;
    virtual auto PublishBalance() const noexcept -> void = 0;
    virtual auto SubchainLastIndexed(const SubchainID& index) const noexcept
        -> std::optional<Bip32Index> = 0;
    virtual auto SubchainLastScanned(const SubchainID& index) const noexcept
        -> block::Position = 0;
    virtual auto SubchainSetLastScanned(
        const SubchainID& index,
        const block::Position& position) const noexcept -> bool = 0;

    virtual auto AddConfirmedTransactions(
        const SubaccountID& account,
        const SubchainID& index,
        BatchedMatches&& transactions,
        TXOs& txoCreated,
        TXOs& txoConsumed) noexcept -> bool = 0;
    virtual auto AddMempoolTransaction(
        const SubaccountID& account,
        const crypto::Subchain subchain,
        const Vector<std::uint32_t> outputIndices,
        const block::Transaction& transaction,
        TXOs& txoCreated) noexcept -> bool = 0;
    virtual auto AddOutgoingTransaction(
        const identifier::Generic& proposalID,
        const proto::BlockchainTransactionProposal& proposal,
        const block::Transaction& transaction) noexcept -> bool = 0;
    virtual auto AddProposal(
        const identifier::Generic& id,
        const proto::BlockchainTransactionProposal& tx) noexcept -> bool = 0;
    virtual auto AdvanceTo(const block::Position& pos) noexcept -> bool = 0;
    virtual auto CancelProposal(const identifier::Generic& id) noexcept
        -> bool = 0;
    virtual auto FinalizeReorg(
        storage::lmdb::Transaction& tx,
        const block::Position& pos) noexcept -> bool = 0;
    virtual auto ForgetProposals(
        const UnallocatedSet<identifier::Generic>& ids) noexcept -> bool = 0;
    virtual auto ReorgTo(
        const node::internal::HeaderOraclePrivate& data,
        storage::lmdb::Transaction& tx,
        const node::HeaderOracle& headers,
        const SubaccountID& account,
        const crypto::Subchain subchain,
        const SubchainID& index,
        std::span<const block::Position> reorg) noexcept -> bool = 0;
    virtual auto ReserveUTXO(
        const identifier::Nym& spender,
        const identifier::Generic& proposal,
        node::internal::SpendPolicy& policy) noexcept
        -> std::optional<UTXO> = 0;
    virtual auto ReserveUTXO(
        const identifier::Nym& spender,
        const identifier::Generic& proposal,
        const block::Outpoint& id) noexcept -> std::optional<UTXO> = 0;
    virtual auto StartReorg() noexcept -> storage::lmdb::Transaction = 0;
    virtual auto SubchainAddElements(
        const SubchainID& index,
        const ElementMap& elements) noexcept -> bool = 0;

    virtual ~Wallet() = default;
};
}  // namespace opentxs::blockchain::database
