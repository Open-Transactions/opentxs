// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <string_view>

#include "opentxs/Export.hpp"

namespace opentxs::contract
{
enum class ProtocolVersion : std::uint32_t;     // IWYU pragma: export
enum class Type : std::uint32_t;                // IWYU pragma: export
enum class UnitDefinitionType : std::uint32_t;  // IWYU pragma: export

OPENTXS_EXPORT auto print(ProtocolVersion) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(Type) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(UnitDefinitionType) noexcept -> std::string_view;
}  // namespace opentxs::contract
