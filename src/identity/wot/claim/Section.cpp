// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/identity/wot/claim/Section.hpp"  // IWYU pragma: associated

#include <ContactData.pb.h>
#include <ContactItem.pb.h>
#include <ContactSection.pb.h>
#include <algorithm>
#include <functional>
#include <utility>

#include "internal/identity/wot/claim/Types.hpp"
#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/serialization/protobuf/verify/VerifyContacts.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/identity/wot/claim/Group.hpp"
#include "opentxs/identity/wot/claim/Item.hpp"
#include "opentxs/identity/wot/claim/SectionType.hpp"  // IWYU pragma: keep
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::identity::wot::claim
{
static auto create_group(
    const UnallocatedCString& nym,
    const claim::SectionType section,
    const std::shared_ptr<Item>& item) -> Section::GroupMap
{
    assert_false(nullptr == item);

    Section::GroupMap output{};
    const auto& itemType = item->Type();

    output[itemType].reset(new Group(nym, section, item));

    return output;
}

static auto extract_groups(
    const api::Session& api,
    const UnallocatedCString& nym,
    const VersionNumber parentVersion,
    const proto::ContactSection& serialized) -> Section::GroupMap
{
    Section::GroupMap groupMap{};
    UnallocatedMap<claim::ClaimType, Group::ItemMap> itemMaps{};
    const auto& section = serialized.name();

    for (const auto& item : serialized.item()) {
        const auto& itemType = item.type();
        auto instantiated = std::make_shared<Item>(
            api,
            nym,
            check_version(serialized.version(), parentVersion),
            translate(section),
            item);

        assert_false(nullptr == instantiated);

        const auto& itemID = instantiated->ID();
        auto& itemMap = itemMaps[translate(itemType)];
        itemMap.emplace(itemID, instantiated);
    }

    for (const auto& itemMap : itemMaps) {
        const auto& type = itemMap.first;
        const auto& map = itemMap.second;
        auto& group = groupMap[type];
        group.reset(new Group(nym, translate(section), type, map));
    }

    return groupMap;
}

struct Section::Imp {
    const api::Session& api_;
    const VersionNumber version_;
    const UnallocatedCString nym_;
    const claim::SectionType section_;
    const GroupMap groups_;

    auto add_scope(const std::shared_ptr<Item>& item) const -> Section
    {
        assert_false(nullptr == item);

        auto scope = item;

        bool needsPrimary{true};

        const auto& groupID = scope->Type();
        GroupMap groups = groups_;
        const auto& group = groups[groupID];

        if (group) { needsPrimary = (1 > group->Size()); }

        if (needsPrimary && false == scope->isPrimary()) {
            scope.reset(new Item(scope->SetPrimary(true)));
        }

        if (false == scope->isActive()) {
            scope.reset(new Item(scope->SetActive(true)));
        }

        groups[groupID].reset(new claim::Group(nym_, section_, scope));

        auto version = proto::RequiredVersion(
            translate(section_), translate(item->Type()), version_);

        return {api_, nym_, version, version, section_, groups};
    }

    Imp(const api::Session& api,
        const UnallocatedCString& nym,
        const VersionNumber version,
        const VersionNumber parentVersion,
        const claim::SectionType section,
        const GroupMap& groups)
        : api_(api)
        , version_(check_version(version, parentVersion))
        , nym_(nym)
        , section_(section)
        , groups_(groups)
    {
    }

    Imp(const Imp& rhs) noexcept

        = default;

    Imp(Imp&& rhs) noexcept
        : api_(rhs.api_)
        , version_(rhs.version_)
        , nym_(std::move(const_cast<UnallocatedCString&>(rhs.nym_)))
        , section_(rhs.section_)
        , groups_(std::move(const_cast<GroupMap&>(rhs.groups_)))
    {
    }
};

Section::Section(
    const api::Session& api,
    const UnallocatedCString& nym,
    const VersionNumber version,
    const VersionNumber parentVersion,
    const claim::SectionType section,
    const GroupMap& groups)
    : imp_(std::make_unique<
           Imp>(api, nym, version, parentVersion, section, groups))
{
}

Section::Section(const Section& rhs) noexcept
    : imp_(std::make_unique<Imp>(*rhs.imp_))
{
    assert_false(nullptr == imp_);
}

Section::Section(Section&& rhs) noexcept
    : imp_(std::move(rhs.imp_))
{
    assert_false(nullptr == imp_);
}

Section::Section(
    const api::Session& api,
    const UnallocatedCString& nym,
    const VersionNumber version,
    const VersionNumber parentVersion,
    const claim::SectionType section,
    const std::shared_ptr<Item>& item)
    : Section(
          api,
          nym,
          version,
          parentVersion,
          section,
          create_group(nym, section, item))
{
    if (0 == version) {
        LogError()()("Warning: malformed version. "
                     "Setting to ")(parentVersion)(".")
            .Flush();
    }
}

Section::Section(
    const api::Session& api,
    const UnallocatedCString& nym,
    const VersionNumber parentVersion,
    const proto::ContactSection& serialized)
    : Section(
          api,
          nym,
          serialized.version(),
          parentVersion,
          translate(serialized.name()),
          extract_groups(api, nym, parentVersion, serialized))
{
}

Section::Section(
    const api::Session& api,
    const UnallocatedCString& nym,
    const VersionNumber parentVersion,
    const ReadView& serialized)
    : Section(
          api,
          nym,
          parentVersion,
          proto::Factory<proto::ContactSection>(serialized))
{
}

auto Section::operator+(const Section& rhs) const -> Section
{
    auto map{imp_->groups_};

    for (const auto& it : rhs.imp_->groups_) {
        const auto& rhsID = it.first;
        const auto& rhsGroup = it.second;

        assert_false(nullptr == rhsGroup);

        auto lhs = map.find(rhsID);
        const bool exists = (map.end() != lhs);

        if (exists) {
            auto& group = lhs->second;

            assert_false(nullptr == group);

            group.reset(new claim::Group(*group + *rhsGroup));

            assert_false(nullptr == group);
        } else {
            [[maybe_unused]] const auto [i, inserted] =
                map.emplace(rhsID, rhsGroup);

            assert_true(inserted);
        }
    }

    const auto version = std::max(imp_->version_, rhs.Version());

    return {imp_->api_, imp_->nym_, version, version, imp_->section_, map};
}

auto Section::AddItem(const std::shared_ptr<Item>& item) const -> Section
{
    assert_false(nullptr == item);

    const bool specialCaseScope = (claim::SectionType::Scope == imp_->section_);

    if (specialCaseScope) { return imp_->add_scope(item); }

    const auto& groupID = item->Type();
    const bool groupExists = imp_->groups_.count(groupID);
    auto map = imp_->groups_;

    if (groupExists) {
        auto& existing = map.at(groupID);

        assert_false(nullptr == existing);

        existing.reset(new claim::Group(existing->AddItem(item)));
    } else {
        map[groupID].reset(new claim::Group(imp_->nym_, imp_->section_, item));
    }

    auto version = proto::RequiredVersion(
        translate(imp_->section_), translate(item->Type()), imp_->version_);

    return {imp_->api_, imp_->nym_, version, version, imp_->section_, map};
}

auto Section::begin() const -> Section::GroupMap::const_iterator
{
    return imp_->groups_.cbegin();
}

auto Section::Claim(const identifier::Generic& item) const
    -> std::shared_ptr<Item>
{
    for (const auto& group : imp_->groups_) {
        assert_false(nullptr == group.second);

        auto claim = group.second->Claim(item);

        if (claim) { return claim; }
    }

    return {};
}

auto Section::Delete(const identifier::Generic& id) const -> Section
{
    bool deleted{false};
    auto map = imp_->groups_;

    for (auto& it : map) {
        auto& group = it.second;

        assert_false(nullptr == group);

        if (group->HaveClaim(id)) {
            group.reset(new claim::Group(group->Delete(id)));
            deleted = true;

            if (0 == group->Size()) { map.erase(it.first); }

            break;
        }
    }

    if (false == deleted) { return *this; }

    return {
        imp_->api_,
        imp_->nym_,
        imp_->version_,
        imp_->version_,
        imp_->section_,
        map};
}

auto Section::end() const -> Section::GroupMap::const_iterator
{
    return imp_->groups_.cend();
}

auto Section::Group(const claim::ClaimType& type) const
    -> std::shared_ptr<claim::Group>
{
    const auto it = imp_->groups_.find(type);

    if (imp_->groups_.end() == it) { return {}; }

    return it->second;
}

auto Section::HaveClaim(const identifier::Generic& item) const -> bool
{
    for (const auto& group : imp_->groups_) {
        assert_false(nullptr == group.second);

        if (group.second->HaveClaim(item)) { return true; }
    }

    return false;
}

auto Section::Serialize(Writer&& destination, const bool withIDs) const -> bool
{
    proto::ContactData data;
    if (false == SerializeTo(data, withIDs) || data.section_size() != 1) {
        LogError()()("Failed to serialize the contactsection.").Flush();
        return false;
    }

    auto section = data.section(0);

    write(section, std::move(destination));

    return true;
}

auto Section::SerializeTo(proto::ContactData& section, const bool withIDs) const
    -> bool
{
    bool output = true;
    auto& serialized = *section.add_section();
    serialized.set_version(imp_->version_);
    serialized.set_name(translate(imp_->section_));

    for (const auto& it : imp_->groups_) {
        const auto& group = it.second;

        assert_false(nullptr == group);

        output &= group->SerializeTo(serialized, withIDs);
    }

    return output;
}

auto Section::Size() const -> std::size_t { return imp_->groups_.size(); }

auto Section::Type() const -> const claim::SectionType&
{
    return imp_->section_;
}

auto Section::Version() const -> VersionNumber { return imp_->version_; }

Section::~Section() = default;
}  // namespace opentxs::identity::wot::claim
