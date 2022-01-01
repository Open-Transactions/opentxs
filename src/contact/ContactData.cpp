// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                     // IWYU pragma: associated
#include "1_Internal.hpp"                   // IWYU pragma: associated
#include "opentxs/contact/ContactData.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <iterator>
#include <sstream>
#include <tuple>
#include <utility>

#include "Proto.hpp"
#include "Proto.tpp"
#include "internal/contact/Contact.hpp"
#include "internal/protobuf/Contact.hpp"
#include "internal/protobuf/verify/VerifyContacts.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/contact/Attribute.hpp"
#include "opentxs/contact/ClaimType.hpp"
#include "opentxs/contact/ContactGroup.hpp"
#include "opentxs/contact/ContactItem.hpp"
#include "opentxs/contact/ContactSection.hpp"
#include "opentxs/contact/SectionType.hpp"
#include "opentxs/contact/Types.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "serialization/protobuf/ContactData.pb.h"
#include "serialization/protobuf/ContactItem.pb.h"
#include "serialization/protobuf/ContactSection.pb.h"

namespace opentxs::contact
{
static auto check_version(
    const VersionNumber in,
    const VersionNumber targetVersion) -> VersionNumber
{
    // Upgrade version
    if (targetVersion > in) { return targetVersion; }

    return in;
}

static auto extract_sections(
    const api::Session& api,
    const std::string& nym,
    const VersionNumber targetVersion,
    const proto::ContactData& serialized) -> ContactData::SectionMap
{
    ContactData::SectionMap sectionMap{};

    for (const auto& it : serialized.section()) {
        if ((0 != it.version()) && (it.item_size() > 0)) {
            sectionMap[translate(it.name())].reset(new ContactSection(
                api,
                nym,
                check_version(serialized.version(), targetVersion),
                it));
        }
    }

    return sectionMap;
}

struct ContactData::Imp {
    using Scope =
        std::pair<contact::ClaimType, std::shared_ptr<const ContactGroup>>;

    const api::Session& api_;
    const VersionNumber version_{0};
    const std::string nym_{};
    const SectionMap sections_{};

    Imp(const api::Session& api,
        const std::string& nym,
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
        const auto it = sections_.find(contact::SectionType::Scope);

        if (sections_.end() == it) {
            return {contact::ClaimType::Unknown, nullptr};
        }

        OT_ASSERT(it->second);

        const auto& section = *it->second;

        if (1 != section.Size()) {
            return {contact::ClaimType::Error, nullptr};
        }

        return *section.begin();
    }
};

ContactData::ContactData(
    const api::Session& api,
    const std::string& nym,
    const VersionNumber version,
    const VersionNumber targetVersion,
    const SectionMap& sections)
    : imp_(std::make_unique<Imp>(api, nym, version, targetVersion, sections))
{
    OT_ASSERT(imp_);
}

ContactData::ContactData(const ContactData& rhs)
    : imp_(std::make_unique<Imp>(*rhs.imp_))
{
    OT_ASSERT(imp_);
}

ContactData::ContactData(
    const api::Session& api,
    const std::string& nym,
    const VersionNumber targetVersion,
    const proto::ContactData& serialized)
    : ContactData(
          api,
          nym,
          serialized.version(),
          targetVersion,
          extract_sections(api, nym, targetVersion, serialized))
{
}

ContactData::ContactData(
    const api::Session& api,
    const std::string& nym,
    const VersionNumber targetVersion,
    const ReadView& serialized)
    : ContactData(
          api,
          nym,
          targetVersion,
          proto::Factory<proto::ContactData>(serialized))
{
}

auto ContactData::operator+(const ContactData& rhs) const -> ContactData
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

            section.reset(new ContactSection(*section + *rhsSection));

            OT_ASSERT(section);
        } else {
            const auto [it, inserted] = map.emplace(rhsID, rhsSection);

            OT_ASSERT(inserted);

            [[maybe_unused]] const auto& notUsed = it;
        }
    }

    const auto version = std::max(imp_->version_, rhs.imp_->version_);

    return ContactData(imp_->api_, imp_->nym_, version, version, map);
}

ContactData::operator std::string() const
{
    return PrintContactData([&] {
        auto proto = proto::ContactData{};
        Serialize(proto, false);

        return proto;
    }());
}

auto ContactData::AddContract(
    const std::string& instrumentDefinitionID,
    const core::UnitType currency,
    const bool primary,
    const bool active) const -> ContactData
{
    bool needPrimary{true};
    const contact::SectionType section{contact::SectionType::Contract};
    auto group = Group(section, UnitToClaim(currency));

    if (group) { needPrimary = group->Primary().empty(); }

    std::pmr::set<contact::Attribute> attrib{};

    if (active || primary || needPrimary) {
        attrib.emplace(contact::Attribute::Active);
    }

    if (primary || needPrimary) { attrib.emplace(contact::Attribute::Primary); }

    auto version = proto::RequiredVersion(
        translate(section), translate(UnitToClaim(currency)), imp_->version_);

    auto item = std::make_shared<ContactItem>(
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

auto ContactData::AddEmail(
    const std::string& value,
    const bool primary,
    const bool active) const -> ContactData
{
    bool needPrimary{true};
    const contact::SectionType section{contact::SectionType::Communication};
    const contact::ClaimType type{contact::ClaimType::Email};
    auto group = Group(section, type);

    if (group) { needPrimary = group->Primary().empty(); }

    std::pmr::set<contact::Attribute> attrib{};

    if (active || primary || needPrimary) {
        attrib.emplace(contact::Attribute::Active);
    }

    if (primary || needPrimary) { attrib.emplace(contact::Attribute::Primary); }

    auto version = proto::RequiredVersion(
        translate(section), translate(type), imp_->version_);

    auto item = std::make_shared<ContactItem>(
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

auto ContactData::AddItem(const ClaimTuple& claim) const -> ContactData
{
    auto version = proto::RequiredVersion(
        translate(static_cast<contact::SectionType>(std::get<1>(claim))),
        translate(static_cast<contact::ClaimType>(std::get<2>(claim))),
        imp_->version_);

    auto item = std::make_shared<ContactItem>(
        imp_->api_, imp_->nym_, version, version, claim);

    return AddItem(item);
}

auto ContactData::AddItem(const std::shared_ptr<ContactItem>& item) const
    -> ContactData
{
    OT_ASSERT(item);

    const auto& sectionID = item->Section();
    auto map{imp_->sections_};
    auto it = map.find(sectionID);

    auto version = proto::RequiredVersion(
        translate(sectionID), translate(item->Type()), imp_->version_);

    if (map.end() == it) {
        auto& section = map[sectionID];
        section.reset(new ContactSection(
            imp_->api_, imp_->nym_, version, version, sectionID, item));

        OT_ASSERT(section);
    } else {
        auto& section = it->second;

        OT_ASSERT(section);

        section.reset(new ContactSection(section->AddItem(item)));

        OT_ASSERT(section);
    }

    return ContactData(imp_->api_, imp_->nym_, version, version, map);
}

auto ContactData::AddPaymentCode(
    const std::string& code,
    const core::UnitType currency,
    const bool primary,
    const bool active) const -> ContactData
{
    auto needPrimary{true};
    static constexpr auto section{contact::SectionType::Procedure};
    auto group = Group(section, UnitToClaim(currency));

    if (group) { needPrimary = group->Primary().empty(); }

    const auto attrib = [&] {
        auto out = std::pmr::set<contact::Attribute>{};

        if (active || primary || needPrimary) {
            out.emplace(contact::Attribute::Active);
        }

        if (primary || needPrimary) {
            out.emplace(contact::Attribute::Primary);
        }

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

    auto item = std::make_shared<ContactItem>(
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

auto ContactData::AddPhoneNumber(
    const std::string& value,
    const bool primary,
    const bool active) const -> ContactData
{
    bool needPrimary{true};
    const contact::SectionType section{contact::SectionType::Communication};
    const contact::ClaimType type{contact::ClaimType::Phone};
    auto group = Group(section, type);

    if (group) { needPrimary = group->Primary().empty(); }

    std::pmr::set<contact::Attribute> attrib{};

    if (active || primary || needPrimary) {
        attrib.emplace(contact::Attribute::Active);
    }

    if (primary || needPrimary) { attrib.emplace(contact::Attribute::Primary); }

    auto version = proto::RequiredVersion(
        translate(section), translate(type), imp_->version_);

    auto item = std::make_shared<ContactItem>(
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

auto ContactData::AddPreferredOTServer(const Identifier& id, const bool primary)
    const -> ContactData
{
    bool needPrimary{true};
    const contact::SectionType section{contact::SectionType::Communication};
    const contact::ClaimType type{contact::ClaimType::Opentxs};
    auto group = Group(section, type);

    if (group) { needPrimary = group->Primary().empty(); }

    std::pmr::set<contact::Attribute> attrib{contact::Attribute::Active};

    if (primary || needPrimary) { attrib.emplace(contact::Attribute::Primary); }

    auto version = proto::RequiredVersion(
        translate(section), translate(type), imp_->version_);

    auto item = std::make_shared<ContactItem>(
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

auto ContactData::AddSocialMediaProfile(
    const std::string& value,
    const contact::ClaimType type,
    const bool primary,
    const bool active) const -> ContactData
{
    auto map = imp_->sections_;
    // Add the item to the profile section.
    auto& section = map[contact::SectionType::Profile];

    bool needPrimary{true};
    if (section) {
        auto group = section->Group(type);

        if (group) { needPrimary = group->Primary().empty(); }
    }

    std::pmr::set<contact::Attribute> attrib{};

    if (active || primary || needPrimary) {
        attrib.emplace(contact::Attribute::Active);
    }

    if (primary || needPrimary) { attrib.emplace(contact::Attribute::Primary); }

    auto version = proto::RequiredVersion(
        translate(contact::SectionType::Profile),
        translate(type),
        imp_->version_);

    auto item = std::make_shared<ContactItem>(
        imp_->api_,
        imp_->nym_,
        version,
        version,
        contact::SectionType::Profile,
        type,
        value,
        attrib,
        NULL_START,
        NULL_END,
        "");

    OT_ASSERT(item);

    if (section) {
        section.reset(new ContactSection(section->AddItem(item)));
    } else {
        section.reset(new ContactSection(
            imp_->api_,
            imp_->nym_,
            version,
            version,
            contact::SectionType::Profile,
            item));
    }

    OT_ASSERT(section);

    // Add the item to the communication section.
    auto commSectionTypes =
        proto::AllowedItemTypes().at(proto::ContactSectionVersion(
            version, translate(contact::SectionType::Communication)));
    if (commSectionTypes.count(translate(type))) {
        auto& commSection = map[contact::SectionType::Communication];

        if (commSection) {
            auto group = commSection->Group(type);

            if (group) { needPrimary = group->Primary().empty(); }
        }

        attrib.clear();

        if (active || primary || needPrimary) {
            attrib.emplace(contact::Attribute::Active);
        }

        if (primary || needPrimary) {
            attrib.emplace(contact::Attribute::Primary);
        }

        item = std::make_shared<ContactItem>(
            imp_->api_,
            imp_->nym_,
            version,
            version,
            contact::SectionType::Communication,
            type,
            value,
            attrib,
            NULL_START,
            NULL_END,
            "");

        OT_ASSERT(item);

        if (commSection) {
            commSection.reset(new ContactSection(commSection->AddItem(item)));
        } else {
            commSection.reset(new ContactSection(
                imp_->api_,
                imp_->nym_,
                version,
                version,
                contact::SectionType::Communication,
                item));
        }

        OT_ASSERT(commSection);
    }

    // Add the item to the identifier section.
    auto identifierSectionTypes =
        proto::AllowedItemTypes().at(proto::ContactSectionVersion(
            version, translate(contact::SectionType::Identifier)));
    if (identifierSectionTypes.count(translate(type))) {
        auto& identifierSection = map[contact::SectionType::Identifier];

        if (identifierSection) {
            auto group = identifierSection->Group(type);

            if (group) { needPrimary = group->Primary().empty(); }
        }

        attrib.clear();

        if (active || primary || needPrimary) {
            attrib.emplace(contact::Attribute::Active);
        }

        if (primary || needPrimary) {
            attrib.emplace(contact::Attribute::Primary);
        }

        item = std::make_shared<ContactItem>(
            imp_->api_,
            imp_->nym_,
            version,
            version,
            contact::SectionType::Identifier,
            type,
            value,
            attrib,
            NULL_START,
            NULL_END,
            "");

        OT_ASSERT(item);

        if (identifierSection) {
            identifierSection.reset(
                new ContactSection(identifierSection->AddItem(item)));
        } else {
            identifierSection.reset(new ContactSection(
                imp_->api_,
                imp_->nym_,
                version,
                version,
                contact::SectionType::Identifier,
                item));
        }

        OT_ASSERT(identifierSection);
    }

    return ContactData(imp_->api_, imp_->nym_, version, version, map);
}

auto ContactData::begin() const -> ContactData::SectionMap::const_iterator
{
    return imp_->sections_.begin();
}

auto ContactData::BestEmail() const -> std::string
{
    std::string bestEmail;

    auto group =
        Group(contact::SectionType::Communication, contact::ClaimType::Email);

    if (group) {
        std::shared_ptr<ContactItem> best = group->Best();

        if (best) { bestEmail = best->Value(); }
    }

    return bestEmail;
}

auto ContactData::BestPhoneNumber() const -> std::string
{
    std::string bestEmail;

    auto group =
        Group(contact::SectionType::Communication, contact::ClaimType::Phone);

    if (group) {
        std::shared_ptr<ContactItem> best = group->Best();

        if (best) { bestEmail = best->Value(); }
    }

    return bestEmail;
}

auto ContactData::BestSocialMediaProfile(const contact::ClaimType type) const
    -> std::string
{
    std::string bestProfile;

    auto group = Group(contact::SectionType::Profile, type);
    if (group) {
        std::shared_ptr<ContactItem> best = group->Best();

        if (best) { bestProfile = best->Value(); }
    }

    return bestProfile;
}

auto ContactData::Claim(const Identifier& item) const
    -> std::shared_ptr<ContactItem>
{
    for (const auto& it : imp_->sections_) {
        const auto& section = it.second;

        OT_ASSERT(section);

        auto claim = section->Claim(item);

        if (claim) { return claim; }
    }

    return {};
}

auto ContactData::Contracts(
    const core::UnitType currency,
    const bool onlyActive) const -> std::pmr::set<OTIdentifier>
{
    std::pmr::set<OTIdentifier> output{};
    const contact::SectionType section{contact::SectionType::Contract};
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

auto ContactData::Delete(const Identifier& id) const -> ContactData
{
    bool deleted{false};
    auto map = imp_->sections_;

    for (auto& it : map) {
        auto& section = it.second;

        OT_ASSERT(section);

        if (section->HaveClaim(id)) {
            section.reset(new ContactSection(section->Delete(id)));

            OT_ASSERT(section);

            deleted = true;

            if (0 == section->Size()) { map.erase(it.first); }

            break;
        }
    }

    if (false == deleted) { return *this; }

    return ContactData(
        imp_->api_, imp_->nym_, imp_->version_, imp_->version_, map);
}

auto ContactData::EmailAddresses(bool active) const -> std::string
{
    std::ostringstream stream;

    auto group =
        Group(contact::SectionType::Communication, contact::ClaimType::Email);
    if (group) {
        for (const auto& it : *group) {
            OT_ASSERT(it.second);

            const auto& claim = *it.second;

            if ((false == active) || claim.isActive()) {
                stream << claim.Value() << ',';
            }
        }
    }

    std::string output = stream.str();

    if (0 < output.size()) { output.erase(output.size() - 1, 1); }

    return output;
}

auto ContactData::end() const -> ContactData::SectionMap::const_iterator
{
    return imp_->sections_.end();
}

auto ContactData::Group(
    const contact::SectionType section,
    const contact::ClaimType type) const -> std::shared_ptr<ContactGroup>
{
    const auto it = imp_->sections_.find(section);

    if (imp_->sections_.end() == it) { return {}; }

    OT_ASSERT(it->second);

    return it->second->Group(type);
}

auto ContactData::HaveClaim(const Identifier& item) const -> bool
{
    for (const auto& section : imp_->sections_) {
        OT_ASSERT(section.second);

        if (section.second->HaveClaim(item)) { return true; }
    }

    return false;
}

auto ContactData::HaveClaim(
    const contact::SectionType section,
    const contact::ClaimType type,
    const std::string& value) const -> bool
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

auto ContactData::Name() const -> std::string
{
    auto group = imp_->scope().second;

    if (false == bool(group)) { return {}; }

    auto claim = group->Best();

    if (false == bool(claim)) { return {}; }

    return claim->Value();
}

auto ContactData::PhoneNumbers(bool active) const -> std::string
{
    std::ostringstream stream;

    auto group =
        Group(contact::SectionType::Communication, contact::ClaimType::Phone);
    if (group) {
        for (const auto& it : *group) {
            OT_ASSERT(it.second);

            const auto& claim = *it.second;

            if ((false == active) || claim.isActive()) {
                stream << claim.Value() << ',';
            }
        }
    }

    std::string output = stream.str();

    if (0 < output.size()) { output.erase(output.size() - 1, 1); }

    return output;
}

auto ContactData::PreferredOTServer() const -> OTServerID
{
    auto group =
        Group(contact::SectionType::Communication, contact::ClaimType::Opentxs);

    if (false == bool(group)) { return identifier::Server::Factory(); }

    auto claim = group->Best();

    if (false == bool(claim)) { return identifier::Server::Factory(); }

    return identifier::Server::Factory(claim->Value());
}

auto ContactData::PrintContactData(const proto::ContactData& data)
    -> std::string
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
                              static_cast<contact::Attribute>(attribute)))
                       << " ";
            }

            output << std::endl;
        }
    }

    return output.str();
}

auto ContactData::Section(const contact::SectionType section) const
    -> std::shared_ptr<ContactSection>
{
    const auto it = imp_->sections_.find(section);

    if (imp_->sections_.end() == it) { return {}; }

    return it->second;
}

auto ContactData::SetCommonName(const std::string& name) const -> ContactData
{
    const contact::SectionType section{contact::SectionType::Identifier};
    const contact::ClaimType type{contact::ClaimType::Commonname};
    std::pmr::set<contact::Attribute> attrib{
        contact::Attribute::Active, contact::Attribute::Primary};

    auto item = std::make_shared<ContactItem>(
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

auto ContactData::SetName(const std::string& name, const bool primary) const
    -> ContactData
{
    const Imp::Scope& scopeInfo = imp_->scope();

    OT_ASSERT(scopeInfo.second);

    const contact::SectionType section{contact::SectionType::Scope};
    const contact::ClaimType type = scopeInfo.first;

    std::pmr::set<contact::Attribute> attrib{contact::Attribute::Active};

    if (primary) { attrib.emplace(contact::Attribute::Primary); }

    auto item = std::make_shared<ContactItem>(
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

auto ContactData::SetScope(
    const contact::ClaimType type,
    const std::string& name) const -> ContactData
{
    OT_ASSERT(type);

    const contact::SectionType section{contact::SectionType::Scope};

    if (contact::ClaimType::Unknown == imp_->scope().first) {
        auto mapCopy = imp_->sections_;
        mapCopy.erase(section);
        std::pmr::set<contact::Attribute> attrib{
            contact::Attribute::Active, contact::Attribute::Primary};

        auto version = proto::RequiredVersion(
            translate(section), translate(type), imp_->version_);

        auto item = std::make_shared<ContactItem>(
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

        auto newSection = std::make_shared<ContactSection>(
            imp_->api_, imp_->nym_, version, version, section, item);

        OT_ASSERT(newSection);

        mapCopy[section] = newSection;

        return ContactData(imp_->api_, imp_->nym_, version, version, mapCopy);
    } else {
        LogError()(OT_PRETTY_CLASS())("Scope already set.").Flush();

        return *this;
    }
}

auto ContactData::Serialize(AllocateOutput destination, const bool withID) const
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

auto ContactData::Serialize(proto::ContactData& output, const bool withID) const
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

auto ContactData::SocialMediaProfiles(
    const contact::ClaimType type,
    bool active) const -> std::string
{
    std::ostringstream stream;

    auto group = Group(contact::SectionType::Profile, type);
    if (group) {
        for (const auto& it : *group) {
            OT_ASSERT(it.second);

            const auto& claim = *it.second;

            if ((false == active) || claim.isActive()) {
                stream << claim.Value() << ',';
            }
        }
    }

    std::string output = stream.str();

    if (0 < output.size()) { output.erase(output.size() - 1, 1); }

    return output;
}

auto ContactData::SocialMediaProfileTypes() const
    -> const std::pmr::set<contact::ClaimType>
{
    try {
        auto profiletypes =
            proto::AllowedItemTypes().at(proto::ContactSectionVersion(
                CONTACT_CONTACT_DATA_VERSION, proto::CONTACTSECTION_PROFILE));

        std::pmr::set<contact::ClaimType> output;
        std::transform(
            profiletypes.begin(),
            profiletypes.end(),
            std::inserter(output, output.end()),
            [](proto::ContactItemType itemtype) -> contact::ClaimType {
                return translate(itemtype);
            });

        return output;

    } catch (...) {
        return {};
    }
}

auto ContactData::Type() const -> contact::ClaimType
{
    return imp_->scope().first;
}

auto ContactData::Version() const -> VersionNumber { return imp_->version_; }

ContactData::~ContactData() = default;
}  // namespace opentxs::contact
