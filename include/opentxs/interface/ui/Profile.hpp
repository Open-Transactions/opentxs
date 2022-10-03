// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <algorithm>
#include <tuple>

#include "opentxs/Export.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/interface/ui/List.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace ui
{
class Profile;
class ProfileSection;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
/**
 This model contains the wallet user's Profile data, and also has methods for
 manipulating that data. This includes the claims on this user's identity
 credentials.
 */
class OPENTXS_EXPORT Profile : virtual public List
{
public:
    using ItemType =
        std::pair<identity::wot::claim::ClaimType, UnallocatedCString>;
    using ItemTypeList = UnallocatedVector<ItemType>;
    using SectionType =
        std::pair<identity::wot::claim::SectionType, UnallocatedCString>;
    using SectionTypeList = UnallocatedVector<SectionType>;

    /// Adds a new claim to the user's credentials. Returns success or failure.
    virtual auto AddClaim(
        const identity::wot::claim::SectionType section,
        const identity::wot::claim::ClaimType type,
        const UnallocatedCString& value,
        const bool primary,
        const bool active) const noexcept -> bool = 0;
    /// Returns a list of allowed item types for a given section and language.
    virtual auto AllowedItems(
        const identity::wot::claim::SectionType section,
        const UnallocatedCString& lang) const noexcept -> ItemTypeList = 0;
    /// Returns a list of allowed sections for a given language.
    virtual auto AllowedSections(const UnallocatedCString& lang) const noexcept
        -> SectionTypeList = 0;
    /// Deletes a claim from the user's credentials. Returns success or failure.
    virtual auto Delete(
        const int section,
        const int type,
        const UnallocatedCString& claimID) const noexcept -> bool = 0;
    /// Returns the display name for the user's profile in this wallet.
    virtual auto DisplayName() const noexcept -> UnallocatedCString = 0;
    /// Returns the first ProfileSection for this profile, containing metadata
    /// about the wallet user.
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ProfileSection> = 0;
    /// Returns the profile ID for the wallet user.
    virtual auto ID() const noexcept -> UnallocatedCString = 0;
    /// Returns the next ProfileSection for this profile, containing metadata
    /// about the wallet user.
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ProfileSection> = 0;
    /// Returns the payment code for the wallet user.
    virtual auto PaymentCode() const noexcept -> UnallocatedCString = 0;
    /// Sets a given claim as 'active' or 'inactive' in the user's credentials.
    virtual auto SetActive(
        const int section,
        const int type,
        const UnallocatedCString& claimID,
        const bool active) const noexcept -> bool = 0;
    /// Sets a given claim as 'primary' or 'not primary' in the user's
    /// credentials. (Such as primary display name).
    virtual auto SetPrimary(
        const int section,
        const int type,
        const UnallocatedCString& claimID,
        const bool primary) const noexcept -> bool = 0;
    /// Sets the value for a given claim in this user's credentials.
    virtual auto SetValue(
        const int section,
        const int type,
        const UnallocatedCString& claimID,
        const UnallocatedCString& value) const noexcept -> bool = 0;

    Profile(const Profile&) = delete;
    Profile(Profile&&) = delete;
    auto operator=(const Profile&) -> Profile& = delete;
    auto operator=(Profile&&) -> Profile& = delete;

    ~Profile() override = default;

protected:
    Profile() noexcept = default;
};
}  // namespace opentxs::ui
