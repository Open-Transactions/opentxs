// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/core/identifier/HDSeed.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/core/identifier/Factory.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/identifier/AccountSubtype.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Type.hpp"            // IWYU pragma: keep
#include "opentxs/core/identifier/Types.hpp"

namespace opentxs::identifier
{
HDSeed::HDSeed(IdentifierPrivate* imp) noexcept
    : Generic(std::move(imp))
{
}

HDSeed::HDSeed(allocator_type a) noexcept
    : Generic(factory::Identifier(
          identifier::Type::hdseed,
          identifier::AccountSubtype::invalid_subtype,
          std::move(a)))
{
}

HDSeed::HDSeed(const HDSeed& rhs, allocator_type alloc) noexcept
    : Generic(rhs, std::move(alloc))
{
}

HDSeed::HDSeed(HDSeed&& rhs) noexcept
    : Generic(std::move(rhs))
{
}

HDSeed::HDSeed(HDSeed&& rhs, allocator_type alloc) noexcept
    : Generic(std::move(rhs), std::move(alloc))
{
}

auto HDSeed::operator=(const HDSeed& rhs) noexcept -> HDSeed&
{
    return pmr::copy_assign_child<Generic>(*this, rhs);
}

auto HDSeed::operator=(HDSeed&& rhs) noexcept -> HDSeed&
{
    return pmr::move_assign_child<Generic>(*this, std::move(rhs));
}

HDSeed::~HDSeed() = default;
}  // namespace opentxs::identifier
