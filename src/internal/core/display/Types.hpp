// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/display/Scale.hpp"

#pragma once

#include <string_view>
#include <tuple>

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
}  // namespace opentxs::display
