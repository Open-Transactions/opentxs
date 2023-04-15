// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/core/contract/Types.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <string_view>
#include <utility>

#include "opentxs/core/contract/ContractType.hpp"
#include "opentxs/core/contract/ProtocolVersion.hpp"
#include "opentxs/core/contract/UnitType.hpp"

namespace opentxs::contract
{
using namespace std::literals;

auto print(ProtocolVersion in) noexcept -> std::string_view
{
    using enum ProtocolVersion;
    static constexpr auto map =
        frozen::make_unordered_map<ProtocolVersion, std::string_view>({
            {Error, "invalid"sv},
            {Legacy, "legacy"sv},
            {Notify, "notify"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown contract::ProtocolVersion"sv;
    }
}

auto print(Type in) noexcept -> std::string_view
{
    using enum Type;
    static constexpr auto map =
        frozen::make_unordered_map<Type, std::string_view>({
            {invalid, "invalid"sv},
            {nym, "nym"sv},
            {notary, "notary"sv},
            {unit, "unit"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown contract::Type"sv;
    }
}

auto print(UnitType in) noexcept -> std::string_view
{
    using enum UnitType;
    static constexpr auto map =
        frozen::make_unordered_map<UnitType, std::string_view>({
            {Error, "invalid"sv},
            {Currency, "currency"sv},
            {Security, "security"sv},
            {Basket, "basket"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown contract::UnitType"sv;
    }
}
}  // namespace opentxs::contract
