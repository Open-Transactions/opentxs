// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/api/client/Contacts.hpp"
#include "opentxs/contact/ContactGroup.hpp"
#include "opentxs/contact/ContactSection.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/network/zeromq/Context.hpp"
#include "opentxs/network/zeromq/ListenCallback.hpp"
#include "opentxs/network/zeromq/Frame.hpp"
#include "opentxs/network/zeromq/SubscribeSocket.hpp"
#include "opentxs/ui/ContactSection.hpp"
#include "opentxs/ui/ContactSubsection.hpp"

#include "ContactSubsectionBlank.hpp"
#include "InternalUI.hpp"
#include "List.hpp"
#include "RowType.hpp"

#include <map>
#include <memory>
#include <set>
#include <thread>
#include <tuple>
#include <vector>

#include "ContactSection.hpp"

template class opentxs::SharedPimpl<opentxs::ui::ContactSection>;

//#define OT_METHOD "opentxs::ui::implementation::ContactSection::"

namespace opentxs
{
ui::implementation::ContactRowInternal* Factory::ContactSectionWidget(
    const ui::implementation::ContactInternalInterface& parent,
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& publisher,
    const ui::implementation::ContactRowID& rowID,
    const ui::implementation::ContactSortKey& key,
    const ui::implementation::CustomData& custom)
{
    return new ui::implementation::ContactSection(
        parent, api, publisher, rowID, key, custom);
}
}  // namespace opentxs

namespace opentxs::ui::implementation
{
const std::map<proto::ContactSectionName, std::set<proto::ContactItemType>>
    ContactSection::allowed_types_{
        {proto::CONTACTSECTION_COMMUNICATION,
         {
             proto::CITEMTYPE_PHONE,
             proto::CITEMTYPE_EMAIL,
             proto::CITEMTYPE_SKYPE,
             proto::CITEMTYPE_WIRE,
             proto::CITEMTYPE_QQ,
             proto::CITEMTYPE_BITMESSAGE,
             proto::CITEMTYPE_WHATSAPP,
             proto::CITEMTYPE_TELEGRAM,
             proto::CITEMTYPE_KIK,
             proto::CITEMTYPE_BBM,
             proto::CITEMTYPE_WECHAT,
             proto::CITEMTYPE_KAKAOTALK,
         }},
        {proto::CONTACTSECTION_PROFILE,
         {
             proto::CITEMTYPE_FACEBOOK,  proto::CITEMTYPE_GOOGLE,
             proto::CITEMTYPE_LINKEDIN,  proto::CITEMTYPE_VK,
             proto::CITEMTYPE_ABOUTME,   proto::CITEMTYPE_ONENAME,
             proto::CITEMTYPE_TWITTER,   proto::CITEMTYPE_MEDIUM,
             proto::CITEMTYPE_TUMBLR,    proto::CITEMTYPE_YAHOO,
             proto::CITEMTYPE_MYSPACE,   proto::CITEMTYPE_MEETUP,
             proto::CITEMTYPE_REDDIT,    proto::CITEMTYPE_HACKERNEWS,
             proto::CITEMTYPE_WIKIPEDIA, proto::CITEMTYPE_ANGELLIST,
             proto::CITEMTYPE_GITHUB,    proto::CITEMTYPE_BITBUCKET,
             proto::CITEMTYPE_YOUTUBE,   proto::CITEMTYPE_VIMEO,
             proto::CITEMTYPE_TWITCH,    proto::CITEMTYPE_SNAPCHAT,
         }},
    };

const std::map<proto::ContactSectionName, std::map<proto::ContactItemType, int>>
    ContactSection::sort_keys_{
        {proto::CONTACTSECTION_COMMUNICATION,
         {
             {proto::CITEMTYPE_PHONE, 0},
             {proto::CITEMTYPE_EMAIL, 1},
             {proto::CITEMTYPE_SKYPE, 2},
             {proto::CITEMTYPE_TELEGRAM, 3},
             {proto::CITEMTYPE_WIRE, 4},
             {proto::CITEMTYPE_WECHAT, 5},
             {proto::CITEMTYPE_QQ, 6},
             {proto::CITEMTYPE_KIK, 7},
             {proto::CITEMTYPE_KAKAOTALK, 8},
             {proto::CITEMTYPE_BBM, 9},
             {proto::CITEMTYPE_WHATSAPP, 10},
             {proto::CITEMTYPE_BITMESSAGE, 11},
         }},
        {proto::CONTACTSECTION_PROFILE,
         {
             {proto::CITEMTYPE_FACEBOOK, 0},
             {proto::CITEMTYPE_TWITTER, 1},
             {proto::CITEMTYPE_REDDIT, 2},
             {proto::CITEMTYPE_GOOGLE, 3},
             {proto::CITEMTYPE_SNAPCHAT, 4},
             {proto::CITEMTYPE_YOUTUBE, 5},
             {proto::CITEMTYPE_TWITCH, 6},
             {proto::CITEMTYPE_GITHUB, 7},
             {proto::CITEMTYPE_LINKEDIN, 8},
             {proto::CITEMTYPE_MEDIUM, 9},
             {proto::CITEMTYPE_TUMBLR, 10},
             {proto::CITEMTYPE_YAHOO, 11},
             {proto::CITEMTYPE_MYSPACE, 12},
             {proto::CITEMTYPE_VK, 13},
             {proto::CITEMTYPE_MEETUP, 14},
             {proto::CITEMTYPE_VIMEO, 15},
             {proto::CITEMTYPE_ANGELLIST, 16},
             {proto::CITEMTYPE_ONENAME, 17},
             {proto::CITEMTYPE_ABOUTME, 18},
             {proto::CITEMTYPE_BITBUCKET, 19},
             {proto::CITEMTYPE_WIKIPEDIA, 20},
             {proto::CITEMTYPE_HACKERNEWS, 21},
         }},
    };

ContactSection::ContactSection(
    const ContactInternalInterface& parent,
    const api::client::Manager& api,
    const network::zeromq::PublishSocket& publisher,
    const ContactRowID& rowID,
    const ContactSortKey& key,
    const CustomData& custom)
    : ContactSectionList(
          api,
          publisher,
          identifier::Nym::Factory(parent.ContactID()),
          parent.WidgetID())
    , ContactSectionRow(parent, rowID, true)
{
    init();
    startup_.reset(new std::thread(&ContactSection::startup, this, custom));

    OT_ASSERT(startup_)
}

bool ContactSection::check_type(const ContactSectionRowID type)
{
    try {
        return 1 == allowed_types_.at(type.first).count(type.second);
    } catch (const std::out_of_range&) {
    }

    return false;
}

void ContactSection::construct_row(
    const ContactSectionRowID& id,
    const ContactSectionSortKey& index,
    const CustomData& custom) const
{
    OT_ASSERT(1 == custom.size())

    names_.emplace(id, index);
    items_[index].emplace(
        id,
        Factory::ContactSubsectionWidget(
            *this, api_, publisher_, id, index, custom));
}

std::set<ContactSectionRowID> ContactSection::process_section(
    const opentxs::ContactSection& section)
{
    OT_ASSERT(row_id_ == section.Type())

    std::set<ContactSectionRowID> active{};

    for (const auto& [type, group] : section) {
        OT_ASSERT(group)

        const ContactSectionRowID key{row_id_, type};

        if (check_type(key)) {
            CustomData custom{new opentxs::ContactGroup(*group)};
            add_item(key, sort_key(key), custom);
            active.emplace(key);
        }
    }

    return active;
}

void ContactSection::reindex(
    const implementation::ContactSortKey&,
    const implementation::CustomData& custom)
{
    delete_inactive(
        process_section(extract_custom<opentxs::ContactSection>(custom)));
}

int ContactSection::sort_key(const ContactSectionRowID type)
{
    return sort_keys_.at(type.first).at(type.second);
}

void ContactSection::startup(const CustomData custom)
{
    process_section(extract_custom<opentxs::ContactSection>(custom));
    startup_complete_->On();
}
}  // namespace opentxs::ui::implementation
