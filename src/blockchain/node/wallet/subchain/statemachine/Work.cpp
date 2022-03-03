// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/node/wallet/subchain/statemachine/Work.hpp"  // IWYU pragma: associated

#include <atomic>
#include <chrono>
#include <future>
#include <iterator>
#include <memory>
#include <type_traits>
#include <utility>

#include "blockchain/node/wallet/subchain/SubchainStateData.hpp"
#include "blockchain/node/wallet/subchain/statemachine/Batch.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/GCS.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::blockchain::node::wallet
{
Work::Work(const block::Position& position, Batch& batch) noexcept
    : id_(NextCookie())
    , position_(position)
    , batch_(batch)
    , block_()
    , matches_()
    , processed_(false)
    , match_count_(0)
{
}

auto Work::Do(SubchainStateData& parent) noexcept -> bool
{
    const auto start = Clock::now();
    const auto& name = parent.name_;
    const auto& type = parent.filter_type_;
    const auto& node = parent.node_;
    const auto& filters = node.FilterOracleInternal();
    const auto& position = position_;
    const auto& future = block_;
    const auto& blockHash = position.second.get();
    auto postcondition = ScopeGuard{[&] {
        if (processed_) { batch_.CompleteJob(); }
    }};
    const auto pBlock = future.get();

    if (false == bool(pBlock)) {
        LogVerbose()(OT_PRETTY_CLASS())(name)(" invalid block ")(
            blockHash.asHex())
            .Flush();

        return false;
    }

    processed_ = true;
    const auto& block = *pBlock;
    auto [elements, utxos, targets, outpoints] =
        parent.get_block_targets(blockHash, matches_);
    const auto pFilter = filters.LoadFilter(type, blockHash);

    OT_ASSERT(pFilter);

    const auto& filter = *pFilter;
    auto potential = node::internal::WalletDatabase::Patterns{};

    for (const auto& it : filter.Match(targets)) {
        // NOTE GCS::Match returns const_iterators to items in the input vector
        const auto pos = std::distance(targets.cbegin(), it);
        auto& [id, element] = elements.at(pos);
        potential.emplace_back(std::move(id), std::move(element));
    }

    const auto confirmed =
        block.Internal().FindMatches(type, outpoints, potential);
    const auto& [utxo, general] = confirmed;
    match_count_ = general.size();
    const auto& oracle = node.HeaderOracle();
    const auto pHeader = oracle.LoadHeader(blockHash);

    OT_ASSERT(pHeader);

    const auto& header = *pHeader;

    OT_ASSERT(position == header.Position());

    parent.handle_confirmed_matches(block, position, confirmed);
    const auto& log = LogVerbose();
    log(OT_PRETTY_CLASS())(name)(" block ")(block.ID().asHex())(" at height ")(
        position.first)(" processed in ")(
        std::chrono::nanoseconds{Clock::now() - start})(". ")(
        match_count_)(" of ")(potential.size())(" potential matches confirmed.")
        .Flush();

    return true;
}

auto Work::DownloadBlock(const BlockOracle& oracle) noexcept -> void
{
    const auto& log = LogTrace();
    const auto start = Clock::now();
    block_ = oracle.LoadBitcoin(position_.second);
    log(OT_PRETTY_CLASS())(" block requested from oracle in ")(
        std::chrono::nanoseconds{Clock::now() - start})
        .Flush();

    OT_ASSERT(block_.valid());
}

auto Work::GetProgress(ProgressBatch& out) noexcept -> void
{
    OT_ASSERT(processed_);

    out.emplace_back(position_, match_count_);
}

auto Work::GetResults(Results& out) noexcept -> void
{
    OT_ASSERT(processed_);

    out.emplace_back(position_.second->Bytes(), matches_);
}

auto Work::IsReady() const noexcept -> bool
{
    static constexpr auto zero = 0us;
    using State = std::future_status;

    return State::ready == block_.wait_for(zero);
}

auto Work::NextCookie() noexcept -> Cookie
{
    static auto counter = std::atomic<Cookie>{};

    return ++counter;
}

Work::~Work() = default;
}  // namespace opentxs::blockchain::node::wallet
