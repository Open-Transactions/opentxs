// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <algorithm>
#include <functional>
#include <iterator>
#include <memory>
#include <ranges>
#include <span>
#include <utility>

#include "ottest/fixtures/contact/ContactGroup.hpp"
#include "ottest/fixtures/contact/ContactItem.hpp"

namespace ot = opentxs;

namespace ottest
{
TEST_F(ContactGroup, first_constructor)
{
    // Test constructing a group with a map containing two primary items.
    const auto primary2 = std::make_shared<ot::identity::wot::claim::Item>(
        claim_to_contact_item(client_1_.Factory().Claim(
            {nym_id_},
            ot::identity::wot::claim::SectionType::Identifier,
            ot::identity::wot::claim::ClaimType::Employee,
            "primaryContactItemValue2",
            primary_attr_)));
    auto map = ot::identity::wot::claim::Group::ItemMap{};
    map[primary_->ID()] = primary_;
    map[primary2->ID()] = primary2;
    const ot::identity::wot::claim::Group group1(
        ot::UnallocatedCString("testContactGroupNym1"),
        ot::identity::wot::claim::SectionType::Identifier,
        ot::identity::wot::claim::ClaimType::Employee,
        map);

    // Verify two items were added.
    EXPECT_EQ(2, group1.Size());
    EXPECT_EQ(ot::identity::wot::claim::ClaimType::Employee, group1.Type());

    // Verify only one item is primary.
    constexpr auto is_primary = [](const auto& item) {
        return item->HasAttribute(
            opentxs::identity::wot::claim::Attribute::Primary);
    };
    const auto count =
        std::ranges::count_if(group1 | std::views::values, is_primary);

    EXPECT_EQ(count, 1);
}

TEST_F(ContactGroup, first_constructor_no_items)
{
    // Test constructing a group with a map containing no items.
    const ot::identity::wot::claim::Group group1(
        ot::UnallocatedCString("testContactGroupNym1"),
        ot::identity::wot::claim::SectionType::Identifier,
        ot::identity::wot::claim::ClaimType::Employee,
        {});
    // Verify the private static methods didn't blow up.
    EXPECT_EQ(group1.Size(), 0);
    EXPECT_EQ(ot::identity::wot::claim::ClaimType::Employee, group1.Type());
}

TEST_F(ContactGroup, second_constructor)
{
    const ot::identity::wot::claim::Group group1(
        ot::UnallocatedCString("testContactGroupNym1"),
        ot::identity::wot::claim::SectionType::Identifier,
        active_);

    EXPECT_EQ(1, group1.Size());
    // Verify the group type matches the type of the item.
    EXPECT_EQ(active_->Type(), group1.Type());
    EXPECT_EQ(active_->ID(), group1.begin()->second->ID());
}

TEST_F(ContactGroup, copy_constructor)
{
    const ot::identity::wot::claim::Group group1(
        ot::UnallocatedCString("testContactGroupNym1"),
        ot::identity::wot::claim::SectionType::Identifier,
        active_);

    ot::identity::wot::claim::Group copiedContactGroup(group1);

    EXPECT_EQ(1, copiedContactGroup.Size());
    // Verify the group type matches the type of the item.
    EXPECT_EQ(active_->Type(), copiedContactGroup.Type());
    EXPECT_EQ(active_->ID(), copiedContactGroup.begin()->second->ID());
}

TEST_F(ContactGroup, move_constructor)
{
    ot::identity::wot::claim::Group movedContactGroup(
        contact_group_.AddItem(active_));

    EXPECT_EQ(1, movedContactGroup.Size());
    // Verify the group type matches the type of the item.
    EXPECT_EQ(active_->Type(), movedContactGroup.Type());
    EXPECT_EQ(active_->ID(), movedContactGroup.begin()->second->ID());
}

TEST_F(ContactGroup, operator_plus)
{
    // Test adding a group with a primary (rhs) to a group without a primary.
    const auto& group1 = contact_group_.AddItem(active_);
    const auto& group2 = contact_group_.AddItem(primary_);
    const auto& group3 = group1 + group2;
    EXPECT_EQ(2, group3.Size());
    // Verify that the primary for the new group comes from rhs.
    EXPECT_EQ(group2.Primary(), group3.Primary());

    // Test adding a group with 2 items to a group with 1 item.
    const auto primary2 = std::make_shared<ot::identity::wot::claim::Item>(
        claim_to_contact_item(client_1_.Factory().Claim(
            nym_id_,
            ot::identity::wot::claim::SectionType::Identifier,
            ot::identity::wot::claim::ClaimType::Employee,
            "primaryContactItemNym2",
            primary_attr_)));
    const auto& group4 = contact_group_.AddItem(primary2);
    const auto& group5 = contact_group_.AddItem(primary_);
    const auto& group6 = group5.AddItem(active_);
    const auto& group7 = group4 + group6;

    // Verify that the group has 3 items.
    EXPECT_EQ(3, group7.Size());
    // Verify that the primary of the new group came from the lhs.
    EXPECT_EQ(group4.Primary(), group7.Primary());

    for (auto it(group7.begin()); it != group7.end(); ++it) {
        if (it->second->ID() == primary_->ID()) {
            // Verify that the item that was primary on the rhs doesn't have
            // the primary attribute.
            EXPECT_FALSE(it->second->HasAttribute(
                opentxs::identity::wot::claim::Attribute::Primary));
        }
    }
}

TEST_F(ContactGroup, AddItem)
{
    const auto& group1 = contact_group_.AddItem(active_);
    EXPECT_EQ(1, group1.Size());
    EXPECT_EQ(active_->ID(), group1.begin()->second->ID());

    // Test whether AddItem handles items that have already been added.
    const auto& group2 = group1.AddItem(active_);
    // Verify that there is still only one item.
    EXPECT_EQ(1, group2.Size());

    // Test that AddItem handles adding a primary.
    const auto& group3 = contact_group_.AddItem(primary_);
    EXPECT_EQ(1, group3.Size());
    EXPECT_EQ(primary_->ID(), group3.Primary());
    EXPECT_TRUE(group3.begin()->second->HasAttribute(
        opentxs::identity::wot::claim::Attribute::Primary));
}

TEST_F(ContactGroup, AddPrimary)
{
    // Test that AddPrimary ignores a null pointer.
    const auto& group1 = contact_group_.AddPrimary(nullptr);
    EXPECT_TRUE(group1.Primary().empty());

    // Test that AddPrimary sets the primary attribute on an active item.
    const auto& group2 = contact_group_.AddPrimary(active_);
    // Verify that primary is set to the item id.
    EXPECT_EQ(active_->ID(), group2.Primary());
    // Verify that the item has the primary attribute.
    EXPECT_TRUE(group2.begin()->second->HasAttribute(
        opentxs::identity::wot::claim::Attribute::Primary));

    // Test adding a primary when the group already has one.
    const auto& group3 = contact_group_.AddPrimary(primary_);
    const auto& group4 = group3.AddPrimary(active_);
    // Verify that primary is set to the new item id.
    EXPECT_EQ(active_->ID(), group4.Primary());

    for (auto it(group4.begin()); it != group4.end(); ++it) {
        if (it->second->ID() == primary_->ID()) {
            // Verify that the old primary item doesn't have the primary
            // attribute.
            EXPECT_FALSE(it->second->HasAttribute(
                opentxs::identity::wot::claim::Attribute::Primary));
        } else if (it->second->ID() == active_->ID()) {
            // Verify that the new primary item has the primary attribute.
            EXPECT_TRUE(it->second->HasAttribute(
                opentxs::identity::wot::claim::Attribute::Primary));
        }
    }
}

TEST_F(ContactGroup, begin)
{
    auto it = contact_group_.begin();
    EXPECT_EQ(contact_group_.end(), it);
    EXPECT_EQ(std::distance(it, contact_group_.end()), 0);

    const auto& group1 = contact_group_.AddItem(active_);
    it = group1.begin();
    EXPECT_NE(group1.end(), it);
    EXPECT_EQ(1, std::distance(it, group1.end()));

    std::advance(it, 1);
    EXPECT_EQ(group1.end(), it);
    EXPECT_EQ(std::distance(it, group1.end()), 0);
}

TEST_F(ContactGroup, Best_none) { EXPECT_FALSE(contact_group_.Best()); }

TEST_F(ContactGroup, Best_primary)
{
    const auto& group1 = contact_group_.AddItem(primary_);

    const std::shared_ptr<ot::identity::wot::claim::Item>& best = group1.Best();
    EXPECT_NE(nullptr, best);
    EXPECT_EQ(primary_->ID(), best->ID());
}

TEST_F(ContactGroup, Best_active_and_local)
{
    static constexpr auto attrib = {ot::identity::wot::claim::Attribute::Local};
    const auto local = std::make_shared<ot::identity::wot::claim::Item>(
        claim_to_contact_item(client_1_.Factory().Claim(
            nym_id_,
            ot::identity::wot::claim::SectionType::Identifier,
            ot::identity::wot::claim::ClaimType::Employee,
            "localContactItemValue",
            attrib)));
    const auto& group1 = contact_group_.AddItem(active_);
    const auto& group2 = group1.AddItem(local);

    const std::shared_ptr<ot::identity::wot::claim::Item>& best = group2.Best();
    // Verify the best item is the active one.
    EXPECT_NE(nullptr, best);
    EXPECT_EQ(active_->ID(), best->ID());
    EXPECT_TRUE(
        best->HasAttribute(opentxs::identity::wot::claim::Attribute::Active));

    const auto& group3 = group2.Delete(active_->ID());
    const std::shared_ptr<ot::identity::wot::claim::Item>& best2 =
        group3.Best();
    // Verify the best item is the local one.
    EXPECT_NE(nullptr, best2);
    EXPECT_EQ(local->ID(), best2->ID());
    EXPECT_TRUE(
        best2->HasAttribute(opentxs::identity::wot::claim::Attribute::Local));
}

TEST_F(ContactGroup, Claim_found)
{
    const auto& group1 = contact_group_.AddItem(active_);

    const std::shared_ptr<ot::identity::wot::claim::Item>& claim =
        group1.Claim(active_->ID());
    EXPECT_NE(nullptr, claim);
    EXPECT_EQ(active_->ID(), claim->ID());
}

TEST_F(ContactGroup, Claim_notfound)
{
    const std::shared_ptr<ot::identity::wot::claim::Item>& claim =
        contact_group_.Claim(active_->ID());
    EXPECT_FALSE(claim);
}

TEST_F(ContactGroup, end)
{
    auto it = contact_group_.end();
    EXPECT_EQ(contact_group_.begin(), it);
    EXPECT_EQ(std::distance(contact_group_.begin(), it), 0);

    const auto& group1 = contact_group_.AddItem(active_);
    it = group1.end();
    EXPECT_NE(group1.begin(), it);
    EXPECT_EQ(1, std::distance(group1.begin(), it));

    std::advance(it, -1);
    EXPECT_EQ(group1.begin(), it);
    EXPECT_EQ(std::distance(group1.begin(), it), 0);
}

TEST_F(ContactGroup, HaveClaim_true)
{
    const auto& group1 = contact_group_.AddItem(active_);

    EXPECT_TRUE(group1.HaveClaim(active_->ID()));
}

TEST_F(ContactGroup, HaveClaim_false)
{
    EXPECT_FALSE(contact_group_.HaveClaim(active_->ID()));
}

TEST_F(ContactGroup, Delete)
{
    const auto& group1 = contact_group_.AddItem(active_);
    EXPECT_TRUE(group1.HaveClaim(active_->ID()));

    // Add a second item to help testing the size after trying to delete twice.
    const auto& group2 = group1.AddItem(primary_);
    EXPECT_EQ(2, group2.Size());

    const auto& group3 = group2.Delete(active_->ID());
    // Verify the item was deleted.
    EXPECT_FALSE(group3.HaveClaim(active_->ID()));
    EXPECT_EQ(1, group3.Size());

    const auto& group4 = group3.Delete(active_->ID());
    // Verify trying to delete the item again didn't change anything.
    EXPECT_EQ(1, group4.Size());
}

TEST_F(ContactGroup, Primary_group_has_primary)
{
    const auto& group1 = contact_group_.AddItem(primary_);
    EXPECT_FALSE(group1.Primary().empty());
    EXPECT_EQ(primary_->ID(), group1.Primary());
}

TEST_F(ContactGroup, Primary_no_primary)
{
    EXPECT_TRUE(contact_group_.Primary().empty());
}

TEST_F(ContactGroup, PrimaryClaim_found)
{
    const auto& group1 = contact_group_.AddItem(primary_);

    const std::shared_ptr<ot::identity::wot::claim::Item>& primaryClaim =
        group1.PrimaryClaim();
    EXPECT_NE(nullptr, primaryClaim);
    EXPECT_EQ(primary_->ID(), primaryClaim->ID());
}

TEST_F(ContactGroup, PrimaryClaim_notfound)
{
    const std::shared_ptr<ot::identity::wot::claim::Item>& primaryClaim =
        contact_group_.PrimaryClaim();
    EXPECT_FALSE(primaryClaim);
}

TEST_F(ContactGroup, Size)
{
    EXPECT_EQ(contact_group_.Size(), 0);
    const auto& group1 = contact_group_.AddItem(primary_);
    EXPECT_EQ(1, group1.Size());
    const auto& group2 = group1.AddItem(active_);
    EXPECT_EQ(2, group2.Size());
    const auto& group3 = group2.Delete(active_->ID());
    EXPECT_EQ(1, group3.Size());
}

TEST_F(ContactGroup, Type)
{
    EXPECT_EQ(
        ot::identity::wot::claim::ClaimType::Employee, contact_group_.Type());
}
}  // namespace ottest
