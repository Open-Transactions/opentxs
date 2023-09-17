// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_plain_guarded.h>
#include <cstring>
#include <functional>
#include <optional>
#include <span>
#include <utility>

#include "blockchain/database/wallet/OutputCache.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/alloc/AllocatesChildren.hpp"
#include "internal/util/alloc/Boost.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Output.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

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
class TransactionHash;
}  // namespace block

namespace database
{
namespace wallet
{
class Proposal;
class SubchainData;
}  // namespace wallet
}  // namespace database

namespace node
{
namespace internal
{
struct SpendPolicy;
}  // namespace internal
}  // namespace node

namespace protocol
{
namespace bitcoin
{
namespace base
{
namespace block
{
namespace internal
{
class Transaction;
}  // namespace internal
}  // namespace block
}  // namespace base
}  // namespace bitcoin
}  // namespace protocol
}  // namespace blockchain

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
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database::wallet
{
using NymBalances = UnallocatedMap<identifier::Nym, Balance>;

class Output final : private pmr::HasUpstreamAllocator,
                     private pmr::AllocatesChildren<alloc::BoostPoolSync>
{
public:
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
    auto GetPosition() const noexcept -> block::Position;
    auto GetReserved(const identifier::Generic& proposal, alloc::Strategy alloc)
        const noexcept -> Vector<UTXO>;
    auto GetTransactions() const noexcept
        -> UnallocatedVector<block::TransactionHash>;
    auto GetTransactions(const identifier::Nym& account) const noexcept
        -> UnallocatedVector<block::TransactionHash>;
    auto GetUnconfirmedTransactions() const noexcept
        -> UnallocatedSet<block::TransactionHash>;
    auto GetUnspentOutputs(alloc::Default alloc) const noexcept -> Vector<UTXO>;
    auto GetUnspentOutputs(
        const SubaccountID& balanceNode,
        alloc::Default alloc) const noexcept -> Vector<UTXO>;
    auto GetWalletHeight() const noexcept -> block::Height;
    auto PublishBalance() const noexcept -> void;

    auto AddConfirmedTransactions(
        const Log& log,
        const SubaccountID& account,
        const SubchainID& index,
        BatchedMatches&& transactions,
        TXOs& txoCreated,
        TXOs& txoConsumed,
        alloc::Strategy alloc) noexcept -> bool;
    auto AddMempoolTransaction(
        const Log& log,
        const AccountID& account,
        const SubchainID& subchain,
        MatchedTransaction&& match,
        TXOs& txoConsumed,
        alloc::Strategy alloc) noexcept -> bool;
    auto AdvanceTo(
        const Log& log,
        const block::Position& pos,
        alloc::Strategy alloc) noexcept -> bool;
    auto CancelProposal(
        const Log& log,
        const identifier::Generic& id,
        alloc::Strategy alloc) noexcept -> bool;
    auto FinalizeProposal(
        const Log& log,
        const identifier::Generic& proposalID,
        const proto::BlockchainTransactionProposal& proposal,
        const block::Transaction& transaction,
        alloc::Strategy alloc) noexcept -> bool;
    auto FinalizeReorg(
        const Log& log,
        const block::Position& pos,
        storage::lmdb::Transaction& tx,
        alloc::Strategy alloc) noexcept -> bool;
    auto ReserveUTXO(
        const Log& log,
        const identifier::Nym& spender,
        const identifier::Generic& proposal,
        const node::internal::SpendPolicy& policy,
        alloc::Strategy alloc) noexcept -> std::pair<std::optional<UTXO>, bool>;
    auto ReserveUTXO(
        const Log& log,
        const identifier::Nym& spender,
        const identifier::Generic& proposal,
        const block::Outpoint& id,
        alloc::Strategy alloc) noexcept -> std::optional<UTXO>;
    auto StartReorg(
        const Log& log,
        const SubchainID& subchain,
        const block::Position& position,
        storage::lmdb::Transaction& tx,
        alloc::Strategy alloc) noexcept -> bool;

    Output(
        const api::Session& api,
        const storage::lmdb::Database& lmdb,
        const blockchain::Type chain,
        const wallet::SubchainData& subchains,
        wallet::Proposal& proposals,
        alloc::Default alloc) noexcept;
    Output() = delete;
    Output(const Output&) = delete;
    auto operator=(const Output&) -> Output& = delete;
    auto operator=(Output&&) -> Output& = delete;

    ~Output() final;

private:
    struct ParsedBlockMatches {
        ParsedTXOs gen_;
        ParsedTXOs created_;
        ParsedTXOs confirmed_;
        ParsedTXOs consumed_;
        ParsedTXOs consumed_new_;
        ParsedTXOs consume_confirm_;
        Map<identifier::Generic, Outpoints> proposals_;

        ParsedBlockMatches() = default;
        ParsedBlockMatches(alloc::Default alloc) noexcept
            : gen_(alloc)
            , created_(alloc)
            , confirmed_(alloc)
            , consumed_(alloc)
            , consumed_new_(alloc)
            , consume_confirm_(alloc)
            , proposals_(alloc)
        {
        }
        ParsedBlockMatches(ParsedBlockMatches&& rhs) noexcept = default;
        ParsedBlockMatches(const ParsedBlockMatches&) = delete;
    };

    using GuardedSocket =
        libguarded::plain_guarded<network::zeromq::socket::Raw>;

    const api::Session& api_;
    const storage::lmdb::Database& lmdb_;
    const blockchain::Type chain_;
    const wallet::SubchainData& subchain_;
    wallet::Proposal& proposals_;
    const block::Position blank_;
    const block::Height maturation_target_;
    mutable OutputCache cache_dont_use_without_populating_;
    mutable GuardedSocket to_balance_oracle_;

    static auto allowed_initial_states(
        node::TxoState finalState,
        alloc::Strategy alloc) noexcept -> Vector<node::TxoState>;
    static auto associate_input(
        const std::size_t index,
        const protocol::bitcoin::base::block::Output& output,
        protocol::bitcoin::base::block::internal::Transaction&
            tx) noexcept(false) -> void;
    static auto filter_existing(
        const Log& log,
        const OutputCache& cache,
        node::TxoState consumeState,
        node::TxoState createState,
        ParsedBlockMatches& parsed,
        alloc::Strategy alloc) noexcept(false) -> void;
    [[nodiscard]] static auto get_inputs(
        const block::Transaction& transaction,
        alloc::Strategy alloc) noexcept(false) -> Outpoints;
    [[nodiscard]] static auto get_outputs(
        const block::Transaction& transaction,
        alloc::Strategy alloc) noexcept(false) -> Outpoints;
    [[nodiscard]] static auto is_confirmed(node::TxoState state) noexcept
        -> bool;
    [[nodiscard]] static auto pick_one(
        const OutputCache& cache,
        const identifier::Nym& spender,
        const Outpoints& in,
        Reserved& reserved,
        std::optional<node::TxoTag> required,
        alloc::Strategy alloc) noexcept
        -> std::pair<std::optional<block::Outpoint>, bool>;
    [[nodiscard]] static auto sort_fifo(
        const OutputCache& cache,
        std::span<const block::Outpoint> in,
        alloc::Strategy alloc) noexcept -> Vector<block::Outpoint>;
    [[nodiscard]] static auto states(node::TxoState in) noexcept -> States;
    static auto validate_inputs(
        const OutputCache& cache,
        const Outpoints& consumed) noexcept(false) -> void;
    static auto validate_outputs(
        const OutputCache& cache,
        const Outpoints& created,
        alloc::Strategy alloc) noexcept(false) -> void;

    auto add_transactions(
        const Log& log,
        const AccountID& account,
        const SubchainID& subchain,
        node::TxoState consumed,
        node::TxoState created,
        BatchedMatches&& transactions,
        TXOs& txoCreated,
        TXOs& txoConsumed,
        OutputCache& cache,
        alloc::Strategy alloc) const noexcept(false) -> bool;
    auto add_transactions_in_block(
        const Log& log,
        const AccountID& account,
        const SubchainID& subchain,
        const block::Position& block,
        node::TxoState consumeState,
        node::TxoState createState,
        BlockMatches& blockMatches,
        Set<block::Transaction>& processed,
        TXOs& txoCreated,
        TXOs& txoConsumed,
        OutputCache& cache,
        storage::lmdb::Transaction& tx,
        alloc::Strategy alloc) const noexcept(false) -> void;
    auto cache() const noexcept -> const OutputCache&;
    auto change_state(
        const block::Outpoint& id,
        const block::Position& newPosition,
        node::TxoState newState,
        OutputCache& cache,
        storage::lmdb::Transaction& tx) const noexcept(false) -> void;
    auto change_state(
        const block::Outpoint& id,
        const block::Position& newPosition,
        node::TxoState newState,
        protocol::bitcoin::base::block::Output& output,
        OutputCache& cache,
        storage::lmdb::Transaction& tx) const noexcept(false) -> void;
    auto check_proposals(
        const Log& log,
        const block::Outpoint& outpoint,
        std::span<const identifier::Generic> proposals,
        node::TxoState state,
        ParsedBlockMatches& matches,
        OutputCache& cache,
        storage::lmdb::Transaction& tx) const noexcept(false) -> void;
    [[nodiscard]] auto choose(
        const Log& log,
        const identifier::Nym& spender,
        const identifier::Generic& id,
        const block::Outpoint& outpoint,
        OutputCache& cache,
        storage::lmdb::Transaction& tx) const noexcept(false)
        -> std::optional<UTXO>;
    auto confirm(
        const Log& log,
        const block::Position& block,
        const block::Outpoint& outpoint,
        std::span<const identifier::Generic> proposals,
        node::TxoState state,
        ParsedBlockMatches& matches,
        OutputCache& cache,
        storage::lmdb::Transaction& tx,
        alloc::Strategy alloc) const noexcept(false) -> void;
    auto confirm_proposal(
        const Log& log,
        const identifier::Generic& proposal,
        const block::Outpoint& output,
        ParsedBlockMatches& matches,
        OutputCache& cache,
        storage::lmdb::Transaction& tx) const noexcept(false) -> void;
    auto create_generation(
        const AccountID& account,
        const SubchainID& subchain,
        const block::Position& block,
        const block::Outpoint& outpoint,
        protocol::bitcoin::base::block::Output& output,
        std::span<const identifier::Generic> proposals,
        node::TxoState state,
        OutputCache& cache,
        storage::lmdb::Transaction& tx,
        alloc::Strategy alloc) const noexcept(false) -> void;
    auto create_new(
        const Log& log,
        const AccountID& account,
        const SubchainID& subchain,
        const block::Position& block,
        const block::Outpoint& outpoint,
        protocol::bitcoin::base::block::Output& output,
        std::span<const identifier::Generic> proposals,
        node::TxoState state,
        ParsedBlockMatches& matches,
        OutputCache& cache,
        storage::lmdb::Transaction& tx,
        alloc::Strategy alloc) const noexcept(false) -> void;
    auto create_state(
        const AccountID& accountID,
        const SubchainID& subchainID,
        const block::Outpoint& id,
        const block::Position& position,
        protocol::bitcoin::base::block::Output&& output,
        bool isGeneration,
        node::TxoState state,
        OutputCache& cache,
        storage::lmdb::Transaction& tx) const noexcept(false) -> void;
    [[nodiscard]] auto effective_position(
        const node::TxoState state,
        const block::Position& oldPos,
        const block::Position& newPos) const noexcept -> const block::Position&;
    [[nodiscard]] auto get_balance(const OutputCache& cach) const noexcept
        -> Balance;
    [[nodiscard]] auto get_balance(
        const OutputCache& cache,
        const identifier::Nym& owner) const noexcept -> Balance;
    [[nodiscard]] auto get_balance(
        const OutputCache& cache,
        const identifier::Nym& owner,
        const AccountID& account,
        const crypto::Key* key) const noexcept -> Balance;
    [[nodiscard]] auto get_balances(const OutputCache& cache) const noexcept
        -> NymBalances;
    [[nodiscard]] auto get_outputs(
        const OutputCache& cache,
        const States states,
        const identifier::Nym* owner,
        const AccountID* account,
        const SubaccountID* subchain,
        const crypto::Key* key,
        alloc::Default alloc) const noexcept -> Vector<UTXO>;
    [[nodiscard]] auto get_unspent_outputs(
        const OutputCache& cache,
        const SubaccountID& id,
        alloc::Default alloc) const noexcept -> Vector<UTXO>;
    [[nodiscard]] auto has_account(
        const OutputCache& cache,
        const AccountID& id,
        const block::Outpoint& outpoint) const noexcept -> bool;
    [[nodiscard]] auto has_key(
        const OutputCache& cache,
        const crypto::Key& key,
        const block::Outpoint& outpoint) const noexcept -> bool;
    [[nodiscard]] auto has_nym(
        const OutputCache& cache,
        const identifier::Nym& id,
        const block::Outpoint& outpoint) const noexcept -> bool;
    [[nodiscard]] auto has_subchain(
        const OutputCache& cache,
        const SubaccountID& id,
        const block::Outpoint& outpoint) const noexcept -> bool;
    auto index_keys(
        const block::Outpoint& outpoint,
        const Set<crypto::Key>& keys,
        OutputCache& cache,
        storage::lmdb::Transaction& tx,
        alloc::Strategy alloc) const noexcept(false) -> void;
    // NOTE: a mature output is available to be spent in the next block
    [[nodiscard]] auto is_mature(
        const OutputCache& cache,
        const block::Height minedAt) const noexcept -> bool;
    [[nodiscard]] auto is_mature(
        const block::Height minedAt,
        const block::Position& bestChain) const noexcept -> bool;
    [[nodiscard]] auto is_mature(
        const block::Height minedAt,
        const block::Height bestChain) const noexcept -> bool;
    [[nodiscard]] auto match(
        const OutputCache& cache,
        const States states,
        const identifier::Nym* owner,
        const AccountID* account,
        const SubaccountID* subchain,
        const crypto::Key* key) const noexcept -> Matches;
    [[nodiscard]] auto parse(
        const Log& log,
        const OutputCache& cache,
        const block::Position& block,
        BlockMatches& matches,
        Set<block::Transaction>& processed,
        TXOs& txoCreated,
        TXOs& txoConsumed,
        alloc::Strategy alloc) const noexcept(false) -> ParsedBlockMatches;
    auto publish_balance(const OutputCache& cache) const noexcept -> void;
    auto release_unused_inputs(
        const Log& log,
        const identifier::Generic& proposal,
        const Outpoints& consumed,
        OutputCache& cache,
        storage::lmdb::Transaction& tx,
        alloc::Strategy alloc) const noexcept(false) -> void;
    auto spend(
        const Log& log,
        const block::Position& block,
        const block::Outpoint& outpoint,
        std::span<const identifier::Generic> proposals,
        node::TxoState state,
        ParsedBlockMatches& matches,
        OutputCache& cache,
        storage::lmdb::Transaction& tx,
        alloc::Strategy alloc) const noexcept(false) -> void;
    auto spend_new(
        const Log& log,
        const AccountID& account,
        const SubchainID& subchain,
        const block::Position& block,
        const block::Outpoint& outpoint,
        protocol::bitcoin::base::block::Output& output,
        std::span<const identifier::Generic> proposals,
        node::TxoState state,
        ParsedBlockMatches& matches,
        OutputCache& cache,
        storage::lmdb::Transaction& tx,
        alloc::Strategy alloc) const noexcept(false) -> void;
    auto transaction_success(storage::lmdb::Transaction& tx) const
        noexcept(false) -> bool;
    [[nodiscard]] auto translate(Vector<UTXO>&& outputs) const noexcept
        -> UnallocatedVector<block::TransactionHash>;

    auto cache() noexcept -> OutputCache&
    {
        return const_cast<OutputCache&>(std::as_const(*this).cache());
    }
    auto fail_transaction() noexcept -> bool;
};
}  // namespace opentxs::blockchain::database::wallet
