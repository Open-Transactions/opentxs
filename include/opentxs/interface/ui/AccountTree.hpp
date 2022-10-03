// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/interface/ui/List.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Nym;
}  // namespace identifier

namespace ui
{
class AccountTree;
class AccountCurrency;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

/**
  This class is not yet in use. (Coming soon).
  The purpose is to provide a tree model for the UI to display the wallet's
  accounts in a tree, as a replacement for the simple list UI currently in use
  (AccountList). Each row in the AccountTree is an AccountCurrency, which
  contains all of the accounts of a given unit type. Each row in the
  AccountCurrency model contains an AccountTreeItem representing each of the
  accounts in the wallet of that currency type. For example, AccountTree may
  contain an AccountCurrency row for Bitcoin and an AccountCurrency row for
  Ethereum. The row for Bitcoin contains a list of Bitcoin accounts, and the row
  for Ethereum contains a list of Ethereum accounts (each represented as an
  AccountTreeItem).
 */
namespace opentxs::ui
{
class OPENTXS_EXPORT AccountTree : virtual public List
{
public:
    /// returns debug information relevant to the Currency of the current row.
    virtual auto Debug() const noexcept -> UnallocatedCString = 0;
    /// returns the first row, containing a valid AccountCurrency or an empty
    /// smart pointer (if list is empty).
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<AccountCurrency> = 0;
    /// returns the next row, containing a valid AccountCurrency or an empty
    /// smart pointer (if at end of list).
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<AccountCurrency> = 0;
    /// returns the NymID of the wallet owner as an identifier::Nym object.
    virtual auto Owner() const noexcept -> const identifier::Nym& = 0;

    AccountTree(const AccountTree&) = delete;
    AccountTree(AccountTree&&) = delete;
    auto operator=(const AccountTree&) -> AccountTree& = delete;
    auto operator=(AccountTree&&) -> AccountTree& = delete;

    ~AccountTree() override = default;

protected:
    AccountTree() noexcept = default;
};
}  // namespace opentxs::ui
