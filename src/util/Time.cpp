// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"           // IWYU pragma: associated
#include "1_Internal.hpp"         // IWYU pragma: associated
#include "opentxs/util/Time.hpp"  // IWYU pragma: associated

#include <thread>

namespace opentxs
{
auto Sleep(const std::chrono::microseconds us) -> bool
{
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + us;

    do {
        std::this_thread::yield();
        std::this_thread::sleep_for(us);
    } while (std::chrono::high_resolution_clock::now() < end);

    return true;
}
}  // namespace opentxs
