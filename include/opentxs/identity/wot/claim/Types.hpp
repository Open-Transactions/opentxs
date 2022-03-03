// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/identity/IdentityType.hpp"

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstdint>

#include "opentxs/core/Types.hpp"
#include "opentxs/identity/Types.hpp"

namespace opentxs::identity::wot::claim
{
enum class Attribute : std::uint8_t;
enum class ClaimType : std::uint32_t;
enum class SectionType : std::uint8_t;
}  // namespace opentxs::identity::wot::claim

namespace opentxs
{
OPENTXS_EXPORT auto ClaimToNym(
    const identity::wot::claim::ClaimType in) noexcept -> identity::Type;
OPENTXS_EXPORT auto ClaimToUnit(
    const identity::wot::claim::ClaimType in) noexcept -> UnitType;
OPENTXS_EXPORT auto NymToClaim(const identity::Type in) noexcept
    -> identity::wot::claim::ClaimType;
OPENTXS_EXPORT auto UnitToClaim(const UnitType in) noexcept
    -> identity::wot::claim::ClaimType;
}  // namespace opentxs
