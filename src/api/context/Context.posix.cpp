// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/context/Context.hpp"  // IWYU pragma: associated

extern "C" {
#include <sys/resource.h>
}

#include <cerrno>
#include <cstring>

#include "opentxs/util/Log.hpp"

namespace opentxs::api::imp
{
auto Context::HandleSignals(ShutdownCallback* callback) const noexcept -> void
{
    signal_handler_.modify([&](auto& data) {
        auto& handler = data.handler_;

        if (nullptr != callback) { data.callback_ = callback; }

        if (false == handler.operator bool()) {
            handler = std::make_unique<Signals>(running_);
        }
    });
}

auto Context::Init_Rlimit() noexcept -> void
{
    auto original = ::rlimit{};
    auto desired = ::rlimit{};
    auto result = ::rlimit{};
    set_desired_files(desired);

    if (0 != ::getrlimit(RLIMIT_NOFILE, &original)) {
        LogConsole()("Failed to query resource limits")(" errno: ")(
            strerror(errno))
            .Flush();

        return;
    }

    LogVerbose()("Current open files limit: ")(original.rlim_cur)(" / ")(
        original.rlim_max)
        .Flush();

    if (0 != ::setrlimit(RLIMIT_NOFILE, &desired)) {
        LogConsole()("Failed to set open file limit to ")(desired.rlim_cur)(
            ". You must increase this user account's resource limits via the "
            "method appropriate for your operating system.")(" errno: ")(
            strerror(errno))
            .Flush();

        return;
    }

    if (0 != ::getrlimit(RLIMIT_NOFILE, &result)) {
        LogConsole()("Failed to query resource limits")(" errno: ")(
            strerror(errno))
            .Flush();

        return;
    }

    LogVerbose()("Adjusted open files limit: ")(result.rlim_cur)(" / ")(
        result.rlim_max)
        .Flush();
}

auto Context::set_desired_files(::rlimit& out) noexcept -> void
{
    out.rlim_cur = 65536;
    out.rlim_max = 65536;
}
}  // namespace opentxs::api::imp
