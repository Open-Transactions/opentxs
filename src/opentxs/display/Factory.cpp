// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/core/display/Factory.hpp"  // IWYU pragma: associated

#include <memory>
#include <utility>

#include "opentxs/display/DefinitionPrivate.hpp"
#include "opentxs/display/Scale.hpp"
#include "opentxs/display/ScalePrivate.hpp"

namespace opentxs::factory
{
auto DisplayDefinition(
    std::string_view shortname,
    display::ScaleIndex defaultScale,
    display::ScaleIndex atomicScale,
    display::Scales&& scales) noexcept -> display::DefinitionPrivate*
{
    return std::make_unique<display::DefinitionPrivate>(
               shortname, defaultScale, atomicScale, std::move(scales))
        .release();
}

auto DisplayScale(
    std::string_view prefix,
    std::string_view suffix,
    Vector<display::Ratio>&& ratios,
    const display::DecimalPlaces defaultMinDecimals,
    const display::DecimalPlaces defaultMaxDecimals) noexcept
    -> display::ScalePrivate*
{
    // NOLINTBEGIN(clang-analyzer-core.StackAddressEscape)
    return std::make_unique<display::ScalePrivate>(
               prefix,
               suffix,
               std::move(ratios),
               defaultMinDecimals,
               defaultMaxDecimals)
        .release();
    // NOLINTEND(clang-analyzer-core.StackAddressEscape)
}
}  // namespace opentxs::factory
