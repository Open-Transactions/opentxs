// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <string_view>
#include <type_traits>

#include "opentxs/Export.hpp"

namespace opentxs::crypto::symmetric
{
enum class Algorithm : std::uint8_t;  // IWYU pragma: export
enum class Source : std::uint8_t;     // IWYU pragma: export

OPENTXS_EXPORT auto print(Algorithm) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(Source) noexcept -> std::string_view;

constexpr auto value(const Algorithm in) noexcept
{
    return static_cast<std::underlying_type_t<Algorithm>>(in);
}

constexpr auto value(const Source in) noexcept
{
    return static_cast<std::underlying_type_t<Source>>(in);
}
}  // namespace opentxs::crypto::symmetric
