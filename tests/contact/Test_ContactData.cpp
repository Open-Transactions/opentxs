// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <cstdint>
#include <memory>
#include <tuple>
#include <utility>

#include "1_Internal.hpp"
#include "internal/identity/wot/claim/Types.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/UnitType.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/identity/credential/Contact.hpp"
#include "opentxs/identity/wot/claim/Attribute.hpp"
#include "opentxs/identity/wot/claim/ClaimType.hpp"
#include "opentxs/identity/wot/claim/Data.hpp"
#include "opentxs/identity/wot/claim/Group.hpp"
#include "opentxs/identity/wot/claim/Item.hpp"
#include "opentxs/identity/wot/claim/Section.hpp"
#include "opentxs/identity/wot/claim/SectionType.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace ot = opentxs;

namespace ottest
{
class Test_ContactData : public ::testing::Test
{
public:
    Test_ContactData()
        : api_(ot::Context().StartClientSession(0))
        , contactData_(
              dynamic_cast<const ot::api::session::Client&>(api_),
              ot::UnallocatedCString("contactDataNym"),
              opentxs::CONTACT_CONTACT_DATA_VERSION,
              opentxs::CONTACT_CONTACT_DATA_VERSION,
              {})
        , activeContactItem_(new ot::identity::wot::claim::Item(
              dynamic_cast<const ot::api::session::Client&>(api_),
              ot::UnallocatedCString("activeContactItem"),
              opentxs::CONTACT_CONTACT_DATA_VERSION,
              opentxs::CONTACT_CONTACT_DATA_VERSION,
              ot::identity::wot::claim::SectionType::Identifier,
              ot::identity::wot::claim::ClaimType::Employee,
              ot::UnallocatedCString("activeContactItemValue"),
              {ot::identity::wot::claim::Attribute::Active},
              NULL_START,
              NULL_END,
              ""))
    {
    }

    const ot::api::session::Client& api_;
    const ot::identity::wot::claim::Data contactData_;
    const std::shared_ptr<ot::identity::wot::claim::Item> activeContactItem_;

    using CallbackType1 = ot::identity::wot::claim::Data (*)(
        const ot::identity::wot::claim::Data&,
        const ot::UnallocatedCString&,
        const ot::UnitType,
        const bool,
        const bool);
    using CallbackType2 = ot::identity::wot::claim::Data (*)(
        const ot::identity::wot::claim::Data&,
        const ot::UnallocatedCString&,
        const bool,
        const bool);

    void testAddItemMethod(
        const CallbackType1 contactDataMethod,
        ot::identity::wot::claim::SectionType sectionName,
        std::uint32_t version = opentxs::CONTACT_CONTACT_DATA_VERSION,
        std::uint32_t targetVersion = 0);

    void testAddItemMethod2(
        const CallbackType2 contactDataMethod,
        ot::identity::wot::claim::SectionType sectionName,
        ot::identity::wot::claim::ClaimType itemType,
        std::uint32_t version = opentxs::CONTACT_CONTACT_DATA_VERSION,
        std::uint32_t targetVersion = 0);
};

ot::identity::wot::claim::Data add_contract(
    const ot::identity::wot::claim::Data& data,
    const ot::UnallocatedCString& id,
    const ot::UnitType type,
    const bool active,
    const bool primary);
ot::identity::wot::claim::Data add_contract(
    const ot::identity::wot::claim::Data& data,
    const ot::UnallocatedCString& id,
    const ot::UnitType type,
    const bool active,
    const bool primary)
{
    return data.AddContract(id, type, active, primary);
}

ot::identity::wot::claim::Data add_email(
    const ot::identity::wot::claim::Data& data,
    const ot::UnallocatedCString& id,
    const bool active,
    const bool primary);
ot::identity::wot::claim::Data add_email(
    const ot::identity::wot::claim::Data& data,
    const ot::UnallocatedCString& id,
    const bool active,
    const bool primary)
{
    return data.AddEmail(id, active, primary);
}

ot::identity::wot::claim::Data add_payment_code(
    const ot::identity::wot::claim::Data& data,
    const ot::UnallocatedCString& id,
    const ot::UnitType type,
    const bool active,
    const bool primary);
ot::identity::wot::claim::Data add_payment_code(
    const ot::identity::wot::claim::Data& data,
    const ot::UnallocatedCString& id,
    const ot::UnitType type,
    const bool active,
    const bool primary)
{
    return data.AddPaymentCode(id, type, active, primary);
}

ot::identity::wot::claim::Data add_phone_number(
    const ot::identity::wot::claim::Data& data,
    const ot::UnallocatedCString& id,
    const bool active,
    const bool primary);
ot::identity::wot::claim::Data add_phone_number(
    const ot::identity::wot::claim::Data& data,
    const ot::UnallocatedCString& id,
    const bool active,
    const bool primary)
{
    return data.AddPhoneNumber(id, active, primary);
}

void Test_ContactData::testAddItemMethod(
    const CallbackType1 contactDataMethod,
    ot::identity::wot::claim::SectionType sectionName,
    std::uint32_t version,
    std::uint32_t targetVersion)
{
    // Add a contact to a group with no primary.
    const auto& group1 = std::shared_ptr<ot::identity::wot::claim::Group>(
        new ot::identity::wot::claim::Group(
            "contactGroup1",
            sectionName,
            ot::identity::wot::claim::ClaimType::Bch,
            {}));

    const auto& section1 = std::shared_ptr<ot::identity::wot::claim::Section>(
        new ot::identity::wot::claim::Section(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactSectionNym1",
            version,
            version,
            sectionName,
            ot::identity::wot::claim::Section::GroupMap{
                {ot::identity::wot::claim::ClaimType::Bch, group1}}));

    const ot::identity::wot::claim::Data data1(
        dynamic_cast<const ot::api::session::Client&>(api_),
        ot::UnallocatedCString("contactDataNym1"),
        version,
        version,
        ot::identity::wot::claim::Data::SectionMap{{sectionName, section1}});

    const auto& data2 = contactDataMethod(
        data1, "instrumentDefinitionID1", ot::UnitType::Bch, false, false);

    if (targetVersion) {
        ASSERT_EQ(targetVersion, data2.Version());
        return;
    }

    // Verify that the item was made primary.
    const ot::OTIdentifier identifier1(
        ot::Identifier::Factory(ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactDataNym1",
            sectionName,
            ot::identity::wot::claim::ClaimType::Bch,
            NULL_START,
            NULL_END,
            "instrumentDefinitionID1",
            "")));
    const auto& contactItem1 = data2.Claim(identifier1);
    ASSERT_NE(nullptr, contactItem1);
    ASSERT_TRUE(contactItem1->isPrimary());

    // Add a contact to a group with a primary.
    const auto& data3 = contactDataMethod(
        data2, "instrumentDefinitionID2", ot::UnitType::Bch, false, false);

    // Verify that the item wasn't made primary.
    const ot::OTIdentifier identifier2(
        ot::Identifier::Factory(ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactDataNym1",
            sectionName,
            ot::identity::wot::claim::ClaimType::Bch,
            NULL_START,
            NULL_END,
            "instrumentDefinitionID2",
            "")));
    const auto& contactItem2 = data3.Claim(identifier2);
    ASSERT_NE(nullptr, contactItem2);
    ASSERT_FALSE(contactItem2->isPrimary());

    // Add a contact for a type with no group.
    const auto& data4 = contactDataMethod(
        data3, "instrumentDefinitionID3", ot::UnitType::Eur, false, false);

    // Verify the group was created.
    ASSERT_NE(
        nullptr,
        data4.Group(sectionName, ot::identity::wot::claim::ClaimType::Eur));
    // Verify that the item was made primary.
    const ot::OTIdentifier identifier3(
        ot::Identifier::Factory(ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactDataNym1",
            sectionName,
            ot::identity::wot::claim::ClaimType::Eur,
            NULL_START,
            NULL_END,
            "instrumentDefinitionID3",
            "")));
    const auto& contactItem3 = data4.Claim(identifier3);
    ASSERT_NE(nullptr, contactItem3);
    ASSERT_TRUE(contactItem3->isPrimary());

    // Add an active contact.
    const auto& data5 = contactDataMethod(
        data4, "instrumentDefinitionID4", ot::UnitType::Usd, false, true);

    // Verify the group was created.
    ASSERT_NE(
        nullptr,
        data5.Group(sectionName, ot::identity::wot::claim::ClaimType::Usd));
    // Verify that the item was made active.
    const ot::OTIdentifier identifier4(
        ot::Identifier::Factory(ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactDataNym1",
            sectionName,
            ot::identity::wot::claim::ClaimType::Usd,
            NULL_START,
            NULL_END,
            "instrumentDefinitionID4",
            "")));
    const auto& contactItem4 = data5.Claim(identifier4);
    ASSERT_NE(nullptr, contactItem4);
    ASSERT_TRUE(contactItem4->isActive());

    // Add a primary contact.
    const auto& data6 = contactDataMethod(
        data5, "instrumentDefinitionID5", ot::UnitType::Usd, true, false);

    // Verify that the item was made primary.
    const ot::OTIdentifier identifier5(
        ot::Identifier::Factory(ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactDataNym1",
            sectionName,
            ot::identity::wot::claim::ClaimType::Usd,
            NULL_START,
            NULL_END,
            "instrumentDefinitionID5",
            "")));
    const auto& contactItem5 = data6.Claim(identifier5);
    ASSERT_NE(nullptr, contactItem5);
    ASSERT_TRUE(contactItem5->isPrimary());
}

void Test_ContactData::testAddItemMethod2(
    const CallbackType2 contactDataMethod,
    ot::identity::wot::claim::SectionType sectionName,
    ot::identity::wot::claim::ClaimType itemType,
    std::uint32_t version,
    std::uint32_t targetVersion)
{
    // Add a contact to a group with no primary.
    const auto& group1 = std::shared_ptr<ot::identity::wot::claim::Group>(
        new ot::identity::wot::claim::Group(
            "contactGroup1", sectionName, itemType, {}));

    const auto& section1 = std::shared_ptr<ot::identity::wot::claim::Section>(
        new ot::identity::wot::claim::Section(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactSectionNym1",
            version,
            version,
            sectionName,
            ot::identity::wot::claim::Section::GroupMap{{itemType, group1}}));

    const ot::identity::wot::claim::Data data1(
        dynamic_cast<const ot::api::session::Client&>(api_),
        ot::UnallocatedCString("contactDataNym1"),
        version,
        version,
        ot::identity::wot::claim::Data::SectionMap{{sectionName, section1}});

    const auto& data2 = contactDataMethod(data1, "contactValue1", false, false);

    if (targetVersion) {
        ASSERT_EQ(targetVersion, data2.Version());
        return;
    }

    // Verify that the item was made primary.
    const ot::OTIdentifier identifier1(
        ot::Identifier::Factory(ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactDataNym1",
            sectionName,
            itemType,
            NULL_START,
            NULL_END,
            "contactValue1",
            "")));
    const auto& contactItem1 = data2.Claim(identifier1);
    ASSERT_NE(nullptr, contactItem1);
    ASSERT_TRUE(contactItem1->isPrimary());

    // Add a contact to a group with a primary.
    const auto& data3 = contactDataMethod(data2, "contactValue2", false, false);

    // Verify that the item wasn't made primary.
    const ot::OTIdentifier identifier2(
        ot::Identifier::Factory(ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactDataNym1",
            sectionName,
            itemType,
            NULL_START,
            NULL_END,
            "contactValue2",
            "")));
    const auto& contactItem2 = data3.Claim(identifier2);
    ASSERT_NE(nullptr, contactItem2);
    ASSERT_FALSE(contactItem2->isPrimary());

    // Add a contact for a type with no group.
    const auto& section2 = std::shared_ptr<ot::identity::wot::claim::Section>(
        new ot::identity::wot::claim::Section(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactSectionNym2",
            version,
            version,
            sectionName,
            ot::identity::wot::claim::Section::GroupMap{}));

    const ot::identity::wot::claim::Data data4(
        dynamic_cast<const ot::api::session::Client&>(api_),
        ot::UnallocatedCString("contactDataNym4"),
        version,
        version,
        ot::identity::wot::claim::Data::SectionMap{{sectionName, section2}});

    const auto& data5 = contactDataMethod(data4, "contactValue3", false, false);

    // Verify the group was created.
    ASSERT_NE(nullptr, data5.Group(sectionName, itemType));
    // Verify that the item was made primary.
    const ot::OTIdentifier identifier3(
        ot::Identifier::Factory(ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactDataNym4",
            sectionName,
            itemType,
            NULL_START,
            NULL_END,
            "contactValue3",
            "")));
    const auto& contactItem3 = data5.Claim(identifier3);
    ASSERT_NE(nullptr, contactItem3);
    ASSERT_TRUE(contactItem3->isPrimary());

    // Add an active contact.
    const auto& data6 = contactDataMethod(data5, "contactValue4", false, true);

    // Verify that the item was made active.
    const ot::OTIdentifier identifier4(
        ot::Identifier::Factory(ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactDataNym4",
            sectionName,
            itemType,
            NULL_START,
            NULL_END,
            "contactValue4",
            "")));
    const auto& contactItem4 = data6.Claim(identifier4);
    ASSERT_NE(nullptr, contactItem4);
    ASSERT_TRUE(contactItem4->isActive());
}

static const ot::UnallocatedCString expectedStringOutput =
    ot::UnallocatedCString{"Version "} +
    std::to_string(opentxs::CONTACT_CONTACT_DATA_VERSION) +
    ot::UnallocatedCString(
        " contact data\nSections found: 1\n- Section: Identifier, version: ") +
    std::to_string(opentxs::CONTACT_CONTACT_DATA_VERSION) +
    ot::UnallocatedCString{
        " containing 1 item(s).\n-- Item type: \"employee of\", value: "
        "\"activeContactItemValue\", start: 0, end: 0, version: "} +
    std::to_string(opentxs::CONTACT_CONTACT_DATA_VERSION) +
    ot::UnallocatedCString{"\n--- Attributes: Active \n"};

TEST_F(Test_ContactData, first_constructor)
{
    const std::shared_ptr<ot::identity::wot::claim::Section> section1(
        new ot::identity::wot::claim::Section(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "testContactSectionNym1",
            opentxs::CONTACT_CONTACT_DATA_VERSION,
            opentxs::CONTACT_CONTACT_DATA_VERSION,
            ot::identity::wot::claim::SectionType::Identifier,
            activeContactItem_));

    const ot::identity::wot::claim::Data::SectionMap map{
        {section1->Type(), section1}};

    const ot::identity::wot::claim::Data contactData(
        dynamic_cast<const ot::api::session::Client&>(api_),
        ot::UnallocatedCString("contactDataNym"),
        opentxs::CONTACT_CONTACT_DATA_VERSION,
        opentxs::CONTACT_CONTACT_DATA_VERSION,
        map);
    ASSERT_EQ(opentxs::CONTACT_CONTACT_DATA_VERSION, contactData.Version());
    ASSERT_NE(
        nullptr,
        contactData.Section(ot::identity::wot::claim::SectionType::Identifier));
    ASSERT_NE(
        nullptr,
        contactData.Group(
            ot::identity::wot::claim::SectionType::Identifier,
            ot::identity::wot::claim::ClaimType::Employee));
    ASSERT_TRUE(contactData.HaveClaim(
        ot::identity::wot::claim::SectionType::Identifier,
        ot::identity::wot::claim::ClaimType::Employee,
        activeContactItem_->Value()));
}

TEST_F(Test_ContactData, first_constructor_no_sections)
{
    const ot::identity::wot::claim::Data contactData(
        dynamic_cast<const ot::api::session::Client&>(api_),
        ot::UnallocatedCString("contactDataNym"),
        opentxs::CONTACT_CONTACT_DATA_VERSION,
        opentxs::CONTACT_CONTACT_DATA_VERSION,
        {});
    ASSERT_EQ(opentxs::CONTACT_CONTACT_DATA_VERSION, contactData.Version());
}

TEST_F(Test_ContactData, first_constructor_different_versions)
{
    const ot::identity::wot::claim::Data contactData(
        dynamic_cast<const ot::api::session::Client&>(api_),
        ot::UnallocatedCString("contactDataNym"),
        opentxs::CONTACT_CONTACT_DATA_VERSION - 1,  // previous version
        opentxs::CONTACT_CONTACT_DATA_VERSION,
        {});
    ASSERT_EQ(opentxs::CONTACT_CONTACT_DATA_VERSION, contactData.Version());
}

TEST_F(Test_ContactData, copy_constructor)
{
    const std::shared_ptr<ot::identity::wot::claim::Section> section1(
        new ot::identity::wot::claim::Section(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "testContactSectionNym1",
            opentxs::CONTACT_CONTACT_DATA_VERSION,
            opentxs::CONTACT_CONTACT_DATA_VERSION,
            ot::identity::wot::claim::SectionType::Identifier,
            activeContactItem_));

    const ot::identity::wot::claim::Data::SectionMap map{
        {section1->Type(), section1}};

    const ot::identity::wot::claim::Data contactData(
        dynamic_cast<const ot::api::session::Client&>(api_),
        ot::UnallocatedCString("contactDataNym"),
        opentxs::CONTACT_CONTACT_DATA_VERSION,
        opentxs::CONTACT_CONTACT_DATA_VERSION,
        map);

    const ot::identity::wot::claim::Data copiedContactData(contactData);

    ASSERT_EQ(
        opentxs::CONTACT_CONTACT_DATA_VERSION, copiedContactData.Version());
    ASSERT_NE(
        nullptr,
        copiedContactData.Section(
            ot::identity::wot::claim::SectionType::Identifier));
    ASSERT_NE(
        nullptr,
        copiedContactData.Group(
            ot::identity::wot::claim::SectionType::Identifier,
            ot::identity::wot::claim::ClaimType::Employee));
    ASSERT_TRUE(copiedContactData.HaveClaim(
        ot::identity::wot::claim::SectionType::Identifier,
        ot::identity::wot::claim::ClaimType::Employee,
        activeContactItem_->Value()));
}

TEST_F(Test_ContactData, operator_plus)
{
    const auto& data1 = contactData_.AddItem(activeContactItem_);

    // Add a ContactData object with a section of the same type.
    const auto& contactItem2 = std::shared_ptr<ot::identity::wot::claim::Item>(
        new ot::identity::wot::claim::Item(
            dynamic_cast<const ot::api::session::Client&>(api_),
            ot::UnallocatedCString("contactItem2"),
            opentxs::CONTACT_CONTACT_DATA_VERSION,
            opentxs::CONTACT_CONTACT_DATA_VERSION,
            ot::identity::wot::claim::SectionType::Identifier,
            ot::identity::wot::claim::ClaimType::Employee,
            ot::UnallocatedCString("contactItemValue2"),
            {ot::identity::wot::claim::Attribute::Active},
            NULL_START,
            NULL_END,
            ""));

    const auto& group2 = std::shared_ptr<ot::identity::wot::claim::Group>(
        new ot::identity::wot::claim::Group(
            "contactGroup2",
            ot::identity::wot::claim::SectionType::Identifier,
            contactItem2));

    const auto& section2 = std::shared_ptr<ot::identity::wot::claim::Section>(
        new ot::identity::wot::claim::Section(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactSectionNym2",
            opentxs::CONTACT_CONTACT_DATA_VERSION,
            opentxs::CONTACT_CONTACT_DATA_VERSION,
            ot::identity::wot::claim::SectionType::Identifier,
            ot::identity::wot::claim::Section::GroupMap{
                {contactItem2->Type(), group2}}));

    const ot::identity::wot::claim::Data data2(
        dynamic_cast<const ot::api::session::Client&>(api_),
        ot::UnallocatedCString("contactDataNym2"),
        opentxs::CONTACT_CONTACT_DATA_VERSION,
        opentxs::CONTACT_CONTACT_DATA_VERSION,
        ot::identity::wot::claim::Data::SectionMap{
            {ot::identity::wot::claim::SectionType::Identifier, section2}});

    const auto& data3 = data1 + data2;
    // Verify the section exists.
    ASSERT_NE(
        nullptr,
        data3.Section(ot::identity::wot::claim::SectionType::Identifier));
    // Verify it has one group.
    ASSERT_EQ(
        1,
        data3.Section(ot::identity::wot::claim::SectionType::Identifier)
            ->Size());
    // Verify the group exists.
    ASSERT_NE(
        nullptr,
        data3.Group(
            ot::identity::wot::claim::SectionType::Identifier,
            ot::identity::wot::claim::ClaimType::Employee));
    // Verify it has two items.
    ASSERT_EQ(
        2,
        data3
            .Group(
                ot::identity::wot::claim::SectionType::Identifier,
                ot::identity::wot::claim::ClaimType::Employee)
            ->Size());

    // Add a ContactData object with a section of a different type.
    const auto& contactItem4 = std::shared_ptr<ot::identity::wot::claim::Item>(
        new ot::identity::wot::claim::Item(
            dynamic_cast<const ot::api::session::Client&>(api_),
            ot::UnallocatedCString("contactItem4"),
            opentxs::CONTACT_CONTACT_DATA_VERSION,
            opentxs::CONTACT_CONTACT_DATA_VERSION,
            ot::identity::wot::claim::SectionType::Address,
            ot::identity::wot::claim::ClaimType::Physical,
            ot::UnallocatedCString("contactItemValue4"),
            {ot::identity::wot::claim::Attribute::Active},
            NULL_START,
            NULL_END,
            ""));

    const auto& group4 = std::shared_ptr<ot::identity::wot::claim::Group>(
        new ot::identity::wot::claim::Group(
            "contactGroup4",
            ot::identity::wot::claim::SectionType::Address,
            contactItem4));

    const auto& section4 = std::shared_ptr<ot::identity::wot::claim::Section>(
        new ot::identity::wot::claim::Section(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactSectionNym4",
            opentxs::CONTACT_CONTACT_DATA_VERSION,
            opentxs::CONTACT_CONTACT_DATA_VERSION,
            ot::identity::wot::claim::SectionType::Address,
            ot::identity::wot::claim::Section::GroupMap{
                {contactItem4->Type(), group4}}));

    const ot::identity::wot::claim::Data data4(
        dynamic_cast<const ot::api::session::Client&>(api_),
        ot::UnallocatedCString("contactDataNym4"),
        opentxs::CONTACT_CONTACT_DATA_VERSION,
        opentxs::CONTACT_CONTACT_DATA_VERSION,
        ot::identity::wot::claim::Data::SectionMap{
            {ot::identity::wot::claim::SectionType::Address, section4}});

    const auto& data5 = data3 + data4;
    // Verify the first section exists.
    ASSERT_NE(
        nullptr,
        data5.Section(ot::identity::wot::claim::SectionType::Identifier));
    // Verify it has one group.
    ASSERT_EQ(
        1,
        data5.Section(ot::identity::wot::claim::SectionType::Identifier)
            ->Size());
    // Verify the group exists.
    ASSERT_NE(
        nullptr,
        data5.Group(
            ot::identity::wot::claim::SectionType::Identifier,
            ot::identity::wot::claim::ClaimType::Employee));
    // Verify it has two items.
    ASSERT_EQ(
        2,
        data5
            .Group(
                ot::identity::wot::claim::SectionType::Identifier,
                ot::identity::wot::claim::ClaimType::Employee)
            ->Size());

    // Verify the second section exists.
    ASSERT_NE(
        nullptr, data5.Section(ot::identity::wot::claim::SectionType::Address));
    // Verify it has one group.
    ASSERT_EQ(
        1,
        data5.Section(ot::identity::wot::claim::SectionType::Address)->Size());
    // Verify the group exists.
    ASSERT_NE(
        nullptr,
        data5.Group(
            ot::identity::wot::claim::SectionType::Address,
            ot::identity::wot::claim::ClaimType::Physical));
    // Verify it has one item.
    ASSERT_EQ(
        1,
        data5
            .Group(
                ot::identity::wot::claim::SectionType::Address,
                ot::identity::wot::claim::ClaimType::Physical)
            ->Size());
}

TEST_F(Test_ContactData, operator_plus_different_version)
{
    // rhs version less than lhs
    const ot::identity::wot::claim::Data contactData2(
        dynamic_cast<const ot::api::session::Client&>(api_),
        ot::UnallocatedCString("contactDataNym"),
        opentxs::CONTACT_CONTACT_DATA_VERSION - 1,
        opentxs::CONTACT_CONTACT_DATA_VERSION - 1,
        {});

    const auto& contactData3 = contactData_ + contactData2;
    // Verify the new contact data has the latest version.
    ASSERT_EQ(opentxs::CONTACT_CONTACT_DATA_VERSION, contactData3.Version());

    // lhs version less than rhs
    const auto& contactData4 = contactData2 + contactData_;
    // Verify the new contact data has the latest version.
    ASSERT_EQ(opentxs::CONTACT_CONTACT_DATA_VERSION, contactData4.Version());
}

TEST_F(Test_ContactData, operator_string)
{
    const auto& data1 = contactData_.AddItem(activeContactItem_);
    const ot::UnallocatedCString dataString = data1;
    ASSERT_EQ(expectedStringOutput, dataString);
}

TEST_F(Test_ContactData, Serialize)
{
    const auto& data1 = contactData_.AddItem(activeContactItem_);

    // Serialize without ids.
    auto bytes = ot::Space{};
    EXPECT_TRUE(data1.Serialize(ot::writer(bytes), false));

    auto restored1 = ot::identity::wot::claim::Data{
        dynamic_cast<const ot::api::session::Client&>(api_),
        "ContactDataNym1",
        data1.Version(),
        ot::reader(bytes)};

    ASSERT_EQ(restored1.Version(), data1.Version());
    auto section_iterator = restored1.begin();
    auto section_name = section_iterator->first;
    ASSERT_EQ(section_name, ot::identity::wot::claim::SectionType::Identifier);
    auto section1 = section_iterator->second;
    ASSERT_TRUE(section1);
    auto group_iterator = section1->begin();
    ASSERT_EQ(
        group_iterator->first, ot::identity::wot::claim::ClaimType::Employee);
    auto group1 = group_iterator->second;
    ASSERT_TRUE(group1);
    auto item_iterator = group1->begin();
    auto contact_item = item_iterator->second;
    ASSERT_TRUE(contact_item);
    ASSERT_EQ(activeContactItem_->Value(), contact_item->Value());
    ASSERT_EQ(activeContactItem_->Version(), contact_item->Version());
    ASSERT_EQ(activeContactItem_->Type(), contact_item->Type());
    ASSERT_EQ(activeContactItem_->Start(), contact_item->Start());
    ASSERT_EQ(activeContactItem_->End(), contact_item->End());

    // Serialize with ids.
    EXPECT_TRUE(data1.Serialize(ot::writer(bytes), true));

    auto restored2 = ot::identity::wot::claim::Data{
        dynamic_cast<const ot::api::session::Client&>(api_),
        "ContactDataNym1",
        data1.Version(),
        ot::reader(bytes)};

    ASSERT_EQ(restored2.Version(), data1.Version());
    section_iterator = restored2.begin();
    section_name = section_iterator->first;
    ASSERT_EQ(section_name, ot::identity::wot::claim::SectionType::Identifier);
    section1 = section_iterator->second;
    ASSERT_TRUE(section1);
    group_iterator = section1->begin();
    ASSERT_EQ(
        group_iterator->first, ot::identity::wot::claim::ClaimType::Employee);
    group1 = group_iterator->second;
    ASSERT_TRUE(group1);
    item_iterator = group1->begin();
    contact_item = item_iterator->second;
    ASSERT_TRUE(contact_item);
    ASSERT_EQ(activeContactItem_->Value(), contact_item->Value());
    ASSERT_EQ(activeContactItem_->Version(), contact_item->Version());
    ASSERT_EQ(activeContactItem_->Type(), contact_item->Type());
    ASSERT_EQ(activeContactItem_->Start(), contact_item->Start());
    ASSERT_EQ(activeContactItem_->End(), contact_item->End());
}

TEST_F(Test_ContactData, AddContract)
{
    testAddItemMethod(
        add_contract, ot::identity::wot::claim::SectionType::Contract);
}

TEST_F(Test_ContactData, AddContract_different_versions)
{
    testAddItemMethod(
        add_contract,
        ot::identity::wot::claim::SectionType::Contract,
        3,  // version of CONTACTSECTION_CONTRACT section before CITEMTYPE_BCH
            // was added
        4);
}

TEST_F(Test_ContactData, AddEmail)
{
    testAddItemMethod2(
        add_email,
        ot::identity::wot::claim::SectionType::Communication,
        ot::identity::wot::claim::ClaimType::Email);
}

// Nothing to test for required version of contact section for email
// because all current contact item types have been available for all
// versions of CONTACTSECTION_COMMUNICATION section.

// TEST_F(Test_ContactData, AddEmail_different_versions)
//{
//    testAddItemMethod2(
//        std::mem_fn<ot::identity::wot::claim::ContactData(
//            const ot::UnallocatedCString&, const bool, const bool) const>(
//            &ot::identity::wot::claim::ContactData::AddEmail),
//        ot::identity::wot::claim::ContactSectionName::Communication,
//        ot::identity::wot::claim::ClaimType::Email,
//        5,   // Change this to the old version of the section when a new
//             // version is added with new item types.
//        5);  // Change this to the version of the section with the new
//             // item type.
//}

TEST_F(Test_ContactData, AddItem_claim)
{
    ot::Claim claim = std::make_tuple(
        ot::UnallocatedCString(""),
        ot::translate(ot::identity::wot::claim::SectionType::Contract),
        ot::translate(ot::identity::wot::claim::ClaimType::Usd),
        ot::UnallocatedCString("contactItemValue"),
        NULL_START,
        NULL_END,
        ot::UnallocatedSet<std::uint32_t>{static_cast<uint32_t>(
            ot::identity::wot::claim::Attribute::Active)});
    const auto& data1 = contactData_.AddItem(claim);
    // Verify the section was added.
    ASSERT_NE(
        nullptr,
        data1.Section(ot::identity::wot::claim::SectionType::Contract));
    // Verify the group was added.
    ASSERT_NE(
        nullptr,
        data1.Group(
            ot::identity::wot::claim::SectionType::Contract,
            ot::identity::wot::claim::ClaimType::Usd));
    ASSERT_TRUE(data1.HaveClaim(
        ot::identity::wot::claim::SectionType::Contract,
        ot::identity::wot::claim::ClaimType::Usd,
        "contactItemValue"));
}

TEST_F(Test_ContactData, AddItem_claim_different_versions)
{
    const auto& group1 = std::shared_ptr<ot::identity::wot::claim::Group>(
        new ot::identity::wot::claim::Group(
            "contactGroup1",
            ot::identity::wot::claim::SectionType::Contract,
            ot::identity::wot::claim::ClaimType::Bch,
            {}));

    const auto& section1 = std::shared_ptr<ot::identity::wot::claim::Section>(
        new ot::identity::wot::claim::Section(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactSectionNym1",
            3,  // version of CONTACTSECTION_CONTRACT section before
                // CITEMTYPE_BCH was added
            3,
            ot::identity::wot::claim::SectionType::Contract,
            ot::identity::wot::claim::Section::GroupMap{
                {ot::identity::wot::claim::ClaimType::Bch, group1}}));

    const ot::identity::wot::claim::Data data1(
        dynamic_cast<const ot::api::session::Client&>(api_),
        ot::UnallocatedCString("contactDataNym1"),
        3,  // version of CONTACTSECTION_CONTRACT section before CITEMTYPE_BCH
            // was added
        3,
        ot::identity::wot::claim::Data::SectionMap{
            {ot::identity::wot::claim::SectionType::Contract, section1}});

    ot::Claim claim = std::make_tuple(
        ot::UnallocatedCString(""),
        ot::translate(ot::identity::wot::claim::SectionType::Contract),
        ot::translate(ot::identity::wot::claim::ClaimType::Bch),
        ot::UnallocatedCString("contactItemValue"),
        NULL_START,
        NULL_END,
        ot::UnallocatedSet<std::uint32_t>{static_cast<uint32_t>(
            ot::identity::wot::claim::Attribute::Active)});

    const auto& data2 = data1.AddItem(claim);

    ASSERT_EQ(4, data2.Version());
}

TEST_F(Test_ContactData, AddItem_item)
{
    // Add an item to a ContactData with no section.
    const auto& data1 = contactData_.AddItem(activeContactItem_);
    // Verify the section was added.
    ASSERT_NE(
        nullptr,
        data1.Section(ot::identity::wot::claim::SectionType::Identifier));
    // Verify the group was added.
    ASSERT_NE(
        nullptr,
        data1.Group(
            ot::identity::wot::claim::SectionType::Identifier,
            ot::identity::wot::claim::ClaimType::Employee));
    // Verify the item was added.
    ASSERT_TRUE(data1.HaveClaim(
        activeContactItem_->Section(),
        activeContactItem_->Type(),
        activeContactItem_->Value()));

    // Add an item to a ContactData with a section.
    const auto& contactItem2 = std::shared_ptr<ot::identity::wot::claim::Item>(
        new ot::identity::wot::claim::Item(
            dynamic_cast<const ot::api::session::Client&>(api_),
            ot::UnallocatedCString("contactItem2"),
            opentxs::CONTACT_CONTACT_DATA_VERSION,
            opentxs::CONTACT_CONTACT_DATA_VERSION,
            ot::identity::wot::claim::SectionType::Identifier,
            ot::identity::wot::claim::ClaimType::Employee,
            ot::UnallocatedCString("contactItemValue2"),
            {ot::identity::wot::claim::Attribute::Active},
            NULL_START,
            NULL_END,
            ""));
    const auto& data2 = data1.AddItem(contactItem2);
    // Verify the item was added.
    ASSERT_TRUE(data2.HaveClaim(
        contactItem2->Section(), contactItem2->Type(), contactItem2->Value()));
    // Verify the group has two items.
    ASSERT_EQ(
        2,
        data2
            .Group(
                ot::identity::wot::claim::SectionType::Identifier,
                ot::identity::wot::claim::ClaimType::Employee)
            ->Size());
}

TEST_F(Test_ContactData, AddItem_item_different_versions)
{
    const auto& group1 = std::shared_ptr<ot::identity::wot::claim::Group>(
        new ot::identity::wot::claim::Group(
            "contactGroup1",
            ot::identity::wot::claim::SectionType::Contract,
            ot::identity::wot::claim::ClaimType::Bch,
            {}));

    const auto& section1 = std::shared_ptr<ot::identity::wot::claim::Section>(
        new ot::identity::wot::claim::Section(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactSectionNym1",
            3,  // version of CONTACTSECTION_CONTRACT section before
                // CITEMTYPE_BCH was added
            3,
            ot::identity::wot::claim::SectionType::Contract,
            ot::identity::wot::claim::Section::GroupMap{
                {ot::identity::wot::claim::ClaimType::Bch, group1}}));

    const ot::identity::wot::claim::Data data1(
        dynamic_cast<const ot::api::session::Client&>(api_),
        ot::UnallocatedCString("contactDataNym1"),
        3,  // version of CONTACTSECTION_CONTRACT section before CITEMTYPE_BCH
            // was added
        3,
        ot::identity::wot::claim::Data::SectionMap{
            {ot::identity::wot::claim::SectionType::Contract, section1}});

    const auto& contactItem1 = std::shared_ptr<ot::identity::wot::claim::Item>(
        new ot::identity::wot::claim::Item(
            dynamic_cast<const ot::api::session::Client&>(api_),
            ot::UnallocatedCString("contactItem1"),
            opentxs::CONTACT_CONTACT_DATA_VERSION,
            opentxs::CONTACT_CONTACT_DATA_VERSION,
            ot::identity::wot::claim::SectionType::Contract,
            ot::identity::wot::claim::ClaimType::Bch,
            ot::UnallocatedCString("contactItemValue1"),
            {ot::identity::wot::claim::Attribute::Active},
            NULL_START,
            NULL_END,
            ""));

    const auto& data2 = data1.AddItem(contactItem1);

    ASSERT_EQ(4, data2.Version());
}

TEST_F(Test_ContactData, AddPaymentCode)
{
    testAddItemMethod(
        add_payment_code, ot::identity::wot::claim::SectionType::Procedure);
}

TEST_F(Test_ContactData, AddPaymentCode_different_versions)
{
    testAddItemMethod(
        add_contract,
        ot::identity::wot::claim::SectionType::Procedure,
        3,  // version of CONTACTSECTION_PROCEDURE section before CITEMTYPE_BCH
            // was added
        4);
}

TEST_F(Test_ContactData, AddPhoneNumber)
{
    testAddItemMethod2(
        add_phone_number,
        ot::identity::wot::claim::SectionType::Communication,
        ot::identity::wot::claim::ClaimType::Phone);
}

// Nothing to test for required version of contact section for phone number
// because all current contact item types have been available for all
// versions of CONTACTSECTION_COMMUNICATION section.

// TEST_F(Test_ContactData, AddPhoneNumber_different_versions)
//{
//    testAddItemMethod2(
//        std::mem_fn<ot::identity::wot::claim::ContactData(
//            const ot::UnallocatedCString&, const bool, const bool) const>(
//            &ot::identity::wot::claim::ContactData::AddPhoneNumber),
//        ot::identity::wot::claim::ContactSectionName::Communication,
//        ot::identity::wot::claim::ClaimType::Phone,
//        5,   // Change this to the old version of the section when a new
//             // version is added with new item types.
//        5);  // Change this to the version of the section with the new
//             // item type.
//}

TEST_F(Test_ContactData, AddPreferredOTServer)
{
    // Add a server to a group with no primary.
    const auto& group1 = std::shared_ptr<ot::identity::wot::claim::Group>(
        new ot::identity::wot::claim::Group(
            "contactGroup1",
            ot::identity::wot::claim::SectionType::Communication,
            ot::identity::wot::claim::ClaimType::Opentxs,
            {}));

    const auto& section1 = std::shared_ptr<ot::identity::wot::claim::Section>(
        new ot::identity::wot::claim::Section(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactSectionNym1",
            opentxs::CONTACT_CONTACT_DATA_VERSION,
            opentxs::CONTACT_CONTACT_DATA_VERSION,
            ot::identity::wot::claim::SectionType::Communication,
            ot::identity::wot::claim::Section::GroupMap{
                {ot::identity::wot::claim::ClaimType::Opentxs, group1}}));

    const ot::identity::wot::claim::Data data1(
        dynamic_cast<const ot::api::session::Client&>(api_),
        ot::UnallocatedCString("contactDataNym1"),
        opentxs::CONTACT_CONTACT_DATA_VERSION,
        opentxs::CONTACT_CONTACT_DATA_VERSION,
        ot::identity::wot::claim::Data::SectionMap{
            {ot::identity::wot::claim::SectionType::Communication, section1}});

    const ot::OTIdentifier serverIdentifier1(
        ot::Identifier::Factory(ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactDataNym1",
            ot::identity::wot::claim::SectionType::Communication,
            ot::identity::wot::claim::ClaimType::Opentxs,
            NULL_START,
            NULL_END,
            ot::UnallocatedCString("serverID1"),
            "")));
    const auto& data2 = data1.AddPreferredOTServer(serverIdentifier1, false);

    // Verify that the item was made primary.
    const ot::OTIdentifier identifier1(
        ot::Identifier::Factory(ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactDataNym1",
            ot::identity::wot::claim::SectionType::Communication,
            ot::identity::wot::claim::ClaimType::Opentxs,
            NULL_START,
            NULL_END,
            ot::String::Factory(serverIdentifier1)->Get(),
            "")));
    const auto& contactItem1 = data2.Claim(identifier1);
    ASSERT_NE(nullptr, contactItem1);
    ASSERT_TRUE(contactItem1->isPrimary());

    // Add a server to a group with a primary.
    const ot::OTIdentifier serverIdentifier2(
        ot::Identifier::Factory(ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactDataNym1",
            ot::identity::wot::claim::SectionType::Communication,
            ot::identity::wot::claim::ClaimType::Opentxs,
            NULL_START,
            NULL_END,
            ot::UnallocatedCString("serverID2"),
            "")));
    const auto& data3 = data2.AddPreferredOTServer(serverIdentifier2, false);

    // Verify that the item wasn't made primary.
    const ot::OTIdentifier identifier2(
        ot::Identifier::Factory(ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactDataNym1",
            ot::identity::wot::claim::SectionType::Communication,
            ot::identity::wot::claim::ClaimType::Opentxs,
            NULL_START,
            NULL_END,
            ot::String::Factory(serverIdentifier2)->Get(),
            "")));
    const auto& contactItem2 = data3.Claim(identifier2);
    ASSERT_NE(nullptr, contactItem2);
    ASSERT_FALSE(contactItem2->isPrimary());

    // Add a server to a ContactData with no group.
    const ot::OTIdentifier serverIdentifier3(
        ot::Identifier::Factory(ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactDataNym",
            ot::identity::wot::claim::SectionType::Communication,
            ot::identity::wot::claim::ClaimType::Opentxs,
            NULL_START,
            NULL_END,
            ot::UnallocatedCString("serverID3"),
            "")));
    const auto& data4 =
        contactData_.AddPreferredOTServer(serverIdentifier3, false);

    // Verify the group was created.
    ASSERT_NE(
        nullptr,
        data4.Group(
            ot::identity::wot::claim::SectionType::Communication,
            ot::identity::wot::claim::ClaimType::Opentxs));
    // Verify that the item was made primary.
    const ot::OTIdentifier identifier3(
        ot::Identifier::Factory(ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactDataNym",
            ot::identity::wot::claim::SectionType::Communication,
            ot::identity::wot::claim::ClaimType::Opentxs,
            NULL_START,
            NULL_END,
            ot::String::Factory(serverIdentifier3)->Get(),
            "")));
    const auto& contactItem3 = data4.Claim(identifier3);
    ASSERT_NE(nullptr, contactItem3);
    ASSERT_TRUE(contactItem3->isPrimary());

    // Add a primary server.
    const ot::OTIdentifier serverIdentifier4(
        ot::Identifier::Factory(ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactDataNym",
            ot::identity::wot::claim::SectionType::Communication,
            ot::identity::wot::claim::ClaimType::Opentxs,
            NULL_START,
            NULL_END,
            ot::UnallocatedCString("serverID4"),
            "")));
    const auto& data5 = data4.AddPreferredOTServer(serverIdentifier4, true);

    // Verify that the item was made primary.
    const ot::OTIdentifier identifier4(
        ot::Identifier::Factory(ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactDataNym",
            ot::identity::wot::claim::SectionType::Communication,
            ot::identity::wot::claim::ClaimType::Opentxs,
            NULL_START,
            NULL_END,
            ot::String::Factory(serverIdentifier4)->Get(),
            "")));
    const auto& contactItem4 = data5.Claim(identifier4);
    ASSERT_NE(nullptr, contactItem4);
    ASSERT_TRUE(contactItem4->isPrimary());
    // Verify the previous preferred server is no longer primary.
    const auto& contactItem5 = data5.Claim(identifier3);
    ASSERT_NE(nullptr, contactItem5);
    ASSERT_FALSE(contactItem5->isPrimary());
}

// Nothing to test for required version of contact section for OTServer
// because CMITEMTYPE_OPENTXS has been available for all versions of
// CONTACTSECTION_COMMUNICATION section.

// TEST_F(Test_ContactData, AddPreferredOTServer_different_versions)
//{
//}

TEST_F(Test_ContactData, AddSocialMediaProfile)
{
    // Add a profile that only resides in the profile section.

    // Add a profile to a contact with no primary profile.
    const auto& data2 = contactData_.AddSocialMediaProfile(
        "profileValue1",
        ot::identity::wot::claim::ClaimType::Aboutme,
        false,
        false);
    // Verify that the item was made primary.
    const ot::OTIdentifier identifier1(
        ot::Identifier::Factory(ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactDataNym",
            ot::identity::wot::claim::SectionType::Profile,
            ot::identity::wot::claim::ClaimType::Aboutme,
            NULL_START,
            NULL_END,
            "profileValue1",
            "")));
    const auto& contactItem1 = data2.Claim(identifier1);
    ASSERT_NE(nullptr, contactItem1);
    ASSERT_TRUE(contactItem1->isPrimary());

    // Add a primary profile.
    const auto& data3 = data2.AddSocialMediaProfile(
        "profileValue2",
        ot::identity::wot::claim::ClaimType::Aboutme,
        true,
        false);
    // Verify that the item was made primary.
    const ot::OTIdentifier identifier2(
        ot::Identifier::Factory(ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactDataNym",
            ot::identity::wot::claim::SectionType::Profile,
            ot::identity::wot::claim::ClaimType::Aboutme,
            NULL_START,
            NULL_END,
            "profileValue2",
            "")));
    const auto& contactItem2 = data3.Claim(identifier2);
    ASSERT_NE(nullptr, contactItem2);
    ASSERT_TRUE(contactItem2->isPrimary());

    // Add an active profile.
    const auto& data4 = data3.AddSocialMediaProfile(
        "profileValue3",
        ot::identity::wot::claim::ClaimType::Aboutme,
        false,
        true);
    // Verify that the item was made active.
    const ot::OTIdentifier identifier3(
        ot::Identifier::Factory(ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactDataNym",
            ot::identity::wot::claim::SectionType::Profile,
            ot::identity::wot::claim::ClaimType::Aboutme,
            NULL_START,
            NULL_END,
            "profileValue3",
            "")));
    const auto& contactItem3 = data4.Claim(identifier3);
    ASSERT_NE(nullptr, contactItem3);
    ASSERT_TRUE(contactItem3->isActive());

    // Add a profile that resides in the profile and communication sections.

    const auto& data5 = contactData_.AddSocialMediaProfile(
        "profileValue4",
        ot::identity::wot::claim::ClaimType::Linkedin,
        false,
        false);
    // Verify that it was added to the profile section.
    const ot::OTIdentifier identifier4(
        ot::Identifier::Factory(ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactDataNym",
            ot::identity::wot::claim::SectionType::Profile,
            ot::identity::wot::claim::ClaimType::Linkedin,
            NULL_START,
            NULL_END,
            "profileValue4",
            "")));
    const auto& contactItem4 = data5.Claim(identifier4);
    ASSERT_NE(nullptr, contactItem4);
    // Verify that it was added to the communication section.
    const ot::OTIdentifier identifier5(
        ot::Identifier::Factory(ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactDataNym",
            ot::identity::wot::claim::SectionType::Communication,
            ot::identity::wot::claim::ClaimType::Linkedin,
            NULL_START,
            NULL_END,
            "profileValue4",
            "")));
    const auto& contactItem5 = data5.Claim(identifier5);
    ASSERT_NE(nullptr, contactItem5);

    // Add a profile that resides in the profile and identifier sections.

    const auto& data6 = data5.AddSocialMediaProfile(
        "profileValue5",
        ot::identity::wot::claim::ClaimType::Yahoo,
        false,
        false);
    // Verify that it was added to the profile section.
    const ot::OTIdentifier identifier6(
        ot::Identifier::Factory(ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactDataNym",
            ot::identity::wot::claim::SectionType::Profile,
            ot::identity::wot::claim::ClaimType::Yahoo,
            NULL_START,
            NULL_END,
            "profileValue5",
            "")));
    const auto& contactItem6 = data6.Claim(identifier6);
    ASSERT_NE(nullptr, contactItem6);
    // Verify that it was added to the identifier section.
    const ot::OTIdentifier identifier7(
        ot::Identifier::Factory(ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactDataNym",
            ot::identity::wot::claim::SectionType::Identifier,
            ot::identity::wot::claim::ClaimType::Yahoo,
            NULL_START,
            NULL_END,
            "profileValue5",
            "")));
    const auto& contactItem7 = data6.Claim(identifier7);
    ASSERT_NE(nullptr, contactItem7);

    // Add a profile that resides in all three sections.

    const auto& data7 = data6.AddSocialMediaProfile(
        "profileValue6",
        ot::identity::wot::claim::ClaimType::Twitter,
        false,
        false);
    // Verify that it was added to the profile section.
    const ot::OTIdentifier identifier8(
        ot::Identifier::Factory(ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactDataNym",
            ot::identity::wot::claim::SectionType::Profile,
            ot::identity::wot::claim::ClaimType::Twitter,
            NULL_START,
            NULL_END,
            "profileValue6",
            "")));
    const auto& contactItem8 = data7.Claim(identifier8);
    ASSERT_NE(nullptr, contactItem8);
    // Verify that it was added to the communication section.
    const ot::OTIdentifier identifier9(
        ot::Identifier::Factory(ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactDataNym",
            ot::identity::wot::claim::SectionType::Communication,
            ot::identity::wot::claim::ClaimType::Twitter,
            NULL_START,
            NULL_END,
            "profileValue6",
            "")));
    const auto& contactItem9 = data7.Claim(identifier9);
    ASSERT_NE(nullptr, contactItem9);
    // Verify that it was added to the identifier section.
    const ot::OTIdentifier identifier10(
        ot::Identifier::Factory(ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactDataNym",
            ot::identity::wot::claim::SectionType::Identifier,
            ot::identity::wot::claim::ClaimType::Twitter,
            NULL_START,
            NULL_END,
            "profileValue6",
            "")));
    const auto& contactItem10 = data7.Claim(identifier10);
    ASSERT_NE(nullptr, contactItem10);
}

// Nothing to test for required version of contact sections for social media
// profiles because all current contact item types have been available for
// all versions of CONTACTSECTION_COMMUNICATION, CONTACTSECTION_IDENTIFIER,
// and CONTACTSECTION_PROFILE sections.

// TEST_F(Test_ContactData, AddSocialMediaProfile_different_versions)
//{
//    // Add a profile to the CONTACTSECTION_PROFILE section.
//    testAddItemMethod3(
//        std::mem_fn<ot::identity::wot::claim::ContactData(
//            const ot::UnallocatedCString&,
//            const ot::proto::ContactSectionName,
//            const ot::identity::wot::claim::,
//            const bool,
//            const bool)
//            const>(&ot::identity::wot::claim::ContactData::AddSocialMediaProfile),
//        ot::identity::wot::claim::ContactSectionName::Profile,
//        ot::identity::wot::claim::ClaimType::Twitter,
//        5,   // Change this to the old version of the section when a new
//             // version is added with new item types.
//        5);  // Change this to the version of the section with the new
//             // item type.
//}

TEST_F(Test_ContactData, BestEmail)
{
    // Add a non-active, non-primary email.
    const auto& data1 = contactData_.AddEmail("emailValue", false, false);
    // Verify it is the best email.
    ASSERT_STREQ("emailValue", data1.BestEmail().c_str());

    // Add an active, non-primary email.
    const auto& data2 = contactData_.AddEmail("activeEmailValue", false, true);
    // Verify it is the best email.
    ASSERT_STREQ("activeEmailValue", data2.BestEmail().c_str());

    // Add an active email to a contact data with a primary email (data1).
    const auto& data3 = data1.AddEmail("activeEmailValue", false, true);
    // Verify the primary email is the best.
    ASSERT_STREQ("emailValue", data3.BestEmail().c_str());

    // Add a new primary email.
    const auto& data4 = data3.AddEmail("primaryEmailValue", true, false);
    // Verify it is the best email.
    ASSERT_STREQ("primaryEmailValue", data4.BestEmail().c_str());
}

TEST_F(Test_ContactData, BestPhoneNumber)
{
    // Add a non-active, non-primary phone number.
    const auto& data1 =
        contactData_.AddPhoneNumber("phoneNumberValue", false, false);
    // Verify it is the best phone number.
    ASSERT_STREQ("phoneNumberValue", data1.BestPhoneNumber().c_str());

    // Add an active, non-primary phone number.
    const auto& data2 =
        contactData_.AddPhoneNumber("activePhoneNumberValue", false, true);
    // Verify it is the best phone number.
    ASSERT_STREQ("activePhoneNumberValue", data2.BestPhoneNumber().c_str());

    // Add an active phone number to a contact data with a primary phone number
    // (data1).
    const auto& data3 =
        data1.AddPhoneNumber("activePhoneNumberValue", false, true);
    // Verify the primary phone number is the best.
    ASSERT_STREQ("phoneNumberValue", data3.BestPhoneNumber().c_str());

    // Add a new primary phone number.
    const auto& data4 =
        data3.AddPhoneNumber("primaryPhoneNumberValue", true, false);
    // Verify it is the best phone number.
    ASSERT_STREQ("primaryPhoneNumberValue", data4.BestPhoneNumber().c_str());
}

TEST_F(Test_ContactData, BestSocialMediaProfile)
{
    // Add a non-active, non-primary profile.
    const auto& data1 = contactData_.AddSocialMediaProfile(
        "profileValue",
        ot::identity::wot::claim::ClaimType::Facebook,
        false,
        false);
    // Verify it is the best profile.
    ASSERT_STREQ(
        "profileValue",
        data1
            .BestSocialMediaProfile(
                ot::identity::wot::claim::ClaimType::Facebook)
            .c_str());

    // Add an active, non-primary profile.
    const auto& data2 = contactData_.AddSocialMediaProfile(
        "activeProfileValue",
        ot::identity::wot::claim::ClaimType::Facebook,
        false,
        true);
    // Verify it is the best profile.
    ASSERT_STREQ(
        "activeProfileValue",
        data2
            .BestSocialMediaProfile(
                ot::identity::wot::claim::ClaimType::Facebook)
            .c_str());

    // Add an active profile to a contact data with a primary profile (data1).
    const auto& data3 = data1.AddSocialMediaProfile(
        "activeProfileValue",
        ot::identity::wot::claim::ClaimType::Facebook,
        false,
        true);
    // Verify the primary profile is the best.
    ASSERT_STREQ(
        "profileValue",
        data3
            .BestSocialMediaProfile(
                ot::identity::wot::claim::ClaimType::Facebook)
            .c_str());

    // Add a new primary profile.
    const auto& data4 = data3.AddSocialMediaProfile(
        "primaryProfileValue",
        ot::identity::wot::claim::ClaimType::Facebook,
        true,
        false);
    // Verify it is the best profile.
    ASSERT_STREQ(
        "primaryProfileValue",
        data4
            .BestSocialMediaProfile(
                ot::identity::wot::claim::ClaimType::Facebook)
            .c_str());
}

TEST_F(Test_ContactData, Claim_found)
{
    const auto& data1 = contactData_.AddItem(activeContactItem_);
    ASSERT_NE(nullptr, data1.Claim(activeContactItem_->ID()));
}

TEST_F(Test_ContactData, Claim_not_found)
{
    ASSERT_FALSE(contactData_.Claim(activeContactItem_->ID()));
}

TEST_F(Test_ContactData, Contracts)
{
    const auto& data1 = contactData_.AddContract(
        "instrumentDefinitionID1", ot::UnitType::Usd, false, false);
    const auto& contracts = data1.Contracts(ot::UnitType::Usd, false);
    ASSERT_EQ(1, contracts.size());
}

TEST_F(Test_ContactData, Contracts_onlyactive)
{
    const auto& data1 = contactData_.AddContract(
        "instrumentDefinitionID1", ot::UnitType::Usd, false, true);
    const auto& data2 = data1.AddContract(
        "instrumentDefinitionID2", ot::UnitType::Usd, false, false);
    const auto& contracts = data2.Contracts(ot::UnitType::Usd, true);
    ASSERT_EQ(1, contracts.size());
}

TEST_F(Test_ContactData, Delete)
{
    const auto& data1 = contactData_.AddItem(activeContactItem_);
    const auto& contactItem2 = std::shared_ptr<ot::identity::wot::claim::Item>(
        new ot::identity::wot::claim::Item(
            dynamic_cast<const ot::api::session::Client&>(api_),
            ot::UnallocatedCString("contactItem2"),
            opentxs::CONTACT_CONTACT_DATA_VERSION,
            opentxs::CONTACT_CONTACT_DATA_VERSION,
            ot::identity::wot::claim::SectionType::Identifier,
            ot::identity::wot::claim::ClaimType::Employee,
            ot::UnallocatedCString("contactItemValue2"),
            {ot::identity::wot::claim::Attribute::Active},
            NULL_START,
            NULL_END,
            ""));
    const auto& data2 = data1.AddItem(contactItem2);

    const auto& data3 = data2.Delete(activeContactItem_->ID());
    // Verify the item was deleted.
    ASSERT_EQ(
        1,
        data3.Section(ot::identity::wot::claim::SectionType::Identifier)
            ->Size());
    ASSERT_FALSE(data3.Claim(activeContactItem_->ID()));

    const auto& data4 = data3.Delete(activeContactItem_->ID());
    // Verify trying to delete the item again didn't change anything.
    ASSERT_EQ(
        1,
        data4.Section(ot::identity::wot::claim::SectionType::Identifier)
            ->Size());

    const auto& data5 = data4.Delete(contactItem2->ID());
    // Verify the section was removed.
    ASSERT_FALSE(
        data5.Section(ot::identity::wot::claim::SectionType::Identifier));
}

TEST_F(Test_ContactData, EmailAddresses)
{
    const auto& data2 = contactData_.AddEmail("email1", true, false);
    const auto& data3 = data2.AddEmail("email2", false, true);
    const auto& data4 = data3.AddEmail("email3", false, false);

    auto emails = data4.EmailAddresses(false);
    ASSERT_TRUE(
        emails.find("email1") != ot::UnallocatedCString::npos &&
        emails.find("email2") != ot::UnallocatedCString::npos &&
        emails.find("email3") != ot::UnallocatedCString::npos);

    emails = data4.EmailAddresses();
    ASSERT_TRUE(
        emails.find("email1") != ot::UnallocatedCString::npos &&
        emails.find("email2") != ot::UnallocatedCString::npos);
    ASSERT_TRUE(emails.find("email3") == ot::UnallocatedCString::npos);
}

TEST_F(Test_ContactData, Group_found)
{
    const auto& data1 = contactData_.AddItem(activeContactItem_);
    ASSERT_NE(
        nullptr,
        data1.Group(
            ot::identity::wot::claim::SectionType::Identifier,
            ot::identity::wot::claim::ClaimType::Employee));
}

TEST_F(Test_ContactData, Group_notfound)
{
    ASSERT_FALSE(contactData_.Group(
        ot::identity::wot::claim::SectionType::Identifier,
        ot::identity::wot::claim::ClaimType::Employee));
}

TEST_F(Test_ContactData, HaveClaim_1)
{
    ASSERT_FALSE(contactData_.HaveClaim(activeContactItem_->ID()));

    const auto& data1 = contactData_.AddItem(activeContactItem_);
    ASSERT_TRUE(data1.HaveClaim(activeContactItem_->ID()));
}

TEST_F(Test_ContactData, HaveClaim_2)
{
    // Test for an item in group that doesn't exist.
    ASSERT_FALSE(contactData_.HaveClaim(
        ot::identity::wot::claim::SectionType::Identifier,
        ot::identity::wot::claim::ClaimType::Employee,
        "activeContactItemValue"));

    // Test for an item that does exist.
    const auto& data1 = contactData_.AddItem(activeContactItem_);
    ASSERT_TRUE(data1.HaveClaim(
        ot::identity::wot::claim::SectionType::Identifier,
        ot::identity::wot::claim::ClaimType::Employee,
        "activeContactItemValue"));

    // Test for an item that doesn't exist in a group that does.
    ASSERT_FALSE(data1.HaveClaim(
        ot::identity::wot::claim::SectionType::Identifier,
        ot::identity::wot::claim::ClaimType::Employee,
        "dummyContactItemValue"));
}

TEST_F(Test_ContactData, Name)
{
    // Verify that Name returns an empty string if there is no scope group.
    ASSERT_STREQ("", contactData_.Name().c_str());

    // Test when the scope group is emtpy.
    const auto& group1 = std::shared_ptr<ot::identity::wot::claim::Group>(
        new ot::identity::wot::claim::Group(
            "contactGroup1",
            ot::identity::wot::claim::SectionType::Scope,
            ot::identity::wot::claim::ClaimType::Individual,
            {}));

    const auto& section1 = std::shared_ptr<ot::identity::wot::claim::Section>(
        new ot::identity::wot::claim::Section(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactSectionNym1",
            opentxs::CONTACT_CONTACT_DATA_VERSION,
            opentxs::CONTACT_CONTACT_DATA_VERSION,
            ot::identity::wot::claim::SectionType::Scope,
            ot::identity::wot::claim::Section::GroupMap{
                {ot::identity::wot::claim::ClaimType::Individual, group1}}));

    const ot::identity::wot::claim::Data data1(
        dynamic_cast<const ot::api::session::Client&>(api_),
        ot::UnallocatedCString("contactDataNym1"),
        opentxs::CONTACT_CONTACT_DATA_VERSION,
        opentxs::CONTACT_CONTACT_DATA_VERSION,
        ot::identity::wot::claim::Data::SectionMap{
            {ot::identity::wot::claim::SectionType::Scope, section1}});
    // Verify that Name returns an empty string.
    ASSERT_STREQ("", data1.Name().c_str());

    // Test when the scope is set.
    const auto& data2 = contactData_.SetScope(
        ot::identity::wot::claim::ClaimType::Individual,
        "activeContactItemValue");
    ASSERT_STREQ("activeContactItemValue", data2.Name().c_str());
}

TEST_F(Test_ContactData, PhoneNumbers)
{
    const auto& data2 =
        contactData_.AddPhoneNumber("phonenumber1", true, false);
    const auto& data3 = data2.AddPhoneNumber("phonenumber2", false, true);
    const auto& data4 = data3.AddPhoneNumber("phonenumber3", false, false);

    auto phonenumbers = data4.PhoneNumbers(false);
    ASSERT_TRUE(
        phonenumbers.find("phonenumber1") != ot::UnallocatedCString::npos &&
        phonenumbers.find("phonenumber2") != ot::UnallocatedCString::npos &&
        phonenumbers.find("phonenumber3") != ot::UnallocatedCString::npos);

    phonenumbers = data4.PhoneNumbers();
    ASSERT_TRUE(
        phonenumbers.find("phonenumber1") != ot::UnallocatedCString::npos &&
        phonenumbers.find("phonenumber2") != ot::UnallocatedCString::npos);
    ASSERT_TRUE(
        phonenumbers.find("phonenumber3") == ot::UnallocatedCString::npos);
}

TEST_F(Test_ContactData, PreferredOTServer)
{
    // Test getting the preferred server with no group.
    const auto& identifier = contactData_.PreferredOTServer();
    ASSERT_TRUE(identifier->empty());

    // Test getting the preferred server with an empty group.
    const auto& group1 = std::shared_ptr<ot::identity::wot::claim::Group>(
        new ot::identity::wot::claim::Group(
            "contactGroup1",
            ot::identity::wot::claim::SectionType::Communication,
            ot::identity::wot::claim::ClaimType::Opentxs,
            {}));

    const auto& section1 = std::shared_ptr<ot::identity::wot::claim::Section>(
        new ot::identity::wot::claim::Section(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactSectionNym1",
            opentxs::CONTACT_CONTACT_DATA_VERSION,
            opentxs::CONTACT_CONTACT_DATA_VERSION,
            ot::identity::wot::claim::SectionType::Communication,
            ot::identity::wot::claim::Section::GroupMap{
                {ot::identity::wot::claim::ClaimType::Opentxs, group1}}));

    const ot::identity::wot::claim::Data data1(
        dynamic_cast<const ot::api::session::Client&>(api_),
        ot::UnallocatedCString("contactDataNym1"),
        opentxs::CONTACT_CONTACT_DATA_VERSION,
        opentxs::CONTACT_CONTACT_DATA_VERSION,
        ot::identity::wot::claim::Data::SectionMap{
            {ot::identity::wot::claim::SectionType::Communication, section1}});

    const auto& identifier2 = data1.PreferredOTServer();
    ASSERT_TRUE(identifier2->empty());

    // Test getting the preferred server.
    const ot::OTIdentifier serverIdentifier2(
        ot::Identifier::Factory(ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactDataNym",
            ot::identity::wot::claim::SectionType::Communication,
            ot::identity::wot::claim::ClaimType::Opentxs,
            NULL_START,
            NULL_END,
            ot::UnallocatedCString("serverID2"),
            "")));
    const auto& data2 =
        contactData_.AddPreferredOTServer(serverIdentifier2, true);
    const auto& preferredServer = data2.PreferredOTServer();
    ASSERT_FALSE(preferredServer->empty());
    ASSERT_EQ(serverIdentifier2, preferredServer);
}

TEST_F(Test_ContactData, Section)
{
    ASSERT_FALSE(contactData_.Section(
        ot::identity::wot::claim::SectionType::Identifier));

    const auto& data1 = contactData_.AddItem(activeContactItem_);
    ASSERT_NE(
        nullptr,
        data1.Section(ot::identity::wot::claim::SectionType::Identifier));
}

TEST_F(Test_ContactData, SetCommonName)
{
    const auto& data1 = contactData_.SetCommonName("commonName");
    const ot::OTIdentifier identifier(
        ot::Identifier::Factory(ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactDataNym",
            ot::identity::wot::claim::SectionType::Identifier,
            ot::identity::wot::claim::ClaimType::Commonname,
            NULL_START,
            NULL_END,
            ot::UnallocatedCString("commonName"),
            "")));
    const auto& commonNameItem = data1.Claim(identifier);
    ASSERT_NE(nullptr, commonNameItem);
    ASSERT_TRUE(commonNameItem->isPrimary());
    ASSERT_TRUE(commonNameItem->isActive());
}

TEST_F(Test_ContactData, SetName)
{
    const auto& data1 = contactData_.SetScope(
        ot::identity::wot::claim::ClaimType::Individual, "firstName");

    // Test that SetName creates a scope item.
    const auto& data2 = data1.SetName("secondName");
    // Verify the item was created in the scope section and made primary.
    const ot::OTIdentifier identifier1(
        ot::Identifier::Factory(ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactDataNym",
            ot::identity::wot::claim::SectionType::Scope,
            ot::identity::wot::claim::ClaimType::Individual,
            NULL_START,
            NULL_END,
            ot::UnallocatedCString("secondName"),
            "")));
    const auto& scopeItem1 = data2.Claim(identifier1);
    ASSERT_NE(nullptr, scopeItem1);
    ASSERT_TRUE(scopeItem1->isPrimary());
    ASSERT_TRUE(scopeItem1->isActive());

    // Test that SetName creates an item in the scope section without making it
    // primary.
    const auto& data3 = data2.SetName("thirdName", false);
    const ot::OTIdentifier identifier2(
        ot::Identifier::Factory(ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactDataNym",
            ot::identity::wot::claim::SectionType::Scope,
            ot::identity::wot::claim::ClaimType::Individual,
            NULL_START,
            NULL_END,
            ot::UnallocatedCString("thirdName"),
            "")));
    const auto& contactItem2 = data3.Claim(identifier2);
    ASSERT_NE(nullptr, contactItem2);
    ASSERT_FALSE(contactItem2->isPrimary());
    ASSERT_TRUE(contactItem2->isActive());
}

TEST_F(Test_ContactData, SetScope)
{
    const auto& data1 = contactData_.SetScope(
        ot::identity::wot::claim::ClaimType::Organization, "organizationScope");
    // Verify the scope item was created.
    const ot::OTIdentifier identifier1(
        ot::Identifier::Factory(ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactDataNym",
            ot::identity::wot::claim::SectionType::Scope,
            ot::identity::wot::claim::ClaimType::Organization,
            NULL_START,
            NULL_END,
            ot::UnallocatedCString("organizationScope"),
            "")));
    const auto& scopeItem1 = data1.Claim(identifier1);
    ASSERT_NE(nullptr, scopeItem1);
    ASSERT_TRUE(scopeItem1->isPrimary());
    ASSERT_TRUE(scopeItem1->isActive());

    // Test when there is an existing scope.
    const auto& data2 = data1.SetScope(
        ot::identity::wot::claim::ClaimType::Organization, "businessScope");
    // Verify the item wasn't added.
    const ot::OTIdentifier identifier2(
        ot::Identifier::Factory(ot::identity::credential::Contact::ClaimID(
            dynamic_cast<const ot::api::session::Client&>(api_),
            "contactDataNym",
            ot::identity::wot::claim::SectionType::Scope,
            ot::identity::wot::claim::ClaimType::Business,
            NULL_START,
            NULL_END,
            ot::UnallocatedCString("businessScope"),
            "")));
    ASSERT_FALSE(data2.Claim(identifier2));
    // Verify the scope wasn't changed.
    const auto& scopeItem2 = data2.Claim(identifier1);
    ASSERT_NE(nullptr, scopeItem2);
    ASSERT_TRUE(scopeItem2->isPrimary());
    ASSERT_TRUE(scopeItem2->isActive());
}

TEST_F(Test_ContactData, SetScope_different_versions)
{
    const ot::identity::wot::claim::Data data1(
        dynamic_cast<const ot::api::session::Client&>(api_),
        ot::UnallocatedCString("dataNym1"),
        3,  // version of CONTACTSECTION_SCOPE section before CITEMTYPE_BOT
            // was added
        3,
        {});

    const auto& data2 =
        data1.SetScope(ot::identity::wot::claim::ClaimType::Bot, "botScope");

    ASSERT_EQ(4, data2.Version());
}

TEST_F(Test_ContactData, SocialMediaProfiles)
{
    const auto& data2 = contactData_.AddSocialMediaProfile(
        "facebook1",
        ot::identity::wot::claim::ClaimType::Facebook,
        true,
        false);
    const auto& data3 = data2.AddSocialMediaProfile(
        "linkedin1",
        ot::identity::wot::claim::ClaimType::Linkedin,
        false,
        true);
    const auto& data4 = data3.AddSocialMediaProfile(
        "facebook2",
        ot::identity::wot::claim::ClaimType::Facebook,
        false,
        false);

    auto profiles = data4.SocialMediaProfiles(
        ot::identity::wot::claim::ClaimType::Facebook, false);
    ASSERT_TRUE(
        profiles.find("facebook1") != ot::UnallocatedCString::npos &&
        profiles.find("facebook2") != ot::UnallocatedCString::npos);

    profiles = data4.SocialMediaProfiles(
        ot::identity::wot::claim::ClaimType::Linkedin, false);
    ASSERT_STREQ("linkedin1", profiles.c_str());

    profiles = data4.SocialMediaProfiles(
        ot::identity::wot::claim::ClaimType::Facebook);
    ASSERT_STREQ("facebook1", profiles.c_str());
    ASSERT_TRUE(profiles.find("facebook2") == ot::UnallocatedCString::npos);
    ASSERT_TRUE(profiles.find("linkedin1") == ot::UnallocatedCString::npos);
}

TEST_F(Test_ContactData, Type)
{
    ASSERT_EQ(
        ot::identity::wot::claim::ClaimType::Unknown, contactData_.Type());

    const auto& data1 = contactData_.SetScope(
        ot::identity::wot::claim::ClaimType::Individual, "scopeName");
    ASSERT_EQ(ot::identity::wot::claim::ClaimType::Individual, data1.Type());
}

TEST_F(Test_ContactData, Version)
{
    ASSERT_EQ(opentxs::CONTACT_CONTACT_DATA_VERSION, contactData_.Version());
}
}  // namespace ottest
