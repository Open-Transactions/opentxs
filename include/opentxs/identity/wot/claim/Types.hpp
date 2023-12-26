// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/Export.hpp"  // IWYU pragma: keep
#include "opentxs/Types.hpp"   // IWYU pragma: keep
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Numbers.hpp"

namespace opentxs::identity::wot::claim
{
enum class Attribute : std::uint8_t;    // IWYU pragma: export
enum class ClaimType : std::uint32_t;   // IWYU pragma: export
enum class SectionType : std::uint8_t;  // IWYU pragma: export

OPENTXS_EXPORT auto ClaimToNym(ClaimType) noexcept -> identity::Type;
OPENTXS_EXPORT auto ClaimToUnit(ClaimType) noexcept -> UnitType;
OPENTXS_EXPORT auto DefaultVersion() noexcept -> VersionNumber;
}  // namespace opentxs::identity::wot::claim

namespace opentxs::identity
{
OPENTXS_EXPORT auto NymToClaim(Type) noexcept -> wot::claim::ClaimType;
}  // namespace opentxs::identity

namespace opentxs
{
OPENTXS_EXPORT auto UnitToClaim(UnitType) noexcept
    -> identity::wot::claim::ClaimType;
}  // namespace opentxs
