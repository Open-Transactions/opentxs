// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/otx/blind/Types.hpp"  // IWYU pragma: associated

#include <boost/container/flat_set.hpp>
#include <boost/move/algo/move.hpp>
#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <functional>
#include <utility>

#include "opentxs/otx/blind/CashType.hpp"  // IWYU pragma: keep

namespace opentxs::otx::blind
{
static auto supported_tokens() noexcept
    -> const boost::container::flat_set<CashType>&
{
    using enum CashType;
    static const auto map = boost::container::flat_set<CashType>{Lucre};

    return map;
}
}  // namespace opentxs::otx::blind

namespace opentxs::otx::blind
{
using namespace std::literals;

auto is_supported(CashType in) noexcept -> bool
{
    return supported_tokens().contains(in);
}

auto print(CashType in) noexcept -> std::string_view
{
    using enum CashType;
    static constexpr auto map =
        frozen::make_unordered_map<CashType, const char*>({
            {Lucre, "lucre"},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown blind::CashType"sv;
    }
}

auto supported_types() noexcept -> std::span<const CashType>
{
    return supported_tokens();
}
}  // namespace opentxs::otx::blind
