// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

#include "opentxs/Export.hpp"

namespace opentxs
{
struct OPENTXS_EXPORT HexType {
};

static constexpr auto IsHex = HexType{};

enum class AccountType : std::int8_t;   // IWYU pragma: export
enum class AddressType : std::uint8_t;  // IWYU pragma: export
enum class UnitType : std::uint32_t;    // IWYU pragma: export

auto print(AccountType) noexcept -> std::string_view;
auto print(AddressType) noexcept -> std::string_view;
auto print(UnitType) noexcept -> std::string_view;
}  // namespace opentxs
