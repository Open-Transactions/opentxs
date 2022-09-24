// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/contract/ContractType.hpp"
// IWYU pragma: no_include <opentxs/core/contract/ContractType.hpp>

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <string_view>

#include "opentxs/core/contract/Types.hpp"

namespace opentxs::identifier
{
enum class Algorithm : std::uint8_t;  // IWYU pragma: export
enum class Type : std::uint16_t;      // IWYU pragma: export

OPENTXS_EXPORT auto print(Algorithm) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(Type) noexcept -> std::string_view;
OPENTXS_EXPORT auto translate(Type) noexcept -> contract::Type;
}  // namespace opentxs::identifier
