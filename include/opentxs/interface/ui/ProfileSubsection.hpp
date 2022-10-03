// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/interface/ui/List.hpp"
#include "opentxs/interface/ui/ListRow.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
class ProfileItem;
class ProfileSubsection;
}  // namespace ui

using OTUIProfileSubsection = SharedPimpl<ui::ProfileSubsection>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
/**
  This model represents a subsection of meta-data for a ProfileSection for the
  wallet user Profile. Each row is a ProfileItem containing metadata about the
  wallet user from his identity credentials.
*/
class OPENTXS_EXPORT ProfileSubsection : virtual public List,
                                         virtual public ListRow
{
public:
    /// Adds a new claim to this ProfileSubsection. Returns success or failure.
    virtual auto AddItem(
        const UnallocatedCString& value,
        const bool primary,
        const bool active) const noexcept -> bool = 0;
    /// Deletes a claim from this subsection of the user's credentials.
    virtual auto Delete(const UnallocatedCString& claimID) const noexcept
        -> bool = 0;
    /// Returns the first claim in this ProfileSubsection as a ProfileItem.
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ProfileItem> = 0;
    /// Returns the display name of this ProfileSubsection for a given language.
    virtual auto Name(const UnallocatedCString& lang) const noexcept
        -> UnallocatedCString = 0;
    /// Returns the next claim in this ProfileSubsection as a ProfileItem.
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ProfileItem> = 0;
    /// Used to set a given claim in this subsection as 'active' or 'inactive'.
    virtual auto SetActive(const UnallocatedCString& claimID, const bool active)
        const noexcept -> bool = 0;
    /// Used to set a given claim in this subsection as 'primary' or 'not
    /// primary'.
    virtual auto SetPrimary(
        const UnallocatedCString& claimID,
        const bool primary) const noexcept -> bool = 0;
    /// Sets the value for a given claimID in this subsection of the user's
    /// credentials.
    virtual auto SetValue(
        const UnallocatedCString& claimID,
        const UnallocatedCString& value) const noexcept -> bool = 0;
    /// Returns the ClaimType for this subsection of the user's credentials. All
    /// claims in this subsection are the same type.
    virtual auto Type() const noexcept -> identity::wot::claim::ClaimType = 0;

    ProfileSubsection(const ProfileSubsection&) = delete;
    ProfileSubsection(ProfileSubsection&&) = delete;
    auto operator=(const ProfileSubsection&) -> ProfileSubsection& = delete;
    auto operator=(ProfileSubsection&&) -> ProfileSubsection& = delete;

    ~ProfileSubsection() override = default;

protected:
    ProfileSubsection() noexcept = default;
};
}  // namespace opentxs::ui
