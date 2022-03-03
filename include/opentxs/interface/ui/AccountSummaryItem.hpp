// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "ListRow.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace ui
{
class AccountSummaryItem;
}  // namespace ui

using OTUIAccountSummaryItem = SharedPimpl<ui::AccountSummaryItem>;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT AccountSummaryItem : virtual public ListRow
{
public:
    virtual auto AccountID() const noexcept -> UnallocatedCString = 0;
    virtual auto Balance() const noexcept -> Amount = 0;
    virtual auto DisplayBalance() const noexcept -> UnallocatedCString = 0;
    virtual auto Name() const noexcept -> UnallocatedCString = 0;

    ~AccountSummaryItem() override = default;

protected:
    AccountSummaryItem() noexcept = default;

private:
    AccountSummaryItem(const AccountSummaryItem&) = delete;
    AccountSummaryItem(AccountSummaryItem&&) = delete;
    auto operator=(const AccountSummaryItem&) -> AccountSummaryItem& = delete;
    auto operator=(AccountSummaryItem&&) -> AccountSummaryItem& = delete;
};
}  // namespace opentxs::ui
