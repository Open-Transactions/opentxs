// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/core/Amount.hpp"

#pragma once

#include <cstdint>
#include <span>
#include <string_view>
#include <utility>

#include "opentxs/Export.hpp"  // IWYU pragma: keep
#include "opentxs/core/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace display
{
class Definition;
}  // namespace display

class Amount;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain
{
using TypeEnum = std::uint32_t;

enum class Category : TypeEnum;  // IWYU pragma: export
enum class Type : TypeEnum;      // IWYU pragma: export

using HDIndex = std::uint32_t;
using ConfirmedBalance = Amount;
using UnconfirmedBalance = Amount;
using Balance = std::pair<ConfirmedBalance, UnconfirmedBalance>;

OPENTXS_EXPORT auto associated_mainnet(Type) noexcept -> Type;
OPENTXS_EXPORT auto blockchain_to_unit(const Type type) noexcept -> UnitType;
OPENTXS_EXPORT auto category(Type) noexcept -> Category;
OPENTXS_EXPORT auto defined_chains() noexcept -> std::span<const Type>;
OPENTXS_EXPORT auto display(Type) noexcept -> const display::Definition&;
OPENTXS_EXPORT auto has_segwit(Type) noexcept -> bool;
OPENTXS_EXPORT auto is_defined(Type) noexcept -> bool;
OPENTXS_EXPORT auto is_descended_from(Type lhs, Type rhs) noexcept -> bool;
OPENTXS_EXPORT auto is_supported(Type) noexcept -> bool;
OPENTXS_EXPORT auto is_testnet(Type) noexcept -> bool;
OPENTXS_EXPORT auto print(Category) noexcept -> std::string_view;
OPENTXS_EXPORT auto print(Type) noexcept -> std::string_view;
OPENTXS_EXPORT auto supported_chains() noexcept -> std::span<const Type>;
OPENTXS_EXPORT auto ticker_symbol(const Type type) noexcept
    -> UnallocatedCString;
OPENTXS_EXPORT auto ticker_symbol(const Type type, alloc::Strategy) noexcept
    -> CString;
}  // namespace opentxs::blockchain

namespace opentxs
{
OPENTXS_EXPORT auto unit_to_blockchain(const UnitType type) noexcept
    -> blockchain::Type;
}  // namespace opentxs
