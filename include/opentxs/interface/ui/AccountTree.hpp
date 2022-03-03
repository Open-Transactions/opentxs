// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/interface/ui/List.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace identifier
{
class Nym;
}  // namespace identifier

namespace ui
{
class AccountTree;
class AccountCurrency;
}  // namespace ui
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT AccountTree : virtual public List
{
public:
    virtual auto Debug() const noexcept -> UnallocatedCString = 0;
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<AccountCurrency> = 0;
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<AccountCurrency> = 0;
    virtual auto Owner() const noexcept -> const identifier::Nym& = 0;

    ~AccountTree() override = default;

protected:
    AccountTree() noexcept = default;

private:
    AccountTree(const AccountTree&) = delete;
    AccountTree(AccountTree&&) = delete;
    auto operator=(const AccountTree&) -> AccountTree& = delete;
    auto operator=(AccountTree&&) -> AccountTree& = delete;
};
}  // namespace opentxs::ui
