// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/crypto/SeedStyle.hpp"

#pragma once

#include <cs_ordered_guarded.h>
#include <iosfwd>
#include <shared_mutex>
#include <utility>

#include "interface/ui/base/Combined.hpp"
#include "interface/ui/base/List.hpp"
#include "interface/ui/base/RowType.hpp"
#include "interface/ui/seedtree/SeedTreeItem.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/SharedPimpl.hpp"

class QVariant;

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace ui
{
class SeedTreeItem;
}  // namespace ui
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

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
    auto SeedID() const noexcept -> UnallocatedCString final
    {
        return row_id_->str();
    }
    auto Debug() const noexcept -> UnallocatedCString final;
    auto Name() const noexcept -> UnallocatedCString final;
    auto Type() const noexcept -> crypto::SeedStyle final { return seed_type_; }

    SeedTreeItem(
        const SeedTreeInternalInterface& parent,
        const api::session::Client& api,
        const SeedTreeRowID& rowID,
        const SeedTreeSortKey& key,
        CustomData& custom) noexcept;
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

    SeedTreeItem() = delete;
    SeedTreeItem(const SeedTreeItem&) = delete;
    SeedTreeItem(SeedTreeItem&&) = delete;
    auto operator=(const SeedTreeItem&) -> SeedTreeItem& = delete;
    auto operator=(SeedTreeItem&&) -> SeedTreeItem& = delete;
};
}  // namespace opentxs::ui::implementation
