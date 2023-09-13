// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/wallet/subchain/DeterministicStateData.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <chrono>
#include <compare>
#include <functional>
#include <iterator>
#include <memory>
#include <optional>
#include <span>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include "blockchain/node/wallet/subchain/statemachine/ElementCache.hpp"
#include "internal/blockchain/block/Transaction.hpp"
#include "internal/blockchain/database/Wallet.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Index.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Transaction.hpp"  // IWYU pragma: keep
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/crypto/Deterministic.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Input.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Output.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Pattern.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/protocol/bitcoin/base/block/Script.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Transaction.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/asymmetric/key/EllipticCurve.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"
#include "util/Container.hpp"

namespace opentxs::blockchain::node::wallet
{
DeterministicStateData::DeterministicStateData(
    Reorg& reorg,
    const crypto::Deterministic& subaccount,
    std::shared_ptr<const api::Session> api,
    std::shared_ptr<const node::Manager> node,
    crypto::Subchain subchain,
    std::string_view fromParent,
    network::zeromq::BatchID batch,
    allocator_type alloc) noexcept
    : SubchainStateData(
          reorg,
          subaccount,
          std::move(api),
          std::move(node),
          std::move(subchain),
          std::move(fromParent),
          std::move(batch),
          std::move(alloc))
    , deterministic_(subaccount)
    , cache_(Clock::now(), get_allocator())
{
}

auto DeterministicStateData::CheckCache(
    const std::size_t outstanding,
    FinishedCallback cb) const noexcept -> void
{
    cache_.modify([=, this](auto& data) {
        auto& [time, blockMap] = data;
        using namespace std::literals;
        static constexpr auto maxTime = 10s;
        const auto interval = Clock::now() - time;
        const auto flush = (0u == outstanding) || (interval > maxTime);

        if (flush) {
            if (false == flush_cache(blockMap, cb)) { TriggerRescan(); }

            time = Clock::now();
        }
    });
}

auto DeterministicStateData::confirm_match_by_key(
    const Log& log,
    const block::Position& position,
    const block::Match& match,
    const block::Transaction& transaction,
    database::MatchedTransaction& matched,
    allocator_type monotonic) const noexcept -> void
{
    auto& [inputs, outputs, tx] = matched;

    if (tx.IsValid()) {
        tx.Internal().asBitcoin().MergeMetadata(
            api_.Crypto().Blockchain(),
            chain_,
            transaction.Internal().asBitcoin(),
            log_);
    } else {
        tx = transaction;
        tx.Internal().asBitcoin().SetMinedPosition(position);
    }

    set_key_data(tx, monotonic);
    const auto& [txid, elementID] = match;
    const auto& [index, subchainID] = elementID;
    const auto& [subchain, accountID] = subchainID;
    const auto& element = deterministic_.BalanceElement(subchain, index);
    const auto txout = tx.asBitcoin().Outputs();
    const auto count = txout.size();
    outputs.reserve(count);

    if (crypto::Subchain::Outgoing == subchain_) { return; }

    for (auto n = Bip32Index{0}; n < count; ++n) {
        const auto& output = txout[n];
        const auto& script = output.Script();
        using enum protocol::bitcoin::base::block::script::Pattern;

        switch (script.Type()) {
            case PayToPubkey: {
                const auto& key = element.Key();

                OT_ASSERT(key.IsValid());
                OT_ASSERT(script.Pubkey().has_value());

                if (key.PublicKey() == script.Pubkey().value()) {
                    log(OT_PRETTY_CLASS())(name_)(" element ")(
                        index)(": P2PK match found for ")(print(chain_))(
                        " transaction ")
                        .asHex(txid)(" output ")(n)(" via ")
                        .asHex(key.PublicKey())
                        .Flush();
                    outputs.emplace_back(n);
                    const auto confirmed = api_.Crypto().Blockchain().Confirm(
                        element.KeyID(), txid);

                    OT_ASSERT(confirmed);
                }
            } break;
            case PayToPubkeyHash: {
                const auto hash = element.PubkeyHash();

                OT_ASSERT(script.PubkeyHash().has_value());

                if (hash.Bytes() == script.PubkeyHash().value()) {
                    log(OT_PRETTY_CLASS())(name_)(" element ")(
                        index)(": P2PKH match found for ")(print(chain_))(
                        " transaction ")
                        .asHex(txid)(" output ")(n)(" via ")
                        .asHex(hash)
                        .Flush();
                    outputs.emplace_back(n);
                    const auto confirmed = api_.Crypto().Blockchain().Confirm(
                        element.KeyID(), txid);

                    OT_ASSERT(confirmed);
                }
            } break;
            case PayToWitnessPubkeyHash: {
                const auto hash = element.PubkeyHash();

                OT_ASSERT(script.PubkeyHash().has_value());

                if (hash.Bytes() == script.PubkeyHash().value()) {
                    log(OT_PRETTY_CLASS())(name_)(" element ")(
                        index)(": P2WPKH match found for ")(print(chain_))(
                        " transaction ")
                        .asHex(txid)(" output ")(n)(" via ")
                        .asHex(hash)
                        .Flush();
                    outputs.emplace_back(n);
                    const auto confirmed = api_.Crypto().Blockchain().Confirm(
                        element.KeyID(), txid);

                    OT_ASSERT(confirmed);
                }
            } break;
            case PayToMultisig: {
                const auto required = script.M();
                const auto of = script.N();

                OT_ASSERT(required.has_value());
                OT_ASSERT(of.has_value());

                if (1u != required.value() || (3u != of.value())) {
                    // TODO handle non-payment code multisig eventually

                    continue;
                }

                const auto& key = element.Key();

                OT_ASSERT(key.IsValid());

                if (key.PublicKey() == script.MultisigPubkey(0).value()) {
                    log(OT_PRETTY_CLASS())(name_)(" element ")(index)(": ")(
                        required.value())(" of ")(of.value())(
                        " P2MS match found for ")(print(chain_))(
                        " transaction ")
                        .asHex(txid)(" output ")(n)(" via ")
                        .asHex(key.PublicKey())
                        .Flush();
                    outputs.emplace_back(n);
                    const auto confirmed = api_.Crypto().Blockchain().Confirm(
                        element.KeyID(), txid);

                    OT_ASSERT(confirmed);
                }
            } break;
            case PayToScriptHash:
            default: {
            }
        }
    }
}

auto DeterministicStateData::confirm_match_by_outpoint(
    const Log& log,
    const block::Position& position,
    const block::InputMatch& match,
    const block::Transaction& transaction,
    database::MatchedTransaction& matched,
    allocator_type monotonic) const noexcept -> void
{
    auto& [inputs, outputs, tx] = matched;

    if (tx.IsValid()) {
        tx.Internal().asBitcoin().MergeMetadata(
            api_.Crypto().Blockchain(),
            chain_,
            transaction.Internal().asBitcoin(),
            log_);
    } else {
        tx = transaction;
        tx.Internal().asBitcoin().SetMinedPosition(position);
    }

    set_key_data(tx, monotonic);
    const auto& [txid, outpoint, index] = match;
    const auto txin = tx.asBitcoin().Inputs();
    const auto count = txin.size();
    inputs.reserve(count);

    if (crypto::Subchain::Outgoing == subchain_) { return; }

    for (auto n = Bip32Index{0}; n < count; ++n) {
        const auto& input = txin[n];

        if (input.PreviousOutput() == outpoint) {
            log(OT_PRETTY_CLASS())(name_)(" input ")(n)(" of transaction ")
                .asHex(txid)(" spends ")(outpoint)
                .Flush();
            inputs.emplace_back(n);
            break;
        }
    }
}

auto DeterministicStateData::flush_cache(
    database::BatchedMatches& matches,
    FinishedCallback cb) const noexcept -> bool
{
    auto alloc = alloc::Strategy{get_allocator()};  // TODO monotonic
    const auto& log = log_;

    if (false == matches.empty()) {
        auto txoCreated = TXOs{alloc.result_};
        auto txoConsumed = TXOs{alloc.result_};
        auto positions = Vector<block::Position>{alloc.result_};
        positions.reserve(matches.size());
        std::transform(
            matches.begin(),
            matches.end(),
            std::back_inserter(positions),
            [](const auto& data) { return data.first; });
        const auto updated = db_.AddConfirmedTransactions(
            log_,
            id_,
            db_key_,
            std::move(matches),
            txoCreated,
            txoConsumed,
            alloc);

        if (false == updated) {
            LogError()(OT_PRETTY_CLASS())(
                name_)(": initiating rescan due to database error")
                .Flush();
            matches.clear();

            return false;
        }

        element_cache_.lock()->Add(
            std::move(txoCreated), std::move(txoConsumed));

        if (cb) { cb(positions); }
    } else {
        log(OT_PRETTY_CLASS())(name_)(" no cached transactions").Flush();
    }

    return true;
}

auto DeterministicStateData::get_index(
    const boost::shared_ptr<const SubchainStateData>& me) const noexcept -> void
{
    Index::DeterministicFactory(me, *this).Init();
}

auto DeterministicStateData::handle_block_matches(
    const block::Block& block,
    const block::Position& position,
    const block::Matches& mined,
    const Log& log,
    allocator_type monotonic) const noexcept -> void
{
    const auto& [utxo, general] = mined;
    auto transactions = database::BlockMatches{get_allocator()};

    for (const auto& match : utxo) {
        const auto& [txid, outpoint, element] = match;
        auto& arg = transactions[txid];
        log(OT_PRETTY_CLASS())(name_)(" checking transaction ")
            .asHex(txid)(" for inputs which spend ")(outpoint)
            .Flush();
        confirm_match_by_outpoint(
            log, position, match, block.FindByID(txid), arg, monotonic);
    }

    for (const auto& match : general) {
        const auto& [txid, elementID] = match;
        const auto& [index, subchainID] = elementID;
        const auto& [subchain, accountID] = subchainID;
        auto& arg = transactions[txid];
        log(OT_PRETTY_CLASS())(name_)(" checking transaction ")
            .asHex(txid)(" for outputs which match key #")(
                index)(" in subchain ")(print(subchain))(" of subaccount ")(
                accountID, api_.Crypto())
            .Flush();
        confirm_match_by_key(
            log, position, match, block.FindByID(txid), arg, monotonic);
    }

    prune_false_positives(log, transactions);
    log(OT_PRETTY_CLASS())(name_)(" adding ")(transactions.size())(
        " confirmed transaction(s) to cache")
        .Flush();
    cache_.modify([this, &log, position, incoming = std::move(transactions)](
                      auto& data) mutable {
        auto& [_, existing] = data;

        try {
            merge(log, position, incoming, existing);
        } catch (const std::exception& e) {
            LogAbort()(OT_PRETTY_CLASS())(name_)(": ")(e.what()).Abort();
        }
    });
}

auto DeterministicStateData::handle_mempool_match(
    const block::Matches& mempool,
    block::Transaction in,
    allocator_type monotonic) const noexcept -> void
{
    auto alloc = alloc::Strategy{get_allocator(), monotonic};
    const auto& log = log_;
    const auto& [utxo, general] = mempool;

    if (general.empty()) { return; }

    auto data = database::MatchedTransaction{};  // TODO allocator
    auto& [inputs, outputs, tx] = data;

    for (const auto& match : utxo) {
        confirm_match_by_outpoint(log, {}, match, in, data, alloc.work_);
    }

    for (const auto& match : general) {
        confirm_match_by_key(log, {}, match, in, data, alloc.work_);
    }

    if (inputs.empty() && outputs.empty()) { return; }

    OT_ASSERT(tx.IsValid());

    auto txoCreated = TXOs{alloc.result_};
    auto updated = db_.AddMempoolTransaction(
        log_, id_, subchain_, std::move(data), txoCreated, alloc);

    OT_ASSERT(updated);  // TODO handle database errors

    element_cache_.lock()->Add(std::move(txoCreated), {});
    log(OT_PRETTY_CLASS())(name_)(
        " finished processing unconfirmed transaction ")(tx.ID().asHex())
        .Flush();
}

auto DeterministicStateData::merge(
    const Log& log,
    block::Position block,
    database::BlockMatches& matches,
    database::BatchedMatches& into) const noexcept(false) -> void
{
    if (auto i = into.find(block); into.end() == i) {
        log(OT_PRETTY_CLASS())(name_)(" caching transactions for new block ")(
            block)
            .Flush();
        into.try_emplace(std::move(block), std::move(matches));
    } else {
        log(OT_PRETTY_CLASS())(name_)(
            " caching transactions for cached block ")(block)
            .Flush();
        merge(log, matches, i->second);
    }
}

auto DeterministicStateData::merge(
    const Log& log,
    database::BlockMatches& matches,
    database::BlockMatches& into) const noexcept(false) -> void
{
    for (auto& [txid, tx] : matches) {
        if (auto i = into.find(txid); into.end() == i) {
            log(OT_PRETTY_CLASS())(name_)(" caching transaction ")
                .asHex(txid)
                .Flush();
            into.try_emplace(txid, std::move(tx));
        } else {
            log(OT_PRETTY_CLASS())(name_)(
                " caching new indices for transaction ")
                .asHex(txid)
                .Flush();
            merge(log, tx, i->second);
        }
    }
}

auto DeterministicStateData::merge(
    const Log& log,
    const database::MatchedTransaction& tx,
    database::MatchedTransaction& into) const noexcept(false) -> void
{
    const auto& [lInputs, lOutputs, lTx] = tx;
    auto& [rInputs, rOutputs, rTx] = into;
    std::copy(lInputs.begin(), lInputs.end(), std::back_inserter(rInputs));
    std::copy(lOutputs.begin(), lOutputs.end(), std::back_inserter(rOutputs));
    dedup(rInputs);
    dedup(rOutputs);

    if (false == lTx.IsValid()) {
        throw std::runtime_error{"invalid incoming transaction"};
    }

    if (false == rTx.IsValid()) {
        throw std::runtime_error{"invalid cached transaction"};
    }

    rTx.Internal().asBitcoin().MergeMetadata(
        api_.Crypto().Blockchain(), chain_, lTx.Internal().asBitcoin(), log);
}

auto DeterministicStateData::prune_false_positives(
    const Log& log,
    database::BlockMatches& matches) const noexcept -> void
{
    for (auto i = matches.begin(); i != matches.end();) {
        const auto& [txid, match] = *i;
        const auto& [inputs, outputs, tx] = match;
        const auto falsePositive = (inputs.empty() && outputs.empty());

        if (falsePositive) {
            log(OT_PRETTY_CLASS())(name_)(" pruning transaction ")
                .asHex(txid)(
                    " since it does not contain input or output matches")
                .Flush();
            i = matches.erase(i);
        } else {
            OT_ASSERT(tx.IsValid());
            ++i;
        }
    }
}

DeterministicStateData::~DeterministicStateData() = default;
}  // namespace opentxs::blockchain::node::wallet
