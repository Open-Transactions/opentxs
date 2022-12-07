// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/Legacy.hpp"             // IWYU pragma: associated
#include "api/context/Context.hpp"    // IWYU pragma: associated
#include "core/FixedByteArray.tpp"    // IWYU pragma: associated
#include "core/String.hpp"            // IWYU pragma: associated
#include "internal/util/File.hpp"     // IWYU pragma: associated
#include "internal/util/Signals.hpp"  // IWYU pragma: associated
#include "internal/util/Thread.hpp"   // IWYU pragma: associated
#include "opentxs/blockchain/bitcoin/cfilter/Hash.hpp"  // IWYU pragma: associated
#include "opentxs/blockchain/bitcoin/cfilter/Header.hpp"  // IWYU pragma: associated
#include "opentxs/blockchain/block/Hash.hpp"  // IWYU pragma: associated
#include "util/storage/drivers/filesystem/Common.hpp"  // IWYU pragma: associated

#include <Windows.h>  // IWYU pragma: associated

extern "C" {
#include <sodium.h>
}

#include <Memoryapi.h>
#include <Processthreadsapi.h>
#include <ShlObj.h>
#include <WinSock2.h>
#include <direct.h>
#include <fileapi.h>
#include <frozen/unordered_map.h>
#include <pthread.h>
#include <iostream>
#include <xstring>

#include "internal/util/LogMacros.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WriteBuffer.hpp"

namespace opentxs
{
template class OPENTXS_EXPORT FixedByteArray<2_uz * sizeof(std::uint64_t)>;
template class OPENTXS_EXPORT FixedByteArray<3_uz * sizeof(std::uint64_t)>;
template class OPENTXS_EXPORT FixedByteArray<4_uz * sizeof(std::uint64_t)>;

auto flush_mapped_bytes(WriteBuffer& buf) noexcept -> bool
{
    FlushViewOfFile(buf.data(), buf.size());

    return true;
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

    if ((false == drive.empty()) && (false == path.empty())) {

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

namespace opentxs::blockchain::block
{
class OPENTXS_EXPORT Hash;
}  // namespace opentxs::blockchain::block

namespace opentxs::blockchain::cfilter
{
class OPENTXS_EXPORT Header;
class OPENTXS_EXPORT Hash;
}  // namespace opentxs::blockchain::cfilter

namespace opentxs::storage::driver::filesystem
{
auto Common::sync(DescriptorType::handle_type fd) const -> bool
{
    return FlushFileBuffers(fd);
}

Common::FileDescriptor::FileDescriptor(const fs::path& path) noexcept
    : fd_(CreateFileA(
          path.string().c_str(),
          GENERIC_READ | GENERIC_WRITE,
          0,
          NULL,
          OPEN_ALWAYS,
          FILE_FLAG_BACKUP_SEMANTICS,
          NULL))
{
    if (INVALID_HANDLE_VALUE == fd_) {
        LogError()(" FileDescriptor: error code: ")(GetLastError()).Flush();
    }
}

auto Common::FileDescriptor::good() const noexcept -> bool
{
    return (INVALID_HANDLE_VALUE != fd_);
}

Common::FileDescriptor::~FileDescriptor()
{
    if (good()) { CloseHandle(fd_); }
}
}  // namespace opentxs::storage::driver::filesystem
