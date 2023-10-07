// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/contact/ContactItem.hpp"  // IWYU pragma: associated

#include <memory>

#include "interface/ui/base/Widget.hpp"
#include "opentxs/identity/wot/claim/Item.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto ContactItemWidget(
    const ui::implementation::ContactSubsectionInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::ContactSubsectionRowID& rowID,
    const ui::implementation::ContactSubsectionSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::ContactSubsectionRowInternal>
{
    using ReturnType = ui::implementation::ContactItem;

    return std::make_shared<ReturnType>(parent, api, rowID, sortKey, custom);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
ContactItem::ContactItem(
    const ContactSubsectionInternalInterface& parent,
    const api::session::Client& api,
    const ContactSubsectionRowID& rowID,
    const ContactSubsectionSortKey& sortKey,
    CustomData& custom) noexcept
    : ContactItemRow(parent, api, rowID, true)
    , api_(api)
    , item_{new identity::wot::claim::Item(
          extract_custom<identity::wot::claim::Item>(custom))}
{
    assert_false(nullptr == item_);
}

auto ContactItem::reindex(
    const ContactSubsectionSortKey&,
    CustomData& custom) noexcept -> bool
{
    eLock lock(shared_lock_);
    item_ = std::make_unique<identity::wot::claim::Item>(
        extract_custom<identity::wot::claim::Item>(custom));

    assert_false(nullptr == item_);

    return true;
}
}  // namespace opentxs::ui::implementation
