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
namespace identifier
{
class Generic;
class Nym;
}  // namespace identifier

namespace ui
{
class SeedTree;
class SeedTreeItem;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
/**
 This model represents the list of seeds available in this wallet.
 */
class SeedTree : virtual public List
{
public:
    /// Returns debug information about the seeds in the wallet.
    virtual auto Debug() const noexcept -> UnallocatedCString = 0;
    /// Returns the ID for the wallet's default Nym.
    virtual auto DefaultNym() const noexcept -> identifier::Nym = 0;
    /// Returns the ID for the wallet's default Seed.
    virtual auto DefaultSeed() const noexcept -> identifier::Generic = 0;

    /// Returns the first SeedTreeItem row.
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<SeedTreeItem> = 0;
    /// Returns the next SeedTreeItem row.
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<SeedTreeItem> = 0;

    SeedTree(const SeedTree&) = delete;
    SeedTree(SeedTree&&) = delete;
    auto operator=(const SeedTree&) -> SeedTree& = delete;
    auto operator=(SeedTree&&) -> SeedTree& = delete;

    ~SeedTree() override = default;

protected:
    SeedTree() noexcept = default;
};
}  // namespace opentxs::ui
