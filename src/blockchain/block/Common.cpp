// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "internal/blockchain/block/Types.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iterator>

#include "opentxs/blockchain/block/Outpoint.hpp"  // IWYU pragma: keep
#include "opentxs/core/ByteArray.hpp"             // IWYU pragma: keep
#include "opentxs/util/Bytes.hpp"

namespace opentxs::blockchain::block
{
ParsedPatterns::ParsedPatterns(
    const Patterns& in,
    allocator_type alloc) noexcept
    : data_(alloc)
    , map_(alloc)
{
    data_.reserve(in.size());
    data_.clear();
    map_.clear();

    for (auto i{in.cbegin()}; i != in.cend(); std::advance(i, 1)) {
        const auto& [elementID, data] = *i;
        map_.emplace(reader(data), i);
        data_.emplace_back(data);
    }

    std::sort(data_.begin(), data_.end());
}

auto ParsedPatterns::get_allocator() const noexcept -> allocator_type
{
    return data_.get_allocator();
}
}  // namespace opentxs::blockchain::block

namespace opentxs::blockchain::block::internal
{
auto SetIntersection(
    const ReadView txid,
    const ParsedPatterns& parsed,
    const Elements& compare,
    alloc::Default alloc,
    alloc::Default monotonic) noexcept -> Matches
{
    auto output = std::make_pair(InputMatches{alloc}, OutputMatches{alloc});
    SetIntersection(txid, parsed, compare, {}, output, monotonic);

    return output;
}

auto SetIntersection(
    const ReadView txid,
    const ParsedPatterns& patterns,
    const Elements& compare,
    std::function<void(const Match&)> cb,
    Matches& out,
    alloc::Default monotonic) noexcept -> void
{
    const auto matches = [&] {
        auto intersection = Elements{monotonic};
        intersection.reserve(std::min(patterns.data_.size(), compare.size()));
        intersection.clear();
        std::set_intersection(
            std::begin(patterns.data_),
            std::end(patterns.data_),
            std::begin(compare),
            std::end(compare),
            std::back_inserter(intersection));

        return intersection;
    }();

    auto alloc = out.second.get_allocator();
    std::transform(
        std::begin(matches),
        std::end(matches),
        std::back_inserter(out.second),
        [&](const auto& match) {
            auto out =
                Match{{txid, alloc}, patterns.map_.at(reader(match))->first};

            if (cb) { std::invoke(cb, out); }

            return out;
        });
}
}  // namespace opentxs::blockchain::block::internal
