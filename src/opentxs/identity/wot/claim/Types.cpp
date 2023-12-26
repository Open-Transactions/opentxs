// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <boost/unordered/detail/foa.hpp>
// IWYU pragma: no_include <boost/unordered/detail/foa/flat_map_types.hpp>
// IWYU pragma: no_include <boost/unordered/detail/foa/table.hpp>

#include "opentxs/identity/wot/claim/Types.hpp"  // IWYU pragma: associated

#include <boost/unordered/unordered_flat_map.hpp>
#include <frozen/bits/algorithms.h>
#include <frozen/bits/elsa.h>
#include <frozen/unordered_map.h>
#include <functional>
#include <utility>

#include "opentxs/Types.hpp"
#include "opentxs/UnitType.hpp"               // IWYU pragma: keep
#include "opentxs/identity/IdentityType.hpp"  // IWYU pragma: keep
#include "opentxs/identity/Types.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"  // IWYU pragma: keep
#include "util/Container.hpp"

namespace opentxs::identity::wot::claim
{
static constexpr auto identitytype_map_ = [] {
    using enum identity::Type;
    using enum ClaimType;

    return frozen::make_unordered_map<identity::Type, ClaimType>({
#include "opentxs/identity/wot/claim/identity_to_claim.inc"  // IWYU pragma: keep
    });
}();

static const auto unittype_map_ = [] {
    using enum UnitType;
    // NOTE this could be a constexpr frozen map however it takes too long to
    // compile that way.
    auto map = boost::unordered_flat_map<UnitType, ClaimType>({
#include "opentxs/identity/wot/claim/unit_to_claim.inc"  // IWYU pragma: keep
    });
    map.rehash(map.size());

    return map;
}();
}  // namespace opentxs::identity::wot::claim

namespace opentxs::identity::wot::claim
{
auto ClaimToNym(const identity::wot::claim::ClaimType in) noexcept
    -> identity::Type
{
    static constexpr auto map =
        frozen::invert_unordered_map(identity::wot::claim::identitytype_map_);

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return identity::Type::invalid;
    }
}

auto ClaimToUnit(const identity::wot::claim::ClaimType in) noexcept -> UnitType
{
    static const auto map = reverse_arbitrary_map<
        ClaimType,
        UnitType,
        boost::unordered_flat_map<ClaimType, UnitType>,
        boost::unordered_flat_map<UnitType, ClaimType>>(unittype_map_);

    if (const auto i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return UnitType::Error;
    }
}

auto DefaultVersion() noexcept -> VersionNumber { return 6; }
}  // namespace opentxs::identity::wot::claim

namespace opentxs::identity
{
auto NymToClaim(const identity::Type in) noexcept
    -> identity::wot::claim::ClaimType
{
    const auto& map = wot::claim::identitytype_map_;

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return wot::claim::ClaimType::Error;
    }
}
}  // namespace opentxs::identity

namespace opentxs
{
auto UnitToClaim(const UnitType in) noexcept -> identity::wot::claim::ClaimType
{
    const auto& map = identity::wot::claim::unittype_map_;

    if (const auto i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return identity::wot::claim::ClaimType::Error;
    }
}
}  // namespace opentxs
