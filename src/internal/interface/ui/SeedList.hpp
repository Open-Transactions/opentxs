// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/interface/ui/List.hpp"
#include "internal/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
class SeedList;
class SeedListItem;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
/**
 This model represents the list of seeds available in this wallet.
 */
class SeedList : virtual public List
{
public:
    /// Returns the first SeedListItem row.
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<SeedListItem> = 0;
    /// Returns the next SeedListItem row.
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<SeedListItem> = 0;

    SeedList(const SeedList&) = delete;
    SeedList(SeedList&&) = delete;
    auto operator=(const SeedList&) -> SeedList& = delete;
    auto operator=(SeedList&&) -> SeedList& = delete;

    ~SeedList() override = default;

protected:
    SeedList() noexcept = default;
};
}  // namespace opentxs::ui
