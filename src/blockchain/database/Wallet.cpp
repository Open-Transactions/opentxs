// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type
// IWYU pragma: no_forward_declare opentxs::blockchain::cfilter::Type
// IWYU pragma: no_forward_declare opentxs::blockchain::crypto::Subchain
// IWYU pragma: no_forward_declare opentxs::blockchain::node::TxoState
// IWYU pragma: no_forward_declare opentxs::blockchain::node::TxoTag

#include "0_stdafx.hpp"                    // IWYU pragma: associated
#include "blockchain/database/Wallet.hpp"  // IWYU pragma: associated

#include <BlockchainTransactionProposal.pb.h>
#include <stdexcept>
#include <utility>

#include "blockchain/database/common/Database.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/core/Amount.hpp"  // IWYU pragma: keep
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::database::implementation
{
Wallet::Wallet(
    const api::Session& api,
    const common::Database& common,
    const storage::lmdb::Database& lmdb,
    const blockchain::Type chain,
    const blockchain::cfilter::Type filter) noexcept
    : api_(api)
    , common_(common)
    , lmdb_(lmdb)
    , subchains_(api_, lmdb_, filter)
    , proposals_(lmdb_)
    , outputs_(api_, lmdb_, chain, subchains_, proposals_)
{
}

auto Wallet::AddConfirmedTransactions(
    const SubaccountID& account,
    const SubchainID& index,
    BatchedMatches&& transactions,
    TXOs& txoCreated,
    TXOs& txoConsumed) noexcept -> bool
{
    return outputs_.AddConfirmedTransactions(
        account, index, std::move(transactions), txoCreated, txoConsumed);
}

auto Wallet::AddMempoolTransaction(
    const SubaccountID& balanceNode,
    const crypto::Subchain subchain,
    const Vector<std::uint32_t> outputIndices,
    const bitcoin::block::Transaction& original,
    TXOs& txoCreated) const noexcept -> bool
{
    const auto id = subchains_.GetSubchainID(balanceNode, subchain);

    return outputs_.AddMempoolTransaction(
        balanceNode, id, outputIndices, original, txoCreated);
}

auto Wallet::AddOutgoingTransaction(
    const identifier::Generic& proposalID,
    const proto::BlockchainTransactionProposal& proposal,
    const bitcoin::block::Transaction& transaction) const noexcept -> bool
{
    return outputs_.AddOutgoingTransaction(proposalID, proposal, transaction);
}

auto Wallet::AddProposal(
    const identifier::Generic& id,
    const proto::BlockchainTransactionProposal& tx) const noexcept -> bool
{
    return proposals_.AddProposal(id, tx);
}

auto Wallet::AdvanceTo(const block::Position& pos) const noexcept -> bool
{
    return outputs_.AdvanceTo(pos);
}

auto Wallet::CancelProposal(const identifier::Generic& id) const noexcept
    -> bool
{
    return outputs_.CancelProposal(id);
}

auto Wallet::CompletedProposals() const noexcept
    -> UnallocatedSet<identifier::Generic>
{
    return proposals_.CompletedProposals();
}

auto Wallet::FinalizeReorg(
    storage::lmdb::Transaction& tx,
    const block::Position& pos) const noexcept -> bool
{
    return outputs_.FinalizeReorg(tx, pos);
}

auto Wallet::ForgetProposals(
    const UnallocatedSet<identifier::Generic>& ids) const noexcept -> bool
{
    return proposals_.ForgetProposals(ids);
}

auto Wallet::GetBalance() const noexcept -> Balance
{
    return outputs_.GetBalance();
}

auto Wallet::GetBalance(const identifier::Nym& owner) const noexcept -> Balance
{
    return outputs_.GetBalance(owner);
}

auto Wallet::GetBalance(const identifier::Nym& owner, const SubaccountID& node)
    const noexcept -> Balance
{
    return outputs_.GetBalance(owner, node);
}

auto Wallet::GetBalance(const crypto::Key& key) const noexcept -> Balance
{
    return outputs_.GetBalance(key);
}

auto Wallet::GetOutputs(node::TxoState type, alloc::Default alloc)
    const noexcept -> Vector<UTXO>
{
    return outputs_.GetOutputs(type, alloc);
}

auto Wallet::GetOutputs(
    const identifier::Nym& owner,
    node::TxoState type,
    alloc::Default alloc) const noexcept -> Vector<UTXO>
{
    return outputs_.GetOutputs(owner, type, alloc);
}

auto Wallet::GetOutputs(
    const identifier::Nym& owner,
    const identifier::Generic& node,
    node::TxoState type,
    alloc::Default alloc) const noexcept -> Vector<UTXO>
{
    return outputs_.GetOutputs(owner, node, type, alloc);
}

auto Wallet::GetOutputs(
    const crypto::Key& key,
    node::TxoState type,
    alloc::Default alloc) const noexcept -> Vector<UTXO>
{
    return outputs_.GetOutputs(key, type, alloc);
}

auto Wallet::GetOutputTags(const block::Outpoint& output) const noexcept
    -> UnallocatedSet<node::TxoTag>
{
    return outputs_.GetOutputTags(output);
}

auto Wallet::GetPatterns(const SubchainID& index, alloc::Default alloc)
    const noexcept -> Patterns
{
    return subchains_.GetPatterns(index, alloc);
}

auto Wallet::GetPosition() const noexcept -> block::Position
{
    return outputs_.GetPosition();
}

auto Wallet::GetSubchainID(
    const SubaccountID& balanceNode,
    const crypto::Subchain subchain) const noexcept -> SubchainID
{
    return subchains_.GetSubchainID(balanceNode, subchain);
}

auto Wallet::GetTransactions() const noexcept -> UnallocatedVector<block::pTxid>
{
    return outputs_.GetTransactions();
}

auto Wallet::GetTransactions(const identifier::Nym& account) const noexcept
    -> UnallocatedVector<block::pTxid>
{
    return outputs_.GetTransactions(account);
}

auto Wallet::GetUnconfirmedTransactions() const noexcept
    -> UnallocatedSet<block::pTxid>
{
    return outputs_.GetUnconfirmedTransactions();
}

auto Wallet::GetUnspentOutputs(alloc::Default alloc) const noexcept
    -> Vector<UTXO>
{
    return outputs_.GetUnspentOutputs(alloc);
}

auto Wallet::GetUnspentOutputs(
    const SubaccountID& balanceNode,
    const crypto::Subchain subchain,
    alloc::Default alloc) const noexcept -> Vector<UTXO>
{
    const auto id = subchains_.GetSubchainID(balanceNode, subchain);

    return outputs_.GetUnspentOutputs(id, alloc);
}

auto Wallet::GetWalletHeight() const noexcept -> block::Height
{
    return outputs_.GetWalletHeight();
}

auto Wallet::LoadProposal(const identifier::Generic& id) const noexcept
    -> std::optional<proto::BlockchainTransactionProposal>
{
    return proposals_.LoadProposal(id);
}

auto Wallet::LoadProposals() const noexcept
    -> UnallocatedVector<proto::BlockchainTransactionProposal>
{
    return proposals_.LoadProposals();
}

auto Wallet::LookupContact(const Data& pubkeyHash) const noexcept
    -> UnallocatedSet<identifier::Generic>
{
    return common_.LookupContact(pubkeyHash);
}

auto Wallet::PublishBalance() const noexcept -> void
{
    outputs_.PublishBalance();
}

auto Wallet::ReorgTo(
    const node::internal::HeaderOraclePrivate& data,
    storage::lmdb::Transaction& tx,
    const node::HeaderOracle& headers,
    const SubaccountID& balanceNode,
    const crypto::Subchain subchain,
    const SubchainID& index,
    const UnallocatedVector<block::Position>& reorg) const noexcept -> bool
{
    if (reorg.empty()) { return true; }

    const auto& oldest = *reorg.crbegin();
    const auto lastGoodHeight = block::Height{oldest.height_ - 1};
    const auto subchainID = subchains_.GetSubchainID(balanceNode, subchain, tx);

    try {
        if (subchains_.Reorg(data, tx, headers, index, lastGoodHeight)) {
            return true;
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        OT_FAIL;
    }

    for (const auto& position : reorg) {
        if (false == outputs_.StartReorg(tx, subchainID, position)) {

            return false;
        }
    }

    return true;
}

auto Wallet::ReserveUTXO(
    const identifier::Nym& spender,
    const identifier::Generic& id,
    node::internal::SpendPolicy& policy) const noexcept -> std::optional<UTXO>
{
    if (false == proposals_.Exists(id)) {
        LogError()(OT_PRETTY_CLASS())("Proposal ")(id)(" does not exist")
            .Flush();

        return std::nullopt;
    }

    return outputs_.ReserveUTXO(spender, id, policy);
}

auto Wallet::SubchainAddElements(
    const SubchainID& index,
    const ElementMap& elements) const noexcept -> bool
{
    return subchains_.SubchainAddElements(index, elements);
}

auto Wallet::SubchainLastIndexed(const SubchainID& index) const noexcept
    -> std::optional<Bip32Index>
{
    return subchains_.SubchainLastIndexed(index);
}

auto Wallet::SubchainLastScanned(const SubchainID& index) const noexcept
    -> block::Position
{
    return subchains_.SubchainLastScanned(index);
}

auto Wallet::SubchainSetLastScanned(
    const SubchainID& index,
    const block::Position& position) const noexcept -> bool
{
    return subchains_.SubchainSetLastScanned(index, position);
}
}  // namespace opentxs::blockchain::database::implementation
