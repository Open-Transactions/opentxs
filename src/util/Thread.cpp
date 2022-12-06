// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/util/Thread.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/bits/basic_types.h>
#include <frozen/unordered_map.h>
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
    using enum ThreadPriority;
    static constexpr auto map =
        frozen::make_unordered_map<ThreadPriority, const char*>({
            {Idle, "idle"},
            {Lowest, "lowest"},
            {BelowNormal, "below normal"},
            {Normal, "normal"},
            {AboveNormal, "above normal"},
            {Highest, "highest"},
            {TimeCritical, "time critical"},
        });

    try {

        return map.at(priority);
    } catch (...) {

        return "error";
    }
}
}  // namespace opentxs
