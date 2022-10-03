// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/interface/ui/List.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
class UnitList;
class UnitListItem;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
/**
 This model contains a row for each of the different unit types available in
 this wallet.
 */
class OPENTXS_EXPORT UnitList : virtual public List
{
public:
    /// Returns the first unit type available in this wallet as a UnitListItem.
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::UnitListItem> = 0;
    /// Returns the next unit type available in this wallet as a UnitListItem.
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::UnitListItem> = 0;

    UnitList(const UnitList&) = delete;
    UnitList(UnitList&&) = delete;
    auto operator=(const UnitList&) -> UnitList& = delete;
    auto operator=(UnitList&&) -> UnitList& = delete;

    ~UnitList() override = default;

protected:
    UnitList() noexcept = default;
};
}  // namespace opentxs::ui
