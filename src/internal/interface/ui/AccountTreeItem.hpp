// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/interface/ui/ListRow.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Amount;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
/**
  AccountTreeItem is the tree item that represents a single account, as seen
  from the AccountTree model. Each row in the AccountTree is an AccountCurrency,
  which contains all of the accounts of a given unit type. Each row in the
  AccountCurrency model contains an AccountTreeItem representing each of the
  accounts in the wallet of that currency type. For example, AccountTree may
  contain an AccountCurrency row for Bitcoin and an AccountCurrency row for
  Ethereum. The row for Bitcoin contains a list of Bitcoin accounts, and the row
  for Ethereum contains a list of Ethereum accounts (each represented as an
  AccountTreeItem).
 */
class AccountTreeItem : virtual public ListRow
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

    AccountTreeItem(const AccountTreeItem&) = delete;
    AccountTreeItem(AccountTreeItem&&) = delete;
    auto operator=(const AccountTreeItem&) -> AccountTreeItem& = delete;
    auto operator=(AccountTreeItem&&) -> AccountTreeItem& = delete;

    ~AccountTreeItem() override = default;

protected:
    AccountTreeItem() noexcept = default;
};
}  // namespace opentxs::ui
