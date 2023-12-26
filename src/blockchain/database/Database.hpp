// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "internal/util/storage/lmdb/Transaction.hpp"

#pragma once

#include <BlockchainTransactionProposal.pb.h>
#include <functional>
#include <optional>
#include <span>
#include <utility>

#include "blockchain/database/Blocks.hpp"
#include "blockchain/database/Filters.hpp"
#include "blockchain/database/Headers.hpp"
#include "blockchain/database/Sync.hpp"
#include "blockchain/database/Wallet.hpp"
#include "blockchain/database/common/Database.hpp"
#include "internal/blockchain/database/Database.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "internal/blockchain/database/common/Common.hpp"
#include "internal/util/alloc/AllocatesChildren.hpp"
#include "internal/util/storage/lmdb/Database.hpp"
#include "internal/util/storage/lmdb/Types.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/block/Types.internal.hpp"
#include "opentxs/blockchain/cfilter/GCS.hpp"
#include "opentxs/blockchain/cfilter/Hash.hpp"
#include "opentxs/blockchain/cfilter/Header.hpp"
#include "opentxs/blockchain/cfilter/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Output.hpp"  // IWYU pragma: keep
#include "opentxs/core/Amount.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/network/blockchain/Address.hpp"
#include "opentxs/network/blockchain/Types.hpp"
#include "opentxs/network/blockchain/bitcoin/Types.hpp"
#include "opentxs/network/otdht/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

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
class Transaction;
}  // namespace block

namespace node
{
namespace internal
{
struct HeaderOraclePrivate;
struct SpendPolicy;
}  // namespace internal

class HeaderOracle;
class UpdateTransaction;
struct Endpoints;
}  // namespace node
}  // namespace blockchain

namespace identifier
{
class Nym;
}  // namespace identifier

class Data;
class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database::implementation
{
class Database final : public database::Database,
                       private pmr::AllocatesChildren<alloc::PoolSync>
{
public:
    auto AddConfirmedTransactions(
        const Log& log,
        const SubaccountID& account,
        const SubchainID& index,
        BatchedMatches&& transactions,
        TXOs& txoCreated,
        ConsumedTXOs& txoConsumed,
        alloc::Strategy alloc) noexcept -> bool final
    {
        return wallet_.AddConfirmedTransactions(
            log,
            account,
            index,
            std::move(transactions),
            txoCreated,
            txoConsumed,
            alloc);
    }
    auto AddMempoolTransaction(
        const Log& log,
        const SubaccountID& account,
        const crypto::Subchain subchain,
        MatchedTransaction&& match,
        TXOs& txoCreated,
        alloc::Strategy alloc) noexcept -> bool final
    {
        return wallet_.AddMempoolTransaction(
            log, account, subchain, std::move(match), txoCreated, alloc);
    }
    auto AddOrUpdate(network::blockchain::Address address) noexcept
        -> bool final
    {
        return common_.AddOrUpdate(std::move(address));
    }
    auto AddProposal(
        const Log& log,
        const identifier::Generic& id,
        const proto::BlockchainTransactionProposal& tx,
        alloc::Strategy alloc) noexcept -> bool final
    {
        return wallet_.AddProposal(log, id, tx, alloc);
    }
    auto AdvanceTo(
        const Log& log,
        const block::Position& pos,
        alloc::Strategy alloc) noexcept -> bool final
    {
        return wallet_.AdvanceTo(log, pos, alloc);
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
    auto BlockDelete(const block::Hash& block) const noexcept -> bool final
    {
        return common_.BlockForget(block);
    }
    auto BlockExists(const block::Hash& block) const noexcept -> bool final
    {
        return common_.BlockExists(block);
    }
    auto BlockLoad(
        const std::span<const block::Hash> hashes,
        alloc::Default alloc,
        alloc::Default monotonic) const noexcept -> Vector<ReadView> final
    {
        return common_.BlockLoad(chain_, hashes, alloc, monotonic);
    }
    auto BlockStore(
        const block::Hash& id,
        const ReadView bytes,
        alloc::Default monotonic) noexcept -> ReadView final
    {
        return common_.BlockStore(id, bytes, monotonic);
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
    auto CurrentBest() const noexcept -> block::Header final
    {
        return headers_.CurrentBest();
    }
    auto CurrentCheckpoint() const noexcept -> block::Position final
    {
        return headers_.CurrentCheckpoint();
    }
    auto CancelProposal(
        const Log& log,
        const identifier::Generic& id,
        alloc::Strategy alloc) noexcept -> bool final
    {
        return wallet_.CancelProposal(log, id, alloc);
    }
    auto Confirm(const network::blockchain::AddressID& id) noexcept
        -> void final
    {
        return common_.Confirm(chain_, id);
    }
    auto Fail(const network::blockchain::AddressID& id) noexcept -> void final
    {
        return common_.Fail(chain_, id);
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
    auto FinalizeProposal(
        const Log& log,
        const identifier::Generic& proposalID,
        const proto::BlockchainTransactionProposal& proposal,
        const block::Transaction& transaction,
        alloc::Strategy alloc) noexcept -> bool final
    {
        return wallet_.FinalizeProposal(
            log, proposalID, proposal, transaction, alloc);
    }
    auto FinalizeReorg(
        const Log& log,
        const block::Position& pos,
        storage::lmdb::Transaction& tx,
        alloc::Strategy alloc) noexcept -> bool final
    {
        return wallet_.FinalizeReorg(log, pos, tx, alloc);
    }
    auto ForgetProposals(
        const Log& log,
        const UnallocatedSet<identifier::Generic>& ids,
        alloc::Strategy alloc) noexcept -> bool final
    {
        return wallet_.ForgetProposals(log, ids, alloc);
    }
    auto DisconnectedHashes() const noexcept -> database::DisconnectedList final
    {
        return headers_.DisconnectedHashes();
    }
    auto Get(
        const Protocol protocol,
        const Set<Transport>& onNetworks,
        const Set<Service>& withServices,
        const Set<network::blockchain::AddressID>& exclude) noexcept
        -> network::blockchain::Address final
    {
        return common_.Find(
            chain_, protocol, onNetworks, withServices, exclude);
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
    auto GetBalance(const identifier::Nym& owner, const SubaccountID& node)
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
        const SubaccountID& node,
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
    auto GetPatterns(const SubchainID& index, alloc::Default alloc)
        const noexcept -> Patterns final
    {
        return wallet_.GetPatterns(index, alloc);
    }
    auto GetPosition() const noexcept -> block::Position final
    {
        return wallet_.GetPosition();
    }
    auto GetReserved(const identifier::Generic& proposal, alloc::Strategy alloc)
        const noexcept -> Vector<UTXO> final
    {
        return wallet_.GetReserved(proposal, alloc);
    }
    auto GetSubchainID(
        const SubaccountID& account,
        const crypto::Subchain subchain) const noexcept -> SubchainID final
    {
        return wallet_.GetSubchainID(account, subchain);
    }
    auto GetTransactions() const noexcept
        -> UnallocatedVector<block::TransactionHash> final
    {
        return wallet_.GetTransactions();
    }
    auto GetTransactions(const identifier::Nym& account) const noexcept
        -> UnallocatedVector<block::TransactionHash> final
    {
        return wallet_.GetTransactions(account);
    }
    auto GetUnconfirmedTransactions() const noexcept
        -> UnallocatedSet<block::TransactionHash> final
    {
        return wallet_.GetUnconfirmedTransactions();
    }
    auto GetUnspentOutputs(alloc::Default alloc) const noexcept
        -> Vector<UTXO> final
    {
        return wallet_.GetUnspentOutputs(alloc);
    }
    auto GetUnspentOutputs(
        const SubaccountID& account,
        const crypto::Subchain subchain,
        alloc::Default alloc) const noexcept -> Vector<UTXO> final
    {
        return wallet_.GetUnspentOutputs(account, subchain, alloc);
    }
    auto GetWalletHeight() const noexcept -> block::Height final
    {
        return wallet_.GetWalletHeight();
    }
    auto Good(alloc::Default alloc, alloc::Default monotonic) const noexcept
        -> Vector<network::blockchain::Address> final
    {
        return common_.Good(chain_, alloc, monotonic);
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
    auto Import(Vector<network::blockchain::Address> peers) noexcept
        -> bool final
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
        alloc::Strategy alloc) const noexcept -> cfilter::GCS final
    {
        return filters_.LoadFilter(type, block, alloc);
    }
    auto LoadFilters(
        const cfilter::Type type,
        std::span<const block::Hash> blocks,
        alloc::Strategy alloc) const noexcept -> Vector<cfilter::GCS> final
    {
        return filters_.LoadFilters(type, blocks, alloc);
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
        -> block::Header final
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
    auto PeerIsReady() const noexcept -> bool final
    {
        return common_.PeerIsReady();
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
    auto Release(const network::blockchain::AddressID& id) noexcept
        -> void final
    {
        return common_.Release(chain_, id);
    }
    auto ReorgSync(const Height height) noexcept -> bool final
    {
        return sync_.Reorg(height);
    }
    auto ReorgTo(
        const Log& log,
        const node::internal::HeaderOraclePrivate& data,
        const node::HeaderOracle& headers,
        const SubaccountID& account,
        const crypto::Subchain subchain,
        const SubchainID& index,
        std::span<const block::Position> reorg,
        storage::lmdb::Transaction& tx,
        alloc::Strategy alloc) noexcept -> bool final
    {
        return wallet_.ReorgTo(
            log, data, headers, account, subchain, index, reorg, tx, alloc);
    }
    auto ReportHeaderTip() noexcept -> void final { headers_.ReportTip(); }
    auto ReserveUTXO(
        const Log& log,
        const identifier::Nym& spender,
        const identifier::Generic& proposal,
        const node::internal::SpendPolicy& policy,
        alloc::Strategy alloc) noexcept
        -> std::pair<std::optional<UTXO>, bool> final
    {
        return wallet_.ReserveUTXO(log, spender, proposal, policy, alloc);
    }
    auto ReserveUTXO(
        const Log& log,
        const identifier::Nym& spender,
        const identifier::Generic& proposal,
        const block::Outpoint& id,
        alloc::Strategy alloc) noexcept -> std::optional<UTXO> final
    {
        return wallet_.ReserveUTXO(log, spender, proposal, id, alloc);
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
    auto StartReorg(const Log& log) noexcept
        -> storage::lmdb::Transaction final;
    auto StoreFilters(
        const cfilter::Type type,
        Vector<CFilterParams> filters,
        alloc::Strategy alloc) noexcept -> bool final
    {
        return filters_.StoreFilters(type, std::move(filters), alloc);
    }
    auto StoreFilters(
        const cfilter::Type type,
        const Vector<CFHeaderParams>& headers,
        const Vector<CFilterParams>& filters,
        const block::Position& tip,
        alloc::Strategy alloc) noexcept -> bool final
    {
        return filters_.StoreFilters(type, headers, filters, tip, alloc);
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
        const Log& log,
        const SubchainID& index,
        const ElementMap& elements,
        alloc::Strategy alloc) noexcept -> bool final
    {
        return wallet_.SubchainAddElements(log, index, elements, alloc);
    }
    auto SubchainLastIndexed(const SubchainID& index) const noexcept
        -> std::optional<crypto::Bip32Index> final
    {
        return wallet_.SubchainLastIndexed(index);
    }
    auto SubchainLastScanned(const SubchainID& index) const noexcept
        -> block::Position final
    {
        return wallet_.SubchainLastScanned(index);
    }
    auto SubchainSetLastScanned(
        const Log& log,
        const SubchainID& index,
        const block::Position& position,
        alloc::Strategy alloc) noexcept -> bool final
    {
        return wallet_.SubchainSetLastScanned(log, index, position, alloc);
    }
    auto SyncTip() const noexcept -> block::Position final
    {
        return sync_.Tip();
    }
    auto TryLoadHeader(const block::Hash& hash) const noexcept
        -> block::Header final
    {
        return headers_.TryLoadHeader(hash);
    }

    Database(
        const api::Session& api,
        const node::Endpoints& endpoints,
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
    const VersionNumber original_version_;
    const VersionNumber current_version_;
    mutable database::Blocks blocks_;
    mutable database::Filters filters_;
    mutable database::Headers headers_;
    mutable database::implementation::Wallet wallet_;
    mutable database::implementation::Sync sync_;

    static auto get_current_version(
        const VersionNumber& original,
        storage::lmdb::Database& db) noexcept -> VersionNumber;
    static auto get_original_version(storage::lmdb::Database& db) noexcept
        -> VersionNumber;
    static auto init_db(
        const VersionNumber target,
        const VersionNumber current,
        storage::lmdb::Database& db) noexcept -> void;
};
}  // namespace opentxs::blockchain::database::implementation
