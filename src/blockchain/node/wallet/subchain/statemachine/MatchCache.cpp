// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/wallet/subchain/statemachine/MatchCache.hpp"  // IWYU pragma: associated

#include <utility>

namespace opentxs::blockchain::node::wallet
{
MatchCache::MatchCache(allocator_type alloc) noexcept
    : results_(alloc)
{
}

auto MatchCache::Add(Results&& results) noexcept -> void
{
    for (auto& [block, index] : results) {
        results_[block].Merge(std::move(index));
    }
}

auto MatchCache::Forget(const block::Position& last) noexcept -> void
{
    auto& map = results_;
    map.erase(map.begin(), map.upper_bound(last));
}

auto MatchCache::get_allocator() const noexcept -> allocator_type
{
    return results_.get_allocator();
}

auto MatchCache::GetMatches(const block::Position& block) const noexcept
    -> std::optional<MatchIndex>
{
    if (auto i = results_.find(block); results_.end() == i) {

        return std::nullopt;
    } else {

        return i->second;
    }
}

auto MatchCache::Reset() noexcept -> void { results_.clear(); }
}  // namespace opentxs::blockchain::node::wallet
