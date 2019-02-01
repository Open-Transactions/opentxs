// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/Wallet.hpp"
#include "opentxs/client/NymData.hpp"
#include "opentxs/contact/ContactGroup.hpp"
#include "opentxs/contact/ContactItem.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/SubscribeSocket.hpp"
#include "opentxs/ui/ProfileItem.hpp"
#include "opentxs/ui/ProfileSubsection.hpp"

#include "InternalUI.hpp"
#include "List.hpp"
#include "ProfileItemBlank.hpp"
#include "RowType.hpp"

#include <map>
#include <memory>
#include <set>
#include <thread>
#include <tuple>
#include <vector>

#include "ProfileSubsection.hpp"

template class opentxs::SharedPimpl<opentxs::ui::ProfileSubsection>;

//#define OT_METHOD "opentxs::ui::implementation::ProfileSubsection::"

namespace opentxs
{
ui::implementation::ProfileSectionRowInternal* Factory::ProfileSubsectionWidget(
    const ui::implementation::ProfileSectionInternalInterface& parent,
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& publisher,
    const ui::implementation::ProfileSectionRowID& rowID,
    const ui::implementation::ProfileSectionSortKey& key,
    const ui::implementation::CustomData& custom)
{
    return new ui::implementation::ProfileSubsection(
        parent, api, publisher, rowID, key, custom);
}
}  // namespace opentxs

namespace opentxs::ui::implementation
{
ProfileSubsection::ProfileSubsection(
    const ProfileSectionInternalInterface& parent,
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& publisher,
    const ProfileSectionRowID& rowID,
    const ProfileSectionSortKey& key,
    const CustomData& custom)
    : ProfileSubsectionList(api, publisher, parent.NymID(), parent.WidgetID())
    , ProfileSubsectionRow(parent, rowID, true)
{
    init();
    startup_.reset(new std::thread(&ProfileSubsection::startup, this, custom));

    OT_ASSERT(startup_)
}

bool ProfileSubsection::AddItem(
    const std::string& value,
    const bool primary,
    const bool active) const
{
    return parent_.AddClaim(row_id_.second, value, primary, active);
}

void ProfileSubsection::construct_row(
    const ProfileSubsectionRowID& id,
    const ProfileSubsectionSortKey& index,
    const CustomData& custom) const
{
    OT_ASSERT(1 == custom.size())

    names_.emplace(id, index);
    items_[index].emplace(
        id,
        Factory::ProfileItemWidget(*this, api_, publisher_, id, index, custom));
}

bool ProfileSubsection::Delete(const std::string& claimID) const
{
    Lock lock(lock_);
    auto& claim = find_by_id(lock, Identifier::Factory(claimID));

    if (false == claim.Valid()) { return false; }

    return claim.Delete();
}

std::string ProfileSubsection::Name(const std::string& lang) const
{
    return proto::TranslateItemType(row_id_.second, lang);
}

std::set<ProfileSubsectionRowID> ProfileSubsection::process_group(
    const opentxs::ContactGroup& group)
{
    OT_ASSERT(row_id_.second == group.Type())

    std::set<ProfileSubsectionRowID> active{};

    for (const auto& [id, claim] : group) {
        OT_ASSERT(claim)

        CustomData custom{new opentxs::ContactItem(*claim)};
        add_item(id, sort_key(id), custom);
        active.emplace(id);
    }

    return active;
}

void ProfileSubsection::reindex(
    const ProfileSectionSortKey&,
    const CustomData& custom)
{
    delete_inactive(
        process_group(extract_custom<opentxs::ContactGroup>(custom)));
}

bool ProfileSubsection::SetActive(const std::string& claimID, const bool active)
    const
{
    Lock lock(lock_);
    auto& claim = find_by_id(lock, Identifier::Factory(claimID));

    if (false == claim.Valid()) { return false; }

    return claim.SetActive(active);
}

bool ProfileSubsection::SetPrimary(
    const std::string& claimID,
    const bool primary) const
{
    Lock lock(lock_);
    auto& claim = find_by_id(lock, Identifier::Factory(claimID));

    if (false == claim.Valid()) { return false; }

    return claim.SetPrimary(primary);
}

bool ProfileSubsection::SetValue(
    const std::string& claimID,
    const std::string& value) const
{
    Lock lock(lock_);
    auto& claim = find_by_id(lock, Identifier::Factory(claimID));

    if (false == claim.Valid()) { return false; }

    return claim.SetValue(value);
}

int ProfileSubsection::sort_key(const ProfileSubsectionRowID) const
{
    return static_cast<int>(items_.size());
}

void ProfileSubsection::startup(const CustomData& custom)
{
    process_group(extract_custom<opentxs::ContactGroup>(custom));
    startup_complete_->On();
}
}  // namespace opentxs::ui::implementation
