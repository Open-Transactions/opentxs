// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/Legacy.hpp"            // IWYU pragma: associated
#include "api/context/Context.hpp"   // IWYU pragma: associated
#include "internal/util/Thread.hpp"  // IWYU pragma: associated
#include "util/storage/drivers/filesystem/Common.hpp"  // IWYU pragma: associated

extern "C" {
#include <sys/resource.h>
#include <unistd.h>
}

#include <frozen/bits/algorithms.h>
#include <frozen/bits/basic_types.h>
#include <frozen/unordered_map.h>
#include <pthread.h>  // IWYU pragma: keep
#include <array>
#include <cerrno>
#include <cstring>

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

namespace opentxs::storage::driver::filesystem
{
auto Common::sync(DescriptorType::handle_type fd) const -> bool
{
    return 0 == ::fsync(fd);
}
}  // namespace opentxs::storage::driver::filesystem
