// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/interface/ui/List.hpp"
#include "internal/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
class AccountSummary;
class IssuerItem;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
/**
  This model manages the AccountSummary, containing connection and trusted
  status for various issuers. NOTE: this model is being updated to manage a list
  of AccountSummaryItem instead of IssuerItem.
*/
class AccountSummary : virtual public List
{
public:
    /// returns the first row, containing a valid IssuerItem or an empty smart
    /// pointer (if list is empty).
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::IssuerItem> = 0;
    /// returns the next row, containing a valid IssuerItem or an empty smart
    /// pointer (if at end of list).
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::IssuerItem> = 0;

    AccountSummary(const AccountSummary&) = delete;
    AccountSummary(AccountSummary&&) = delete;
    auto operator=(const AccountSummary&) -> AccountSummary& = delete;
    auto operator=(AccountSummary&&) -> AccountSummary& = delete;

    ~AccountSummary() override = default;

protected:
    AccountSummary() noexcept = default;
};
}  // namespace opentxs::ui
