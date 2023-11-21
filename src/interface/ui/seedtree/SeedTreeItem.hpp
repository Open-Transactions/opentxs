// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_ordered_guarded.h>
#include <shared_mutex>

#include "interface/ui/base/Combined.hpp"
#include "interface/ui/base/List.hpp"
#include "interface/ui/base/RowType.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/HDSeed.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Container.hpp"

class QVariant;

namespace opentxs::ui::implementation
{
using SeedTreeItemList = List<
    SeedTreeItemExternalInterface,
    SeedTreeItemInternalInterface,
    SeedTreeItemRowID,
    SeedTreeItemRowInterface,
    SeedTreeItemRowInternal,
    SeedTreeItemRowBlank,
    SeedTreeItemSortKey,
    SeedTreeItemPrimaryID>;
using SeedTreeItemRow =
    RowType<SeedTreeRowInternal, SeedTreeInternalInterface, SeedTreeRowID>;

class SeedTreeItem final
    : public Combined<SeedTreeItemList, SeedTreeItemRow, SeedTreeSortKey>
{
public:
    const api::session::Client& api_;

    auto API() const noexcept -> const api::Session& final { return api_; }
    auto SeedID() const noexcept -> crypto::SeedID final { return row_id_; }
    auto Debug() const noexcept -> UnallocatedCString final;
    auto Name() const noexcept -> UnallocatedCString final;
    auto Type() const noexcept -> crypto::SeedStyle final { return seed_type_; }

    SeedTreeItem(
        const SeedTreeInternalInterface& parent,
        const api::session::Client& api,
        const SeedTreeRowID& rowID,
        const SeedTreeSortKey& key,
        CustomData& custom) noexcept;
    SeedTreeItem() = delete;
    SeedTreeItem(const SeedTreeItem&) = delete;
    SeedTreeItem(SeedTreeItem&&) = delete;
    auto operator=(const SeedTreeItem&) -> SeedTreeItem& = delete;
    auto operator=(SeedTreeItem&&) -> SeedTreeItem& = delete;

    ~SeedTreeItem() final;

private:
    const crypto::SeedStyle seed_type_;
    libguarded::ordered_guarded<UnallocatedCString, std::shared_mutex>
        seed_name_;

    auto construct_row(
        const SeedTreeItemRowID& id,
        const SeedTreeItemSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;

    auto last(const SeedTreeItemRowID& id) const noexcept -> bool final
    {
        return SeedTreeItemList::last(id);
    }
    auto qt_data(const int column, const int role, QVariant& out) const noexcept
        -> void final;

    auto reindex(
        const implementation::SeedTreeSortKey& key,
        implementation::CustomData& custom) noexcept -> bool final;
};
}  // namespace opentxs::ui::implementation
