// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <future>
#include <memory>
#include <string_view>
#include <utility>

#include "internal/api/session/Wallet.hpp"
#include "internal/core/String.hpp"
#include "internal/core/contract/ServerContract.hpp"
#include "internal/core/contract/Unit.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/otx/common/Message.hpp"
#include "ottest/env/OTTestEnvironment.hpp"

#include "ottest/fixtures/otx/broken/DepositCheques.hpp"

#define UNIT_DEFINITION_CONTRACT_NAME "Mt Gox USD"
#define UNIT_DEFINITION_TERMS "YOLO"
#define UNIT_DEFINITION_UNIT_OF_ACCOUNT ot::UnitType::Usd
#define CHEQUE_AMOUNT_1 2000
#define CHEQUE_MEMO_1 "memo"

namespace ot = opentxs;

namespace ottest
{

TEST_F(DepositCheques, payment_codes)
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

TEST_F(DepositCheques, introduction_server)
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

TEST_F(DepositCheques, add_contacts)
{
    const auto aliceBob = alice_client_.Contacts().NewContact(
        BOB,
        bob_nym_id_,
        alice_client_.Factory().PaymentCodeFromBase58(bob_payment_code_));
    const auto aliceIssuer = alice_client_.Contacts().NewContact(
        ISSUER,
        issuer_nym_id_,
        alice_client_.Factory().PaymentCodeFromBase58(issuer_payment_code_));
    const auto bobAlice = bob_client_.Contacts().NewContact(
        ALEX,
        alice_nym_id_,
        bob_client_.Factory().PaymentCodeFromBase58(alice_payment_code_));
    const auto bobIssuer = bob_client_.Contacts().NewContact(
        ISSUER,
        issuer_nym_id_,
        bob_client_.Factory().PaymentCodeFromBase58(issuer_payment_code_));
    const auto issuerAlice = issuer_client_.Contacts().NewContact(
        ALEX,
        alice_nym_id_,
        issuer_client_.Factory().PaymentCodeFromBase58(alice_payment_code_));
    const auto issuerBob = issuer_client_.Contacts().NewContact(
        BOB,
        bob_nym_id_,
        issuer_client_.Factory().PaymentCodeFromBase58(bob_payment_code_));

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

TEST_F(DepositCheques, issue_dollars)
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

    issuer_account_id_ = issuer_client_.Factory().AccountIDFromBase58(
        result.second->acct_id_->Bytes());

    EXPECT_FALSE(issuer_account_id_.empty());

    issuer_client_.OTX().ContextIdle(issuer_nym_id_, server_1_.ID()).get();
}

TEST_F(DepositCheques, pay_alice)
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

TEST_F(DepositCheques, accept_cheque_alice)
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

TEST_F(DepositCheques, process_inbox_issuer)
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

TEST_F(DepositCheques, shutdown)
{
    alice_client_.OTX().ContextIdle(alice_nym_id_, server_1_.ID()).get();
    bob_client_.OTX().ContextIdle(bob_nym_id_, server_1_.ID()).get();
    issuer_client_.OTX().ContextIdle(issuer_nym_id_, server_1_.ID()).get();
}
}  // namespace ottest
