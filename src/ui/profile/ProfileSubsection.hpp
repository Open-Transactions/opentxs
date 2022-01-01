// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <iosfwd>
#include <list>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include "1_Internal.hpp"
#include "Proto.hpp"
#include "internal/ui/UI.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/contact/ClaimType.hpp"
#include "opentxs/contact/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/SharedPimpl.hpp"
#include "ui/base/Combined.hpp"
#include "ui/base/List.hpp"
#include "ui/base/RowType.hpp"

namespace opentxs
{
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace contact
{
class ContactGroup;
}  // namespace contact

namespace identifier
{
class Nym;
}  // namespace identifier

namespace network
{
namespace zeromq
{
namespace socket
{
class Publish;
}  // namespace socket
}  // namespace zeromq
}  // namespace network

namespace ui
{
class ProfileSubsection;
}  // namespace ui
}  // namespace opentxs

namespace opentxs::ui::implementation
{
using ProfileSubsectionList = List<
    ProfileSubsectionExternalInterface,
    ProfileSubsectionInternalInterface,
    ProfileSubsectionRowID,
    ProfileSubsectionRowInterface,
    ProfileSubsectionRowInternal,
    ProfileSubsectionRowBlank,
    ProfileSubsectionSortKey,
    ProfileSubsectionPrimaryID>;
using ProfileSubsectionRow = RowType<
    ProfileSectionRowInternal,
    ProfileSectionInternalInterface,
    ProfileSectionRowID>;

class ProfileSubsection final : public Combined<
                                    ProfileSubsectionList,
                                    ProfileSubsectionRow,
                                    ProfileSectionSortKey>
{
public:
    auto AddItem(
        const std::string& value,
        const bool primary,
        const bool active) const noexcept -> bool final;
    auto Delete(const std::string& claimID) const noexcept -> bool final;
    auto Name(const std::string& lang) const noexcept -> std::string final;
    auto NymID() const noexcept -> const identifier::Nym& final
    {
        return primary_id_;
    }
    auto Section() const noexcept -> contact::SectionType final
    {
        return row_id_.first;
    }
    auto SetActive(const std::string& claimID, const bool active) const noexcept
        -> bool final;
    auto SetPrimary(const std::string& claimID, const bool primary)
        const noexcept -> bool final;
    auto SetValue(const std::string& claimID, const std::string& value)
        const noexcept -> bool final;
    auto Type() const noexcept -> contact::ClaimType final
    {
        return row_id_.second;
    }

    ProfileSubsection(
        const ProfileSectionInternalInterface& parent,
        const api::session::Client& api,
        const ProfileSectionRowID& rowID,
        const ProfileSectionSortKey& key,
        CustomData& custom) noexcept;
    ~ProfileSubsection() final = default;

private:
    ProfileSectionSortKey sequence_;

    static auto check_type(const ProfileSubsectionRowID type) -> bool;

    auto construct_row(
        const ProfileSubsectionRowID& id,
        const ProfileSubsectionSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;

    auto last(const ProfileSubsectionRowID& id) const noexcept -> bool final
    {
        return ProfileSubsectionList::last(id);
    }
    auto process_group(const contact::ContactGroup& group) noexcept
        -> std::pmr::set<ProfileSubsectionRowID>;
    auto reindex(const ProfileSectionSortKey& key, CustomData& custom) noexcept
        -> bool final;
    auto startup(const contact::ContactGroup group) noexcept -> void;

    ProfileSubsection() = delete;
    ProfileSubsection(const ProfileSubsection&) = delete;
    ProfileSubsection(ProfileSubsection&&) = delete;
    auto operator=(const ProfileSubsection&) -> ProfileSubsection& = delete;
    auto operator=(ProfileSubsection&&) -> ProfileSubsection& = delete;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::ProfileSubsection>;
