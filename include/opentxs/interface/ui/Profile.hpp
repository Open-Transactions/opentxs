// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <tuple>

#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/interface/ui/List.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace ui
{
class Profile;
class ProfileSection;
}  // namespace ui
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT Profile : virtual public List
{
public:
    using ItemType =
        std::pair<identity::wot::claim::ClaimType, UnallocatedCString>;
    using ItemTypeList = UnallocatedVector<ItemType>;
    using SectionType =
        std::pair<identity::wot::claim::SectionType, UnallocatedCString>;
    using SectionTypeList = UnallocatedVector<SectionType>;

    virtual auto AddClaim(
        const identity::wot::claim::SectionType section,
        const identity::wot::claim::ClaimType type,
        const UnallocatedCString& value,
        const bool primary,
        const bool active) const noexcept -> bool = 0;
    virtual auto AllowedItems(
        const identity::wot::claim::SectionType section,
        const UnallocatedCString& lang) const noexcept -> ItemTypeList = 0;
    virtual auto AllowedSections(const UnallocatedCString& lang) const noexcept
        -> SectionTypeList = 0;
    virtual auto Delete(
        const int section,
        const int type,
        const UnallocatedCString& claimID) const noexcept -> bool = 0;
    virtual auto DisplayName() const noexcept -> UnallocatedCString = 0;
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ProfileSection> = 0;
    virtual auto ID() const noexcept -> UnallocatedCString = 0;
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ProfileSection> = 0;
    virtual auto PaymentCode() const noexcept -> UnallocatedCString = 0;
    virtual auto SetActive(
        const int section,
        const int type,
        const UnallocatedCString& claimID,
        const bool active) const noexcept -> bool = 0;
    virtual auto SetPrimary(
        const int section,
        const int type,
        const UnallocatedCString& claimID,
        const bool primary) const noexcept -> bool = 0;
    virtual auto SetValue(
        const int section,
        const int type,
        const UnallocatedCString& claimID,
        const UnallocatedCString& value) const noexcept -> bool = 0;

    ~Profile() override = default;

protected:
    Profile() noexcept = default;

private:
    Profile(const Profile&) = delete;
    Profile(Profile&&) = delete;
    auto operator=(const Profile&) -> Profile& = delete;
    auto operator=(Profile&&) -> Profile& = delete;
};
}  // namespace opentxs::ui
