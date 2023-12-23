// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/Amount.hpp"

#pragma once

#include <cstdint>
#include <span>
#include <string_view>

#include "opentxs/Export.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Amount;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::otx::blind
{
using Denomination = Amount;
using MintSeries = std::uint64_t;

enum class CashType : std::uint8_t;    // IWYU pragma: export
enum class PurseType : std::uint8_t;   // IWYU pragma: export
enum class TokenState : std::uint8_t;  // IWYU pragma: export

auto is_supported(CashType) noexcept -> bool;
auto print(CashType) noexcept -> std::string_view;
auto supported_types() noexcept -> std::span<const CashType>;
}  // namespace opentxs::otx::blind
