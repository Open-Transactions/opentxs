// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/contact/ContactData.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstdint>
#include <memory>

#include "internal/identity/wot/claim/Types.hpp"
#include "ottest/env/OTTestEnvironment.hpp"

namespace ottest
{
namespace ot = opentxs;
namespace claim = ot::identity::wot::claim;
using namespace std::literals;

ContactData::ContactData()
    : api_(OTTestEnvironment::GetOT().StartClientSession(0))
    , nym_id_1_(api_.Factory().NymIDFromRandom())
    , nym_id_2_(api_.Factory().NymIDFromRandom())
    , nym_id_3_(api_.Factory().NymIDFromRandom())
    , nym_id_4_(api_.Factory().NymIDFromRandom())
    , contact_data_(
          api_,
          nym_id_1_.asBase58(api_.Crypto()),
          opentxs::CONTACT_CONTACT_DATA_VERSION,
          opentxs::CONTACT_CONTACT_DATA_VERSION,
          {})
    , active_contact_item_(std::make_shared<claim::Item>(
          api_,
          "activeContactItem"s,
          opentxs::CONTACT_CONTACT_DATA_VERSION,
          opentxs::CONTACT_CONTACT_DATA_VERSION,
          claim::SectionType::Identifier,
          claim::ClaimType::Employee,
          "activeContactItemValue"s,
          ot::UnallocatedSet<claim::Attribute>{claim::Attribute::Active},
          ot::Time{},
          ot::Time{},
          ""))
{
}

void ContactData::testAddItemMethod(
    const CallbackType1 contactDataMethod,
    claim::SectionType sectionName,
    std::uint32_t version,
    std::uint32_t targetVersion)
{
    // Add a contact to a group with no primary.
    using Group = claim::Group;
    using Section = claim::Section;
    const auto group1 = std::make_shared<Group>(
        "contactGroup1", sectionName, claim::ClaimType::Bch, Group::ItemMap{});
    const auto section1 = std::make_shared<Section>(
        api_,
        "contactSectionNym1",
        version,
        version,
        sectionName,
        Section::GroupMap{{claim::ClaimType::Bch, group1}});
    const auto data1 = claim::Data{
        api_,
        nym_id_2_.asBase58(api_.Crypto()),
        version,
        version,
        claim::Data::SectionMap{{sectionName, section1}}};

    const auto data2 = contactDataMethod(
        data1, "instrumentDefinitionID1", ot::UnitType::Bch, false, false);

    if (targetVersion) {
        ASSERT_EQ(targetVersion, data2.Version());
        return;
    }

    // Verify that the item was made primary.
    const ot::identifier::Generic identifier1(
        api_.Factory().IdentifierFromBase58(
            ot::identity::credential::Contact::ClaimID(
                api_,
                nym_id_2_.asBase58(api_.Crypto()),
                sectionName,
                claim::ClaimType::Bch,
                {},
                {},
                "instrumentDefinitionID1",
                "")));
    const auto contactItem1 = data2.Claim(identifier1);
    ASSERT_NE(nullptr, contactItem1);
    ASSERT_TRUE(contactItem1->isPrimary());

    // Add a contact to a group with a primary.
    const auto data3 = contactDataMethod(
        data2, "instrumentDefinitionID2", ot::UnitType::Bch, false, false);

    // Verify that the item wasn't made primary.
    const ot::identifier::Generic identifier2(
        api_.Factory().IdentifierFromBase58(
            ot::identity::credential::Contact::ClaimID(
                api_,
                nym_id_2_.asBase58(api_.Crypto()),
                sectionName,
                claim::ClaimType::Bch,
                {},
                {},
                "instrumentDefinitionID2",
                "")));
    const auto contactItem2 = data3.Claim(identifier2);
    ASSERT_NE(nullptr, contactItem2);
    ASSERT_FALSE(contactItem2->isPrimary());

    // Add a contact for a type with no group.
    const auto data4 = contactDataMethod(
        data3, "instrumentDefinitionID3", ot::UnitType::Eur, false, false);

    // Verify the group was created.
    ASSERT_NE(nullptr, data4.Group(sectionName, claim::ClaimType::Eur));
    // Verify that the item was made primary.
    const ot::identifier::Generic identifier3(
        api_.Factory().IdentifierFromBase58(
            ot::identity::credential::Contact::ClaimID(
                api_,
                nym_id_2_.asBase58(api_.Crypto()),
                sectionName,
                claim::ClaimType::Eur,
                {},
                {},
                "instrumentDefinitionID3",
                "")));
    const auto contactItem3 = data4.Claim(identifier3);
    ASSERT_NE(nullptr, contactItem3);
    ASSERT_TRUE(contactItem3->isPrimary());

    // Add an active contact.
    const auto data5 = contactDataMethod(
        data4, "instrumentDefinitionID4", ot::UnitType::Usd, false, true);

    // Verify the group was created.
    ASSERT_NE(nullptr, data5.Group(sectionName, claim::ClaimType::Usd));
    // Verify that the item was made active.
    const ot::identifier::Generic identifier4(
        api_.Factory().IdentifierFromBase58(
            ot::identity::credential::Contact::ClaimID(
                api_,
                nym_id_2_.asBase58(api_.Crypto()),
                sectionName,
                claim::ClaimType::Usd,
                {},
                {},
                "instrumentDefinitionID4",
                "")));
    const auto contactItem4 = data5.Claim(identifier4);
    ASSERT_NE(nullptr, contactItem4);
    ASSERT_TRUE(contactItem4->isActive());

    // Add a primary contact.
    const auto data6 = contactDataMethod(
        data5, "instrumentDefinitionID5", ot::UnitType::Usd, true, false);

    // Verify that the item was made primary.
    const ot::identifier::Generic identifier5(
        api_.Factory().IdentifierFromBase58(
            ot::identity::credential::Contact::ClaimID(
                api_,
                nym_id_2_.asBase58(api_.Crypto()),
                sectionName,
                claim::ClaimType::Usd,
                {},
                {},
                "instrumentDefinitionID5",
                "")));
    const auto contactItem5 = data6.Claim(identifier5);
    ASSERT_NE(nullptr, contactItem5);
    ASSERT_TRUE(contactItem5->isPrimary());
}

void ContactData::testAddItemMethod2(
    const CallbackType2 contactDataMethod,
    claim::SectionType sectionName,
    claim::ClaimType itemType,
    std::uint32_t version,
    std::uint32_t targetVersion)
{
    // Add a contact to a group with no primary.
    using Group = claim::Group;
    using Section = claim::Section;
    const auto group1 = std::make_shared<Group>(
        "contactGroup1", sectionName, itemType, Group::ItemMap{});
    const auto section1 = std::make_shared<Section>(
        api_,
        "contactSectionNym1",
        version,
        version,
        sectionName,
        Section::GroupMap{{itemType, group1}});

    const claim::Data data1(
        api_,
        nym_id_2_.asBase58(api_.Crypto()),
        version,
        version,
        claim::Data::SectionMap{{sectionName, section1}});

    const auto data2 = contactDataMethod(data1, "contactValue1", false, false);

    if (targetVersion) {
        ASSERT_EQ(targetVersion, data2.Version());
        return;
    }

    // Verify that the item was made primary.
    const ot::identifier::Generic identifier1(
        api_.Factory().IdentifierFromBase58(
            ot::identity::credential::Contact::ClaimID(
                api_,
                nym_id_2_.asBase58(api_.Crypto()),
                sectionName,
                itemType,
                {},
                {},
                "contactValue1",
                "")));
    const auto contactItem1 = data2.Claim(identifier1);
    ASSERT_NE(nullptr, contactItem1);
    ASSERT_TRUE(contactItem1->isPrimary());

    // Add a contact to a group with a primary.
    const auto data3 = contactDataMethod(data2, "contactValue2", false, false);

    // Verify that the item wasn't made primary.
    const ot::identifier::Generic identifier2(
        api_.Factory().IdentifierFromBase58(
            ot::identity::credential::Contact::ClaimID(
                api_,
                nym_id_2_.asBase58(api_.Crypto()),
                sectionName,
                itemType,
                {},
                {},
                "contactValue2",
                "")));
    const auto contactItem2 = data3.Claim(identifier2);
    ASSERT_NE(nullptr, contactItem2);
    ASSERT_FALSE(contactItem2->isPrimary());

    // Add a contact for a type with no group.
    const auto section2 = std::make_shared<claim::Section>(

        api_,
        "contactSectionNym2",
        version,
        version,
        sectionName,
        claim::Section::GroupMap{});

    const claim::Data data4(
        api_,
        nym_id_4_.asBase58(api_.Crypto()),
        version,
        version,
        claim::Data::SectionMap{{sectionName, section2}});

    const auto data5 = contactDataMethod(data4, "contactValue3", false, false);

    // Verify the group was created.
    ASSERT_NE(nullptr, data5.Group(sectionName, itemType));
    // Verify that the item was made primary.
    const ot::identifier::Generic identifier3(
        api_.Factory().IdentifierFromBase58(
            ot::identity::credential::Contact::ClaimID(
                api_,
                nym_id_4_.asBase58(api_.Crypto()),
                sectionName,
                itemType,
                {},
                {},
                "contactValue3",
                "")));
    const auto contactItem3 = data5.Claim(identifier3);
    ASSERT_NE(nullptr, contactItem3);
    ASSERT_TRUE(contactItem3->isPrimary());

    // Add an active contact.
    const auto data6 = contactDataMethod(data5, "contactValue4", false, true);

    // Verify that the item was made active.
    const ot::identifier::Generic identifier4(
        api_.Factory().IdentifierFromBase58(
            ot::identity::credential::Contact::ClaimID(
                api_,
                nym_id_4_.asBase58(api_.Crypto()),
                sectionName,
                itemType,
                {},
                {},
                "contactValue4",
                "")));
    const auto contactItem4 = data6.Claim(identifier4);
    ASSERT_NE(nullptr, contactItem4);
    ASSERT_TRUE(contactItem4->isActive());
}
}  // namespace ottest
