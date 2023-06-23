// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/util/storage/file/Mapped.hpp"  // IWYU pragma: associated

extern "C" {
#include <errno.h>
#include <string.h>
#include <sys/mman.h>
}

#include <cstdint>

#include "internal/util/LogMacros.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::storage::file
{
auto Mapped::preload_platform(std::span<ReadView> bytes) noexcept -> void
{
    for (const auto& item : bytes) {
        const auto rc = ::madvise(
            const_cast<char*>(item.data()), item.size(), MADV_WILLNEED);

        if (0 != rc) {
            char buf[1024];  // NOLINT(modernize-avoid-c-arrays)

            if (0 == ::strerror_r(errno, buf, sizeof(buf))) {
                LogError()(OT_PRETTY_STATIC(Mapped))(
                    "error calling madvise (MADV_WILLNEED) for ")(
                    reinterpret_cast<std::uintptr_t>(item.data()))(", ")(
                    item.size())(": ")(buf)
                    .Flush();
            } else {
                LogError()(OT_PRETTY_STATIC(Mapped))(
                    "error calling madvise (MADV_WILLNEED) for ")(
                    reinterpret_cast<std::uintptr_t>(item.data()))(", ")(
                    item.size())(": error code: ")(rc)
                    .Flush();
            }
        }
    }
}
}  // namespace opentxs::storage::file