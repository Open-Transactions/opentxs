// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <map>

#include "opentxs/util/Numbers.hpp"

namespace opentxs
{
using VersionConversionMap = std::pmr::map<VersionNumber, VersionNumber>;
}  // namespace opentxs
