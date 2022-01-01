// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

// IWYU pragma: no_include "opentxs/contact/ClaimType.hpp"
// IWYU pragma: no_include "opentxs/contact/SectionType.hpp"
// IWYU pragma: no_include "opentxs/core/UnitType.hpp"

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <map>
#include <memory>
#include <set>
#include <string>
#include <tuple>
#include <utility>

#include "opentxs/Types.hpp"
#include "opentxs/contact/Types.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Numbers.hpp"

namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace contact
{
class ContactGroup;
class ContactItem;
class ContactSection;
}  // namespace contact

namespace proto
{
class ContactData;
}  // namespace proto
}  // namespace opentxs

namespace opentxs::contact
{
class OPENTXS_EXPORT ContactData
{
public:
    using SectionMap =
        std::pmr::map<contact::SectionType, std::shared_ptr<ContactSection>>;

    OPENTXS_NO_EXPORT static auto PrintContactData(
        const proto::ContactData& data) -> std::string;

    ContactData(
        const api::Session& api,
        const std::string& nym,
        const VersionNumber version,
        const VersionNumber targetVersion,
        const SectionMap& sections);
    OPENTXS_NO_EXPORT ContactData(
        const api::Session& api,
        const std::string& nym,
        const VersionNumber targetVersion,
        const proto::ContactData& serialized);
    ContactData(
        const api::Session& api,
        const std::string& nym,
        const VersionNumber targetVersion,
        const ReadView& serialized);
    ContactData(const ContactData&);

    auto operator+(const ContactData& rhs) const -> ContactData;

    operator std::string() const;

    auto AddContract(
        const std::string& instrumentDefinitionID,
        const core::UnitType currency,
        const bool primary,
        const bool active) const -> ContactData;
    auto AddEmail(
        const std::string& value,
        const bool primary,
        const bool active) const -> ContactData;
    auto AddItem(const Claim& claim) const -> ContactData;
    auto AddItem(const std::shared_ptr<ContactItem>& item) const -> ContactData;
    auto AddPaymentCode(
        const std::string& code,
        const core::UnitType currency,
        const bool primary,
        const bool active) const -> ContactData;
    auto AddPhoneNumber(
        const std::string& value,
        const bool primary,
        const bool active) const -> ContactData;
    auto AddPreferredOTServer(const Identifier& id, const bool primary) const
        -> ContactData;
    auto AddSocialMediaProfile(
        const std::string& value,
        const contact::ClaimType type,
        const bool primary,
        const bool active) const -> ContactData;
    auto begin() const -> SectionMap::const_iterator;
    auto BestEmail() const -> std::string;
    auto BestPhoneNumber() const -> std::string;
    auto BestSocialMediaProfile(const contact::ClaimType type) const
        -> std::string;
    auto Claim(const Identifier& item) const -> std::shared_ptr<ContactItem>;
    auto Contracts(const core::UnitType currency, const bool onlyActive) const
        -> std::pmr::set<OTIdentifier>;
    auto Delete(const Identifier& id) const -> ContactData;
    auto EmailAddresses(bool active = true) const -> std::string;
    auto end() const -> SectionMap::const_iterator;
    auto Group(
        const contact::SectionType section,
        const contact::ClaimType type) const -> std::shared_ptr<ContactGroup>;
    auto HaveClaim(const Identifier& item) const -> bool;
    auto HaveClaim(
        const contact::SectionType section,
        const contact::ClaimType type,
        const std::string& value) const -> bool;
    auto Name() const -> std::string;
    auto PhoneNumbers(bool active = true) const -> std::string;
    auto PreferredOTServer() const -> OTServerID;
    auto Section(const contact::SectionType section) const
        -> std::shared_ptr<ContactSection>;
    auto Serialize(AllocateOutput destination, const bool withID = false) const
        -> bool;
    OPENTXS_NO_EXPORT auto Serialize(
        proto::ContactData& out,
        const bool withID = false) const -> bool;
    auto SetCommonName(const std::string& name) const -> ContactData;
    auto SetName(const std::string& name, const bool primary = true) const
        -> ContactData;
    auto SetScope(const contact::ClaimType type, const std::string& name) const
        -> ContactData;
    auto SocialMediaProfiles(const contact::ClaimType type, bool active = true)
        const -> std::string;
    auto SocialMediaProfileTypes() const
        -> const std::pmr::set<contact::ClaimType>;
    auto Type() const -> contact::ClaimType;
    auto Version() const -> VersionNumber;

    ~ContactData();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;

    ContactData() = delete;
    ContactData(ContactData&&) = delete;
    auto operator=(const ContactData&) -> ContactData& = delete;
    auto operator=(ContactData&&) -> ContactData& = delete;
};
}  // namespace opentxs::contact
