// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/wallet/subchain/DeterministicStateData.hpp"  // IWYU pragma: associated

#include <boost/container/flat_set.hpp>
#include <boost/container/vector.hpp>
#include <algorithm>
#include <chrono>
#include <compare>
#include <functional>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include "blockchain/node/wallet/subchain/statemachine/ElementCache.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/block/Transaction.hpp"
#include "internal/blockchain/database/Wallet.hpp"
#include "internal/blockchain/node/wallet/subchain/statemachine/Index.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Transaction.hpp"  // IWYU pragma: keep
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/crypto/Deterministic.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Pattern.hpp"  // IWYU pragma: keep
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Time.hpp"

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

auto DeterministicStateData::flush_cache(
    database::BatchedMatches& matches,
    FinishedCallback cb) const noexcept -> bool
{
    auto alloc = alloc::Strategy{
        element_cache_.lock_shared()->get_allocator()};  // TODO monotonic
    const auto& log = log_;

    if (false == matches.empty()) {
        auto txoCreated = TXOs{alloc.result_};
        auto txoConsumed = database::ConsumedTXOs{alloc.work_};
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

        element_cache_.lock()->Add(txoConsumed, std::move(txoCreated));

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
    const block::Position& position,
    const block::Matches& mined,
    const Log& log,
    block::Block& block,
    allocator_type monotonic) const noexcept -> void
{
    OT_ASSERT(position.hash_ == block.ID());

    block.Internal().SetMinedPosition(position.height_);
    auto alloc = alloc::Strategy{get_allocator(), monotonic};
    auto confirmed = block.Internal().ConfirmMatches(
        log, api_.Crypto().Blockchain(), mined, alloc);
    log(OT_PRETTY_CLASS())(name_)(" adding ")(confirmed.size())(
        " confirmed transaction(s) to cache")
        .Flush();
    cache_.modify([this, &log, position, incoming = std::move(confirmed)](
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
    OT_ASSERT(in.IsValid());

    const auto& log = log_;
    auto alloc = alloc::Strategy{get_allocator(), monotonic};
    auto matches = database::BlockMatches{alloc.result_};
    in.Internal().ConfirmMatches(
        log_, api_.Crypto().Blockchain(), mempool, matches, alloc);

    if (matches.empty()) { return; }

    auto& match = matches.begin()->second;
    auto& [inputs, outputs, tx] = match;

    OT_ASSERT(tx.IsValid());

    auto txoCreated = TXOs{alloc.result_};
    auto updated = db_.AddMempoolTransaction(
        log_, id_, subchain_, std::move(match), txoCreated, alloc);

    OT_ASSERT(updated);  // TODO handle database errors

    element_cache_.lock()->Add(std::move(txoCreated));
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
    constexpr auto combine = [](auto& lhs, const auto& rhs) {
        auto temp = database::MatchingIndices{lhs.get_allocator()};
        std::set_union(
            rhs.begin(),
            rhs.end(),
            lhs.begin(),
            lhs.end(),
            std::inserter(temp, temp.end()));
        lhs.swap(temp);
    };
    combine(rInputs, lInputs);
    combine(rOutputs, lOutputs);

    if (false == lTx.IsValid()) {
        throw std::runtime_error{"invalid incoming transaction"};
    }

    if (false == rTx.IsValid()) {
        throw std::runtime_error{"invalid cached transaction"};
    }

    rTx.Internal().asBitcoin().MergeMetadata(
        api_.Crypto().Blockchain(), chain_, lTx.Internal().asBitcoin(), log);
}

DeterministicStateData::~DeterministicStateData() = default;
}  // namespace opentxs::blockchain::node::wallet
