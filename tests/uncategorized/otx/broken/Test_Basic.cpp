// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::api::session::Notary

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <atomic>
#include <future>
#include <memory>
#include <sstream>
#include <utility>

#include "internal/api/session/UI.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/String.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/interface/ui/AccountActivity.hpp"
#include "internal/interface/ui/AccountList.hpp"
#include "internal/interface/ui/AccountListItem.hpp"
#include "internal/interface/ui/AccountSummary.hpp"
#include "internal/interface/ui/ActivitySummary.hpp"
#include "internal/interface/ui/ActivitySummaryItem.hpp"
#include "internal/interface/ui/BalanceItem.hpp"
#include "internal/interface/ui/Contact.hpp"
#include "internal/interface/ui/ContactActivity.hpp"
#include "internal/interface/ui/ContactActivityItem.hpp"
#include "internal/interface/ui/ContactList.hpp"
#include "internal/interface/ui/ContactListItem.hpp"
#include "internal/interface/ui/ContactSection.hpp"
#include "internal/interface/ui/IssuerItem.hpp"
#include "internal/interface/ui/MessagableList.hpp"
#include "internal/interface/ui/PayableList.hpp"
#include "internal/interface/ui/PayableListItem.hpp"
#include "internal/interface/ui/Profile.hpp"
#include "internal/interface/ui/ProfileSection.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/otx/common/Message.hpp"
#include "internal/util/SharedPimpl.hpp"
#include "ottest/env/OTTestEnvironment.hpp"
#include "ottest/fixtures/common/Counter.hpp"
#include "ottest/fixtures/common/User.hpp"
#include "ottest/fixtures/integration/Helpers.hpp"

#define UNIT_DEFINITION_CONTRACT_VERSION 2
#define UNIT_DEFINITION_CONTRACT_NAME "Mt Gox USD"
#define UNIT_DEFINITION_TERMS "YOLO"
#define UNIT_DEFINITION_TLA "USD"
#define UNIT_DEFINITION_UNIT_OF_ACCOUNT ot::UnitType::Usd
#define CHEQUE_AMOUNT_1 100
#define CHEQUE_AMOUNT_2 75
#define CHEQUE_MEMO "memo"

namespace ottest
{
Counter account_activity_usd_alex_{};
Counter account_list_alex_{};
Counter account_summary_bch_alex_{};
Counter account_summary_btc_alex_{};
Counter account_summary_usd_alex_{};
Counter activity_summary_alex_{};
Counter contact_activity_bob_alex_{};
Counter contact_activity_issuer_alex_{};
Counter contact_issuer_alex_{};
Counter contact_list_alex_{};
Counter messagable_list_alex_{};
Counter payable_list_bch_alex_{};
Counter payable_list_btc_alex_{};
Counter profile_alex_{};

Counter account_activity_usd_bob_{};
Counter account_list_bob_{};
Counter account_summary_bch_bob_{};
Counter account_summary_btc_bob_{};
Counter account_summary_usd_bob_{};
Counter activity_summary_bob_{};
Counter contact_activity_alex_bob_{};
Counter contact_list_bob_{};
Counter messagable_list_bob_{};
Counter payable_list_bch_bob_{};
Counter payable_list_btc_bob_{};
Counter profile_bob_{};

class Integration : public IntegrationFixture
{
public:
    static const bool have_hd_;
    static Issuer issuer_data_;
    static int msg_count_;
    static ot::UnallocatedMap<int, ot::UnallocatedCString> message_;
    static ot::identifier::UnitDefinition unit_id_;

    const ot::api::session::Client& api_alex_;
    const ot::api::session::Client& api_bob_;
    const ot::api::session::Client& api_issuer_;
    const ot::api::session::Notary& api_server_1_;

    auto idle() const noexcept -> void
    {
        api_alex_.OTX().ContextIdle(alex_.nym_id_, server_1_.id_).get();
        api_bob_.OTX().ContextIdle(bob_.nym_id_, server_1_.id_).get();
        api_issuer_.OTX().ContextIdle(issuer_.nym_id_, server_1_.id_).get();
    }

    Integration()
        : api_alex_(OTTestEnvironment::GetOT().StartClientSession(0))
        , api_bob_(OTTestEnvironment::GetOT().StartClientSession(1))
        , api_issuer_(OTTestEnvironment::GetOT().StartClientSession(2))
        , api_server_1_(OTTestEnvironment::GetOT().StartNotarySession(0))
    {
        const_cast<Server&>(server_1_).init(api_server_1_);
        const_cast<User&>(alex_).init(api_alex_, server_1_);
        const_cast<User&>(bob_).init(api_bob_, server_1_);
        const_cast<User&>(issuer_).init(api_issuer_, server_1_);
    }
};

const bool Integration::have_hd_{
    ot::api::crypto::HaveHDKeys() &&
    ot::api::crypto::HaveSupport(ot::crypto::asymmetric::Algorithm::Secp256k1)

};
int Integration::msg_count_ = 0;
ot::UnallocatedMap<int, ot::UnallocatedCString> Integration::message_{};
ot::identifier::UnitDefinition Integration::unit_id_{};
Issuer Integration::issuer_data_{};

TEST_F(Integration, instantiate_ui_objects)
{
    account_activity_usd_alex_.expected_ = 0;
    account_list_alex_.expected_ = 0;
    account_summary_bch_alex_.expected_ = 0;
    account_summary_btc_alex_.expected_ = 0;
    account_summary_usd_alex_.expected_ = 0;
    activity_summary_alex_.expected_ = 0;
    contact_activity_bob_alex_.expected_ = 0;
    contact_activity_issuer_alex_.expected_ = 0;
    contact_issuer_alex_.expected_ = 0;
    contact_list_alex_.expected_ = 1;
    messagable_list_alex_.expected_ = 0;
    payable_list_bch_alex_.expected_ = 0;
    if (have_hd_) {
        payable_list_btc_alex_.expected_ = 1;
    } else {
        payable_list_btc_alex_.expected_ = 0;
    }
    profile_alex_.expected_ = 1;

    account_activity_usd_bob_.expected_ = 0;
    account_list_bob_.expected_ = 0;
    account_summary_bch_bob_.expected_ = 0;
    account_summary_btc_bob_.expected_ = 0;
    account_summary_usd_bob_.expected_ = 0;
    activity_summary_bob_.expected_ = 0;
    contact_activity_alex_bob_.expected_ = 0;
    contact_list_bob_.expected_ = 1;
    messagable_list_bob_.expected_ = 0;
    payable_list_bch_bob_.expected_ = 0;
    if (have_hd_) {
        payable_list_btc_bob_.expected_ = 1;
    } else {
        payable_list_btc_bob_.expected_ = 0;
    }
    profile_bob_.expected_ = 1;

    api_alex_.UI().Internal().AccountList(
        alex_.nym_id_, make_cb(account_list_alex_, "alex account list"));
    api_alex_.UI().Internal().AccountSummary(
        alex_.nym_id_,
        ot::UnitType::Bch,
        make_cb(account_summary_bch_alex_, "alex account summary (BCH)"));
    api_alex_.UI().Internal().AccountSummary(
        alex_.nym_id_,
        ot::UnitType::Btc,
        make_cb(account_summary_btc_alex_, "alex account summary (BTC)"));
    api_alex_.UI().Internal().AccountSummary(
        alex_.nym_id_,
        ot::UnitType::Usd,
        make_cb(account_summary_usd_alex_, "alex account summary (USD)"));
    api_alex_.UI().Internal().ActivitySummary(
        alex_.nym_id_,
        make_cb(activity_summary_alex_, "alex activity summary"));
    api_alex_.UI().Internal().ContactList(
        alex_.nym_id_, make_cb(contact_list_alex_, "alex contact list"));
    api_alex_.UI().Internal().MessagableList(
        alex_.nym_id_, make_cb(messagable_list_alex_, "alex messagable list"));
    api_alex_.UI().Internal().PayableList(
        alex_.nym_id_,
        ot::UnitType::Bch,
        make_cb(payable_list_bch_alex_, "alex payable list (BCH)"));
    api_alex_.UI().Internal().PayableList(
        alex_.nym_id_,
        ot::UnitType::Btc,
        make_cb(payable_list_btc_alex_, "alex payable list (BTC)"));
    api_alex_.UI().Internal().Profile(
        alex_.nym_id_, make_cb(profile_alex_, "alex profile"));

    api_bob_.UI().Internal().AccountList(
        bob_.nym_id_, make_cb(account_list_bob_, "bob account list"));
    api_bob_.UI().Internal().AccountSummary(
        bob_.nym_id_,
        ot::UnitType::Bch,
        make_cb(account_summary_bch_bob_, "bob account summary (BCH)"));
    api_bob_.UI().Internal().AccountSummary(
        bob_.nym_id_,
        ot::UnitType::Btc,
        make_cb(account_summary_btc_bob_, "bob account summary (BTC)"));
    api_bob_.UI().Internal().AccountSummary(
        bob_.nym_id_,
        ot::UnitType::Usd,
        make_cb(account_summary_usd_bob_, "bob account summary (USD)"));
    api_bob_.UI().Internal().ActivitySummary(
        bob_.nym_id_, make_cb(activity_summary_bob_, "bob activity summary"));
    api_bob_.UI().Internal().ContactList(
        bob_.nym_id_, make_cb(contact_list_bob_, "bob contact list"));
    api_bob_.UI().Internal().MessagableList(
        bob_.nym_id_, make_cb(messagable_list_bob_, "bob messagable list"));
    api_bob_.UI().Internal().PayableList(
        bob_.nym_id_,
        ot::UnitType::Bch,
        make_cb(payable_list_bch_bob_, "bob payable list (BCH)"));
    api_bob_.UI().Internal().PayableList(
        bob_.nym_id_,
        ot::UnitType::Btc,
        make_cb(payable_list_btc_bob_, "bob payable list (BTC)"));
    api_bob_.UI().Internal().Profile(
        bob_.nym_id_, make_cb(profile_bob_, "bob profile"));
}

TEST_F(Integration, account_list_alex_0)
{
    ASSERT_TRUE(wait_for_counter(account_list_alex_));

    const auto& widget = alex_.api_->UI().Internal().AccountList(alex_.nym_id_);
    auto row = widget.First();

    EXPECT_FALSE(row->Valid());
}

TEST_F(Integration, account_summary_bch_alex_0)
{
    ASSERT_TRUE(wait_for_counter(account_summary_bch_alex_));

    const auto& widget = alex_.api_->UI().Internal().AccountSummary(
        alex_.nym_id_, ot::UnitType::Bch);
    auto row = widget.First();

    EXPECT_FALSE(row->Valid());
}

TEST_F(Integration, account_summary_btc_alex_0)
{
    ASSERT_TRUE(wait_for_counter(account_summary_btc_alex_));

    const auto& widget = alex_.api_->UI().Internal().AccountSummary(
        alex_.nym_id_, ot::UnitType::Btc);
    auto row = widget.First();

    EXPECT_FALSE(row->Valid());
}

TEST_F(Integration, account_summary_usd_alex_0)
{
    ASSERT_TRUE(wait_for_counter(account_summary_usd_alex_));

    const auto& widget = alex_.api_->UI().Internal().AccountSummary(
        alex_.nym_id_, ot::UnitType::Usd);
    auto row = widget.First();

    EXPECT_FALSE(row->Valid());
}

TEST_F(Integration, activity_summary_alex_0)
{
    ASSERT_TRUE(wait_for_counter(activity_summary_alex_));

    const auto& widget =
        alex_.api_->UI().Internal().ActivitySummary(alex_.nym_id_);
    auto row = widget.First();

    EXPECT_FALSE(row->Valid());
}

TEST_F(Integration, contact_list_alex_0)
{
    ASSERT_TRUE(wait_for_counter(contact_list_alex_));

    const auto& widget = alex_.api_->UI().Internal().ContactList(alex_.nym_id_);
    const auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_TRUE(
        row->DisplayName() == alex_.name_ || row->DisplayName() == "Owner");
    ASSERT_TRUE(row->Valid());
    EXPECT_STREQ("", row->ImageURI().c_str());
    EXPECT_STREQ("ME", row->Section().c_str());
    EXPECT_TRUE(row->Last());

    EXPECT_TRUE(alex_.SetContact(alex_.name_, row->ContactID()));
    EXPECT_FALSE(alex_.Contact(alex_.name_).empty());
}

TEST_F(Integration, messagable_list_alex_0)
{
    ASSERT_TRUE(wait_for_counter(messagable_list_alex_));

    const auto& widget =
        alex_.api_->UI().Internal().MessagableList(alex_.nym_id_);
    auto row = widget.First();

    EXPECT_FALSE(row->Valid());
}

TEST_F(Integration, payable_list_bch_alex_0)
{
    ASSERT_TRUE(wait_for_counter(payable_list_bch_alex_));

    const auto& widget = alex_.api_->UI().Internal().PayableList(
        alex_.nym_id_, ot::UnitType::Bch);
    auto row = widget.First();

    EXPECT_FALSE(row->Valid());
}

TEST_F(Integration, payable_list_btc_alex_0)
{
    ASSERT_TRUE(wait_for_counter(payable_list_btc_alex_));

    const auto& widget = alex_.api_->UI().Internal().PayableList(
        alex_.nym_id_, ot::UnitType::Btc);
    auto row = widget.First();

    if (have_hd_) {
        ASSERT_TRUE(row->Valid());
        EXPECT_EQ(row->DisplayName(), alex_.name_);
        EXPECT_TRUE(row->Last());
    } else {
        EXPECT_FALSE(row->Valid());
    }
}

TEST_F(Integration, profile_alex_0)
{
    ASSERT_TRUE(wait_for_counter(profile_alex_));

    const auto& widget = alex_.api_->UI().Internal().Profile(alex_.nym_id_);

    if (have_hd_) {
        EXPECT_EQ(widget.PaymentCode(), alex_.PaymentCode().asBase58());
    } else {
        EXPECT_EQ(widget.DisplayName(), alex_.name_);
    }

    auto row = widget.First();

    EXPECT_FALSE(row->Valid());
}

TEST_F(Integration, account_list_bob_0)
{
    ASSERT_TRUE(wait_for_counter(account_list_bob_));

    const auto& widget = bob_.api_->UI().Internal().AccountList(bob_.nym_id_);
    auto row = widget.First();

    EXPECT_FALSE(row->Valid());
}

TEST_F(Integration, account_summary_bch_bob_0)
{
    ASSERT_TRUE(wait_for_counter(account_summary_bch_bob_));

    const auto& widget = bob_.api_->UI().Internal().AccountSummary(
        bob_.nym_id_, ot::UnitType::Bch);
    auto row = widget.First();

    EXPECT_FALSE(row->Valid());
}

TEST_F(Integration, account_summary_btc_bob_0)
{
    ASSERT_TRUE(wait_for_counter(account_summary_btc_bob_));

    const auto& widget = bob_.api_->UI().Internal().AccountSummary(
        bob_.nym_id_, ot::UnitType::Btc);
    auto row = widget.First();

    EXPECT_FALSE(row->Valid());
}

TEST_F(Integration, account_summary_usd_bob_0)
{
    ASSERT_TRUE(wait_for_counter(account_summary_usd_bob_));

    const auto& widget = bob_.api_->UI().Internal().AccountSummary(
        bob_.nym_id_, ot::UnitType::Usd);
    auto row = widget.First();

    EXPECT_FALSE(row->Valid());
}

TEST_F(Integration, activity_summary_bob_0)
{
    ASSERT_TRUE(wait_for_counter(activity_summary_bob_));

    const auto& widget =
        bob_.api_->UI().Internal().ActivitySummary(bob_.nym_id_);
    auto row = widget.First();

    EXPECT_FALSE(row->Valid());
}

TEST_F(Integration, contact_list_bob_0)
{
    ASSERT_TRUE(wait_for_counter(contact_list_bob_));

    const auto& widget = bob_.api_->UI().Internal().ContactList(bob_.nym_id_);
    const auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_TRUE(
        row->DisplayName() == bob_.name_ || row->DisplayName() == "Owner");
    ASSERT_TRUE(row->Valid());
    EXPECT_STREQ("", row->ImageURI().c_str());
    EXPECT_STREQ("ME", row->Section().c_str());
    EXPECT_TRUE(row->Last());

    EXPECT_TRUE(bob_.SetContact(bob_.name_, row->ContactID()));
    EXPECT_FALSE(bob_.Contact(bob_.name_).empty());
}

TEST_F(Integration, messagable_list_bob_0)
{
    ASSERT_TRUE(wait_for_counter(messagable_list_bob_));

    const auto& widget =
        bob_.api_->UI().Internal().MessagableList(bob_.nym_id_);
    auto row = widget.First();

    EXPECT_FALSE(row->Valid());
}

TEST_F(Integration, payable_list_bch_bob_0)
{
    ASSERT_TRUE(wait_for_counter(payable_list_bch_bob_));

    const auto& widget =
        bob_.api_->UI().Internal().PayableList(bob_.nym_id_, ot::UnitType::Bch);
    auto row = widget.First();

    EXPECT_FALSE(row->Valid());
}

TEST_F(Integration, payable_list_btc_bob_0)
{
    ASSERT_TRUE(wait_for_counter(payable_list_btc_bob_));

    const auto& widget =
        bob_.api_->UI().Internal().PayableList(bob_.nym_id_, ot::UnitType::Btc);
    auto row = widget.First();

    if (have_hd_) {
        ASSERT_TRUE(row->Valid());
        EXPECT_EQ(row->DisplayName(), bob_.name_);
        EXPECT_TRUE(row->Last());
    } else {
        EXPECT_FALSE(row->Valid());
    }
}

TEST_F(Integration, profile_bob_0)
{
    ASSERT_TRUE(wait_for_counter(profile_bob_));

    const auto& widget = bob_.api_->UI().Internal().Profile(bob_.nym_id_);

    if (have_hd_) {
        EXPECT_EQ(widget.PaymentCode(), bob_.PaymentCode().asBase58());
    } else {
        EXPECT_EQ(widget.DisplayName(), bob_.name_);
    }

    auto row = widget.First();

    EXPECT_FALSE(row->Valid());
}

TEST_F(Integration, payment_codes)
{
    auto alex = api_alex_.Wallet().mutable_Nym(alex_.nym_id_, alex_.Reason());
    auto bob = api_bob_.Wallet().mutable_Nym(bob_.nym_id_, bob_.Reason());
    auto issuer =
        api_issuer_.Wallet().mutable_Nym(issuer_.nym_id_, issuer_.Reason());

    EXPECT_EQ(ot::identity::wot::claim::ClaimType::Individual, alex.Type());
    EXPECT_EQ(ot::identity::wot::claim::ClaimType::Individual, bob.Type());
    EXPECT_EQ(ot::identity::wot::claim::ClaimType::Individual, issuer.Type());

    auto alexScopeSet = alex.SetScope(
        ot::identity::wot::claim::ClaimType::Individual,
        alex_.name_,
        true,
        alex_.Reason());
    auto bobScopeSet = bob.SetScope(
        ot::identity::wot::claim::ClaimType::Individual,
        bob_.name_,
        true,
        bob_.Reason());
    auto issuerScopeSet = issuer.SetScope(
        ot::identity::wot::claim::ClaimType::Individual,
        issuer_.name_,
        true,
        issuer_.Reason());

    EXPECT_TRUE(alexScopeSet);
    EXPECT_TRUE(bobScopeSet);
    EXPECT_TRUE(issuerScopeSet);

    if (have_hd_) {
        EXPECT_FALSE(alex_.payment_code_.empty());
        EXPECT_FALSE(bob_.payment_code_.empty());
        EXPECT_FALSE(issuer_.payment_code_.empty());

        alex.AddPaymentCode(
            alex_.payment_code_, ot::UnitType::Btc, true, true, alex_.Reason());
        bob.AddPaymentCode(
            bob_.payment_code_, ot::UnitType::Btc, true, true, bob_.Reason());
        issuer.AddPaymentCode(
            issuer_.payment_code_,
            ot::UnitType::Btc,
            true,
            true,
            issuer_.Reason());
        alex.AddPaymentCode(
            alex_.payment_code_, ot::UnitType::Bch, true, true, alex_.Reason());
        bob.AddPaymentCode(
            bob_.payment_code_, ot::UnitType::Bch, true, true, bob_.Reason());
        issuer.AddPaymentCode(
            issuer_.payment_code_,
            ot::UnitType::Bch,
            true,
            true,
            issuer_.Reason());

        EXPECT_FALSE(alex.PaymentCode(ot::UnitType::Btc).empty());
        EXPECT_FALSE(bob.PaymentCode(ot::UnitType::Btc).empty());
        EXPECT_FALSE(issuer.PaymentCode(ot::UnitType::Btc).empty());
        EXPECT_FALSE(alex.PaymentCode(ot::UnitType::Bch).empty());
        EXPECT_FALSE(bob.PaymentCode(ot::UnitType::Bch).empty());
        EXPECT_FALSE(issuer.PaymentCode(ot::UnitType::Bch).empty());
    }

    alex.Release();
    bob.Release();
    issuer.Release();
}

TEST_F(Integration, introduction_server)
{
    api_alex_.OTX().StartIntroductionServer(alex_.nym_id_);
    api_bob_.OTX().StartIntroductionServer(bob_.nym_id_);
    auto task1 =
        api_alex_.OTX().RegisterNymPublic(alex_.nym_id_, server_1_.id_, true);
    auto task2 =
        api_bob_.OTX().RegisterNymPublic(bob_.nym_id_, server_1_.id_, true);

    ASSERT_NE(0, task1.first);
    ASSERT_NE(0, task2.first);
    EXPECT_EQ(
        ot::otx::LastReplyStatus::MessageSuccess, task1.second.get().first);
    EXPECT_EQ(
        ot::otx::LastReplyStatus::MessageSuccess, task2.second.get().first);

    api_alex_.OTX().ContextIdle(alex_.nym_id_, server_1_.id_).get();
    api_bob_.OTX().ContextIdle(bob_.nym_id_, server_1_.id_).get();
}

TEST_F(Integration, add_contact_preconditions)
{
    // Neither alex nor bob should know about each other yet
    auto alex = api_bob_.Wallet().Nym(alex_.nym_id_);
    auto bob = api_alex_.Wallet().Nym(bob_.nym_id_);

    EXPECT_FALSE(alex);
    EXPECT_FALSE(bob);
}

TEST_F(Integration, add_contact_Bob_To_Alex)
{
    contact_list_alex_.expected_ += 1;
    messagable_list_alex_.expected_ += 1;
    if (have_hd_) {
        payable_list_bch_alex_.expected_ += 1;
        payable_list_btc_alex_.expected_ += 1;
    }

    alex_.api_->UI()
        .Internal()
        .ContactList(alex_.nym_id_)
        .AddContact(
            bob_.name_,
            bob_.payment_code_,
            bob_.nym_id_.asBase58(api_alex_.Crypto()));
}

TEST_F(Integration, contact_list_alex_1)
{
    ASSERT_TRUE(wait_for_counter(contact_list_alex_));

    const auto& widget = alex_.api_->UI().Internal().ContactList(alex_.nym_id_);
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->DisplayName(), alex_.name_);
    ASSERT_TRUE(row->Valid());
    EXPECT_STREQ("", row->ImageURI().c_str());
    EXPECT_STREQ("ME", row->Section().c_str());
    EXPECT_FALSE(row->Last());

    row = widget.Next();

    EXPECT_EQ(row->DisplayName(), bob_.name_);
    ASSERT_TRUE(row->Valid());
    EXPECT_STREQ("", row->ImageURI().c_str());
    EXPECT_STREQ("B", row->Section().c_str());
    EXPECT_TRUE(row->Last());

    EXPECT_TRUE(alex_.SetContact(bob_.name_, row->ContactID()));
    EXPECT_FALSE(alex_.Contact(bob_.name_).empty());
}

TEST_F(Integration, messagable_list_alex_1)
{
    ASSERT_TRUE(wait_for_counter(messagable_list_alex_));

    const auto& widget =
        alex_.api_->UI().Internal().MessagableList(alex_.nym_id_);
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->DisplayName(), bob_.name_);
    ASSERT_TRUE(row->Valid());
    EXPECT_STREQ("", row->ImageURI().c_str());
    EXPECT_STREQ("B", row->Section().c_str());
    EXPECT_TRUE(row->Last());
}

TEST_F(Integration, payable_list_bch_alex_1)
{
    if (have_hd_) {
        ASSERT_TRUE(wait_for_counter(payable_list_bch_alex_));

        const auto& widget = alex_.api_->UI().Internal().PayableList(
            alex_.nym_id_, ot::UnitType::Bch);
        auto row = widget.First();

        ASSERT_TRUE(row->Valid());
        EXPECT_EQ(row->DisplayName(), bob_.name_);
        EXPECT_TRUE(row->Last());
    }
}

TEST_F(Integration, payable_list_btc_alex_1)
{
    if (have_hd_) {
        ASSERT_TRUE(wait_for_counter(payable_list_btc_alex_));

        const auto& widget = alex_.api_->UI().Internal().PayableList(
            alex_.nym_id_, ot::UnitType::Btc);
        auto row = widget.First();

        ASSERT_TRUE(row->Valid());
        EXPECT_EQ(row->DisplayName(), alex_.name_);
        EXPECT_FALSE(row->Last());

        row = widget.Next();

        EXPECT_EQ(row->DisplayName(), bob_.name_);
        EXPECT_TRUE(row->Last());
    }
}

TEST_F(Integration, contact_activity_bob_alex_0)
{
    contact_activity_bob_alex_.expected_ += 2;

    const auto& widget = api_alex_.UI().Internal().ContactActivity(
        alex_.nym_id_,
        alex_.Contact(bob_.name_),
        make_cb(contact_activity_bob_alex_, "Alex's activity thread with Bob"));

    ASSERT_TRUE(wait_for_counter(contact_activity_bob_alex_));

    auto row = widget.First();

    EXPECT_FALSE(row->Valid());
}

TEST_F(Integration, send_message_from_Alex_to_Bob_1)
{
    activity_summary_alex_.expected_ += 2;
    contact_activity_bob_alex_.expected_ += 2;
    messagable_list_alex_.expected_ += 1;
    activity_summary_bob_.expected_ += 2;
    contact_list_bob_.expected_ += 2;
    messagable_list_bob_.expected_ += 2;

    if (have_hd_) {
        payable_list_bch_alex_.expected_ += 1;
        payable_list_bch_bob_.expected_ += 1;
        payable_list_btc_bob_.expected_ += 1;
    }

    const auto& from_client = api_alex_;
    const auto messageID = ++msg_count_;
    auto text = std::stringstream{};
    text << alex_.name_ << " messaged " << bob_.name_ << " with message #"
         << std::to_string(messageID);
    auto& firstMessage = message_[messageID];
    firstMessage = text.str();
    const auto& conversation = from_client.UI().Internal().ContactActivity(
        alex_.nym_id_, alex_.Contact(bob_.name_));
    conversation.SetDraft(firstMessage);

    EXPECT_EQ(conversation.GetDraft(), firstMessage);

    idle();
    conversation.SendDraft();

    EXPECT_EQ(conversation.GetDraft(), "");
}

TEST_F(Integration, activity_summary_alex_1)
{
    ASSERT_TRUE(wait_for_counter(activity_summary_alex_));

    const auto& firstMessage = message_[msg_count_];
    const auto& widget =
        alex_.api_->UI().Internal().ActivitySummary(alex_.nym_id_);
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->DisplayName(), bob_.name_);
    EXPECT_EQ(row->ImageURI(), "");
    EXPECT_EQ(row->Text(), firstMessage);
    EXPECT_FALSE(row->ThreadID().empty());
    EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
    EXPECT_EQ(row->Type(), ot::otx::client::StorageBox::MAILOUTBOX);
    EXPECT_TRUE(row->Last());
}

TEST_F(Integration, contact_activity_bob_alex_1)
{
    ASSERT_TRUE(wait_for_counter(contact_activity_bob_alex_));

    const auto& firstMessage = message_[msg_count_];
    const auto& widget = alex_.api_->UI().Internal().ContactActivity(
        alex_.nym_id_, alex_.Contact(bob_.name_));
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->Amount(), 0);
    EXPECT_EQ(row->DisplayAmount(), "");
    EXPECT_FALSE(row->Loading());
    EXPECT_EQ(row->Memo(), "");
    EXPECT_FALSE(row->Pending());
    EXPECT_EQ(row->Text(), firstMessage);
    EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
    EXPECT_EQ(row->Type(), ot::otx::client::StorageBox::MAILOUTBOX);
    EXPECT_TRUE(row->Last());
}

TEST_F(Integration, contact_list_alex_2)
{
    ASSERT_TRUE(wait_for_counter(contact_list_alex_));

    const auto& widget = alex_.api_->UI().Internal().ContactList(alex_.nym_id_);
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->DisplayName(), alex_.name_);
    ASSERT_TRUE(row->Valid());
    EXPECT_STREQ("", row->ImageURI().c_str());
    EXPECT_STREQ("ME", row->Section().c_str());
    EXPECT_FALSE(row->Last());

    row = widget.Next();

    EXPECT_EQ(row->DisplayName(), bob_.name_);
    ASSERT_TRUE(row->Valid());
    EXPECT_STREQ("", row->ImageURI().c_str());
    EXPECT_STREQ("B", row->Section().c_str());
    EXPECT_TRUE(row->Last());
}

TEST_F(Integration, activity_summary_bob_1)
{
    ASSERT_TRUE(wait_for_counter(activity_summary_bob_));

    const auto& firstMessage = message_[msg_count_];
    const auto& widget =
        bob_.api_->UI().Internal().ActivitySummary(bob_.nym_id_);
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->DisplayName(), alex_.name_);
    EXPECT_EQ(row->ImageURI(), "");
    EXPECT_EQ(row->Text(), firstMessage);
    EXPECT_FALSE(row->ThreadID().empty());
    EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
    EXPECT_EQ(row->Type(), ot::otx::client::StorageBox::MAILINBOX);
    EXPECT_TRUE(row->Last());
}

TEST_F(Integration, contact_list_bob_1)
{
    ASSERT_TRUE(wait_for_counter(contact_list_bob_));

    const auto& widget = bob_.api_->UI().Internal().ContactList(bob_.nym_id_);
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->DisplayName(), bob_.name_);
    EXPECT_STREQ("", row->ImageURI().c_str());
    EXPECT_STREQ("ME", row->Section().c_str());
    EXPECT_FALSE(row->Last());

    row = widget.Next();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->DisplayName(), alex_.name_);
    EXPECT_STREQ("", row->ImageURI().c_str());
    EXPECT_STREQ("A", row->Section().c_str());
    EXPECT_TRUE(row->Last());

    EXPECT_TRUE(bob_.SetContact(alex_.name_, row->ContactID()));
    EXPECT_FALSE(bob_.Contact(alex_.name_).empty());
}

TEST_F(Integration, messagable_list_bob_1)
{
    ASSERT_TRUE(wait_for_counter(messagable_list_bob_));

    const auto& widget =
        bob_.api_->UI().Internal().MessagableList(bob_.nym_id_);
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->DisplayName(), alex_.name_);
    EXPECT_STREQ("", row->ImageURI().c_str());
    EXPECT_STREQ("A", row->Section().c_str());
    EXPECT_TRUE(row->Last());
}

TEST_F(Integration, payable_list_bch_alex_2)
{
    if (have_hd_) {
        ASSERT_TRUE(wait_for_counter(payable_list_bch_alex_));

        const auto& widget = alex_.api_->UI().Internal().PayableList(
            alex_.nym_id_, ot::UnitType::Bch);
        auto row = widget.First();

        ASSERT_TRUE(row->Valid());
        EXPECT_EQ(row->DisplayName(), alex_.name_);
        EXPECT_FALSE(row->Last());

        row = widget.Next();

        EXPECT_EQ(row->DisplayName(), bob_.name_);
        EXPECT_TRUE(row->Last());
    }
}

TEST_F(Integration, payable_list_bch_bob_1)
{
    if (have_hd_) {
        ASSERT_TRUE(wait_for_counter(payable_list_bch_bob_));

        const auto& widget = bob_.api_->UI().Internal().PayableList(
            bob_.nym_id_, ot::UnitType::Btc);
        auto row = widget.First();

        ASSERT_TRUE(row->Valid());
        EXPECT_EQ(row->DisplayName(), alex_.name_);
        EXPECT_FALSE(row->Last());

        row = widget.Next();

        EXPECT_EQ(row->DisplayName(), bob_.name_);
        EXPECT_TRUE(row->Last());
    }
}

TEST_F(Integration, payable_list_btc_bob_1)
{
    if (have_hd_) {
        ASSERT_TRUE(wait_for_counter(payable_list_btc_bob_));

        const auto& widget = bob_.api_->UI().Internal().PayableList(
            bob_.nym_id_, ot::UnitType::Bch);
        auto row = widget.First();

        ASSERT_TRUE(row->Valid());
        EXPECT_EQ(row->DisplayName(), alex_.name_);
        EXPECT_TRUE(row->Last());

        // TODO why isn't Bob in this list?
    }
}

TEST_F(Integration, contact_activity_alex_bob_0)
{
    contact_activity_alex_bob_.expected_ += 3;

    const auto& widget = api_bob_.UI().Internal().ContactActivity(
        bob_.nym_id_,
        bob_.Contact(alex_.name_),
        make_cb(contact_activity_alex_bob_, "Bob's activity thread with Alex"));

    ASSERT_TRUE(wait_for_counter(contact_activity_alex_bob_));

    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->Amount(), 0);
    EXPECT_EQ(row->DisplayAmount(), "");
    EXPECT_FALSE(row->Loading());
    EXPECT_EQ(row->Memo(), "");
    EXPECT_FALSE(row->Pending());
    EXPECT_EQ(row->Text(), message_.at(msg_count_));
    EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
    EXPECT_EQ(row->Type(), ot::otx::client::StorageBox::MAILINBOX);
}

TEST_F(Integration, send_message_from_Bob_to_Alex_2)
{
    activity_summary_alex_.expected_ += 1;
    contact_activity_bob_alex_.expected_ += 3;
    activity_summary_bob_.expected_ += 2;
    contact_activity_alex_bob_.expected_ += 5;

    if (have_hd_) { payable_list_bch_bob_.expected_ += 1; }

    const auto& from_client = api_bob_;
    const auto messageID = ++msg_count_;
    std::stringstream text{};
    text << bob_.name_ << " messaged " << alex_.name_ << " with message #"
         << std::to_string(messageID);
    auto& secondMessage = message_[messageID];
    secondMessage = text.str();
    const auto& conversation = from_client.UI().Internal().ContactActivity(
        bob_.nym_id_, bob_.Contact(alex_.name_));
    conversation.SetDraft(secondMessage);

    EXPECT_EQ(conversation.GetDraft(), secondMessage);

    idle();
    conversation.SendDraft();

    EXPECT_EQ(conversation.GetDraft(), "");
}

TEST_F(Integration, activity_summary_alex_2)
{
    ASSERT_TRUE(wait_for_counter(activity_summary_alex_));

    const auto& secondMessage = message_[msg_count_];
    const auto& widget =
        alex_.api_->UI().Internal().ActivitySummary(alex_.nym_id_);
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->DisplayName(), bob_.name_);
    EXPECT_EQ(row->ImageURI(), "");
    EXPECT_EQ(row->Text(), secondMessage);
    EXPECT_FALSE(row->ThreadID().empty());
    EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
    EXPECT_EQ(row->Type(), ot::otx::client::StorageBox::MAILINBOX);
    EXPECT_TRUE(row->Last());
}

TEST_F(Integration, contact_activity_bob_alex_2)
{
    ASSERT_TRUE(wait_for_counter(contact_activity_bob_alex_));

    const auto& firstMessage = message_[msg_count_ - 1];
    const auto& secondMessage = message_[msg_count_];
    const auto& widget = alex_.api_->UI().Internal().ContactActivity(
        alex_.nym_id_, alex_.Contact(bob_.name_));
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->Amount(), 0);
    EXPECT_EQ(row->DisplayAmount(), "");
    EXPECT_FALSE(row->Loading());
    EXPECT_EQ(row->Memo(), "");
    EXPECT_FALSE(row->Pending());
    EXPECT_EQ(row->Text(), firstMessage);
    EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
    EXPECT_EQ(row->Type(), ot::otx::client::StorageBox::MAILOUTBOX);
    EXPECT_FALSE(row->Last());

    row = widget.Next();

    EXPECT_EQ(row->Amount(), 0);
    EXPECT_EQ(row->DisplayAmount(), "");
    EXPECT_FALSE(row->Loading());
    EXPECT_EQ(row->Memo(), "");
    EXPECT_FALSE(row->Pending());
    EXPECT_EQ(row->Text(), secondMessage);
    EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
    EXPECT_EQ(row->Type(), ot::otx::client::StorageBox::MAILINBOX);
    EXPECT_TRUE(row->Last());
}

TEST_F(Integration, activity_summary_bob_2)
{
    ASSERT_TRUE(wait_for_counter(activity_summary_bob_));

    const auto& secondMessage = message_[msg_count_];
    const auto& widget =
        bob_.api_->UI().Internal().ActivitySummary(bob_.nym_id_);
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->DisplayName(), alex_.name_);
    EXPECT_EQ(row->ImageURI(), "");
    EXPECT_EQ(row->Text(), secondMessage);
    EXPECT_FALSE(row->ThreadID().empty());
    EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
    EXPECT_EQ(row->Type(), ot::otx::client::StorageBox::MAILOUTBOX);
    EXPECT_TRUE(row->Last());
}

TEST_F(Integration, contact_activity_alex_bob_1)
{
    ASSERT_TRUE(wait_for_counter(contact_activity_alex_bob_));

    const auto& firstMessage = message_[msg_count_ - 1];
    const auto& secondMessage = message_[msg_count_];
    const auto& widget = bob_.api_->UI().Internal().ContactActivity(
        bob_.nym_id_, bob_.Contact(alex_.name_));
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->Amount(), 0);
    EXPECT_EQ(row->DisplayAmount(), "");
    EXPECT_FALSE(row->Loading());
    EXPECT_EQ(row->Memo(), "");
    EXPECT_FALSE(row->Pending());
    EXPECT_EQ(row->Text(), firstMessage);
    EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
    EXPECT_EQ(row->Type(), ot::otx::client::StorageBox::MAILINBOX);
    EXPECT_FALSE(row->Last());

    row = widget.Next();
    bool loading{true};

    // This allows the test to work correctly in valgrind when
    // loading is unusually slow
    while (loading) {
        row = widget.First();
        row = widget.Next();
        loading = row->Loading();
    }

    EXPECT_EQ(row->Amount(), 0);
    EXPECT_EQ(row->DisplayAmount(), "");
    EXPECT_FALSE(row->Loading());
    EXPECT_EQ(row->Memo(), "");
    EXPECT_FALSE(row->Pending());
    EXPECT_EQ(row->Text(), secondMessage);
    EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
    EXPECT_EQ(row->Type(), ot::otx::client::StorageBox::MAILOUTBOX);
    EXPECT_TRUE(row->Last());
}

TEST_F(Integration, contact_list_bob_2)
{
    ASSERT_TRUE(wait_for_counter(contact_list_bob_));

    const auto& widget = bob_.api_->UI().Internal().ContactList(bob_.nym_id_);
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->DisplayName(), bob_.name_);
    ASSERT_TRUE(row->Valid());
    EXPECT_STREQ("", row->ImageURI().c_str());
    EXPECT_STREQ("ME", row->Section().c_str());
    EXPECT_FALSE(row->Last());

    row = widget.Next();

    EXPECT_EQ(row->DisplayName(), alex_.name_);
    ASSERT_TRUE(row->Valid());
    EXPECT_STREQ("", row->ImageURI().c_str());
    EXPECT_STREQ("A", row->Section().c_str());
    EXPECT_TRUE(row->Last());
}

TEST_F(Integration, payable_list_bch_bob_2)
{
    if (have_hd_) {
        ASSERT_TRUE(wait_for_counter(payable_list_bch_bob_));

        const auto& widget = bob_.api_->UI().Internal().PayableList(
            bob_.nym_id_, ot::UnitType::Btc);
        auto row = widget.First();

        ASSERT_TRUE(row->Valid());
        EXPECT_EQ(row->DisplayName(), alex_.name_);
        EXPECT_FALSE(row->Last());

        row = widget.Next();

        EXPECT_EQ(row->DisplayName(), bob_.name_);
        EXPECT_TRUE(row->Last());
    }
}

TEST_F(Integration, issue_dollars)
{
    const auto contract = api_issuer_.Wallet().Internal().CurrencyContract(
        issuer_.nym_id_.asBase58(api_alex_.Crypto()),
        UNIT_DEFINITION_CONTRACT_NAME,
        UNIT_DEFINITION_TERMS,
        UNIT_DEFINITION_UNIT_OF_ACCOUNT,
        1,
        issuer_.Reason());

    EXPECT_EQ(UNIT_DEFINITION_CONTRACT_VERSION, contract->Version());
    EXPECT_EQ(ot::contract::UnitType::Currency, contract->Type());
    EXPECT_EQ(UNIT_DEFINITION_UNIT_OF_ACCOUNT, contract->UnitOfAccount());
    EXPECT_TRUE(unit_id_.empty());

    unit_id_.Assign(contract->ID());

    EXPECT_FALSE(unit_id_.empty());

    {
        auto issuer =
            api_issuer_.Wallet().mutable_Nym(issuer_.nym_id_, issuer_.Reason());
        issuer.AddPreferredOTServer(
            server_1_.id_.asBase58(api_alex_.Crypto()), true, issuer_.Reason());
    }

    auto task = api_issuer_.OTX().IssueUnitDefinition(
        issuer_.nym_id_, server_1_.id_, unit_id_, ot::UnitType::Usd);
    auto& [taskID, future] = task;
    const auto result = future.get();

    EXPECT_NE(0, taskID);
    EXPECT_EQ(ot::otx::LastReplyStatus::MessageSuccess, result.first);
    ASSERT_TRUE(result.second);

    EXPECT_TRUE(issuer_.SetAccount(
        UNIT_DEFINITION_TLA, result.second->acct_id_->Get()));
    EXPECT_FALSE(issuer_.Account(UNIT_DEFINITION_TLA).empty());

    api_issuer_.OTX().ContextIdle(issuer_.nym_id_, server_1_.id_).get();

    {
        const auto pNym = api_issuer_.Wallet().Nym(issuer_.nym_id_);

        ASSERT_TRUE(pNym);

        const auto& nym = *pNym;
        const auto& claims = nym.Claims();
        const auto pSection =
            claims.Section(ot::identity::wot::claim::SectionType::Contract);

        ASSERT_TRUE(pSection);

        const auto& section = *pSection;
        const auto pGroup =
            section.Group(ot::identity::wot::claim::ClaimType::Usd);

        ASSERT_TRUE(pGroup);

        const auto& group = *pGroup;
        const auto& pClaim = group.PrimaryClaim();

        EXPECT_EQ(1, group.Size());
        ASSERT_TRUE(pClaim);

        const auto& claim = *pClaim;

        EXPECT_EQ(claim.Value(), unit_id_.asBase58(api_alex_.Crypto()));
    }
}

TEST_F(Integration, add_alex_contact_to_issuer)
{
    EXPECT_TRUE(issuer_.SetContact(
        alex_.name_, api_issuer_.Contacts().NymToContact(alex_.nym_id_)));
    EXPECT_FALSE(issuer_.Contact(alex_.name_).empty());

    api_issuer_.OTX().Refresh();
    api_issuer_.OTX().ContextIdle(issuer_.nym_id_, server_1_.id_).get();
}

TEST_F(Integration, pay_alex)
{
    activity_summary_alex_.expected_ += 2;
    contact_list_alex_.expected_ += 2;
    messagable_list_alex_.expected_ += 1;

    if (have_hd_) {
        payable_list_bch_alex_.expected_ += 1;
        payable_list_btc_alex_.expected_ += 1;
    }

    idle();
    auto task = api_issuer_.OTX().SendCheque(
        issuer_.nym_id_,
        issuer_.Account(UNIT_DEFINITION_TLA),
        issuer_.Contact(alex_.name_),
        CHEQUE_AMOUNT_1,
        CHEQUE_MEMO);
    auto& [taskID, future] = task;

    ASSERT_NE(0, taskID);
    EXPECT_EQ(ot::otx::LastReplyStatus::MessageSuccess, future.get().first);

    api_alex_.OTX().Refresh();
    idle();
}

TEST_F(Integration, activity_summary_alex_3)
{
    ASSERT_TRUE(wait_for_counter(activity_summary_alex_));

    const auto& secondMessage = message_[msg_count_];
    const auto& widget =
        alex_.api_->UI().Internal().ActivitySummary(alex_.nym_id_);
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->DisplayName(), issuer_.name_);
    EXPECT_STREQ("", row->ImageURI().c_str());
    EXPECT_STREQ("Received cheque", row->Text().c_str());
    EXPECT_FALSE(row->ThreadID().empty());
    EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
    EXPECT_EQ(ot::otx::client::StorageBox::INCOMINGCHEQUE, row->Type());
    EXPECT_FALSE(row->Last());

    row = widget.Next();

    EXPECT_EQ(row->DisplayName(), bob_.name_);
    EXPECT_EQ(row->ImageURI(), "");
    EXPECT_EQ(row->Text(), secondMessage);
    EXPECT_FALSE(row->ThreadID().empty());
    EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
    EXPECT_EQ(row->Type(), ot::otx::client::StorageBox::MAILINBOX);
    EXPECT_TRUE(row->Last());
}

TEST_F(Integration, contact_list_alex_3)
{
    ASSERT_TRUE(wait_for_counter(contact_list_alex_));

    const auto& widget = alex_.api_->UI().Internal().ContactList(alex_.nym_id_);
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->DisplayName(), alex_.name_);
    ASSERT_TRUE(row->Valid());
    EXPECT_STREQ("", row->ImageURI().c_str());
    EXPECT_STREQ("ME", row->Section().c_str());
    EXPECT_FALSE(row->Last());

    row = widget.Next();

    EXPECT_EQ(row->DisplayName(), bob_.name_);
    ASSERT_TRUE(row->Valid());
    EXPECT_STREQ("", row->ImageURI().c_str());
    EXPECT_STREQ("B", row->Section().c_str());
    EXPECT_FALSE(row->Last());

    row = widget.Next();

    EXPECT_EQ(row->DisplayName(), issuer_.name_);
    ASSERT_TRUE(row->Valid());
    EXPECT_STREQ("", row->ImageURI().c_str());
    EXPECT_STREQ("I", row->Section().c_str());
    EXPECT_TRUE(row->Last());

    EXPECT_TRUE(alex_.SetContact(issuer_.name_, row->ContactID()));
    EXPECT_FALSE(alex_.Contact(issuer_.name_).empty());
}

TEST_F(Integration, messagable_list_alex_2)
{
    ASSERT_TRUE(wait_for_counter(messagable_list_alex_));

    const auto& widget =
        alex_.api_->UI().Internal().MessagableList(alex_.nym_id_);
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->DisplayName(), bob_.name_);
    ASSERT_TRUE(row->Valid());
    EXPECT_STREQ("", row->ImageURI().c_str());
    EXPECT_STREQ("B", row->Section().c_str());
    EXPECT_FALSE(row->Last());

    row = widget.Next();

    EXPECT_EQ(row->DisplayName(), issuer_.name_);
    ASSERT_TRUE(row->Valid());
    EXPECT_STREQ("", row->ImageURI().c_str());
    EXPECT_STREQ("I", row->Section().c_str());
    EXPECT_TRUE(row->Last());
}

TEST_F(Integration, payable_list_bch_alex_3)
{
    if (have_hd_) {
        ASSERT_TRUE(wait_for_counter(payable_list_bch_alex_));

        const auto& widget = alex_.api_->UI().Internal().PayableList(
            alex_.nym_id_, ot::UnitType::Bch);
        auto row = widget.First();

        ASSERT_TRUE(row->Valid());
        EXPECT_EQ(row->DisplayName(), alex_.name_);
        EXPECT_FALSE(row->Last());

        row = widget.Next();

        EXPECT_EQ(row->DisplayName(), bob_.name_);
        EXPECT_FALSE(row->Last());

        row = widget.Next();

        EXPECT_EQ(row->DisplayName(), issuer_.name_);
        EXPECT_TRUE(row->Last());
    }
}

TEST_F(Integration, payable_list_btc_alex_2)
{
    if (have_hd_) {
        ASSERT_TRUE(wait_for_counter(payable_list_bch_alex_));

        const auto& widget = alex_.api_->UI().Internal().PayableList(
            alex_.nym_id_, ot::UnitType::Btc);
        auto row = widget.First();

        ASSERT_TRUE(row->Valid());
        EXPECT_EQ(row->DisplayName(), alex_.name_);
        EXPECT_FALSE(row->Last());

        row = widget.Next();

        EXPECT_EQ(row->DisplayName(), bob_.name_);
        EXPECT_FALSE(row->Last());

        row = widget.Next();

        EXPECT_EQ(row->DisplayName(), issuer_.name_);
        EXPECT_TRUE(row->Last());
    }
}

TEST_F(Integration, issuer_claims)
{
    const auto pNym = api_alex_.Wallet().Nym(issuer_.nym_id_);

    ASSERT_TRUE(pNym);

    const auto& nym = *pNym;
    const auto& claims = nym.Claims();
    const auto pSection =
        claims.Section(ot::identity::wot::claim::SectionType::Contract);

    ASSERT_TRUE(pSection);

    const auto& section = *pSection;
    const auto pGroup = section.Group(ot::identity::wot::claim::ClaimType::Usd);

    ASSERT_TRUE(pGroup);

    const auto& group = *pGroup;
    const auto& pClaim = group.PrimaryClaim();

    EXPECT_EQ(1, group.Size());
    ASSERT_TRUE(pClaim);

    const auto& claim = *pClaim;

    EXPECT_EQ(claim.Value(), unit_id_.asBase58(api_alex_.Crypto()));
}

TEST_F(Integration, deposit_cheque_alex)
{
    contact_issuer_alex_.expected_ += 8;
    contact_activity_issuer_alex_.expected_ += 3;
    account_list_alex_.expected_ += 2;

    api_alex_.UI().Internal().Contact(
        alex_.Contact(issuer_.name_),
        make_cb(contact_issuer_alex_, "alex's contact for issuer"));
    const auto& thread = alex_.api_->UI().Internal().ContactActivity(
        alex_.nym_id_,
        alex_.Contact(issuer_.name_),
        make_cb(
            contact_activity_issuer_alex_,
            "Alex's activity thread with issuer"));

    ASSERT_TRUE(wait_for_counter(contact_activity_issuer_alex_));

    auto row = thread.First();

    ASSERT_TRUE(row->Valid());

    idle();

    EXPECT_TRUE(row->Deposit());

    idle();
}

TEST_F(Integration, account_list_alex_1)
{
    ASSERT_TRUE(wait_for_counter(account_list_alex_));

    const auto& widget = alex_.api_->UI().Internal().AccountList(alex_.nym_id_);
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());

    alex_.SetAccount(UNIT_DEFINITION_TLA, row->AccountID());

    EXPECT_FALSE(alex_.Account(UNIT_DEFINITION_TLA).empty());
    EXPECT_EQ(unit_id_.asBase58(api_alex_.Crypto()), row->ContractID());
    EXPECT_STREQ("dollars 1.00", row->DisplayBalance().c_str());
    EXPECT_STREQ("", row->Name().c_str());
    EXPECT_EQ(server_1_.id_.asBase58(api_alex_.Crypto()), row->NotaryID());
    EXPECT_EQ(server_1_.Contract()->EffectiveName(), row->NotaryName());
    EXPECT_EQ(ot::AccountType::Custodial, row->Type());
    EXPECT_EQ(ot::UnitType::Usd, row->Unit());
}

TEST_F(Integration, contact_activity_issuer_alex_0)
{
    ASSERT_TRUE(wait_for_counter(contact_activity_issuer_alex_));

    const auto& widget = alex_.api_->UI().Internal().ContactActivity(
        alex_.nym_id_, alex_.Contact(issuer_.name_));
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());

    bool loading{true};

    // This allows the test to work correctly in valgrind when
    // loading is unusually slow
    while (loading) {
        row = widget.First();
        loading = row->Loading();
    }

    EXPECT_EQ(CHEQUE_AMOUNT_1, row->Amount());
    EXPECT_FALSE(row->Loading());
    EXPECT_STREQ(CHEQUE_MEMO, row->Memo().c_str());
    EXPECT_FALSE(row->Pending());
    EXPECT_STREQ("Received cheque for dollars 1.00", row->Text().c_str());
    EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
    EXPECT_EQ(ot::otx::client::StorageBox::INCOMINGCHEQUE, row->Type());
    EXPECT_TRUE(row->Last());
}

TEST_F(Integration, contact_issuer_alex_0)
{
    ASSERT_TRUE(wait_for_counter(contact_issuer_alex_));

    const auto& widget =
        api_alex_.UI().Internal().Contact(alex_.Contact(issuer_.name_));

    EXPECT_EQ(
        alex_.Contact(issuer_.name_).asBase58(api_alex_.Crypto()),
        widget.ContactID());
    EXPECT_EQ(ot::UnallocatedCString(issuer_.name_), widget.DisplayName());

    if (have_hd_) { EXPECT_EQ(issuer_.payment_code_, widget.PaymentCode()); }

    auto row = widget.First();

    ASSERT_TRUE(row->Valid());

    // TODO
}

TEST_F(Integration, account_activity_usd_alex_0)
{
    account_activity_usd_alex_.expected_ += 1;

    const auto& widget = api_alex_.UI().Internal().AccountActivity(
        alex_.nym_id_,
        alex_.Account(UNIT_DEFINITION_TLA),
        make_cb(account_activity_usd_alex_, "alex account activity (USD)"));

    ASSERT_TRUE(wait_for_counter(account_activity_usd_alex_));

    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(CHEQUE_AMOUNT_1, row->Amount());
    EXPECT_EQ(1, row->Contacts().size());

    if (0 < row->Contacts().size()) {
        EXPECT_EQ(
            alex_.Contact(issuer_.name_).asBase58(api_alex_.Crypto()),
            *row->Contacts().begin());
    }

    EXPECT_EQ("dollars 1.00", row->DisplayAmount());
    EXPECT_EQ(CHEQUE_MEMO, row->Memo());
    EXPECT_FALSE(row->Workflow().empty());
    EXPECT_EQ("Received cheque #510 from Issuer", row->Text());
    EXPECT_EQ(ot::otx::client::StorageBox::INCOMINGCHEQUE, row->Type());
    EXPECT_FALSE(row->UUID().empty());
    EXPECT_TRUE(row->Last());
}

TEST_F(Integration, process_inbox_issuer)
{
    auto task = api_issuer_.OTX().ProcessInbox(
        issuer_.nym_id_, server_1_.id_, issuer_.Account(UNIT_DEFINITION_TLA));
    auto& [id, future] = task;

    ASSERT_NE(0, id);

    const auto [status, message] = future.get();

    EXPECT_EQ(ot::otx::LastReplyStatus::MessageSuccess, status);
    ASSERT_TRUE(message);

    const auto account = api_issuer_.Wallet().Internal().Account(
        issuer_.Account(UNIT_DEFINITION_TLA));

    EXPECT_EQ(-1 * CHEQUE_AMOUNT_1, account.get().GetBalance());
}

TEST_F(Integration, pay_bob)
{
    account_activity_usd_alex_.expected_ += 1;
    activity_summary_alex_.expected_ += 2;
    contact_activity_bob_alex_.expected_ += 6;
    contact_issuer_alex_.expected_ += 4;
    contact_activity_alex_bob_.expected_ += 4;
    activity_summary_bob_.expected_ += 2;

    const auto& thread = api_alex_.UI().Internal().ContactActivity(
        alex_.nym_id_, alex_.Contact(bob_.name_));
    idle();
    const auto sent = thread.Pay(
        CHEQUE_AMOUNT_2,
        alex_.Account(UNIT_DEFINITION_TLA),
        CHEQUE_MEMO,
        ot::otx::client::PaymentType::Cheque);

    EXPECT_TRUE(sent);

    idle();
}

TEST_F(Integration, account_activity_usd_alex_1)
{
    ASSERT_TRUE(wait_for_counter(account_activity_usd_alex_));

    const auto& widget = alex_.api_->UI().Internal().AccountActivity(
        alex_.nym_id_, alex_.Account(UNIT_DEFINITION_TLA));
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(-1 * CHEQUE_AMOUNT_2, row->Amount());
    EXPECT_EQ(1, row->Contacts().size());

    if (0 < row->Contacts().size()) {
        EXPECT_EQ(
            alex_.Contact(bob_.name_).asBase58(api_alex_.Crypto()),
            *row->Contacts().begin());
    }

    EXPECT_EQ("-dollars 0.75", row->DisplayAmount());
    EXPECT_EQ(CHEQUE_MEMO, row->Memo());
    EXPECT_FALSE(row->Workflow().empty());
    EXPECT_EQ("Wrote cheque #721 for Bob", row->Text());
    EXPECT_EQ(ot::otx::client::StorageBox::OUTGOINGCHEQUE, row->Type());
    EXPECT_FALSE(row->UUID().empty());
    EXPECT_FALSE(row->Last());

    row = widget.Next();

    EXPECT_EQ(CHEQUE_AMOUNT_1, row->Amount());
    EXPECT_EQ(1, row->Contacts().size());

    if (0 < row->Contacts().size()) {
        EXPECT_EQ(
            alex_.Contact(issuer_.name_).asBase58(api_alex_.Crypto()),
            *row->Contacts().begin());
    }

    EXPECT_EQ("dollars 1.00", row->DisplayAmount());
    EXPECT_EQ(CHEQUE_MEMO, row->Memo());
    EXPECT_FALSE(row->Workflow().empty());
    EXPECT_EQ("Received cheque #510 from Issuer", row->Text());
    EXPECT_EQ(ot::otx::client::StorageBox::INCOMINGCHEQUE, row->Type());
    EXPECT_FALSE(row->UUID().empty());
    EXPECT_TRUE(row->Last());
}

TEST_F(Integration, activity_summary_alex_4)
{
    ASSERT_TRUE(wait_for_counter(activity_summary_alex_));

    const auto& widget =
        alex_.api_->UI().Internal().ActivitySummary(alex_.nym_id_);
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->DisplayName(), bob_.name_);
    EXPECT_EQ(row->ImageURI(), "");
    EXPECT_EQ(row->Text(), "Sent cheque for dollars 0.75");
    EXPECT_FALSE(row->ThreadID().empty());
    EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
    EXPECT_EQ(row->Type(), ot::otx::client::StorageBox::OUTGOINGCHEQUE);
    EXPECT_FALSE(row->Last());

    row = widget.Next();

    EXPECT_EQ(row->DisplayName(), issuer_.name_);
    EXPECT_STREQ("", row->ImageURI().c_str());
    EXPECT_STREQ("Received cheque", row->Text().c_str());
    EXPECT_FALSE(row->ThreadID().empty());
    EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
    EXPECT_EQ(ot::otx::client::StorageBox::INCOMINGCHEQUE, row->Type());
}

TEST_F(Integration, contact_activity_bob_alex_3)
{
    ASSERT_TRUE(wait_for_counter(contact_activity_bob_alex_));

    const auto& firstMessage = message_[msg_count_ - 1];
    const auto& secondMessage = message_[msg_count_];
    const auto& widget = alex_.api_->UI().Internal().ContactActivity(
        alex_.nym_id_, alex_.Contact(bob_.name_));
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->Amount(), 0);
    EXPECT_EQ(row->DisplayAmount(), "");
    EXPECT_FALSE(row->Loading());
    EXPECT_EQ(row->Memo(), "");
    EXPECT_FALSE(row->Pending());
    EXPECT_EQ(row->Text(), firstMessage);
    EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
    EXPECT_EQ(row->Type(), ot::otx::client::StorageBox::MAILOUTBOX);
    EXPECT_FALSE(row->Last());

    row = widget.Next();

    EXPECT_EQ(row->Amount(), 0);
    EXPECT_EQ(row->DisplayAmount(), "");
    EXPECT_FALSE(row->Loading());
    EXPECT_EQ(row->Memo(), "");
    EXPECT_FALSE(row->Pending());
    EXPECT_EQ(row->Text(), secondMessage);
    EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
    EXPECT_EQ(row->Type(), ot::otx::client::StorageBox::MAILINBOX);
    EXPECT_FALSE(row->Last());

    row = widget.Next();

    EXPECT_EQ(row->Amount(), CHEQUE_AMOUNT_2);
    EXPECT_EQ(row->DisplayAmount(), "dollars 0.75");
    EXPECT_FALSE(row->Loading());
    EXPECT_EQ(row->Memo(), CHEQUE_MEMO);
    EXPECT_FALSE(row->Pending());
    EXPECT_EQ(row->Text(), "Sent cheque for dollars 0.75");
    EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
    EXPECT_EQ(row->Type(), ot::otx::client::StorageBox::OUTGOINGCHEQUE);
    EXPECT_TRUE(row->Last());
}

TEST_F(Integration, contact_issuer_alex_1)
{
    ASSERT_TRUE(wait_for_counter(contact_issuer_alex_));

    const auto& widget =
        api_alex_.UI().Internal().Contact(alex_.Contact(issuer_.name_));

    EXPECT_EQ(
        alex_.Contact(issuer_.name_).asBase58(api_alex_.Crypto()),
        widget.ContactID());
    EXPECT_EQ(ot::UnallocatedCString(issuer_.name_), widget.DisplayName());

    if (have_hd_) { EXPECT_EQ(issuer_.payment_code_, widget.PaymentCode()); }

    auto row = widget.First();

    ASSERT_TRUE(row->Valid());

    // TODO
}

TEST_F(Integration, contact_activity_alex_bob_2)
{
    ASSERT_TRUE(wait_for_counter(contact_activity_alex_bob_));

    const auto& firstMessage = message_[msg_count_ - 1];
    const auto& secondMessage = message_[msg_count_];
    const auto& widget = bob_.api_->UI().Internal().ContactActivity(
        bob_.nym_id_, bob_.Contact(alex_.name_));
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->Amount(), 0);
    EXPECT_EQ(row->DisplayAmount(), "");
    EXPECT_FALSE(row->Loading());
    EXPECT_EQ(row->Memo(), "");
    EXPECT_FALSE(row->Pending());
    EXPECT_EQ(row->Text(), firstMessage);
    EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
    EXPECT_EQ(row->Type(), ot::otx::client::StorageBox::MAILINBOX);
    EXPECT_FALSE(row->Last());

    row = widget.Next();
    bool loading{true};

    EXPECT_EQ(row->Amount(), 0);
    EXPECT_EQ(row->DisplayAmount(), "");
    EXPECT_FALSE(row->Loading());
    EXPECT_EQ(row->Memo(), "");
    EXPECT_FALSE(row->Pending());
    EXPECT_EQ(row->Text(), secondMessage);
    EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
    EXPECT_EQ(row->Type(), ot::otx::client::StorageBox::MAILOUTBOX);
    EXPECT_FALSE(row->Last());

    // This allows the test to work correctly in valgrind when
    // loading is unusually slow
    while (loading) {
        row = widget.First();
        row = widget.Next();
        row = widget.Next();
        loading = row->Loading();
    }

    EXPECT_EQ(row->Amount(), CHEQUE_AMOUNT_2);
    EXPECT_EQ(row->DisplayAmount(), "");
    EXPECT_FALSE(row->Loading());
    EXPECT_EQ(row->Memo(), CHEQUE_MEMO);
    EXPECT_FALSE(row->Pending());
    EXPECT_EQ(row->Text(), "Received cheque");
    EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
    EXPECT_EQ(row->Type(), ot::otx::client::StorageBox::INCOMINGCHEQUE);
    EXPECT_TRUE(row->Last());
}

TEST_F(Integration, activity_summary_bob_3)
{
    ASSERT_TRUE(wait_for_counter(activity_summary_bob_));

    const auto& widget =
        bob_.api_->UI().Internal().ActivitySummary(bob_.nym_id_);
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->DisplayName(), alex_.name_);
    EXPECT_EQ(row->ImageURI(), "");
    EXPECT_EQ(row->Text(), "Received cheque");
    EXPECT_FALSE(row->ThreadID().empty());
    EXPECT_LT(0, ot::Clock::to_time_t(row->Timestamp()));
    EXPECT_EQ(row->Type(), ot::otx::client::StorageBox::INCOMINGCHEQUE);
    EXPECT_TRUE(row->Last());
}

TEST_F(Integration, shutdown)
{
    idle();

    EXPECT_EQ(
        account_activity_usd_alex_.expected_,
        account_activity_usd_alex_.updated_);
    EXPECT_EQ(account_list_alex_.expected_, account_list_alex_.updated_);
    EXPECT_EQ(
        account_summary_bch_alex_.expected_,
        account_summary_bch_alex_.updated_);
    EXPECT_EQ(
        account_summary_btc_alex_.expected_,
        account_summary_btc_alex_.updated_);
    EXPECT_EQ(
        account_summary_usd_alex_.expected_,
        account_summary_usd_alex_.updated_);
    EXPECT_EQ(
        activity_summary_alex_.expected_, activity_summary_alex_.updated_);
    EXPECT_EQ(
        contact_activity_bob_alex_.expected_,
        contact_activity_bob_alex_.updated_);
    EXPECT_EQ(
        contact_activity_issuer_alex_.expected_,
        contact_activity_issuer_alex_.updated_);
    EXPECT_EQ(contact_issuer_alex_.expected_, contact_issuer_alex_.updated_);
    EXPECT_EQ(contact_list_alex_.expected_, contact_list_alex_.updated_);
    EXPECT_EQ(messagable_list_alex_.expected_, messagable_list_alex_.updated_);
    EXPECT_EQ(
        payable_list_bch_alex_.expected_, payable_list_bch_alex_.updated_);
    EXPECT_EQ(
        payable_list_btc_alex_.expected_, payable_list_btc_alex_.updated_);
    EXPECT_EQ(profile_alex_.expected_, profile_alex_.updated_);

    EXPECT_EQ(
        account_activity_usd_bob_.expected_,
        account_activity_usd_bob_.updated_);
    EXPECT_EQ(account_list_bob_.expected_, account_list_bob_.updated_);
    EXPECT_EQ(
        account_summary_bch_bob_.expected_, account_summary_bch_bob_.updated_);
    EXPECT_EQ(
        account_summary_btc_bob_.expected_, account_summary_btc_bob_.updated_);
    EXPECT_EQ(
        account_summary_usd_bob_.expected_, account_summary_usd_bob_.updated_);
    EXPECT_EQ(activity_summary_bob_.expected_, activity_summary_bob_.updated_);
    EXPECT_EQ(
        contact_activity_alex_bob_.expected_,
        contact_activity_alex_bob_.updated_);
    EXPECT_EQ(contact_list_bob_.expected_, contact_list_bob_.updated_);
    EXPECT_EQ(messagable_list_bob_.expected_, messagable_list_bob_.updated_);
    EXPECT_EQ(payable_list_bch_bob_.expected_, payable_list_bch_bob_.updated_);
    EXPECT_EQ(payable_list_btc_bob_.expected_, payable_list_btc_bob_.updated_);
    EXPECT_EQ(profile_bob_.expected_, profile_bob_.updated_);
}
}  // namespace ottest
