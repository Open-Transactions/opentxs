// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "opentxs/core/contract/Types.hpp"  // IWYU pragma: associated

#include <robin_hood.h>

#include "opentxs/core/contract/ContractType.hpp"
#include "opentxs/core/contract/ProtocolVersion.hpp"
#include "opentxs/core/contract/UnitType.hpp"

namespace opentxs
{
auto print(contract::ProtocolVersion in) noexcept -> const char*
{
    static const auto map =
        robin_hood::unordered_flat_map<contract::ProtocolVersion, const char*>{
            {contract::ProtocolVersion::Error, "invalid"},
            {contract::ProtocolVersion::Legacy, "legacy"},
            {contract::ProtocolVersion::Notify, "notify"},
        };

    try {

        return map.at(in);
    } catch (...) {

        return "invalid";
    }
}

auto print(contract::Type in) noexcept -> const char*
{
    static const auto map =
        robin_hood::unordered_flat_map<contract::Type, const char*>{
            {contract::Type::invalid, "invalid"},
            {contract::Type::nym, "nym"},
            {contract::Type::notary, "notary"},
            {contract::Type::unit, "unit"},
        };

    try {

        return map.at(in);
    } catch (...) {

        return "invalid";
    }
}

auto print(contract::UnitType in) noexcept -> const char*
{
    static const auto map =
        robin_hood::unordered_flat_map<contract::UnitType, const char*>{
            {contract::UnitType::Error, "invalid"},
            {contract::UnitType::Currency, "currency"},
            {contract::UnitType::Security, "security"},
            {contract::UnitType::Basket, "basket"},
        };

    try {

        return map.at(in);
    } catch (...) {

        return "invalid";
    }
}
}  // namespace opentxs
