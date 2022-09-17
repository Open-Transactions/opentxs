// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstdint>

#include "1_Internal.hpp"
#include "internal/identity/wot/claim/Types.hpp"

namespace ot = opentxs;

namespace ottest
{
class Test_ContactItem : public ::testing::Test
{
public:
    Test_ContactItem()
        : api_(ot::Context().StartClientSession(0))
        , contactItem_(
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
    {
    }

    const ot::api::session::Client& api_;
    const ot::identity::wot::claim::Item contactItem_;
};

TEST_F(Test_ContactItem, first_constructor)
{
    const ot::identity::wot::claim::Item contactItem1(
        dynamic_cast<const ot::api::session::Client&>(api_),
        ot::UnallocatedCString("testContactItemNym"),
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
                "testContactItemNym",
                ot::identity::wot::claim::SectionType::Identifier,
                ot::identity::wot::claim::ClaimType::Employee,
                {},
                {},
                "testValue",
                "")));
    ASSERT_EQ(identifier, contactItem1.ID());
    ASSERT_EQ(opentxs::CONTACT_CONTACT_DATA_VERSION, contactItem1.Version());
    ASSERT_EQ(
        ot::identity::wot::claim::SectionType::Identifier,
        contactItem1.Section());
    ASSERT_EQ(
        ot::identity::wot::claim::ClaimType::Employee, contactItem1.Type());
    ASSERT_EQ("testValue", contactItem1.Value());
    ASSERT_EQ(contactItem1.Start(), ot::Time{});
    ASSERT_EQ(contactItem1.End(), ot::Time{});

    ASSERT_TRUE(contactItem1.isActive());
    ASSERT_FALSE(contactItem1.isLocal());
    ASSERT_FALSE(contactItem1.isPrimary());
}

TEST_F(Test_ContactItem, first_constructor_different_versions)
{
    const ot::identity::wot::claim::Item contactItem1(
        dynamic_cast<const ot::api::session::Client&>(api_),
        ot::UnallocatedCString("testContactItemNym"),
        opentxs::CONTACT_CONTACT_DATA_VERSION - 1,  // previous version
        opentxs::CONTACT_CONTACT_DATA_VERSION,
        ot::identity::wot::claim::SectionType::Identifier,
        ot::identity::wot::claim::ClaimType::Employee,
        ot::UnallocatedCString("testValue"),
        {ot::identity::wot::claim::Attribute::Active},
        {},
        {},
        "");
    ASSERT_EQ(opentxs::CONTACT_CONTACT_DATA_VERSION, contactItem1.Version());
}

TEST_F(Test_ContactItem, second_constructor)
{
    const ot::identity::wot::claim::Item contactItem1(
        dynamic_cast<const ot::api::session::Client&>(api_),
        ot::UnallocatedCString("testContactItemNym"),
        opentxs::CONTACT_CONTACT_DATA_VERSION,
        opentxs::CONTACT_CONTACT_DATA_VERSION,
        ot::Claim(
            "",
            ot::translate(ot::identity::wot::claim::SectionType::Identifier),
            ot::translate(ot::identity::wot::claim::ClaimType::Employee),
            "testValue",
            {},
            {},
            {static_cast<uint32_t>(
                ot::identity::wot::claim::Attribute::Active)}));

    const ot::identifier::Generic identifier(
        api_.Factory().IdentifierFromBase58(
            ot::identity::credential::Contact::ClaimID(
                dynamic_cast<const ot::api::session::Client&>(api_),
                "testContactItemNym",
                ot::identity::wot::claim::SectionType::Identifier,
                ot::identity::wot::claim::ClaimType::Employee,
                {},
                {},
                "testValue",
                "")));
    ASSERT_EQ(identifier, contactItem1.ID());
    ASSERT_EQ(opentxs::CONTACT_CONTACT_DATA_VERSION, contactItem1.Version());
    ASSERT_EQ(
        ot::identity::wot::claim::SectionType::Identifier,
        contactItem1.Section());
    ASSERT_EQ(
        ot::identity::wot::claim::ClaimType::Employee, contactItem1.Type());
    ASSERT_EQ("testValue", contactItem1.Value());
    ASSERT_EQ(contactItem1.Start(), ot::Time{});
    ASSERT_EQ(contactItem1.End(), ot::Time{});

    ASSERT_TRUE(contactItem1.isActive());
    ASSERT_FALSE(contactItem1.isLocal());
    ASSERT_FALSE(contactItem1.isPrimary());
}

TEST_F(Test_ContactItem, copy_constructor)
{
    ot::identity::wot::claim::Item copiedContactItem(contactItem_);

    ASSERT_EQ(contactItem_.ID(), copiedContactItem.ID());
    ASSERT_EQ(contactItem_.Version(), copiedContactItem.Version());
    ASSERT_EQ(contactItem_.Section(), copiedContactItem.Section());
    ASSERT_EQ(contactItem_.Type(), copiedContactItem.Type());
    ASSERT_EQ(contactItem_.Value(), copiedContactItem.Value());
    ASSERT_EQ(contactItem_.Start(), copiedContactItem.Start());
    ASSERT_EQ(contactItem_.End(), copiedContactItem.End());

    ASSERT_EQ(contactItem_.isActive(), copiedContactItem.isActive());
    ASSERT_EQ(contactItem_.isLocal(), copiedContactItem.isLocal());
    ASSERT_EQ(contactItem_.isPrimary(), copiedContactItem.isPrimary());
}

TEST_F(Test_ContactItem, operator_equal_true)
{
    ASSERT_EQ(contactItem_, contactItem_);
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

    // Can't use ASSERT_NE because there's no != operator defined for
    // ContactItem.
    ASSERT_FALSE(contactItem_ == contactItem2);
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
    ASSERT_EQ(identifier, contactItem_.ID());
    ASSERT_EQ(
        ot::identity::wot::claim::SectionType::Identifier,
        contactItem_.Section());
    ASSERT_EQ(
        ot::identity::wot::claim::ClaimType::Employee, contactItem_.Type());
    ASSERT_EQ("testValue", contactItem_.Value());
    ASSERT_EQ(contactItem_.Start(), ot::Time{});
    ASSERT_EQ(contactItem_.End(), ot::Time{});
    ASSERT_EQ(opentxs::CONTACT_CONTACT_DATA_VERSION, contactItem_.Version());

    ASSERT_TRUE(contactItem_.isActive());
    ASSERT_FALSE(contactItem_.isLocal());
    ASSERT_FALSE(contactItem_.isPrimary());
}

TEST_F(Test_ContactItem, public_setters)
{
    const auto now = ot::Clock::now();

    const auto& valueItem = contactItem_.SetValue("newTestValue");
    ASSERT_FALSE(valueItem == contactItem_);
    ASSERT_STREQ(valueItem.Value().c_str(), "newTestValue");

    const auto& startItem = contactItem_.SetStart(now);
    ASSERT_FALSE(startItem == contactItem_);
    ASSERT_EQ(now, startItem.Start());
    ASSERT_NE(startItem.Start(), ot::Time{});

    const auto& endItem = contactItem_.SetEnd(now);
    ASSERT_FALSE(endItem == contactItem_);
    ASSERT_EQ(now, endItem.End());
    ASSERT_NE(ot::Time{}, endItem.End());

    // _contactItem is active, so test setting active to false first.
    const auto& notActiveItem = contactItem_.SetActive(false);
    ASSERT_FALSE(notActiveItem == contactItem_);
    ASSERT_FALSE(notActiveItem.isActive());
    const auto& activeItem = notActiveItem.SetActive(true);
    ASSERT_FALSE(activeItem == notActiveItem);
    ASSERT_TRUE(activeItem.isActive());

    const auto& localItem = contactItem_.SetLocal(true);
    ASSERT_FALSE(localItem == contactItem_);
    ASSERT_TRUE(localItem.isLocal());
    const auto& notLocalItem = localItem.SetLocal(false);
    ASSERT_FALSE(notLocalItem == localItem);
    ASSERT_FALSE(notLocalItem.isLocal());

    // First, create an item with no attributes.
    const auto& notPrimaryItem = contactItem_.SetActive(false);
    ASSERT_FALSE(notPrimaryItem == contactItem_);
    ASSERT_FALSE(notPrimaryItem.isPrimary());
    ASSERT_FALSE(notPrimaryItem.isActive());
    ASSERT_FALSE(notPrimaryItem.isLocal());
    // Now, set the primary attribute, and test for primary and active.
    const auto& primaryItem = notPrimaryItem.SetPrimary(true);
    ASSERT_FALSE(primaryItem == notPrimaryItem);
    ASSERT_TRUE(primaryItem.isPrimary());
    ASSERT_TRUE(primaryItem.isActive());
}

TEST_F(Test_ContactItem, Serialize)
{
    // Test without id.
    auto bytes = ot::Space{};
    EXPECT_TRUE(contactItem_.Serialize(ot::writer(bytes), false));

    auto restored1 = ot::identity::wot::claim::Item{
        dynamic_cast<const ot::api::session::Client&>(api_),
        "testNym",
        contactItem_.Version(),
        contactItem_.Section(),
        ot::reader(bytes)};

    ASSERT_EQ(restored1.Value(), contactItem_.Value());
    ASSERT_EQ(restored1.Version(), contactItem_.Version());
    ASSERT_EQ(restored1.Type(), contactItem_.Type());
    ASSERT_EQ(restored1.Start(), contactItem_.Start());
    ASSERT_EQ(restored1.End(), contactItem_.End());

    // Test with id.
    EXPECT_TRUE(contactItem_.Serialize(ot::writer(bytes), true));

    auto restored2 = ot::identity::wot::claim::Item{
        dynamic_cast<const ot::api::session::Client&>(api_),
        "testNym",
        contactItem_.Version(),
        contactItem_.Section(),
        ot::reader(bytes)};

    ASSERT_EQ(restored2.Value(), contactItem_.Value());
    ASSERT_EQ(restored2.Version(), contactItem_.Version());
    ASSERT_EQ(restored2.Type(), contactItem_.Type());
    ASSERT_EQ(restored2.Start(), contactItem_.Start());
    ASSERT_EQ(restored2.End(), contactItem_.End());
}
}  // namespace ottest
