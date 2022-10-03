// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <iterator>
#include <memory>
#include <utility>

#include "internal/identity/wot/claim/Types.hpp"

namespace ot = opentxs;

namespace ottest
{
class Test_ContactGroup : public ::testing::Test
{
public:
    Test_ContactGroup()
        : api_(ot::Context().StartClientSession(0))
        , contact_group_(
              ot::UnallocatedCString("testContactGroupNym1"),
              ot::identity::wot::claim::SectionType::Identifier,
              ot::identity::wot::claim::ClaimType::Employee,
              {})
        , primary_(new ot::identity::wot::claim::Item(
              dynamic_cast<const ot::api::session::Client&>(api_),
              ot::UnallocatedCString("primaryContactItem"),
              opentxs::CONTACT_CONTACT_DATA_VERSION,
              opentxs::CONTACT_CONTACT_DATA_VERSION,
              ot::identity::wot::claim::SectionType::Identifier,
              ot::identity::wot::claim::ClaimType::Employee,
              ot::UnallocatedCString("primaryContactItemValue"),
              {ot::identity::wot::claim::Attribute::Primary},
              {},
              {},
              ""))
        , active_(new ot::identity::wot::claim::Item(
              dynamic_cast<const ot::api::session::Client&>(api_),
              ot::UnallocatedCString("activeContactItem"),
              opentxs::CONTACT_CONTACT_DATA_VERSION,
              opentxs::CONTACT_CONTACT_DATA_VERSION,
              ot::identity::wot::claim::SectionType::Identifier,
              ot::identity::wot::claim::ClaimType::Employee,
              ot::UnallocatedCString("activeContactItemValue"),
              {ot::identity::wot::claim::Attribute::Active},
              {},
              {},
              ""))
    {
    }

    const ot::api::session::Client& api_;
    const ot::identity::wot::claim::Group contact_group_;
    const std::shared_ptr<ot::identity::wot::claim::Item> primary_;
    const std::shared_ptr<ot::identity::wot::claim::Item> active_;
};

TEST_F(Test_ContactGroup, first_constructor)
{
    // Test constructing a group with a map containing two primary items.
    const std::shared_ptr<ot::identity::wot::claim::Item> primary2(
        new ot::identity::wot::claim::Item(
            dynamic_cast<const ot::api::session::Client&>(api_),
            ot::UnallocatedCString("primaryContactItemNym2"),
            opentxs::CONTACT_CONTACT_DATA_VERSION,
            opentxs::CONTACT_CONTACT_DATA_VERSION,
            ot::identity::wot::claim::SectionType::Identifier,
            ot::identity::wot::claim::ClaimType::Employee,
            ot::UnallocatedCString("primaryContactItemValue2"),
            {ot::identity::wot::claim::Attribute::Primary},
            {},
            {},
            ""));

    ot::identity::wot::claim::Group::ItemMap map;
    map[primary_->ID()] = primary_;
    map[primary2->ID()] = primary2;

    const ot::identity::wot::claim::Group group1(
        ot::UnallocatedCString("testContactGroupNym1"),
        ot::identity::wot::claim::SectionType::Identifier,
        ot::identity::wot::claim::ClaimType::Employee,
        map);
    // Verify two items were added.
    ASSERT_EQ(2, group1.Size());
    ASSERT_EQ(ot::identity::wot::claim::ClaimType::Employee, group1.Type());
    // Verify only one item is primary.
    if (primary_->ID() == group1.Primary()) {
        ASSERT_TRUE(group1.Claim(primary_->ID())->isPrimary());
        ASSERT_FALSE(group1.Claim(primary2->ID())->isPrimary());
    } else {
        ASSERT_FALSE(group1.Claim(primary_->ID())->isPrimary());
        ASSERT_TRUE(group1.Claim(primary2->ID())->isPrimary());
    }
}

TEST_F(Test_ContactGroup, first_constructor_no_items)
{
    // Test constructing a group with a map containing no items.
    const ot::identity::wot::claim::Group group1(
        ot::UnallocatedCString("testContactGroupNym1"),
        ot::identity::wot::claim::SectionType::Identifier,
        ot::identity::wot::claim::ClaimType::Employee,
        {});
    // Verify the private static methods didn't blow up.
    ASSERT_EQ(group1.Size(), 0);
    ASSERT_EQ(ot::identity::wot::claim::ClaimType::Employee, group1.Type());
}

TEST_F(Test_ContactGroup, second_constructor)
{
    const ot::identity::wot::claim::Group group1(
        ot::UnallocatedCString("testContactGroupNym1"),
        ot::identity::wot::claim::SectionType::Identifier,
        active_);

    ASSERT_EQ(1, group1.Size());
    // Verify the group type matches the type of the item.
    ASSERT_EQ(active_->Type(), group1.Type());
    ASSERT_EQ(active_->ID(), group1.begin()->second->ID());
}

TEST_F(Test_ContactGroup, copy_constructor)
{
    const ot::identity::wot::claim::Group group1(
        ot::UnallocatedCString("testContactGroupNym1"),
        ot::identity::wot::claim::SectionType::Identifier,
        active_);

    ot::identity::wot::claim::Group copiedContactGroup(group1);

    ASSERT_EQ(1, copiedContactGroup.Size());
    // Verify the group type matches the type of the item.
    ASSERT_EQ(active_->Type(), copiedContactGroup.Type());
    ASSERT_EQ(active_->ID(), copiedContactGroup.begin()->second->ID());
}

TEST_F(Test_ContactGroup, move_constructor)
{
    ot::identity::wot::claim::Group movedContactGroup(
        contact_group_.AddItem(active_));

    ASSERT_EQ(1, movedContactGroup.Size());
    // Verify the group type matches the type of the item.
    ASSERT_EQ(active_->Type(), movedContactGroup.Type());
    ASSERT_EQ(active_->ID(), movedContactGroup.begin()->second->ID());
}

TEST_F(Test_ContactGroup, operator_plus)
{
    // Test adding a group with a primary (rhs) to a group without a primary.
    const auto& group1 = contact_group_.AddItem(active_);
    const auto& group2 = contact_group_.AddItem(primary_);
    const auto& group3 = group1 + group2;
    ASSERT_EQ(2, group3.Size());
    // Verify that the primary for the new group comes from rhs.
    ASSERT_EQ(group2.Primary(), group3.Primary());

    // Test adding a group with 2 items to a group with 1 item.
    const std::shared_ptr<ot::identity::wot::claim::Item> primary2(
        new ot::identity::wot::claim::Item(
            dynamic_cast<const ot::api::session::Client&>(api_),
            ot::UnallocatedCString("primaryContactItemNym2"),
            opentxs::CONTACT_CONTACT_DATA_VERSION,
            opentxs::CONTACT_CONTACT_DATA_VERSION,
            ot::identity::wot::claim::SectionType::Identifier,
            ot::identity::wot::claim::ClaimType::Employee,
            ot::UnallocatedCString("primaryContactItemValue2"),
            {ot::identity::wot::claim::Attribute::Primary},
            {},
            {},
            ""));
    const auto& group4 = contact_group_.AddItem(primary2);
    const auto& group5 = contact_group_.AddItem(primary_);
    const auto& group6 = group5.AddItem(active_);

    const auto& group7 = group4 + group6;
    // Verify that the group has 3 items.
    ASSERT_EQ(3, group7.Size());
    // Verify that the primary of the new group came from the lhs.
    ASSERT_EQ(group4.Primary(), group7.Primary());

    for (auto it(group7.begin()); it != group7.end(); ++it) {
        if (it->second->ID() == primary_->ID()) {
            // Verify that the item that was primary on the rhs doesn't have
            // the primary attribute.
            ASSERT_FALSE(it->second->isPrimary());
        }
    }
}

TEST_F(Test_ContactGroup, AddItem)
{
    const auto& group1 = contact_group_.AddItem(active_);
    ASSERT_EQ(1, group1.Size());
    ASSERT_EQ(active_->ID(), group1.begin()->second->ID());

    // Test whether AddItem handles items that have already been added.
    const auto& group2 = group1.AddItem(active_);
    // Verify that there is still only one item.
    ASSERT_EQ(1, group2.Size());

    // Test that AddItem handles adding a primary.
    const auto& group3 = contact_group_.AddItem(primary_);
    ASSERT_EQ(1, group3.Size());
    ASSERT_EQ(primary_->ID(), group3.Primary());
    ASSERT_TRUE(group3.begin()->second->isPrimary());
}

TEST_F(Test_ContactGroup, AddPrimary)
{
    // Test that AddPrimary ignores a null pointer.
    const auto& group1 = contact_group_.AddPrimary(nullptr);
    ASSERT_TRUE(group1.Primary().empty());

    // Test that AddPrimary sets the primary attribute on an active item.
    const auto& group2 = contact_group_.AddPrimary(active_);
    // Verify that primary is set to the item id.
    ASSERT_EQ(active_->ID(), group2.Primary());
    // Verify that the item has the primary attribute.
    ASSERT_TRUE(group2.begin()->second->isPrimary());

    // Test adding a primary when the group already has one.
    const auto& group3 = contact_group_.AddPrimary(primary_);
    const auto& group4 = group3.AddPrimary(active_);
    // Verify that primary is set to the new item id.
    ASSERT_EQ(active_->ID(), group4.Primary());

    for (auto it(group4.begin()); it != group4.end(); ++it) {
        if (it->second->ID() == primary_->ID()) {
            // Verify that the old primary item doesn't have the primary
            // attribute.
            ASSERT_FALSE(it->second->isPrimary());
        } else if (it->second->ID() == active_->ID()) {
            // Verify that the new primary item has the primary attribute.
            ASSERT_TRUE(it->second->isPrimary());
        }
    }
}

TEST_F(Test_ContactGroup, begin)
{
    auto it = contact_group_.begin();
    ASSERT_EQ(contact_group_.end(), it);
    ASSERT_EQ(std::distance(it, contact_group_.end()), 0);

    const auto& group1 = contact_group_.AddItem(active_);
    it = group1.begin();
    ASSERT_NE(group1.end(), it);
    ASSERT_EQ(1, std::distance(it, group1.end()));

    std::advance(it, 1);
    ASSERT_EQ(group1.end(), it);
    ASSERT_EQ(std::distance(it, group1.end()), 0);
}

TEST_F(Test_ContactGroup, Best_none) { ASSERT_FALSE(contact_group_.Best()); }

TEST_F(Test_ContactGroup, Best_primary)
{
    const auto& group1 = contact_group_.AddItem(primary_);

    const std::shared_ptr<ot::identity::wot::claim::Item>& best = group1.Best();
    ASSERT_NE(nullptr, best);
    ASSERT_EQ(primary_->ID(), best->ID());
}

TEST_F(Test_ContactGroup, Best_active_and_local)
{
    const std::shared_ptr<ot::identity::wot::claim::Item> local(
        new ot::identity::wot::claim::Item(
            dynamic_cast<const ot::api::session::Client&>(api_),
            ot::UnallocatedCString("localContactItemNym"),
            opentxs::CONTACT_CONTACT_DATA_VERSION,
            opentxs::CONTACT_CONTACT_DATA_VERSION,
            ot::identity::wot::claim::SectionType::Identifier,
            ot::identity::wot::claim::ClaimType::Employee,
            ot::UnallocatedCString("localContactItemValue"),
            {ot::identity::wot::claim::Attribute::Local},
            {},
            {},
            ""));
    const auto& group1 = contact_group_.AddItem(active_);
    const auto& group2 = group1.AddItem(local);

    const std::shared_ptr<ot::identity::wot::claim::Item>& best = group2.Best();
    // Verify the best item is the active one.
    ASSERT_NE(nullptr, best);
    ASSERT_EQ(active_->ID(), best->ID());
    ASSERT_TRUE(best->isActive());

    const auto& group3 = group2.Delete(active_->ID());
    const std::shared_ptr<ot::identity::wot::claim::Item>& best2 =
        group3.Best();
    // Verify the best item is the local one.
    ASSERT_NE(nullptr, best2);
    ASSERT_EQ(local->ID(), best2->ID());
    ASSERT_TRUE(best2->isLocal());
}

TEST_F(Test_ContactGroup, Claim_found)
{
    const auto& group1 = contact_group_.AddItem(active_);

    const std::shared_ptr<ot::identity::wot::claim::Item>& claim =
        group1.Claim(active_->ID());
    ASSERT_NE(nullptr, claim);
    ASSERT_EQ(active_->ID(), claim->ID());
}

TEST_F(Test_ContactGroup, Claim_notfound)
{
    const std::shared_ptr<ot::identity::wot::claim::Item>& claim =
        contact_group_.Claim(active_->ID());
    ASSERT_FALSE(claim);
}

TEST_F(Test_ContactGroup, end)
{
    auto it = contact_group_.end();
    ASSERT_EQ(contact_group_.begin(), it);
    ASSERT_EQ(std::distance(contact_group_.begin(), it), 0);

    const auto& group1 = contact_group_.AddItem(active_);
    it = group1.end();
    ASSERT_NE(group1.begin(), it);
    ASSERT_EQ(1, std::distance(group1.begin(), it));

    std::advance(it, -1);
    ASSERT_EQ(group1.begin(), it);
    ASSERT_EQ(std::distance(group1.begin(), it), 0);
}

TEST_F(Test_ContactGroup, HaveClaim_true)
{
    const auto& group1 = contact_group_.AddItem(active_);

    ASSERT_TRUE(group1.HaveClaim(active_->ID()));
}

TEST_F(Test_ContactGroup, HaveClaim_false)
{
    ASSERT_FALSE(contact_group_.HaveClaim(active_->ID()));
}

TEST_F(Test_ContactGroup, Delete)
{
    const auto& group1 = contact_group_.AddItem(active_);
    ASSERT_TRUE(group1.HaveClaim(active_->ID()));

    // Add a second item to help testing the size after trying to delete twice.
    const auto& group2 = group1.AddItem(primary_);
    ASSERT_EQ(2, group2.Size());

    const auto& group3 = group2.Delete(active_->ID());
    // Verify the item was deleted.
    ASSERT_FALSE(group3.HaveClaim(active_->ID()));
    ASSERT_EQ(1, group3.Size());

    const auto& group4 = group3.Delete(active_->ID());
    // Verify trying to delete the item again didn't change anything.
    ASSERT_EQ(1, group4.Size());
}

TEST_F(Test_ContactGroup, Primary_group_has_primary)
{
    const auto& group1 = contact_group_.AddItem(primary_);
    ASSERT_FALSE(group1.Primary().empty());
    ASSERT_EQ(primary_->ID(), group1.Primary());
}

TEST_F(Test_ContactGroup, Primary_no_primary)
{
    ASSERT_TRUE(contact_group_.Primary().empty());
}

TEST_F(Test_ContactGroup, PrimaryClaim_found)
{
    const auto& group1 = contact_group_.AddItem(primary_);

    const std::shared_ptr<ot::identity::wot::claim::Item>& primaryClaim =
        group1.PrimaryClaim();
    ASSERT_NE(nullptr, primaryClaim);
    ASSERT_EQ(primary_->ID(), primaryClaim->ID());
}

TEST_F(Test_ContactGroup, PrimaryClaim_notfound)
{
    const std::shared_ptr<ot::identity::wot::claim::Item>& primaryClaim =
        contact_group_.PrimaryClaim();
    ASSERT_FALSE(primaryClaim);
}

TEST_F(Test_ContactGroup, Size)
{
    ASSERT_EQ(contact_group_.Size(), 0);
    const auto& group1 = contact_group_.AddItem(primary_);
    ASSERT_EQ(1, group1.Size());
    const auto& group2 = group1.AddItem(active_);
    ASSERT_EQ(2, group2.Size());
    const auto& group3 = group2.Delete(active_->ID());
    ASSERT_EQ(1, group3.Size());
}

TEST_F(Test_ContactGroup, Type)
{
    ASSERT_EQ(
        ot::identity::wot::claim::ClaimType::Employee, contact_group_.Type());
}
}  // namespace ottest
