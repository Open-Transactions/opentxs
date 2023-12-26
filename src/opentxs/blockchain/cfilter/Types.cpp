// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/blockchain/cfilter/Types.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <utility>

#include "opentxs/blockchain/cfilter/FilterType.hpp"  // IWYU pragma: keep

namespace opentxs::blockchain::cfilter
{
using namespace std::literals;

auto print(Type in) noexcept -> std::string_view
{
    using enum Type;
    static constexpr auto map =
        frozen::make_unordered_map<Type, std::string_view>({
            {Basic_BIP158, "Basic_BIP158"sv},
            {Basic_BCHVariant, "Basic_BCHVariant"sv},
            {ES, "ES"sv},
            {UnknownCfilter, "Unknown"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown cfilter::Type"sv;
    }
}
}  // namespace opentxs::blockchain::cfilter
