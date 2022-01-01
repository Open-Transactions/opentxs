// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <string>
#include <tuple>
#include <vector>

#include "opentxs/contact/Types.hpp"
#include "opentxs/ui/List.hpp"
#include "opentxs/util/SharedPimpl.hpp"

namespace opentxs
{
namespace ui
{
class Profile;
class ProfileSection;
}  // namespace ui
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class OPENTXS_EXPORT Profile : virtual public List
{
public:
    using ItemType = std::pair<contact::ClaimType, std::string>;
    using ItemTypeList = std::pmr::vector<ItemType>;
    using SectionType = std::pair<contact::SectionType, std::string>;
    using SectionTypeList = std::pmr::vector<SectionType>;

    virtual auto AddClaim(
        const contact::SectionType section,
        const contact::ClaimType type,
        const std::string& value,
        const bool primary,
        const bool active) const noexcept -> bool = 0;
    virtual auto AllowedItems(
        const contact::SectionType section,
        const std::string& lang) const noexcept -> ItemTypeList = 0;
    virtual auto AllowedSections(const std::string& lang) const noexcept
        -> SectionTypeList = 0;
    virtual auto Delete(
        const int section,
        const int type,
        const std::string& claimID) const noexcept -> bool = 0;
    virtual auto DisplayName() const noexcept -> std::string = 0;
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ProfileSection> = 0;
    virtual auto ID() const noexcept -> std::string = 0;
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ProfileSection> = 0;
    virtual auto PaymentCode() const noexcept -> std::string = 0;
    virtual auto SetActive(
        const int section,
        const int type,
        const std::string& claimID,
        const bool active) const noexcept -> bool = 0;
    virtual auto SetPrimary(
        const int section,
        const int type,
        const std::string& claimID,
        const bool primary) const noexcept -> bool = 0;
    virtual auto SetValue(
        const int section,
        const int type,
        const std::string& claimID,
        const std::string& value) const noexcept -> bool = 0;

    ~Profile() override = default;

protected:
    Profile() noexcept = default;

private:
    Profile(const Profile&) = delete;
    Profile(Profile&&) = delete;
    auto operator=(const Profile&) -> Profile& = delete;
    auto operator=(Profile&&) -> Profile& = delete;
};
}  // namespace ui
}  // namespace opentxs
