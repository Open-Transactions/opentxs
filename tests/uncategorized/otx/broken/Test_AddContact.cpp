// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <atomic>
#include <future>
#include <utility>

#include "internal/api/session/UI.hpp"
#include "internal/interface/ui/ContactList.hpp"
#include "ottest/env/OTTestEnvironment.hpp"
#include "ottest/fixtures/common/Counter.hpp"
#include "ottest/fixtures/common/User.hpp"
#include "ottest/fixtures/integration/Helpers.hpp"
#include "ottest/fixtures/ui/ContactList.hpp"

namespace ottest
{
Counter contact_list_alex_{};
Counter messagable_list_alex_{};
Counter contact_list_bob_{};
Counter messagable_list_bob_{};
Counter contact_list_chris_{};
Counter messagable_list_chris_{};

struct Test_AddContact : public IntegrationFixture {
    static const bool have_hd_;

    const ot::api::session::Client& api_alex_;
    const ot::api::session::Client& api_bob_;
    const ot::api::session::Client& api_chris_;
    const ot::api::session::Notary& api_server_1_;

    Test_AddContact()
        : api_alex_(OTTestEnvironment::GetOT().StartClientSession(0))
        , api_bob_(OTTestEnvironment::GetOT().StartClientSession(1))
        , api_chris_(OTTestEnvironment::GetOT().StartClientSession(2))
        , api_server_1_(OTTestEnvironment::GetOT().StartNotarySession(0))
    {
        const_cast<Server&>(server_1_).init(api_server_1_);
        const_cast<User&>(alex_).init(api_alex_, server_1_);
        const_cast<User&>(bob_).init(api_bob_, server_1_);
        const_cast<User&>(chris_).init(api_chris_, server_1_);
    }
};

const bool Test_AddContact::have_hd_{
    ot::api::crypto::HaveHDKeys() &&
    ot::api::crypto::HaveSupport(ot::crypto::asymmetric::Algorithm::Secp256k1)

};

TEST_F(Test_AddContact, init_ot) {}

TEST_F(Test_AddContact, init_ui)
{
    contact_list_alex_.expected_ = 1;
    contact_list_bob_.expected_ = 1;
    contact_list_chris_.expected_ = 1;
    messagable_list_alex_.expected_ = 0;
    messagable_list_bob_.expected_ = 0;
    messagable_list_chris_.expected_ = 0;

    init_contact_list(alex_, contact_list_alex_);
    init_contact_list(bob_, contact_list_bob_);
    init_contact_list(chris_, contact_list_chris_);
    init_messagable_list(alex_, messagable_list_alex_);
    init_messagable_list(bob_, messagable_list_bob_);
    init_messagable_list(chris_, messagable_list_chris_);
}

TEST_F(Test_AddContact, initial_state_contact_list_alex)
{
    ASSERT_TRUE(wait_for_counter(contact_list_alex_));

    const auto expected = ContactListData{{
        {true, alex_.name_, alex_.name_, "ME", ""},
    }};

    ASSERT_TRUE(wait_for_counter(contact_list_alex_));
    EXPECT_TRUE(check_contact_list(alex_, expected));
    EXPECT_TRUE(check_contact_list_qt(alex_, expected));
}

TEST_F(Test_AddContact, initial_state_messagable_list_alex)
{
    ASSERT_TRUE(wait_for_counter(messagable_list_alex_));

    const auto expected = ContactListData{};

    ASSERT_TRUE(wait_for_counter(messagable_list_alex_));
    EXPECT_TRUE(check_messagable_list(alex_, expected));
    EXPECT_TRUE(check_messagable_list_qt(alex_, expected));
}

TEST_F(Test_AddContact, initial_state_contact_list_bob)
{
    ASSERT_TRUE(wait_for_counter(contact_list_bob_));

    const auto expected = ContactListData{{
        {true, bob_.name_, bob_.name_, "ME", ""},
    }};

    ASSERT_TRUE(wait_for_counter(contact_list_bob_));
    EXPECT_TRUE(check_contact_list(bob_, expected));
    EXPECT_TRUE(check_contact_list_qt(bob_, expected));
}

TEST_F(Test_AddContact, initial_state_messagable_list_bob)
{
    ASSERT_TRUE(wait_for_counter(messagable_list_bob_));

    const auto expected = ContactListData{};

    ASSERT_TRUE(wait_for_counter(messagable_list_bob_));
    EXPECT_TRUE(check_messagable_list(bob_, expected));
    EXPECT_TRUE(check_messagable_list_qt(bob_, expected));
}

TEST_F(Test_AddContact, initial_state_contact_list_chris)
{
    ASSERT_TRUE(wait_for_counter(contact_list_chris_));

    const auto expected = ContactListData{{
        {true, chris_.name_, chris_.name_, "ME", ""},
    }};

    ASSERT_TRUE(wait_for_counter(contact_list_chris_));
    EXPECT_TRUE(check_contact_list(chris_, expected));
    EXPECT_TRUE(check_contact_list_qt(chris_, expected));
}

TEST_F(Test_AddContact, initial_state_messagable_list_chris)
{
    ASSERT_TRUE(wait_for_counter(messagable_list_chris_));

    const auto expected = ContactListData{};

    ASSERT_TRUE(wait_for_counter(messagable_list_chris_));
    EXPECT_TRUE(check_messagable_list(chris_, expected));
    EXPECT_TRUE(check_messagable_list_qt(chris_, expected));
}

TEST_F(Test_AddContact, introduction_server)
{
    api_alex_.OTX().StartIntroductionServer(alex_.nym_id_);
    api_bob_.OTX().StartIntroductionServer(bob_.nym_id_);
    api_chris_.OTX().StartIntroductionServer(chris_.nym_id_);
    auto alexTask =
        api_alex_.OTX().RegisterNymPublic(alex_.nym_id_, server_1_.id_, true);
    auto bobTask =
        api_bob_.OTX().RegisterNymPublic(bob_.nym_id_, server_1_.id_, true);
    auto chrisTask =
        api_chris_.OTX().RegisterNymPublic(chris_.nym_id_, server_1_.id_, true);

    ASSERT_NE(0, alexTask.first);
    ASSERT_NE(0, bobTask.first);
    ASSERT_NE(0, chrisTask.first);
    EXPECT_EQ(
        ot::otx::LastReplyStatus::MessageSuccess, alexTask.second.get().first);
    EXPECT_EQ(
        ot::otx::LastReplyStatus::MessageSuccess, bobTask.second.get().first);
    EXPECT_EQ(
        ot::otx::LastReplyStatus::MessageSuccess, chrisTask.second.get().first);

    api_alex_.OTX().ContextIdle(alex_.nym_id_, server_1_.id_).get();
    api_bob_.OTX().ContextIdle(bob_.nym_id_, server_1_.id_).get();
    api_chris_.OTX().ContextIdle(chris_.nym_id_, server_1_.id_).get();
}

TEST_F(Test_AddContact, nymid)
{
    ASSERT_FALSE(bob_.nym_id_.empty());

    contact_list_alex_.expected_ += 1;
    messagable_list_alex_.expected_ += 1;
    const auto& widget = alex_.api_->UI().Internal().ContactList(alex_.nym_id_);
    const auto id = widget.AddContact(
        bob_.name_, bob_.nym_id_.asBase58(api_alex_.Crypto()), "");
    api_alex_.OTX().ContextIdle(alex_.nym_id_, server_1_.id_).get();

    ASSERT_FALSE(id.empty());
    EXPECT_TRUE(alex_.SetContact(bob_.name_, id));
}

TEST_F(Test_AddContact, add_nymid_contact_list_alex)
{
    ASSERT_TRUE(wait_for_counter(contact_list_alex_));

    const auto expected = ContactListData{{
        {true, alex_.name_, alex_.name_, "ME", ""},
        {true, bob_.name_, bob_.name_, "B", ""},
    }};

    ASSERT_TRUE(wait_for_counter(contact_list_alex_));
    EXPECT_TRUE(check_contact_list(alex_, expected));
    EXPECT_TRUE(check_contact_list_qt(alex_, expected));
}

TEST_F(Test_AddContact, add_nymid_messagable_list_alex)
{
    ASSERT_TRUE(wait_for_counter(messagable_list_alex_));

    const auto expected = ContactListData{{
        {true, bob_.name_, bob_.name_, "B", ""},
    }};

    ASSERT_TRUE(wait_for_counter(messagable_list_alex_));
    EXPECT_TRUE(check_messagable_list(alex_, expected));
    EXPECT_TRUE(check_messagable_list_qt(alex_, expected));
}

TEST_F(Test_AddContact, paymentcode)
{
    if (have_hd_) {
        ASSERT_FALSE(chris_.payment_code_.empty());

        contact_list_alex_.expected_ += 1;
        messagable_list_alex_.expected_ += 1;
        const auto& widget =
            alex_.api_->UI().Internal().ContactList(alex_.nym_id_);
        const auto id =
            widget.AddContact(chris_.name_, "", chris_.payment_code_);
        api_alex_.OTX().ContextIdle(alex_.nym_id_, server_1_.id_).get();

        ASSERT_FALSE(id.empty());
        EXPECT_TRUE(alex_.SetContact(chris_.name_, id));
    } else {
        // TODO
    }
}

TEST_F(Test_AddContact, add_paymentcode_contact_list_alex)
{
    if (have_hd_) {
        ASSERT_TRUE(wait_for_counter(contact_list_alex_));

        const auto expected = ContactListData{{
            {true, alex_.name_, alex_.name_, "ME", ""},
            {true, bob_.name_, bob_.name_, "B", ""},
            {true, chris_.name_, chris_.name_, "C", ""},
        }};

        ASSERT_TRUE(wait_for_counter(contact_list_alex_));
        EXPECT_TRUE(check_contact_list(alex_, expected));
        EXPECT_TRUE(check_contact_list_qt(alex_, expected));
    } else {
        // TODO
    }
}

TEST_F(Test_AddContact, add_paymentcode_messagable_list_alex)
{
    if (have_hd_) {
        ASSERT_TRUE(wait_for_counter(messagable_list_alex_));

        const auto expected = ContactListData{{
            {true, bob_.name_, bob_.name_, "B", ""},
            {true, chris_.name_, chris_.name_, "C", ""},
        }};

        ASSERT_TRUE(wait_for_counter(messagable_list_alex_));
        EXPECT_TRUE(check_messagable_list(alex_, expected));
        EXPECT_TRUE(check_messagable_list_qt(alex_, expected));
    } else {
        // TODO
    }
}

TEST_F(Test_AddContact, both)
{
    ASSERT_FALSE(alex_.nym_id_.empty());

    if (have_hd_) {
        ASSERT_FALSE(alex_.payment_code_.empty());
    } else {
        // TODO
    }

    contact_list_bob_.expected_ += 1;
    messagable_list_bob_.expected_ += 1;
    const auto& widget = bob_.api_->UI().Internal().ContactList(bob_.nym_id_);
    const auto id = widget.AddContact(
        alex_.name_,
        alex_.nym_id_.asBase58(api_alex_.Crypto()),
        alex_.payment_code_);
    api_bob_.OTX().ContextIdle(bob_.nym_id_, server_1_.id_).get();

    ASSERT_FALSE(id.empty());
    EXPECT_TRUE(bob_.SetContact(alex_.name_, id));
}

TEST_F(Test_AddContact, add_both_contact_list_bob)
{
    ASSERT_TRUE(wait_for_counter(contact_list_bob_));

    const auto expected = ContactListData{{
        {true, bob_.name_, bob_.name_, "ME", ""},
        {true, alex_.name_, alex_.name_, "A", ""},
    }};

    ASSERT_TRUE(wait_for_counter(contact_list_bob_));
    EXPECT_TRUE(check_contact_list(bob_, expected));
    EXPECT_TRUE(check_contact_list_qt(bob_, expected));
}

TEST_F(Test_AddContact, add_both_messagable_list_bob)
{
    ASSERT_TRUE(wait_for_counter(messagable_list_bob_));

    const auto expected = ContactListData{{
        {true, alex_.name_, alex_.name_, "A", ""},
    }};

    ASSERT_TRUE(wait_for_counter(messagable_list_bob_));
    EXPECT_TRUE(check_messagable_list(bob_, expected));
    EXPECT_TRUE(check_messagable_list_qt(bob_, expected));
}

TEST_F(Test_AddContact, backwards)
{
    ASSERT_FALSE(chris_.nym_id_.empty());

    if (have_hd_) {
        ASSERT_FALSE(chris_.payment_code_.empty());
    } else {
        // TODO
    }

    contact_list_bob_.expected_ += 1;
    messagable_list_bob_.expected_ += 1;
    const auto& widget = bob_.api_->UI().Internal().ContactList(bob_.nym_id_);
    const auto id = widget.AddContact(
        chris_.name_,
        chris_.payment_code_,
        chris_.nym_id_.asBase58(api_alex_.Crypto()));
    api_bob_.OTX().ContextIdle(bob_.nym_id_, server_1_.id_).get();

    ASSERT_FALSE(id.empty());
    EXPECT_TRUE(bob_.SetContact(chris_.name_, id));
}

TEST_F(Test_AddContact, add_backwards_contact_list_bob)
{
    ASSERT_TRUE(wait_for_counter(contact_list_bob_));

    const auto expected = ContactListData{{
        {true, bob_.name_, bob_.name_, "ME", ""},
        {true, alex_.name_, alex_.name_, "A", ""},
        {true, chris_.name_, chris_.name_, "C", ""},
    }};

    ASSERT_TRUE(wait_for_counter(contact_list_bob_));
    EXPECT_TRUE(check_contact_list(bob_, expected));
    EXPECT_TRUE(check_contact_list_qt(bob_, expected));
}

TEST_F(Test_AddContact, add_backwards_messagable_list_bob)
{
    ASSERT_TRUE(wait_for_counter(messagable_list_bob_));

    const auto expected = ContactListData{{
        {true, alex_.name_, alex_.name_, "A", ""},
        {true, chris_.name_, chris_.name_, "C", ""},
    }};

    ASSERT_TRUE(wait_for_counter(messagable_list_bob_));
    EXPECT_TRUE(check_messagable_list(bob_, expected));
    EXPECT_TRUE(check_messagable_list_qt(bob_, expected));
}

TEST_F(Test_AddContact, paymentcode_as_nymid)
{
    if (have_hd_) {
        ASSERT_FALSE(alex_.payment_code_.empty());

        contact_list_chris_.expected_ += 1;
        messagable_list_chris_.expected_ += 1;
        const auto& widget =
            chris_.api_->UI().Internal().ContactList(chris_.nym_id_);
        const auto id = widget.AddContact(alex_.name_, alex_.payment_code_, "");
        api_chris_.OTX().ContextIdle(chris_.nym_id_, server_1_.id_).get();

        ASSERT_FALSE(id.empty());
        EXPECT_TRUE(chris_.SetContact(alex_.name_, id));
    } else {
        // TODO
    }
}

TEST_F(Test_AddContact, add_payment_code_as_nymid_contact_list_chris)
{
    if (have_hd_) {
        ASSERT_TRUE(wait_for_counter(contact_list_chris_));

        const auto expected = ContactListData{{
            {true, chris_.name_, chris_.name_, "ME", ""},
            {true, alex_.name_, alex_.name_, "A", ""},
        }};

        ASSERT_TRUE(wait_for_counter(contact_list_chris_));
        EXPECT_TRUE(check_contact_list(chris_, expected));
        EXPECT_TRUE(check_contact_list_qt(chris_, expected));
    } else {
        // TODO
    }
}

TEST_F(Test_AddContact, add_payment_code_as_nymid_messagable_list_chris)
{
    if (have_hd_) {
        ASSERT_TRUE(wait_for_counter(messagable_list_chris_));

        const auto expected = ContactListData{{
            {true, alex_.name_, alex_.name_, "A", ""},
        }};

        ASSERT_TRUE(wait_for_counter(messagable_list_chris_));
        EXPECT_TRUE(check_messagable_list(chris_, expected));
        EXPECT_TRUE(check_messagable_list_qt(chris_, expected));
    } else {
        // TODO
    }
}

TEST_F(Test_AddContact, nymid_as_paymentcode)
{
    ASSERT_FALSE(bob_.nym_id_.empty());

    contact_list_chris_.expected_ += 1;
    messagable_list_chris_.expected_ += 1;
    const auto& widget =
        chris_.api_->UI().Internal().ContactList(chris_.nym_id_);
    const auto id = widget.AddContact(
        bob_.name_, "", bob_.nym_id_.asBase58(api_alex_.Crypto()));
    api_chris_.OTX().ContextIdle(chris_.nym_id_, server_1_.id_).get();

    ASSERT_FALSE(id.empty());
    EXPECT_TRUE(chris_.SetContact(bob_.name_, id));
}

TEST_F(Test_AddContact, add_nymid_as_paymentcode_contact_list_chris)
{
    ASSERT_TRUE(wait_for_counter(contact_list_chris_));

    const auto expected = [] {
        auto out = ContactListData{};
        out.rows_.emplace_back(
            ContactListRow{true, chris_.name_, chris_.name_, "ME", ""});

        if (have_hd_) {
            out.rows_.emplace_back(
                ContactListRow{true, alex_.name_, alex_.name_, "A", ""});
        }

        out.rows_.emplace_back(
            ContactListRow{true, bob_.name_, bob_.name_, "B", ""});

        return out;
    }();

    ASSERT_TRUE(wait_for_counter(contact_list_chris_));
    EXPECT_TRUE(check_contact_list(chris_, expected));
    EXPECT_TRUE(check_contact_list_qt(chris_, expected));
}

TEST_F(Test_AddContact, add_nymid_as_paymentcode_messagable_list_chris)
{
    ASSERT_TRUE(wait_for_counter(messagable_list_chris_));

    const auto expected = [] {
        auto out = ContactListData{};

        if (have_hd_) {
            out.rows_.emplace_back(
                ContactListRow{true, alex_.name_, alex_.name_, "A", ""});
        }

        out.rows_.emplace_back(
            ContactListRow{true, bob_.name_, bob_.name_, "B", ""});

        return out;
    }();

    ASSERT_TRUE(wait_for_counter(messagable_list_chris_));
    EXPECT_TRUE(check_messagable_list(chris_, expected));
    EXPECT_TRUE(check_messagable_list_qt(chris_, expected));
}

TEST_F(Test_AddContact, shutdown)
{
    api_alex_.OTX().ContextIdle(alex_.nym_id_, server_1_.id_).get();
    api_bob_.OTX().ContextIdle(bob_.nym_id_, server_1_.id_).get();
    api_chris_.OTX().ContextIdle(chris_.nym_id_, server_1_.id_).get();

    EXPECT_EQ(contact_list_alex_.expected_, contact_list_alex_.updated_);
    EXPECT_EQ(messagable_list_alex_.expected_, messagable_list_alex_.updated_);
    EXPECT_EQ(contact_list_bob_.expected_, contact_list_bob_.updated_);
    EXPECT_EQ(messagable_list_bob_.expected_, messagable_list_bob_.updated_);
    EXPECT_EQ(contact_list_chris_.expected_, contact_list_chris_.updated_);
    EXPECT_EQ(
        messagable_list_chris_.expected_, messagable_list_chris_.updated_);
}
}  // namespace ottest
