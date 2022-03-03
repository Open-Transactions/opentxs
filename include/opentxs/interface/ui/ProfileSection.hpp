// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/interface/ui/List.hpp"
#include "opentxs/interface/ui/ListRow.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace ui
{
class ProfileSection;
class ProfileSubsection;
}  // namespace ui

using OTUIProfileSection = SharedPimpl<ui::ProfileSection>;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui
{
class OPENTXS_EXPORT ProfileSection : virtual public List,
                                      virtual public ListRow
{
public:
    using ItemType =
        std::pair<identity::wot::claim::ClaimType, UnallocatedCString>;
    using ItemTypeList = UnallocatedVector<ItemType>;

    static auto AllowedItems(
        const identity::wot::claim::SectionType section,
        const UnallocatedCString& lang) noexcept -> ItemTypeList;

    virtual auto AddClaim(
        const identity::wot::claim::ClaimType type,
        const UnallocatedCString& value,
        const bool primary,
        const bool active) const noexcept -> bool = 0;
    virtual auto Delete(const int type, const UnallocatedCString& claimID)
        const noexcept -> bool = 0;
    virtual auto Items(const UnallocatedCString& lang) const noexcept
        -> ItemTypeList = 0;
    virtual auto Name(const UnallocatedCString& lang) const noexcept
        -> UnallocatedCString = 0;
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ProfileSubsection> = 0;
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ProfileSubsection> = 0;
    virtual auto SetActive(
        const int type,
        const UnallocatedCString& claimID,
        const bool active) const noexcept -> bool = 0;
    virtual auto SetPrimary(
        const int type,
        const UnallocatedCString& claimID,
        const bool primary) const noexcept -> bool = 0;
    virtual auto SetValue(
        const int type,
        const UnallocatedCString& claimID,
        const UnallocatedCString& value) const noexcept -> bool = 0;
    virtual auto Type() const noexcept -> identity::wot::claim::SectionType = 0;

    ~ProfileSection() override = default;

protected:
    ProfileSection() noexcept = default;

private:
    ProfileSection(const ProfileSection&) = delete;
    ProfileSection(ProfileSection&&) = delete;
    auto operator=(const ProfileSection&) -> ProfileSection& = delete;
    auto operator=(ProfileSection&&) -> ProfileSection& = delete;
};
}  // namespace opentxs::ui
