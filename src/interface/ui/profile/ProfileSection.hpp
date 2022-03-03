// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <iosfwd>
#include <utility>

#include "1_Internal.hpp"
#include "Proto.hpp"
#include "interface/ui/base/Combined.hpp"
#include "interface/ui/base/List.hpp"
#include "interface/ui/base/RowType.hpp"
#include "internal/interface/ui/UI.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"
#include "opentxs/identity/wot/claim/SectionType.hpp"
#include "opentxs/interface/ui/ProfileSection.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/SharedPimpl.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace api
{
namespace session
{
class Client;
}  // namespace session
}  // namespace api

namespace identifier
{
class Nym;
}  // namespace identifier

namespace identity
{
namespace wot
{
namespace claim
{
class Section;
}  // namespace claim
}  // namespace wot
}  // namespace identity

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
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::ui::implementation
{
using ProfileSectionList = List<
    ProfileSectionExternalInterface,
    ProfileSectionInternalInterface,
    ProfileSectionRowID,
    ProfileSectionRowInterface,
    ProfileSectionRowInternal,
    ProfileSectionRowBlank,
    ProfileSectionSortKey,
    ProfileSectionPrimaryID>;
using ProfileSectionRow =
    RowType<ProfileRowInternal, ProfileInternalInterface, ProfileRowID>;

class ProfileSection final
    : public Combined<ProfileSectionList, ProfileSectionRow, ProfileSortKey>
{
public:
    auto AddClaim(
        const identity::wot::claim::ClaimType type,
        const UnallocatedCString& value,
        const bool primary,
        const bool active) const noexcept -> bool final;
    auto Delete(const int type, const UnallocatedCString& claimID)
        const noexcept -> bool final;
    auto Items(const UnallocatedCString& lang) const noexcept
        -> ItemTypeList final;
    auto Name(const UnallocatedCString& lang) const noexcept
        -> UnallocatedCString final;
    auto NymID() const noexcept -> const identifier::Nym& final
    {
        return primary_id_;
    }
    auto SetActive(
        const int type,
        const UnallocatedCString& claimID,
        const bool active) const noexcept -> bool final;
    auto SetPrimary(
        const int type,
        const UnallocatedCString& claimID,
        const bool primary) const noexcept -> bool final;
    auto SetValue(
        const int type,
        const UnallocatedCString& claimID,
        const UnallocatedCString& value) const noexcept -> bool final;
    auto Type() const noexcept -> identity::wot::claim::SectionType final
    {
        return row_id_;
    }

    ProfileSection(
        const ProfileInternalInterface& parent,
        const api::session::Client& api,
        const ProfileRowID& rowID,
        const ProfileSortKey& key,
        CustomData& custom) noexcept;
    ~ProfileSection() final = default;

private:
    static auto sort_key(const ProfileSectionRowID type) noexcept -> int;
    static auto check_type(const ProfileSectionRowID type) noexcept -> bool;

    auto construct_row(
        const ProfileSectionRowID& id,
        const ProfileSectionSortKey& index,
        CustomData& custom) const noexcept -> RowPointer final;

    auto last(const ProfileSectionRowID& id) const noexcept -> bool final
    {
        return ProfileSectionList::last(id);
    }
    auto process_section(const identity::wot::claim::Section& section) noexcept
        -> UnallocatedSet<ProfileSectionRowID>;

    auto reindex(const ProfileSortKey& key, CustomData& custom) noexcept
        -> bool final;
    auto startup(const identity::wot::claim::Section section) noexcept -> void;

    ProfileSection() = delete;
    ProfileSection(const ProfileSection&) = delete;
    ProfileSection(ProfileSection&&) = delete;
    auto operator=(const ProfileSection&) -> ProfileSection& = delete;
    auto operator=(ProfileSection&&) -> ProfileSection& = delete;
};
}  // namespace opentxs::ui::implementation
