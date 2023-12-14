// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/contact/ContactData.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <memory>
#include <span>
#include <utility>

#include "ottest/fixtures/contact/ContactItem.hpp"

namespace ottest
{
namespace ot = opentxs;
namespace claim = ot::identity::wot::claim;
using namespace std::literals;

ContactData::ContactData()
    : nym_id_1_(client_1_.Factory().NymIDFromRandom())
    , nym_id_2_(client_1_.Factory().NymIDFromRandom())
    , nym_id_3_(client_1_.Factory().NymIDFromRandom())
    , nym_id_4_(client_1_.Factory().NymIDFromRandom())
    , contact_data_(
          client_1_,
          nym_id_1_.asBase58(client_1_.Crypto()),
          opentxs::identity::wot::claim::DefaultVersion(),
          opentxs::identity::wot::claim::DefaultVersion(),
          {})
    , active_contact_item_(std::make_shared<ot::identity::wot::claim::Item>(
          claim_to_contact_item(client_1_.Factory().Claim(
              nym_id_1_,
              claim::SectionType::Identifier,
              claim::ClaimType::Employee,
              "activeContactItemValue",
              active_attr_))))
{
}

void ContactData::testAddItemMethod(
    const CallbackType1 contactDataMethod,
    claim::SectionType sectionName,
    opentxs::VersionNumber version,
    opentxs::VersionNumber targetVersion)
{
    // Add a contact to a group with no primary.
    using Group = claim::Group;
    using Section = claim::Section;
    const auto group1 = std::make_shared<Group>(
        "contactGroup1", sectionName, claim::ClaimType::Bch, Group::ItemMap{});
    const auto section1 = std::make_shared<Section>(
        client_1_,
        "contactSectionNym1",
        version,
        version,
        sectionName,
        Section::GroupMap{{claim::ClaimType::Bch, group1}});
    const auto data1 = claim::Data{
        client_1_,
        nym_id_2_.asBase58(client_1_.Crypto()),
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
    const auto identifier1 = ot::identity::credential::Contact::ClaimID(
        client_1_,
        {nym_id_2_},
        sectionName,
        claim::ClaimType::Bch,
        {},
        {},
        "instrumentDefinitionID1",
        "",
        ot::identity::wot::claim::DefaultVersion());
    const auto contactItem1 = data2.Claim(identifier1);

    ASSERT_NE(nullptr, contactItem1);
    ASSERT_TRUE(contactItem1->HasAttribute(
        opentxs::identity::wot::claim::Attribute::Primary));

    // Add a contact to a group with a primary.
    const auto data3 = contactDataMethod(
        data2, "instrumentDefinitionID2", ot::UnitType::Bch, false, false);

    // Verify that the item wasn't made primary.
    const auto identifier2 = ot::identity::credential::Contact::ClaimID(
        client_1_,
        {nym_id_2_},
        sectionName,
        claim::ClaimType::Bch,
        {},
        {},
        "instrumentDefinitionID2",
        "",
        ot::identity::wot::claim::DefaultVersion());
    const auto contactItem2 = data3.Claim(identifier2);
    ASSERT_NE(nullptr, contactItem2);
    ASSERT_FALSE(contactItem2->HasAttribute(
        opentxs::identity::wot::claim::Attribute::Primary));

    // Add a contact for a type with no group.
    const auto data4 = contactDataMethod(
        data3, "instrumentDefinitionID3", ot::UnitType::Eur, false, false);

    // Verify the group was created.
    ASSERT_NE(nullptr, data4.Group(sectionName, claim::ClaimType::Eur));

    // Verify that the item was made primary.
    const auto identifier3 = ot::identity::credential::Contact::ClaimID(
        client_1_,
        {nym_id_2_},
        sectionName,
        claim::ClaimType::Eur,
        {},
        {},
        "instrumentDefinitionID3",
        "",
        ot::identity::wot::claim::DefaultVersion());
    const auto contactItem3 = data4.Claim(identifier3);

    ASSERT_NE(nullptr, contactItem3);
    ASSERT_TRUE(contactItem3->HasAttribute(
        opentxs::identity::wot::claim::Attribute::Primary));

    // Add an active contact.
    const auto data5 = contactDataMethod(
        data4, "instrumentDefinitionID4", ot::UnitType::Usd, false, true);

    // Verify the group was created.
    ASSERT_NE(nullptr, data5.Group(sectionName, claim::ClaimType::Usd));
    // Verify that the item was made active.
    const auto identifier4 = ot::identity::credential::Contact::ClaimID(
        client_1_,
        {nym_id_2_},
        sectionName,
        claim::ClaimType::Usd,
        {},
        {},
        "instrumentDefinitionID4",
        "",
        ot::identity::wot::claim::DefaultVersion());
    const auto contactItem4 = data5.Claim(identifier4);
    ASSERT_NE(nullptr, contactItem4);
    ASSERT_TRUE(contactItem4->HasAttribute(
        opentxs::identity::wot::claim::Attribute::Active));

    // Add a primary contact.
    const auto data6 = contactDataMethod(
        data5, "instrumentDefinitionID5", ot::UnitType::Usd, true, false);

    // Verify that the item was made primary.
    const auto identifier5 = ot::identity::credential::Contact::ClaimID(
        client_1_,
        {nym_id_2_},
        sectionName,
        claim::ClaimType::Usd,
        {},
        {},
        "instrumentDefinitionID5",
        "",
        ot::identity::wot::claim::DefaultVersion());
    const auto contactItem5 = data6.Claim(identifier5);

    ASSERT_NE(nullptr, contactItem5);
    ASSERT_TRUE(contactItem5->HasAttribute(
        opentxs::identity::wot::claim::Attribute::Primary));
}

void ContactData::testAddItemMethod2(
    const CallbackType2 contactDataMethod,
    claim::SectionType sectionName,
    claim::ClaimType itemType,
    opentxs::VersionNumber version,
    opentxs::VersionNumber targetVersion)
{
    // Add a contact to a group with no primary.
    using Group = claim::Group;
    using Section = claim::Section;
    const auto group1 = std::make_shared<Group>(
        "contactGroup1", sectionName, itemType, Group::ItemMap{});
    const auto section1 = std::make_shared<Section>(
        client_1_,
        "contactSectionNym1",
        version,
        version,
        sectionName,
        Section::GroupMap{{itemType, group1}});

    const claim::Data data1(
        client_1_,
        nym_id_2_.asBase58(client_1_.Crypto()),
        version,
        version,
        claim::Data::SectionMap{{sectionName, section1}});

    const auto data2 = contactDataMethod(data1, "contactValue1", false, false);

    if (targetVersion) {
        ASSERT_EQ(targetVersion, data2.Version());
        return;
    }

    // Verify that the item was made primary.
    const auto identifier1 = ot::identity::credential::Contact::ClaimID(
        client_1_,
        {nym_id_2_},
        sectionName,
        itemType,
        {},
        {},
        "contactValue1",
        "",
        ot::identity::wot::claim::DefaultVersion());
    const auto contactItem1 = data2.Claim(identifier1);

    ASSERT_NE(nullptr, contactItem1);
    ASSERT_TRUE(contactItem1->HasAttribute(
        opentxs::identity::wot::claim::Attribute::Primary));

    // Add a contact to a group with a primary.
    const auto data3 = contactDataMethod(data2, "contactValue2", false, false);
    // Verify that the item wasn't made primary.
    const auto identifier2 = ot::identity::credential::Contact::ClaimID(
        client_1_,
        {nym_id_2_},
        sectionName,
        itemType,
        {},
        {},
        "contactValue2",
        "",
        ot::identity::wot::claim::DefaultVersion());
    const auto contactItem2 = data3.Claim(identifier2);

    ASSERT_NE(nullptr, contactItem2);
    ASSERT_FALSE(contactItem2->HasAttribute(
        opentxs::identity::wot::claim::Attribute::Primary));

    // Add a contact for a type with no group.
    const auto section2 = std::make_shared<claim::Section>(

        client_1_,
        "contactSectionNym2",
        version,
        version,
        sectionName,
        claim::Section::GroupMap{});

    const claim::Data data4(
        client_1_,
        nym_id_4_.asBase58(client_1_.Crypto()),
        version,
        version,
        claim::Data::SectionMap{{sectionName, section2}});

    const auto data5 = contactDataMethod(data4, "contactValue3", false, false);

    // Verify the group was created.
    ASSERT_NE(nullptr, data5.Group(sectionName, itemType));

    // Verify that the item was made primary.
    const auto identifier3 = ot::identity::credential::Contact::ClaimID(
        client_1_,
        {nym_id_4_},
        sectionName,
        itemType,
        {},
        {},
        "contactValue3",
        "",
        ot::identity::wot::claim::DefaultVersion());
    const auto contactItem3 = data5.Claim(identifier3);

    ASSERT_NE(nullptr, contactItem3);
    ASSERT_TRUE(contactItem3->HasAttribute(
        opentxs::identity::wot::claim::Attribute::Primary));

    // Add an active contact.
    const auto data6 = contactDataMethod(data5, "contactValue4", false, true);
    // Verify that the item was made active.
    const auto identifier4 = ot::identity::credential::Contact::ClaimID(
        client_1_,
        {nym_id_4_},
        sectionName,
        itemType,
        {},
        {},
        "contactValue4",
        "",
        ot::identity::wot::claim::DefaultVersion());
    const auto contactItem4 = data6.Claim(identifier4);

    ASSERT_NE(nullptr, contactItem4);
    ASSERT_TRUE(contactItem4->HasAttribute(
        opentxs::identity::wot::claim::Attribute::Active));
}
}  // namespace ottest
