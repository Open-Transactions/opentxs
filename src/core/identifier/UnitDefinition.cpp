// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/core/identifier/UnitDefinition.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/core/identifier/Factory.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/identifier/AccountSubtype.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Type.hpp"            // IWYU pragma: keep
#include "opentxs/core/identifier/Types.hpp"

namespace opentxs::identifier
{
UnitDefinition::UnitDefinition(IdentifierPrivate* imp) noexcept
    : Generic(std::move(imp))
{
}

UnitDefinition::UnitDefinition(allocator_type a) noexcept
    : Generic(factory::Identifier(
          identifier::Type::unitdefinition,
          identifier::AccountSubtype::invalid_subtype,
          std::move(a)))
{
}

UnitDefinition::UnitDefinition(
    const UnitDefinition& rhs,
    allocator_type alloc) noexcept
    : Generic(rhs, std::move(alloc))
{
}

UnitDefinition::UnitDefinition(UnitDefinition&& rhs) noexcept
    : Generic(std::move(rhs))
{
}

UnitDefinition::UnitDefinition(
    UnitDefinition&& rhs,
    allocator_type alloc) noexcept
    : Generic(std::move(rhs), std::move(alloc))
{
}

auto UnitDefinition::operator=(const UnitDefinition& rhs) noexcept
    -> UnitDefinition&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::copy_assign_child<Generic>(*this, rhs);
}

auto UnitDefinition::operator=(UnitDefinition&& rhs) noexcept -> UnitDefinition&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::move_assign_child<Generic>(*this, std::move(rhs));
}

UnitDefinition::~UnitDefinition() = default;
}  // namespace opentxs::identifier
