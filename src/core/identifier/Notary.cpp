// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/core/identifier/Notary.hpp"  // IWYU pragma: associated

#include <utility>

#include "internal/core/identifier/Factory.hpp"
#include "opentxs/core/identifier/Type.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Types.hpp"

namespace opentxs::identifier
{
Notary::Notary(IdentifierPrivate* imp) noexcept
    : Generic(std::move(imp))
{
}

Notary::Notary(allocator_type a) noexcept
    : Generic(factory::Identifier(identifier::Type::notary, std::move(a)))
{
}

Notary::Notary(const Notary& rhs, allocator_type alloc) noexcept
    : Generic(rhs, std::move(alloc))
{
}

Notary::Notary(Notary&& rhs) noexcept
    : Generic(std::move(rhs))
{
}

Notary::Notary(Notary&& rhs, allocator_type alloc) noexcept
    : Generic(std::move(rhs), std::move(alloc))
{
}

auto Notary::operator=(const Notary& rhs) noexcept -> Notary& = default;

auto Notary::operator=(Notary&& rhs) noexcept -> Notary&
{
    Generic::operator=(std::move(rhs));

    return *this;
}

Notary::~Notary() = default;
}  // namespace opentxs::identifier
