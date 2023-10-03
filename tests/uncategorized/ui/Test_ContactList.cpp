// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <atomic>
#include <memory>

#include "ottest/fixtures/common/Counter.hpp"
#include "ottest/fixtures/common/User.hpp"
#include "ottest/fixtures/ui/ContactList.hpp"

namespace ottest
{
Counter counter_{1, 0};

TEST_F(ContactList, initialize_opentxs)
{
    init_contact_list(alice_, counter_);

    ASSERT_TRUE(bob_payment_code_.Valid());
    ASSERT_TRUE(chris_payment_code_.Valid());
}

TEST_F(ContactList, initial_state)
{
    ASSERT_TRUE(wait_for_counter(counter_));

    const auto expected = ContactListData{{
        {true, alice_.name_, alice_.name_, "ME", ""},
    }};

    ASSERT_TRUE(wait_for_counter(counter_));
    EXPECT_TRUE(check_contact_list(alice_, expected));
    EXPECT_TRUE(check_contact_list_qt(alice_, expected));
}

TEST_F(ContactList, add_chris)
{
    counter_.expected_ += 1;
    const auto chris = api_.Contacts().NewContact(
        chris_, chris_payment_code_.ID(), chris_payment_code_);

    ASSERT_TRUE(chris);

    alice_.SetContact(chris_, chris->ID());
}

TEST_F(ContactList, add_chris_state)
{
    ASSERT_TRUE(wait_for_counter(counter_));

    const auto expected = ContactListData{{
        {true, alice_.name_, alice_.name_, "ME", ""},
        {true, chris_, chris_, "C", ""},
    }};

    ASSERT_TRUE(wait_for_counter(counter_));
    EXPECT_TRUE(check_contact_list(alice_, expected));
    EXPECT_TRUE(check_contact_list_qt(alice_, expected));
}

TEST_F(ContactList, add_bob)
{
    counter_.expected_ += 1;
    const auto bob = api_.Contacts().NewContact(
        bob_, bob_payment_code_.ID(), bob_payment_code_);

    ASSERT_TRUE(bob);

    alice_.SetContact(bob_, bob->ID());
}

TEST_F(ContactList, add_bob_state)
{
    ASSERT_TRUE(wait_for_counter(counter_));

    const auto expected = ContactListData{{
        {true, alice_.name_, alice_.name_, "ME", ""},
        {true, bob_, bob_, "B", ""},
        {true, chris_, chris_, "C", ""},
    }};

    ASSERT_TRUE(wait_for_counter(counter_));
    EXPECT_TRUE(check_contact_list(alice_, expected));
    EXPECT_TRUE(check_contact_list_qt(alice_, expected));
}

TEST_F(ContactList, add_contact_payment_code)
{
    counter_.expected_ += 1;
    const auto expected = ContactListData{{
        {true, alice_.name_, alice_.name_, "ME", ""},
        {true, bob_, bob_, "B", ""},
        {true, chris_, chris_, "C", ""},
        {false, daniel_, daniel_, "D", ""},
    }};
    const auto id =
        contact_list_add_contact(alice_, "Daniel", payment_code_3_, "");

    EXPECT_FALSE(id.empty());
    ASSERT_TRUE(wait_for_counter(counter_));
    EXPECT_TRUE(check_contact_list(alice_, expected));
    // TODO EXPECT_TRUE(check_contact_list_qt(alice_, expected));
}

TEST_F(ContactList, change_contact_name)
{
    counter_.expected_ += 1;
    const auto expected = ContactListData{{
        {true, alice_.name_, alice_.name_, "ME", ""},
        {true, chris_, chris_, "C", ""},
        {true, daniel_, daniel_, "D", ""},
        {true, bob_, "Robert", "R", ""},
    }};

    const auto& contact = alice_.Contact(bob_);
    auto renamed = contact_list_rename_contact(
        alice_, contact.asBase58(api_.Crypto()), "Robert");

    ASSERT_TRUE(renamed);
    ASSERT_TRUE(wait_for_counter(counter_));
    EXPECT_TRUE(check_contact_list(alice_, expected));

    renamed = contact_list_rename_contact(alice_, "notacontactid", "noname");
    ASSERT_FALSE(renamed);
    EXPECT_TRUE(check_contact_list(alice_, expected));
}

TEST_F(ContactList, shutdown)
{
    EXPECT_EQ(counter_.expected_, counter_.updated_);
}
}  // namespace ottest
