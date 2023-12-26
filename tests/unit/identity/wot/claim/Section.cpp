// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <iterator>
#include <memory>
#include <span>
#include <utility>

#include "ottest/fixtures/contact/ContactItem.hpp"
#include "ottest/fixtures/contact/ContactSection.hpp"

namespace ottest
{
TEST_F(ContactSection, first_constructor)
{
    const auto& group1 = std::make_shared<ot::identity::wot::claim::Group>(
        contact_group_->AddItem(active_contact_item_));
    auto groupMap = ot::identity::wot::claim::Section::GroupMap{
        {ot::identity::wot::claim::ClaimType::Employee, group1}};
    const auto section1 = ot::identity::wot::claim::Section{
        client_1_,
        "testContactSectionNym1",
        opentxs::identity::wot::claim::DefaultVersion(),
        opentxs::identity::wot::claim::DefaultVersion(),
        ot::identity::wot::claim::SectionType::Identifier,
        groupMap};

    ASSERT_EQ(
        ot::identity::wot::claim::SectionType::Identifier, section1.Type());
    ASSERT_EQ(
        opentxs::identity::wot::claim::DefaultVersion(), section1.Version());
    ASSERT_EQ(section1.Size(), 1);
    ASSERT_EQ(
        group1->Size(),
        section1.Group(ot::identity::wot::claim::ClaimType::Employee)->Size());
    ASSERT_NE(section1.end(), section1.begin());
}

TEST_F(ContactSection, first_constructor_different_versions)
{
    // Test private static method check_version.
    const ot::identity::wot::claim::Section section2(
        client_1_,
        "testContactSectionNym2",
        opentxs::identity::wot::claim::DefaultVersion() - 1,  // previous
                                                              // version
        opentxs::identity::wot::claim::DefaultVersion(),
        ot::identity::wot::claim::SectionType::Identifier,
        ot::identity::wot::claim::Section::GroupMap{});
    ASSERT_EQ(
        opentxs::identity::wot::claim::DefaultVersion(), section2.Version());
}

TEST_F(ContactSection, second_constructor)
{
    const ot::identity::wot::claim::Section section1(
        client_1_,
        "testContactSectionNym1",
        opentxs::identity::wot::claim::DefaultVersion(),
        opentxs::identity::wot::claim::DefaultVersion(),
        ot::identity::wot::claim::SectionType::Identifier,
        active_contact_item_);
    ASSERT_EQ(
        ot::identity::wot::claim::SectionType::Identifier, section1.Type());
    ASSERT_EQ(
        opentxs::identity::wot::claim::DefaultVersion(), section1.Version());
    ASSERT_EQ(section1.Size(), 1);
    ASSERT_NE(
        nullptr, section1.Group(ot::identity::wot::claim::ClaimType::Employee));
    ASSERT_EQ(
        section1.Group(ot::identity::wot::claim::ClaimType::Employee)->Size(),
        1);
    ASSERT_NE(section1.end(), section1.begin());
}

TEST_F(ContactSection, copy_constructor)
{
    const auto& section1(contact_section_.AddItem(active_contact_item_));

    const ot::identity::wot::claim::Section copiedContactSection(section1);
    ASSERT_EQ(section1.Type(), copiedContactSection.Type());
    ASSERT_EQ(section1.Version(), copiedContactSection.Version());
    ASSERT_EQ(copiedContactSection.Size(), 1);
    ASSERT_NE(
        nullptr,
        copiedContactSection.Group(
            ot::identity::wot::claim::ClaimType::Employee));
    ASSERT_EQ(
        copiedContactSection
            .Group(ot::identity::wot::claim::ClaimType::Employee)
            ->Size(),
        1);
    ASSERT_NE(copiedContactSection.end(), copiedContactSection.begin());
}

TEST_F(ContactSection, operator_plus)
{
    // Combine two sections with one item each of the same type.
    const auto& section1 = contact_section_.AddItem(active_contact_item_);
    const auto contactItem2 = std::make_shared<ot::identity::wot::claim::Item>(
        claim_to_contact_item(client_1_.Factory().Claim(
            nym_id_,
            ot::identity::wot::claim::SectionType::Identifier,
            ot::identity::wot::claim::ClaimType::Employee,
            "activeContactItemValue2",
            active_attr_)));
    const ot::identity::wot::claim::Section section2(
        client_1_,
        "testContactSectionNym2",
        opentxs::identity::wot::claim::DefaultVersion(),
        opentxs::identity::wot::claim::DefaultVersion(),
        ot::identity::wot::claim::SectionType::Identifier,
        contactItem2);

    const auto& section3 = section1 + section2;
    // Verify the section has one group.
    ASSERT_EQ(section3.Size(), 1);
    // Verify the group has two items.
    ASSERT_EQ(
        section3.Group(ot::identity::wot::claim::ClaimType::Employee)->Size(),
        2);

    // Add a section that has one group with one item of the same type, and
    // another group with one item of a different type.
    const auto contactItem3 = std::make_shared<ot::identity::wot::claim::Item>(
        claim_to_contact_item(client_1_.Factory().Claim(
            nym_id_,
            ot::identity::wot::claim::SectionType::Identifier,
            ot::identity::wot::claim::ClaimType::Employee,
            "activeContactItemValue3",
            active_attr_)));
    const auto contactItem4 = std::make_shared<ot::identity::wot::claim::Item>(
        claim_to_contact_item(client_1_.Factory().Claim(
            nym_id_,
            ot::identity::wot::claim::SectionType::Identifier,
            ot::identity::wot::claim::ClaimType::Ssl,
            "activeContactItemValue4",
            active_attr_)));
    const ot::identity::wot::claim::Section section4(
        client_1_,
        "testContactSectionNym4",
        opentxs::identity::wot::claim::DefaultVersion(),
        opentxs::identity::wot::claim::DefaultVersion(),
        ot::identity::wot::claim::SectionType::Identifier,
        contactItem3);
    const auto& section5 = section4.AddItem(contactItem4);

    const auto& section6 = section3 + section5;
    // Verify the section has two groups.
    ASSERT_EQ(section6.Size(), 2);
    // Verify the first group has three items.
    ASSERT_EQ(
        section6.Group(ot::identity::wot::claim::ClaimType::Employee)->Size(),
        3);
    // Verify the second group has one item.
    ASSERT_EQ(
        section6.Group(ot::identity::wot::claim::ClaimType::Ssl)->Size(), 1);
}

TEST_F(ContactSection, operator_plus_different_versions)
{
    // rhs version less than lhs
    const ot::identity::wot::claim::Section section2(
        client_1_,
        "testContactSectionNym2",
        opentxs::identity::wot::claim::DefaultVersion() - 1,
        opentxs::identity::wot::claim::DefaultVersion() - 1,
        ot::identity::wot::claim::SectionType::Identifier,
        ot::identity::wot::claim::Section::GroupMap{});

    const auto& section3 = contact_section_ + section2;
    // Verify the new section has the latest version.
    ASSERT_EQ(
        opentxs::identity::wot::claim::DefaultVersion(), section3.Version());

    // lhs version less than rhs
    const auto& section4 = section2 + contact_section_;
    // Verify the new section has the latest version.
    ASSERT_EQ(
        opentxs::identity::wot::claim::DefaultVersion(), section4.Version());
}

TEST_F(ContactSection, AddItem)
{
    // Add an item to a SCOPE section.
    static constexpr auto attr = {ot::identity::wot::claim::Attribute::Local};
    const auto scopeContactItem =
        std::make_shared<ot::identity::wot::claim::Item>(
            claim_to_contact_item(client_1_.Factory().Claim(
                nym_id_,
                ot::identity::wot::claim::SectionType::Scope,
                ot::identity::wot::claim::ClaimType::Individual,
                "scopeContactItemValue",
                attr)));
    const ot::identity::wot::claim::Section section1(
        client_1_,
        "testContactSectionNym2",
        opentxs::identity::wot::claim::DefaultVersion(),
        opentxs::identity::wot::claim::DefaultVersion(),
        ot::identity::wot::claim::SectionType::Scope,
        ot::identity::wot::claim::Section::GroupMap{});
    const auto& section2 = section1.AddItem(scopeContactItem);
    ASSERT_EQ(section2.Size(), 1);
    ASSERT_EQ(
        section2.Group(ot::identity::wot::claim::ClaimType::Individual)->Size(),
        1);
    ASSERT_TRUE(
        section2.Claim(scopeContactItem->ID())
            ->HasAttribute(opentxs::identity::wot::claim::Attribute::Primary));
    ASSERT_TRUE(
        section2.Claim(scopeContactItem->ID())
            ->HasAttribute(opentxs::identity::wot::claim::Attribute::Active));

    // Add an item to a non-scope section.
    const auto& section4 = contact_section_.AddItem(active_contact_item_);
    ASSERT_EQ(section4.Size(), 1);
    ASSERT_EQ(
        section4.Group(ot::identity::wot::claim::ClaimType::Employee)->Size(),
        1);
    // Add a second item of the same type.
    const auto contactItem2 = std::make_shared<ot::identity::wot::claim::Item>(
        claim_to_contact_item(client_1_.Factory().Claim(
            nym_id_,
            ot::identity::wot::claim::SectionType::Identifier,
            ot::identity::wot::claim::ClaimType::Employee,
            "activeContactItemValue2",
            active_attr_)));
    const auto& section5 = section4.AddItem(contactItem2);
    // Verify there are two items.
    ASSERT_EQ(section5.Size(), 1);
    ASSERT_EQ(
        section5.Group(ot::identity::wot::claim::ClaimType::Employee)->Size(),
        2);

    // Add an item of a different type.
    const auto contactItem3 = std::make_shared<ot::identity::wot::claim::Item>(
        claim_to_contact_item(client_1_.Factory().Claim(
            nym_id_,
            ot::identity::wot::claim::SectionType::Identifier,
            ot::identity::wot::claim::ClaimType::Ssl,
            "activeContactItemValue3",
            active_attr_)));
    const auto& section6 = section5.AddItem(contactItem3);
    // Verify there are two groups.
    ASSERT_EQ(section6.Size(), 2);
    // Verify there is one in the second group.
    ASSERT_EQ(
        section6.Group(ot::identity::wot::claim::ClaimType::Ssl)->Size(), 1);
}

TEST_F(ContactSection, AddItem_different_versions)
{
    // Add an item with a newer version to a SCOPE section.
    const auto scopeContactItem =
        std::make_shared<ot::identity::wot::claim::Item>(
            claim_to_contact_item(client_1_.Factory().Claim(
                nym_id_,
                ot::identity::wot::claim::SectionType::Scope,
                ot::identity::wot::claim::ClaimType::Bot,
                "scopeContactItemValue",
                local_attr_)));
    const ot::identity::wot::claim::Section section1(
        client_1_,
        "testContactSectionNym2",
        3,  // version of CONTACTSECTION_SCOPE section before CITEMTYPE_BOT was
            // added
        3,
        ot::identity::wot::claim::SectionType::Scope,
        ot::identity::wot::claim::Section::GroupMap{});
    const auto& section2 = section1.AddItem(scopeContactItem);
    ASSERT_EQ(section2.Size(), 1);
    ASSERT_EQ(
        section2.Group(ot::identity::wot::claim::ClaimType::Bot)->Size(), 1);
    ASSERT_TRUE(
        section2.Claim(scopeContactItem->ID())
            ->HasAttribute(opentxs::identity::wot::claim::Attribute::Primary));
    ASSERT_TRUE(
        section2.Claim(scopeContactItem->ID())
            ->HasAttribute(opentxs::identity::wot::claim::Attribute::Active));
    // Verify the section version has been updated to the minimum version to
    // support CITEMTYPE_BOT.
    ASSERT_EQ(section2.Version(), 4);

    // Add an item with a newer version to a non-scope section.
    const auto contactItem2 = std::make_shared<ot::identity::wot::claim::Item>(
        claim_to_contact_item(client_1_.Factory().Claim(
            nym_id_,
            ot::identity::wot::claim::SectionType::Relationship,
            ot::identity::wot::claim::ClaimType::Owner,
            "contactItem2Value",
            local_attr_)));
    const ot::identity::wot::claim::Section section3(
        client_1_,
        "testContactSectionNym3",
        3,  // version of CONTACTSECTION_RELATIONSHIP section before
            // CITEMTYPE_OWNER was added
        3,
        ot::identity::wot::claim::SectionType::Relationship,
        ot::identity::wot::claim::Section::GroupMap{});
    const auto& section4 = section3.AddItem(contactItem2);
    ASSERT_EQ(section4.Size(), 1);
    ASSERT_EQ(
        section4.Group(ot::identity::wot::claim::ClaimType::Owner)->Size(), 1);
    // Verify the section version has been updated to the minimum version to
    // support CITEMTYPE_OWNER.
    ASSERT_EQ(4, section4.Version());
}

TEST_F(ContactSection, begin)
{
    auto it = contact_section_.begin();
    ASSERT_EQ(contact_section_.end(), it);
    ASSERT_EQ(std::distance(it, contact_section_.end()), 0);

    const auto& section1 = contact_section_.AddItem(active_contact_item_);
    it = section1.begin();
    ASSERT_NE(section1.end(), it);
    ASSERT_EQ(std::distance(it, section1.end()), 1);

    std::advance(it, 1);
    ASSERT_EQ(section1.end(), it);
    ASSERT_EQ(std::distance(it, section1.end()), 0);
}

TEST_F(ContactSection, Claim_found)
{
    const auto& section1 = contact_section_.AddItem(active_contact_item_);

    const std::shared_ptr<ot::identity::wot::claim::Item>& claim =
        section1.Claim(active_contact_item_->ID());
    ASSERT_NE(nullptr, claim);
    ASSERT_EQ(active_contact_item_->ID(), claim->ID());

    // Find a claim in a different group.
    const auto contactItem2 = std::make_shared<ot::identity::wot::claim::Item>(
        claim_to_contact_item(client_1_.Factory().Claim(
            nym_id_,
            ot::identity::wot::claim::SectionType::Identifier,
            ot::identity::wot::claim::ClaimType::Ssl,
            "activeContactItemValue2",
            active_attr_)));
    const auto& section2 = section1.AddItem(contactItem2);
    const std::shared_ptr<ot::identity::wot::claim::Item>& claim2 =
        section2.Claim(contactItem2->ID());
    ASSERT_NE(nullptr, claim2);
    ASSERT_EQ(contactItem2->ID(), claim2->ID());
}

TEST_F(ContactSection, Claim_notfound)
{
    const std::shared_ptr<ot::identity::wot::claim::Item>& claim =
        contact_section_.Claim(active_contact_item_->ID());
    ASSERT_FALSE(claim);
}

TEST_F(ContactSection, end)
{
    auto it = contact_section_.end();
    ASSERT_EQ(contact_section_.begin(), it);
    ASSERT_EQ(std::distance(contact_section_.begin(), it), 0);

    const auto& section1 = contact_section_.AddItem(active_contact_item_);
    it = section1.end();
    ASSERT_NE(section1.begin(), it);
    ASSERT_EQ(std::distance(section1.begin(), it), 1);

    std::advance(it, -1);
    ASSERT_EQ(section1.begin(), it);
    ASSERT_EQ(std::distance(section1.begin(), it), 0);
}

TEST_F(ContactSection, Group_found)
{
    const auto& section1 = contact_section_.AddItem(active_contact_item_);
    ASSERT_NE(
        nullptr, section1.Group(ot::identity::wot::claim::ClaimType::Employee));
}

TEST_F(ContactSection, Group_notfound)
{
    ASSERT_FALSE(
        contact_section_.Group(ot::identity::wot::claim::ClaimType::Employee));
}

TEST_F(ContactSection, HaveClaim_true)
{
    const auto& section1 = contact_section_.AddItem(active_contact_item_);

    ASSERT_TRUE(section1.HaveClaim(active_contact_item_->ID()));

    // Find a claim in a different group.
    const auto contactItem2 = std::make_shared<ot::identity::wot::claim::Item>(
        claim_to_contact_item(client_1_.Factory().Claim(
            nym_id_,
            ot::identity::wot::claim::SectionType::Identifier,
            ot::identity::wot::claim::ClaimType::Ssl,
            "activeContactItemValue2",
            active_attr_)));
    const auto& section2 = section1.AddItem(contactItem2);
    ASSERT_TRUE(section2.HaveClaim(contactItem2->ID()));
}

TEST_F(ContactSection, HaveClaim_false)
{
    ASSERT_FALSE(contact_section_.HaveClaim(active_contact_item_->ID()));
}

TEST_F(ContactSection, Delete)
{
    const auto& section1 = contact_section_.AddItem(active_contact_item_);
    ASSERT_TRUE(section1.HaveClaim(active_contact_item_->ID()));

    // Add a second item to help testing the size after trying to delete twice.
    const auto contactItem2 = std::make_shared<ot::identity::wot::claim::Item>(
        claim_to_contact_item(client_1_.Factory().Claim(
            nym_id_,
            ot::identity::wot::claim::SectionType::Identifier,
            ot::identity::wot::claim::ClaimType::Employee,
            "activeContactItemValue2",
            active_attr_)));
    const auto& section2 = section1.AddItem(contactItem2);
    ASSERT_EQ(
        section2.Group(ot::identity::wot::claim::ClaimType::Employee)->Size(),
        2);

    const auto& section3 = section2.Delete(active_contact_item_->ID());
    // Verify the item was deleted.
    ASSERT_FALSE(section3.HaveClaim(active_contact_item_->ID()));
    ASSERT_EQ(
        section3.Group(ot::identity::wot::claim::ClaimType::Employee)->Size(),
        1);

    const auto& section4 = section3.Delete(active_contact_item_->ID());
    // Verify trying to delete the item again didn't change anything.
    ASSERT_EQ(
        section4.Group(ot::identity::wot::claim::ClaimType::Employee)->Size(),
        1);

    // Add an item of a different type.
    const auto contactItem3 = std::make_shared<ot::identity::wot::claim::Item>(
        claim_to_contact_item(client_1_.Factory().Claim(
            nym_id_,
            ot::identity::wot::claim::SectionType::Identifier,
            ot::identity::wot::claim::ClaimType::Ssl,
            "activeContactItemValue3",
            active_attr_)));
    const auto& section5 = section4.AddItem(contactItem3);
    // Verify the section has two groups.
    ASSERT_EQ(section5.Size(), 2);
    ASSERT_EQ(
        section5.Group(ot::identity::wot::claim::ClaimType::Ssl)->Size(), 1);
    ASSERT_TRUE(section5.HaveClaim(contactItem3->ID()));

    const auto& section6 = section5.Delete(contactItem3->ID());
    // Verify the item was deleted and the group was removed.
    ASSERT_EQ(section6.Size(), 1);
    ASSERT_FALSE(section6.HaveClaim(contactItem3->ID()));
    ASSERT_FALSE(section6.Group(ot::identity::wot::claim::ClaimType::Ssl));
}

TEST_F(ContactSection, SerializeTo)
{
    // Serialize without ids.
    const auto& section1 = contact_section_.AddItem(active_contact_item_);
    auto bytes = ot::Space{};
    ASSERT_TRUE(section1.Serialize(ot::writer(bytes), false));

    auto restored1 = ot::identity::wot::claim::Section{
        client_1_, "ContactDataNym1", section1.Version(), ot::reader(bytes)};

    ASSERT_EQ(restored1.Size(), section1.Size());
    ASSERT_EQ(restored1.Type(), section1.Type());
    auto group_iterator = restored1.begin();
    ASSERT_EQ(
        group_iterator->first, ot::identity::wot::claim::ClaimType::Employee);
    auto group1 = group_iterator->second;
    ASSERT_TRUE(group1);
    auto item_iterator = group1->begin();
    auto contact_item = item_iterator->second;
    ASSERT_TRUE(contact_item);
    ASSERT_EQ(active_contact_item_->Value(), contact_item->Value());
    ASSERT_EQ(active_contact_item_->Version(), contact_item->Version());
    ASSERT_EQ(active_contact_item_->Type(), contact_item->Type());
    ASSERT_EQ(active_contact_item_->Start(), contact_item->Start());
    ASSERT_EQ(active_contact_item_->End(), contact_item->End());

    //    // Serialize with ids.
    auto restored2 = ot::identity::wot::claim::Section{
        client_1_, "ContactDataNym1", section1.Version(), ot::reader(bytes)};

    ASSERT_EQ(restored2.Size(), section1.Size());
    ASSERT_EQ(restored2.Type(), section1.Type());
    group_iterator = restored2.begin();
    ASSERT_EQ(
        group_iterator->first, ot::identity::wot::claim::ClaimType::Employee);
    group1 = group_iterator->second;
    ASSERT_TRUE(group1);
    item_iterator = group1->begin();
    contact_item = item_iterator->second;
    ASSERT_TRUE(contact_item);
    ASSERT_EQ(active_contact_item_->Value(), contact_item->Value());
    ASSERT_EQ(active_contact_item_->Version(), contact_item->Version());
    ASSERT_EQ(active_contact_item_->Type(), contact_item->Type());
    ASSERT_EQ(active_contact_item_->Start(), contact_item->Start());
    ASSERT_EQ(active_contact_item_->End(), contact_item->End());
}

TEST_F(ContactSection, Size)
{
    ASSERT_EQ(contact_section_.Size(), 0);
    const auto& section1 = contact_section_.AddItem(active_contact_item_);
    ASSERT_EQ(section1.Size(), 1);

    // Add a second item of the same type.
    const auto contactItem2 = std::make_shared<ot::identity::wot::claim::Item>(
        claim_to_contact_item(client_1_.Factory().Claim(
            nym_id_,
            ot::identity::wot::claim::SectionType::Identifier,
            ot::identity::wot::claim::ClaimType::Employee,
            "activeContactItemValue2",
            active_attr_)));
    const auto& section2 = section1.AddItem(contactItem2);
    // Verify the size is the same.
    ASSERT_EQ(section2.Size(), 1);

    // Add an item of a different type.
    const auto contactItem3 = std::make_shared<ot::identity::wot::claim::Item>(
        claim_to_contact_item(client_1_.Factory().Claim(
            nym_id_,
            ot::identity::wot::claim::SectionType::Identifier,
            ot::identity::wot::claim::ClaimType::Ssl,
            "activeContactItemValue3",
            active_attr_)));
    const auto& section3 = section2.AddItem(contactItem3);
    // Verify the size is now two.
    ASSERT_EQ(section3.Size(), 2);

    // Delete an item from the first group.
    const auto& section4 = section3.Delete(contactItem2->ID());
    // Verify the size is still two.
    ASSERT_EQ(section4.Size(), 2);

    // Delete the item from the second group.
    const auto& section5 = section4.Delete(contactItem3->ID());
    // Verify that the size is now one.
    ASSERT_EQ(section5.Size(), 1);
}

TEST_F(ContactSection, Type)
{
    ASSERT_EQ(
        ot::identity::wot::claim::SectionType::Identifier,
        contact_section_.Type());
}

TEST_F(ContactSection, Version)
{
    ASSERT_EQ(
        opentxs::identity::wot::claim::DefaultVersion(),
        contact_section_.Version());
}
}  // namespace ottest
