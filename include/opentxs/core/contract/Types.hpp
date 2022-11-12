// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

#include "opentxs/Export.hpp"

namespace opentxs::contract
{
enum class ProtocolVersion : std::uint32_t;
enum class Type : std::uint32_t;
enum class UnitType : std::uint32_t;

OPENTXS_EXPORT auto print(ProtocolVersion) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(Type) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(UnitType) noexcept -> std::string_view;
}  // namespace opentxs::contract
