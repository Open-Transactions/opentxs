// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"

namespace opentxs
{
auto formatBool(bool in) -> UnallocatedCString;
auto formatTimestamp(const opentxs::Time in) -> UnallocatedCString;
auto getTimestamp() -> UnallocatedCString;
auto parseTimestamp(UnallocatedCString in) -> opentxs::Time;
}  // namespace opentxs
