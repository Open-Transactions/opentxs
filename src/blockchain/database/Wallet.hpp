// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/node/TxoState.hpp"
// IWYU pragma: no_include "opentxs/blockchain/node/TxoTag.hpp"

#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <map>
#include <memory>
#include <mutex>
#include <optional>
#include <set>
#include <string>
#include <tuple>
#include <utility>
#include <vector>

#include "Proto.hpp"
#include "blockchain/database/wallet/Output.hpp"
#include "blockchain/database/wallet/Proposal.hpp"
#include "blockchain/database/wallet/Subchain.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/blockchain/node/Wallet.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "serialization/protobuf/BlockchainTransactionProposal.pb.h"
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
class Output;
class Transaction;
}  // namespace bitcoin
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
class LMDB;
}  // namespace lmdb
}  // namespace storage

class Data;
class Identifier;
}  // namespace opentxs

namespace opentxs::blockchain::database
{
class Wallet
{
public:
    using Parent = node::internal::WalletDatabase;
    using NodeID = Parent::NodeID;
    using pNodeID = Parent::pNodeID;
    using SubchainIndex = Parent::SubchainIndex;
    using pSubchainIndex = Parent::pSubchainIndex;
    using Subchain = Parent::Subchain;
    using ElementID = Parent::ElementID;
    using ElementMap = Parent::ElementMap;
    using Pattern = Parent::Pattern;
    using Patterns = Parent::Patterns;
    using MatchingIndices = Parent::MatchingIndices;
    using UTXO = Parent::UTXO;

    auto AddConfirmedTransaction(
        const NodeID& balanceNode,
        const Subchain subchain,
        const block::Position& block,
        const std::size_t blockIndex,
        const std::vector<std::uint32_t> outputIndices,
        const block::bitcoin::Transaction& transaction) const noexcept -> bool;
    auto AddMempoolTransaction(
        const NodeID& balanceNode,
        const Subchain subchain,
        const std::vector<std::uint32_t> outputIndices,
        const block::bitcoin::Transaction& transaction) const noexcept -> bool;
    auto AddOutgoingTransaction(
        const Identifier& proposalID,
        const proto::BlockchainTransactionProposal& proposal,
        const block::bitcoin::Transaction& transaction) const noexcept -> bool;
    auto AddProposal(
        const Identifier& id,
        const proto::BlockchainTransactionProposal& tx) const noexcept -> bool;
    auto AdvanceTo(const block::Position& pos) const noexcept -> bool;
    auto CancelProposal(const Identifier& id) const noexcept -> bool;
    auto CompletedProposals() const noexcept -> std::set<OTIdentifier>;
    auto FinalizeReorg(
        storage::lmdb::LMDB::Transaction& tx,
        const block::Position& pos) const noexcept -> bool;
    auto ForgetProposals(const std::set<OTIdentifier>& ids) const noexcept
        -> bool;
    auto GetBalance() const noexcept -> Balance;
    auto GetBalance(const identifier::Nym& owner) const noexcept -> Balance;
    auto GetBalance(const identifier::Nym& owner, const NodeID& node)
        const noexcept -> Balance;
    auto GetBalance(const crypto::Key& key) const noexcept -> Balance;
    auto GetOutputs(node::TxoState type) const noexcept -> std::vector<UTXO>;
    auto GetOutputs(const identifier::Nym& owner, node::TxoState type)
        const noexcept -> std::vector<UTXO>;
    auto GetOutputs(
        const identifier::Nym& owner,
        const Identifier& node,
        node::TxoState type) const noexcept -> std::vector<UTXO>;
    auto GetOutputs(const crypto::Key& key, node::TxoState type) const noexcept
        -> std::vector<UTXO>;
    auto GetOutputTags(const block::Outpoint& output) const noexcept
        -> std::set<node::TxoTag>;
    auto GetPatterns(const SubchainIndex& index) const noexcept -> Patterns;
    auto GetSubchainID(const NodeID& balanceNode, const Subchain subchain)
        const noexcept -> pSubchainIndex;
    auto GetTransactions() const noexcept -> std::vector<block::pTxid>;
    auto GetTransactions(const identifier::Nym& account) const noexcept
        -> std::vector<block::pTxid>;
    auto GetUnconfirmedTransactions() const noexcept -> std::set<block::pTxid>;
    auto GetUnspentOutputs() const noexcept -> std::vector<UTXO>;
    auto GetUnspentOutputs(const NodeID& balanceNode, const Subchain subchain)
        const noexcept -> std::vector<UTXO>;
    auto GetUntestedPatterns(const SubchainIndex& index, const ReadView blockID)
        const noexcept -> Patterns;
    auto GetWalletHeight() const noexcept -> block::Height;
    auto LoadProposal(const Identifier& id) const noexcept
        -> std::optional<proto::BlockchainTransactionProposal>;
    auto LoadProposals() const noexcept
        -> std::vector<proto::BlockchainTransactionProposal>;
    auto LookupContact(const Data& pubkeyHash) const noexcept
        -> std::set<OTIdentifier>;
    auto ReorgTo(
        const Lock& headerOracleLock,
        storage::lmdb::LMDB::Transaction& tx,
        const node::HeaderOracle& headers,
        const NodeID& balanceNode,
        const Subchain subchain,
        const SubchainIndex& index,
        const std::vector<block::Position>& reorg) const noexcept -> bool;
    auto ReserveUTXO(
        const identifier::Nym& spender,
        const Identifier& proposal,
        node::internal::SpendPolicy& policy) const noexcept
        -> std::optional<UTXO>;
    auto SubchainAddElements(
        const SubchainIndex& index,
        const ElementMap& elements) const noexcept -> bool;
    auto SubchainLastIndexed(const SubchainIndex& index) const noexcept
        -> std::optional<Bip32Index>;
    auto SubchainLastScanned(const SubchainIndex& index) const noexcept
        -> block::Position;
    auto SubchainMatchBlock(
        const SubchainIndex& index,
        const std::vector<std::pair<ReadView, MatchingIndices>>& results)
        const noexcept -> bool;
    auto SubchainSetLastScanned(
        const SubchainIndex& index,
        const block::Position& position) const noexcept -> bool;

    Wallet(
        const api::Session& api,
        const common::Database& common,
        const storage::lmdb::LMDB& lmdb,
        const blockchain::Type chain,
        const blockchain::filter::Type filter) noexcept;

private:
    const api::Session& api_;
    const common::Database& common_;
    const storage::lmdb::LMDB& lmdb_;
    mutable wallet::SubchainData subchains_;
    mutable wallet::Proposal proposals_;
    mutable wallet::Output outputs_;

    Wallet() = delete;
    Wallet(const Wallet&) = delete;
    auto operator=(const Wallet&) -> Wallet& = delete;
    auto operator=(Wallet&&) -> Wallet& = delete;
};
}  // namespace opentxs::blockchain::database
