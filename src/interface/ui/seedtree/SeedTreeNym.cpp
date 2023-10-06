// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/seedtree/SeedTreeNym.hpp"  // IWYU pragma: associated

#include <memory>

#include "interface/ui/base/Widget.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto SeedTreeNym(
    const ui::implementation::SeedTreeItemInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::SeedTreeItemRowID& rowID,
    const ui::implementation::SeedTreeItemSortKey& sortKey,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::SeedTreeItemRowInternal>
{
    using ReturnType = ui::implementation::SeedTreeNym;

    return std::make_shared<ReturnType>(parent, api, rowID, sortKey, custom);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
SeedTreeNym::SeedTreeNym(
    const SeedTreeItemInternalInterface& parent,
    const api::session::Client& api,
    const SeedTreeItemRowID& rowID,
    const SeedTreeItemSortKey& sortKey,
    CustomData& custom) noexcept
    : SeedTreeNymRow(parent, api, rowID, true)
    , api_(api)
    , index_(sortKey)
    , name_(extract_custom<UnallocatedCString>(custom, 0))
{
}

auto SeedTreeNym::Name() const noexcept -> UnallocatedCString
{
    auto lock = Lock{lock_};

    return name_;
}

auto SeedTreeNym::reindex(
    const SeedTreeItemSortKey& key,
    CustomData& custom) noexcept -> bool
{
    const auto& index = key;
    const auto name = extract_custom<UnallocatedCString>(custom, 0);

    assert_true(index_ == index);

    auto lock = Lock{lock_};
    auto changed{false};

    if (name_ != name) {
        changed = true;
        name_ = name;
    }

    return changed;
}

SeedTreeNym::~SeedTreeNym() = default;
}  // namespace opentxs::ui::implementation
