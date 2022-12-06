// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/unitlist/UnitListItem.hpp"  // IWYU pragma: associated

#include <memory>

namespace opentxs::factory
{
auto UnitListItem(
    const ui::implementation::UnitListInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::UnitListRowID& rowID,
    const ui::implementation::UnitListSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::UnitListRowInternal>
{
    using ReturnType = ui::implementation::UnitListItem;

    return std::make_shared<ReturnType>(parent, api, rowID, sortKey, custom);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
UnitListItem::UnitListItem(
    const UnitListInternalInterface& parent,
    const api::session::Client& api,
    const UnitListRowID& rowID,
    const UnitListSortKey& sortKey,
    [[maybe_unused]] CustomData& custom) noexcept
    : UnitListItemRow(parent, api, rowID, true)
    , api_(api)
    , name_(sortKey)
{
}
}  // namespace opentxs::ui::implementation
