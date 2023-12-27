// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <limits>
#include <stdexcept>
#include <string_view>

#include "opentxs/Types.hpp"

namespace opentxs
{
template <typename Input, typename Output = std::size_t>
auto convert_to_size(Input in) noexcept(false) -> Output
{
    if (0 > in) { throw std::out_of_range{"negative size"}; }

    static_assert(sizeof(Input) >= sizeof(Output));

    static constexpr unsigned long long max =
        std::numeric_limits<Output>::max();
    const auto compare = static_cast<unsigned long long>(in);

    if (max < compare) {
        throw std::out_of_range{"input value too large to fit in output"};
    }

    return static_cast<Output>(in);
}

inline auto int64_to_size(std::int64_t in) noexcept(false) -> std::size_t
{
    return convert_to_size<std::int64_t, std::size_t>(in);
}

inline auto shorten(std::size_t in) noexcept(false) -> std::uint32_t
{
    return convert_to_size<std::size_t, std::uint32_t>(in);
}

inline auto size_to_int(std::size_t in) noexcept(false) -> int
{
    return convert_to_size<std::size_t, int>(in);
}

inline auto uint64_to_size(std::uint64_t in) noexcept(false) -> std::size_t
{
    return convert_to_size<std::uint64_t, std::size_t>(in);
}

extern template auto convert_to_size<std::int64_t, std::size_t>(
    std::int64_t) noexcept(false) -> std::size_t;
extern template auto convert_to_size<std::uint64_t, std::size_t>(
    std::uint64_t) noexcept(false) -> std::size_t;
extern template auto convert_to_size<std::size_t, std::uint32_t>(
    std::size_t) noexcept(false) -> std::uint32_t;
auto decode_compact_size(ReadView& in, const std::string_view msg) noexcept(
    false) -> std::size_t;
}  // namespace opentxs
