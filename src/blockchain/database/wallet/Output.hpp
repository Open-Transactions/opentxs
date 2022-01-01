// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"
// IWYU pragma: no_include "opentxs/blockchain/node/TxoState.hpp"
// IWYU pragma: no_include "opentxs/blockchain/node/TxoTag.hpp"

#pragma once

#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <shared_mutex>
#include <vector>

#include "internal/blockchain/node/Node.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Wallet.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "util/LMDB.hpp"

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
namespace bitcoin
{
class Transaction;
}  // namespace bitcoin
}  // namespace block

namespace database
{
namespace wallet
{
class Proposal;
class SubchainData;
}  // namespace wallet
}  // namespace database
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
class LMDB;
}  // namespace lmdb
}  // namespace storage

class Identifier;
}  // namespace opentxs

extern "C" {
typedef struct MDB_txn MDB_txn;
}

namespace opentxs::blockchain::database::wallet
{
using AccountID = Identifier;
using SubchainID = Identifier;
using Parent = node::internal::WalletDatabase;
using NodeID = Parent::NodeID;
using Subchain = Parent::Subchain;
using UTXO = Parent::UTXO;

class Output
{
public:
    auto CancelProposal(const Identifier& id) noexcept -> bool;
    auto GetBalance() const noexcept -> Balance;
    auto GetBalance(const identifier::Nym& owner) const noexcept -> Balance;
    auto GetBalance(const identifier::Nym& owner, const NodeID& node)
        const noexcept -> Balance;
    auto GetBalance(const crypto::Key& key) const noexcept -> Balance;
    auto GetOutputs(node::TxoState type) const noexcept
        -> std::pmr::vector<UTXO>;
    auto GetOutputs(const identifier::Nym& owner, node::TxoState type)
        const noexcept -> std::pmr::vector<UTXO>;
    auto GetOutputs(
        const identifier::Nym& owner,
        const Identifier& node,
        node::TxoState type) const noexcept -> std::pmr::vector<UTXO>;
    auto GetOutputs(const crypto::Key& key, node::TxoState type) const noexcept
        -> std::pmr::vector<UTXO>;
    auto GetTransactions() const noexcept -> std::pmr::vector<block::pTxid>;
    auto GetTransactions(const identifier::Nym& account) const noexcept
        -> std::pmr::vector<block::pTxid>;
    auto GetUnconfirmedTransactions() const noexcept
        -> std::pmr::set<block::pTxid>;
    auto GetUnspentOutputs() const noexcept -> std::pmr::vector<UTXO>;
    auto GetUnspentOutputs(const NodeID& balanceNode) const noexcept
        -> std::pmr::vector<UTXO>;

    auto AddConfirmedTransaction(
        const AccountID& account,
        const SubchainID& subchain,
        const block::Position& block,
        const std::size_t blockIndex,
        const std::pmr::vector<std::uint32_t> outputIndices,
        const block::bitcoin::Transaction& transaction) noexcept -> bool;
    auto AddMempoolTransaction(
        const AccountID& account,
        const SubchainID& subchain,
        const std::pmr::vector<std::uint32_t> outputIndices,
        const block::bitcoin::Transaction& transaction) const noexcept -> bool;
    auto AddOutgoingTransaction(
        const Identifier& proposalID,
        const proto::BlockchainTransactionProposal& proposal,
        const block::bitcoin::Transaction& transaction) noexcept -> bool;
    auto AdvanceTo(const block::Position& pos) noexcept -> bool;
    auto FinalizeReorg(MDB_txn* tx, const block::Position& pos) noexcept
        -> bool;
    auto GetOutputTags(const block::Outpoint& output) const noexcept
        -> std::pmr::set<node::TxoTag>;
    auto GetWalletHeight() const noexcept -> block::Height;
    auto ReserveUTXO(
        const identifier::Nym& spender,
        const Identifier& proposal,
        node::internal::SpendPolicy& policy) noexcept -> std::optional<UTXO>;
    auto StartReorg(
        MDB_txn* tx,
        const SubchainID& subchain,
        const block::Position& position) noexcept -> bool;

    Output(
        const api::Session& api,
        const storage::lmdb::LMDB& lmdb,
        const blockchain::Type chain,
        const wallet::SubchainData& subchains,
        wallet::Proposal& proposals) noexcept;

    ~Output();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;

    Output() = delete;
    Output(const Output&) = delete;
    auto operator=(const Output&) -> Output& = delete;
    auto operator=(Output&&) -> Output& = delete;
};
}  // namespace opentxs::blockchain::database::wallet
