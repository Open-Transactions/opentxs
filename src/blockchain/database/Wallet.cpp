// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/database/Wallet.hpp"  // IWYU pragma: associated

#include <BlockchainTransactionProposal.pb.h>
#include <functional>
#include <memory>
#include <stdexcept>
#include <utility>

#include "blockchain/database/common/Database.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Output.hpp"  // IWYU pragma: keep
#include "opentxs/core/Amount.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::database::implementation
{
Wallet::Wallet(
    const api::Session& api,
    const common::Database& common,
    const storage::lmdb::Database& lmdb,
    const blockchain::Type chain,
    const blockchain::cfilter::Type filter,
    alloc::Default alloc) noexcept
    : HasUpstreamAllocator(alloc.resource())
    , AllocatesChildren(parent_resource())
    , api_(api)
    , common_(common)
    , lmdb_(lmdb)
    , subchains_(api_, lmdb_, filter)
    , proposals_(api_.Crypto(), lmdb_)
    , outputs_(api_, lmdb_, chain, subchains_, proposals_, child_alloc_)
{
}

auto Wallet::AddConfirmedTransactions(
    const Log& log,
    const SubaccountID& account,
    const SubchainID& index,
    BatchedMatches&& transactions,
    TXOs& txoCreated,
    ConsumedTXOs& txoConsumed,
    alloc::Strategy alloc) noexcept -> bool
{
    return outputs_.lock()->AddConfirmedTransactions(
        log,
        account,
        index,
        std::move(transactions),
        txoCreated,
        txoConsumed,
        alloc);
}

auto Wallet::AddMempoolTransaction(
    const Log& log,
    const SubaccountID& subaccount,
    const crypto::Subchain subchain,
    MatchedTransaction&& match,
    TXOs& txoCreated,
    alloc::Strategy alloc) noexcept -> bool
{
    const auto id = subchains_.GetSubchainID(subaccount, subchain);

    return outputs_.lock()->AddMempoolTransaction(
        log, subaccount, id, std::move(match), txoCreated, alloc);
}

auto Wallet::AddProposal(
    const Log& log,
    const identifier::Generic& id,
    const proto::BlockchainTransactionProposal& tx,
    alloc::Strategy alloc) noexcept -> bool
{
    return proposals_.AddProposal(id, tx);
}

auto Wallet::AdvanceTo(
    const Log& log,
    const block::Position& pos,
    alloc::Strategy alloc) noexcept -> bool
{
    return outputs_.lock()->AdvanceTo(log, pos, alloc);
}

auto Wallet::CancelProposal(
    const Log& log,
    const identifier::Generic& id,
    alloc::Strategy alloc) noexcept -> bool
{
    return outputs_.lock()->CancelProposal(log, id, alloc);
}

auto Wallet::CompletedProposals() const noexcept
    -> UnallocatedSet<identifier::Generic>
{
    return proposals_.CompletedProposals();
}

auto Wallet::FinalizeProposal(
    const Log& log,
    const identifier::Generic& proposalID,
    const proto::BlockchainTransactionProposal& proposal,
    const block::Transaction& transaction,
    alloc::Strategy alloc) noexcept -> bool
{
    return outputs_.lock()->FinalizeProposal(
        log, proposalID, proposal, transaction, alloc);
}

auto Wallet::FinalizeReorg(
    const Log& log,
    const block::Position& pos,
    storage::lmdb::Transaction& tx,
    alloc::Strategy alloc) noexcept -> bool
{
    return outputs_.lock()->FinalizeReorg(log, pos, tx, alloc);
}

auto Wallet::ForgetProposals(
    const Log& log,
    const UnallocatedSet<identifier::Generic>& ids,
    alloc::Strategy alloc) noexcept -> bool
{
    return proposals_.ForgetProposals(ids);
}

auto Wallet::GetBalance() const noexcept -> Balance
{
    return outputs_.lock_shared()->GetBalance();
}

auto Wallet::GetBalance(const identifier::Nym& owner) const noexcept -> Balance
{
    return outputs_.lock_shared()->GetBalance(owner);
}

auto Wallet::GetBalance(const identifier::Nym& owner, const SubaccountID& node)
    const noexcept -> Balance
{
    return outputs_.lock_shared()->GetBalance(owner, node);
}

auto Wallet::GetBalance(const crypto::Key& key) const noexcept -> Balance
{
    return outputs_.lock_shared()->GetBalance(key);
}

auto Wallet::GetOutputs(node::TxoState type, alloc::Default alloc)
    const noexcept -> Vector<UTXO>
{
    return outputs_.lock_shared()->GetOutputs(type, alloc);
}

auto Wallet::GetOutputs(
    const identifier::Nym& owner,
    node::TxoState type,
    alloc::Default alloc) const noexcept -> Vector<UTXO>
{
    return outputs_.lock_shared()->GetOutputs(owner, type, alloc);
}

auto Wallet::GetOutputs(
    const identifier::Nym& owner,
    const SubaccountID& node,
    node::TxoState type,
    alloc::Default alloc) const noexcept -> Vector<UTXO>
{
    return outputs_.lock_shared()->GetOutputs(owner, node, type, alloc);
}

auto Wallet::GetOutputs(
    const crypto::Key& key,
    node::TxoState type,
    alloc::Default alloc) const noexcept -> Vector<UTXO>
{
    return outputs_.lock_shared()->GetOutputs(key, type, alloc);
}

auto Wallet::GetOutputTags(const block::Outpoint& output) const noexcept
    -> UnallocatedSet<node::TxoTag>
{
    return outputs_.lock_shared()->GetOutputTags(output);
}

auto Wallet::GetPatterns(const SubchainID& index, alloc::Default alloc)
    const noexcept -> Patterns
{
    return subchains_.GetPatterns(index, alloc);
}

auto Wallet::GetPosition() const noexcept -> block::Position
{
    return outputs_.lock_shared()->GetPosition();
}

auto Wallet::GetReserved(
    const identifier::Generic& proposal,
    alloc::Strategy alloc) const noexcept -> Vector<UTXO>
{
    return outputs_.lock_shared()->GetReserved(proposal, alloc);
}

auto Wallet::GetSubchainID(
    const SubaccountID& balanceNode,
    const crypto::Subchain subchain) const noexcept -> SubchainID
{
    return subchains_.GetSubchainID(balanceNode, subchain);
}

auto Wallet::GetTransactions() const noexcept
    -> UnallocatedVector<block::TransactionHash>
{
    return outputs_.lock_shared()->GetTransactions();
}

auto Wallet::GetTransactions(const identifier::Nym& account) const noexcept
    -> UnallocatedVector<block::TransactionHash>
{
    return outputs_.lock_shared()->GetTransactions(account);
}

auto Wallet::GetUnconfirmedTransactions() const noexcept
    -> UnallocatedSet<block::TransactionHash>
{
    return outputs_.lock_shared()->GetUnconfirmedTransactions();
}

auto Wallet::GetUnspentOutputs(alloc::Default alloc) const noexcept
    -> Vector<UTXO>
{
    return outputs_.lock_shared()->GetUnspentOutputs(alloc);
}

auto Wallet::GetUnspentOutputs(
    const SubaccountID& balanceNode,
    const crypto::Subchain subchain,
    alloc::Default alloc) const noexcept -> Vector<UTXO>
{
    const auto id = subchains_.GetSubchainID(balanceNode, subchain);

    return outputs_.lock_shared()->GetUnspentOutputs(id, alloc);
}

auto Wallet::GetWalletHeight() const noexcept -> block::Height
{
    return outputs_.lock_shared()->GetWalletHeight();
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
    outputs_.lock_shared()->PublishBalance();
}

auto Wallet::ReorgTo(
    const Log& log,
    const node::internal::HeaderOraclePrivate& data,
    const node::HeaderOracle& headers,
    const SubaccountID& balanceNode,
    const crypto::Subchain subchain,
    const SubchainID& index,
    std::span<const block::Position> reorg,
    storage::lmdb::Transaction& tx,
    alloc::Strategy alloc) noexcept -> bool
{
    if (reorg.empty()) { return true; }

    const auto& oldest = reorg.front();
    const auto lastGoodHeight = block::Height{oldest.height_ - 1};
    const auto subchainID = subchains_.GetSubchainID(balanceNode, subchain, tx);

    try {
        if (subchains_.Reorg(data, tx, headers, index, lastGoodHeight)) {
            return true;
        }
    } catch (const std::exception& e) {
        LogAbort()()(e.what()).Abort();
    }

    {
        auto handle = outputs_.lock();

        for (const auto& position : reorg) {
            if (!handle->StartReorg(log, subchainID, position, tx, alloc)) {

                return false;
            }
        }
    }

    return true;
}

auto Wallet::ReserveUTXO(
    const Log& log,
    const identifier::Nym& spender,
    const identifier::Generic& id,
    const node::internal::SpendPolicy& policy,
    alloc::Strategy alloc) noexcept -> std::pair<std::optional<UTXO>, bool>
{
    if (false == proposals_.Exists(id)) {
        LogError()()("Proposal ")(id, api_.Crypto())(" does not exist").Flush();

        return std::make_pair(std::nullopt, false);
    }

    return outputs_.lock()->ReserveUTXO(log, spender, id, policy, alloc);
}

auto Wallet::ReserveUTXO(
    const Log& log,
    const identifier::Nym& spender,
    const identifier::Generic& proposal,
    const block::Outpoint& id,
    alloc::Strategy alloc) noexcept -> std::optional<UTXO>
{
    if (false == proposals_.Exists(proposal)) {
        LogError()()("Proposal ")(proposal, api_.Crypto())(" does not exist")
            .Flush();

        return std::nullopt;
    }

    return outputs_.lock()->ReserveUTXO(log, spender, proposal, id, alloc);
}

auto Wallet::SubchainAddElements(
    const Log& log,
    const SubchainID& index,
    const ElementMap& elements,
    alloc::Strategy alloc) noexcept -> bool
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
    const Log& log,
    const SubchainID& index,
    const block::Position& position,
    alloc::Strategy alloc) noexcept -> bool
{
    return subchains_.SubchainSetLastScanned(index, position);
}
}  // namespace opentxs::blockchain::database::implementation
