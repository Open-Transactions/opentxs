// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <string_view>

#include "opentxs/Export.hpp"

namespace opentxs::crypto::symmetric
{
// IWYU pragma: begin_exports
enum class Algorithm : std::uint8_t;  // IWYU pragma: keep
enum class Source : std::uint8_t;     // IWYU pragma: keep
// IWYU pragma: end_exports

OPENTXS_EXPORT auto print(Algorithm) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(Source) noexcept -> std::string_view;
}  // namespace opentxs::crypto::symmetric
