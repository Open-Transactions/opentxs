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
#include <cerrno>
#include <functional>  // IWYU pragma: keep
#include <utility>

#include "opentxs/strerror_r.hpp"
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

    if (-1 == rc) {
        LogDebug()()("failed to set thread priority to ")(
            opentxs::print(priority))(" due to: ")(error_code_to_string(errno))
            .Flush();
    }
}
}  // namespace opentxs
