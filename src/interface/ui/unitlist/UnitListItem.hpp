// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "interface/ui/base/Row.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/util/Container.hpp"

class QVariant;

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace ui
{
class UnitListItem;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
using UnitListItemRow =
    Row<UnitListRowInternal, UnitListInternalInterface, UnitListRowID>;

class UnitListItem final : public UnitListItemRow
{
public:
    const api::session::Client& api_;

    auto Name() const noexcept -> UnallocatedCString final { return name_; }
    auto Unit() const noexcept -> UnitType final { return row_id_; }

    UnitListItem(
        const UnitListInternalInterface& parent,
        const api::session::Client& api,
        const UnitListRowID& rowID,
        const UnitListSortKey& sortKey,
        CustomData& custom) noexcept;
    UnitListItem() = delete;
    UnitListItem(const UnitListItem&) = delete;
    UnitListItem(UnitListItem&&) = delete;
    auto operator=(const UnitListItem&) -> UnitListItem& = delete;
    auto operator=(UnitListItem&&) -> UnitListItem& = delete;

    ~UnitListItem() final = default;

private:
    const UnitListSortKey name_;

    auto qt_data(const int column, const int role, QVariant& out) const noexcept
        -> void final;

    auto reindex(const UnitListSortKey&, CustomData&) noexcept -> bool final
    {
        return false;
    }
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::UnitListItem>;
