// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/profile/ProfileSection.hpp"  // IWYU pragma: associated

#include <ContactEnums.pb.h>
#include <memory>
#include <stdexcept>
#include <string_view>
#include <thread>

#include "interface/ui/base/Combined.hpp"
#include "interface/ui/base/Widget.hpp"
#include "internal/identity/wot/claim/Types.hpp"
#include "internal/interface/ui/ProfileSection.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/serialization/protobuf/verify/VerifyContacts.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/identity/wot/claim/Group.hpp"
#include "opentxs/identity/wot/claim/Section.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Container.hpp"

template class opentxs::SharedPimpl<opentxs::ui::ProfileSection>;

namespace opentxs::factory
{
auto ProfileSectionWidget(
    const ui::implementation::ProfileInternalInterface& parent,
    const api::session::Client& api,
    const ui::implementation::ProfileRowID& rowID,
    const ui::implementation::ProfileSortKey& key,
    ui::implementation::CustomData& custom) noexcept
    -> std::shared_ptr<ui::implementation::ProfileRowInternal>
{
    using ReturnType = ui::implementation::ProfileSection;

    return std::make_shared<ReturnType>(parent, api, rowID, key, custom);
}
}  // namespace opentxs::factory

namespace opentxs::ui
{
static const std::map<
    identity::wot::claim::SectionType,
    UnallocatedSet<proto::ContactItemType>>
    allowed_types_{
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

static const UnallocatedMap<
    identity::wot::claim::SectionType,
    UnallocatedMap<proto::ContactItemType, int>>
    sort_keys_{
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

auto ProfileSection::AllowedItems(
    const identity::wot::claim::SectionType section,
    const UnallocatedCString& lang) noexcept -> ProfileSection::ItemTypeList
{
    ItemTypeList output{};

    try {
        for (const auto& type : allowed_types_.at(section)) {
            output.emplace_back(
                translate(type), proto::TranslateItemType(type, lang));
        }
    } catch (const std::out_of_range&) {
    }

    return output;
}
}  // namespace opentxs::ui

namespace opentxs::ui::implementation
{
ProfileSection::ProfileSection(
    const ProfileInternalInterface& parent,
    const api::session::Client& api,
    const ProfileRowID& rowID,
    const ProfileSortKey& key,
    CustomData& custom) noexcept
    : Combined(api, parent.NymID(), parent.WidgetID(), parent, rowID, key)
    , api_(api)
{
    startup_ = std::make_unique<std::thread>(
        &ProfileSection::startup,
        this,
        extract_custom<identity::wot::claim::Section>(custom));

    OT_ASSERT(startup_);
}

auto ProfileSection::AddClaim(
    const identity::wot::claim::ClaimType type,
    const UnallocatedCString& value,
    const bool primary,
    const bool active) const noexcept -> bool
{
    return parent_.AddClaim(row_id_, type, value, primary, active);
}

auto ProfileSection::check_type(const ProfileSectionRowID type) noexcept -> bool
{
    try {
        return 1 == allowed_types_.at(type.first).count(translate(type.second));
    } catch (const std::out_of_range&) {
    }

    return false;
}

auto ProfileSection::construct_row(
    const ProfileSectionRowID& id,
    const ProfileSectionSortKey& index,
    CustomData& custom) const noexcept -> RowPointer
{
    return factory::ProfileSubsectionWidget(*this, api_, id, index, custom);
}

auto ProfileSection::Delete(const int type, const UnallocatedCString& claimID)
    const noexcept -> bool
{
    rLock lock{recursive_lock_};
    const ProfileSectionRowID key{
        row_id_, static_cast<identity::wot::claim::ClaimType>(type)};
    const auto& group = lookup(lock, key);

    if (false == group.Valid()) { return false; }

    return group.Delete(claimID);
}

auto ProfileSection::Items(const UnallocatedCString& lang) const noexcept
    -> ProfileSection::ItemTypeList
{
    return AllowedItems(row_id_, lang);
}

auto ProfileSection::Name(const UnallocatedCString& lang) const noexcept
    -> UnallocatedCString
{
    return UnallocatedCString{
        proto::TranslateSectionName(translate(row_id_), lang)};
}

auto ProfileSection::process_section(
    const identity::wot::claim::Section& section) noexcept
    -> UnallocatedSet<ProfileSectionRowID>
{
    OT_ASSERT(row_id_ == section.Type());

    UnallocatedSet<ProfileSectionRowID> active{};

    for (const auto& [type, group] : section) {
        OT_ASSERT(group);

        const ProfileSectionRowID key{row_id_, type};

        if (check_type(key)) {
            CustomData custom{new identity::wot::claim::Group(*group)};
            add_item(key, sort_key(key), custom);
            active.emplace(key);
        }
    }

    return active;
}

auto ProfileSection::reindex(const ProfileSortKey&, CustomData& custom) noexcept
    -> bool
{
    delete_inactive(
        process_section(extract_custom<identity::wot::claim::Section>(custom)));

    return true;
}

auto ProfileSection::SetActive(
    const int type,
    const UnallocatedCString& claimID,
    const bool active) const noexcept -> bool
{
    rLock lock{recursive_lock_};
    const ProfileSectionRowID key{
        row_id_, static_cast<identity::wot::claim::ClaimType>(type)};
    const auto& group = lookup(lock, key);

    if (false == group.Valid()) { return false; }

    return group.SetActive(claimID, active);
}

auto ProfileSection::SetPrimary(
    const int type,
    const UnallocatedCString& claimID,
    const bool primary) const noexcept -> bool
{
    rLock lock{recursive_lock_};
    const ProfileSectionRowID key{
        row_id_, static_cast<identity::wot::claim::ClaimType>(type)};
    const auto& group = lookup(lock, key);

    if (false == group.Valid()) { return false; }

    return group.SetPrimary(claimID, primary);
}

auto ProfileSection::SetValue(
    const int type,
    const UnallocatedCString& claimID,
    const UnallocatedCString& value) const noexcept -> bool
{
    rLock lock{recursive_lock_};
    const ProfileSectionRowID key{
        row_id_, static_cast<identity::wot::claim::ClaimType>(type)};
    const auto& group = lookup(lock, key);

    if (false == group.Valid()) { return false; }

    return group.SetValue(claimID, value);
}

auto ProfileSection::sort_key(const ProfileSectionRowID type) noexcept -> int
{
    return sort_keys_.at(type.first).at(translate(type.second));
}

void ProfileSection::startup(
    const identity::wot::claim::Section section) noexcept
{
    process_section(section);
    finish_startup();
}
}  // namespace opentxs::ui::implementation
