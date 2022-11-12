// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "interface/ui/profile/ProfileSubsection.hpp"  // IWYU pragma: associated

#include <memory>
#include <string_view>
#include <thread>

#include "interface/ui/base/Combined.hpp"
#include "interface/ui/base/Widget.hpp"
#include "internal/identity/wot/claim/Types.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/serialization/protobuf/verify/VerifyContacts.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/identity/wot/claim/Group.hpp"
#include "opentxs/identity/wot/claim/Item.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::factory
{
auto ProfileSubsectionWidget(
    const ui::implementation::ProfileSectionInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::ProfileSectionRowID& rowID,
    const ui::implementation::ProfileSectionSortKey& key,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::ProfileSectionRowInternal>
{
    using ReturnType = ui::implementation::ProfileSubsection;

    return std::make_shared<ReturnType>(parent, api, rowID, key, custom);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
ProfileSubsection::ProfileSubsection(
    const ProfileSectionInternalInterface& parent,
    const api::session::Client& api,
    const ProfileSectionRowID& rowID,
    const ProfileSectionSortKey& key,
    CustomData& custom) noexcept
    : Combined(api, parent.NymID(), parent.WidgetID(), parent, rowID, key)
    , api_(api)
    , sequence_(-1)
{
    startup_ = std::make_unique<std::thread>(
        &ProfileSubsection::startup,
        this,
        extract_custom<identity::wot::claim::Group>(custom));

    OT_ASSERT(startup_);
}

auto ProfileSubsection::AddItem(
    const UnallocatedCString& value,
    const bool primary,
    const bool active) const noexcept -> bool
{
    return parent_.AddClaim(row_id_.second, value, primary, active);
}

auto ProfileSubsection::construct_row(
    const ProfileSubsectionRowID& id,
    const ProfileSubsectionSortKey& index,
    CustomData& custom) const noexcept -> RowPointer
{
    return factory::ProfileItemWidget(*this, api_, id, index, custom);
}

auto ProfileSubsection::Delete(const UnallocatedCString& claimID) const noexcept
    -> bool
{
    rLock lock{recursive_lock_};
    const auto& claim =
        lookup(lock, api_.Factory().IdentifierFromBase58(claimID));

    if (false == claim.Valid()) { return false; }

    return claim.Delete();
}

auto ProfileSubsection::Name(const UnallocatedCString& lang) const noexcept
    -> UnallocatedCString
{
    return UnallocatedCString{
        proto::TranslateItemType(translate(row_id_.second), lang)};
}

auto ProfileSubsection::process_group(
    const identity::wot::claim::Group& group) noexcept
    -> UnallocatedSet<ProfileSubsectionRowID>
{
    OT_ASSERT(row_id_.second == group.Type());

    UnallocatedSet<ProfileSubsectionRowID> active{};

    for (const auto& [id, claim] : group) {
        OT_ASSERT(claim);

        CustomData custom{new identity::wot::claim::Item(*claim)};
        add_item(id, ++sequence_, custom);
        active.emplace(id);
    }

    return active;
}

auto ProfileSubsection::reindex(
    const ProfileSectionSortKey&,
    CustomData& custom) noexcept -> bool
{
    delete_inactive(
        process_group(extract_custom<identity::wot::claim::Group>(custom)));

    return true;
}

auto ProfileSubsection::SetActive(
    const UnallocatedCString& claimID,
    const bool active) const noexcept -> bool
{
    rLock lock{recursive_lock_};
    const auto& claim =
        lookup(lock, api_.Factory().IdentifierFromBase58(claimID));

    if (false == claim.Valid()) { return false; }

    return claim.SetActive(active);
}

auto ProfileSubsection::SetPrimary(
    const UnallocatedCString& claimID,
    const bool primary) const noexcept -> bool
{
    rLock lock{recursive_lock_};
    const auto& claim =
        lookup(lock, api_.Factory().IdentifierFromBase58(claimID));

    if (false == claim.Valid()) { return false; }

    return claim.SetPrimary(primary);
}

auto ProfileSubsection::SetValue(
    const UnallocatedCString& claimID,
    const UnallocatedCString& value) const noexcept -> bool
{
    rLock lock{recursive_lock_};
    const auto& claim =
        lookup(lock, api_.Factory().IdentifierFromBase58(claimID));

    if (false == claim.Valid()) { return false; }

    return claim.SetValue(value);
}

void ProfileSubsection::startup(
    const identity::wot::claim::Group group) noexcept
{
    process_group(group);
    finish_startup();
}
}  // namespace opentxs::ui::implementation
