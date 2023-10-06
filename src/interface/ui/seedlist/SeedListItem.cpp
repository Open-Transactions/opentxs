// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/seedlist/SeedListItem.hpp"  // IWYU pragma: associated

#include <memory>

#include "interface/ui/base/Widget.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto SeedListItem(
    const ui::implementation::SeedListInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::SeedListRowID& rowID,
    const ui::implementation::SeedListSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::SeedListRowInternal>
{
    using ReturnType = ui::implementation::SeedListItem;

    return std::make_shared<ReturnType>(parent, api, rowID, sortKey, custom);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
SeedListItem::SeedListItem(
    const SeedListInternalInterface& parent,
    const api::session::Client& api,
    const SeedListRowID& rowID,
    const SeedListSortKey& sortKey,
    CustomData& custom) noexcept
    : SeedListItemRow(parent, api, rowID, true)
    , api_(api)
    , type_(extract_custom<decltype(type_)>(custom, 0))
    , name_(sortKey)
{
}

auto SeedListItem::Name() const noexcept -> UnallocatedCString
{
    auto lock = Lock{lock_};

    return name_;
}

auto SeedListItem::reindex(
    const SeedListSortKey& key,
    CustomData& custom) noexcept -> bool
{
    const auto& name = key;
    const auto type = extract_custom<decltype(type_)>(custom, 0);

    assert_true(type_ == type);

    auto lock = Lock{lock_};
    auto changed{false};

    if (name_ != name) {
        changed = true;
        name_ = name;
    }

    return changed;
}

SeedListItem::~SeedListItem() = default;
}  // namespace opentxs::ui::implementation
