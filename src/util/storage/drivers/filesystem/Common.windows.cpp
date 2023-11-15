// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/drivers/filesystem/Common.hpp"  // IWYU pragma: associated

#include <Windows.h>  // IWYU pragma: associated
#include <fileapi.h>

namespace opentxs::storage::driver::filesystem
{
auto Common::sync(DescriptorType::handle_type fd) noexcept -> bool
{
    try {

        return FlushFileBuffers(fd);
    } catch (...) {

        return false;
    }
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
