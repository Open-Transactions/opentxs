// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/Amount.hpp"
// IWYU pragma: no_include "opentxs/core/UnitType.hpp"
// IWYU pragma: no_include "opentxs/util/Log.hpp"

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <functional>
#include <string_view>
#include <tuple>
#include <utility>

#include "opentxs/core/Data.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
class Amount;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain
{
using TypeEnum = std::uint32_t;

enum class Type : TypeEnum;

using Amount = opentxs::Amount;
using ChainHeight = std::int64_t;
using HDIndex = std::uint32_t;
using ConfirmedBalance = Amount;
using UnconfirmedBalance = Amount;
using Balance = std::pair<ConfirmedBalance, UnconfirmedBalance>;
using PatternID = std::uint64_t;
using Hash = Data;
using pHash = OTData;

OPENTXS_EXPORT auto print(Type) noexcept -> std::string_view;
}  // namespace opentxs::blockchain

namespace opentxs
{
OPENTXS_EXPORT auto BlockchainToUnit(const blockchain::Type type) noexcept
    -> UnitType;
OPENTXS_EXPORT auto UnitToBlockchain(const UnitType type) noexcept
    -> blockchain::Type;
}  // namespace opentxs
