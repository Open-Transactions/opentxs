// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "opentxs/core/identifier/UnitDefinition.hpp"  // IWYU pragma: associated

#include <type_traits>
#include <utility>

#include "internal/core/identifier/Factory.hpp"
#include "opentxs/core/identifier/Type.hpp"

namespace opentxs::identifier
{
UnitDefinition::UnitDefinition(IdentifierPrivate* imp) noexcept
    : Generic(std::move(imp))
{
}

UnitDefinition::UnitDefinition(allocator_type a) noexcept
    : Generic(
          factory::Identifier(identifier::Type::unitdefinition, std::move(a)))
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
    Generic::operator=(rhs);

    return *this;
}

auto UnitDefinition::operator=(UnitDefinition&& rhs) noexcept -> UnitDefinition&
{
    Generic::operator=(std::move(rhs));

    return *this;
}

UnitDefinition::~UnitDefinition() = default;
}  // namespace opentxs::identifier
