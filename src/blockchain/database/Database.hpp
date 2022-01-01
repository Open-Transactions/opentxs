// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/node/TxoState.hpp"
// IWYU pragma: no_include "opentxs/blockchain/node/TxoTag.hpp"

#pragma once

#include <boost/container/flat_set.hpp>
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
#include <utility>
#include <vector>

#include "Proto.hpp"
#include "blockchain/database/Blocks.hpp"
#include "blockchain/database/Filters.hpp"
#include "blockchain/database/Headers.hpp"
#include "blockchain/database/Sync.hpp"
#include "blockchain/database/Wallet.hpp"
#include "blockchain/database/common/Database.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/database/common/Common.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/FilterType.hpp"
#include "opentxs/blockchain/GCS.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/bitcoin/Header.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Numbers.hpp"
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
class Block;
class Header;
class Transaction;
}  // namespace bitcoin

class Block;
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
class HeaderOracle;
class UpdateTransaction;
}  // namespace node

class GCS;
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

namespace network
{
namespace p2p
{
class Block;
}  // namespace p2p
}  // namespace network

namespace proto
{
class BlockchainTransactionProposal;
}  // namespace proto

class Data;
class Identifier;
}  // namespace opentxs

namespace opentxs::blockchain::implementation
{
class Database final : public internal::Database
{
public:
    auto AddConfirmedTransaction(
        const NodeID& balanceNode,
        const Subchain subchain,
        const block::Position& block,
        const std::size_t blockIndex,
        const std::pmr::vector<std::uint32_t> outputIndices,
        const block::bitcoin::Transaction& transaction) const noexcept
        -> bool final
    {
        return wallet_.AddConfirmedTransaction(
            balanceNode,
            subchain,
            block,
            blockIndex,
            outputIndices,
            transaction);
    }
    auto AddMempoolTransaction(
        const NodeID& balanceNode,
        const Subchain subchain,
        const std::pmr::vector<std::uint32_t> outputIndices,
        const block::bitcoin::Transaction& transaction) const noexcept
        -> bool final
    {
        return wallet_.AddMempoolTransaction(
            balanceNode, subchain, outputIndices, transaction);
    }
    auto AddOutgoingTransaction(
        const Identifier& proposalID,
        const proto::BlockchainTransactionProposal& proposal,
        const block::bitcoin::Transaction& transaction) const noexcept
        -> bool final
    {
        return wallet_.AddOutgoingTransaction(
            proposalID, proposal, transaction);
    }
    auto AddOrUpdate(Address address) const noexcept -> bool final
    {
        return common_.AddOrUpdate(std::move(address));
    }
    auto AddProposal(
        const Identifier& id,
        const proto::BlockchainTransactionProposal& tx) const noexcept
        -> bool final
    {
        return wallet_.AddProposal(id, tx);
    }
    auto AdvanceTo(const block::Position& pos) const noexcept -> bool final
    {
        return wallet_.AdvanceTo(pos);
    }
    auto ApplyUpdate(const node::UpdateTransaction& update) const noexcept
        -> bool final
    {
        return headers_.ApplyUpdate(update);
    }
    // Throws std::out_of_range if no block at that position
    auto BestBlock(const block::Height position) const noexcept(false)
        -> block::pHash final
    {
        return headers_.BestBlock(position);
    }
    auto BlockExists(const block::Hash& block) const noexcept -> bool final
    {
        return common_.BlockExists(block);
    }
    auto BlockLoadBitcoin(const block::Hash& block) const noexcept
        -> std::shared_ptr<const block::bitcoin::Block> final
    {
        return blocks_.LoadBitcoin(block);
    }
    auto BlockPolicy() const noexcept -> database::BlockStorage final
    {
        return common_.BlockPolicy();
    }
    auto BlockStore(const block::Block& block) const noexcept -> bool final
    {
        return blocks_.Store(block);
    }
    auto BlockTip() const noexcept -> block::Position final
    {
        return blocks_.Tip();
    }
    auto CompletedProposals() const noexcept
        -> std::pmr::set<OTIdentifier> final
    {
        return wallet_.CompletedProposals();
    }
    auto CurrentBest() const noexcept -> std::unique_ptr<block::Header> final
    {
        return headers_.CurrentBest();
    }
    auto CurrentCheckpoint() const noexcept -> block::Position final
    {
        return headers_.CurrentCheckpoint();
    }
    auto CancelProposal(const Identifier& id) const noexcept -> bool final
    {
        return wallet_.CancelProposal(id);
    }
    auto FilterHeaderTip(const filter::Type type) const noexcept
        -> block::Position final
    {
        return filters_.CurrentHeaderTip(type);
    }
    auto FilterTip(const filter::Type type) const noexcept
        -> block::Position final
    {
        return filters_.CurrentTip(type);
    }
    auto FinalizeReorg(
        storage::lmdb::LMDB::Transaction& tx,
        const block::Position& pos) const noexcept -> bool final
    {
        return wallet_.FinalizeReorg(tx, pos);
    }
    auto ForgetProposals(const std::pmr::set<OTIdentifier>& ids) const noexcept
        -> bool final
    {
        return wallet_.ForgetProposals(ids);
    }
    auto DisconnectedHashes() const noexcept -> node::DisconnectedList final
    {
        return headers_.DisconnectedHashes();
    }
    auto Get(
        const Protocol protocol,
        const std::pmr::set<Type> onNetworks,
        const std::pmr::set<Service> withServices) const noexcept
        -> Address final
    {
        return common_.Find(chain_, protocol, onNetworks, withServices);
    }
    auto GetBalance() const noexcept -> Balance final
    {
        return wallet_.GetBalance();
    }
    auto GetBalance(const identifier::Nym& owner) const noexcept
        -> Balance final
    {
        return wallet_.GetBalance(owner);
    }
    auto GetBalance(const identifier::Nym& owner, const NodeID& node)
        const noexcept -> Balance final
    {
        return wallet_.GetBalance(owner, node);
    }
    auto GetBalance(const crypto::Key& key) const noexcept -> Balance final
    {
        return wallet_.GetBalance(key);
    }
    auto GetOutputs(node::TxoState type) const noexcept
        -> std::pmr::vector<UTXO> final
    {
        return wallet_.GetOutputs(type);
    }
    auto GetOutputs(const identifier::Nym& owner, node::TxoState type)
        const noexcept -> std::pmr::vector<UTXO> final
    {
        return wallet_.GetOutputs(owner, type);
    }
    auto GetOutputs(
        const identifier::Nym& owner,
        const Identifier& node,
        node::TxoState type) const noexcept -> std::pmr::vector<UTXO> final
    {
        return wallet_.GetOutputs(owner, node, type);
    }
    auto GetOutputs(const crypto::Key& key, node::TxoState type) const noexcept
        -> std::pmr::vector<UTXO> final
    {
        return wallet_.GetOutputs(key, type);
    }
    auto GetOutputTags(const block::Outpoint& output) const noexcept
        -> std::pmr::set<node::TxoTag> final
    {
        return wallet_.GetOutputTags(output);
    }
    auto GetPatterns(const SubchainIndex& index) const noexcept
        -> Patterns final
    {
        return wallet_.GetPatterns(index);
    }
    auto GetSubchainID(const NodeID& balanceNode, const Subchain subchain)
        const noexcept -> pSubchainIndex final
    {
        return wallet_.GetSubchainID(balanceNode, subchain);
    }
    auto GetTransactions() const noexcept
        -> std::pmr::vector<block::pTxid> final
    {
        return wallet_.GetTransactions();
    }
    auto GetTransactions(const identifier::Nym& account) const noexcept
        -> std::pmr::vector<block::pTxid> final
    {
        return wallet_.GetTransactions(account);
    }
    auto GetUnconfirmedTransactions() const noexcept
        -> std::pmr::set<block::pTxid> final
    {
        return wallet_.GetUnconfirmedTransactions();
    }
    auto GetUnspentOutputs() const noexcept -> std::pmr::vector<UTXO> final
    {
        return wallet_.GetUnspentOutputs();
    }
    auto GetUnspentOutputs(const NodeID& balanceNode, const Subchain subchain)
        const noexcept -> std::pmr::vector<UTXO> final
    {
        return wallet_.GetUnspentOutputs(balanceNode, subchain);
    }
    auto GetUntestedPatterns(const SubchainIndex& index, const ReadView blockID)
        const noexcept -> Patterns final
    {
        return wallet_.GetUntestedPatterns(index, blockID);
    }
    auto GetWalletHeight() const noexcept -> block::Height final
    {
        return wallet_.GetWalletHeight();
    }
    auto HasDisconnectedChildren(const block::Hash& hash) const noexcept
        -> bool final
    {
        return headers_.HasDisconnectedChildren(hash);
    }
    auto HaveCheckpoint() const noexcept -> bool final
    {
        return headers_.HaveCheckpoint();
    }
    auto HaveFilter(const filter::Type type, const block::Hash& block)
        const noexcept -> bool final
    {
        return filters_.HaveFilter(type, block);
    }
    auto HaveFilterHeader(const filter::Type type, const block::Hash& block)
        const noexcept -> bool final
    {
        return filters_.HaveFilterHeader(type, block);
    }
    auto HeaderExists(const block::Hash& hash) const noexcept -> bool final
    {
        return headers_.HeaderExists(hash);
    }
    auto Import(std::pmr::vector<Address> peers) const noexcept -> bool final
    {
        return common_.Import(std::move(peers));
    }
    auto IsSibling(const block::Hash& hash) const noexcept -> bool final
    {
        return headers_.IsSibling(hash);
    }
    auto LoadFilter(const filter::Type type, const ReadView block)
        const noexcept -> std::unique_ptr<const GCS> final
    {
        return filters_.LoadFilter(type, block);
    }
    auto LoadFilterHash(const filter::Type type, const ReadView block)
        const noexcept -> Hash final
    {
        return filters_.LoadFilterHash(type, block);
    }
    auto LoadFilterHeader(const filter::Type type, const ReadView block)
        const noexcept -> Hash final
    {
        return filters_.LoadFilterHeader(type, block);
    }
    // Throws std::out_of_range if the header does not exist
    auto LoadHeader(const block::Hash& hash) const noexcept(false)
        -> std::unique_ptr<block::Header> final
    {
        return headers_.LoadHeader(hash);
    }
    auto LoadProposal(const Identifier& id) const noexcept
        -> std::optional<proto::BlockchainTransactionProposal> final
    {
        return wallet_.LoadProposal(id);
    }
    auto LoadProposals() const noexcept
        -> std::pmr::vector<proto::BlockchainTransactionProposal> final
    {
        return wallet_.LoadProposals();
    }
    auto LoadSync(const Height height, Message& output) const noexcept
        -> bool final
    {
        return sync_.Load(height, output);
    }
    auto LookupContact(const Data& pubkeyHash) const noexcept
        -> std::pmr::set<OTIdentifier> final
    {
        return wallet_.LookupContact(pubkeyHash);
    }
    auto RecentHashes() const noexcept -> std::pmr::vector<block::pHash> final
    {
        return headers_.RecentHashes();
    }
    auto ReorgSync(const Height height) const noexcept -> bool final
    {
        return sync_.Reorg(height);
    }
    auto ReorgTo(
        const Lock& headerOracleLock,
        storage::lmdb::LMDB::Transaction& tx,
        const node::HeaderOracle& headers,
        const NodeID& balanceNode,
        const Subchain subchain,
        const SubchainIndex& index,
        const std::pmr::vector<block::Position>& reorg) const noexcept
        -> bool final
    {
        return wallet_.ReorgTo(
            headerOracleLock, tx, headers, balanceNode, subchain, index, reorg);
    }
    auto ReserveUTXO(
        const identifier::Nym& spender,
        const Identifier& proposal,
        node::internal::SpendPolicy& policy) const noexcept
        -> std::optional<UTXO> final
    {
        return wallet_.ReserveUTXO(spender, proposal, policy);
    }
    auto SetBlockTip(const block::Position& position) const noexcept
        -> bool final
    {
        return blocks_.SetTip(position);
    }
    auto SetFilterHeaderTip(
        const filter::Type type,
        const block::Position& position) const noexcept -> bool final
    {
        return filters_.SetHeaderTip(type, position);
    }
    auto SetFilterTip(const filter::Type type, const block::Position& position)
        const noexcept -> bool final
    {
        return filters_.SetTip(type, position);
    }
    auto SetSyncTip(const block::Position& position) const noexcept
        -> bool final
    {
        return sync_.SetTip(position);
    }
    auto SiblingHashes() const noexcept -> node::Hashes final
    {
        return headers_.SiblingHashes();
    }
    auto StartReorg() const noexcept -> storage::lmdb::LMDB::Transaction final
    {
        return lmdb_.TransactionRW();
    }
    auto StoreFilters(const filter::Type type, std::pmr::vector<Filter> filters)
        const noexcept -> bool final
    {
        return filters_.StoreFilters(type, std::move(filters));
    }
    auto StoreFilters(
        const filter::Type type,
        const std::pmr::vector<Header>& headers,
        const std::pmr::vector<Filter>& filters,
        const block::Position& tip) const noexcept -> bool final
    {
        return filters_.StoreFilters(type, headers, filters, tip);
    }
    auto StoreFilterHeaders(
        const filter::Type type,
        const ReadView previous,
        const std::pmr::vector<Header> headers) const noexcept -> bool final
    {
        return filters_.StoreHeaders(type, previous, std::move(headers));
    }
    auto StoreSync(const block::Position& tip, const Items& items)
        const noexcept -> bool final
    {
        return sync_.Store(tip, items);
    }
    auto SubchainAddElements(
        const SubchainIndex& index,
        const ElementMap& elements) const noexcept -> bool final
    {
        return wallet_.SubchainAddElements(index, elements);
    }
    auto SubchainLastIndexed(const SubchainIndex& index) const noexcept
        -> std::optional<Bip32Index> final
    {
        return wallet_.SubchainLastIndexed(index);
    }
    auto SubchainLastScanned(const SubchainIndex& index) const noexcept
        -> block::Position final
    {
        return wallet_.SubchainLastScanned(index);
    }
    auto SubchainMatchBlock(
        const SubchainIndex& index,
        const std::pmr::vector<std::pair<ReadView, MatchingIndices>>& results)
        const noexcept -> bool final
    {
        return wallet_.SubchainMatchBlock(index, results);
    }
    auto SubchainSetLastScanned(
        const SubchainIndex& index,
        const block::Position& position) const noexcept -> bool final
    {
        return wallet_.SubchainSetLastScanned(index, position);
    }
    auto SyncTip() const noexcept -> block::Position final
    {
        return sync_.Tip();
    }
    // Returns null pointer if the header does not exist
    auto TryLoadBitcoinHeader(const block::Hash& hash) const noexcept
        -> std::unique_ptr<block::bitcoin::Header> final
    {
        return headers_.TryLoadBitcoinHeader(hash);
    }
    // Returns null pointer if the header does not exist
    auto TryLoadHeader(const block::Hash& hash) const noexcept
        -> std::unique_ptr<block::Header> final
    {
        return headers_.TryLoadHeader(hash);
    }

    Database(
        const api::Session& api,
        const node::internal::Network& network,
        const database::common::Database& common,
        const blockchain::Type chain,
        const blockchain::filter::Type filter) noexcept;

    ~Database() final = default;

private:
    static const VersionNumber db_version_;
    static const storage::lmdb::TableNames table_names_;

    const api::Session& api_;
    const blockchain::Type chain_;
    const database::common::Database& common_;
    storage::lmdb::LMDB lmdb_;
    mutable database::Blocks blocks_;
    mutable database::Filters filters_;
    mutable database::Headers headers_;
    mutable database::Wallet wallet_;
    mutable database::Sync sync_;

    static auto init_db(storage::lmdb::LMDB& db) noexcept -> void;

    Database() = delete;
    Database(const Database&) = delete;
    Database(Database&&) = delete;
    auto operator=(const Database&) -> Database& = delete;
    auto operator=(Database&&) -> Database& = delete;
};
}  // namespace opentxs::blockchain::implementation
