// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"            // IWYU pragma: associated
#include "1_Internal.hpp"          // IWYU pragma: associated
#include "internal/util/Size.hpp"  // IWYU pragma: associated

namespace opentxs
{
template auto convert_to_size<std::uint64_t, std::size_t>(
    std::uint64_t) noexcept(false) -> std::size_t;
template auto convert_to_size<std::size_t, std::uint32_t>(std::size_t) noexcept(
    false) -> std::uint32_t;
}  // namespace opentxs
