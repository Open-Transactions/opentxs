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
class PayableList;
class PayableListItem;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
/**
  Like ContactList, this model manages a set of rows containing the Contacts in
  the wallet. However, this model only contains contacts that are payable. Each
  row contains a PayableListItem model, which is derived from ContactListItem.
*/
class PayableList : virtual public List
{
public:
    /// returns the first row, containing a valid PayableListItem or an empty
    /// smart pointer (if list is empty).
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::PayableListItem> = 0;
    /// returns the next row, containing a valid PayableListItem or an empty
    /// smart pointer (if at end of list).
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::PayableListItem> = 0;

    PayableList(const PayableList&) = delete;
    PayableList(PayableList&&) = delete;
    auto operator=(const PayableList&) -> PayableList& = delete;
    auto operator=(PayableList&&) -> PayableList& = delete;

    ~PayableList() override = default;

protected:
    PayableList() noexcept = default;
};
}  // namespace opentxs::ui
