// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "internal/util/storage/lmdb/Transaction.hpp"
// IWYU pragma: no_include "opentxs/blockchain/node/TxoState.hpp"
// IWYU pragma: no_include "opentxs/blockchain/node/TxoTag.hpp"

#pragma once

#include <BlockchainTransactionProposal.pb.h>
#include <boost/container/flat_set.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <optional>
#include <utility>

#include "Proto.hpp"
#include "blockchain/database/Blocks.hpp"
#include "blockchain/database/Filters.hpp"
#include "blockchain/database/Headers.hpp"
#include "blockchain/database/Sync.hpp"
#include "blockchain/database/Wallet.hpp"
#include "blockchain/database/common/Database.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/crypto/Crypto.hpp"
#include "internal/blockchain/database/Cfilter.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/database/Peer.hpp"
#include "internal/blockchain/database/Sync.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "internal/blockchain/database/Wallet.hpp"
#include "internal/blockchain/database/common/Common.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/storage/lmdb/Database.hpp"
#include "internal/util/storage/lmdb/Types.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Header.hpp"
#include "opentxs/blockchain/bitcoin/block/Input.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/GCS.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Hash.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Header.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/blockchain/p2p/Address.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/network/otdht/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Pimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
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
class Header;
class Transaction;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Block;
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
struct HeaderOraclePrivate;
struct SpendPolicy;
}  // namespace internal

class HeaderOracle;
class Manager;
class UpdateTransaction;
}  // namespace node

class GCS;
}  // namespace blockchain

namespace identifier
{
class Generic;
class Nym;
}  // namespace identifier

namespace network
{
namespace otdht
{
class Block;
}  // namespace otdht
}  // namespace network

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

class Data;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::implementation
{
class Database final : public database::Database
{
public:
    auto AddConfirmedTransactions(
        const NodeID& account,
        const SubchainIndex& index,
        BatchedMatches&& transactions,
        TXOs& txoCreated,
        TXOs& txoConsumed) noexcept -> bool final
    {
        return wallet_.AddConfirmedTransactions(
            account, index, std::move(transactions), txoCreated, txoConsumed);
    }
    auto AddMempoolTransaction(
        const NodeID& account,
        const crypto::Subchain subchain,
        const Vector<std::uint32_t> outputIndices,
        const bitcoin::block::Transaction& transaction,
        TXOs& txoCreated) noexcept -> bool final
    {
        return wallet_.AddMempoolTransaction(
            account, subchain, outputIndices, transaction, txoCreated);
    }
    auto AddOutgoingTransaction(
        const identifier::Generic& proposalID,
        const proto::BlockchainTransactionProposal& proposal,
        const bitcoin::block::Transaction& transaction) noexcept -> bool final
    {
        return wallet_.AddOutgoingTransaction(
            proposalID, proposal, transaction);
    }
    auto AddOrUpdate(Address address) noexcept -> bool final
    {
        return common_.AddOrUpdate(std::move(address));
    }
    auto AddProposal(
        const identifier::Generic& id,
        const proto::BlockchainTransactionProposal& tx) noexcept -> bool final
    {
        return wallet_.AddProposal(id, tx);
    }
    auto AdvanceTo(const block::Position& pos) noexcept -> bool final
    {
        return wallet_.AdvanceTo(pos);
    }
    auto ApplyUpdate(const node::UpdateTransaction& update) noexcept
        -> bool final
    {
        return headers_.ApplyUpdate(update);
    }
    // Throws std::out_of_range if no block at that position
    auto BestBlock(const block::Height position) const noexcept(false)
        -> block::Hash final
    {
        return headers_.BestBlock(position);
    }
    auto BlockExists(const block::Hash& block) const noexcept -> bool final
    {
        return common_.BlockExists(block);
    }
    auto BlockLoadBitcoin(const block::Hash& block) const noexcept
        -> std::shared_ptr<const bitcoin::block::Block> final
    {
        return blocks_.LoadBitcoin(block);
    }
    auto BlockStore(const block::Block& block) noexcept -> bool final
    {
        return blocks_.Store(block);
    }
    auto BlockTip() const noexcept -> block::Position final
    {
        return blocks_.Tip();
    }
    auto CompletedProposals() const noexcept
        -> UnallocatedSet<identifier::Generic> final
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
    auto CancelProposal(const identifier::Generic& id) noexcept -> bool final
    {
        return wallet_.CancelProposal(id);
    }
    auto FilterHeaderTip(const cfilter::Type type) const noexcept
        -> block::Position final
    {
        return filters_.CurrentHeaderTip(type);
    }
    auto FilterTip(const cfilter::Type type) const noexcept
        -> block::Position final
    {
        return filters_.CurrentTip(type);
    }
    auto FinalizeReorg(
        storage::lmdb::Transaction& tx,
        const block::Position& pos) noexcept -> bool final
    {
        return wallet_.FinalizeReorg(tx, pos);
    }
    auto ForgetProposals(
        const UnallocatedSet<identifier::Generic>& ids) noexcept -> bool final
    {
        return wallet_.ForgetProposals(ids);
    }
    auto DisconnectedHashes() const noexcept -> database::DisconnectedList final
    {
        return headers_.DisconnectedHashes();
    }
    auto Get(
        const Protocol protocol,
        const UnallocatedSet<Type> onNetworks,
        const UnallocatedSet<Service> withServices) const noexcept
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
    auto GetOutputs(node::TxoState type, alloc::Default alloc) const noexcept
        -> Vector<UTXO> final
    {
        return wallet_.GetOutputs(type, alloc);
    }
    auto GetOutputs(
        const identifier::Nym& owner,
        node::TxoState type,
        alloc::Default alloc) const noexcept -> Vector<UTXO> final
    {
        return wallet_.GetOutputs(owner, type, alloc);
    }
    auto GetOutputs(
        const identifier::Nym& owner,
        const identifier::Generic& node,
        node::TxoState type,
        alloc::Default alloc) const noexcept -> Vector<UTXO> final
    {
        return wallet_.GetOutputs(owner, node, type, alloc);
    }
    auto GetOutputs(
        const crypto::Key& key,
        node::TxoState type,
        alloc::Default alloc) const noexcept -> Vector<UTXO> final
    {
        return wallet_.GetOutputs(key, type, alloc);
    }
    auto GetOutputTags(const block::Outpoint& output) const noexcept
        -> UnallocatedSet<node::TxoTag> final
    {
        return wallet_.GetOutputTags(output);
    }
    auto GetPatterns(const SubchainIndex& index, alloc::Default alloc)
        const noexcept -> Patterns final
    {
        return wallet_.GetPatterns(index, alloc);
    }
    auto GetPosition() const noexcept -> block::Position final
    {
        return wallet_.GetPosition();
    }
    auto GetSubchainID(const NodeID& account, const crypto::Subchain subchain)
        const noexcept -> SubchainIndex final
    {
        return wallet_.GetSubchainID(account, subchain);
    }
    auto GetTransactions() const noexcept
        -> UnallocatedVector<block::pTxid> final
    {
        return wallet_.GetTransactions();
    }
    auto GetTransactions(const identifier::Nym& account) const noexcept
        -> UnallocatedVector<block::pTxid> final
    {
        return wallet_.GetTransactions(account);
    }
    auto GetUnconfirmedTransactions() const noexcept
        -> UnallocatedSet<block::pTxid> final
    {
        return wallet_.GetUnconfirmedTransactions();
    }
    auto GetUnspentOutputs(alloc::Default alloc) const noexcept
        -> Vector<UTXO> final
    {
        return wallet_.GetUnspentOutputs(alloc);
    }
    auto GetUnspentOutputs(
        const NodeID& account,
        const crypto::Subchain subchain,
        alloc::Default alloc) const noexcept -> Vector<UTXO> final
    {
        return wallet_.GetUnspentOutputs(account, subchain, alloc);
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
    auto HaveFilter(const cfilter::Type type, const block::Hash& block)
        const noexcept -> bool final
    {
        return filters_.HaveFilter(type, block);
    }
    auto HaveFilterHeader(const cfilter::Type type, const block::Hash& block)
        const noexcept -> bool final
    {
        return filters_.HaveFilterHeader(type, block);
    }
    auto HeaderExists(const block::Hash& hash) const noexcept -> bool final
    {
        return headers_.HeaderExists(hash);
    }
    auto Import(UnallocatedVector<Address> peers) noexcept -> bool final
    {
        return common_.Import(std::move(peers));
    }
    auto IsSibling(const block::Hash& hash) const noexcept -> bool final
    {
        return headers_.IsSibling(hash);
    }
    auto LoadFilter(
        const cfilter::Type type,
        const ReadView block,
        alloc::Default alloc) const noexcept -> blockchain::GCS final
    {
        return filters_.LoadFilter(type, block, alloc);
    }
    auto LoadFilters(
        const cfilter::Type type,
        const Vector<block::Hash>& blocks) const noexcept
        -> Vector<blockchain::GCS> final
    {
        return filters_.LoadFilters(type, blocks);
    }
    auto LoadFilterHash(const cfilter::Type type, const ReadView block)
        const noexcept -> cfilter::Hash final
    {
        return filters_.LoadFilterHash(type, block);
    }
    auto LoadFilterHeader(const cfilter::Type type, const ReadView block)
        const noexcept -> cfilter::Header final
    {
        return filters_.LoadFilterHeader(type, block);
    }
    // Throws std::out_of_range if the header does not exist
    auto LoadHeader(const block::Hash& hash) const noexcept(false)
        -> std::unique_ptr<block::Header> final
    {
        return headers_.LoadHeader(hash);
    }
    auto LoadProposal(const identifier::Generic& id) const noexcept
        -> std::optional<proto::BlockchainTransactionProposal> final
    {
        return wallet_.LoadProposal(id);
    }
    auto LoadProposals() const noexcept
        -> UnallocatedVector<proto::BlockchainTransactionProposal> final
    {
        return wallet_.LoadProposals();
    }
    auto LoadSync(const Height height, Message& output) noexcept -> bool final
    {
        return sync_.Load(height, output);
    }
    auto LookupContact(const Data& pubkeyHash) const noexcept
        -> UnallocatedSet<identifier::Generic> final
    {
        return wallet_.LookupContact(pubkeyHash);
    }
    auto PublishBalance() const noexcept -> void final
    {
        wallet_.PublishBalance();
    }
    auto RecentHashes(alloc::Default alloc) const noexcept
        -> Vector<block::Hash> final
    {
        return headers_.RecentHashes(alloc);
    }
    auto ReorgSync(const Height height) noexcept -> bool final
    {
        return sync_.Reorg(height);
    }
    auto ReorgTo(
        const node::internal::HeaderOraclePrivate& data,
        storage::lmdb::Transaction& tx,
        const node::HeaderOracle& headers,
        const NodeID& account,
        const crypto::Subchain subchain,
        const SubchainIndex& index,
        const UnallocatedVector<block::Position>& reorg) noexcept -> bool final
    {
        return wallet_.ReorgTo(
            data, tx, headers, account, subchain, index, reorg);
    }
    auto ReportHeaderTip() noexcept -> void final { headers_.ReportTip(); }
    auto ReserveUTXO(
        const identifier::Nym& spender,
        const identifier::Generic& proposal,
        node::internal::SpendPolicy& policy) noexcept
        -> std::optional<UTXO> final
    {
        return wallet_.ReserveUTXO(spender, proposal, policy);
    }
    auto SetBlockTip(const block::Position& position) noexcept -> bool final
    {
        return blocks_.SetTip(position);
    }
    auto SetFilterHeaderTip(
        const cfilter::Type type,
        const block::Position& position) noexcept -> bool final
    {
        return filters_.SetHeaderTip(type, position);
    }
    auto SetFilterTip(
        const cfilter::Type type,
        const block::Position& position) noexcept -> bool final
    {
        return filters_.SetTip(type, position);
    }
    auto SetSyncTip(const block::Position& position) noexcept -> bool final
    {
        return sync_.SetTip(position);
    }
    auto SiblingHashes() const noexcept -> database::Hashes final
    {
        return headers_.SiblingHashes();
    }
    auto StartReorg() noexcept -> storage::lmdb::Transaction final;
    auto StoreFilters(
        const cfilter::Type type,
        Vector<CFilterParams> filters) noexcept -> bool final
    {
        return filters_.StoreFilters(type, std::move(filters));
    }
    auto StoreFilters(
        const cfilter::Type type,
        const Vector<CFHeaderParams>& headers,
        const Vector<CFilterParams>& filters,
        const block::Position& tip) noexcept -> bool final
    {
        return filters_.StoreFilters(type, headers, filters, tip);
    }
    auto StoreFilterHeaders(
        const cfilter::Type type,
        const ReadView previous,
        const Vector<CFHeaderParams> headers) noexcept -> bool final
    {
        return filters_.StoreHeaders(type, previous, std::move(headers));
    }
    auto StoreSync(
        const block::Position& tip,
        const network::otdht::SyncData& items) noexcept -> bool final
    {
        return sync_.Store(tip, items);
    }
    auto SubchainAddElements(
        const SubchainIndex& index,
        const ElementMap& elements) noexcept -> bool final
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
        -> std::unique_ptr<bitcoin::block::Header> final
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
        const node::Manager& network,
        const database::common::Database& common,
        const blockchain::Type chain,
        const blockchain::cfilter::Type filter) noexcept;
    Database() = delete;
    Database(const Database&) = delete;
    Database(Database&&) = delete;
    auto operator=(const Database&) -> Database& = delete;
    auto operator=(Database&&) -> Database& = delete;

    ~Database() final = default;

private:
    static const VersionNumber db_version_;
    static const storage::lmdb::TableNames table_names_;

    const api::Session& api_;
    const blockchain::Type chain_;
    const database::common::Database& common_;
    storage::lmdb::Database lmdb_;
    mutable database::Blocks blocks_;
    mutable database::Filters filters_;
    mutable database::Headers headers_;
    mutable database::implemenation::Wallet wallet_;
    mutable database::implementation::Sync sync_;

    static auto init_db(storage::lmdb::Database& db) noexcept -> void;
};
}  // namespace opentxs::blockchain::implementation
