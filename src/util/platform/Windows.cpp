// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"              // IWYU pragma: associated
#include "1_Internal.hpp"            // IWYU pragma: associated
#include "api/Legacy.hpp"            // IWYU pragma: associated
#include "api/context/Context.hpp"   // IWYU pragma: associated
#include "core/String.hpp"           // IWYU pragma: associated
#include "internal/util/String.hpp"  // IWYU pragma: associated
#include "opentxs/util/Signals.hpp"  // IWYU pragma: associated
#include "util/Thread.hpp"           // IWYU pragma: associated

#include <Windows.h>  // IWYU pragma: associated

extern "C" {
#include <sodium.h>
}

#include <Processthreadsapi.h>
#include <ShlObj.h>
#include <WinSock2.h>
#include <direct.h>
#include <robin_hood.h>
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

auto vformat(const char* fmt, va_list* pvl, std::string& str_Output) -> bool
{
    OT_ASSERT(nullptr != fmt);
    OT_ASSERT(nullptr != pvl);

    std::int32_t size = 0;
    std::int32_t nsize = 0;
    char* buffer = nullptr;
    va_list args;
    va_list args_2 = *pvl;  // windows only.

    args = *pvl;
    size = _vscprintf(fmt, args) + 1;
    buffer = new char[size + 100];
    OT_ASSERT(nullptr != buffer);
    ::sodium_memzero(buffer, size + 100);
    nsize = vsnprintf_s(buffer, size, size, fmt, args_2);

    OT_ASSERT(nsize >= 0);

    // fail -- delete buffer and try again If nsize was 1024 bytes, then that
    // would mean that it printed 1024 characters, even though the actual string
    // must be 1025 in length (to have room for the null terminator.) If size,
    // the ACTUAL buffer, was 1024 (that is, if size <= nsize) then size would
    // LACK the necessary space to store the 1025th byte containing the null
    // terminator. Therefore we are forced to delete the buffer and make one
    // that is nsize+1, so that it will be 1025 bytes and thus have the
    // necessary space for the terminator
    if (size <= nsize) {
        size = nsize + 1;
        delete[] buffer;
        buffer = new char[size + 100];
        OT_ASSERT(nullptr != buffer);
        ::sodium_memzero(buffer, size + 100);
        nsize = vsnprintf_s(buffer, size, size, fmt, *pvl);
        va_end(args);
        va_end(args_2);

        OT_ASSERT(nsize >= 0);
    }
    OT_ASSERT(size > nsize);

    str_Output = buffer;
    delete[] buffer;
    buffer = nullptr;
    return true;
}

auto Signals::Block() -> void
{
    LogError()("Signal handling is not supported on Windows").Flush();
}

auto Signals::handle() -> void
{
    LogError()("Signal handling is not supported on Windows").Flush();
}
}  // namespace opentxs

namespace opentxs::implementation
{
auto String::tokenize_basic(
    std::pmr::map<std::string, std::string>& mapOutput) const -> bool
{
    // simple parser that allows for one level of quotes nesting but no escaped
    // quotes
    if (!Exists()) return true;

    const char* txt = Get();
    std::string buf = txt;
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
        const std::string key = buf.substr(k, k2 - k);

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
        const std::string value = buf.substr(v, v2 - v);

        if (key.length() != 0 && value.length() != 0) {
            LogVerbose()(OT_PRETTY_CLASS())("Parsed: ")(key)(" = ")(value)
                .Flush();
            mapOutput.insert(std::pair<std::string, std::string>(key, value));
        }
    }
    return true;
}

auto String::tokenize_enhanced(
    std::pmr::map<std::string, std::string>& mapOutput) const -> bool
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

auto Legacy::get_home_platform() noexcept -> std::string
{
    auto home = std::string{getenv("USERPROFILE")};

    if (false == home.empty()) { return std::move(home); }

    const auto drive = std::string{getenv("HOMEDRIVE")};
    const auto path = std::string{getenv("HOMEPATH")};

    if ((false == drive.empty()) && (false == drive.empty())) {

        return drive + path;
    }

    return {};
}

auto Legacy::get_suffix() noexcept -> fs::path
{
    return get_suffix("OpenTransactions");
}

auto Legacy::prepend() noexcept -> std::string { return {}; }

auto Legacy::use_dot() noexcept -> bool { return false; }
}  // namespace opentxs::api::imp
