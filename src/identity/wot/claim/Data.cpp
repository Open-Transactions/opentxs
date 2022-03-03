// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "1_Internal.hpp"                       // IWYU pragma: associated
#include "opentxs/identity/wot/claim/Data.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iterator>
#include <sstream>
#include <tuple>
#include <utility>

#include "Proto.hpp"
#include "Proto.tpp"
#include "internal/identity/wot/claim/Types.hpp"
#include "internal/serialization/protobuf/Contact.hpp"
#include "internal/serialization/protobuf/verify/VerifyContacts.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/identity/wot/claim/Attribute.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"
#include "opentxs/identity/wot/claim/Group.hpp"
#include "opentxs/identity/wot/claim/Item.hpp"
#include "opentxs/identity/wot/claim/Section.hpp"
#include "opentxs/identity/wot/claim/SectionType.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "serialization/protobuf/ContactData.pb.h"
#include "serialization/protobuf/ContactItem.pb.h"
#include "serialization/protobuf/ContactSection.pb.h"

namespace opentxs::identity::wot::claim
{
static auto extract_sections(
    const api::Session& api,
    const UnallocatedCString& nym,
    const VersionNumber targetVersion,
    const proto::ContactData& serialized) -> Data::SectionMap
{
    Data::SectionMap sectionMap{};

    for (const auto& it : serialized.section()) {
        if ((0 != it.version()) && (it.item_size() > 0)) {
            sectionMap[translate(it.name())].reset(new Section(
                api,
                nym,
                check_version(serialized.version(), targetVersion),
                it));
        }
    }

    return sectionMap;
}

struct Data::Imp {
    using Scope =
        std::pair<claim::ClaimType, std::shared_ptr<const claim::Group>>;

    const api::Session& api_;
    const VersionNumber version_{0};
    const UnallocatedCString nym_{};
    const SectionMap sections_{};

    Imp(const api::Session& api,
        const UnallocatedCString& nym,
        const VersionNumber version,
        const VersionNumber targetVersion,
        const SectionMap& sections)
        : api_(api)
        , version_(check_version(version, targetVersion))
        , nym_(nym)
        , sections_(sections)
    {
        if (0 == version) {
            LogError()(OT_PRETTY_CLASS())("Warning: malformed version. "
                                          "Setting to ")(targetVersion)(".")
                .Flush();
        }
    }

    Imp(const Imp& rhs)
        : api_(rhs.api_)
        , version_(rhs.version_)
        , nym_(rhs.nym_)
        , sections_(rhs.sections_)
    {
    }

    auto scope() const -> Scope
    {
        const auto it = sections_.find(claim::SectionType::Scope);

        if (sections_.end() == it) {
            return {claim::ClaimType::Unknown, nullptr};
        }

        OT_ASSERT(it->second);

        const auto& section = *it->second;

        if (1 != section.Size()) { return {claim::ClaimType::Error, nullptr}; }

        return *section.begin();
    }
};

Data::Data(
    const api::Session& api,
    const UnallocatedCString& nym,
    const VersionNumber version,
    const VersionNumber targetVersion,
    const SectionMap& sections)
    : imp_(std::make_unique<Imp>(api, nym, version, targetVersion, sections))
{
    OT_ASSERT(imp_);
}

Data::Data(const Data& rhs)
    : imp_(std::make_unique<Imp>(*rhs.imp_))
{
    OT_ASSERT(imp_);
}

Data::Data(
    const api::Session& api,
    const UnallocatedCString& nym,
    const VersionNumber targetVersion,
    const proto::ContactData& serialized)
    : Data(
          api,
          nym,
          serialized.version(),
          targetVersion,
          extract_sections(api, nym, targetVersion, serialized))
{
}

Data::Data(
    const api::Session& api,
    const UnallocatedCString& nym,
    const VersionNumber targetVersion,
    const ReadView& serialized)
    : Data(
          api,
          nym,
          targetVersion,
          proto::Factory<proto::ContactData>(serialized))
{
}

auto Data::operator+(const Data& rhs) const -> Data
{
    auto map{imp_->sections_};

    for (auto& it : rhs.imp_->sections_) {
        const auto& rhsID = it.first;
        const auto& rhsSection = it.second;

        OT_ASSERT(rhsSection);

        auto lhs = map.find(rhsID);
        const bool exists = (map.end() != lhs);

        if (exists) {
            auto& section = lhs->second;

            OT_ASSERT(section);

            section.reset(new claim::Section(*section + *rhsSection));

            OT_ASSERT(section);
        } else {
            const auto [it, inserted] = map.emplace(rhsID, rhsSection);

            OT_ASSERT(inserted);

            [[maybe_unused]] const auto& notUsed = it;
        }
    }

    const auto version = std::max(imp_->version_, rhs.imp_->version_);

    return Data(imp_->api_, imp_->nym_, version, version, map);
}

Data::operator UnallocatedCString() const
{
    return PrintContactData([&] {
        auto proto = proto::ContactData{};
        Serialize(proto, false);

        return proto;
    }());
}

auto Data::AddContract(
    const UnallocatedCString& instrumentDefinitionID,
    const UnitType currency,
    const bool primary,
    const bool active) const -> Data
{
    bool needPrimary{true};
    const claim::SectionType section{claim::SectionType::Contract};
    auto group = Group(section, UnitToClaim(currency));

    if (group) { needPrimary = group->Primary().empty(); }

    UnallocatedSet<claim::Attribute> attrib{};

    if (active || primary || needPrimary) {
        attrib.emplace(claim::Attribute::Active);
    }

    if (primary || needPrimary) { attrib.emplace(claim::Attribute::Primary); }

    auto version = proto::RequiredVersion(
        translate(section), translate(UnitToClaim(currency)), imp_->version_);

    auto item = std::make_shared<Item>(
        imp_->api_,
        imp_->nym_,
        version,
        version,
        section,
        UnitToClaim(currency),
        instrumentDefinitionID,
        attrib,
        NULL_START,
        NULL_END,
        "");

    OT_ASSERT(item);

    return AddItem(item);
}

auto Data::AddEmail(
    const UnallocatedCString& value,
    const bool primary,
    const bool active) const -> Data
{
    bool needPrimary{true};
    const claim::SectionType section{claim::SectionType::Communication};
    const claim::ClaimType type{claim::ClaimType::Email};
    auto group = Group(section, type);

    if (group) { needPrimary = group->Primary().empty(); }

    UnallocatedSet<claim::Attribute> attrib{};

    if (active || primary || needPrimary) {
        attrib.emplace(claim::Attribute::Active);
    }

    if (primary || needPrimary) { attrib.emplace(claim::Attribute::Primary); }

    auto version = proto::RequiredVersion(
        translate(section), translate(type), imp_->version_);

    auto item = std::make_shared<Item>(
        imp_->api_,
        imp_->nym_,
        version,
        version,
        section,
        type,
        value,
        attrib,
        NULL_START,
        NULL_END,
        "");

    OT_ASSERT(item);

    return AddItem(item);
}

auto Data::AddItem(const ClaimTuple& claim) const -> Data
{
    auto version = proto::RequiredVersion(
        translate(static_cast<claim::SectionType>(std::get<1>(claim))),
        translate(static_cast<claim::ClaimType>(std::get<2>(claim))),
        imp_->version_);

    auto item =
        std::make_shared<Item>(imp_->api_, imp_->nym_, version, version, claim);

    return AddItem(item);
}

auto Data::AddItem(const std::shared_ptr<Item>& item) const -> Data
{
    OT_ASSERT(item);

    const auto& sectionID = item->Section();
    auto map{imp_->sections_};
    auto it = map.find(sectionID);

    auto version = proto::RequiredVersion(
        translate(sectionID), translate(item->Type()), imp_->version_);

    if (map.end() == it) {
        auto& section = map[sectionID];
        section.reset(new claim::Section(
            imp_->api_, imp_->nym_, version, version, sectionID, item));

        OT_ASSERT(section);
    } else {
        auto& section = it->second;

        OT_ASSERT(section);

        section.reset(new claim::Section(section->AddItem(item)));

        OT_ASSERT(section);
    }

    return Data(imp_->api_, imp_->nym_, version, version, map);
}

auto Data::AddPaymentCode(
    const UnallocatedCString& code,
    const UnitType currency,
    const bool primary,
    const bool active) const -> Data
{
    auto needPrimary{true};
    static constexpr auto section{claim::SectionType::Procedure};
    auto group = Group(section, UnitToClaim(currency));

    if (group) { needPrimary = group->Primary().empty(); }

    const auto attrib = [&] {
        auto out = UnallocatedSet<claim::Attribute>{};

        if (active || primary || needPrimary) {
            out.emplace(claim::Attribute::Active);
        }

        if (primary || needPrimary) { out.emplace(claim::Attribute::Primary); }

        return out;
    }();
    const auto version = proto::RequiredVersion(
        translate(section), translate(UnitToClaim(currency)), imp_->version_);

    if (0 == version) {
        LogError()(OT_PRETTY_CLASS())(
            "This currency is not allowed to set a procedure")
            .Flush();

        return *this;
    }

    auto item = std::make_shared<Item>(
        imp_->api_,
        imp_->nym_,
        version,
        version,
        section,
        UnitToClaim(currency),
        code,
        attrib,
        NULL_START,
        NULL_END,
        "");

    OT_ASSERT(item);

    return AddItem(item);
}

auto Data::AddPhoneNumber(
    const UnallocatedCString& value,
    const bool primary,
    const bool active) const -> Data
{
    bool needPrimary{true};
    const claim::SectionType section{claim::SectionType::Communication};
    const claim::ClaimType type{claim::ClaimType::Phone};
    auto group = Group(section, type);

    if (group) { needPrimary = group->Primary().empty(); }

    UnallocatedSet<claim::Attribute> attrib{};

    if (active || primary || needPrimary) {
        attrib.emplace(claim::Attribute::Active);
    }

    if (primary || needPrimary) { attrib.emplace(claim::Attribute::Primary); }

    auto version = proto::RequiredVersion(
        translate(section), translate(type), imp_->version_);

    auto item = std::make_shared<Item>(
        imp_->api_,
        imp_->nym_,
        version,
        version,
        section,
        type,
        value,
        attrib,
        NULL_START,
        NULL_END,
        "");

    OT_ASSERT(item);

    return AddItem(item);
}

auto Data::AddPreferredOTServer(const Identifier& id, const bool primary) const
    -> Data
{
    bool needPrimary{true};
    const claim::SectionType section{claim::SectionType::Communication};
    const claim::ClaimType type{claim::ClaimType::Opentxs};
    auto group = Group(section, type);

    if (group) { needPrimary = group->Primary().empty(); }

    UnallocatedSet<claim::Attribute> attrib{claim::Attribute::Active};

    if (primary || needPrimary) { attrib.emplace(claim::Attribute::Primary); }

    auto version = proto::RequiredVersion(
        translate(section), translate(type), imp_->version_);

    auto item = std::make_shared<Item>(
        imp_->api_,
        imp_->nym_,
        version,
        version,
        section,
        type,
        String::Factory(id)->Get(),
        attrib,
        NULL_START,
        NULL_END,
        "");

    OT_ASSERT(item);

    return AddItem(item);
}

auto Data::AddSocialMediaProfile(
    const UnallocatedCString& value,
    const claim::ClaimType type,
    const bool primary,
    const bool active) const -> Data
{
    auto map = imp_->sections_;
    // Add the item to the profile section.
    auto& section = map[claim::SectionType::Profile];

    bool needPrimary{true};
    if (section) {
        auto group = section->Group(type);

        if (group) { needPrimary = group->Primary().empty(); }
    }

    UnallocatedSet<claim::Attribute> attrib{};

    if (active || primary || needPrimary) {
        attrib.emplace(claim::Attribute::Active);
    }

    if (primary || needPrimary) { attrib.emplace(claim::Attribute::Primary); }

    auto version = proto::RequiredVersion(
        translate(claim::SectionType::Profile),
        translate(type),
        imp_->version_);

    auto item = std::make_shared<Item>(
        imp_->api_,
        imp_->nym_,
        version,
        version,
        claim::SectionType::Profile,
        type,
        value,
        attrib,
        NULL_START,
        NULL_END,
        "");

    OT_ASSERT(item);

    if (section) {
        section.reset(new claim::Section(section->AddItem(item)));
    } else {
        section.reset(new claim::Section(
            imp_->api_,
            imp_->nym_,
            version,
            version,
            claim::SectionType::Profile,
            item));
    }

    OT_ASSERT(section);

    // Add the item to the communication section.
    auto commSectionTypes =
        proto::AllowedItemTypes().at(proto::ContactSectionVersion(
            version, translate(claim::SectionType::Communication)));
    if (commSectionTypes.count(translate(type))) {
        auto& commSection = map[claim::SectionType::Communication];

        if (commSection) {
            auto group = commSection->Group(type);

            if (group) { needPrimary = group->Primary().empty(); }
        }

        attrib.clear();

        if (active || primary || needPrimary) {
            attrib.emplace(claim::Attribute::Active);
        }

        if (primary || needPrimary) {
            attrib.emplace(claim::Attribute::Primary);
        }

        item = std::make_shared<Item>(
            imp_->api_,
            imp_->nym_,
            version,
            version,
            claim::SectionType::Communication,
            type,
            value,
            attrib,
            NULL_START,
            NULL_END,
            "");

        OT_ASSERT(item);

        if (commSection) {
            commSection.reset(new claim::Section(commSection->AddItem(item)));
        } else {
            commSection.reset(new claim::Section(
                imp_->api_,
                imp_->nym_,
                version,
                version,
                claim::SectionType::Communication,
                item));
        }

        OT_ASSERT(commSection);
    }

    // Add the item to the identifier section.
    auto identifierSectionTypes =
        proto::AllowedItemTypes().at(proto::ContactSectionVersion(
            version, translate(claim::SectionType::Identifier)));
    if (identifierSectionTypes.count(translate(type))) {
        auto& identifierSection = map[claim::SectionType::Identifier];

        if (identifierSection) {
            auto group = identifierSection->Group(type);

            if (group) { needPrimary = group->Primary().empty(); }
        }

        attrib.clear();

        if (active || primary || needPrimary) {
            attrib.emplace(claim::Attribute::Active);
        }

        if (primary || needPrimary) {
            attrib.emplace(claim::Attribute::Primary);
        }

        item = std::make_shared<Item>(
            imp_->api_,
            imp_->nym_,
            version,
            version,
            claim::SectionType::Identifier,
            type,
            value,
            attrib,
            NULL_START,
            NULL_END,
            "");

        OT_ASSERT(item);

        if (identifierSection) {
            identifierSection.reset(
                new claim::Section(identifierSection->AddItem(item)));
        } else {
            identifierSection.reset(new claim::Section(
                imp_->api_,
                imp_->nym_,
                version,
                version,
                claim::SectionType::Identifier,
                item));
        }

        OT_ASSERT(identifierSection);
    }

    return Data(imp_->api_, imp_->nym_, version, version, map);
}

auto Data::begin() const -> Data::SectionMap::const_iterator
{
    return imp_->sections_.begin();
}

auto Data::BestEmail() const -> UnallocatedCString
{
    UnallocatedCString bestEmail;

    auto group =
        Group(claim::SectionType::Communication, claim::ClaimType::Email);

    if (group) {
        std::shared_ptr<Item> best = group->Best();

        if (best) { bestEmail = best->Value(); }
    }

    return bestEmail;
}

auto Data::BestPhoneNumber() const -> UnallocatedCString
{
    UnallocatedCString bestEmail;

    auto group =
        Group(claim::SectionType::Communication, claim::ClaimType::Phone);

    if (group) {
        std::shared_ptr<Item> best = group->Best();

        if (best) { bestEmail = best->Value(); }
    }

    return bestEmail;
}

auto Data::BestSocialMediaProfile(const claim::ClaimType type) const
    -> UnallocatedCString
{
    UnallocatedCString bestProfile;

    auto group = Group(claim::SectionType::Profile, type);
    if (group) {
        std::shared_ptr<Item> best = group->Best();

        if (best) { bestProfile = best->Value(); }
    }

    return bestProfile;
}

auto Data::Claim(const Identifier& item) const -> std::shared_ptr<Item>
{
    for (const auto& it : imp_->sections_) {
        const auto& section = it.second;

        OT_ASSERT(section);

        auto claim = section->Claim(item);

        if (claim) { return claim; }
    }

    return {};
}

auto Data::Contracts(const UnitType currency, const bool onlyActive) const
    -> UnallocatedSet<OTIdentifier>
{
    UnallocatedSet<OTIdentifier> output{};
    const claim::SectionType section{claim::SectionType::Contract};
    auto group = Group(section, UnitToClaim(currency));

    if (group) {
        for (const auto& it : *group) {
            const auto& id = it.first;

            OT_ASSERT(it.second);

            const auto& claim = *it.second;

            if ((false == onlyActive) || claim.isActive()) {
                output.insert(id);
            }
        }
    }

    return output;
}

auto Data::Delete(const Identifier& id) const -> Data
{
    bool deleted{false};
    auto map = imp_->sections_;

    for (auto& it : map) {
        auto& section = it.second;

        OT_ASSERT(section);

        if (section->HaveClaim(id)) {
            section.reset(new claim::Section(section->Delete(id)));

            OT_ASSERT(section);

            deleted = true;

            if (0 == section->Size()) { map.erase(it.first); }

            break;
        }
    }

    if (false == deleted) { return *this; }

    return Data(imp_->api_, imp_->nym_, imp_->version_, imp_->version_, map);
}

auto Data::EmailAddresses(bool active) const -> UnallocatedCString
{
    std::ostringstream stream;

    auto group =
        Group(claim::SectionType::Communication, claim::ClaimType::Email);
    if (group) {
        for (const auto& it : *group) {
            OT_ASSERT(it.second);

            const auto& claim = *it.second;

            if ((false == active) || claim.isActive()) {
                stream << claim.Value() << ',';
            }
        }
    }

    UnallocatedCString output = stream.str();

    if (0 < output.size()) { output.erase(output.size() - 1, 1); }

    return output;
}

auto Data::end() const -> Data::SectionMap::const_iterator
{
    return imp_->sections_.end();
}

auto Data::Group(const claim::SectionType section, const claim::ClaimType type)
    const -> std::shared_ptr<claim::Group>
{
    const auto it = imp_->sections_.find(section);

    if (imp_->sections_.end() == it) { return {}; }

    OT_ASSERT(it->second);

    return it->second->Group(type);
}

auto Data::HaveClaim(const Identifier& item) const -> bool
{
    for (const auto& section : imp_->sections_) {
        OT_ASSERT(section.second);

        if (section.second->HaveClaim(item)) { return true; }
    }

    return false;
}

auto Data::HaveClaim(
    const claim::SectionType section,
    const claim::ClaimType type,
    const UnallocatedCString& value) const -> bool
{
    auto group = Group(section, type);

    if (false == bool(group)) { return false; }

    for (const auto& it : *group) {
        OT_ASSERT(it.second);

        const auto& claim = *it.second;

        if (value == claim.Value()) { return true; }
    }

    return false;
}

auto Data::Name() const -> UnallocatedCString
{
    auto group = imp_->scope().second;

    if (false == bool(group)) { return {}; }

    auto claim = group->Best();

    if (false == bool(claim)) { return {}; }

    return claim->Value();
}

auto Data::PhoneNumbers(bool active) const -> UnallocatedCString
{
    std::ostringstream stream;

    auto group =
        Group(claim::SectionType::Communication, claim::ClaimType::Phone);
    if (group) {
        for (const auto& it : *group) {
            OT_ASSERT(it.second);

            const auto& claim = *it.second;

            if ((false == active) || claim.isActive()) {
                stream << claim.Value() << ',';
            }
        }
    }

    UnallocatedCString output = stream.str();

    if (0 < output.size()) { output.erase(output.size() - 1, 1); }

    return output;
}

auto Data::PreferredOTServer() const -> OTNotaryID
{
    auto group =
        Group(claim::SectionType::Communication, claim::ClaimType::Opentxs);

    if (false == bool(group)) { return identifier::Notary::Factory(); }

    auto claim = group->Best();

    if (false == bool(claim)) { return identifier::Notary::Factory(); }

    return identifier::Notary::Factory(claim->Value());
}

auto Data::PrintContactData(const proto::ContactData& data)
    -> UnallocatedCString
{
    std::stringstream output;
    output << "Version " << data.version() << " contact data" << std::endl;
    output << "Sections found: " << data.section().size() << std::endl;

    for (const auto& section : data.section()) {
        output << "- Section: " << proto::TranslateSectionName(section.name())
               << ", version: " << section.version() << " containing "
               << section.item().size() << " item(s)." << std::endl;

        for (const auto& item : section.item()) {
            output << "-- Item type: \""
                   << proto::TranslateItemType(item.type()) << "\", value: \""
                   << item.value() << "\", start: " << item.start()
                   << ", end: " << item.end() << ", version: " << item.version()
                   << std::endl
                   << "--- Attributes: ";

            for (const auto& attribute : item.attribute()) {
                output << proto::TranslateItemAttributes(translate(
                              static_cast<claim::Attribute>(attribute)))
                       << " ";
            }

            output << std::endl;
        }
    }

    return output.str();
}

auto Data::Section(const claim::SectionType section) const
    -> std::shared_ptr<claim::Section>
{
    const auto it = imp_->sections_.find(section);

    if (imp_->sections_.end() == it) { return {}; }

    return it->second;
}

auto Data::SetCommonName(const UnallocatedCString& name) const -> Data
{
    const claim::SectionType section{claim::SectionType::Identifier};
    const claim::ClaimType type{claim::ClaimType::Commonname};
    UnallocatedSet<claim::Attribute> attrib{
        claim::Attribute::Active, claim::Attribute::Primary};

    auto item = std::make_shared<Item>(
        imp_->api_,
        imp_->nym_,
        imp_->version_,
        imp_->version_,
        section,
        type,
        name,
        attrib,
        NULL_START,
        NULL_END,
        "");

    OT_ASSERT(item);

    return AddItem(item);
}

auto Data::SetName(const UnallocatedCString& name, const bool primary) const
    -> Data
{
    const Imp::Scope& scopeInfo = imp_->scope();

    OT_ASSERT(scopeInfo.second);

    const claim::SectionType section{claim::SectionType::Scope};
    const claim::ClaimType type = scopeInfo.first;

    UnallocatedSet<claim::Attribute> attrib{claim::Attribute::Active};

    if (primary) { attrib.emplace(claim::Attribute::Primary); }

    auto item = std::make_shared<Item>(
        imp_->api_,
        imp_->nym_,
        imp_->version_,
        imp_->version_,
        section,
        type,
        name,
        attrib,
        NULL_START,
        NULL_END,
        "");

    OT_ASSERT(item);

    return AddItem(item);
}

auto Data::SetScope(const claim::ClaimType type, const UnallocatedCString& name)
    const -> Data
{
    OT_ASSERT(type);

    const claim::SectionType section{claim::SectionType::Scope};

    if (claim::ClaimType::Unknown == imp_->scope().first) {
        auto mapCopy = imp_->sections_;
        mapCopy.erase(section);
        UnallocatedSet<claim::Attribute> attrib{
            claim::Attribute::Active, claim::Attribute::Primary};

        auto version = proto::RequiredVersion(
            translate(section), translate(type), imp_->version_);

        auto item = std::make_shared<Item>(
            imp_->api_,
            imp_->nym_,
            version,
            version,
            section,
            type,
            name,
            attrib,
            NULL_START,
            NULL_END,
            "");

        OT_ASSERT(item);

        auto newSection = std::make_shared<claim::Section>(
            imp_->api_, imp_->nym_, version, version, section, item);

        OT_ASSERT(newSection);

        mapCopy[section] = newSection;

        return Data(imp_->api_, imp_->nym_, version, version, mapCopy);
    } else {
        LogError()(OT_PRETTY_CLASS())("Scope already set.").Flush();

        return *this;
    }
}

auto Data::Serialize(AllocateOutput destination, const bool withID) const
    -> bool
{
    return write(
        [&] {
            auto proto = proto::ContactData{};
            Serialize(proto);

            return proto;
        }(),
        destination);
}

auto Data::Serialize(proto::ContactData& output, const bool withID) const
    -> bool
{
    output.set_version(imp_->version_);

    for (const auto& it : imp_->sections_) {
        const auto& section = it.second;

        OT_ASSERT(section);

        section->SerializeTo(output, withID);
    }

    return true;
}

auto Data::SocialMediaProfiles(const claim::ClaimType type, bool active) const
    -> UnallocatedCString
{
    std::ostringstream stream;

    auto group = Group(claim::SectionType::Profile, type);
    if (group) {
        for (const auto& it : *group) {
            OT_ASSERT(it.second);

            const auto& claim = *it.second;

            if ((false == active) || claim.isActive()) {
                stream << claim.Value() << ',';
            }
        }
    }

    UnallocatedCString output = stream.str();

    if (0 < output.size()) { output.erase(output.size() - 1, 1); }

    return output;
}

auto Data::SocialMediaProfileTypes() const
    -> const UnallocatedSet<claim::ClaimType>
{
    try {
        auto profiletypes =
            proto::AllowedItemTypes().at(proto::ContactSectionVersion(
                CONTACT_CONTACT_DATA_VERSION, proto::CONTACTSECTION_PROFILE));

        UnallocatedSet<claim::ClaimType> output;
        std::transform(
            profiletypes.begin(),
            profiletypes.end(),
            std::inserter(output, output.end()),
            [](proto::ContactItemType itemtype) -> claim::ClaimType {
                return translate(itemtype);
            });

        return output;

    } catch (...) {
        return {};
    }
}

auto Data::Type() const -> claim::ClaimType { return imp_->scope().first; }

auto Data::Version() const -> VersionNumber { return imp_->version_; }

Data::~Data() = default;
}  // namespace opentxs::identity::wot::claim
