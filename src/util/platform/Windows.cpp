// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "api/Legacy.hpp"                      // IWYU pragma: associated
#include "api/context/Context.hpp"             // IWYU pragma: associated
#include "core/String.hpp"                     // IWYU pragma: associated
#include "internal/util/Signals.hpp"           // IWYU pragma: associated
#include "network/zeromq/context/Context.hpp"  // IWYU pragma: associated
#include "util/Thread.hpp"                     // IWYU pragma: associated

#include <Windows.h>  // IWYU pragma: associated

extern "C" {
#include <sodium.h>
}

#include <Processthreadsapi.h>
#include <ShlObj.h>
#include <WinSock2.h>
#include <direct.h>
#include <robin_hood.h>
#include <iostream>
#include <xstring>

#include "internal/util/LogMacros.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
auto SetThisThreadsPriority(ThreadPriority priority) noexcept -> void
{
    static const auto map = robin_hood::unordered_flat_map<ThreadPriority, int>{
        {ThreadPriority::Idle, THREAD_PRIORITY_IDLE},
        {ThreadPriority::Lowest, THREAD_PRIORITY_LOWEST},
        {ThreadPriority::BelowNormal, THREAD_PRIORITY_BELOW_NORMAL},
        {ThreadPriority::Normal, THREAD_PRIORITY_NORMAL},
        {ThreadPriority::AboveNormal, THREAD_PRIORITY_ABOVE_NORMAL},
        {ThreadPriority::Highest, THREAD_PRIORITY_HIGHEST},
        {ThreadPriority::TimeCritical, THREAD_PRIORITY_TIME_CRITICAL},
    };
    const auto value = map.at(priority);
    const auto handle = GetCurrentThread();
    const auto rc = SetThreadPriority(handle, value);

    if (false == rc) {
        LogError()(__func__)(": failed to set thread priority to ")(
            opentxs::print(priority))
            .Flush();
    }
}

auto Signals::Block() -> void
{
    std::cout << "Signal handling is not supported on Windows\n";
}

auto Signals::handle() -> void
{
    std::cout << "Signal handling is not supported on Windows\n";
}
}  // namespace opentxs

namespace opentxs::implementation
{
auto String::tokenize_basic(
    UnallocatedMap<UnallocatedCString, UnallocatedCString>& mapOutput) const
    -> bool
{
    // simple parser that allows for one level of quotes nesting but no escaped
    // quotes
    if (!Exists()) return true;

    const char* txt = Get();
    UnallocatedCString buf = txt;
    for (std::int32_t i = 0; txt[i] != 0;) {
        while (txt[i] == ' ') i++;
        std::int32_t k = i;
        std::int32_t k2 = i;
        if (txt[i] == '\'' || txt[i] == '"') {
            // quoted string
            char quote = txt[i++];
            k = i;
            while (txt[i] != quote && txt[i] != 0) i++;
            if (txt[i] != quote) {
                LogError()(OT_PRETTY_CLASS())("Unmatched quotes in: ")(txt)(".")
                    .Flush();
                return false;
            }
            k2 = i;
            i++;
        } else {
            while (txt[i] != ' ' && txt[i] != 0) i++;
            k2 = i;
        }
        const UnallocatedCString key = buf.substr(k, k2 - k);

        while (txt[i] == ' ') i++;
        std::int32_t v = i;
        std::int32_t v2 = i;
        if (txt[i] == '\'' || txt[i] == '"') {
            // quoted string
            char quote = txt[i++];
            v = i;
            while (txt[i] != quote && txt[i] != 0) i++;
            if (txt[i] != quote) {
                LogError()(OT_PRETTY_CLASS())("Unmatched quotes in: ")(txt)(".")
                    .Flush();
                return false;
            }
            v2 = i;
            i++;
        } else {
            while (txt[i] != ' ' && txt[i] != 0) i++;
            v2 = i;
        }
        const UnallocatedCString value = buf.substr(v, v2 - v);

        if (key.length() != 0 && value.length() != 0) {
            LogVerbose()(OT_PRETTY_CLASS())("Parsed: ")(key)(" = ")(value)
                .Flush();
            mapOutput.insert(
                std::pair<UnallocatedCString, UnallocatedCString>(key, value));
        }
    }
    return true;
}

auto String::tokenize_enhanced(
    UnallocatedMap<UnallocatedCString, UnallocatedCString>& mapOutput) const
    -> bool
{
    return false;
}
}  // namespace opentxs::implementation

namespace opentxs::api::imp
{
auto Context::HandleSignals(ShutdownCallback* callback) const noexcept -> void
{
    LogError()("Signal handling is not supported on Windows").Flush();
}

auto Context::Init_Rlimit() noexcept -> void {}

auto Legacy::get_home_platform() noexcept -> UnallocatedCString
{
    auto home = UnallocatedCString{getenv("USERPROFILE")};

    if (false == home.empty()) { return std::move(home); }

    const auto drive = UnallocatedCString{getenv("HOMEDRIVE")};
    const auto path = UnallocatedCString{getenv("HOMEPATH")};

    if ((false == drive.empty()) && (false == drive.empty())) {

        return drive + path;
    }

    return {};
}

auto Legacy::get_suffix() noexcept -> fs::path
{
    return get_suffix("OpenTransactions");
}

auto Legacy::prepend() noexcept -> UnallocatedCString { return {}; }

auto Legacy::use_dot() noexcept -> bool { return false; }
}  // namespace opentxs::api::imp

namespace opentxs::network::zeromq::implementation
{
auto Context::max_sockets() noexcept -> int { return 10240; }
}  // namespace opentxs::network::zeromq::implementation
