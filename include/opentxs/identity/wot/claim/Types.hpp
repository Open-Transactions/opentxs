// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <tuple>

#include "opentxs/Export.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/identity/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"

namespace opentxs::identity::wot::claim
{
enum class Attribute : std::uint8_t;    // IWYU pragma: export
enum class ClaimType : std::uint32_t;   // IWYU pragma: export
enum class SectionType : std::uint8_t;  // IWYU pragma: export

OPENTXS_EXPORT auto ClaimToNym(const ClaimType in) noexcept -> identity::Type;
OPENTXS_EXPORT auto ClaimToUnit(const ClaimType in) noexcept -> UnitType;
}  // namespace opentxs::identity::wot::claim

namespace opentxs::identity
{
OPENTXS_EXPORT auto NymToClaim(const Type in) noexcept -> wot::claim::ClaimType;
}  // namespace opentxs::identity

namespace opentxs
{
OPENTXS_EXPORT auto UnitToClaim(const UnitType in) noexcept
    -> identity::wot::claim::ClaimType;
}  // namespace opentxs
