// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/display/Scale.hpp"

#pragma once

#include <initializer_list>
#include <span>
#include <string_view>
#include <tuple>
#include <variant>

#include "internal/util/P0330.hpp"
#include "opentxs/core/display/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace display
{
class Scale;
}  // namespace display
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::display
{
using Name = CString;
using ScaleData = std::pair<Name, display::Scale>;
using Scales = Vector<ScaleData>;
using ScaleInit = std::tuple<
    std::string_view,
    std::string_view,
    std::span<const Ratio>,
    DecimalPlaces,
    DecimalPlaces>;
using ScaleDef = std::pair<std::string_view, ScaleInit>;
using ScaleRef = std::span<const ScaleDef, 1_uz>;
using ScalesInit = std::span<const ScaleRef>;
using DefInit =
    std::tuple<std::string_view, ScaleIndex, ScaleIndex, ScalesInit>;
using DefRef = std::span<const DefInit, 1_uz>;
}  // namespace opentxs::display
