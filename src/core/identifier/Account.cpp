// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/core/identifier/Account.hpp"  // IWYU pragma: associated

#include <utility>

#include "core/identifier/IdentifierPrivate.hpp"
#include "internal/core/identifier/Factory.hpp"
#include "internal/core/identifier/Identifier.hpp"
#include "internal/util/PMR.hpp"
#include "opentxs/core/identifier/AccountSubtype.hpp"  // IWYU pragma: keep
#include "opentxs/core/identifier/Type.hpp"            // IWYU pragma: keep
#include "opentxs/core/identifier/Types.hpp"

namespace opentxs::identifier
{
Account::Account(IdentifierPrivate* imp) noexcept
    : Generic(std::move(imp))
{
}

Account::Account(allocator_type a) noexcept
    : Generic(factory::Identifier(
          identifier::Type::account,
          identifier::AccountSubtype::invalid_subtype,
          std::move(a)))
{
}

Account::Account(const Account& rhs, allocator_type alloc) noexcept
    : Generic(rhs, std::move(alloc))
{
}

Account::Account(Account&& rhs) noexcept
    : Generic(std::move(rhs))
{
}

Account::Account(Account&& rhs, allocator_type alloc) noexcept
    : Generic(std::move(rhs), std::move(alloc))
{
}

auto Account::AccountType() const noexcept -> opentxs::AccountType
{
    return Internal().Get().AccountType();
}

auto Account::operator=(const Account& rhs) noexcept -> Account&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::copy_assign_child<Generic>(*this, rhs);
}

auto Account::operator=(Account&& rhs) noexcept -> Account&
{
    // NOLINTNEXTLINE(misc-unconventional-assign-operator)
    return pmr::move_assign_child<Generic>(*this, std::move(rhs));
}

auto Account::Subtype() const noexcept -> AccountSubtype
{
    return Internal().Get().account_subtype_;
}

Account::~Account() = default;
}  // namespace opentxs::identifier
