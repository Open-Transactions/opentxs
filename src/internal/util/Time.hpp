// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/util/Time.hpp"

namespace opentxs
{
auto convert_stime(std::int64_t number) noexcept(false) -> Time;
auto convert_time(std::uint64_t number) noexcept(false) -> Time;
}  // namespace opentxs
