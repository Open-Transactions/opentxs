// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "ListRow.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
class ProfileItem;
}  // namespace ui

using OTUIProfileItem = SharedPimpl<ui::ProfileItem>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
/**
 This model represents a single claim on the wallet user's identity credentials.
 Each of these claims is a single row on the ProfileSubsection model.
 */
class ProfileItem : virtual public ListRow
{
public:
    /// Returns the ID for this claim.
    virtual auto ClaimID() const noexcept -> UnallocatedCString = 0;
    /// Deletes this claim from the user's credentials. Returns success or
    /// failure.
    virtual auto Delete() const noexcept -> bool = 0;
    /// Indicates whether or not this claim is active.
    virtual auto IsActive() const noexcept -> bool = 0;
    /// Indicates whether or not this claim is primary.
    virtual auto IsPrimary() const noexcept -> bool = 0;
    /// Sets this claim as 'active' or 'inactive' on the wallet user's
    /// credentials.
    virtual auto SetActive(const bool& active) const noexcept -> bool = 0;
    /// Sets this claim as 'primary' or 'not primary' on the wallet user's
    /// credentials.
    virtual auto SetPrimary(const bool& primary) const noexcept -> bool = 0;
    // Sets the value of this claim.
    virtual auto SetValue(const UnallocatedCString& value) const noexcept
        -> bool = 0;
    /// Returns the value of this claim.
    virtual auto Value() const noexcept -> UnallocatedCString = 0;

    ProfileItem(const ProfileItem&) = delete;
    ProfileItem(ProfileItem&&) = delete;
    auto operator=(const ProfileItem&) -> ProfileItem& = delete;
    auto operator=(ProfileItem&&) -> ProfileItem& = delete;

    ~ProfileItem() override = default;

protected:
    ProfileItem() noexcept = default;
};
}  // namespace opentxs::ui
