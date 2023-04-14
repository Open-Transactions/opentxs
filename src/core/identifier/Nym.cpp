// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/core/identifier/Nym.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/core/identifier/Factory.hpp"
#include "opentxs/core/identifier/Type.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Types.hpp"

namespace opentxs::identifier
{
Nym::Nym(IdentifierPrivate* imp) noexcept
    : Generic(std::move(imp))
{
}

Nym::Nym(allocator_type a) noexcept
    : Generic(factory::Identifier(identifier::Type::nym, std::move(a)))
{
}

Nym::Nym(const Nym& rhs, allocator_type alloc) noexcept
    : Generic(rhs, std::move(alloc))
{
}

Nym::Nym(Nym&& rhs) noexcept
    : Generic(std::move(rhs))
{
}

Nym::Nym(Nym&& rhs, allocator_type alloc) noexcept
    : Generic(std::move(rhs), std::move(alloc))
{
}

auto Nym::operator=(const Nym& rhs) noexcept -> Nym& = default;

auto Nym::operator=(Nym&& rhs) noexcept -> Nym&
{
    Generic::operator=(std::move(rhs));

    return *this;
}

Nym::~Nym() = default;
}  // namespace opentxs::identifier
