// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/util/storage/file/Types.hpp"  // IWYU pragma: associated

#include <iterator>
#include <stdexcept>
#include <utility>

#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::storage::file
{
auto Write(
    const SourceData& in,
    const Location& location,
    FileMap& map) noexcept(false) -> void
{
    const auto& [cb, size] = in;
    const auto& [fileoffset, _] = location;
    const auto& [filename, offset] = fileoffset;
    auto& file = [&]() -> auto& {
        if (auto i = map.find(filename); map.end() != i) {

            return i->second;
        } else {

            return map.try_emplace(filename, filename.string()).first->second;
        }
    }();

    assert_true(file.is_open());
    assert_false(nullptr == cb);

    auto* out = std::next(file.data(), offset);

    if (false == std::invoke(cb, preallocated(size, out))) {
        throw std::runtime_error{"write failed"};
    }
}
}  // namespace opentxs::storage::file
