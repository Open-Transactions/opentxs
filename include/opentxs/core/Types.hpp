// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/identifier/Generic.hpp"

#pragma once

#include <cstdint>
#include <string_view>

#include "opentxs/Export.hpp"  // IWYU pragma: keep

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Generic;  // IWYU pragma: keep
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
using ContactID = identifier::Generic;

struct OPENTXS_EXPORT HexType {
};

static constexpr auto IsHex = HexType{};

enum class AccountType : std::int8_t;   // IWYU pragma: export
enum class AddressType : std::uint8_t;  // IWYU pragma: export

inline namespace unittype
{
enum class UnitType : std::uint32_t;  // IWYU pragma: export
}  // namespace unittype

auto OPENTXS_EXPORT print(AccountType) noexcept -> std::string_view;
auto OPENTXS_EXPORT print(AddressType) noexcept -> std::string_view;
auto OPENTXS_EXPORT print(UnitType) noexcept -> std::string_view;
}  // namespace opentxs
