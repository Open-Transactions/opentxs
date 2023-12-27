// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <cstdint>

#include "ListRow.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
class BalanceItem;
}  // namespace ui

using OTUIBalanceItem = SharedPimpl<ui::BalanceItem>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
/**
  This model contains a Balance Item, representing a single ledger entry for a
  specific account. The AccountActivity class contains a list of Balance Items.
*/
class BalanceItem : virtual public ListRow
{
public:
    /// Returns the balance item's amount, as an Amount object.
    virtual auto Amount() const noexcept -> opentxs::Amount = 0;
    /// Returns the number of confirmations for the associated blockchain
    /// transaction, if relevant.
    virtual auto Confirmations() const noexcept -> int = 0;
    /// Returns a vector of strings containing Contacts relevant to this balance
    /// item.
    virtual auto Contacts() const noexcept
        -> UnallocatedVector<UnallocatedCString> = 0;
    /// Returns the amount of this balance item as a string formatted for
    /// display in the UI.
    virtual auto DisplayAmount() const noexcept -> UnallocatedCString = 0;
    /// Returns the memo for this balance item.
    virtual auto Memo() const noexcept -> UnallocatedCString = 0;
    /// Returns the main display text for this balance item.
    virtual auto Text() const noexcept -> UnallocatedCString = 0;
    /// Returns the timestamp of this balance item, as a Time object.
    virtual auto Timestamp() const noexcept -> Time = 0;
    /// Returns the type of this balance item as an enum.
    virtual auto Type() const noexcept -> otx::client::StorageBox = 0;
    /// Returns the UUID of this balance item.
    virtual auto UUID() const noexcept -> UnallocatedCString = 0;
    /// Returns the workflow of this balance item.
    virtual auto Workflow() const noexcept -> UnallocatedCString = 0;

    BalanceItem(const BalanceItem&) = delete;
    BalanceItem(BalanceItem&&) = delete;
    auto operator=(const BalanceItem&) -> BalanceItem& = delete;
    auto operator=(BalanceItem&&) -> BalanceItem& = delete;

    ~BalanceItem() override = default;

protected:
    BalanceItem() noexcept = default;
};
}  // namespace opentxs::ui
