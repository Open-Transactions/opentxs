// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/database/wallet/Output.hpp"  // IWYU pragma: associated

#include <BlockchainTransactionOutput.pb.h>  // IWYU pragma: keep
#include <boost/container/vector.hpp>
#include <algorithm>
#include <cstdint>
#include <functional>
#include <iterator>
#include <memory>
#include <numeric>
#include <optional>
#include <ranges>
#include <span>
#include <stdexcept>
#include <tuple>
#include <utility>

#include "blockchain/database/wallet/Modification.hpp"
#include "blockchain/database/wallet/OutputCache.hpp"
#include "blockchain/database/wallet/ParsedBlockMatches.hpp"
#include "blockchain/database/wallet/Position.hpp"
#include "blockchain/database/wallet/Proposal.hpp"
#include "blockchain/database/wallet/Subchain.hpp"  // IWYU pragma: keep
#include "internal/api/crypto/Blockchain.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "internal/blockchain/node/SpendPolicy.hpp"
#include "internal/blockchain/params/ChainData.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Output.hpp"
#include "internal/network/zeromq/Context.hpp"
#include "internal/network/zeromq/socket/Raw.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/storage/lmdb/Database.hpp"
#include "internal/util/storage/lmdb/Transaction.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/TxoState.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/node/TxoTag.hpp"    // IWYU pragma: keep
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Input.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/protocol/bitcoin/base/block/Output.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Transaction.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Account.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/message/Message.hpp"
#include "opentxs/network/zeromq/socket/SocketType.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/PasswordPrompt.hpp"  // IWYU pragma: keep
#include "util/Work.hpp"

namespace opentxs::blockchain::database::wallet
{
using namespace std::literals;
using enum node::TxoState;

Output::Output(
    const api::Session& api,
    const storage::lmdb::Database& lmdb,
    const blockchain::Type chain,
    const wallet::SubchainData& subchains,
    wallet::Proposal& proposals,
    alloc::Default alloc) noexcept
    : HasUpstreamAllocator(alloc.resource())
    , AllocatesChildren(parent_resource())
    , api_(api)
    , lmdb_(lmdb)
    , chain_(chain)
    , subchain_(subchains)
    , proposals_(proposals)
    , blank_(-1, block::Hash{})
    , maturation_target_(params::get(chain_).MaturationInterval())
    , cache_dont_use_without_populating_(api_, lmdb_, chain_, blank_)
    , to_balance_oracle_([&] {
        using enum network::zeromq::socket::Type;
        auto out = api_.Network().ZeroMQ().Internal().RawSocket(Push);
        const auto rc = out.Connect(api_.Crypto()
                                        .Blockchain()
                                        .Internal()
                                        .BalanceOracleEndpoint()
                                        .data());

        assert_true(rc);

        return out;
    }())
{
}

auto Output::AddConfirmedTransactions(
    const Log& log,
    const SubaccountID& account,
    const SubchainID& subchain,
    BatchedMatches&& transactions,
    TXOs& txoCreated,
    ConsumedTXOs& txoConsumed,
    alloc::Strategy alloc) noexcept -> bool
{
    auto& cache = this->cache();

    try {

        return add_transactions(
            log,
            account,
            subchain,
            false,
            std::move(transactions),
            txoCreated,
            txoConsumed,
            cache,
            alloc);
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return fail_transaction();
    }
}

auto Output::AddMempoolTransaction(
    const Log& log,
    const AccountID& account,
    const SubchainID& subchain,
    MatchedTransaction&& match,
    TXOs& txoCreated,
    alloc::Strategy alloc) noexcept -> bool
{
    static const auto block = block::Position{};
    auto txoConsumed = ConsumedTXOs{alloc.work_};
    auto txid = std::get<2>(match).ID();
    auto& cache = this->cache();

    try {

        return add_transactions(
            log,
            account,
            subchain,
            true,
            [&] {
                auto out = BatchedMatches{alloc.work_};
                auto& matches = out[block];
                matches.try_emplace(std::move(txid), std::move(match));

                return out;
            }(),
            txoCreated,
            txoConsumed,
            cache,
            alloc);
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return fail_transaction();
    }
}

auto Output::add_transactions(
    const Log& log,
    const AccountID& account,
    const SubchainID& subchain,
    bool mempool,
    BatchedMatches&& transactions,
    TXOs& txoCreated,
    ConsumedTXOs& txoConsumed,
    OutputCache& cache,
    alloc::Strategy alloc) const noexcept(false) -> bool
{
    auto processed = Set<block::Transaction>{transactions.get_allocator()};
    auto tx = lmdb_.TransactionRW();

    for (auto& [block, blockMatches] : transactions) {
        log()("processing block ")(block)(" with ")(blockMatches.size())(
            " transactions ")
            .Flush();
        try {
            add_transactions_in_block(
                log,
                account,
                subchain,
                block,
                mempool,
                blockMatches,
                processed,
                txoCreated,
                txoConsumed,
                cache,
                tx,
                alloc);
        } catch (const std::exception& e) {
            throw std::runtime_error{"failed to process transactions in block "s
                                         .append(block.print())
                                         .append(": ")
                                         .append(e.what())};
        }
    }

    transaction_success(tx);
    const auto reason = api_.Factory().PasswordPrompt(
        "Save a received blockchain transaction(s)");
    const auto added =
        api_.Crypto().Blockchain().Internal().ProcessTransactions(
            chain_, std::move(processed), reason);

    if (false == added) {
        throw std::runtime_error{
            "Error adding transaction to activity database"};
    }

    // NOTE uncomment this for detailed debugging: cache.Print();
    publish_balance(cache);
    transactions.clear();

    return true;
}

auto Output::add_transactions_in_block(
    const Log& log,
    const AccountID& account,
    const SubchainID& subchain,
    const block::Position& block,
    bool mempool,
    BlockMatches& blockMatches,
    Set<block::Transaction>& processed,
    TXOs& txoCreated,
    ConsumedTXOs& txoConsumed,
    OutputCache& cache,
    storage::lmdb::Transaction& tx,
    alloc::Strategy alloc) const noexcept(false) -> void
{
    auto p = ParsedBlockMatches::Parse(
        api_,
        log,
        cache,
        block,
        blockMatches,
        processed,
        txoCreated,
        txoConsumed,
        mempool,
        maturation_target_,
        alloc);
    const auto process = [&, this](auto& data) {
        this->process(
            log,
            account,
            subchain,
            block,
            data.first,
            data.second,
            mempool,
            cache,
            tx,
            alloc);
    };
    std::ranges::for_each(p.transaction_, process);

    if (false == mempool) {
        const auto finished =
            cache.CheckProposals(p.proposals_, alloc.WorkOnly());
        const auto close = [&, this](const auto& id) {
            const auto rc = proposals_.FinishProposal(tx, id);

            if (false == rc) {
                throw std::runtime_error{"failed to finish proposals "s.append(
                    id.asBase58(api_.Crypto()))};
            }
        };
        std::ranges::for_each(finished, close);
    }
}

auto Output::AdvanceTo(
    const Log& log,
    const block::Position& pos,
    alloc::Strategy alloc) noexcept -> bool
{
    auto& cache = this->cache();
    auto output{true};
    auto changed{0};

    try {
        const auto current = cache.GetPosition().Decode(api_);
        const auto start = current.height_;

        if (pos == current) { return true; }
        if (pos.height_ < current.height_) { return true; }

        assert_true(pos.height_ > start);

        const auto firstMatureHeight = start - maturation_target_;
        const auto lastMatureHeight = pos.height_ - maturation_target_;

        if (0 > lastMatureHeight) {
            log()("chain height of ")(pos.height_)(
                " is too low to mature outputs ")
                .Flush();

            return true;
        }

        const auto first = std::max<block::Height>(0, firstMatureHeight);
        // TODO allocator
        const auto matured = cache.GetMatured(first, lastMatureHeight, {});
        log()("found ")(matured.size())(" generation outputs between block ")(
            start)(" and block ")(lastMatureHeight)
            .Flush();
        auto tx = lmdb_.TransactionRW();

        for (const auto& outpoint : matured) {
            static constexpr auto state = ConfirmedNew;
            const auto& out = cache.GetOutput(outpoint);

            if (out.Internal().State() == state) {
                log()("output ")(outpoint)(" already in state ")(print(state))
                    .Flush();
                continue;
            }

            try {
                change_state(outpoint, pos, state, cache, tx);
            } catch (const std::exception& e) {
                throw std::runtime_error{
                    "failed to mature output: "s.append(e.what())};
            }

            log()("matured output ")(
                outpoint)(" due to best chain reaching block height ")(
                pos.height_)
                .Flush();
            ++changed;
        }

        if (false == cache.UpdatePosition(pos, tx)) {
            throw std::runtime_error{"Failed to update wallet position"};
        }

        transaction_success(tx);

        if (0 < changed) { publish_balance(cache); }

        return output;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return fail_transaction();
    }
}

auto Output::cache() const noexcept -> const OutputCache&
{
    cache_dont_use_without_populating_.Populate();

    return cache_dont_use_without_populating_;
}

auto Output::CancelProposal(
    const Log& log,
    const identifier::Generic& id,
    alloc::Strategy alloc) noexcept -> bool
{
    // TODO check each output to verify the status of the proposal
    auto& cache = this->cache();

    try {
        auto tx = lmdb_.TransactionRW();
        cache.FinishProposal(log, id, tx);
        const auto rc = proposals_.CancelProposal(tx, id);

        if (false == rc) {
            throw std::runtime_error{"failed to cancel proposals "s.append(
                id.asBase58(api_.Crypto()))};
        }

        return transaction_success(tx);
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return fail_transaction();
    }
}

auto Output::change_state(
    const block::Outpoint& id,
    const block::Position& newPosition,
    node::TxoState newState,
    OutputCache& cache,
    storage::lmdb::Transaction& tx) const noexcept(false) -> void
{
    change_state(id, newPosition, newState, cache.GetOutput(id), cache, tx);
}

auto Output::change_state(
    const block::Outpoint& id,
    const block::Position& newPosition,
    node::TxoState newState,
    protocol::bitcoin::base::block::Output& output,
    OutputCache& cache,
    storage::lmdb::Transaction& tx) const noexcept(false) -> void
{
    auto& internal = output.Internal();
    const auto oldState = internal.State();
    const auto& oldPosition = internal.MinedPosition();
    const auto effective =
        effective_position(newState, oldPosition, newPosition);
    const auto updateState = (newState != oldState);
    const auto updatePosition = (effective != oldPosition);

    if (updateState) {
        const auto rc = cache.ChangeState(oldState, newState, id, tx);

        if (false == rc) {
            throw std::runtime_error{"Failed to update state index"};
        }

        internal.SetState(newState);
    }

    if (updatePosition) {
        const auto rc = cache.ChangePosition(oldPosition, effective, id, tx);

        if (false == rc) {
            throw std::runtime_error{"Failed to update position index"};
        }

        internal.SetMinedPosition(effective);
    }

    auto updated = cache.UpdateOutput(id, output, tx);

    if (false == updated) {
        throw std::runtime_error{"Failed to update output"};
    }
}

auto Output::check_proposals(
    const Log& log,
    const block::Outpoint& outpoint,
    const Set<identifier::Generic>& proposals,
    bool mempool,
    OutputCache& cache,
    storage::lmdb::Transaction& tx) const noexcept(false) -> void
{
    const auto check = [&, this](const auto& proposal) {
        if (mempool) {
            log()("output ")(
                outpoint)(" will not be dissociated from proposal ")(
                proposal, api_.Crypto())(" until the transaction is confirmed")
                .Flush();
        } else {
            confirm_proposal(log, proposal, outpoint, cache, tx);
        }
    };
    std::ranges::for_each(proposals, check);
}

auto Output::choose(
    const Log& log,
    const identifier::Nym& spender,
    const identifier::Generic& id,
    const block::Outpoint& outpoint,
    OutputCache& cache,
    storage::lmdb::Transaction& tx) const noexcept(false) -> std::optional<UTXO>
{
    auto& existing = cache.GetOutput(outpoint);
    cache.ConsumeOutput(log, id, outpoint, tx);

    return std::make_optional<UTXO>(std::make_pair(outpoint, existing));
}

auto Output::confirm_proposal(
    const Log& log,
    const identifier::Generic& proposal,
    const block::Outpoint& output,
    OutputCache& cache,
    storage::lmdb::Transaction& tx) const noexcept(false) -> void
{
    switch (cache.GetAssociation(proposal, output)) {
        using enum ProposalAssociation;
        case created: {
            cache.ProposalConfirmCreated(log, proposal, output, tx);
        } break;
        case consumed: {
            cache.ProposalConfirmConsumed(log, proposal, output, tx);
        } break;
        case none:
        default: {
            throw std::runtime_error{
                "output "s.append(output.str())
                    .append(" is not associated with proposal ")
                    .append(proposal.asBase58(api_.Crypto()))};
        }
    }
}

auto Output::create_state(
    const AccountID& accountID,
    const SubchainID& subchainID,
    const block::Outpoint& id,
    const block::Position& position,
    protocol::bitcoin::base::block::Output&& output,
    bool isGeneration,
    node::TxoState state,
    OutputCache& cache,
    storage::lmdb::Transaction& tx) const noexcept(false) -> void
{
    if (cache.Exists(id)) {
        throw std::runtime_error{
            "output "s.append(id.str()).append(" already exists")};
    }

    const auto& pos = effective_position(state, blank_, position);

    {
        auto& internal = output.Internal();

        if (false == internal.HasKeys()) {
            throw std::runtime_error{
                "output "s.append(id.str()).append(" missing keys")};
        }

        internal.SetState(state);
        internal.SetMinedPosition(pos);

        if (isGeneration) {
            internal.AddTag(node::TxoTag::Generation);
        } else {
            internal.AddTag(node::TxoTag::Normal);
        }
    }

    const auto rc = cache.AddOutput(
        id, state, pos, accountID, subchainID, tx, std::move(output));

    if (false == rc) {
        throw std::runtime_error{"failed to write output "s.append(id.str())};
    }

    if (isGeneration) { cache.AddGenerationOutput(pos.height_, id, tx); }
}

auto Output::effective_position(
    const node::TxoState state,
    const block::Position& oldPos,
    const block::Position& newPos) const noexcept -> const block::Position&
{
    switch (state) {
        case UnconfirmedNew:
        case UnconfirmedSpend: {

            return oldPos;
        }
        default: {

            return newPos;
        }
    }
}

auto Output::fail_transaction() noexcept -> bool
{
    // NOTE don't populate primarily because it already is populated but also
    // because we're about to clear it anyway.
    cache_dont_use_without_populating_.Clear();

    return false;
}

auto Output::FinalizeProposal(
    const Log& log,
    const identifier::Generic& proposalID,
    const proto::BlockchainTransactionProposal& proposal,
    const block::Transaction& transaction,
    alloc::Strategy alloc) noexcept -> bool
{
    assert_false(transaction.asBitcoin().IsGeneration());

    const auto work = alloc.WorkOnly();
    auto& cache = this->cache();

    try {
        if (false == proposals_.AddProposal(proposalID, proposal)) {
            throw std::runtime_error{"failed to save finalized proposal"};
        }

        auto tx = lmdb_.TransactionRW();
        const auto consumed = get_inputs(transaction, work);
        // NOTE perhaps the transaction builder requested more outputs than it
        // ended up using in the completed transaction.
        release_unused_inputs(log, proposalID, consumed, cache, tx, work);
        // NOTE sanity check to make sure the inputs actually exist in a
        // spendable state
        validate_inputs(cache, consumed);
        const auto created = get_outputs(transaction, work);
        validate_outputs(cache, created, alloc);
        const auto set_state = [&](const auto& id, auto state) {
            change_state(id, blank_, state, cache, tx);
        };
        const auto set_input_state = [&](const auto& id) {
            set_state(id, UnconfirmedSpend);
        };
        const auto set_output_state = [&](const auto& id) {
            cache.CreateOutput(log, proposalID, id, tx);

            if (cache.Exists(id)) {
                set_state(id, UnconfirmedNew);
            } else {
                const auto outputs = transaction.asBitcoin().Outputs();
                const auto& output = outputs[id.Index()];
                const auto keys = output.Keys(alloc.work_);

                if (1_uz != keys.size()) {
                    throw std::runtime_error{
                        "output "s.append(id.str())
                            .append(" has wrong number of keys: ")
                            .append(std::to_string(keys.size()))};
                }

                const auto& key = *keys.cbegin();
                const auto [accountID, subchainID] = [&] {
                    const auto& [account, subchain, idx] = key;

                    return std::make_pair(
                        account,
                        subchain_.GetSubchainID(account, subchain, tx));
                }();
                auto copy{output};
                create_state(
                    accountID,
                    subchainID,
                    id,
                    blank_,
                    std::move(copy),
                    false,
                    UnconfirmedNew,
                    cache,
                    tx);
                index_keys(id, keys, cache, tx, alloc);
            }
        };
        std::ranges::for_each(consumed, set_input_state);
        std::ranges::for_each(created, set_output_state);
        const auto reason = api_.Factory().PasswordPrompt(
            "Save an outgoing blockchain transaction");
        auto transactions = Set<block::Transaction>{transaction};

        if (false == api_.Crypto().Blockchain().Internal().ProcessTransactions(
                         chain_, std::move(transactions), reason)) {
            throw std::runtime_error{"Error adding transaction to database"};
        }

        transaction_success(tx);
        // NOTE uncomment this for detailed debugging: cache.Print(lock);
        publish_balance(cache);

        return true;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return fail_transaction();
    }
}

auto Output::FinalizeReorg(
    const Log& log,
    const block::Position& pos,
    storage::lmdb::Transaction& tx,
    alloc::Strategy alloc) noexcept -> bool
{
    auto& cache = this->cache();
    auto output{true};

    try {
        if (cache.GetPosition().Decode(api_) != pos) {
            auto outputs = OrphanedGeneration{};  // TODO monotonic
            cache.DeleteGenerationAbove(pos.height_, outputs, tx);

            for (const auto& id : outputs) {
                try {
                    change_state(id, pos, OrphanedNew, cache, tx);
                } catch (const std::exception& e) {
                    throw std::runtime_error{
                        "failed to orphan generation output: "s.append(
                            e.what())};
                }
            }

            output = cache.UpdatePosition(pos, tx);

            if (false == output) {
                throw std::runtime_error{"Failed to update wallet position"};
            }
        }
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();
        output = false;

        return fail_transaction();
    }

    return output;
}

auto Output::GetBalance() const noexcept -> Balance
{
    return get_balance(cache());
}

auto Output::GetBalance(const identifier::Nym& owner) const noexcept -> Balance
{
    if (owner.empty()) { return {}; }

    return get_balance(cache(), owner);
}

auto Output::GetBalance(const identifier::Nym& owner, const SubaccountID& node)
    const noexcept -> Balance
{
    if (owner.empty() || node.empty()) { return {}; }

    return get_balance(cache(), owner, node, nullptr);
}

auto Output::GetBalance(const crypto::Key& key) const noexcept -> Balance
{
    static const auto owner = identifier::Nym{};
    static const auto node = identifier::Account{};

    return get_balance(cache(), owner, node, &key);
}

auto Output::get_balance(const OutputCache& cache) const noexcept -> Balance
{
    static const auto blank = identifier::Nym{};

    return get_balance(cache, blank);
}

auto Output::get_balance(const OutputCache& cache, const identifier::Nym& owner)
    const noexcept -> Balance
{
    static const auto blank = identifier::Account{};

    return get_balance(cache, owner, blank, nullptr);
}

auto Output::get_balance(
    const OutputCache& cache,
    const identifier::Nym& owner,
    const AccountID& account,
    const crypto::Key* key) const noexcept -> Balance
{
    auto output = Balance{};
    auto& [confirmed, unconfirmed] = output;
    const auto* pNym = owner.empty() ? nullptr : &owner;
    const auto* pAcct = account.empty() ? nullptr : &account;
    auto cb = [&](const auto previous, const auto& outpoint) -> auto {
        const auto& existing = cache.GetOutput(outpoint);

        return previous + existing.Value();
    };

    const auto unconfirmedSpendTotal = [&] {
        const auto txos =
            match(cache, {UnconfirmedSpend}, pNym, pAcct, nullptr, key);

        return std::accumulate(txos.begin(), txos.end(), Amount{0}, cb);
    }();

    {
        const auto txos =
            match(cache, {ConfirmedNew}, pNym, pAcct, nullptr, key);
        confirmed = unconfirmedSpendTotal +
                    std::accumulate(txos.begin(), txos.end(), Amount{0}, cb);
    }

    {
        const auto txos =
            match(cache, {UnconfirmedNew}, pNym, pAcct, nullptr, key);
        unconfirmed = std::accumulate(txos.begin(), txos.end(), confirmed, cb) -
                      unconfirmedSpendTotal;
    }

    return output;
}

auto Output::get_balances(const OutputCache& cache) const noexcept
    -> NymBalances
{
    auto output = NymBalances{};

    for (const auto& nym : cache.GetNyms()) {
        output[nym] = get_balance(cache, nym);
    }

    return output;
}

auto Output::get_inputs(
    const block::Transaction& transaction,
    alloc::Strategy alloc) noexcept(false) -> Outpoints
{
    const auto& bitcoin = transaction.asBitcoin();

    if (false == bitcoin.IsValid()) {
        throw std::runtime_error{"unsupported transaction type"};
    }

    constexpr auto get_spends = [](const auto& input) {
        return input.PreviousOutput();
    };
    const auto inputs = bitcoin.Inputs();
    auto out = Outpoints{alloc.result_};
    std::ranges::transform(inputs, std::inserter(out, out.end()), get_spends);

    return out;
}

auto Output::GetOutputs(node::TxoState type, alloc::Default alloc)
    const noexcept -> Vector<UTXO>
{
    if (Error == type) { return Vector<UTXO>{alloc}; }

    return get_outputs(
        cache(), states(type), nullptr, nullptr, nullptr, nullptr, alloc);
}

auto Output::GetOutputs(
    const identifier::Nym& owner,
    node::TxoState type,
    alloc::Default alloc) const noexcept -> Vector<UTXO>
{
    if (Error == type) { return Vector<UTXO>{alloc}; }
    if (owner.empty()) { return Vector<UTXO>{alloc}; }

    return get_outputs(
        cache(), states(type), &owner, nullptr, nullptr, nullptr, alloc);
}

auto Output::GetOutputs(
    const identifier::Nym& owner,
    const identifier::Account& node,
    node::TxoState type,
    alloc::Default alloc) const noexcept -> Vector<UTXO>
{
    if (Error == type) { return Vector<UTXO>{alloc}; }
    if (owner.empty() || node.empty()) { return Vector<UTXO>{alloc}; }

    return get_outputs(
        cache(), states(type), &owner, &node, nullptr, nullptr, alloc);
}

auto Output::GetOutputs(
    const crypto::Key& key,
    node::TxoState type,
    alloc::Default alloc) const noexcept -> Vector<UTXO>
{
    if (Error == type) { return Vector<UTXO>{alloc}; }

    return get_outputs(
        cache(), states(type), nullptr, nullptr, nullptr, &key, alloc);
}

auto Output::get_outputs(
    const block::Transaction& transaction,
    alloc::Strategy alloc) noexcept(false) -> Outpoints
{
    const auto& bitcoin = transaction.asBitcoin();

    if (false == bitcoin.IsValid()) {
        throw std::runtime_error{"unsupported transaction type"};
    }

    const auto& txid = transaction.ID();
    const auto outputs = bitcoin.Outputs();
    auto out = Outpoints{alloc.result_};

    for (auto n = 0_uz; n < outputs.size(); ++n) {
        const auto& output = outputs[n];

        if (output.Keys(alloc.work_).empty()) {

            continue;
        } else {

            out.emplace(txid, static_cast<std::uint32_t>(n));
        }
    }

    return out;
}

auto Output::get_outputs(
    const OutputCache& cache,
    const States states,
    const identifier::Nym* owner,
    const AccountID* account,
    const SubaccountID* subchain,
    const crypto::Key* key,
    alloc::Default alloc) const noexcept -> Vector<UTXO>
{
    const auto matches = match(cache, states, owner, account, subchain, key);
    auto output = Vector<UTXO>{alloc};

    for (const auto& outpoint : matches) {
        const auto& existing = cache.GetOutput(outpoint);
        output.emplace_back(std::make_pair(outpoint, existing));
    }

    return output;
}

auto Output::GetOutputTags(const block::Outpoint& output) const noexcept
    -> UnallocatedSet<node::TxoTag>
{
    const auto& cache = this->cache();

    if (cache.Exists(output)) {
        const auto& existing = cache.GetOutput(output);

        return existing.Internal().Tags();
    } else {

        return {};
    }
}

auto Output::get_unspent_outputs(
    const OutputCache& cache,
    const SubaccountID& id,
    alloc::Default alloc) const noexcept -> Vector<UTXO>
{
    const auto* pSub = id.empty() ? nullptr : &id;

    return get_outputs(
        cache,
        {UnconfirmedNew, ConfirmedNew, UnconfirmedSpend},
        nullptr,
        nullptr,
        pSub,
        nullptr,
        alloc);
}

auto Output::GetPosition() const noexcept -> block::Position
{
    return cache().GetPosition().Decode(api_);
}

auto Output::GetReserved(
    const identifier::Generic& proposal,
    alloc::Strategy alloc) const noexcept -> Vector<UTXO>
{
    return cache().GetReserved(proposal, alloc);
}

auto Output::GetTransactions() const noexcept
    -> UnallocatedVector<block::TransactionHash>
{
    return translate(GetOutputs(All, alloc::System()));
}

auto Output::GetTransactions(const identifier::Nym& account) const noexcept
    -> UnallocatedVector<block::TransactionHash>
{
    return translate(GetOutputs(account, All, alloc::System()));
}

auto Output::GetUnconfirmedTransactions() const noexcept
    -> UnallocatedSet<block::TransactionHash>
{
    auto out = UnallocatedSet<block::TransactionHash>{};
    const auto unconfirmed = GetOutputs(UnconfirmedNew, alloc::System());

    for (const auto& [outpoint, output] : unconfirmed) {
        out.emplace(outpoint.Txid());
    }

    return out;
}

auto Output::GetUnspentOutputs(alloc::Default alloc) const noexcept
    -> Vector<UTXO>
{
    static const auto blank = identifier::Account{};

    return GetUnspentOutputs(blank, alloc);
}

auto Output::GetUnspentOutputs(const SubaccountID& id, alloc::Default alloc)
    const noexcept -> Vector<UTXO>
{
    return get_unspent_outputs(cache(), id, alloc);
}

auto Output::GetWalletHeight() const noexcept -> block::Height
{
    return cache().GetHeight();
}

auto Output::has_account(
    const OutputCache& cache,
    const AccountID& id,
    const block::Outpoint& outpoint) const noexcept -> bool
{
    return cache.GetAccount(id).contains(outpoint);
}

auto Output::has_key(
    const OutputCache& cache,
    const crypto::Key& key,
    const block::Outpoint& outpoint) const noexcept -> bool
{
    return cache.GetKey(key).contains(outpoint);
}

auto Output::has_nym(
    const OutputCache& cache,
    const identifier::Nym& id,
    const block::Outpoint& outpoint) const noexcept -> bool
{
    return cache.GetNym(id).contains(outpoint);
}

auto Output::has_subchain(
    const OutputCache& cache,
    const SubaccountID& id,
    const block::Outpoint& outpoint) const noexcept -> bool
{
    return cache.GetSubchain(id).contains(outpoint);
}

auto Output::index_keys(
    const block::Outpoint& outpoint,
    const Set<crypto::Key>& keys,
    OutputCache& cache,
    storage::lmdb::Transaction& tx,
    alloc::Strategy alloc) const noexcept(false) -> void
{
    const auto& api = api_.Crypto().Blockchain();

    if (keys.empty()) { throw std::runtime_error{"no keys to index"}; }

    if (1_uz != keys.size()) {
        throw std::runtime_error{"unsupported multisig output type"};
    }

    for (const auto& key : keys) {
        const auto& [subaccount, subchain, bip32] = key;

        if (crypto::Subchain::Outgoing == subchain) {
            // TODO make sure this output is associated with the
            // correct contact

            continue;
        }

        const auto& owner = api.Owner(key);

        if (owner.empty()) {
            const auto error = CString{"no owner found for key "}.append(
                print(key, api_.Crypto()));

            throw std::runtime_error{error.c_str()};
        }

        try {
            cache.AddToNym(owner, outpoint, tx);
        } catch (const std::exception& e) {
            throw std::runtime_error{
                "unable to associate outpoint to nym: "s.append(e.what())};
        }
    }
}

auto Output::is_confirmed(node::TxoState state) noexcept -> bool
{
    return (ConfirmedNew == state) || (ConfirmedSpend == state);
}

auto Output::match(
    const OutputCache& cache,
    const States states,
    const identifier::Nym* owner,
    const AccountID* account,
    const SubaccountID* subchain,
    const crypto::Key* key) const noexcept -> Matches
{
    auto output = Matches{};
    const auto allSubs = (nullptr == subchain);
    const auto allAccts = (nullptr == account);
    const auto allNyms = (nullptr == owner);
    const auto allKeys = (nullptr == key);

    for (const auto state : states) {
        for (const auto& outpoint : cache.GetState(state)) {
            // NOTE if a more specific conditions is requested then
            // it's not necessary to test any more general
            // conditions. A subchain match implies an account match
            // implies a nym match
            const auto goodKey = allKeys || has_key(cache, *key, outpoint);
            const auto goodSub = (!allKeys) || allSubs ||
                                 has_subchain(cache, *subchain, outpoint);
            const auto goodAcct = (!allKeys) || (!allSubs) || allAccts ||
                                  has_account(cache, *account, outpoint);
            const auto goodNym = (!allKeys) || (!allSubs) || (!allAccts) ||
                                 allNyms || has_nym(cache, *owner, outpoint);

            if (goodNym && goodAcct && goodSub && goodKey) {
                output.emplace_back(outpoint);
            }
        }
    }

    return output;
}

auto Output::pick_one(
    const OutputCache& cache,
    const identifier::Nym& spender,
    const Outpoints& in,
    Reserved& reserved,
    std::optional<node::TxoTag> required,
    alloc::Strategy alloc) noexcept
    -> std::pair<std::optional<block::Outpoint>, bool>
{
    if (in.empty()) { return {std::nullopt, false}; }

    const auto owned = [&] {
        const auto& byNym = cache.GetNym(spender);
        auto out = Vector<block::Outpoint>{alloc.work_};
        out.reserve(std::min(in.size(), byNym.size()));
        out.clear();
        std::ranges::set_intersection(in, byNym, std::back_inserter(out));

        return out;
    }();

    if (owned.empty()) { return {std::nullopt, false}; }

    const auto available = [&] {
        auto out = Vector<block::Outpoint>{alloc.work_};
        out.reserve(owned.size());
        out.clear();
        std::ranges::set_difference(owned, reserved, std::back_inserter(out));

        return out;
    }();

    if (available.empty()) { return {std::nullopt, false}; }

    const auto sorted = sort_fifo(cache, available, alloc.WorkOnly());
    const auto acceptable = [&](const auto& id) {
        if (required.has_value()) {

            return cache.GetOutput(id).Internal().Tags().contains(*required);
        } else {

            return true;
        }
    };
    const auto selected = std::ranges::find_if(sorted, acceptable);

    if (sorted.end() == selected) {

        return std::make_pair(std::nullopt, true);
    } else {

        return std::make_pair(*selected, false);
    }
}

auto Output::process(
    const Log& log,
    const AccountID& account,
    const SubchainID& subchain,
    const block::Position& block,
    const block::Outpoint& id,
    Modification& data,
    bool mempool,
    OutputCache& cache,
    storage::lmdb::Transaction& tx,
    alloc::Strategy alloc) const noexcept(false) -> void
{
    if (data.initial_state_.has_value()) {
        process_modify(log, block, id, data, mempool, cache, tx, alloc);
    } else {
        process_create(
            log, account, subchain, block, id, data, mempool, cache, tx, alloc);
    }
}

auto Output::process_create(
    const Log& log,
    const AccountID& account,
    const SubchainID& subchain,
    const block::Position& block,
    const block::Outpoint& id,
    Modification& data,
    bool mempool,
    OutputCache& cache,
    storage::lmdb::Transaction& tx,
    alloc::Strategy alloc) const noexcept(false) -> void
{
    try {
        if (false == data.output_.has_value()) {
            throw std::runtime_error{"missing txout"};
        }

        if (false == data.CreatesGeneration()) {
            check_proposals(log, id, data.proposals_, mempool, cache, tx);
        } else if (false == data.proposals_.empty()) {
            throw std::runtime_error{"generation output claims an association "
                                     "with one or more proposals"};
        }

        const auto keys = data.output_->Keys(alloc.work_);
        create_state(
            account,
            subchain,
            id,
            block,
            std::move(*data.output_),
            true,
            data.final_state_,
            cache,
            tx);
        index_keys(id, keys, cache, tx, alloc);
    } catch (const std::exception& e) {
        throw std::runtime_error{"failed to create output "s.append(id.str())
                                     .append(" in state ")
                                     .append(print(data.final_state_))
                                     .append(": ")
                                     .append(e.what())};
    }
}

auto Output::process_modify(
    const Log& log,
    const block::Position& block,
    const block::Outpoint& id,
    const Modification& data,
    bool mempool,
    OutputCache& cache,
    storage::lmdb::Transaction& tx,
    alloc::Strategy alloc) const noexcept(false) -> void
{
    try {
        check_proposals(log, id, data.proposals_, mempool, cache, tx);
        change_state(id, block, data.final_state_, cache, tx);
    } catch (const std::exception& e) {
        throw std::runtime_error{
            "failed to transition output "s.append(id.str())
                .append(" to state ")
                .append(print(data.final_state_))
                .append(": ")
                .append(e.what())};
    }
}

auto Output::PublishBalance() const noexcept -> void
{
    publish_balance(cache());
}

auto Output::publish_balance(const OutputCache& cache) const noexcept -> void
{
    const auto total = get_balance(cache);
    const auto byNym = get_balances(cache);
    auto handle = to_balance_oracle_.lock();
    handle->SendDeferred([&] {
        auto out = MakeWork(OT_ZMQ_BALANCE_ORACLE_SUBMIT);
        out.AddFrame(chain_);
        out.AddFrame(total.first);
        out.AddFrame(total.second);

        return out;
    }());

    for (const auto& [nym, balance] : byNym) {
        handle->SendDeferred([&]() {
            auto out = MakeWork(OT_ZMQ_BALANCE_ORACLE_SUBMIT);
            out.AddFrame(chain_);
            out.AddFrame(balance.first);
            out.AddFrame(balance.second);
            out.AddFrame(nym);

            return out;
        }());
    }
}

auto Output::release_unused_inputs(
    const Log& log,
    const identifier::Generic& proposal,
    const Outpoints& consumed,
    OutputCache& cache,
    storage::lmdb::Transaction& tx,
    alloc::Strategy alloc) const noexcept(false) -> void
{
    const auto& crypto = api_.Crypto();
    const auto reserved = cache.GetConsumed(proposal);
    const auto sanity_check = [&](const auto& id) {
        if (false == reserved.contains(id)) {
            throw std::runtime_error{
                "proposal "s.append(proposal.asBase58(crypto))
                    .append(" did not reserve output ")
                    .append(id.str())};
        }
    };
    std::ranges::for_each(consumed, sanity_check);
    const auto unused = [&] {
        auto out = Vector<block::Outpoint>{alloc.work_};
        out.reserve(consumed.size());
        out.clear();
        std::ranges::set_difference(
            reserved, consumed, std::back_inserter(out));

        return out;
    }();
    const auto release = [&](const auto& id) {
        cache.Release(log, id, proposal, tx);
    };
    std::ranges::for_each(unused, release);
}

auto Output::ReserveUTXO(
    const Log& log,
    const identifier::Nym& spender,
    const identifier::Generic& proposal,
    const node::internal::SpendPolicy& policy,
    alloc::Strategy alloc) noexcept -> std::pair<std::optional<UTXO>, bool>
{
    const auto& crypto = api_.Crypto();
    log()("reserving outputs for proposal ")(proposal, crypto).Flush();
    static const auto blank =
        std::make_pair(std::optional<UTXO>{std::nullopt}, false);
    auto output = blank;
    auto& cache = this->cache();

    try {
        // TODO implement smarter selection algorithm using policy
        auto tx = lmdb_.TransactionRW();
        auto reserved = cache.GetReserved(alloc.WorkOnly());
        const auto select =
            [&](const auto& outputs,
                const auto required) -> std::pair<std::optional<UTXO>, bool> {
            const auto outpoint =
                pick_one(cache, spender, outputs, reserved, required, alloc);

            if (outpoint.first.has_value()) {

                return std::make_pair(
                    choose(log, spender, proposal, *outpoint.first, cache, tx),
                    outpoint.second);
            } else {

                return blank;
            }
        };
        const auto& confirmed = cache.GetState(ConfirmedNew);
        log()(confirmed.size())(" confirmed outputs available").Flush();
        output = select(confirmed, std::nullopt);
        const auto spendUnconfirmed =
            policy.unconfirmed_incoming_ || policy.unconfirmed_change_;

        if ((!output.first.has_value()) && spendUnconfirmed) {
            const auto changeOnly = !policy.unconfirmed_incoming_;
            log()("allowing selection of unconfirmed outputs");

            if (changeOnly) { log(", limited to change outputs only"); }

            log.Flush();
            const auto& unconfirmed = cache.GetState(UnconfirmedNew);
            log()(unconfirmed.size())(" unconfirmed outputs available").Flush();
            constexpr auto changeTag = std::make_optional(node::TxoTag::Change);
            const auto tag = changeOnly ? changeTag : std::nullopt;
            output = select(unconfirmed, tag);
        }

        if (output.first.has_value()) {
            log()("reserved output ")(output.first->first)(" for proposal ")(
                proposal, crypto)
                .Flush();
        } else {
            throw std::runtime_error{"No spendable outputs for specified nym"};
        }

        transaction_success(tx);

        return output;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();
        fail_transaction();

        return blank;
    }
}

auto Output::ReserveUTXO(
    const Log& log,
    const identifier::Nym& spender,
    const identifier::Generic& proposal,
    const block::Outpoint& id,
    alloc::Strategy alloc) noexcept -> std::optional<UTXO>
{
    try {
        auto tx = lmdb_.TransactionRW();
        auto utxo = choose(log, spender, proposal, id, cache(), tx);

        if (false == utxo.has_value()) {
            throw std::runtime_error{"requested utxo is not spendable"};
        }

        transaction_success(tx);

        return utxo;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();
        fail_transaction();

        return std::nullopt;
    }
}

auto Output::sort_fifo(
    const OutputCache& cache,
    std::span<const block::Outpoint> in,
    alloc::Strategy alloc) noexcept -> Vector<block::Outpoint>
{
    auto counter = 0_uz;
    auto map = Map<block::Position, Set<block::Outpoint>>{alloc.work_};
    map.clear();

    for (const auto& id : in) {
        const auto& output = cache.GetOutput(id);
        map[output.Internal().MinedPosition()].emplace(id);
        ++counter;
    }

    auto out = Vector<block::Outpoint>{alloc.result_};
    out.reserve(counter);
    out.clear();

    for (const auto& [position, set] : map) {
        for (const auto& id : set) { out.emplace_back(id); }
    }

    return out;
}

auto Output::StartReorg(
    const Log& log,
    const SubchainID& subchain,
    const block::Position& position,
    storage::lmdb::Transaction& tx,
    alloc::Strategy alloc) noexcept -> bool
{
    LogTrace()()("rolling back block ")(position).Flush();
    const auto& api = api_.Crypto().Blockchain();
    auto& cache = this->cache();

    try {
        // TODO rebroadcast transactions which have become unconfirmed
        const auto outpoints = [&] {
            const auto& in = cache.GetPosition(position);
            auto out = Vector<block::Outpoint>{};  // TODO allocator
            out.reserve(in.size());
            out.clear();
            const auto subchain_match = [&](const auto& id) {
                return cache.GetSubchain(subchain).contains(id);
            };
            std::ranges::copy_if(in, std::back_inserter(out), subchain_match);

            return out;
        }();
        LogTrace()()(outpoints.size())(" affected outpoints").Flush();

        for (const auto& id : outpoints) {
            auto& output = cache.GetOutput(id);
            const auto state = [&]() -> std::optional<node::TxoState> {
                switch (output.Internal().State()) {
                    case ConfirmedNew:
                    case OrphanedNew: {

                        return UnconfirmedNew;
                    }
                    case ConfirmedSpend:
                    case OrphanedSpend: {

                        return UnconfirmedSpend;
                    }
                    default: {

                        return std::nullopt;
                    }
                }
            }();

            if (state.has_value()) {
                change_state(id, position, state.value(), output, cache, tx);
            }

            const auto txid = block::TransactionHash{id.Txid()};

            for (const auto& key : output.Keys({})) {  // TODO allocator
                api.Unconfirm(key, txid);
            }
        }

        return true;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return fail_transaction();
    }
}

auto Output::states(node::TxoState in) noexcept -> States
{
    if (All == in) { return all_states(); }

    return States{in};
}

auto Output::transaction_success(storage::lmdb::Transaction& tx) const
    noexcept(false) -> bool
{
    if (false == tx.Finalize(true)) {
        throw std::runtime_error{"Failed to commit database transaction"};
    }

    return true;
}

auto Output::translate(Vector<UTXO>&& outputs) const noexcept
    -> UnallocatedVector<block::TransactionHash>
{
    auto out = UnallocatedVector<block::TransactionHash>{};
    auto temp = UnallocatedSet<block::TransactionHash>{};

    for (auto& [outpoint, output] : outputs) { temp.emplace(outpoint.Txid()); }

    out.reserve(temp.size());
    std::ranges::move(temp, std::back_inserter(out));

    return out;
}

auto Output::validate_inputs(
    const OutputCache& cache,
    const Outpoints& consumed) noexcept(false) -> void
{
    const auto notConfirmed = [&](const auto& id) {
        const auto& confirmed = cache.GetState(ConfirmedNew);

        return false == confirmed.contains(id);
    };
    const auto notUnconfirmed = [&](const auto& id) {
        const auto& unconfirmed = cache.GetState(UnconfirmedNew);

        return false == unconfirmed.contains(id);
    };
    const auto check = [&](const auto& id) {
        if (notConfirmed(id) && notUnconfirmed(id)) {

            throw std::runtime_error{"output "s.append(id.str()).append(
                " is not available for spending")};
        }
    };
    std::ranges::for_each(consumed, check);
}

auto Output::validate_outputs(
    const OutputCache& cache,
    const Outpoints& created,
    alloc::Strategy alloc) noexcept(false) -> void
{
    const auto check = [&](const auto& id) {
        if (false == cache.GetProposals(id, alloc.WorkOnly()).empty()) {
            throw std::runtime_error{"created output "s.append(id.str()).append(
                " is already associated with a proposal")};
        }

        if (cache.Exists(id)) {
            const auto& orphaned = cache.GetState(OrphanedNew);

            if (orphaned.contains(id)) {
                // NOTE what does it mean if we get here? It means somebody
                // finalized a proposal, then cancelled the proposal, and then
                // later created the *exact same* transaction again using a new
                // proposal.
                //
                // It's dubious whether or not this could ever actually occur,
                // but if it does then we should do the right thing, which is to
                // allow the caller of this function to update the state of the
                // existing output.
            } else {
                throw std::runtime_error{"output "s.append(id.str()).append(
                    " already exists and is not eligible for reissuance")};
            }
        }
    };
    std::ranges::for_each(created, check);
}

Output::~Output() = default;
}  // namespace opentxs::blockchain::database::wallet
