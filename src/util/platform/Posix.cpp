// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"               // IWYU pragma: associated
#include "1_Internal.hpp"             // IWYU pragma: associated
#include "api/Legacy.hpp"             // IWYU pragma: associated
#include "api/context/Context.hpp"    // IWYU pragma: associated
#include "core/String.hpp"            // IWYU pragma: associated
#include "internal/util/Signals.hpp"  // IWYU pragma: associated

extern "C" {
#include <pwd.h>
#include <signal.h>
#include <sys/resource.h>
#include <unistd.h>
#include <utility>
#if __has_include(<wordexp.h>)
#include <wordexp.h>
#endif
}

#include <cerrno>
#include <cstring>

#include "internal/util/Flag.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/util/Log.hpp"

#if defined(debug) || defined(_DEBUG) || defined(DEBUG)
#define PREDEF_MODE_DEBUG 1
#endif

namespace opentxs
{

auto Signals::Block() -> void
{
    sigset_t allSignals;
    sigfillset(&allSignals);
    pthread_sigmask(SIG_SETMASK, &allSignals, nullptr);
}

auto Signals::handle() -> void
{
    sigset_t allSignals;
    sigfillset(&allSignals);

    while (running_) {
        int sig{0};

        if (0 == sigwait(&allSignals, &sig)) {
            auto shouldBreak = process(sig);

            if (shouldBreak) { break; }
        } else {
            LogError()(OT_PRETTY_CLASS())("ERROR: Invalid signal received.")
                .Flush();
        }
    }
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
#if __has_include(<wordexp.h>)
    // fabcy-pansy parser that allows for multiple level of quotes nesting and
    // escaped quotes
    if (!Exists()) return true;

    wordexp_t exp_result;

    exp_result.we_wordc = 0;
    exp_result.we_wordv = nullptr;
    exp_result.we_offs = 0;

    if (wordexp(Get(), &exp_result, 0))  // non-zero == failure.
    {
        LogError()(OT_PRETTY_CLASS())(
            "Error calling wordexp() "
            "(to expand user-defined script args). Data: ")(
            static_cast<const opentxs::String&>(*this))(".")
            .Flush();
        //        wordfree(&exp_result);
        return false;
    }

    if ((exp_result.we_wordc > 0) && (nullptr != exp_result.we_wordv)) {
        // wordexp tokenizes by space (as well as expands, which is why I'm
        // using it.)
        // Therefore we need to iterate through the tokens, and create a single
        // string
        // with spaces between the tokens.
        //
        for (std::uint32_t i = 0;
             (i < (exp_result.we_wordc - 1)) &&
             (exp_result.we_wordv[i] != nullptr) &&
             (exp_result.we_wordv[i + 1] !=
              nullptr);  // odd man out. Only PAIRS of strings are processed!
             i += 2) {
            const UnallocatedCString str_key = exp_result.we_wordv[i];
            const UnallocatedCString str_val = exp_result.we_wordv[i + 1];

            LogVerbose()(OT_PRETTY_CLASS())("Parsed: ")(str_key)(" = ")(str_val)
                .Flush();
            mapOutput.insert(std::pair<UnallocatedCString, UnallocatedCString>(
                str_key, str_val));
        }

        wordfree(&exp_result);
    }

    return true;
#else

    return false;
#endif
}
}  // namespace opentxs::implementation

namespace opentxs::api::imp
{
auto Context::HandleSignals(ShutdownCallback* callback) const noexcept -> void
{
    Lock lock(signal_handler_lock_);

    if (nullptr != callback) { shutdown_callback_ = callback; }

    if (false == bool(signal_handler_)) {
        signal_handler_ = std::make_unique<Signals>(running_);
    }
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
// Here is a security measure intended to make it more difficult to capture a
// core dump. (Not used in debug mode, obviously.)
#if !defined(PREDEF_MODE_DEBUG)
    struct rlimit rlim;
    getrlimit(RLIMIT_CORE, &rlim);
    rlim.rlim_max = rlim.rlim_cur = 0;

    if (setrlimit(RLIMIT_CORE, &rlim)) {
        LogConsole()(" setrlimit: ")(strerror(errno)).Flush();
        OT_FAIL_MSG("Crypto::Init: ASSERT: setrlimit failed. (Used for "
                    "preventing core dumps.)\n");
    }
#endif  // !defined(PREDEF_MODE_DEBUG)
}

auto Legacy::get_home_platform() noexcept -> UnallocatedCString
{
    const auto* pwd = getpwuid(getuid());

    if (nullptr != pwd) {
        if (nullptr != pwd->pw_dir) { return pwd->pw_dir; }
    }

    LogConsole()("getpwuid: ")(strerror(errno)).Flush();

    return {};
}
}  // namespace opentxs::api::imp
