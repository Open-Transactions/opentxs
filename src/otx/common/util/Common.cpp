// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/otx/common/util/Common.hpp"  // IWYU pragma: associated

#include <optional>

#include "opentxs/Time.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs
{
auto formatBool(bool in) -> UnallocatedCString { return in ? "true" : "false"; }

auto formatTimestamp(const opentxs::Time in) -> UnallocatedCString
{
    return std::to_string(seconds_since_epoch(in).value());
}

auto getTimestamp() -> UnallocatedCString
{
    return formatTimestamp(opentxs::Clock::now());
}

auto parseTimestamp(UnallocatedCString in) -> opentxs::Time
{
    try {
        return opentxs::seconds_since_epoch_unsigned(std::stoull(in)).value();
    } catch (...) {

        return {};
    }
}
}  // namespace opentxs
