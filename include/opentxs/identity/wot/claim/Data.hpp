// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace identifier
{
class Generic;
class Notary;
}  // namespace identifier

namespace identity
{
namespace wot
{
namespace claim
{
class Group;
class Item;
class Section;
}  // namespace claim

class Claim;
}  // namespace wot
}  // namespace identity

namespace protobuf
{
class ContactData;
}  // namespace protobuf

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::identity::wot::claim
{
class OPENTXS_EXPORT Data
{
public:
    using SectionMap =
        UnallocatedMap<claim::SectionType, std::shared_ptr<claim::Section>>;

    OPENTXS_NO_EXPORT static auto PrintContactData(
        const protobuf::ContactData& data) -> UnallocatedCString;

    Data(
        const api::Session& api,
        const UnallocatedCString& nym,
        const VersionNumber version,
        const VersionNumber targetVersion,
        const SectionMap& sections);
    OPENTXS_NO_EXPORT Data(
        const api::Session& api,
        const UnallocatedCString& nym,
        const VersionNumber targetVersion,
        const protobuf::ContactData& serialized);
    Data(
        const api::Session& api,
        const UnallocatedCString& nym,
        const VersionNumber targetVersion,
        const ReadView& serialized);
    Data(const Data&);

    auto operator+(const Data& rhs) const -> Data;

    operator UnallocatedCString() const;

    auto AddContract(
        const UnallocatedCString& instrumentDefinitionID,
        const UnitType currency,
        const bool primary,
        const bool active) const -> Data;
    auto AddEmail(
        const UnallocatedCString& value,
        const bool primary,
        const bool active) const -> Data;
    auto AddItem(const wot::Claim& claim) const -> Data;
    auto AddItem(const std::shared_ptr<Item>& item) const -> Data;
    auto AddPaymentCode(
        const UnallocatedCString& code,
        const UnitType currency,
        const bool primary,
        const bool active) const -> Data;
    auto AddPhoneNumber(
        const UnallocatedCString& value,
        const bool primary,
        const bool active) const -> Data;
    auto AddPreferredOTServer(const identifier::Generic& id, const bool primary)
        const -> Data;
    auto AddSocialMediaProfile(
        const UnallocatedCString& value,
        const claim::ClaimType type,
        const bool primary,
        const bool active) const -> Data;
    auto begin() const -> SectionMap::const_iterator;
    auto BestEmail() const -> UnallocatedCString;
    auto BestPhoneNumber() const -> UnallocatedCString;
    auto BestSocialMediaProfile(const claim::ClaimType type) const
        -> UnallocatedCString;
    auto Claim(const identifier::Generic& item) const -> std::shared_ptr<Item>;
    auto Contracts(const UnitType currency, const bool onlyActive) const
        -> UnallocatedSet<identifier::Generic>;
    auto Delete(const identifier::Generic& id) const -> Data;
    auto EmailAddresses(bool active = true) const -> UnallocatedCString;
    auto end() const -> SectionMap::const_iterator;
    auto Group(const claim::SectionType section, const claim::ClaimType type)
        const -> std::shared_ptr<Group>;
    auto HaveClaim(const identifier::Generic& item) const -> bool;
    auto HaveClaim(
        const claim::SectionType section,
        const claim::ClaimType type,
        std::string_view value) const -> bool;
    auto Name() const -> UnallocatedCString;
    auto PhoneNumbers(bool active = true) const -> UnallocatedCString;
    auto PreferredOTServer() const -> identifier::Notary;
    auto Section(const claim::SectionType section) const
        -> std::shared_ptr<Section>;
    auto Serialize(Writer&& destination, const bool withID = false) const
        -> bool;
    OPENTXS_NO_EXPORT auto Serialize(
        protobuf::ContactData& out,
        const bool withID = false) const -> bool;
    auto SetCommonName(const UnallocatedCString& name) const -> Data;
    auto SetName(const UnallocatedCString& name, const bool primary = true)
        const -> Data;
    auto SetScope(const claim::ClaimType type, const UnallocatedCString& name)
        const -> Data;
    auto SocialMediaProfiles(const claim::ClaimType type, bool active = true)
        const -> UnallocatedCString;
    auto SocialMediaProfileTypes() const
        -> const UnallocatedSet<claim::ClaimType>;
    auto Type() const -> claim::ClaimType;
    auto Version() const -> VersionNumber;

    Data() = delete;
    Data(Data&&) = delete;
    auto operator=(const Data&) -> Data& = delete;
    auto operator=(Data&&) -> Data& = delete;

    ~Data();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;
};
}  // namespace opentxs::identity::wot::claim
