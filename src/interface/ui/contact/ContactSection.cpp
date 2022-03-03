// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                             // IWYU pragma: associated
#include "1_Internal.hpp"                           // IWYU pragma: associated
#include "interface/ui/contact/ContactSection.hpp"  // IWYU pragma: associated

#include <memory>
#include <stdexcept>
#include <thread>
#include <type_traits>
#include <utility>

#include "interface/ui/base/Combined.hpp"
#include "interface/ui/base/Widget.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/identity/wot/claim/Group.hpp"
#include "opentxs/identity/wot/claim/Section.hpp"
#include "opentxs/identity/wot/claim/SectionType.hpp"
#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/ContactEnums.pb.h"

namespace opentxs::factory
{
auto ContactSectionWidget(
    const ui::implementation::ContactInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::ContactRowID& rowID,
    const ui::implementation::ContactSortKey& key,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::ContactRowInternal>
{
    using ReturnType = ui::implementation::ContactSection;

    return std::make_shared<ReturnType>(parent, api, rowID, key, custom);
}
}  // namespace opentxs::factory

namespace opentxs::ui::implementation
{
const std::map<
    identity::wot::claim::SectionType,
    UnallocatedSet<proto::ContactItemType>>
    ContactSection::allowed_types_{
        {identity::wot::claim::SectionType::Communication,
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
        {identity::wot::claim::SectionType::Profile,
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

const UnallocatedMap<
    identity::wot::claim::SectionType,
    UnallocatedMap<proto::ContactItemType, int>>
    ContactSection::sort_keys_{
        {identity::wot::claim::SectionType::Communication,
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
        {identity::wot::claim::SectionType::Profile,
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
    const api::session::Client& api,
    const ContactRowID& rowID,
    const ContactSortKey& key,
    CustomData& custom) noexcept
    : Combined(
          api,
          Identifier::Factory(parent.ContactID()),
          parent.WidgetID(),
          parent,
          rowID,
          key)
{
    startup_ = std::make_unique<std::thread>(
        &ContactSection::startup,
        this,
        extract_custom<identity::wot::claim::Section>(custom));

    OT_ASSERT(startup_)
}

auto ContactSection::check_type(const ContactSectionRowID type) noexcept -> bool
{
    try {
        return 1 == allowed_types_.at(type.first).count(translate(type.second));
    } catch (const std::out_of_range&) {
    }

    return false;
}

auto ContactSection::construct_row(
    const ContactSectionRowID& id,
    const ContactSectionSortKey& index,
    CustomData& custom) const noexcept -> RowPointer
{
    return factory::ContactSubsectionWidget(*this, api_, id, index, custom);
}

auto ContactSection::process_section(
    const identity::wot::claim::Section& section) noexcept
    -> UnallocatedSet<ContactSectionRowID>
{
    OT_ASSERT(row_id_ == section.Type())

    UnallocatedSet<ContactSectionRowID> active{};

    for (const auto& [type, group] : section) {
        OT_ASSERT(group)

        const ContactSectionRowID key{row_id_, type};

        if (check_type(key)) {
            CustomData custom{new identity::wot::claim::Group(*group)};
            add_item(key, sort_key(key), custom);
            active.emplace(key);
        }
    }

    return active;
}

auto ContactSection::reindex(
    const implementation::ContactSortKey&,
    implementation::CustomData& custom) noexcept -> bool
{
    delete_inactive(
        process_section(extract_custom<identity::wot::claim::Section>(custom)));

    return true;
}

auto ContactSection::sort_key(const ContactSectionRowID type) noexcept -> int
{
    return sort_keys_.at(type.first).at(translate(type.second));
}

void ContactSection::startup(
    const identity::wot::claim::Section section) noexcept
{
    process_section(section);
    finish_startup();
}
}  // namespace opentxs::ui::implementation
