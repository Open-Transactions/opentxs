// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "ListRow.hpp"
#include "opentxs/Export.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
class UnitListItem;
}  // namespace ui

using OTUIUnitListItem = SharedPimpl<ui::UnitListItem>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
/**
 This model describes a single unit type from the UnitList. (Bitcoin, Ethereum,
 etc).
 */
class OPENTXS_EXPORT UnitListItem : virtual public ListRow
{
public:
    /// The display name of this unit type.
    virtual auto Name() const noexcept -> UnallocatedCString = 0;
    /// The appropriate UnitType enum value for this unit type.
    virtual auto Unit() const noexcept -> UnitType = 0;

    UnitListItem(const UnitListItem&) = delete;
    UnitListItem(UnitListItem&&) = delete;
    auto operator=(const UnitListItem&) -> UnitListItem& = delete;
    auto operator=(UnitListItem&&) -> UnitListItem& = delete;

    ~UnitListItem() override = default;

protected:
    UnitListItem() noexcept = default;
};
}  // namespace opentxs::ui
