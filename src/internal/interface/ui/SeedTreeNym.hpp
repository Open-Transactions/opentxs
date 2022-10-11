// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>

#include "internal/interface/ui/ListRow.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::ui
{
/**
 This model represents one of the nyms available for a seed in this wallet.
 Each of the rows in the SeedTreeItem model is a different SeedTreeNym.
 */
class SeedTreeNym : virtual public ListRow
{
public:
    /// Returns the index for this Nym.
    virtual auto Index() const noexcept -> std::size_t = 0;
    /// Returns the display name for this Nym.
    virtual auto Name() const noexcept -> UnallocatedCString = 0;
    /// Returns the NymID for this Nym.
    virtual auto NymID() const noexcept -> UnallocatedCString = 0;

    SeedTreeNym(const SeedTreeNym&) = delete;
    SeedTreeNym(SeedTreeNym&&) = delete;
    auto operator=(const SeedTreeNym&) -> SeedTreeNym& = delete;
    auto operator=(SeedTreeNym&&) -> SeedTreeNym& = delete;

    ~SeedTreeNym() override = default;

protected:
    SeedTreeNym() noexcept = default;
};
}  // namespace opentxs::ui
