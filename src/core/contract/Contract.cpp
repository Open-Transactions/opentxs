// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/unordered/detail/foa.hpp>
// IWYU pragma: no_include <boost/unordered/detail/foa/flat_map_types.hpp>
// IWYU pragma: no_include <boost/unordered/detail/foa/table.hpp>

#include "internal/core/contract/Contract.hpp"  // IWYU pragma: associated

#include <boost/unordered/unordered_flat_map.hpp>
#include <frozen/bits/algorithms.h>
#include <frozen/bits/elsa.h>
#include <frozen/unordered_map.h>
#include <opentxs/protobuf/ContractEnums.pb.h>
#include <opentxs/protobuf/ServerContract.pb.h>
#include <opentxs/protobuf/UnitDefinition.pb.h>
#include <algorithm>
#include <functional>
#include <iterator>

#include "opentxs/contract/ProtocolVersion.hpp"  // IWYU pragma: keep
#include "opentxs/contract/Types.hpp"
#include "opentxs/contract/UnitDefinitionType.hpp"  // IWYU pragma: keep

namespace opentxs::contract::blank
{
auto Server::Serialize(protobuf::ServerContract& output, bool includeNym) const
    -> bool
{
    output = {};
    return true;
}

auto Unit::Serialize(protobuf::UnitDefinition& output, bool includeNym) const
    -> bool
{
    output = {};
    return true;
}
}  // namespace opentxs::contract::blank

namespace opentxs::contract
{
static constexpr auto protocol_version_map_ = [] {
    using enum ProtocolVersion;
    using enum protobuf::ProtocolVersion;

    return frozen::
        make_unordered_map<ProtocolVersion, protobuf::ProtocolVersion>({
            {Error, PROTOCOLVERSION_ERROR},
            {Legacy, PROTOCOLVERSION_LEGACY},
            {Notify, PROTOCOLVERSION_NOTIFY},
        });
}();
static constexpr auto unit_type_map_ = [] {
    using enum UnitDefinitionType;
    using enum protobuf::UnitType;

    return frozen::make_unordered_map<UnitDefinitionType, protobuf::UnitType>({
        {Error, UNITTYPE_ERROR},
        {Currency, UNITTYPE_CURRENCY},
        {Security, UNITTYPE_SECURITY},
        {Basket, UNITTYPE_ERROR},
    });
}();
}  // namespace opentxs::contract

namespace opentxs
{
auto translate(const contract::ProtocolVersion in) noexcept
    -> protobuf::ProtocolVersion
{
    const auto& map = contract::protocol_version_map_;

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return protobuf::PROTOCOLVERSION_ERROR;
    }
}

auto translate(const contract::UnitDefinitionType in) noexcept
    -> protobuf::UnitType
{
    const auto& map = contract::unit_type_map_;

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return protobuf::UNITTYPE_ERROR;
    }
}

auto translate(const protobuf::ProtocolVersion in) noexcept
    -> contract::ProtocolVersion
{
    static constexpr auto map =
        frozen::invert_unordered_map(contract::protocol_version_map_);

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return contract::ProtocolVersion::Error;
    }
}

auto translate(const protobuf::UnitType in) noexcept
    -> contract::UnitDefinitionType
{
    // NOTE unit_type_map_ sometimes takes too long to invert as a frozen map
    static const auto map = [] {
        const auto& unittypes = contract::unit_type_map_;
        auto out = boost::unordered_flat_map<
            protobuf::UnitType,
            contract::UnitDefinitionType>{};
        std::ranges::transform(
            unittypes, std::inserter(out, out.end()), [](const auto& data) {
                const auto& [key, value] = data;

                return std::make_pair(value, key);
            });

        return out;
    }();

    if (const auto i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return contract::UnitDefinitionType::Error;
    }
}
}  // namespace opentxs
