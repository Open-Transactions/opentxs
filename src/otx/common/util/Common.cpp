// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "1_Internal.hpp"                       // IWYU pragma: associated
#include "internal/otx/common/util/Common.hpp"  // IWYU pragma: associated

#include "opentxs/util/Container.hpp"

namespace opentxs
{
auto formatBool(bool in) -> UnallocatedCString { return in ? "true" : "false"; }

auto formatTimestamp(const opentxs::Time in) -> UnallocatedCString
{
    return std::to_string(opentxs::Clock::to_time_t(in));
}

auto getTimestamp() -> UnallocatedCString
{
    return formatTimestamp(opentxs::Clock::now());
}

auto parseTimestamp(UnallocatedCString in) -> opentxs::Time
{
    try {
        return opentxs::Clock::from_time_t(std::stoull(in));
    } catch (...) {

        return {};
    }
}
}  // namespace opentxs
