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
namespace ui
{
class AccountList;
class AccountListItem;
}  // namespace ui
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
/**
  This model manages a set of rows containing a list of accounts for the UI.
  Each row contains an AccountListItem.
*/
class OPENTXS_EXPORT AccountList : virtual public List
{
public:
    /// returns the first row, containing a valid AccountListItem or an empty
    /// smart pointer (if list is empty).
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::AccountListItem> = 0;
    /// returns the next row, containing a valid AccountListItem or an empty
    /// smart pointer (if at end of list).
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::AccountListItem> = 0;

    ~AccountList() override = default;

protected:
    AccountList() noexcept = default;

private:
    AccountList(const AccountList&) = delete;
    AccountList(AccountList&&) = delete;
    auto operator=(const AccountList&) -> AccountList& = delete;
    auto operator=(AccountList&&) -> AccountList& = delete;
};
}  // namespace opentxs::ui
