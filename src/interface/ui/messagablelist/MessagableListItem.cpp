// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "interface/ui/messagablelist/MessagableListItem.hpp"  // IWYU pragma: associated

#include <memory>

namespace opentxs::factory
{
auto MessagableListItem(
    const ui::implementation::ContactListInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::ContactListRowID& rowID,
    const ui::implementation::ContactListSortKey& key) noexcept
    -> std::shared_ptr<ui::implementation::MessagableListRowInternal>
{
    using ReturnType = ui::implementation::MessagableListItem;

    return std::make_shared<ReturnType>(parent, api, rowID, key);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
MessagableListItem::MessagableListItem(
    const ContactListInternalInterface& parent,
    const api::session::Client& api,
    const ContactListRowID& rowID,
    const ContactListSortKey& key) noexcept
    : ot_super(parent, api, rowID, key)
{
    init_contact_list();
}
}  // namespace opentxs::ui::implementation
