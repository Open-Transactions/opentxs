// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/rpc/Helpers.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <atomic>
#include <iostream>

#include "ottest/data/crypto/PaymentCodeV3.hpp"
#include "ottest/fixtures/common/Counter.hpp"
#include "ottest/fixtures/common/User.hpp"
#include "ottest/fixtures/ui/AccountTree.hpp"

namespace ot = opentxs;

namespace ottest
{
const auto issuer_ = ot::UnallocatedCString{
    "ot2xuVPJDdweZvKLQD42UMCzhCmT3okn3W1PktLgCbmQLRnaKy848sX"};
const auto brian_ = ot::UnallocatedCString{
    "ot2xuUvJU5kP6hePQMrwFcm5MiM5g1eHnQjiY82M8LmLtNM9AAzGqE5"};

auto issuerModel = Counter{};
auto brianModel = Counter{};

TEST_F(RPC_fixture, preconditions)
{
    {
        const auto& session = StartNotarySession(0);
        const auto instance = session.Instance();
        const auto& seeds = seed_map_.at(instance);
        const auto& nyms = local_nym_map_.at(instance);

        EXPECT_EQ(seeds.size(), 1);
        EXPECT_EQ(nyms.size(), 1);
    }
    {
        const auto& server = ot_.NotarySession(0);
        const auto& session = StartClient(0);
        const auto instance = session.Instance();
        const auto& nyms = local_nym_map_.at(instance);
        const auto& seeds = seed_map_.at(instance);
        const auto seed =
            ImportBip39(session, GetPaymentCodeVector3().alice_.words_);

        EXPECT_FALSE(seed.empty());
        EXPECT_TRUE(SetIntroductionServer(session, server));

        const auto& issuer = CreateNym(session, "issuer", seed, 0);
        const auto& brian = CreateNym(session, "brian", seed, 1);

        EXPECT_EQ(issuer.nym_id_.asBase58(ot_.Crypto()), issuer_);
        EXPECT_EQ(brian.nym_id_.asBase58(ot_.Crypto()), brian_);
        EXPECT_TRUE(RegisterNym(server, issuer));
        EXPECT_TRUE(RegisterNym(server, brian));
        EXPECT_EQ(seeds.size(), 1);
        EXPECT_EQ(nyms.size(), 2);

        InitAccountTreeCounter(issuer, issuerModel);
        InitAccountTreeCounter(brian, brianModel);
    }
}

TEST_F(RPC_fixture, initial_state)
{
    const auto& issuer = users_.at(0);
    const auto& brian = users_.at(1);
    const auto expectedIssuer = AccountTreeData{};
    const auto expectedBrian = AccountTreeData{};

    ASSERT_TRUE(wait_for_counter(issuerModel));
    ASSERT_TRUE(wait_for_counter(brianModel));
    EXPECT_TRUE(check_account_tree(brian, expectedBrian));
    EXPECT_TRUE(check_account_tree(issuer, expectedIssuer));
    EXPECT_TRUE(check_account_tree_qt(brian, expectedBrian));
    EXPECT_TRUE(check_account_tree_qt(issuer, expectedIssuer));
}

TEST_F(RPC_fixture, create_accounts)
{
    const auto& server = ot_.NotarySession(0);
    const auto& issuer = users_.at(0);
    const auto& brian = users_.at(1);
    issuerModel.expected_ += 4;
    brianModel.expected_ += 6;

    {
        const auto usd =
            IssueUnit(server, issuer, "Mt Gox USD", "YOLO", ot::UnitType::Usd);
        const auto btc =
            IssueUnit(server, issuer, "Mt Gox BTC", "YOLO", ot::UnitType::Btc);

        EXPECT_FALSE(usd.empty());
        EXPECT_FALSE(btc.empty());
        EXPECT_EQ(created_units_.size(), 2);

        const auto account1 =
            RegisterAccount(server, brian, usd, "trading account");
        const auto account2 =
            RegisterAccount(server, brian, btc, "trading account");
        const auto account3 = RegisterAccount(server, brian, btc, "");

        EXPECT_FALSE(account1.empty());
        EXPECT_FALSE(account2.empty());
        EXPECT_FALSE(account3.empty());
    }
    {
        const auto& api = *brian.api_;
        const auto reason = api.Factory().PasswordPrompt("");
        const auto id = brian.api_->Crypto().Blockchain().NewHDSubaccount(
            brian.nym_id_,
            ot::blockchain::crypto::HDProtocol::BIP_44,
            ot::blockchain::Type::Bitcoin,
            reason);
    }
}

TEST_F(RPC_fixture, accounts_after_creation)
{
    const auto& notary = ot_.NotarySession(0).ID();
    const auto& issuer = users_.at(0);
    const auto& brian = users_.at(1);
    const auto& issuerAccounts =
        registered_accounts_[issuer.nym_id_.asBase58(ot_.Crypto())];
    const auto& brianAccounts =
        registered_accounts_[brian.nym_id_.asBase58(ot_.Crypto())];
    const auto& btcAccount = brian.api_->Crypto().Blockchain().Account(
        brian.nym_id_, ot::blockchain::Type::Bitcoin);
    const auto expectedIssuer = AccountTreeData{{
        {ot::UnitType::Btc,
         "Bitcoin",
         {
             {issuerAccounts.at(1),
              created_units_.at(1),
              "BTC",
              "issuer account at localhost",
              notary.asBase58(ot_.Crypto()),
              "localhost",
              ot::AccountType::Custodial,
              ot::UnitType::Btc,
              0,
              0,
              "0 ₿"},
         }},
        {ot::UnitType::Usd,
         "US Dollar",
         {
             {issuerAccounts.at(0),
              created_units_.at(0),
              "USD",
              "issuer account at localhost",
              notary.asBase58(ot_.Crypto()),
              "localhost",
              ot::AccountType::Custodial,
              ot::UnitType::Usd,
              0,
              0,
              "$0.00"},
         }},
    }};
    const auto expectedBrian = AccountTreeData{{
        {ot::UnitType::Btc,
         "Bitcoin",
         {
             {btcAccount.AccountID().asBase58(ot_.Crypto()),
              "ot2yCfLY16UZNqrhYDN5Fy9VSmQqeVnTff69APRouWDsXgRJ1Kf9vpp",
              "BTC",
              "On chain BTC (this device)",
              "ot2y4521R8bEif6mfPDg1rAx5puRtj51PVpyW1VFXBZDc45StKYWQw6",
              "Bitcoin",
              ot::AccountType::Blockchain,
              ot::UnitType::Btc,
              0,
              0,
              "0 ₿"},
             {brianAccounts.at(2),
              created_units_.at(1),
              "BTC",
              "Mt Gox BTC at localhost",
              notary.asBase58(ot_.Crypto()),
              "localhost",
              ot::AccountType::Custodial,
              ot::UnitType::Btc,
              0,
              0,
              "0 ₿"},
             {brianAccounts.at(1),
              created_units_.at(1),
              "BTC",
              "trading account at localhost",
              notary.asBase58(ot_.Crypto()),
              "localhost",
              ot::AccountType::Custodial,
              ot::UnitType::Btc,
              0,
              0,
              "0 ₿"},
         }},
        {ot::UnitType::Usd,
         "US Dollar",
         {
             {brianAccounts.at(0),
              created_units_.at(0),
              "USD",
              "trading account at localhost",
              notary.asBase58(ot_.Crypto()),
              "localhost",
              ot::AccountType::Custodial,
              ot::UnitType::Usd,
              0,
              0,
              "$0.00"},
         }},
    }};
    const auto brianBlockchainAccounts =
        brian.api_->Crypto().Blockchain().AccountList(brian.nym_id_);

    ASSERT_EQ(brianBlockchainAccounts.size(), 1);
    EXPECT_EQ(*brianBlockchainAccounts.begin(), btcAccount.AccountID());
    ASSERT_TRUE(wait_for_counter(issuerModel));
    ASSERT_TRUE(wait_for_counter(brianModel));
    EXPECT_TRUE(check_account_tree(brian, expectedBrian));
    EXPECT_TRUE(check_account_tree(issuer, expectedIssuer));
    EXPECT_TRUE(check_account_tree_qt(brian, expectedBrian));
    EXPECT_TRUE(check_account_tree_qt(issuer, expectedIssuer));
}

TEST_F(RPC_fixture, otx_payment)
{
    const auto& notary = ot_.NotarySession(0);
    const auto& client = ot_.ClientSession(0);
    const auto& issuer = users_.at(0);
    const auto& brian = users_.at(1);
    issuerModel.expected_ += 1;
    brianModel.expected_ += 1;

    EXPECT_TRUE(SendCheque(
        notary,
        issuer,
        registered_accounts_[issuer.nym_id_.asBase58(ot_.Crypto())].front(),
        client.Contacts().ContactID(brian.nym_id_).asBase58(ot_.Crypto()),
        "memo1",
        10));
    EXPECT_GT(DepositCheques(notary, brian), 0);

    RefreshAccount(client, {issuer_, brian_}, notary.ID());
}

TEST_F(RPC_fixture, accounts_after_otx)
{
    const auto& notary = ot_.NotarySession(0).ID();
    const auto& issuer = users_.at(0);
    const auto& brian = users_.at(1);
    const auto& issuerAccounts =
        registered_accounts_[issuer.nym_id_.asBase58(ot_.Crypto())];
    const auto& brianAccounts =
        registered_accounts_[brian.nym_id_.asBase58(ot_.Crypto())];
    const auto& btcAccount = brian.api_->Crypto().Blockchain().Account(
        brian.nym_id_, ot::blockchain::Type::Bitcoin);
    const auto expectedIssuer = AccountTreeData{{
        {ot::UnitType::Btc,
         "Bitcoin",
         {
             {issuerAccounts.at(1),
              created_units_.at(1),
              "BTC",
              "issuer account at localhost",
              notary.asBase58(ot_.Crypto()),
              "localhost",
              ot::AccountType::Custodial,
              ot::UnitType::Btc,
              0,
              0,
              "0 ₿"},
         }},
        {ot::UnitType::Usd,
         "US Dollar",
         {
             {issuerAccounts.at(0),
              created_units_.at(0),
              "USD",
              "issuer account at localhost",
              notary.asBase58(ot_.Crypto()),
              "localhost",
              ot::AccountType::Custodial,
              ot::UnitType::Usd,
              -1,
              -10,
              "$-10.00"},
         }},
    }};
    const auto expectedBrian = AccountTreeData{{
        {ot::UnitType::Btc,
         "Bitcoin",
         {
             {btcAccount.AccountID().asBase58(ot_.Crypto()),
              "ot2yCfLY16UZNqrhYDN5Fy9VSmQqeVnTff69APRouWDsXgRJ1Kf9vpp",
              "BTC",
              "On chain BTC (this device)",
              "ot2y4521R8bEif6mfPDg1rAx5puRtj51PVpyW1VFXBZDc45StKYWQw6",
              "Bitcoin",
              ot::AccountType::Blockchain,
              ot::UnitType::Btc,
              0,
              0,
              "0 ₿"},
             {brianAccounts.at(2),
              created_units_.at(1),
              "BTC",
              "Mt Gox BTC at localhost",
              notary.asBase58(ot_.Crypto()),
              "localhost",
              ot::AccountType::Custodial,
              ot::UnitType::Btc,
              0,
              0,
              "0 ₿"},
             {brianAccounts.at(1),
              created_units_.at(1),
              "BTC",
              "trading account at localhost",
              notary.asBase58(ot_.Crypto()),
              "localhost",
              ot::AccountType::Custodial,
              ot::UnitType::Btc,
              0,
              0,
              "0 ₿"},
         }},
        {ot::UnitType::Usd,
         "US Dollar",
         {
             {brianAccounts.at(0),
              created_units_.at(0),
              "USD",
              "trading account at localhost",
              notary.asBase58(ot_.Crypto()),
              "localhost",
              ot::AccountType::Custodial,
              ot::UnitType::Usd,
              1,
              10,
              "$10.00"},
         }},
    }};

    ASSERT_TRUE(wait_for_counter(issuerModel));
    ASSERT_TRUE(wait_for_counter(brianModel));
    EXPECT_TRUE(check_account_tree(brian, expectedBrian));
    EXPECT_TRUE(check_account_tree(issuer, expectedIssuer));
    EXPECT_TRUE(check_account_tree_qt(brian, expectedBrian));
    EXPECT_TRUE(check_account_tree_qt(issuer, expectedIssuer));
}

TEST_F(RPC_fixture, blockchain_payment) { std::cout << "TODO\n"; }

TEST_F(RPC_fixture, accounts_after_blockchain)
{
    const auto& notary = ot_.NotarySession(0).ID();
    const auto& issuer = users_.at(0);
    const auto& brian = users_.at(1);
    const auto& issuerAccounts =
        registered_accounts_[issuer.nym_id_.asBase58(ot_.Crypto())];
    const auto& brianAccounts =
        registered_accounts_[brian.nym_id_.asBase58(ot_.Crypto())];
    const auto& btcAccount = brian.api_->Crypto().Blockchain().Account(
        brian.nym_id_, ot::blockchain::Type::Bitcoin);
    const auto expectedIssuer = AccountTreeData{{
        {ot::UnitType::Btc,
         "Bitcoin",
         {
             {issuerAccounts.at(1),
              created_units_.at(1),
              "BTC",
              "issuer account at localhost",
              notary.asBase58(ot_.Crypto()),
              "localhost",
              ot::AccountType::Custodial,
              ot::UnitType::Btc,
              0,
              0,
              "0 ₿"},
         }},
        {ot::UnitType::Usd,
         "US Dollar",
         {
             {issuerAccounts.at(0),
              created_units_.at(0),
              "USD",
              "issuer account at localhost",
              notary.asBase58(ot_.Crypto()),
              "localhost",
              ot::AccountType::Custodial,
              ot::UnitType::Usd,
              -1,
              -10,
              "$-10.00"},
         }},
    }};
    const auto expectedBrian = AccountTreeData{{
        {ot::UnitType::Btc,
         "Bitcoin",
         {
             {btcAccount.AccountID().asBase58(ot_.Crypto()),
              "ot2yCfLY16UZNqrhYDN5Fy9VSmQqeVnTff69APRouWDsXgRJ1Kf9vpp",
              "BTC",
              "On chain BTC (this device)",
              "ot2y4521R8bEif6mfPDg1rAx5puRtj51PVpyW1VFXBZDc45StKYWQw6",
              "Bitcoin",
              ot::AccountType::Blockchain,
              ot::UnitType::Btc,
              0,
              0,
              "0 ₿"},
             {brianAccounts.at(2),
              created_units_.at(1),
              "BTC",
              "Mt Gox BTC at localhost",
              notary.asBase58(ot_.Crypto()),
              "localhost",
              ot::AccountType::Custodial,
              ot::UnitType::Btc,
              0,
              0,
              "0 ₿"},
             {brianAccounts.at(1),
              created_units_.at(1),
              "BTC",
              "trading account at localhost",
              notary.asBase58(ot_.Crypto()),
              "localhost",
              ot::AccountType::Custodial,
              ot::UnitType::Btc,
              0,
              0,
              "0 ₿"},
         }},
        {ot::UnitType::Usd,
         "US Dollar",
         {
             {brianAccounts.at(0),
              created_units_.at(0),
              "USD",
              "trading account at localhost",
              notary.asBase58(ot_.Crypto()),
              "localhost",
              ot::AccountType::Custodial,
              ot::UnitType::Usd,
              1,
              10,
              "$10.00"},
         }},
    }};

    ASSERT_TRUE(wait_for_counter(issuerModel));
    ASSERT_TRUE(wait_for_counter(brianModel));
    EXPECT_TRUE(check_account_tree(brian, expectedBrian));
    EXPECT_TRUE(check_account_tree(issuer, expectedIssuer));
    EXPECT_TRUE(check_account_tree_qt(brian, expectedBrian));
    EXPECT_TRUE(check_account_tree_qt(issuer, expectedIssuer));
}

TEST_F(RPC_fixture, cleanup) { Cleanup(); }
}  // namespace ottest
