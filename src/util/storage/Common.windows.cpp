// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/util/storage/file/Types.hpp"  // IWYU pragma: associated

#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::storage::file
{
auto Write(const SourceData& in, const Location& location, FileMap&) noexcept(
    false) -> void
{
    const auto& [cb, size] = in;
    const auto& [_, range] = location;

    assert_true(range.size() == size);

    if (false == std::invoke(cb, preallocated(size, range.data()))) {
        throw std::runtime_error{"write failed"};
    }
}
}  // namespace opentxs::storage::file
