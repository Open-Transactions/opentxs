// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/interface/ui/List.hpp"
#include "opentxs/interface/ui/ListRow.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
class ProfileSection;
class ProfileSubsection;
}  // namespace ui

using OTUIProfileSection = SharedPimpl<ui::ProfileSection>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
/**
  This model represents a section of meta-data for the wallet user.
  Each row is a ProfileSubsection containing metadata about the wallet user from
  his identity credentials.
*/
class OPENTXS_EXPORT ProfileSection : virtual public List,
                                      virtual public ListRow
{
public:
    using ItemType =
        std::pair<identity::wot::claim::ClaimType, UnallocatedCString>;
    using ItemTypeList = UnallocatedVector<ItemType>;

    /// Returns a list of allowed item types for a given ProfileSection and
    /// language.
    static auto AllowedItems(
        const identity::wot::claim::SectionType section,
        const UnallocatedCString& lang) noexcept -> ItemTypeList;

    /// Used for adding a claim to this section of the user's credentials.
    virtual auto AddClaim(
        const identity::wot::claim::ClaimType type,
        const UnallocatedCString& value,
        const bool primary,
        const bool active) const noexcept -> bool = 0;
    /// Used for deleting a claim from this section of the user's credentials.
    virtual auto Delete(const int type, const UnallocatedCString& claimID)
        const noexcept -> bool = 0;
    /// Returns a list of item types for a given language from this section of
    /// the user's credentials.
    virtual auto Items(const UnallocatedCString& lang) const noexcept
        -> ItemTypeList = 0;
    /// Returns the display name for this section of the user's credentials.
    virtual auto Name(const UnallocatedCString& lang) const noexcept
        -> UnallocatedCString = 0;
    /// Returns the first ProfileSubsection for this section of the user's
    /// credentials.
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ProfileSubsection> = 0;
    /// Returns the next ProfileSubsection for this section of the user's
    /// credentials.
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ProfileSubsection> = 0;
    /// Sets a given claim as 'active' or 'inactive' in this section of the
    /// user's credentials.
    virtual auto SetActive(
        const int type,
        const UnallocatedCString& claimID,
        const bool active) const noexcept -> bool = 0;
    /// Sets a given claim as 'primary' or 'not primary' in this section of the
    /// user's credentials.
    virtual auto SetPrimary(
        const int type,
        const UnallocatedCString& claimID,
        const bool primary) const noexcept -> bool = 0;
    /// Sets the contents for a given claim in this section of the user's
    /// credentials.
    virtual auto SetValue(
        const int type,
        const UnallocatedCString& claimID,
        const UnallocatedCString& value) const noexcept -> bool = 0;
    /// Returns the SectionType for this ProfileSection.
    virtual auto Type() const noexcept -> identity::wot::claim::SectionType = 0;

    ProfileSection(const ProfileSection&) = delete;
    ProfileSection(ProfileSection&&) = delete;
    auto operator=(const ProfileSection&) -> ProfileSection& = delete;
    auto operator=(ProfileSection&&) -> ProfileSection& = delete;

    ~ProfileSection() override = default;

protected:
    ProfileSection() noexcept = default;
};
}  // namespace opentxs::ui
