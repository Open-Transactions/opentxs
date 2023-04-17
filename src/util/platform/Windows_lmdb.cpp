// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/util/storage/file/Mapped.hpp"  // IWYU pragma: associated

namespace opentxs::storage::file
{
auto Mapped::preload_platform(std::span<ReadView>) noexcept -> void
{
    // TODO use PrefetchVirtualMemory
    // https://learn.microsoft.com/en-us/windows/win32/api/memoryapi/nf-memoryapi-prefetchvirtualmemory?redirectedfrom=MSDN
}
}  // namespace opentxs::storage::file
