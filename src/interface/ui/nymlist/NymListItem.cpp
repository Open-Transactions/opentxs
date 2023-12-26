// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/nymlist/NymListItem.hpp"  // IWYU pragma: associated

#include <memory>

#include "internal/interface/ui/UI.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/identifier/Nym.hpp"

namespace opentxs::factory
{
auto NymListItem(
    const ui::implementation::NymListInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::NymListRowID& rowID,
    const ui::implementation::NymListSortKey& key,
    ui::implementation::CustomData&) noexcept
    -> std::shared_ptr<ui::implementation::NymListRowInternal>
{
    using ReturnType = ui::implementation::NymListItem;

    return std::make_shared<ReturnType>(parent, api, rowID, key);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
NymListItem::NymListItem(
    const NymListInternalInterface& parent,
    const api::session::Client& api,
    const NymListRowID& rowID,
    const NymListSortKey& key) noexcept
    : NymListItemRow(parent, api, rowID, true)
    , api_(api)
    , name_(key)
{
}

auto NymListItem::NymID() const noexcept -> UnallocatedCString
{
    return row_id_.asBase58(api_.Crypto());
}

auto NymListItem::Name() const noexcept -> UnallocatedCString
{
    return *name_.lock_shared();
}

auto NymListItem::reindex(
    const NymListSortKey& key,
    CustomData& custom) noexcept -> bool
{
    auto changed{false};
    name_.modify([&](auto& name) {
        changed = (name != key);

        if (changed) { name = key; }
    });

    return changed;
}
}  // namespace opentxs::ui::implementation
