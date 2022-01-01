// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <map>
#include <memory_resource>
#include <tuple>

namespace opentxs
{
namespace proto
{
// This defined a map between the version of the parent object and the (minimum,
// maximum) acceptable versions of a child object.
using VersionMap =
    std::pmr::map<std::uint32_t, std::pair<std::uint32_t, std::uint32_t>>;
}  // namespace proto
}  // namespace opentxs
