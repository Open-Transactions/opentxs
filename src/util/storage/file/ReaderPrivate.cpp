// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/file/ReaderPrivate.hpp"  // IWYU pragma: associated

#include <utility>

namespace opentxs::storage::file
{
ReaderPrivate::ReaderPrivate(
    std::filesystem::path file,
    std::size_t offset,
    std::size_t length,
    allocator_type alloc) noexcept
    : Allocated(std::move(alloc))
    , file_(file.string(), length, offset)
{
}
}  // namespace opentxs::storage::file
