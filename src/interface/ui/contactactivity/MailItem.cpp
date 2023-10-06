// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/contactactivity/MailItem.hpp"  // IWYU pragma: associated

#include <memory>

#include "interface/ui/contactactivity/ContactActivityItem.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto MailItem(
    const ui::implementation::ContactActivityInternalInterface& parent,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const ui::implementation::ContactActivityRowID& rowID,
    const ui::implementation::ContactActivitySortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::ContactActivityRowInternal>
{
    using ReturnType = ui::implementation::MailItem;

    return std::make_shared<ReturnType>(
        parent, api, nymID, rowID, sortKey, custom);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
MailItem::MailItem(
    const ContactActivityInternalInterface& parent,
    const api::session::Client& api,
    const identifier::Nym& nymID,
    const ContactActivityRowID& rowID,
    const ContactActivitySortKey& sortKey,
    CustomData& custom) noexcept
    : ContactActivityItem(parent, api, nymID, rowID, sortKey, custom)
{
    assert_false(nym_id_.empty());
    assert_false(item_id_.empty());
}

MailItem::~MailItem() = default;
}  // namespace opentxs::ui::implementation
