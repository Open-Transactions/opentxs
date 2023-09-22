// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <compare>
#include <cstring>

#include "internal/util/P0330.hpp"
#include "opentxs/util/Types.hpp"

namespace opentxs
{
// TODO this function won't be necessary after we make NDK 26 and XCode 15 the
// minimum supported versions. The standard library should provide operator<=>
// for std::string_view but libc++ as usual is slow to implement stuff.
static auto llvm_sucks(ReadView lhs, ReadView rhs) noexcept
    -> std::strong_ordering
{
    if (auto l = lhs.size(), r = rhs.size(); l < r) {

        return std::strong_ordering::less;
    } else if (r < l) {

        return std::strong_ordering::greater;
    } else {
        if (0_uz == l) {

            return std::strong_ordering::equal;
        } else if (auto c = std::memcmp(lhs.data(), rhs.data(), lhs.size());
                   0 == c) {

            return std::strong_ordering::equal;
        } else if (0 < c) {

            return std::strong_ordering::greater;
        } else {

            return std::strong_ordering::less;
        }
    }
}
}  // namespace opentxs
