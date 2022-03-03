// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "ListRow.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace ui
{
class UnitListItem;
}  // namespace ui

using OTUIUnitListItem = SharedPimpl<ui::UnitListItem>;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT UnitListItem : virtual public ListRow
{
public:
    virtual auto Name() const noexcept -> UnallocatedCString = 0;
    virtual auto Unit() const noexcept -> UnitType = 0;

    ~UnitListItem() override = default;

protected:
    UnitListItem() noexcept = default;

private:
    UnitListItem(const UnitListItem&) = delete;
    UnitListItem(UnitListItem&&) = delete;
    auto operator=(const UnitListItem&) -> UnitListItem& = delete;
    auto operator=(UnitListItem&&) -> UnitListItem& = delete;
};
}  // namespace opentxs::ui
