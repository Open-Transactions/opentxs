// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "ListRow.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
class AccountSummaryItem;
}  // namespace ui

using OTUIAccountSummaryItem = SharedPimpl<ui::AccountSummaryItem>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
/**
  This model manages the AccountSummaryItem, which is a single row on the
  AccountSummary model and which represents a single account in the wallet.
*/
class AccountSummaryItem : virtual public ListRow
{
public:
    /// Returns the AccountID as a string.
    virtual auto AccountID() const noexcept -> UnallocatedCString = 0;
    /// Returns the account's balance as an Amount object.
    virtual auto Balance() const noexcept -> Amount = 0;
    /// Returns the balance of the account, formatted as a string for display in
    /// the UI.
    virtual auto DisplayBalance() const noexcept -> UnallocatedCString = 0;
    /// Returns the display name of the account.
    virtual auto Name() const noexcept -> UnallocatedCString = 0;

    AccountSummaryItem(const AccountSummaryItem&) = delete;
    AccountSummaryItem(AccountSummaryItem&&) = delete;
    auto operator=(const AccountSummaryItem&) -> AccountSummaryItem& = delete;
    auto operator=(AccountSummaryItem&&) -> AccountSummaryItem& = delete;

    ~AccountSummaryItem() override = default;

protected:
    AccountSummaryItem() noexcept = default;
};
}  // namespace opentxs::ui
