// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "interface/ui/profile/ProfileSection.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/ContactItemType.pb.h>
#include <functional>
#include <memory>
#include <stdexcept>
#include <thread>
#include <utility>

#include "interface/ui/base/Combined.hpp"
#include "interface/ui/base/Widget.hpp"
#include "internal/interface/ui/ProfileSection.hpp"
#include "internal/interface/ui/UI.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "opentxs/identifier/Generic.hpp"
#include "opentxs/identity/wot/claim/Group.hpp"
#include "opentxs/identity/wot/claim/Section.hpp"
#include "opentxs/identity/wot/claim/SectionType.hpp"  // IWYU pragma: keep
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/identity/wot/claim/Types.internal.hpp"
#include "opentxs/protobuf/syntax/VerifyContacts.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

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
    UnallocatedSet<protobuf::ContactItemType>>
    allowed_types_{
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

static const UnallocatedMap<
    identity::wot::claim::SectionType,
    UnallocatedMap<protobuf::ContactItemType, int>>
    sort_keys_{
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

auto ProfileSection::AllowedItems(
    const identity::wot::claim::SectionType section,
    const UnallocatedCString& lang) noexcept -> ProfileSection::ItemTypeList
{
    ItemTypeList output{};

    try {
        for (const auto& type : allowed_types_.at(section)) {
            output.emplace_back(
                translate(type), protobuf::TranslateItemType(type, lang));
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

    assert_false(nullptr == startup_);
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
    const auto lock = rLock{recursive_lock_};
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
        protobuf::TranslateSectionName(translate(row_id_), lang)};
}

auto ProfileSection::process_section(
    const identity::wot::claim::Section& section) noexcept
    -> UnallocatedSet<ProfileSectionRowID>
{
    assert_true(row_id_ == section.Type());

    UnallocatedSet<ProfileSectionRowID> active{};

    for (const auto& [type, group] : section) {
        assert_false(nullptr == group);

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
    const auto lock = rLock{recursive_lock_};
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
    const auto lock = rLock{recursive_lock_};
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
    const auto lock = rLock{recursive_lock_};
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
