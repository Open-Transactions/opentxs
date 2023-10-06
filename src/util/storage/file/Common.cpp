// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/util/storage/file/Types.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iterator>

#include "internal/util/LogMacros.hpp"
#include "internal/util/PMR.hpp"
#include "internal/util/Thread.hpp"
#include "util/storage/file/ReaderPrivate.hpp"

namespace opentxs::storage::file
{
auto Read(std::span<const Position> in, alloc::Default alloc) noexcept
    -> Vector<Reader>
{
    auto out = Vector<Reader>{alloc};
    out.clear();
    std::ranges::transform(
        in, std::back_inserter(out), [&](const auto& position) {
            return Read(position, alloc);
        });

    return out;
}

auto Read(const Position& in, alloc::Default alloc) noexcept -> Reader
{
    OT_ASSERT(in.IsValid());
    OT_ASSERT(IsPageAligned(in.offset_));

    return pmr::construct<ReaderPrivate>(
        alloc, *in.file_name_, in.offset_, in.length_);
}

// NOTE: Write defined in src/util/platform
}  // namespace opentxs::storage::file
