// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>

#include "opentxs/Export.hpp"

namespace opentxs::contract
{
enum class ProtocolVersion : std::uint32_t;  // IWYU pragma: export
enum class Type : std::uint32_t;             // IWYU pragma: export
enum class UnitType : std::uint32_t;         // IWYU pragma: export
}  // namespace opentxs::contract

namespace opentxs
{
OPENTXS_EXPORT auto print(contract::ProtocolVersion) noexcept -> const char*;
OPENTXS_EXPORT auto print(contract::Type) noexcept -> const char*;
OPENTXS_EXPORT auto print(contract::UnitType) noexcept -> const char*;
}  // namespace opentxs
