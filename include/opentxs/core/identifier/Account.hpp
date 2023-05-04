// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Account;
class IdentifierPrivate;
}  // namespace identifier
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct OPENTXS_EXPORT hash<opentxs::identifier::Account> {
    using is_transparent = void;
    using is_avalanching = void;

    auto operator()(const opentxs::identifier::Account& data) const noexcept
        -> std::size_t;
};
}  // namespace std

namespace opentxs::identifier
{
class OPENTXS_EXPORT Account : virtual public identifier::Generic
{
public:
    auto AccountType() const noexcept -> opentxs::AccountType;
    auto Subtype() const noexcept -> AccountSubtype;

    OPENTXS_NO_EXPORT Account(IdentifierPrivate* imp) noexcept;
    Account(allocator_type alloc = {}) noexcept;
    Account(const Account& rhs, allocator_type alloc = {}) noexcept;
    Account(Account&& rhs) noexcept;
    Account(Account&& rhs, allocator_type alloc) noexcept;
    auto operator=(const Account& rhs) noexcept -> Account&;
    auto operator=(Account&& rhs) noexcept -> Account&;

    ~Account() override;
};
}  // namespace opentxs::identifier
