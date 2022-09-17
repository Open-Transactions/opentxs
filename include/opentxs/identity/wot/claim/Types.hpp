// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/identity/IdentityType.hpp"
// IWYU pragma: no_include <opentxs/identity/IdentityType.hpp>

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstdint>
#include <tuple>

#include "opentxs/core/Types.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"

namespace opentxs::identity::wot::claim
{
enum class Attribute : std::uint8_t;
enum class ClaimType : std::uint32_t;
enum class SectionType : std::uint8_t;
}  // namespace opentxs::identity::wot::claim

namespace opentxs
{
/** C++11 representation of a claim. This version is more useful than the
 *  protobuf version, since it contains the claim ID.
 */
using Claim = std::tuple<
    UnallocatedCString,              // claim identifier
    std::uint32_t,                   // section
    std::uint32_t,                   // type
    UnallocatedCString,              // value
    Time,                            // start time
    Time,                            // end time
    UnallocatedSet<std::uint32_t>>;  // attributes
using ClaimTuple = Claim;
/** C++11 representation of all contact data associated with a nym, aggregating
 *  each the nym's contact credentials in the event it has more than one.
 */
using ClaimSet = UnallocatedSet<Claim>;

OPENTXS_EXPORT auto ClaimToNym(
    const identity::wot::claim::ClaimType in) noexcept -> identity::Type;
OPENTXS_EXPORT auto ClaimToUnit(
    const identity::wot::claim::ClaimType in) noexcept -> UnitType;
OPENTXS_EXPORT auto NymToClaim(const identity::Type in) noexcept
    -> identity::wot::claim::ClaimType;
OPENTXS_EXPORT auto UnitToClaim(const UnitType in) noexcept
    -> identity::wot::claim::ClaimType;
}  // namespace opentxs
