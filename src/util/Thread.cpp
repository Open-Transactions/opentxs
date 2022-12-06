// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/util/Thread.hpp"  // IWYU pragma: associated

#include <robin_hood.h>
#include <algorithm>
#include <thread>

#include "internal/api/Context.hpp"

namespace opentxs
{
auto MaxJobs() noexcept -> unsigned int
{
    const auto configured = api::internal::Context::MaxJobs();
    const auto hardware = std::max(std::thread::hardware_concurrency(), 1u);

    if (0u == configured) {

        return hardware;
    } else {

        return std::min(configured, hardware);
    }
}

auto print(ThreadPriority priority) noexcept -> const char*
{
    static const auto map =
        robin_hood::unordered_flat_map<ThreadPriority, const char*>{
            {ThreadPriority::Idle, "idle"},
            {ThreadPriority::Lowest, "lowest"},
            {ThreadPriority::BelowNormal, "below normal"},
            {ThreadPriority::Normal, "normal"},
            {ThreadPriority::AboveNormal, "above normal"},
            {ThreadPriority::Highest, "highest"},
            {ThreadPriority::TimeCritical, "time critical"},
        };

    try {

        return map.at(priority);
    } catch (...) {

        return "error";
    }
}
}  // namespace opentxs
