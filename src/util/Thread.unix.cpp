// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/util/Thread.hpp"  // IWYU pragma: associated

extern "C" {
#include <pthread.h>  // IWYU pragma: keep
#include <sys/resource.h>
#include <unistd.h>
}

#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <array>
#include <cerrno>
#include <cstring>
#include <functional>  // IWYU pragma: keep
#include <utility>

#include "opentxs/util/Log.hpp"

namespace opentxs
{
auto SetThisThreadsPriority(ThreadPriority priority) noexcept -> void
{
    using enum ThreadPriority;
    static constexpr auto map =
        frozen::make_unordered_map<ThreadPriority, int>({
            {Idle, 20},
            {Lowest, 15},
            {BelowNormal, 10},
            {Normal, 0},
            {AboveNormal, -10},
            {Highest, -15},
            {TimeCritical, -20},
        });
    const auto nice = map.at(priority);
    const auto tid = ::gettid();
    const auto rc = ::setpriority(PRIO_PROCESS, tid, nice);
    const auto error = errno;

    if (-1 == rc) {
        auto buf = std::array<char, 1024>{};
        const auto* text = ::strerror_r(error, buf.data(), buf.size());
        LogDebug()()("failed to set thread priority to ")(
            opentxs::print(priority))(" due to: ")(text)
            .Flush();
    }
}
}  // namespace opentxs
