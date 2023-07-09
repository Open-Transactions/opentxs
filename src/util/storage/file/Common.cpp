// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/util/storage/file/Types.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iterator>

#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Thread.hpp"
#include "util/storage/file/ReaderPrivate.hpp"

namespace opentxs::storage::file
{
auto Read(std::span<const Position> in, alloc::Strategy alloc) noexcept
    -> Vector<Reader>
{
    auto out = Vector<Reader>{alloc.result_};
    out.clear();
    std::transform(
        in.begin(),
        in.end(),
        std::back_inserter(out),
        [&](const auto& position) { return Read(position, alloc); });

    return out;
}

auto Read(const Position& in, alloc::Strategy alloc) noexcept -> Reader
{
    OT_ASSERT(in.IsValid());
    OT_ASSERT(IsPageAligned(in.offset_));

    auto pmr = alloc::PMR<ReaderPrivate>{alloc.result_};
    // TODO c++20
    auto* imp = pmr.allocate(1_uz);
    pmr.construct(imp, *in.file_name_, in.offset_, in.length_);

    return imp;
}

// NOTE: Write defined in src/util/platform
}  // namespace opentxs::storage::file
