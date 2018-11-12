// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/contact/ContactData.hpp"
#include "opentxs/contact/ContactGroup.hpp"
#include "opentxs/contact/ContactSection.hpp"
#include "opentxs/contact/ContactItem.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"

#include <sstream>

#define OT_METHOD "opentxs::ContactData::"

namespace opentxs
{
ContactData::ContactData(
    const std::string& nym,
    const std::uint32_t version,
    const std::uint32_t targetVersion,
    const SectionMap& sections)
    : version_(check_version(version, targetVersion))
    , nym_(nym)
    , sections_(sections)
{
    if (0 == version) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Warning: malformed version. "
                                           "Setting to ")(targetVersion)(".")
            .Flush();
    }
}

ContactData::ContactData(
    const std::string& nym,
    const std::uint32_t targetVersion,
    const proto::ContactData& serialized)
    : ContactData(
          nym,
          serialized.version(),
          targetVersion,
          extract_sections(nym, targetVersion, serialized))
{
}

ContactData ContactData::operator+(const ContactData& rhs) const
{
    auto map{sections_};

    for (auto& it : rhs.sections_) {
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

    const auto version = std::max(version_, rhs.Version());

    return ContactData(nym_, version, version, map);
}

ContactData::operator std::string() const
{
    return PrintContactData(Serialize(false));
}

ContactData ContactData::AddContract(
    const std::string& instrumentDefinitionID,
    const proto::ContactItemType currency,
    const bool primary,
    const bool active) const
{
    bool needPrimary{true};
    const proto::ContactSectionName section{proto::CONTACTSECTION_CONTRACT};
    auto group = Group(section, currency);

    if (group) { needPrimary = group->Primary().empty(); }

    std::set<proto::ContactItemAttribute> attrib{};

    if (active || primary || needPrimary) {
        attrib.emplace(proto::CITEMATTR_ACTIVE);
    }

    if (primary || needPrimary) { attrib.emplace(proto::CITEMATTR_PRIMARY); }

    auto version = proto::RequiredVersion(section, currency, version_);

    auto item = std::make_shared<ContactItem>(
        nym_,
        version,
        version,
        section,
        currency,
        instrumentDefinitionID,
        attrib,
        NULL_START,
        NULL_END);

    OT_ASSERT(item);

    return AddItem(item);
}

ContactData ContactData::AddEmail(
    const std::string& value,
    const bool primary,
    const bool active) const
{
    bool needPrimary{true};
    const proto::ContactSectionName section{
        proto::CONTACTSECTION_COMMUNICATION};
    const proto::ContactItemType type{proto::CITEMTYPE_EMAIL};
    auto group = Group(section, type);

    if (group) { needPrimary = group->Primary().empty(); }

    std::set<proto::ContactItemAttribute> attrib{};

    if (active || primary || needPrimary) {
        attrib.emplace(proto::CITEMATTR_ACTIVE);
    }

    if (primary || needPrimary) { attrib.emplace(proto::CITEMATTR_PRIMARY); }

    auto version = proto::RequiredVersion(section, type, version_);

    auto item = std::make_shared<ContactItem>(
        nym_,
        version,
        version,
        section,
        type,
        value,
        attrib,
        NULL_START,
        NULL_END);

    OT_ASSERT(item);

    return AddItem(item);
}

ContactData ContactData::AddItem(const ClaimTuple& claim) const
{
    auto version = proto::RequiredVersion(
        static_cast<proto::ContactSectionName>(std::get<1>(claim)),
        static_cast<proto::ContactItemType>(std::get<2>(claim)),
        version_);

    auto item = std::make_shared<ContactItem>(nym_, version, version, claim);

    return AddItem(item);
}

ContactData ContactData::AddItem(const std::shared_ptr<ContactItem>& item) const
{
    OT_ASSERT(item);

    const auto& sectionID = item->Section();
    auto map{sections_};
    auto it = map.find(sectionID);

    auto version = proto::RequiredVersion(sectionID, item->Type(), version_);

    if (map.end() == it) {
        auto& section = map[sectionID];
        section.reset(
            new ContactSection(nym_, version, version, sectionID, item));

        OT_ASSERT(section);
    } else {
        auto& section = it->second;

        OT_ASSERT(section);

        section.reset(new ContactSection(section->AddItem(item)));

        OT_ASSERT(section);
    }

    return ContactData(nym_, version, version, map);
}

ContactData ContactData::AddPaymentCode(
    const std::string& code,
    const proto::ContactItemType currency,
    const bool primary,
    const bool active) const
{
    bool needPrimary{true};
    const proto::ContactSectionName section{proto::CONTACTSECTION_PROCEDURE};
    auto group = Group(section, currency);

    if (group) { needPrimary = group->Primary().empty(); }

    std::set<proto::ContactItemAttribute> attrib{};

    if (active || primary || needPrimary) {
        attrib.emplace(proto::CITEMATTR_ACTIVE);
    }

    if (primary || needPrimary) { attrib.emplace(proto::CITEMATTR_PRIMARY); }

    auto version = proto::RequiredVersion(section, currency, version_);

    auto item = std::make_shared<ContactItem>(
        nym_,
        version,
        version,
        section,
        currency,
        code,
        attrib,
        NULL_START,
        NULL_END);

    OT_ASSERT(item);

    return AddItem(item);
}

ContactData ContactData::AddPhoneNumber(
    const std::string& value,
    const bool primary,
    const bool active) const
{
    bool needPrimary{true};
    const proto::ContactSectionName section{
        proto::CONTACTSECTION_COMMUNICATION};
    const proto::ContactItemType type{proto::CITEMTYPE_PHONE};
    auto group = Group(section, type);

    if (group) { needPrimary = group->Primary().empty(); }

    std::set<proto::ContactItemAttribute> attrib{};

    if (active || primary || needPrimary) {
        attrib.emplace(proto::CITEMATTR_ACTIVE);
    }

    if (primary || needPrimary) { attrib.emplace(proto::CITEMATTR_PRIMARY); }

    auto version = proto::RequiredVersion(section, type, version_);

    auto item = std::make_shared<ContactItem>(
        nym_,
        version,
        version,
        section,
        type,
        value,
        attrib,
        NULL_START,
        NULL_END);

    OT_ASSERT(item);

    return AddItem(item);
}

ContactData ContactData::AddPreferredOTServer(
    const Identifier& id,
    const bool primary) const
{
    bool needPrimary{true};
    const proto::ContactSectionName section{
        proto::CONTACTSECTION_COMMUNICATION};
    const proto::ContactItemType type{proto::CITEMTYPE_OPENTXS};
    auto group = Group(section, type);

    if (group) { needPrimary = group->Primary().empty(); }

    std::set<proto::ContactItemAttribute> attrib{proto::CITEMATTR_ACTIVE};

    if (primary || needPrimary) { attrib.emplace(proto::CITEMATTR_PRIMARY); }

    auto version = proto::RequiredVersion(section, type, version_);

    auto item = std::make_shared<ContactItem>(
        nym_,
        version,
        version,
        section,
        type,
        String::Factory(id)->Get(),
        attrib,
        NULL_START,
        NULL_END);

    OT_ASSERT(item);

    return AddItem(item);
}

ContactData ContactData::AddSocialMediaProfile(
    const std::string& value,
    const proto::ContactItemType type,
    const bool primary,
    const bool active) const
{
    auto map = sections_;
    // Add the item to the profile section.
    auto& section = map[proto::CONTACTSECTION_PROFILE];

    bool needPrimary{true};
    if (section) {
        auto group = section->Group(type);

        if (group) { needPrimary = group->Primary().empty(); }
    }

    std::set<proto::ContactItemAttribute> attrib{};

    if (active || primary || needPrimary) {
        attrib.emplace(proto::CITEMATTR_ACTIVE);
    }

    if (primary || needPrimary) { attrib.emplace(proto::CITEMATTR_PRIMARY); }

    auto version =
        proto::RequiredVersion(proto::CONTACTSECTION_PROFILE, type, version_);

    auto item = std::make_shared<ContactItem>(
        nym_,
        version,
        version,
        proto::CONTACTSECTION_PROFILE,
        type,
        value,
        attrib,
        NULL_START,
        NULL_END);

    OT_ASSERT(item);

    if (section) {
        section.reset(new ContactSection(section->AddItem(item)));
    } else {
        section.reset(new ContactSection(
            nym_, version, version, proto::CONTACTSECTION_PROFILE, item));
    }

    OT_ASSERT(section);

    // Add the item to the communication section.
    auto commSectionTypes =
        proto::AllowedItemTypes.at(proto::ContactSectionVersion(
            version, proto::CONTACTSECTION_COMMUNICATION));
    if (commSectionTypes.count(type)) {
        auto& commSection = map[proto::CONTACTSECTION_COMMUNICATION];

        if (commSection) {
            auto group = commSection->Group(type);

            if (group) { needPrimary = group->Primary().empty(); }
        }

        attrib.clear();

        if (active || primary || needPrimary) {
            attrib.emplace(proto::CITEMATTR_ACTIVE);
        }

        if (primary || needPrimary) {
            attrib.emplace(proto::CITEMATTR_PRIMARY);
        }

        item = std::make_shared<ContactItem>(
            nym_,
            version,
            version,
            proto::CONTACTSECTION_COMMUNICATION,
            type,
            value,
            attrib,
            NULL_START,
            NULL_END);

        OT_ASSERT(item);

        if (commSection) {
            commSection.reset(new ContactSection(commSection->AddItem(item)));
        } else {
            commSection.reset(new ContactSection(
                nym_,
                version,
                version,
                proto::CONTACTSECTION_COMMUNICATION,
                item));
        }

        OT_ASSERT(commSection);
    }

    // Add the item to the identifier section.
    auto identifierSectionTypes =
        proto::AllowedItemTypes.at(proto::ContactSectionVersion(
            version, proto::CONTACTSECTION_IDENTIFIER));
    if (identifierSectionTypes.count(type)) {
        auto& identifierSection = map[proto::CONTACTSECTION_IDENTIFIER];

        if (identifierSection) {
            auto group = identifierSection->Group(type);

            if (group) { needPrimary = group->Primary().empty(); }
        }

        attrib.clear();

        if (active || primary || needPrimary) {
            attrib.emplace(proto::CITEMATTR_ACTIVE);
        }

        if (primary || needPrimary) {
            attrib.emplace(proto::CITEMATTR_PRIMARY);
        }

        item = std::make_shared<ContactItem>(
            nym_,
            version,
            version,
            proto::CONTACTSECTION_IDENTIFIER,
            type,
            value,
            attrib,
            NULL_START,
            NULL_END);

        OT_ASSERT(item);

        if (identifierSection) {
            identifierSection.reset(
                new ContactSection(identifierSection->AddItem(item)));
        } else {
            identifierSection.reset(new ContactSection(
                nym_,
                version,
                version,
                proto::CONTACTSECTION_IDENTIFIER,
                item));
        }

        OT_ASSERT(identifierSection);
    }

    return ContactData(nym_, version, version, map);
}

std::string ContactData::BestEmail() const
{
    std::string bestEmail;

    auto group =
        Group(proto::CONTACTSECTION_COMMUNICATION, proto::CITEMTYPE_EMAIL);

    if (group) {
        std::shared_ptr<ContactItem> best = group->Best();

        if (best) { bestEmail = best->Value(); }
    }

    return bestEmail;
}

std::string ContactData::BestPhoneNumber() const
{
    std::string bestEmail;

    auto group =
        Group(proto::CONTACTSECTION_COMMUNICATION, proto::CITEMTYPE_PHONE);

    if (group) {
        std::shared_ptr<ContactItem> best = group->Best();

        if (best) { bestEmail = best->Value(); }
    }

    return bestEmail;
}

std::string ContactData::BestSocialMediaProfile(
    const proto::ContactItemType type) const
{
    std::string bestProfile;

    auto group = Group(proto::CONTACTSECTION_PROFILE, type);
    if (group) {
        std::shared_ptr<ContactItem> best = group->Best();

        if (best) { bestProfile = best->Value(); }
    }

    return bestProfile;
}

std::uint32_t ContactData::check_version(
    const std::uint32_t in,
    const std::uint32_t targetVersion)
{
    // Upgrade version
    if (targetVersion > in) { return targetVersion; }

    return in;
}

std::shared_ptr<ContactItem> ContactData::Claim(const Identifier& item) const
{
    for (const auto& it : sections_) {
        const auto& section = it.second;

        OT_ASSERT(section);

        auto claim = section->Claim(item);

        if (claim) { return claim; }
    }

    return {};
}

std::set<OTIdentifier> ContactData::Contracts(
    const proto::ContactItemType currency,
    const bool onlyActive) const
{
    std::set<OTIdentifier> output{};
    const proto::ContactSectionName section{proto::CONTACTSECTION_CONTRACT};
    auto group = Group(section, currency);

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

ContactData ContactData::Delete(const Identifier& id) const
{
    bool deleted{false};
    auto map = sections_;

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

    return ContactData(nym_, version_, version_, map);
}

std::string ContactData::EmailAddresses(bool active) const
{
    std::ostringstream stream;

    auto group =
        Group(proto::CONTACTSECTION_COMMUNICATION, proto::CITEMTYPE_EMAIL);
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

ContactData::SectionMap ContactData::extract_sections(
    const std::string& nym,
    const std::uint32_t targetVersion,
    const proto::ContactData& serialized)
{
    SectionMap sectionMap{};

    for (const auto it : serialized.section()) {
        if ((0 != it.version()) && (it.item_size() > 0)) {
            sectionMap[it.name()].reset(new ContactSection(
                nym, check_version(serialized.version(), targetVersion), it));
        }
    }

    return sectionMap;
}

std::shared_ptr<ContactGroup> ContactData::Group(
    const proto::ContactSectionName& section,
    const proto::ContactItemType& type) const
{
    const auto it = sections_.find(section);

    if (sections_.end() == it) { return {}; }

    OT_ASSERT(it->second);

    return it->second->Group(type);
}

bool ContactData::HaveClaim(const Identifier& item) const
{
    for (const auto& section : sections_) {
        OT_ASSERT(section.second);

        if (section.second->HaveClaim(item)) { return true; }
    }

    return false;
}

bool ContactData::HaveClaim(
    const proto::ContactSectionName& section,
    const proto::ContactItemType& type,
    const std::string& value) const
{
    auto group = Group(section, type);

    if (false == bool(group)) { return false; }

    for (const auto it : *group) {
        OT_ASSERT(it.second);

        const auto& claim = *it.second;

        if (value == claim.Value()) { return true; }
    }

    return false;
}

std::string ContactData::Name() const
{
    auto group = scope().second;

    if (false == bool(group)) { return {}; }

    auto claim = group->Best();

    if (false == bool(claim)) { return {}; }

    return claim->Value();
}

std::string ContactData::PhoneNumbers(bool active) const
{
    std::ostringstream stream;

    auto group =
        Group(proto::CONTACTSECTION_COMMUNICATION, proto::CITEMTYPE_PHONE);
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

OTIdentifier ContactData::PreferredOTServer() const
{
    auto group =
        Group(proto::CONTACTSECTION_COMMUNICATION, proto::CITEMTYPE_OPENTXS);

    if (false == bool(group)) { return Identifier::Factory(); }

    auto claim = group->Best();

    if (false == bool(claim)) { return Identifier::Factory(); }

    return Identifier::Factory(claim->Value());
}

std::string ContactData::PrintContactData(const proto::ContactData& data)
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
                output << proto::TranslateItemAttributes(
                              static_cast<proto::ContactItemAttribute>(
                                  attribute))
                       << " ";
            }

            output << std::endl;
        }
    }

    return output.str();
}

std::shared_ptr<ContactSection> ContactData::Section(
    const proto::ContactSectionName& section) const
{
    const auto it = sections_.find(section);

    if (sections_.end() == it) { return {}; }

    return it->second;
}

ContactData ContactData::SetCommonName(const std::string& name) const
{
    const proto::ContactSectionName section{proto::CONTACTSECTION_IDENTIFIER};
    const proto::ContactItemType type{proto::CITEMTYPE_COMMONNAME};
    std::set<proto::ContactItemAttribute> attrib{proto::CITEMATTR_ACTIVE,
                                                 proto::CITEMATTR_PRIMARY};

    auto item = std::make_shared<ContactItem>(
        nym_,
        version_,
        version_,
        section,
        type,
        name,
        attrib,
        NULL_START,
        NULL_END);

    OT_ASSERT(item);

    return AddItem(item);
}

ContactData ContactData::SetName(const std::string& name, const bool primary)
    const
{
    const Scope& scopeInfo = scope();

    OT_ASSERT(scopeInfo.second);

    const proto::ContactSectionName section{proto::CONTACTSECTION_SCOPE};
    const proto::ContactItemType type = scopeInfo.first;

    std::set<proto::ContactItemAttribute> attrib{proto::CITEMATTR_ACTIVE};

    if (primary) { attrib.emplace(proto::CITEMATTR_PRIMARY); }

    auto item = std::make_shared<ContactItem>(
        nym_,
        version_,
        version_,
        section,
        type,
        name,
        attrib,
        NULL_START,
        NULL_END);

    OT_ASSERT(item);

    return AddItem(item);
}

ContactData ContactData::SetScope(
    const proto::ContactItemType type,
    const std::string& name) const
{
    OT_ASSERT(type);

    const proto::ContactSectionName section{proto::CONTACTSECTION_SCOPE};

    if (proto::CITEMTYPE_UNKNOWN == scope().first) {
        auto mapCopy = sections_;
        mapCopy.erase(section);
        std::set<proto::ContactItemAttribute> attrib{proto::CITEMATTR_ACTIVE,
                                                     proto::CITEMATTR_PRIMARY};

        auto version = proto::RequiredVersion(section, type, version_);

        auto item = std::make_shared<ContactItem>(
            nym_,
            version,
            version,
            section,
            type,
            name,
            attrib,
            NULL_START,
            NULL_END);

        OT_ASSERT(item);

        auto newSection = std::make_shared<ContactSection>(
            nym_, version, version, section, item);

        OT_ASSERT(newSection);

        mapCopy[section] = newSection;

        return ContactData(nym_, version, version, mapCopy);
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Scope already set.").Flush();
        return *this;
    }
}

ContactData::Scope ContactData::scope() const
{
    const auto it = sections_.find(proto::CONTACTSECTION_SCOPE);

    if (sections_.end() == it) { return {proto::CITEMTYPE_UNKNOWN, nullptr}; }

    OT_ASSERT(it->second);

    const auto& section = *it->second;

    if (1 != section.Size()) { return {proto::CITEMTYPE_ERROR, nullptr}; }

    return *section.begin();
}

proto::ContactData ContactData::Serialize(const bool withID) const
{
    proto::ContactData output;
    output.set_version(version_);

    for (const auto& it : sections_) {
        const auto& section = it.second;

        OT_ASSERT(section);

        section->SerializeTo(output, withID);
    }

    return output;
}

std::string ContactData::SocialMediaProfiles(
    const proto::ContactItemType type,
    bool active) const
{
    std::ostringstream stream;

    auto group = Group(proto::CONTACTSECTION_PROFILE, type);
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

const std::set<proto::ContactItemType> ContactData::SocialMediaProfileTypes()
    const
{
    return proto::AllowedItemTypes.at(proto::ContactSectionVersion(
        CONTACT_CONTACT_DATA_VERSION, proto::CONTACTSECTION_PROFILE));
}

proto::ContactItemType ContactData::Type() const { return scope().first; }

std::uint32_t ContactData::Version() const { return version_; }
}  // namespace opentxs
