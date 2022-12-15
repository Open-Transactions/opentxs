// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::crypto::SeedStyle

#include "interface/ui/seedtree/SeedTreeItem.hpp"  // IWYU pragma: associated

#include <iosfwd>
#include <memory>
#include <sstream>
#include <string_view>

#include "interface/ui/base/Widget.hpp"
#include "internal/interface/ui/SeedTreeNym.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/crypto/Types.hpp"

namespace opentxs::factory
{
auto SeedTreeItemModel(
    const ui::implementation::SeedTreeInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::SeedTreeRowID& rowID,
    const ui::implementation::SeedTreeSortKey& key,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::SeedTreeRowInternal>
{
    using ReturnType = ui::implementation::SeedTreeItem;

    return std::make_shared<ReturnType>(parent, api, rowID, key, custom);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
SeedTreeItem::SeedTreeItem(
    const SeedTreeInternalInterface& parent,
    const api::session::Client& api,
    const SeedTreeRowID& rowID,
    const SeedTreeSortKey& key,
    CustomData& custom) noexcept
    : Combined(
          api,
          identifier::Generic{},
          parent.WidgetID(),
          parent,
          rowID,
          key,
          false)
    , api_(api)
    , seed_type_(extract_custom<crypto::SeedStyle>(custom, 0))
    , seed_name_(key.second)
{
}

auto SeedTreeItem::construct_row(
    const SeedTreeItemRowID& id,
    const SeedTreeItemSortKey& index,
    CustomData& custom) const noexcept -> RowPointer
{
    return factory::SeedTreeNym(*this, api_, id, index, custom);
}

auto SeedTreeItem::Debug() const noexcept -> UnallocatedCString
{
    auto out = std::stringstream{};
    auto counter{-1};
    out << "         Name: " << Name() << "\n";
    out << "       SeedID: " << SeedID() << "\n";
    out << "         Type: " << print(Type()) << "\n";
    auto item = First();
    const auto PrintRow = [&counter, &out](const auto& row) {
        out << "      * row " << std::to_string(++counter) << ":\n";
        out << "                nym ID: " << row.NymID() << '\n';
        out << "              nym name: " << row.Name() << '\n';
        out << "             nym index: " << std::to_string(row.Index())
            << '\n';
    };

    if (item->Valid()) {
        PrintRow(item.get());

        while (false == item->Last()) {
            item = Next();
            PrintRow(item.get());
        }
    } else {
        out << "      * no nyms\n";
    }

    return out.str();
}

auto SeedTreeItem::Name() const noexcept -> UnallocatedCString
{
    return *seed_name_.lock_shared();
}

auto SeedTreeItem::reindex(
    const implementation::SeedTreeSortKey& key,
    implementation::CustomData& custom) noexcept -> bool
{
    const auto type = extract_custom<crypto::SeedStyle>(custom, 0);

    OT_ASSERT(seed_type_ == type);

    auto changed{false};
    seed_name_.modify([&](auto& name) {
        changed = (name != key.second);

        if (changed) { name = key.second; }
    });

    return changed;
}

SeedTreeItem::~SeedTreeItem() = default;
}  // namespace opentxs::ui::implementation
