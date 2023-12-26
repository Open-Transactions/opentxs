// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>

// NOTE these literals can be replaced with standard equivalents after c++23
// https://www.open-std.org/jtc1/sc22/wg21/docs/papers/2020/p0330r8.html

namespace opentxs::inline literals
{
constexpr auto operator""_uz(unsigned long long int value) noexcept
{
    return static_cast<std::size_t>(value);
}

constexpr auto operator""_z(unsigned long long int value) noexcept
{
    return static_cast<std::ptrdiff_t>(value);
}
}  // namespace opentxs::inline literals
