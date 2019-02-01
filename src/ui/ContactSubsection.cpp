// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Contacts.hpp"
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
#include "opentxs/ui/ContactItem.hpp"
#include "opentxs/ui/ContactSubsection.hpp"

#include "ContactItemBlank.hpp"
#include "InternalUI.hpp"
#include "List.hpp"
#include "RowType.hpp"

#include <map>
#include <memory>
#include <set>
#include <thread>
#include <tuple>
#include <vector>

#include "ContactSubsection.hpp"

template class opentxs::SharedPimpl<opentxs::ui::ContactSubsection>;

//#define OT_METHOD "opentxs::ui::implementation::ContactSubsection::"

namespace opentxs
{
ui::implementation::ContactSectionRowInternal* Factory::ContactSubsectionWidget(
    const ui::implementation::ContactSectionInternalInterface& parent,
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& publisher,
    const ui::implementation::ContactSectionRowID& rowID,
    const ui::implementation::ContactSectionSortKey& key,
    const ui::implementation::CustomData& custom)
{
    return new ui::implementation::ContactSubsection(
        parent, api, publisher, rowID, key, custom);
}
}  // namespace opentxs

namespace opentxs::ui::implementation
{
ContactSubsection::ContactSubsection(
    const ContactSectionInternalInterface& parent,
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& publisher,
    const ContactSectionRowID& rowID,
    const ContactSectionSortKey& key,
    const CustomData& custom)
    : ContactSubsectionList(
          api,
          publisher,
          identifier::Nym::Factory(parent.ContactID()),
          parent.WidgetID())
    , ContactSubsectionRow(parent, rowID, true)
{
    init();
    startup_.reset(new std::thread(&ContactSubsection::startup, this, custom));

    OT_ASSERT(startup_)
}

void ContactSubsection::construct_row(
    const ContactSubsectionRowID& id,
    const ContactSubsectionSortKey& index,
    const CustomData& custom) const
{
    OT_ASSERT(1 == custom.size())

    names_.emplace(id, index);
    items_[index].emplace(
        id,
        Factory::ContactItemWidget(*this, api_, publisher_, id, index, custom));
}

std::string ContactSubsection::Name(const std::string& lang) const
{
    return proto::TranslateItemType(row_id_.second, lang);
}

std::set<ContactSubsectionRowID> ContactSubsection::process_group(
    const opentxs::ContactGroup& group)
{
    OT_ASSERT(row_id_.second == group.Type())

    std::set<ContactSubsectionRowID> active{};

    for (const auto& [id, claim] : group) {
        OT_ASSERT(claim)

        CustomData custom{new opentxs::ContactItem(*claim)};
        add_item(id, sort_key(id), custom);
        active.emplace(id);
    }

    return active;
}

void ContactSubsection::reindex(
    const ContactSectionSortKey&,
    const CustomData& custom)
{
    delete_inactive(
        process_group(extract_custom<opentxs::ContactGroup>(custom)));
}

int ContactSubsection::sort_key(const ContactSubsectionRowID) const
{
    return static_cast<int>(items_.size());
}

void ContactSubsection::startup(const CustomData custom)
{
    process_group(extract_custom<opentxs::ContactGroup>(custom));
    startup_complete_->On();
}
}  // namespace opentxs::ui::implementation
