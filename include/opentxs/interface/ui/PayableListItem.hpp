// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "ContactListItem.hpp"
#include "opentxs/Export.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
class PayableListItem;
}  // namespace ui

using OTUIPayableListItem = SharedPimpl<ui::PayableListItem>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
/**
  This model represents a single PayableListItem from a row in the PayableList
  model. It contains everything found in a ContactListItem (from which it is
  derived) but it adds a method for retrieving the payment code for this
  contact.
*/
class OPENTXS_EXPORT PayableListItem : virtual public ContactListItem
{
public:
    /// Returns the payment code for this contact.
    virtual auto PaymentCode() const noexcept -> UnallocatedCString = 0;

    PayableListItem(const PayableListItem&) = delete;
    PayableListItem(PayableListItem&&) = delete;
    auto operator=(const PayableListItem&) -> PayableListItem& = delete;
    auto operator=(PayableListItem&&) -> PayableListItem& = delete;

    ~PayableListItem() override = default;

protected:
    PayableListItem() noexcept = default;
};
}  // namespace opentxs::ui
