// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "1_Internal.hpp"                       // IWYU pragma: associated
#include "internal/blockchain/block/Block.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iterator>

#include "internal/util/BoostPMR.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"

namespace opentxs::blockchain::block
{
ParsedPatterns::ParsedPatterns(const Patterns& in) noexcept
    : data_()
    , map_()
{
    data_.reserve(in.size());

    for (auto i{in.cbegin()}; i != in.cend(); std::advance(i, 1)) {
        const auto& [elementID, data] = *i;
        map_.emplace(reader(data), i);
        data_.emplace_back(data);
    }

    std::sort(data_.begin(), data_.end());
}
}  // namespace opentxs::blockchain::block

namespace opentxs::blockchain::block::internal
{
auto SetIntersection(
    const api::Session& api,
    const ReadView txid,
    const ParsedPatterns& parsed,
    const UnallocatedVector<Space>& compare) noexcept -> Matches
{
    auto alloc = alloc::BoostMonotonic{4096};
    auto matches = Vector<Space>{&alloc};
    auto output = Matches{};
    std::set_intersection(
        std::begin(parsed.data_),
        std::end(parsed.data_),
        std::begin(compare),
        std::end(compare),
        std::back_inserter(matches));
    output.second.reserve(matches.size());
    std::transform(
        std::begin(matches),
        std::end(matches),
        std::back_inserter(output.second),
        [&](const auto& match) -> Match {
            return {
                api.Factory().Data(txid), parsed.map_.at(reader(match))->first};
        });

    return output;
}
}  // namespace opentxs::blockchain::block::internal
