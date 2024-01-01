// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/contact/ContactSection.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/ContactItemType.pb.h>
#include <functional>
#include <memory>
#include <stdexcept>
#include <thread>
#include <utility>

#include "interface/ui/base/Combined.hpp"
#include "interface/ui/base/Widget.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identity/wot/claim/Group.hpp"
#include "opentxs/identity/wot/claim/Section.hpp"
#include "opentxs/identity/wot/claim/SectionType.hpp"  // IWYU pragma: keep
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

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
    UnallocatedSet<protobuf::ContactItemType>>
    ContactSection::allowed_types_{
        {identity::wot::claim::SectionType::Communication,
         {
             protobuf::CITEMTYPE_PHONE,
             protobuf::CITEMTYPE_EMAIL,
             protobuf::CITEMTYPE_SKYPE,
             protobuf::CITEMTYPE_WIRE,
             protobuf::CITEMTYPE_QQ,
             protobuf::CITEMTYPE_BITMESSAGE,
             protobuf::CITEMTYPE_WHATSAPP,
             protobuf::CITEMTYPE_TELEGRAM,
             protobuf::CITEMTYPE_KIK,
             protobuf::CITEMTYPE_BBM,
             protobuf::CITEMTYPE_WECHAT,
             protobuf::CITEMTYPE_KAKAOTALK,
         }},
        {identity::wot::claim::SectionType::Profile,
         {
             protobuf::CITEMTYPE_FACEBOOK,  protobuf::CITEMTYPE_GOOGLE,
             protobuf::CITEMTYPE_LINKEDIN,  protobuf::CITEMTYPE_VK,
             protobuf::CITEMTYPE_ABOUTME,   protobuf::CITEMTYPE_ONENAME,
             protobuf::CITEMTYPE_TWITTER,   protobuf::CITEMTYPE_MEDIUM,
             protobuf::CITEMTYPE_TUMBLR,    protobuf::CITEMTYPE_YAHOO,
             protobuf::CITEMTYPE_MYSPACE,   protobuf::CITEMTYPE_MEETUP,
             protobuf::CITEMTYPE_REDDIT,    protobuf::CITEMTYPE_HACKERNEWS,
             protobuf::CITEMTYPE_WIKIPEDIA, protobuf::CITEMTYPE_ANGELLIST,
             protobuf::CITEMTYPE_GITHUB,    protobuf::CITEMTYPE_BITBUCKET,
             protobuf::CITEMTYPE_YOUTUBE,   protobuf::CITEMTYPE_VIMEO,
             protobuf::CITEMTYPE_TWITCH,    protobuf::CITEMTYPE_SNAPCHAT,
         }},
    };

const UnallocatedMap<
    identity::wot::claim::SectionType,
    UnallocatedMap<protobuf::ContactItemType, int>>
    ContactSection::sort_keys_{
        {identity::wot::claim::SectionType::Communication,
         {
             {protobuf::CITEMTYPE_PHONE, 0},
             {protobuf::CITEMTYPE_EMAIL, 1},
             {protobuf::CITEMTYPE_SKYPE, 2},
             {protobuf::CITEMTYPE_TELEGRAM, 3},
             {protobuf::CITEMTYPE_WIRE, 4},
             {protobuf::CITEMTYPE_WECHAT, 5},
             {protobuf::CITEMTYPE_QQ, 6},
             {protobuf::CITEMTYPE_KIK, 7},
             {protobuf::CITEMTYPE_KAKAOTALK, 8},
             {protobuf::CITEMTYPE_BBM, 9},
             {protobuf::CITEMTYPE_WHATSAPP, 10},
             {protobuf::CITEMTYPE_BITMESSAGE, 11},
         }},
        {identity::wot::claim::SectionType::Profile,
         {
             {protobuf::CITEMTYPE_FACEBOOK, 0},
             {protobuf::CITEMTYPE_TWITTER, 1},
             {protobuf::CITEMTYPE_REDDIT, 2},
             {protobuf::CITEMTYPE_GOOGLE, 3},
             {protobuf::CITEMTYPE_SNAPCHAT, 4},
             {protobuf::CITEMTYPE_YOUTUBE, 5},
             {protobuf::CITEMTYPE_TWITCH, 6},
             {protobuf::CITEMTYPE_GITHUB, 7},
             {protobuf::CITEMTYPE_LINKEDIN, 8},
             {protobuf::CITEMTYPE_MEDIUM, 9},
             {protobuf::CITEMTYPE_TUMBLR, 10},
             {protobuf::CITEMTYPE_YAHOO, 11},
             {protobuf::CITEMTYPE_MYSPACE, 12},
             {protobuf::CITEMTYPE_VK, 13},
             {protobuf::CITEMTYPE_MEETUP, 14},
             {protobuf::CITEMTYPE_VIMEO, 15},
             {protobuf::CITEMTYPE_ANGELLIST, 16},
             {protobuf::CITEMTYPE_ONENAME, 17},
             {protobuf::CITEMTYPE_ABOUTME, 18},
             {protobuf::CITEMTYPE_BITBUCKET, 19},
             {protobuf::CITEMTYPE_WIKIPEDIA, 20},
             {protobuf::CITEMTYPE_HACKERNEWS, 21},
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
          api.Factory().IdentifierFromBase58(parent.ContactID()),
          parent.WidgetID(),
          parent,
          rowID,
          key)
    , api_(api)
{
    startup_ = std::make_unique<std::thread>(
        &ContactSection::startup,
        this,
        extract_custom<identity::wot::claim::Section>(custom));

    assert_false(nullptr == startup_);
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
    assert_true(row_id_ == section.Type());

    UnallocatedSet<ContactSectionRowID> active{};

    for (const auto& [type, group] : section) {
        assert_false(nullptr == group);

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
