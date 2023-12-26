// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <optional>
#include <utility>

namespace opentxs::display
{
using DecimalCount = std::uint_fast8_t;
using DecimalPlaces = std::optional<DecimalCount>;
using ScaleIndex = unsigned int;
using SpecifiedScale = std::optional<ScaleIndex>;
using Ratio = std::pair<std::uint_fast8_t, std::int_fast8_t>;

auto to_scale(int value) noexcept -> SpecifiedScale;
}  // namespace opentxs::display
