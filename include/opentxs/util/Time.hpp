// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>

#include "opentxs/Export.hpp"

namespace opentxs
{
using namespace std::literals::chrono_literals;

using Clock = std::chrono::system_clock;
using sClock = std::chrono::steady_clock;
using Time = Clock::time_point;
using sTime = sClock::time_point;

OPENTXS_EXPORT auto Sleep(const std::chrono::microseconds us) -> bool;
}  // namespace opentxs
