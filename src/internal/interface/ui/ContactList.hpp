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
class ContactList;
class ContactListItem;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
/**
  This model manages a set of rows containing the Contacts in the wallet.
  It also contains a method for adding new contacts.
*/
class ContactList : virtual public List
{
public:
    /// Adds a Contact to the wallet.
    virtual auto AddContact(
        const UnallocatedCString& label,
        const UnallocatedCString& paymentCode = "",
        const UnallocatedCString& nymID = "") const noexcept
        -> UnallocatedCString = 0;
    /// returns the first row, containing a valid ContactListItem or an empty
    /// smart pointer (if list is empty).
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ContactListItem> = 0;
    /// returns the next row, containing a valid ContactListItem or an empty
    /// smart pointer (if at end of list).
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ContactListItem> = 0;
    virtual auto SetContactName(
        const UnallocatedCString& contactID,
        const UnallocatedCString& name) const noexcept -> bool = 0;

    ContactList(const ContactList&) = delete;
    ContactList(ContactList&&) = delete;
    auto operator=(const ContactList&) -> ContactList& = delete;
    auto operator=(ContactList&&) -> ContactList& = delete;

    ~ContactList() override = default;

protected:
    ContactList() noexcept = default;
};
}  // namespace opentxs::ui
