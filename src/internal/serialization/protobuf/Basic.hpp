// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <tuple>

#include "opentxs/util/Container.hpp"

namespace opentxs::proto
{
// This defined a map between the version of the parent object and the (minimum,
// maximum) acceptable versions of a child object.
using VersionMap =
    UnallocatedMap<std::uint32_t, std::pair<std::uint32_t, std::uint32_t>>;
}  // namespace opentxs::proto
