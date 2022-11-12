// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <future>
#include <memory>
#include <utility>

#include "internal/api/session/Client.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/String.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/otx/client/obsolete/OTAPI_Exec.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/otx/common/Message.hpp"

#define ALEX "Alice"
#define BOB "Bob"
#define ISSUER "Issuer"

#define UNIT_DEFINITION_CONTRACT_NAME "Mt Gox USD"
#define UNIT_DEFINITION_TERMS "YOLO"
#define UNIT_DEFINITION_UNIT_OF_ACCOUNT ot::UnitType::Usd
#define CHEQUE_AMOUNT_1 2000
#define CHEQUE_MEMO_1 "memo"

namespace ot = opentxs;

namespace ottest
{
bool init_{false};

class Test_DepositCheques : public ::testing::Test
{
public:
    static const bool have_hd_;
    static const ot::UnallocatedCString SeedA_;
    static const ot::UnallocatedCString SeedB_;
    static const ot::UnallocatedCString SeedC_;
    static const ot::identifier::Nym alice_nym_id_;
    static const ot::identifier::Nym bob_nym_id_;
    static const ot::identifier::Nym issuer_nym_id_;
    static ot::identifier::Generic contact_id_alice_bob_;
    static ot::identifier::Generic contact_id_alice_issuer_;
    static ot::identifier::Generic contact_id_bob_alice_;
    static ot::identifier::Generic contact_id_bob_issuer_;
    static ot::identifier::Generic contact_id_issuer_alice_;
    static ot::identifier::Generic contact_id_issuer_bob_;

    static const ot::api::session::Client* alice_;
    static const ot::api::session::Client* bob_;

    static ot::UnallocatedCString alice_payment_code_;
    static ot::UnallocatedCString bob_payment_code_;
    static ot::UnallocatedCString issuer_payment_code_;

    static ot::identifier::UnitDefinition unit_id_;
    static ot::identifier::Generic alice_account_id_;
    static ot::identifier::Generic issuer_account_id_;

    const ot::api::session::Client& alice_client_;
    const ot::api::session::Client& bob_client_;
    const ot::api::session::Notary& server_1_;
    const ot::api::session::Client& issuer_client_;
    const ot::OTServerContract server_contract_;

    Test_DepositCheques()
        : alice_client_(ot::Context().StartClientSession(0))
        , bob_client_(ot::Context().StartClientSession(1))
        , server_1_(ot::Context().StartNotarySession(0))
        , issuer_client_(ot::Context().StartClientSession(2))
        , server_contract_(server_1_.Wallet().Internal().Server(server_1_.ID()))
    {
        if (false == init_) { init(); }
    }

    void import_server_contract(
        const ot::contract::Server& contract,
        const ot::api::session::Client& client)
    {
        auto reason = client.Factory().PasswordPrompt(__func__);
        auto bytes = ot::Space{};
        EXPECT_TRUE(server_contract_->Serialize(ot::writer(bytes), true));
        auto clientVersion =
            client.Wallet().Internal().Server(ot::reader(bytes));

        client.OTX().SetIntroductionServer(clientVersion);
    }

    void init()
    {
        const_cast<ot::UnallocatedCString&>(SeedA_) =
            alice_client_.InternalClient().Exec().Wallet_ImportSeed(
                "spike nominee miss inquiry fee nothing belt list other "
                "daughter leave valley twelve gossip paper",
                "");
        const_cast<ot::UnallocatedCString&>(SeedB_) =
            bob_client_.InternalClient().Exec().Wallet_ImportSeed(
                "trim thunder unveil reduce crop cradle zone inquiry "
                "anchor skate property fringe obey butter text tank drama "
                "palm guilt pudding laundry stay axis prosper",
                "");
        const_cast<ot::UnallocatedCString&>(SeedC_) =
            issuer_client_.InternalClient().Exec().Wallet_ImportSeed(
                "abandon abandon abandon abandon abandon abandon abandon "
                "abandon abandon abandon abandon about",
                "");
        auto reasonA = alice_client_.Factory().PasswordPrompt(__func__);
        auto reasonB = bob_client_.Factory().PasswordPrompt(__func__);
        auto reasonI = issuer_client_.Factory().PasswordPrompt(__func__);
        const_cast<ot::identifier::Nym&>(alice_nym_id_) =
            alice_client_.Wallet().Nym({SeedA_, 0}, reasonA, ALEX)->ID();
        const_cast<ot::identifier::Nym&>(bob_nym_id_) =
            bob_client_.Wallet().Nym({SeedB_, 0}, reasonB, BOB)->ID();
        const_cast<ot::identifier::Nym&>(issuer_nym_id_) =
            issuer_client_.Wallet().Nym({SeedC_, 0}, reasonI, ISSUER)->ID();

        import_server_contract(server_contract_, alice_client_);
        import_server_contract(server_contract_, bob_client_);
        import_server_contract(server_contract_, issuer_client_);

        alice_ = &alice_client_;
        bob_ = &bob_client_;

        init_ = true;
    }
};

const bool Test_DepositCheques::have_hd_{
    ot::api::crypto::HaveHDKeys() &&
    ot::api::crypto::HaveSupport(ot::crypto::asymmetric::Algorithm::Secp256k1)

};
const ot::UnallocatedCString Test_DepositCheques::SeedA_{""};
const ot::UnallocatedCString Test_DepositCheques::SeedB_{""};
const ot::UnallocatedCString Test_DepositCheques::SeedC_{""};
const ot::identifier::Nym Test_DepositCheques::alice_nym_id_{};
const ot::identifier::Nym Test_DepositCheques::bob_nym_id_{};
const ot::identifier::Nym Test_DepositCheques::issuer_nym_id_{};
ot::identifier::Generic Test_DepositCheques::contact_id_alice_bob_{};
ot::identifier::Generic Test_DepositCheques::contact_id_alice_issuer_{};
ot::identifier::Generic Test_DepositCheques::contact_id_bob_alice_{};
ot::identifier::Generic Test_DepositCheques::contact_id_bob_issuer_{};
ot::identifier::Generic Test_DepositCheques::contact_id_issuer_alice_{};
ot::identifier::Generic Test_DepositCheques::contact_id_issuer_bob_{};
const ot::api::session::Client* Test_DepositCheques::alice_{nullptr};
const ot::api::session::Client* Test_DepositCheques::bob_{nullptr};
ot::UnallocatedCString Test_DepositCheques::alice_payment_code_;
ot::UnallocatedCString Test_DepositCheques::bob_payment_code_;
ot::UnallocatedCString Test_DepositCheques::issuer_payment_code_;
ot::identifier::UnitDefinition Test_DepositCheques::unit_id_{};
ot::identifier::Generic Test_DepositCheques::alice_account_id_{};
ot::identifier::Generic Test_DepositCheques::issuer_account_id_{};

TEST_F(Test_DepositCheques, payment_codes)
{
    auto reasonA = alice_client_.Factory().PasswordPrompt(__func__);
    auto reasonB = bob_client_.Factory().PasswordPrompt(__func__);
    auto reasonI = issuer_client_.Factory().PasswordPrompt(__func__);
    auto alice = alice_client_.Wallet().mutable_Nym(alice_nym_id_, reasonA);
    auto bob = bob_client_.Wallet().mutable_Nym(bob_nym_id_, reasonB);
    auto issuer = issuer_client_.Wallet().mutable_Nym(issuer_nym_id_, reasonI);

    EXPECT_EQ(ot::identity::wot::claim::ClaimType::Individual, alice.Type());
    EXPECT_EQ(ot::identity::wot::claim::ClaimType::Individual, bob.Type());
    EXPECT_EQ(ot::identity::wot::claim::ClaimType::Individual, issuer.Type());

    auto aliceScopeSet = alice.SetScope(
        ot::identity::wot::claim::ClaimType::Individual, ALEX, true, reasonA);
    auto bobScopeSet = bob.SetScope(
        ot::identity::wot::claim::ClaimType::Individual, BOB, true, reasonB);
    auto issuerScopeSet = issuer.SetScope(
        ot::identity::wot::claim::ClaimType::Individual, ISSUER, true, reasonI);

    EXPECT_TRUE(aliceScopeSet);
    EXPECT_TRUE(bobScopeSet);
    EXPECT_TRUE(issuerScopeSet);

    alice_payment_code_ =
        alice_client_.Factory().PaymentCode(SeedA_, 0, 1, reasonA).asBase58();
    bob_payment_code_ =
        bob_client_.Factory().PaymentCode(SeedB_, 0, 1, reasonB).asBase58();
    issuer_payment_code_ =
        issuer_client_.Factory().PaymentCode(SeedC_, 0, 1, reasonI).asBase58();

    if (have_hd_) {
        EXPECT_FALSE(alice_payment_code_.empty());
        EXPECT_FALSE(bob_payment_code_.empty());
        EXPECT_FALSE(issuer_payment_code_.empty());
    } else {
        // TODO
    }

    alice.AddPaymentCode(
        alice_payment_code_, ot::UnitType::Btc, true, true, reasonA);
    bob.AddPaymentCode(
        bob_payment_code_, ot::UnitType::Btc, true, true, reasonB);
    issuer.AddPaymentCode(
        issuer_payment_code_, ot::UnitType::Btc, true, true, reasonI);
    alice.AddPaymentCode(
        alice_payment_code_, ot::UnitType::Bch, true, true, reasonA);
    bob.AddPaymentCode(
        bob_payment_code_, ot::UnitType::Bch, true, true, reasonB);
    issuer.AddPaymentCode(
        issuer_payment_code_, ot::UnitType::Bch, true, true, reasonI);

    if (have_hd_) {
        EXPECT_FALSE(alice.PaymentCode(ot::UnitType::Btc).empty());
        EXPECT_FALSE(bob.PaymentCode(ot::UnitType::Btc).empty());
        EXPECT_FALSE(issuer.PaymentCode(ot::UnitType::Btc).empty());
        EXPECT_FALSE(alice.PaymentCode(ot::UnitType::Bch).empty());
        EXPECT_FALSE(bob.PaymentCode(ot::UnitType::Bch).empty());
        EXPECT_FALSE(issuer.PaymentCode(ot::UnitType::Bch).empty());
    } else {
        // TODO
    }

    alice.Release();
    bob.Release();
    issuer.Release();
}

TEST_F(Test_DepositCheques, introduction_server)
{
    alice_client_.OTX().StartIntroductionServer(alice_nym_id_);
    bob_client_.OTX().StartIntroductionServer(bob_nym_id_);
    auto task1 = alice_client_.OTX().RegisterNymPublic(
        alice_nym_id_, server_1_.ID(), true);
    auto task2 =
        bob_client_.OTX().RegisterNymPublic(bob_nym_id_, server_1_.ID(), true);

    ASSERT_NE(0, task1.first);
    ASSERT_NE(0, task2.first);
    EXPECT_EQ(
        ot::otx::LastReplyStatus::MessageSuccess, task1.second.get().first);
    EXPECT_EQ(
        ot::otx::LastReplyStatus::MessageSuccess, task2.second.get().first);

    alice_client_.OTX().ContextIdle(alice_nym_id_, server_1_.ID()).get();
    bob_client_.OTX().ContextIdle(bob_nym_id_, server_1_.ID()).get();
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
        ALEX,
        alice_nym_id_,
        bob_client_.Factory().PaymentCode(alice_payment_code_));
    const auto bobIssuer = bob_client_.Contacts().NewContact(
        ISSUER,
        issuer_nym_id_,
        bob_client_.Factory().PaymentCode(issuer_payment_code_));
    const auto issuerAlice = issuer_client_.Contacts().NewContact(
        ALEX,
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

    auto bytes = ot::Space{};
    bob_client_.Wallet().Nym(bob_nym_id_)->Serialize(ot::writer(bytes));
    EXPECT_TRUE(alice_client_.Wallet().Nym(ot::reader(bytes)));

    issuer_client_.Wallet().Nym(issuer_nym_id_)->Serialize(ot::writer(bytes));
    EXPECT_TRUE(alice_client_.Wallet().Nym(ot::reader(bytes)));
    alice_client_.Wallet().Nym(alice_nym_id_)->Serialize(ot::writer(bytes));
    EXPECT_TRUE(bob_client_.Wallet().Nym(ot::reader(bytes)));
    issuer_client_.Wallet().Nym(issuer_nym_id_)->Serialize(ot::writer(bytes));
    EXPECT_TRUE(bob_client_.Wallet().Nym(ot::reader(bytes)));
    alice_client_.Wallet().Nym(alice_nym_id_)->Serialize(ot::writer(bytes));
    EXPECT_TRUE(issuer_client_.Wallet().Nym(ot::reader(bytes)));
    bob_client_.Wallet().Nym(bob_nym_id_)->Serialize(ot::writer(bytes));
    EXPECT_TRUE(issuer_client_.Wallet().Nym(ot::reader(bytes)));
}

TEST_F(Test_DepositCheques, issue_dollars)
{
    auto reasonI = issuer_client_.Factory().PasswordPrompt(__func__);
    const auto contract = issuer_client_.Wallet().Internal().CurrencyContract(
        issuer_nym_id_.asBase58(alice_client_.Crypto()),
        UNIT_DEFINITION_CONTRACT_NAME,
        UNIT_DEFINITION_TERMS,
        UNIT_DEFINITION_UNIT_OF_ACCOUNT,
        1,
        reasonI);

    EXPECT_EQ(ot::contract::UnitType::Currency, contract->Type());
    EXPECT_TRUE(unit_id_.empty());

    unit_id_.Assign(contract->ID());

    EXPECT_FALSE(unit_id_.empty());

    {
        auto issuer =
            issuer_client_.Wallet().mutable_Nym(issuer_nym_id_, reasonI);
        issuer.AddPreferredOTServer(
            server_1_.ID().asBase58(alice_client_.Crypto()), true, reasonI);
        issuer.AddContract(
            unit_id_.asBase58(alice_client_.Crypto()),
            ot::UnitType::Usd,
            true,
            true,
            reasonI);
    }

    auto task = issuer_client_.OTX().IssueUnitDefinition(
        issuer_nym_id_, server_1_.ID(), unit_id_);
    auto& [taskID, future] = task;
    const auto result = future.get();

    EXPECT_NE(0, taskID);
    EXPECT_EQ(ot::otx::LastReplyStatus::MessageSuccess, result.first);
    ASSERT_TRUE(result.second);

    issuer_account_id_ = issuer_client_.Factory().IdentifierFromBase58(
        result.second->acct_id_->Bytes());

    EXPECT_FALSE(issuer_account_id_.empty());

    issuer_client_.OTX().ContextIdle(issuer_nym_id_, server_1_.ID()).get();
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
    EXPECT_EQ(ot::otx::LastReplyStatus::MessageSuccess, future.get().first);

    issuer_client_.OTX().ContextIdle(issuer_nym_id_, server_1_.ID()).get();
    alice_client_.OTX().ContextIdle(alice_nym_id_, server_1_.ID()).get();
}

TEST_F(Test_DepositCheques, accept_cheque_alice)
{
    // No meaning to this operation other than to ensure the state machine has
    // completed one full cycle
    alice_client_.OTX()
        .DownloadServerContract(alice_nym_id_, server_1_.ID(), server_1_.ID())
        .second.get();
    alice_client_.OTX().ContextIdle(alice_nym_id_, server_1_.ID()).get();
    const auto count = alice_client_.OTX().DepositCheques(alice_nym_id_);

    EXPECT_EQ(1, count);

    alice_client_.OTX().ContextIdle(alice_nym_id_, server_1_.ID()).get();
    issuer_client_.OTX().ContextIdle(issuer_nym_id_, server_1_.ID()).get();
}

TEST_F(Test_DepositCheques, process_inbox_issuer)
{
    auto task = issuer_client_.OTX().ProcessInbox(
        issuer_nym_id_, server_1_.ID(), issuer_account_id_);
    auto& [id, future] = task;

    ASSERT_NE(0, id);

    const auto [status, message] = future.get();

    EXPECT_EQ(ot::otx::LastReplyStatus::MessageSuccess, status);
    ASSERT_TRUE(message);

    const auto account =
        issuer_client_.Wallet().Internal().Account(issuer_account_id_);

    EXPECT_EQ(-1 * CHEQUE_AMOUNT_1, account.get().GetBalance());
}

TEST_F(Test_DepositCheques, shutdown)
{
    alice_client_.OTX().ContextIdle(alice_nym_id_, server_1_.ID()).get();
    bob_client_.OTX().ContextIdle(bob_nym_id_, server_1_.ID()).get();
    issuer_client_.OTX().ContextIdle(issuer_nym_id_, server_1_.ID()).get();
}
}  // namespace ottest
