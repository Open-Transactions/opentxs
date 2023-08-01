// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "internal/interface/ui/List.hpp"
#include "internal/interface/ui/ListRow.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace identifier
{
class Generic;
}  // namespace identifier

namespace ui
{
class SeedListItem;
}  // namespace ui

using OTUISeedListItem = SharedPimpl<ui::SeedListItem>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
/**
 This model represents one of the seeds available in this wallet.
 Each of the rows in the SeedList model is a different SeedListItem.
 */
class SeedListItem : virtual public ListRow
{
public:
    /// Returns the display name for this seed.
    virtual auto Name() const noexcept -> UnallocatedCString = 0;
    /// Returns the ID for this seed.
    virtual auto SeedID() const noexcept -> const crypto::SeedID& = 0;
    /// Returns the seed type as an enum SeedStyle.
    virtual auto Type() const noexcept -> crypto::SeedStyle = 0;

    SeedListItem(const SeedListItem&) = delete;
    SeedListItem(SeedListItem&&) = delete;
    auto operator=(const SeedListItem&) -> SeedListItem& = delete;
    auto operator=(SeedListItem&&) -> SeedListItem& = delete;

    ~SeedListItem() override = default;

protected:
    SeedListItem() noexcept = default;
};
}  // namespace opentxs::ui
