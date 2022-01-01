// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <string>

#include "opentxs/contact/Types.hpp"
#include "opentxs/ui/List.hpp"
#include "opentxs/ui/ListRow.hpp"
#include "opentxs/util/SharedPimpl.hpp"

namespace opentxs
{
namespace ui
{
class ProfileSection;
class ProfileSubsection;
}  // namespace ui

using OTUIProfileSection = SharedPimpl<ui::ProfileSection>;
}  // namespace opentxs

namespace opentxs
{
namespace ui
{
class OPENTXS_EXPORT ProfileSection : virtual public List,
                                      virtual public ListRow
{
public:
    using ItemType = std::pair<contact::ClaimType, std::string>;
    using ItemTypeList = std::pmr::vector<ItemType>;

    static auto AllowedItems(
        const contact::SectionType section,
        const std::string& lang) noexcept -> ItemTypeList;

    virtual auto AddClaim(
        const contact::ClaimType type,
        const std::string& value,
        const bool primary,
        const bool active) const noexcept -> bool = 0;
    virtual auto Delete(const int type, const std::string& claimID)
        const noexcept -> bool = 0;
    virtual auto Items(const std::string& lang) const noexcept
        -> ItemTypeList = 0;
    virtual auto Name(const std::string& lang) const noexcept
        -> std::string = 0;
    virtual auto First() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ProfileSubsection> = 0;
    virtual auto Next() const noexcept
        -> opentxs::SharedPimpl<opentxs::ui::ProfileSubsection> = 0;
    virtual auto SetActive(
        const int type,
        const std::string& claimID,
        const bool active) const noexcept -> bool = 0;
    virtual auto SetPrimary(
        const int type,
        const std::string& claimID,
        const bool primary) const noexcept -> bool = 0;
    virtual auto SetValue(
        const int type,
        const std::string& claimID,
        const std::string& value) const noexcept -> bool = 0;
    virtual auto Type() const noexcept -> contact::SectionType = 0;

    ~ProfileSection() override = default;

protected:
    ProfileSection() noexcept = default;

private:
    ProfileSection(const ProfileSection&) = delete;
    ProfileSection(ProfileSection&&) = delete;
    auto operator=(const ProfileSection&) -> ProfileSection& = delete;
    auto operator=(ProfileSection&&) -> ProfileSection& = delete;
};
}  // namespace ui
}  // namespace opentxs
