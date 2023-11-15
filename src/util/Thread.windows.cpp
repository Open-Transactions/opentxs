// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/util/Thread.hpp"  // IWYU pragma: associated

#ifdef _WIN32
// NOTE this is needed to prevent iwyu from sorting Processthreadsapi.h before
// Windows.h
#include <Windows.h>
#endif

#include <Processthreadsapi.h>
#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>

#include "opentxs/util/Log.hpp"

namespace opentxs
{
auto PageSize() noexcept -> std::size_t
{
    auto info = SYSTEM_INFO{};
    GetSystemInfo(std::addressof(info));
    static_assert(sizeof(info.dwPageSize) <= sizeof(std::size_t));

    return info.dwPageSize;
}

auto SetThisThreadsPriority(ThreadPriority priority) noexcept -> void
{
    using enum ThreadPriority;
    static constexpr auto map =
        frozen::make_unordered_map<ThreadPriority, int>({
            {Idle, THREAD_PRIORITY_IDLE},
            {Lowest, THREAD_PRIORITY_LOWEST},
            {BelowNormal, THREAD_PRIORITY_BELOW_NORMAL},
            {Normal, THREAD_PRIORITY_NORMAL},
            {AboveNormal, THREAD_PRIORITY_ABOVE_NORMAL},
            {Highest, THREAD_PRIORITY_HIGHEST},
            {TimeCritical, THREAD_PRIORITY_TIME_CRITICAL},
        });
    const auto value = map.at(priority);
    const auto handle = GetCurrentThread();
    const auto rc = SetThreadPriority(handle, value);

    if (false == rc) {
        LogError()()("failed to set thread priority to ")(
            opentxs::print(priority))
            .Flush();
    }
}
}  // namespace opentxs
