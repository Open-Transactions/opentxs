// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::UnitType
// IWYU pragma: no_include "opentxs/core/Amount.hpp"

#pragma once

#include <cstdint>
#include <string_view>
#include <tuple>

#include "opentxs/Export.hpp"
#include "opentxs/core/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Amount;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain
{
using TypeEnum = std::uint32_t;

enum class Type : TypeEnum;

using HDIndex = std::uint32_t;
using ConfirmedBalance = Amount;
using UnconfirmedBalance = Amount;
using Balance = std::pair<ConfirmedBalance, UnconfirmedBalance>;

OPENTXS_EXPORT auto BlockchainToUnit(const Type type) noexcept -> UnitType;
OPENTXS_EXPORT auto print(Type) noexcept -> std::string_view;
}  // namespace opentxs::blockchain

namespace opentxs
{
OPENTXS_EXPORT auto UnitToBlockchain(const UnitType type) noexcept
    -> blockchain::Type;
}  // namespace opentxs
