// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "ListRow.hpp"
#include "opentxs/Export.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
class AccountListItem;
}  // namespace ui

using OTUIAccountListItem = SharedPimpl<ui::AccountListItem>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
/**
  This model manages an AccountListItem, representing a single row in the wallet
  UI's list of accounts.
*/
class OPENTXS_EXPORT AccountListItem : virtual public ListRow
{
public:
    /// Returns the AccountID for the account.
    virtual auto AccountID() const noexcept -> UnallocatedCString = 0;
    /// Returns the balance of the account as an Amount object.
    virtual auto Balance() const noexcept -> Amount = 0;
    /// Returns the ContractID when relevant.
    virtual auto ContractID() const noexcept -> UnallocatedCString = 0;
    /// Returns the balance of the account as a formatted string for display in
    /// the UI.
    virtual auto DisplayBalance() const noexcept -> UnallocatedCString = 0;
    /// Returns the unit type of the account ("bitcoin" etc) as a formatted
    /// string for display in the UI.
    virtual auto DisplayUnit() const noexcept -> UnallocatedCString = 0;
    /// Returns display name for the account.
    virtual auto Name() const noexcept -> UnallocatedCString = 0;
    /// Returns the NotaryID for the account when relevant. (For off-chain
    /// accounts).
    virtual auto NotaryID() const noexcept -> UnallocatedCString = 0;
    /// Returns the display name for the account's notary, when relevant. (For
    /// off-chain accounts).
    virtual auto NotaryName() const noexcept -> UnallocatedCString = 0;
    /// Returns the account type. (Issuer account, off-chain account, or
    /// blockchain account).
    virtual auto Type() const noexcept -> AccountType = 0;
    /// Returns the unit type of the account as an enum.
    virtual auto Unit() const noexcept -> UnitType = 0;

    AccountListItem(const AccountListItem&) = delete;
    AccountListItem(AccountListItem&&) = delete;
    auto operator=(const AccountListItem&) -> AccountListItem& = delete;
    auto operator=(AccountListItem&&) -> AccountListItem& = delete;

    ~AccountListItem() override = default;

protected:
    AccountListItem() noexcept = default;
};
}  // namespace opentxs::ui
