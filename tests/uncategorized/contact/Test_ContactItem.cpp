// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>

#include "internal/identity/wot/claim/Types.hpp"
#include "ottest/env/OTTestEnvironment.hpp"

namespace ot = opentxs;

namespace ottest
{
class Test_ContactItem : public ::testing::Test
{
public:
    Test_ContactItem()
        : api_(OTTestEnvironment::GetOT().StartClientSession(0))
        , contact_item_(
              dynamic_cast<const ot::api::session::Client&>(api_),
              ot::UnallocatedCString("testNym"),
              opentxs::CONTACT_CONTACT_DATA_VERSION,
              opentxs::CONTACT_CONTACT_DATA_VERSION,
              ot::identity::wot::claim::SectionType::Identifier,
              ot::identity::wot::claim::ClaimType::Employee,
              ot::UnallocatedCString("testValue"),
              {ot::identity::wot::claim::Attribute::Active},
              {},
              {},
              "")
        , nym_id_(api_.Factory().NymIDFromRandom())
    {
    }

    const ot::api::session::Client& api_;
    const ot::identity::wot::claim::Item contact_item_;
    const ot::identifier::Nym nym_id_;
};

TEST_F(Test_ContactItem, first_constructor)
{
    const ot::identity::wot::claim::Item contactItem1(
        dynamic_cast<const ot::api::session::Client&>(api_),
        nym_id_.asBase58(api_.Crypto()),
        opentxs::CONTACT_CONTACT_DATA_VERSION,
        opentxs::CONTACT_CONTACT_DATA_VERSION,
        ot::identity::wot::claim::SectionType::Identifier,
        ot::identity::wot::claim::ClaimType::Employee,
        ot::UnallocatedCString("testValue"),
        {ot::identity::wot::claim::Attribute::Active},
        {},
        {},
        "");

    const ot::identifier::Generic identifier(
        api_.Factory().IdentifierFromBase58(
            ot::identity::credential::Contact::ClaimID(
                dynamic_cast<const ot::api::session::Client&>(api_),
                nym_id_.asBase58(api_.Crypto()),
                ot::identity::wot::claim::SectionType::Identifier,
                ot::identity::wot::claim::ClaimType::Employee,
                {},
                {},
                "testValue",
                "")));
    EXPECT_EQ(identifier, contactItem1.ID());
    EXPECT_EQ(opentxs::CONTACT_CONTACT_DATA_VERSION, contactItem1.Version());
    EXPECT_EQ(
        ot::identity::wot::claim::SectionType::Identifier,
        contactItem1.Section());
    EXPECT_EQ(
        ot::identity::wot::claim::ClaimType::Employee, contactItem1.Type());
    EXPECT_EQ("testValue", contactItem1.Value());
    EXPECT_EQ(contactItem1.Start(), ot::Time{});
    EXPECT_EQ(contactItem1.End(), ot::Time{});

    EXPECT_TRUE(contactItem1.isActive());
    EXPECT_FALSE(contactItem1.isLocal());
    EXPECT_FALSE(contactItem1.isPrimary());
}

TEST_F(Test_ContactItem, first_constructor_different_versions)
{
    const ot::identity::wot::claim::Item contactItem1(
        dynamic_cast<const ot::api::session::Client&>(api_),
        nym_id_.asBase58(api_.Crypto()),
        opentxs::CONTACT_CONTACT_DATA_VERSION - 1,  // previous version
        opentxs::CONTACT_CONTACT_DATA_VERSION,
        ot::identity::wot::claim::SectionType::Identifier,
        ot::identity::wot::claim::ClaimType::Employee,
        ot::UnallocatedCString("testValue"),
        {ot::identity::wot::claim::Attribute::Active},
        {},
        {},
        "");
    EXPECT_EQ(opentxs::CONTACT_CONTACT_DATA_VERSION, contactItem1.Version());
}

TEST_F(Test_ContactItem, second_constructor)
{
    static constexpr auto attrib = {
        ot::identity::wot::claim::Attribute::Active};
    const auto claim = api_.Factory().Claim(
        nym_id_,
        ot::identity::wot::claim::SectionType::Identifier,
        ot::identity::wot::claim::ClaimType::Employee,
        "testValue",
        {},
        attrib);
    const ot::identity::wot::claim::Item contactItem1(
        dynamic_cast<const ot::api::session::Client&>(api_),
        nym_id_.asBase58(api_.Crypto()),
        opentxs::CONTACT_CONTACT_DATA_VERSION,
        opentxs::CONTACT_CONTACT_DATA_VERSION,
        claim);

    const ot::identifier::Generic identifier(
        api_.Factory().IdentifierFromBase58(
            ot::identity::credential::Contact::ClaimID(
                dynamic_cast<const ot::api::session::Client&>(api_),
                nym_id_.asBase58(api_.Crypto()),
                ot::identity::wot::claim::SectionType::Identifier,
                ot::identity::wot::claim::ClaimType::Employee,
                {},
                {},
                "testValue",
                "")));
    EXPECT_EQ(identifier.asHex(), contactItem1.ID().asHex());
    EXPECT_EQ(opentxs::CONTACT_CONTACT_DATA_VERSION, contactItem1.Version());
    EXPECT_EQ(
        ot::identity::wot::claim::SectionType::Identifier,
        contactItem1.Section());
    EXPECT_EQ(
        ot::identity::wot::claim::ClaimType::Employee, contactItem1.Type());
    EXPECT_EQ("testValue", contactItem1.Value());
    EXPECT_EQ(contactItem1.Start(), ot::Time{});
    EXPECT_EQ(contactItem1.End(), ot::Time{});

    EXPECT_TRUE(contactItem1.isActive());
    EXPECT_FALSE(contactItem1.isLocal());
    EXPECT_FALSE(contactItem1.isPrimary());
}

TEST_F(Test_ContactItem, copy_constructor)
{
    ot::identity::wot::claim::Item copiedContactItem(contact_item_);

    EXPECT_EQ(contact_item_.ID(), copiedContactItem.ID());
    EXPECT_EQ(contact_item_.Version(), copiedContactItem.Version());
    EXPECT_EQ(contact_item_.Section(), copiedContactItem.Section());
    EXPECT_EQ(contact_item_.Type(), copiedContactItem.Type());
    EXPECT_EQ(contact_item_.Value(), copiedContactItem.Value());
    EXPECT_EQ(contact_item_.Start(), copiedContactItem.Start());
    EXPECT_EQ(contact_item_.End(), copiedContactItem.End());

    EXPECT_EQ(contact_item_.isActive(), copiedContactItem.isActive());
    EXPECT_EQ(contact_item_.isLocal(), copiedContactItem.isLocal());
    EXPECT_EQ(contact_item_.isPrimary(), copiedContactItem.isPrimary());
}

TEST_F(Test_ContactItem, operator_equal_true)
{
    EXPECT_EQ(contact_item_, contact_item_);
}

TEST_F(Test_ContactItem, operator_equal_false)
{
    ot::identity::wot::claim::Item contactItem2(
        dynamic_cast<const ot::api::session::Client&>(api_),
        ot::UnallocatedCString("testNym2"),
        opentxs::CONTACT_CONTACT_DATA_VERSION,
        opentxs::CONTACT_CONTACT_DATA_VERSION,
        ot::identity::wot::claim::SectionType::Identifier,
        ot::identity::wot::claim::ClaimType::Employee,
        ot::UnallocatedCString("testValue2"),
        {ot::identity::wot::claim::Attribute::Active},
        {},
        {},
        "");

    // Can't use EXPECT_NE because there's no != operator defined for
    // ContactItem.
    EXPECT_FALSE(contact_item_ == contactItem2);
}

TEST_F(Test_ContactItem, public_accessors)
{
    const ot::identifier::Generic identifier(
        api_.Factory().IdentifierFromBase58(
            ot::identity::credential::Contact::ClaimID(
                dynamic_cast<const ot::api::session::Client&>(api_),
                "testNym",
                ot::identity::wot::claim::SectionType::Identifier,
                ot::identity::wot::claim::ClaimType::Employee,
                {},
                {},
                "testValue",
                "")));
    EXPECT_EQ(identifier, contact_item_.ID());
    EXPECT_EQ(
        ot::identity::wot::claim::SectionType::Identifier,
        contact_item_.Section());
    EXPECT_EQ(
        ot::identity::wot::claim::ClaimType::Employee, contact_item_.Type());
    EXPECT_EQ("testValue", contact_item_.Value());
    EXPECT_EQ(contact_item_.Start(), ot::Time{});
    EXPECT_EQ(contact_item_.End(), ot::Time{});
    EXPECT_EQ(opentxs::CONTACT_CONTACT_DATA_VERSION, contact_item_.Version());

    EXPECT_TRUE(contact_item_.isActive());
    EXPECT_FALSE(contact_item_.isLocal());
    EXPECT_FALSE(contact_item_.isPrimary());
}

TEST_F(Test_ContactItem, public_setters)
{
    const auto now = ot::Clock::now();

    const auto& valueItem = contact_item_.SetValue("newTestValue");
    EXPECT_FALSE(valueItem == contact_item_);
    EXPECT_STREQ(valueItem.Value().c_str(), "newTestValue");

    const auto& startItem = contact_item_.SetStart(now);
    EXPECT_FALSE(startItem == contact_item_);
    EXPECT_EQ(now, startItem.Start());
    EXPECT_NE(startItem.Start(), ot::Time{});

    const auto& endItem = contact_item_.SetEnd(now);
    EXPECT_FALSE(endItem == contact_item_);
    EXPECT_EQ(now, endItem.End());
    EXPECT_NE(ot::Time{}, endItem.End());

    // _contactItem is active, so test setting active to false first.
    const auto& notActiveItem = contact_item_.SetActive(false);
    EXPECT_FALSE(notActiveItem == contact_item_);
    EXPECT_FALSE(notActiveItem.isActive());
    const auto& activeItem = notActiveItem.SetActive(true);
    EXPECT_FALSE(activeItem == notActiveItem);
    EXPECT_TRUE(activeItem.isActive());

    const auto& localItem = contact_item_.SetLocal(true);
    EXPECT_FALSE(localItem == contact_item_);
    EXPECT_TRUE(localItem.isLocal());
    const auto& notLocalItem = localItem.SetLocal(false);
    EXPECT_FALSE(notLocalItem == localItem);
    EXPECT_FALSE(notLocalItem.isLocal());

    // First, create an item with no attributes.
    const auto& notPrimaryItem = contact_item_.SetActive(false);
    EXPECT_FALSE(notPrimaryItem == contact_item_);
    EXPECT_FALSE(notPrimaryItem.isPrimary());
    EXPECT_FALSE(notPrimaryItem.isActive());
    EXPECT_FALSE(notPrimaryItem.isLocal());
    // Now, set the primary attribute, and test for primary and active.
    const auto& primaryItem = notPrimaryItem.SetPrimary(true);
    EXPECT_FALSE(primaryItem == notPrimaryItem);
    EXPECT_TRUE(primaryItem.isPrimary());
    EXPECT_TRUE(primaryItem.isActive());
}

TEST_F(Test_ContactItem, Serialize)
{
    // Test without id.
    auto bytes = ot::Space{};
    EXPECT_TRUE(contact_item_.Serialize(ot::writer(bytes), false));

    auto restored1 = ot::identity::wot::claim::Item{
        dynamic_cast<const ot::api::session::Client&>(api_),
        "testNym",
        contact_item_.Version(),
        contact_item_.Section(),
        ot::reader(bytes)};

    EXPECT_EQ(restored1.Value(), contact_item_.Value());
    EXPECT_EQ(restored1.Version(), contact_item_.Version());
    EXPECT_EQ(restored1.Type(), contact_item_.Type());
    EXPECT_EQ(restored1.Start(), contact_item_.Start());
    EXPECT_EQ(restored1.End(), contact_item_.End());

    // Test with id.
    EXPECT_TRUE(contact_item_.Serialize(ot::writer(bytes), true));

    auto restored2 = ot::identity::wot::claim::Item{
        dynamic_cast<const ot::api::session::Client&>(api_),
        "testNym",
        contact_item_.Version(),
        contact_item_.Section(),
        ot::reader(bytes)};

    EXPECT_EQ(restored2.Value(), contact_item_.Value());
    EXPECT_EQ(restored2.Version(), contact_item_.Version());
    EXPECT_EQ(restored2.Type(), contact_item_.Type());
    EXPECT_EQ(restored2.Start(), contact_item_.Start());
    EXPECT_EQ(restored2.End(), contact_item_.End());
}
}  // namespace ottest
