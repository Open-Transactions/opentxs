// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <chrono>
#include <optional>
#include <ratio>
#include <span>
#include <string_view>

#include "ottest/fixtures/contact/ContactItem.hpp"

namespace ottest
{
using namespace std::literals;

TEST_F(ContactItem, first_constructor)
{
    const auto contactItem1 = claim_to_contact_item(client_1_.Factory().Claim(
        nym_id_,
        ot::identity::wot::claim::SectionType::Identifier,
        ot::identity::wot::claim::ClaimType::Employee,
        "testValue",
        active_,
        {},
        {},
        {},
        opentxs::identity::wot::claim::DefaultVersion()));
    const auto identifier = ot::identity::credential::Contact::ClaimID(
        client_1_,
        nym_id_,
        ot::identity::wot::claim::SectionType::Identifier,
        ot::identity::wot::claim::ClaimType::Employee,
        {},
        {},
        "testValue",
        "",
        ot::identity::wot::claim::DefaultVersion());

    EXPECT_EQ(identifier, contactItem1.ID());
    EXPECT_EQ(
        opentxs::identity::wot::claim::DefaultVersion(),
        contactItem1.Version());
    EXPECT_EQ(
        ot::identity::wot::claim::SectionType::Identifier,
        contactItem1.Section());
    EXPECT_EQ(
        ot::identity::wot::claim::ClaimType::Employee, contactItem1.Type());
    EXPECT_EQ("testValue", contactItem1.Value());
    EXPECT_EQ(contactItem1.Start(), ot::Time{});
    EXPECT_EQ(contactItem1.End(), ot::Time{});
    EXPECT_TRUE(contactItem1.HasAttribute(
        opentxs::identity::wot::claim::Attribute::Active));
    EXPECT_FALSE(contactItem1.HasAttribute(
        opentxs::identity::wot::claim::Attribute::Local));
    EXPECT_FALSE(contactItem1.HasAttribute(
        opentxs::identity::wot::claim::Attribute::Primary));
}

TEST_F(ContactItem, first_constructor_different_versions)
{
    const auto version = opentxs::identity::wot::claim::DefaultVersion() - 1;
    const auto contactItem1 = claim_to_contact_item(client_1_.Factory().Claim(
        nym_id_,
        ot::identity::wot::claim::SectionType::Identifier,
        ot::identity::wot::claim::ClaimType::Employee,
        "testValue",
        active_,
        {},
        {},
        {},
        version));

    EXPECT_EQ(version, contactItem1.Version());
}

TEST_F(ContactItem, second_constructor)
{
    const auto claim = client_1_.Factory().Claim(
        nym_id_,
        ot::identity::wot::claim::SectionType::Identifier,
        ot::identity::wot::claim::ClaimType::Employee,
        "testValue",
        active_);
    const auto contactItem1 = claim_to_contact_item(claim);
    const auto identifier = ot::identity::credential::Contact::ClaimID(
        client_1_,
        nym_id_,
        ot::identity::wot::claim::SectionType::Identifier,
        ot::identity::wot::claim::ClaimType::Employee,
        {},
        {},
        "testValue",
        "",
        ot::identity::wot::claim::DefaultVersion());

    EXPECT_EQ(identifier.asHex(), contactItem1.ID().asHex());
    EXPECT_EQ(
        opentxs::identity::wot::claim::DefaultVersion(),
        contactItem1.Version());
    EXPECT_EQ(
        ot::identity::wot::claim::SectionType::Identifier,
        contactItem1.Section());
    EXPECT_EQ(
        ot::identity::wot::claim::ClaimType::Employee, contactItem1.Type());
    EXPECT_EQ("testValue", contactItem1.Value());
    EXPECT_EQ(contactItem1.Start(), ot::Time{});
    EXPECT_EQ(contactItem1.End(), ot::Time{});
    EXPECT_TRUE(contactItem1.HasAttribute(
        opentxs::identity::wot::claim::Attribute::Active));
    EXPECT_FALSE(contactItem1.HasAttribute(
        opentxs::identity::wot::claim::Attribute::Local));
    EXPECT_FALSE(contactItem1.HasAttribute(
        opentxs::identity::wot::claim::Attribute::Primary));
}

TEST_F(ContactItem, copy_constructor)
{
    const auto copiedContactItem{contact_item_};

    EXPECT_EQ(contact_item_, copiedContactItem);
    EXPECT_EQ(contact_item_.ID(), copiedContactItem.ID());
    EXPECT_EQ(contact_item_.Version(), copiedContactItem.Version());
    EXPECT_EQ(contact_item_.Section(), copiedContactItem.Section());
    EXPECT_EQ(contact_item_.Type(), copiedContactItem.Type());
    EXPECT_EQ(contact_item_.Value(), copiedContactItem.Value());
    EXPECT_EQ(contact_item_.Subtype(), copiedContactItem.Subtype());
    EXPECT_EQ(contact_item_.Start(), copiedContactItem.Start());
    EXPECT_EQ(contact_item_.End(), copiedContactItem.End());

    EXPECT_EQ(
        contact_item_.HasAttribute(
            opentxs::identity::wot::claim::Attribute::Active),
        copiedContactItem.HasAttribute(
            opentxs::identity::wot::claim::Attribute::Active));
    EXPECT_EQ(
        contact_item_.HasAttribute(
            opentxs::identity::wot::claim::Attribute::Local),
        copiedContactItem.HasAttribute(
            opentxs::identity::wot::claim::Attribute::Local));
    EXPECT_EQ(
        contact_item_.HasAttribute(
            opentxs::identity::wot::claim::Attribute::Primary),
        copiedContactItem.HasAttribute(
            opentxs::identity::wot::claim::Attribute::Primary));
}

TEST_F(ContactItem, operator_equal_true)
{
    EXPECT_EQ(contact_item_, contact_item_);
}

TEST_F(ContactItem, operator_equal_false)
{
    const auto contactItem2 = [&, this] {
        auto out = claim_to_contact_item(client_1_.Factory().Claim(
            nym_id_,
            ot::identity::wot::claim::SectionType::Identifier,
            ot::identity::wot::claim::ClaimType::Employee,
            "testValue2",
            active_));
        out.SetVersion(opentxs::identity::wot::claim::DefaultVersion());

        return out;
    }();

    EXPECT_FALSE(contact_item_ == contactItem2);
    EXPECT_NE(contact_item_, contactItem2);
}

TEST_F(ContactItem, public_accessors)
{
    const auto identifier = ot::identity::credential::Contact::ClaimID(
        client_1_,
        nym_id_,
        ot::identity::wot::claim::SectionType::Identifier,
        ot::identity::wot::claim::ClaimType::Employee,
        {},
        {},
        "testValue",
        "",
        ot::identity::wot::claim::DefaultVersion());

    EXPECT_EQ(identifier, contact_item_.ID());
    EXPECT_EQ(
        ot::identity::wot::claim::SectionType::Identifier,
        contact_item_.Section());
    EXPECT_EQ(
        ot::identity::wot::claim::ClaimType::Employee, contact_item_.Type());
    EXPECT_EQ("testValue", contact_item_.Value());
    EXPECT_EQ(contact_item_.Start(), ot::Time{});
    EXPECT_EQ(contact_item_.End(), ot::Time{});
    EXPECT_EQ(
        opentxs::identity::wot::claim::DefaultVersion(),
        contact_item_.Version());
    EXPECT_TRUE(contact_item_.HasAttribute(
        opentxs::identity::wot::claim::Attribute::Active));
    EXPECT_FALSE(contact_item_.HasAttribute(
        opentxs::identity::wot::claim::Attribute::Local));
    EXPECT_FALSE(contact_item_.HasAttribute(
        opentxs::identity::wot::claim::Attribute::Primary));
}

TEST_F(ContactItem, modify)
{
    const auto start = ot::Clock::now();
    const auto end = start + 1h;
    const auto value = "newTestValue"sv;
    auto modified = modify_item(contact_item_, value, std::nullopt, start, end);

    EXPECT_NE(modified, contact_item_);
    EXPECT_EQ(modified.Section(), contact_item_.Section());
    EXPECT_EQ(modified.Start(), start);
    EXPECT_EQ(modified.Subtype(), contact_item_.Subtype());
    EXPECT_EQ(modified.Type(), contact_item_.Type());
    EXPECT_EQ(modified.Value(), value);
    EXPECT_EQ(modified.Version(), contact_item_.Version());

    using enum opentxs::identity::wot::claim::Attribute;

    {
        modified.Remove(Local);

        EXPECT_FALSE(modified.HasAttribute(Local));

        modified.Add(Local);

        EXPECT_TRUE(modified.HasAttribute(Local));
    }
    {
        modified.Remove(Active);
        modified.Remove(Primary);

        EXPECT_FALSE(modified.HasAttribute(Primary));
        EXPECT_FALSE(modified.HasAttribute(Active));

        modified.Add(Primary);

        EXPECT_TRUE(modified.HasAttribute(Primary));
        EXPECT_TRUE(modified.HasAttribute(Active));
    }
    {
        EXPECT_TRUE(modified.HasAttribute(Primary));
        EXPECT_TRUE(modified.HasAttribute(Active));

        modified.Remove(Active);

        EXPECT_FALSE(modified.HasAttribute(Active));
        EXPECT_FALSE(modified.HasAttribute(Primary));

        modified.Add(Active);

        EXPECT_TRUE(modified.HasAttribute(Active));
        EXPECT_FALSE(modified.HasAttribute(Primary));
    }
}

TEST_F(ContactItem, Serialize)
{
    // Test without id.
    auto bytes = opentxs::ByteArray{};

    EXPECT_TRUE(contact_item_.Serialize(bytes.WriteInto(), false));

    const auto restored1 = deserialize_contact_item(
        client_1_, {nym_id_}, contact_item_.Section(), bytes.Bytes());

    EXPECT_EQ(restored1.Value(), contact_item_.Value());
    EXPECT_EQ(restored1.Version(), contact_item_.Version());
    EXPECT_EQ(restored1.Type(), contact_item_.Type());
    EXPECT_EQ(restored1.Start(), contact_item_.Start());
    EXPECT_EQ(restored1.End(), contact_item_.End());
    EXPECT_EQ(restored1, contact_item_);

    // Test with id.
    EXPECT_TRUE(contact_item_.Serialize(bytes.WriteInto(), true));

    const auto restored2 = deserialize_contact_item(
        client_1_, {nym_id_}, contact_item_.Section(), bytes.Bytes());

    EXPECT_EQ(restored2.Value(), contact_item_.Value());
    EXPECT_EQ(restored2.Version(), contact_item_.Version());
    EXPECT_EQ(restored2.Type(), contact_item_.Type());
    EXPECT_EQ(restored2.Start(), contact_item_.Start());
    EXPECT_EQ(restored2.End(), contact_item_.End());
    EXPECT_EQ(restored2, contact_item_);
}
}  // namespace ottest
