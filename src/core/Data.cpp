// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"           // IWYU pragma: associated
#include "1_Internal.hpp"         // IWYU pragma: associated
#include "opentxs/core/Data.hpp"  // IWYU pragma: associated

#include <cstring>
#include <iomanip>
#include <limits>
#include <sstream>

#include "internal/core/Core.hpp"
#include "internal/util/P0330.hpp"

namespace opentxs
{
auto check_subset(
    const std::size_t size,
    const std::size_t target,
    const std::size_t pos) noexcept -> bool
{
    if (pos > size) { return false; }

    if ((std::numeric_limits<std::size_t>::max() - pos) < target) {

        return false;
    }

    if ((pos + target) > size) { return false; }

    return true;
}

auto operator==(const Data& lhs, const Data& rhs) noexcept -> bool
{
    const auto lSize = lhs.size();
    const auto rSize = rhs.size();

    if (lSize == rSize) {
        if (0_uz == lSize) {

            return true;
        } else {

            return 0 == std::memcmp(lhs.data(), rhs.data(), lhs.size());
        }
    } else {

        return false;
    }
}

auto operator<=>(const Data& lhs, const Data& rhs) noexcept
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

auto to_hex(const std::byte* in, std::size_t size) noexcept
    -> UnallocatedCString
{
    if (nullptr == in) { return {}; }

    auto out = std::stringstream{};

    for (auto i = 0_uz; i < size; ++i, ++in) {
        out << std::hex << std::setfill('0') << std::setw(2)
            << std::to_integer<int>(*in);
    }

    return out.str();
}

auto to_hex(
    const std::byte* in,
    std::size_t size,
    alloc::Default alloc) noexcept -> CString
{
    if (nullptr == in) { return CString{alloc}; }

    auto out = std::stringstream{};  // TODO c++20 use allocator

    for (auto i = 0_uz; i < size; ++i, ++in) {
        out << std::hex << std::setfill('0') << std::setw(2)
            << std::to_integer<int>(*in);
    }

    return CString{alloc}.append(out.str());
}
}  // namespace opentxs
