// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/unordered/detail/foa.hpp>

#include "internal/core/contract/Contract.hpp"  // IWYU pragma: associated

#include <ContractEnums.pb.h>
#include <ServerContract.pb.h>
#include <UnitDefinition.pb.h>
#include <boost/container_hash/hash.hpp>
#include <boost/unordered/unordered_flat_map.hpp>
#include <frozen/bits/algorithms.h>
#include <frozen/bits/elsa.h>
#include <frozen/unordered_map.h>
#include <algorithm>
#include <functional>
#include <iterator>

#include "opentxs/core/contract/ProtocolVersion.hpp"  // IWYU pragma: keep
#include "opentxs/core/contract/Types.hpp"
#include "opentxs/core/contract/UnitType.hpp"  // IWYU pragma: keep

namespace opentxs::contract::blank
{
auto Server::Serialize(proto::ServerContract& output, bool includeNym) const
    -> bool
{
    output = {};
    return true;
}

auto Unit::Serialize(proto::UnitDefinition& output, bool includeNym) const
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
    using enum proto::ProtocolVersion;

    return frozen::make_unordered_map<ProtocolVersion, proto::ProtocolVersion>({
        {Error, PROTOCOLVERSION_ERROR},
        {Legacy, PROTOCOLVERSION_LEGACY},
        {Notify, PROTOCOLVERSION_NOTIFY},
    });
}();
static constexpr auto unit_type_map_ = [] {
    using enum UnitType;
    using enum proto::UnitType;

    return frozen::make_unordered_map<UnitType, proto::UnitType>({
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
    -> proto::ProtocolVersion
{
    const auto& map = contract::protocol_version_map_;

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return proto::PROTOCOLVERSION_ERROR;
    }
}

auto translate(const contract::UnitType in) noexcept -> proto::UnitType
{
    const auto& map = contract::unit_type_map_;

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return proto::UNITTYPE_ERROR;
    }
}

auto translate(const proto::ProtocolVersion in) noexcept
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

auto translate(const proto::UnitType in) noexcept -> contract::UnitType
{
    // NOTE unit_type_map_ sometimes takes too long to invert as a frozen map
    static const auto map = [] {
        const auto& unittypes = contract::unit_type_map_;
        auto out =
            boost::unordered_flat_map<proto::UnitType, contract::UnitType>{};
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

        return contract::UnitType::Error;
    }
}
}  // namespace opentxs
