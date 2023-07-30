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
namespace ui
{
class SeedTreeNym;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
/**
 This model represents one of the seeds available in this wallet.
 Each of the rows in the SeedTree model is a different SeedTreeItem.
 */
class SeedTreeItem : virtual public List, virtual public ListRow
{
public:
    /// Returns debug information about this item.
    virtual auto Debug() const noexcept -> UnallocatedCString = 0;
    /// Returns the first nym for this seed, as a SeedTreeNym.
    virtual auto First() const noexcept -> SharedPimpl<SeedTreeNym> = 0;
    /// Returns the display name for this seed.
    virtual auto Name() const noexcept -> UnallocatedCString = 0;
    /// Returns the next nym for this seed, as a SeedTreeNym.
    virtual auto Next() const noexcept -> SharedPimpl<SeedTreeNym> = 0;
    /// Returns the ID for this seed.
    virtual auto SeedID() const noexcept -> crypto::SeedID = 0;
    /// Returns the seed type as an enum SeedStyle.
    virtual auto Type() const noexcept -> crypto::SeedStyle = 0;

    SeedTreeItem(const SeedTreeItem&) = delete;
    SeedTreeItem(SeedTreeItem&&) = delete;
    auto operator=(const SeedTreeItem&) -> SeedTreeItem& = delete;
    auto operator=(SeedTreeItem&&) -> SeedTreeItem& = delete;

    ~SeedTreeItem() override = default;

protected:
    SeedTreeItem() noexcept = default;
};
}  // namespace opentxs::ui
