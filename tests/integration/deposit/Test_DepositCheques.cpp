// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"

#include <gtest/gtest.h>

#include <mutex>

using namespace opentxs;

#define ALICE "Alice"
#define BOB "Bob"
#define ISSUER "Issuer"
#define ACCOUNT_ACTIVITY_USD "ACCOUNT_ACTIVITY_USD"
#define ACCOUNT_LIST "ACCOUNT_LIST"
#define ACCOUNT_SUMMARY_BTC "ACCOUNT_SUMMARY_BTC"
#define ACCOUNT_SUMMARY_BCH "ACCOUNT_SUMMARY_BCH"
#define ACCOUNT_SUMMARY_USD "ACCOUNT_SUMMARY_USD"
#define ACTIVITY_SUMMARY "ACTIVITY_SUMMARY"
#define ACTIVITY_THREAD_ALICE_BOB "ACTIVITY_THREAD_ALICE_BOB"
#define ACTIVITY_THREAD_ALICE_ISSUER "ACTIVITY_THREAD_ALICE_ISSUER"
#define ACTIVITY_THREAD_BOB_ALICE "ACTIVITY_THREAD_BOB_ALICE"
#define CONTACT_LIST "CONTACT_LIST"
#define CONTACT_ALICE_ISSUER "CONTACT_ALICE_ISSUER"
#define MESSAGABLE_LIST "MESSAGAGABLE_LIST"
#define PAYABLE_LIST_BTC "PAYABLE_LIST_BTC"
#define PAYABLE_LIST_BCH "PAYABLE_LIST_BCH"
#define PROFILE "PROFILE"

#define UNIT_DEFINITION_CONTRACT_NAME "Mt Gox USD"
#define UNIT_DEFINITION_TERMS "YOLO"
#define UNIT_DEFINITION_PRIMARY_UNIT_NAME "dollars"
#define UNIT_DEFINITION_SYMBOL "$"
#define UNIT_DEFINITION_TLA "USD"
#define UNIT_DEFINITION_POWER 2
#define UNIT_DEFINITION_FRACTIONAL_UNIT_NAME "cents"
#define CHEQUE_AMOUNT_1 2000
#define CHEQUE_MEMO_1 "memo"

#define OT_METHOD "::Test_DepositCheques::"

namespace
{
bool init_{false};

class Test_DepositCheques : public ::testing::Test
{
public:
    static const opentxs::ArgList args_;
    static const std::string SeedA_;
    static const std::string SeedB_;
    static const std::string SeedC_;
    static const std::string Alice_;
    static const std::string Bob_;
    static const std::string Issuer_;
    static const OTNymID alice_nym_id_;
    static const OTNymID bob_nym_id_;
    static const OTNymID issuer_nym_id_;
    static OTIdentifier contact_id_alice_bob_;
    static OTIdentifier contact_id_alice_issuer_;
    static OTIdentifier contact_id_bob_alice_;
    static OTIdentifier contact_id_bob_issuer_;
    static OTIdentifier contact_id_issuer_alice_;
    static OTIdentifier contact_id_issuer_bob_;
    static const std::shared_ptr<const ServerContract> server_contract_;
    static const OTServerID server_1_id_;

    static const opentxs::api::client::Manager* alice_;
    static const opentxs::api::client::Manager* bob_;

    static std::string alice_payment_code_;
    static std::string bob_payment_code_;
    static std::string issuer_payment_code_;

    static OTUnitID unit_id_;
    static OTIdentifier alice_account_id_;
    static OTIdentifier issuer_account_id_;

    const opentxs::api::client::Manager& alice_client_;
    const opentxs::api::client::Manager& bob_client_;
    const opentxs::api::server::Manager& server_1_;
    const opentxs::api::client::Manager& issuer_client_;

    Test_DepositCheques()
        : alice_client_(OT::App().StartClient(args_, 0))
        , bob_client_(OT::App().StartClient(args_, 1))
        , server_1_(OT::App().StartServer(args_, 0, true))
        , issuer_client_(OT::App().StartClient(args_, 2))
    {
#if OT_CASH
        server_1_.SetMintKeySize(OT_MINT_KEY_SIZE_TEST);
#endif

        if (false == init_) { init(); }
    }

    void import_server_contract(
        const ServerContract& contract,
        const opentxs::api::client::Manager& client)
    {
        auto clientVersion =
            client.Wallet().Server(server_contract_->PublicContract());

        OT_ASSERT(clientVersion)

        client.OTX().SetIntroductionServer(*clientVersion);
    }

    void init()
    {
        const_cast<std::string&>(SeedA_) =
            alice_client_.Exec().Wallet_ImportSeed(
                "spike nominee miss inquiry fee nothing belt list other "
                "daughter leave valley twelve gossip paper",
                "");
        const_cast<std::string&>(SeedB_) = bob_client_.Exec().Wallet_ImportSeed(
            "trim thunder unveil reduce crop cradle zone inquiry "
            "anchor skate property fringe obey butter text tank drama "
            "palm guilt pudding laundry stay axis prosper",
            "");
        const_cast<std::string&>(SeedC_) =
            issuer_client_.Exec().Wallet_ImportSeed(
                "abandon abandon abandon abandon abandon abandon abandon "
                "abandon abandon abandon abandon about",
                "");
        const_cast<std::string&>(Alice_) = alice_client_.Exec().CreateNymHD(
            proto::CITEMTYPE_INDIVIDUAL, ALICE, SeedA_, 0);
        const_cast<std::string&>(Bob_) = bob_client_.Exec().CreateNymHD(
            proto::CITEMTYPE_INDIVIDUAL, BOB, SeedB_, 0);
        const_cast<std::string&>(Issuer_) = issuer_client_.Exec().CreateNymHD(
            proto::CITEMTYPE_INDIVIDUAL, ISSUER, SeedC_, 0);
        const_cast<OTNymID&>(alice_nym_id_) = identifier::Nym::Factory(Alice_);
        const_cast<OTNymID&>(bob_nym_id_) = identifier::Nym::Factory(Bob_);
        const_cast<OTNymID&>(issuer_nym_id_) =
            identifier::Nym::Factory(Issuer_);
        const_cast<OTServerID&>(server_1_id_) =
            identifier::Server::Factory(server_1_.ID().str());
        const_cast<std::shared_ptr<const ServerContract>&>(server_contract_) =
            server_1_.Wallet().Server(server_1_id_);

        OT_ASSERT(server_contract_);
        OT_ASSERT(false == server_1_id_->empty());

        import_server_contract(*server_contract_, alice_client_);
        import_server_contract(*server_contract_, bob_client_);
        import_server_contract(*server_contract_, issuer_client_);

        alice_ = &alice_client_;
        bob_ = &bob_client_;

        init_ = true;
    }
};

const opentxs::ArgList Test_DepositCheques::args_{
    {{OPENTXS_ARG_STORAGE_PLUGIN, {"mem"}}}};
const std::string Test_DepositCheques::SeedA_{""};
const std::string Test_DepositCheques::SeedB_{""};
const std::string Test_DepositCheques::SeedC_{""};
const std::string Test_DepositCheques::Alice_{""};
const std::string Test_DepositCheques::Bob_{""};
const std::string Test_DepositCheques::Issuer_{""};
const OTNymID Test_DepositCheques::alice_nym_id_{identifier::Nym::Factory()};
const OTNymID Test_DepositCheques::bob_nym_id_{identifier::Nym::Factory()};
const OTNymID Test_DepositCheques::issuer_nym_id_{identifier::Nym::Factory()};
OTIdentifier Test_DepositCheques::contact_id_alice_bob_{Identifier::Factory()};
OTIdentifier Test_DepositCheques::contact_id_alice_issuer_{
    Identifier::Factory()};
OTIdentifier Test_DepositCheques::contact_id_bob_alice_{Identifier::Factory()};
OTIdentifier Test_DepositCheques::contact_id_bob_issuer_{Identifier::Factory()};
OTIdentifier Test_DepositCheques::contact_id_issuer_alice_{
    Identifier::Factory()};
OTIdentifier Test_DepositCheques::contact_id_issuer_bob_{Identifier::Factory()};
const std::shared_ptr<const ServerContract>
    Test_DepositCheques::server_contract_{nullptr};
const OTServerID Test_DepositCheques::server_1_id_{
    identifier::Server::Factory()};
const opentxs::api::client::Manager* Test_DepositCheques::alice_{nullptr};
const opentxs::api::client::Manager* Test_DepositCheques::bob_{nullptr};
std::string Test_DepositCheques::alice_payment_code_;
std::string Test_DepositCheques::bob_payment_code_;
std::string Test_DepositCheques::issuer_payment_code_;
OTUnitID Test_DepositCheques::unit_id_{identifier::UnitDefinition::Factory()};
OTIdentifier Test_DepositCheques::alice_account_id_{Identifier::Factory()};
OTIdentifier Test_DepositCheques::issuer_account_id_{Identifier::Factory()};

TEST_F(Test_DepositCheques, payment_codes)
{
    auto alice = alice_client_.Wallet().mutable_Nym(alice_nym_id_);
    auto bob = bob_client_.Wallet().mutable_Nym(bob_nym_id_);
    auto issuer = issuer_client_.Wallet().mutable_Nym(issuer_nym_id_);

    EXPECT_EQ(opentxs::proto::CITEMTYPE_INDIVIDUAL, alice.Type());
    EXPECT_EQ(opentxs::proto::CITEMTYPE_INDIVIDUAL, bob.Type());
    EXPECT_EQ(opentxs::proto::CITEMTYPE_INDIVIDUAL, issuer.Type());

    auto aliceScopeSet =
        alice.SetScope(opentxs::proto::CITEMTYPE_INDIVIDUAL, ALICE, true);
    auto bobScopeSet = bob.SetScope(proto::CITEMTYPE_INDIVIDUAL, BOB, true);
    auto issuerScopeSet =
        issuer.SetScope(proto::CITEMTYPE_INDIVIDUAL, ISSUER, true);

    EXPECT_TRUE(aliceScopeSet);
    EXPECT_TRUE(bobScopeSet);
    EXPECT_TRUE(issuerScopeSet);

    alice_payment_code_ =
        alice_client_.Factory().PaymentCode(SeedA_, 0, 1)->asBase58();
    bob_payment_code_ =
        bob_client_.Factory().PaymentCode(SeedB_, 0, 1)->asBase58();
    issuer_payment_code_ =
        issuer_client_.Factory().PaymentCode(SeedC_, 0, 1)->asBase58();

    EXPECT_FALSE(alice_payment_code_.empty());
    EXPECT_FALSE(bob_payment_code_.empty());
    EXPECT_FALSE(issuer_payment_code_.empty());

    alice.AddPaymentCode(
        alice_payment_code_, opentxs::proto::CITEMTYPE_BTC, true, true);
    bob.AddPaymentCode(
        bob_payment_code_, opentxs::proto::CITEMTYPE_BTC, true, true);
    issuer.AddPaymentCode(
        issuer_payment_code_, opentxs::proto::CITEMTYPE_BTC, true, true);
    alice.AddPaymentCode(
        alice_payment_code_, opentxs::proto::CITEMTYPE_BCH, true, true);
    bob.AddPaymentCode(
        bob_payment_code_, opentxs::proto::CITEMTYPE_BCH, true, true);
    issuer.AddPaymentCode(
        issuer_payment_code_, opentxs::proto::CITEMTYPE_BCH, true, true);

    EXPECT_FALSE(alice.PaymentCode(proto::CITEMTYPE_BTC).empty());
    EXPECT_FALSE(bob.PaymentCode(proto::CITEMTYPE_BTC).empty());
    EXPECT_FALSE(issuer.PaymentCode(proto::CITEMTYPE_BTC).empty());
    EXPECT_FALSE(alice.PaymentCode(proto::CITEMTYPE_BCH).empty());
    EXPECT_FALSE(bob.PaymentCode(proto::CITEMTYPE_BCH).empty());
    EXPECT_FALSE(issuer.PaymentCode(proto::CITEMTYPE_BCH).empty());

    alice.Release();
    bob.Release();
    issuer.Release();
}

TEST_F(Test_DepositCheques, introduction_server)
{
    alice_client_.OTX().StartIntroductionServer(alice_nym_id_);
    bob_client_.OTX().StartIntroductionServer(bob_nym_id_);
    auto task1 = alice_client_.OTX().RegisterNymPublic(
        alice_nym_id_, server_1_id_, true);
    auto task2 =
        bob_client_.OTX().RegisterNymPublic(bob_nym_id_, server_1_id_, true);

    ASSERT_NE(0, task1.first);
    ASSERT_NE(0, task2.first);
    EXPECT_EQ(proto::LASTREPLYSTATUS_MESSAGESUCCESS, task1.second.get().first);
    EXPECT_EQ(proto::LASTREPLYSTATUS_MESSAGESUCCESS, task2.second.get().first);

    alice_client_.OTX().ContextIdle(alice_nym_id_, server_1_id_).get();
    bob_client_.OTX().ContextIdle(bob_nym_id_, server_1_id_).get();
}

TEST_F(Test_DepositCheques, add_contacts)
{
    const auto aliceBob = alice_client_.Contacts().NewContact(
        BOB,
        bob_nym_id_,
        alice_client_.Factory().PaymentCode(bob_payment_code_));
    const auto aliceIssuer = alice_client_.Contacts().NewContact(
        ISSUER,
        issuer_nym_id_,
        alice_client_.Factory().PaymentCode(issuer_payment_code_));
    const auto bobAlice = bob_client_.Contacts().NewContact(
        ALICE,
        alice_nym_id_,
        bob_client_.Factory().PaymentCode(alice_payment_code_));
    const auto bobIssuer = bob_client_.Contacts().NewContact(
        ISSUER,
        issuer_nym_id_,
        bob_client_.Factory().PaymentCode(issuer_payment_code_));
    const auto issuerAlice = issuer_client_.Contacts().NewContact(
        ALICE,
        alice_nym_id_,
        issuer_client_.Factory().PaymentCode(alice_payment_code_));
    const auto issuerBob = issuer_client_.Contacts().NewContact(
        BOB,
        bob_nym_id_,
        issuer_client_.Factory().PaymentCode(bob_payment_code_));

    ASSERT_TRUE(aliceBob);
    ASSERT_TRUE(aliceIssuer);
    ASSERT_TRUE(bobAlice);
    ASSERT_TRUE(bobIssuer);
    ASSERT_TRUE(issuerAlice);
    ASSERT_TRUE(issuerBob);

    contact_id_alice_bob_ = aliceBob->ID();
    contact_id_alice_issuer_ = aliceIssuer->ID();
    contact_id_bob_alice_ = bobAlice->ID();
    contact_id_bob_issuer_ = bobIssuer->ID();
    contact_id_issuer_alice_ = issuerAlice->ID();
    contact_id_issuer_bob_ = issuerAlice->ID();

    EXPECT_TRUE(alice_client_.Wallet().Nym(
        bob_client_.Wallet().Nym(bob_nym_id_)->asPublicNym()));
    EXPECT_TRUE(alice_client_.Wallet().Nym(
        issuer_client_.Wallet().Nym(issuer_nym_id_)->asPublicNym()));
    EXPECT_TRUE(bob_client_.Wallet().Nym(
        alice_client_.Wallet().Nym(alice_nym_id_)->asPublicNym()));
    EXPECT_TRUE(bob_client_.Wallet().Nym(
        issuer_client_.Wallet().Nym(issuer_nym_id_)->asPublicNym()));
    EXPECT_TRUE(issuer_client_.Wallet().Nym(
        alice_client_.Wallet().Nym(alice_nym_id_)->asPublicNym()));
    EXPECT_TRUE(issuer_client_.Wallet().Nym(
        bob_client_.Wallet().Nym(bob_nym_id_)->asPublicNym()));
}

TEST_F(Test_DepositCheques, issue_dollars)
{
    const auto contract = issuer_client_.Wallet().UnitDefinition(
        issuer_nym_id_->str(),
        UNIT_DEFINITION_CONTRACT_NAME,
        UNIT_DEFINITION_TERMS,
        UNIT_DEFINITION_PRIMARY_UNIT_NAME,
        UNIT_DEFINITION_SYMBOL,
        UNIT_DEFINITION_TLA,
        UNIT_DEFINITION_POWER,
        UNIT_DEFINITION_FRACTIONAL_UNIT_NAME);

    ASSERT_TRUE(contract);
    EXPECT_EQ(proto::UNITTYPE_CURRENCY, contract->Type());
    EXPECT_TRUE(unit_id_->empty());

    unit_id_->Assign(contract->ID());

    EXPECT_FALSE(unit_id_->empty());

    {
        auto issuer = issuer_client_.Wallet().mutable_Nym(issuer_nym_id_);
        issuer.AddPreferredOTServer(server_1_id_->str(), true);
        issuer.AddContract(unit_id_->str(), proto::CITEMTYPE_USD, true, true);
    }

    auto task = issuer_client_.OTX().IssueUnitDefinition(
        issuer_nym_id_, server_1_id_, unit_id_);
    auto& [taskID, future] = task;
    const auto result = future.get();

    EXPECT_NE(0, taskID);
    EXPECT_EQ(proto::LASTREPLYSTATUS_MESSAGESUCCESS, result.first);
    ASSERT_TRUE(result.second);

    issuer_account_id_->SetString(result.second->m_strAcctID);

    EXPECT_FALSE(issuer_account_id_->empty());

    issuer_client_.OTX().ContextIdle(issuer_nym_id_, server_1_id_).get();
}

TEST_F(Test_DepositCheques, pay_alice)
{
    auto task = issuer_client_.OTX().SendCheque(
        issuer_nym_id_,
        issuer_account_id_,
        contact_id_issuer_alice_,
        CHEQUE_AMOUNT_1,
        CHEQUE_MEMO_1);
    auto& [taskID, future] = task;

    ASSERT_NE(0, taskID);
    EXPECT_EQ(proto::LASTREPLYSTATUS_MESSAGESUCCESS, future.get().first);

    issuer_client_.OTX().ContextIdle(issuer_nym_id_, server_1_id_).get();
}

TEST_F(Test_DepositCheques, accept_cheque_alice)
{
    const auto count = alice_client_.OTX().DepositCheques(alice_nym_id_);

    EXPECT_EQ(1, count);

    alice_client_.OTX().ContextIdle(alice_nym_id_, server_1_id_).get();
    issuer_client_.OTX().ContextIdle(issuer_nym_id_, server_1_id_).get();
}

TEST_F(Test_DepositCheques, process_inbox_issuer)
{
    auto task = issuer_client_.OTX().ProcessInbox(
        issuer_nym_id_, server_1_id_, issuer_account_id_);
    auto& [id, future] = task;

    ASSERT_NE(0, id);

    const auto [status, message] = future.get();

    EXPECT_EQ(proto::LASTREPLYSTATUS_MESSAGESUCCESS, status);
    ASSERT_TRUE(message);

    const auto account = issuer_client_.Wallet().Account(issuer_account_id_);

    EXPECT_EQ(-1 * CHEQUE_AMOUNT_1, account.get().GetBalance());
}

TEST_F(Test_DepositCheques, shutdown)
{
    alice_client_.OTX().ContextIdle(alice_nym_id_, server_1_id_).get();
    bob_client_.OTX().ContextIdle(bob_nym_id_, server_1_id_).get();
    issuer_client_.OTX().ContextIdle(issuer_nym_id_, server_1_id_).get();
}
}  // namespace