// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::identity::wot::claim::ClaimType
// IWYU pragma: no_forward_declare opentxs::identity::wot::claim::SectionType

#pragma once

#include <iosfwd>
#include <utility>

#include "interface/ui/base/Combined.hpp"
#include "interface/ui/base/List.hpp"
#include "interface/ui/base/RowType.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/Pimpl.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
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
class Group;
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

namespace ui
{
class ProfileSubsection;
}  // namespace ui
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

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
    const api::session::Client& api_;

    auto AddItem(
        const UnallocatedCString& value,
        const bool primary,
        const bool active) const noexcept -> bool final;
    auto API() const noexcept -> const api::Session& final { return api_; }
    auto Delete(const UnallocatedCString& claimID) const noexcept -> bool final;
    auto Name(const UnallocatedCString& lang) const noexcept
        -> UnallocatedCString final;
    auto NymID() const noexcept -> const identifier::Nym& final
    {
        return primary_id_;
    }
    auto Section() const noexcept -> identity::wot::claim::SectionType final
    {
        return row_id_.first;
    }
    auto SetActive(const UnallocatedCString& claimID, const bool active)
        const noexcept -> bool final;
    auto SetPrimary(const UnallocatedCString& claimID, const bool primary)
        const noexcept -> bool final;
    auto SetValue(
        const UnallocatedCString& claimID,
        const UnallocatedCString& value) const noexcept -> bool final;
    auto Type() const noexcept -> identity::wot::claim::ClaimType final
    {
        return row_id_.second;
    }

    ProfileSubsection(
        const ProfileSectionInternalInterface& parent,
        const api::session::Client& api,
        const ProfileSectionRowID& rowID,
        const ProfileSectionSortKey& key,
        CustomData& custom) noexcept;
    ProfileSubsection() = delete;
    ProfileSubsection(const ProfileSubsection&) = delete;
    ProfileSubsection(ProfileSubsection&&) = delete;
    auto operator=(const ProfileSubsection&) -> ProfileSubsection& = delete;
    auto operator=(ProfileSubsection&&) -> ProfileSubsection& = delete;

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
    auto process_group(const identity::wot::claim::Group& group) noexcept
        -> UnallocatedSet<ProfileSubsectionRowID>;
    auto reindex(const ProfileSectionSortKey& key, CustomData& custom) noexcept
        -> bool final;
    auto startup(const identity::wot::claim::Group group) noexcept -> void;
};
}  // namespace opentxs::ui::implementation

template class opentxs::SharedPimpl<opentxs::ui::ProfileSubsection>;
