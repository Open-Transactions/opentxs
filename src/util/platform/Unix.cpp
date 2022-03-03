// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "api/Legacy.hpp"                      // IWYU pragma: associated
#include "api/context/Context.hpp"             // IWYU pragma: associated
#include "network/zeromq/context/Context.hpp"  // IWYU pragma: associated
#include "util/Thread.hpp"                     // IWYU pragma: associated

extern "C" {
#include <sys/resource.h>
}

#include <errno.h>
#include <robin_hood.h>
#include <string.h>
#include <unistd.h>
#include <array>

#include "opentxs/util/Log.hpp"

namespace opentxs
{
auto SetThisThreadsPriority(ThreadPriority priority) noexcept -> void
{
    static const auto map = robin_hood::unordered_flat_map<ThreadPriority, int>{
        {ThreadPriority::Idle, 20},
        {ThreadPriority::Lowest, 15},
        {ThreadPriority::BelowNormal, 10},
        {ThreadPriority::Normal, 0},
        {ThreadPriority::AboveNormal, -10},
        {ThreadPriority::Highest, -15},
        {ThreadPriority::TimeCritical, -20},
    };
    const auto nice = map.at(priority);
    const auto tid = ::gettid();
    const auto rc = ::setpriority(PRIO_PROCESS, tid, nice);
    const auto error = errno;

    if (-1 == rc) {
        auto buf = std::array<char, 1024>{};
        const auto* text = ::strerror_r(error, buf.data(), buf.size());
        LogDebug()(__func__)(": failed to set thread priority to ")(
            opentxs::print(priority))(" due to: ")(text)
            .Flush();
    }
}
}  // namespace opentxs

namespace opentxs::api::imp
{
auto Context::set_desired_files(::rlimit& out) noexcept -> void
{
    out.rlim_cur = 32768;
    out.rlim_max = 32768;
}

auto Legacy::get_suffix() noexcept -> fs::path { return get_suffix("ot"); }

auto Legacy::prepend() noexcept -> UnallocatedCString { return {}; }
}  // namespace opentxs::api::imp

namespace opentxs::network::zeromq::implementation
{
auto Context::max_sockets() noexcept -> int { return 16384; }
}  // namespace opentxs::network::zeromq::implementation
