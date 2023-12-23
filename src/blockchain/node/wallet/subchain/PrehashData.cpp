// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/wallet/subchain/PrehashData.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <compare>
#include <iterator>
#include <shared_mutex>
#include <type_traits>
#include <utility>

#include "blockchain/node/wallet/subchain/statemachine/MatchIndex.hpp"
#include "blockchain/node/wallet/subchain/statemachine/Matches.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/cfilter/GCS.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "util/Container.hpp"

namespace opentxs::blockchain::node::wallet
{
template <typename Input, typename Output>
auto SubchainStateData::PrehashData::hash(
    const block::Hash& block,
    const std::pair<Vector<Input>, Targets>& targets,
    Output& dest) noexcept -> void
{
    const auto key = blockchain::internal::BlockHashToFilterKey(block.Bytes());
    const auto& [indices, bytes] = targets;
    auto& [hashes, map] = dest;
    auto i = indices.cbegin();
    auto t = bytes.cbegin();
    auto end = indices.cend();

    for (; i < end; ++i, ++t) {
        auto& hash = hashes.emplace_back(gcs::Siphash(api_, key, *t));
        map[hash].emplace_back(&(*i));
    }

    dedup(hashes);
}
}  // namespace opentxs::blockchain::node::wallet

namespace opentxs::blockchain::node::wallet
{
SubchainStateData::PrehashData::PrehashData(
    const api::Session& api,
    const BlockTargets& targets,
    const std::string_view name,
    wallet::MatchCache::Results& results,
    block::Height start,
    std::size_t jobs,
    allocator_type alloc) noexcept
    : job_count_(jobs)
    , api_(api)
    , targets_(targets)
    , name_(name)
    , data_(alloc)
{
    assert_true(0 < job_count_);

    data_.reserve(targets_.size());

    for (const auto& [block, elements] : targets_) {
        const auto& [e20, e32, e33, e64, e65, eTxo] = elements;
        auto& [height, data20, data32, data33, data64, data65, dataTxo] =
            data_.emplace_back();
        data20.first.reserve(e20.first.size());
        data32.first.reserve(e32.first.size());
        data33.first.reserve(e33.first.size());
        data64.first.reserve(e64.first.size());
        data65.first.reserve(e65.first.size());
        dataTxo.first.reserve(eTxo.first.size());
        height = start++;
        results[block::Position{height, block}];
    }

    assert_true(targets_.size() == data_.size());
}

auto SubchainStateData::PrehashData::hash(
    const BlockTarget& target,
    BlockData& row) noexcept -> void
{
    const auto& [block, elements] = target;
    const auto& [e20, e32, e33, e64, e65, eTxo] = elements;
    auto& [height, data20, data32, data33, data64, data65, dataTxo] = row;
    hash(block, e20, data20);
    hash(block, e32, data32);
    hash(block, e33, data33);
    hash(block, e64, data64);
    hash(block, e65, data65);
    hash(block, eTxo, dataTxo);
}

auto SubchainStateData::PrehashData::match(
    const std::string_view procedure,
    const Log& log,
    const Vector<cfilter::GCS>& cfilters,
    std::atomic_bool& atLeastOnce,
    const std::size_t job,
    wallet::MatchCache::Results& results,
    MatchResults& matched,
    alloc::Default monotonic) noexcept -> void
{
    const auto end = std::min(targets_.size(), cfilters.size());
    auto cache = std::make_tuple(
        Positions{monotonic}, Positions{monotonic}, FilterMap{monotonic});

    for (auto i = job; i < end; i += job_count_) {
        atLeastOnce.store(true);
        const auto& cfilter = cfilters.at(i);
        const auto& selected = targets_.at(i);
        const auto& data = data_.at(i);
        const auto position =
            block::Position{std::get<0>(data), selected.first};
        auto& result = results.at(position);
        match(
            procedure,
            log,
            position,
            cfilter,
            selected,
            data,
            cache,
            result,
            monotonic);
    }
    matched.modify([&](auto& out) {
        const auto& [iClean, iDirty, iSizes] = cache;
        auto& [oClean, oDirty, oSizes] = out;
        std::ranges::copy(iClean, std::inserter(oClean, oClean.end()));
        std::ranges::copy(iDirty, std::inserter(oDirty, oDirty.end()));
        std::ranges::copy(iSizes, std::inserter(oSizes, oSizes.end()));
    });
}

auto SubchainStateData::PrehashData::match(
    const std::string_view procedure,
    const Log& log,
    const block::Position& position,
    const cfilter::GCS& cfilter,
    const BlockTarget& targets,
    const BlockData& prehashed,
    AsyncResults& cache,
    wallet::MatchIndex& results,
    alloc::Default monotonic) const noexcept -> void
{
    const auto GetKeys = [&](const auto& data) {
        auto out = Set<crypto::Bip32Index>{monotonic};
        out.clear();
        const auto& [hashes, map] = data;
        const auto start = hashes.cbegin();
        const auto matches = cfilter.Internal().Match(hashes, monotonic);

        for (const auto& match : matches) {
            const auto dist = std::distance(start, match);

            assert_true(0 <= dist);

            const auto& hash = hashes.at(static_cast<std::size_t>(dist));

            for (const auto* item : map.at(hash)) { out.emplace(*item); }
        }

        return out;
    };
    const auto GetOutpoints = [&](const auto& data) {
        auto out = Set<block::Outpoint>{monotonic};
        out.clear();
        const auto& [hashes, map] = data;
        const auto start = hashes.cbegin();
        const auto matches = cfilter.Internal().Match(hashes, monotonic);

        for (const auto& match : matches) {
            const auto dist = std::distance(start, match);

            assert_true(0 <= dist);

            const auto& hash = hashes.at(static_cast<std::size_t>(dist));

            for (const auto* item : map.at(hash)) { out.emplace(*item); }
        }

        return out;
    };
    const auto GetResults = [&](const auto& cb,
                                const auto& pre,
                                const auto& selected,
                                auto& clean,
                                auto& dirty,
                                auto& output) {
        const auto matches = cb(pre);

        for (const auto& index : selected.first) {
            if (0_uz == matches.count(index)) {
                clean.emplace(index);
            } else {
                dirty.emplace(index);
            }
        }

        output.first += matches.size();
        output.second += selected.first.size();
    };
    const auto& selected = targets.second;
    const auto& [height, p20, p32, p33, p64, p65, pTxo] = prehashed;
    const auto& [s20, s32, s33, s64, s65, sTxo] = selected;
    auto output = std::pair<std::size_t, std::size_t>{};
    GetResults(
        GetKeys,
        p20,
        s20,
        results.confirmed_no_match_.match_20_,
        results.confirmed_match_.match_20_,
        output);
    GetResults(
        GetKeys,
        p32,
        s32,
        results.confirmed_no_match_.match_32_,
        results.confirmed_match_.match_32_,
        output);
    GetResults(
        GetKeys,
        p33,
        s33,
        results.confirmed_no_match_.match_33_,
        results.confirmed_match_.match_33_,
        output);
    GetResults(
        GetKeys,
        p64,
        s64,
        results.confirmed_no_match_.match_64_,
        results.confirmed_match_.match_64_,
        output);
    GetResults(
        GetKeys,
        p65,
        s65,
        results.confirmed_no_match_.match_65_,
        results.confirmed_match_.match_65_,
        output);
    GetResults(
        GetOutpoints,
        pTxo,
        sTxo,
        results.confirmed_no_match_.match_txo_,
        results.confirmed_match_.match_txo_,
        output);
    const auto& [count, of] = output;
    log()(name_)(" GCS ")(procedure)(" for block ")(position)(" matched ")(
        count)(" of ")(of)(" target elements")
        .Flush();
    auto& [clean, dirty, sizes] = cache;

    if (0_uz == count) {
        clean.emplace(position);
    } else {
        dirty.emplace(position);
    }

    sizes.emplace(position.height_, cfilter.ElementCount());
}

auto SubchainStateData::PrehashData::prepare(const std::size_t job) noexcept
    -> void
{
    const auto end = targets_.size();

    for (auto i = job; i < end; i += job_count_) {
        hash(targets_.at(i), data_.at(i));
    }
}
}  // namespace opentxs::blockchain::node::wallet
