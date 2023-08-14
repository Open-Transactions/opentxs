// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <string_view>

#include "internal/core/display/Types.hpp"
#include "opentxs/core/display/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace display
{
class DefinitionPrivate;
class ScalePrivate;
}  // namespace display
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
auto DisplayDefinition(
    std::string_view shortname,
    display::ScaleIndex defaultScale,
    display::ScaleIndex atomicScale,
    display::Scales&& scales) noexcept -> display::DefinitionPrivate*;
auto DisplayScale(
    std::string_view prefix,
    std::string_view suffix,
    Vector<display::Ratio>&& ratios,
    display::DecimalPlaces defaultMinDecimals,
    display::DecimalPlaces defaultMaxDecimals) noexcept
    -> display::ScalePrivate*;
}  // namespace opentxs::factory
