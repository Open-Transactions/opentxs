// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <chrono>
#include <iostream>
#include <memory>
#include <optional>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "internal/util/AsyncConst.hpp"
#include "ottest/data/crypto/Bip32.hpp"
#include "ottest/fixtures/api/crypto/Blockchain.hpp"

namespace ottest
{
TEST_F(ApiCryptoBlockchain, init)
{
    EXPECT_TRUE(invalid_nym_.empty());
    EXPECT_FALSE(nym_not_in_wallet_.empty());
    EXPECT_FALSE(alex_.empty());
    EXPECT_FALSE(bob_.empty());
    EXPECT_FALSE(chris_.empty());
    EXPECT_FALSE(daniel_.empty());
    EXPECT_TRUE(empty_id_.empty());
    EXPECT_FALSE(contact_alex_.empty());
    EXPECT_FALSE(contact_bob_.empty());
    EXPECT_FALSE(contact_chris_.empty());
    EXPECT_FALSE(contact_daniel_.empty());
    EXPECT_FALSE(fingerprint_a_.get().empty());
    EXPECT_FALSE(fingerprint_b_.get().empty());
    EXPECT_FALSE(fingerprint_c_.get().empty());
}

TEST_F(ApiCryptoBlockchain, invalid_nym)
{
    bool loaded(false);

    try {
        api_.Crypto().Blockchain().Account(invalid_nym_, btc_chain_);
        loaded = true;
    } catch (...) {
    }

    EXPECT_FALSE(loaded);

    auto accountID = api_.Crypto().Blockchain().NewHDSubaccount(
        invalid_nym_,
        ot::blockchain::crypto::HDProtocol::BIP_44,
        btc_chain_,
        reason_);

    EXPECT_TRUE(accountID.empty());

    auto list =
        api_.Crypto().Blockchain().SubaccountList(invalid_nym_, btc_chain_);

    EXPECT_EQ(list.size(), 0);
    EXPECT_EQ(list.count(accountID), 0);

    try {
        api_.Crypto().Blockchain().Account(nym_not_in_wallet_, btc_chain_);
        loaded = true;
    } catch (...) {
    }

    EXPECT_FALSE(loaded);

    accountID = api_.Crypto().Blockchain().NewHDSubaccount(
        nym_not_in_wallet_,
        ot::blockchain::crypto::HDProtocol::BIP_44,
        btc_chain_,
        reason_);

    EXPECT_TRUE(accountID.empty());

    list = api_.Crypto().Blockchain().SubaccountList(
        nym_not_in_wallet_, btc_chain_);

    EXPECT_EQ(list.size(), 0);
    EXPECT_EQ(list.count(accountID), 0);
}

// Test: when you create a nym with seed A, then the root of every HDPath for a
// blockchain account associated with that nym should also be A.
TEST_F(ApiCryptoBlockchain, TestSeedRoot)
{
    account_1_id_.set_value(api_.Crypto().Blockchain().NewHDSubaccount(
        alex_,
        ot::blockchain::crypto::HDProtocol::BIP_32,
        btc_chain_,
        reason_));
    account_2_id_.set_value(api_.Crypto().Blockchain().NewHDSubaccount(
        daniel_,
        ot::blockchain::crypto::HDProtocol::BIP_32,
        btc_chain_,
        reason_));

    EXPECT_FALSE(account_1_id_.get().empty());
    EXPECT_FALSE(account_2_id_.get().empty());

    auto list = api_.Crypto().Blockchain().SubaccountList(alex_, btc_chain_);

    EXPECT_EQ(list.size(), 1);
    EXPECT_EQ(list.count(account_1_id_.get()), 1);

    list = api_.Crypto().Blockchain().SubaccountList(daniel_, btc_chain_);

    EXPECT_EQ(list.size(), 1);
    EXPECT_EQ(list.count(account_2_id_.get()), 1);

    // Test difference in index on BIP32 implies a different account
    EXPECT_NE(account_1_id_.get(), account_2_id_.get());

    try {
        const auto& account1 =
            api_.Crypto().Blockchain().HDSubaccount(alex_, account_1_id_.get());

        EXPECT_EQ(account1.PathRoot(), fingerprint_a_.get());
    } catch (const std::exception& e) {
        std::cout << __LINE__ << ": " << e.what() << '\n';
        EXPECT_TRUE(false);
    }

    try {
        const auto& account2 = api_.Crypto().Blockchain().HDSubaccount(
            daniel_, account_2_id_.get());

        EXPECT_EQ(account2.PathRoot(), fingerprint_a_.get());
    } catch (const std::exception& e) {
        std::cout << __LINE__ << ": " << e.what() << '\n';
        EXPECT_TRUE(false);
    }

    EXPECT_EQ(alex_, api_.Crypto().Blockchain().Owner(account_1_id_.get()));
    EXPECT_EQ(daniel_, api_.Crypto().Blockchain().Owner(account_2_id_.get()));
}

// Test that one nym creates the same account for the same chain (BIP32 or
// BIP44).
TEST_F(ApiCryptoBlockchain, TestNym_AccountIdempotence)
{
    account_3_id_.set_value(api_.Crypto().Blockchain().NewHDSubaccount(
        chris_,
        ot::blockchain::crypto::HDProtocol::BIP_32,
        btc_chain_,
        reason_));
    account_4_id_.set_value(api_.Crypto().Blockchain().NewHDSubaccount(
        chris_,
        ot::blockchain::crypto::HDProtocol::BIP_44,
        btc_chain_,
        reason_));

    EXPECT_FALSE(account_3_id_.get().empty());
    EXPECT_FALSE(account_4_id_.get().empty());
    EXPECT_NE(account_3_id_.get(), account_4_id_.get());

    const auto& before = api_.Crypto()
                             .Blockchain()
                             .Account(chris_, btc_chain_)
                             .GetHD()
                             .at(account_4_id_.get());

    EXPECT_EQ(before.ID(), account_4_id_.get());

    const auto duplicate = api_.Crypto().Blockchain().NewHDSubaccount(
        chris_,
        ot::blockchain::crypto::HDProtocol::BIP_44,
        btc_chain_,
        reason_);

    EXPECT_EQ(account_4_id_.get(), duplicate);

    const auto& after = api_.Crypto()
                            .Blockchain()
                            .Account(chris_, btc_chain_)
                            .GetHD()
                            .at(account_4_id_.get());

    EXPECT_EQ(after.ID(), account_4_id_.get());

    auto list = api_.Crypto().Blockchain().SubaccountList(chris_, btc_chain_);

    EXPECT_EQ(list.size(), 2);
    EXPECT_EQ(list.count(account_3_id_.get()), 1);
    EXPECT_EQ(list.count(account_4_id_.get()), 1);
}

// Test that the same nym creates different accounts for two chains
TEST_F(ApiCryptoBlockchain, TestChainDiff)
{
    account_5_id_.set_value(api_.Crypto().Blockchain().NewHDSubaccount(
        chris_,
        ot::blockchain::crypto::HDProtocol::BIP_44,
        bch_chain_,
        reason_));

    EXPECT_NE(account_5_id_.get(), account_4_id_.get());

    auto list = api_.Crypto().Blockchain().SubaccountList(chris_, bch_chain_);

    EXPECT_EQ(list.size(), 1);
    EXPECT_EQ(list.count(account_5_id_.get()), 1);
}

TEST_F(ApiCryptoBlockchain, TestBip32_standard_1)
{
    const auto& test = Bip32TestCases().at(0);
    const ot::UnallocatedCString empty{};
    auto bytes = api_.Factory().DataFromHex(test.seed_);
    auto seed = api_.Factory().SecretFromBytes(bytes.Bytes());
    const auto fingerprint = api_.Crypto().Seed().ImportRaw(seed, reason_);

    ASSERT_FALSE(fingerprint.empty());

    const auto& nymID = api_.Wallet()
                            .Nym(
                                {api_.Factory(), fingerprint, 0},
                                individual_,
                                reason_,
                                "John Doe")
                            ->ID();

    ASSERT_FALSE(nymID.empty());

    const auto accountID = api_.Crypto().Blockchain().NewHDSubaccount(
        nymID, ot::blockchain::crypto::HDProtocol::BIP_32, btc_chain_, reason_);

    ASSERT_FALSE(accountID.empty());

    const auto& account =
        api_.Crypto().Blockchain().Account(nymID, btc_chain_).GetHD().at(0);

    EXPECT_EQ(account.ID(), accountID);

    const auto& root = account.RootNode(reason_);

    ASSERT_TRUE(root.IsValid());

    const auto& expected = test.children_.at(1);
    auto xpub = api_.Factory().Secret(0);
    auto xprv = api_.Factory().Secret(0);

    EXPECT_TRUE(root.Xpub(reason_, xpub.WriteInto()));
    EXPECT_TRUE(root.Xprv(reason_, xprv.WriteInto()));
    EXPECT_EQ(expected.xpub_, xpub.Bytes());
    EXPECT_EQ(expected.xprv_, xprv.Bytes());
}

// https://github.com/bitcoin/bips/blob/master/bip-0032.mediawiki#test-vector-3
TEST_F(ApiCryptoBlockchain, TestBip32_standard_3)
{
    const auto& test = Bip32TestCases().at(2);
    const ot::UnallocatedCString empty{};
    auto bytes = api_.Factory().DataFromHex(test.seed_);
    auto seed = api_.Factory().SecretFromBytes(bytes.Bytes());
    const auto fingerprint = api_.Crypto().Seed().ImportRaw(seed, reason_);

    ASSERT_FALSE(fingerprint.empty());

    const auto& nymID = api_.Wallet()
                            .Nym(
                                {api_.Factory(), fingerprint, 0},
                                individual_,
                                reason_,
                                "John Doe")
                            ->ID();

    ASSERT_FALSE(nymID.empty());

    const auto accountID = api_.Crypto().Blockchain().NewHDSubaccount(
        nymID, ot::blockchain::crypto::HDProtocol::BIP_32, btc_chain_, reason_);

    ASSERT_FALSE(accountID.empty());

    const auto& account =
        api_.Crypto().Blockchain().Account(nymID, btc_chain_).GetHD().at(0);

    EXPECT_EQ(account.ID(), accountID);

    const auto& root = account.RootNode(reason_);

    ASSERT_TRUE(root.IsValid());

    const auto& expected = test.children_.at(1);
    auto xpub = api_.Factory().Secret(0);
    auto xprv = api_.Factory().Secret(0);

    EXPECT_TRUE(root.Xpub(reason_, xpub.WriteInto()));
    EXPECT_TRUE(root.Xprv(reason_, xprv.WriteInto()));
    EXPECT_EQ(expected.xpub_, xpub.Bytes());
    EXPECT_EQ(expected.xprv_, xprv.Bytes());
}

TEST_F(ApiCryptoBlockchain, testBip32_SeedA)
{
    EXPECT_TRUE(check_hd_account(
        alex_,
        btc_chain_,
        account_1_id_.get(),
        contact_bob_,
        alex_external_,
        alex_internal_));
}

TEST_F(ApiCryptoBlockchain, testBip32_SeedB)
{
    account_6_id_.set_value(api_.Crypto().Blockchain().NewHDSubaccount(
        bob_, ot::blockchain::crypto::HDProtocol::BIP_32, btc_chain_, reason_));

    ASSERT_FALSE(account_6_id_.get().empty());

    auto list = api_.Crypto().Blockchain().SubaccountList(bob_, btc_chain_);

    EXPECT_EQ(list.size(), 1);
    EXPECT_EQ(list.count(account_6_id_.get()), 1);
    EXPECT_TRUE(check_hd_account(
        bob_,
        btc_chain_,
        account_6_id_.get(),
        contact_alex_,
        bob_external_,
        bob_internal_));
}

TEST_F(ApiCryptoBlockchain, testBip44_btc)
{
    EXPECT_TRUE(check_hd_account(
        chris_,
        btc_chain_,
        account_4_id_.get(),
        contact_daniel_,
        chris_btc_external_,
        chris_btc_internal_));
}

TEST_F(ApiCryptoBlockchain, testBip44_bch)
{
    EXPECT_TRUE(check_hd_account(
        chris_,
        bch_chain_,
        account_5_id_.get(),
        empty_id_,
        chris_bch_external_,
        chris_bch_internal_));
}

TEST_F(ApiCryptoBlockchain, testBip44_ltc)
{
    account_7_id_.set_value(api_.Crypto().Blockchain().NewHDSubaccount(
        chris_,
        ot::blockchain::crypto::HDProtocol::BIP_44,
        ltc_chain_,
        reason_));

    ASSERT_FALSE(account_7_id_.get().empty());

    auto list = api_.Crypto().Blockchain().SubaccountList(chris_, ltc_chain_);

    EXPECT_EQ(list.size(), 1);
    EXPECT_EQ(list.count(account_7_id_.get()), 1);
    EXPECT_TRUE(check_hd_account(
        chris_,
        ltc_chain_,
        account_7_id_.get(),
        contact_alex_,
        chris_ltc_external_,
        chris_ltc_internal_));
}

TEST_F(ApiCryptoBlockchain, AccountList)
{
    auto list = api_.Crypto().Blockchain().SubaccountList(alex_, bch_chain_);

    EXPECT_EQ(list.size(), 0);

    list = api_.Crypto().Blockchain().SubaccountList(alex_, ltc_chain_);

    EXPECT_EQ(list.size(), 0);

    list = api_.Crypto().Blockchain().SubaccountList(bob_, bch_chain_);

    EXPECT_EQ(list.size(), 0);

    list = api_.Crypto().Blockchain().SubaccountList(bob_, ltc_chain_);

    EXPECT_EQ(list.size(), 0);

    list = api_.Crypto().Blockchain().SubaccountList(daniel_, bch_chain_);

    EXPECT_EQ(list.size(), 0);

    list = api_.Crypto().Blockchain().SubaccountList(daniel_, ltc_chain_);

    EXPECT_EQ(list.size(), 0);
}

TEST_F(ApiCryptoBlockchain, reserve_addresses)
{
    const auto& nym = alex_;
    const auto chain = btc_chain_;
    account_8_id_.set_value(api_.Crypto().Blockchain().NewHDSubaccount(
        nym, ot::blockchain::crypto::HDProtocol::BIP_44, chain, reason_));
    const auto& accountID = account_8_id_.get();
    const auto subchain = Subchain::External;

    ASSERT_FALSE(accountID.empty());

    auto list = api_.Crypto().Blockchain().SubaccountList(nym, chain);

    EXPECT_EQ(list.count(accountID), 1);

    const auto& account =
        api_.Crypto().Blockchain().Account(nym, chain).GetHD().at(accountID);

    ASSERT_EQ(account.ID(), accountID);
    ASSERT_EQ(account.Lookahead(), 20u);

    auto& times = address_data_.times_;
    times.emplace_back(ot::Clock::now() - std::chrono::hours{24u * 8});
    times.emplace_back(ot::Clock::now() - std::chrono::hours{24u * 3});
    times.emplace_back(ot::Clock::now() - std::chrono::hours{24u * 1});

    auto counter{-1};

    for (const auto& time : times) {
        {  // contact only matches, no transactions
            const auto index = account.Reserve(
                subchain, random_id(), reason_, "mismatch", time);

            ASSERT_TRUE(index.has_value());
            EXPECT_EQ(index.value(), ++counter);
        }
        {  // metadata match, no transactions
            const auto index =
                account.Reserve(subchain, random_id(), reason_, "match", time);

            ASSERT_TRUE(index.has_value());
            EXPECT_EQ(index.value(), ++counter);
        }
        {  // metadata mismatch, no transactions
            const auto index = account.Reserve(
                subchain, random_id(), reason_, "mismatch", time);

            ASSERT_TRUE(index.has_value());
            EXPECT_EQ(index.value(), ++counter);
        }
        {  // no metadata, no transactions
            const auto index =
                account.Reserve(subchain, random_id(), reason_, "", time);

            ASSERT_TRUE(index.has_value());
            EXPECT_EQ(index.value(), ++counter);
        }
        {  // no metadata, unconfirmed transactions
            const auto index =
                account.Reserve(subchain, random_id(), reason_, "", time);

            ASSERT_TRUE(index.has_value());
            EXPECT_EQ(index.value(), ++counter);
        }
        {  // metadata mismatch, unconfirmed transactions
            const auto index = account.Reserve(
                subchain, random_id(), reason_, "mismatch", time);

            ASSERT_TRUE(index.has_value());
            EXPECT_EQ(index.value(), ++counter);
        }
        {  // metadata match, unconfirmed transactions
            const auto index =
                account.Reserve(subchain, random_id(), reason_, "match", time);

            ASSERT_TRUE(index.has_value());
            EXPECT_EQ(index.value(), ++counter);
        }
        {  // metadata match, confirmed transactions
            const auto index =
                account.Reserve(subchain, random_id(), reason_, "match", time);

            ASSERT_TRUE(index.has_value());
            EXPECT_EQ(index.value(), ++counter);
        }
    }
}

TEST_F(ApiCryptoBlockchain, set_metadata)
{
    const auto& nym = alex_;
    const auto chain = btc_chain_;
    const auto& accountID = account_8_id_.get();
    const auto& account =
        api_.Crypto().Blockchain().Account(nym, chain).GetHD().at(accountID);
    const auto subchain = Subchain::External;
    const auto setContact =
        ot::UnallocatedVector<ot::Bip32Index>{0, 1, 6, 8, 9, 14, 16, 17, 22};
    const auto clearContact =
        ot::UnallocatedVector<ot::Bip32Index>{3, 4, 11, 12, 19, 20};
    const auto unconfirmed =
        ot::UnallocatedVector<std::pair<ot::Bip32Index, ot::Bip32Index>>{
            {4, 0},
            {5, 0},
            {6, 0},
            {12, 1},
            {13, 1},
            {14, 1},
            {20, 2},
            {21, 2},
            {22, 2},
        };
    const auto confirmed = ot::UnallocatedVector<ot::Bip32Index>{7, 15, 23};

    for (const auto& index : setContact) {
        EXPECT_TRUE(api_.Crypto().Blockchain().AssignContact(
            nym, accountID, subchain, index, contact_bob_));

        const auto& element = account.BalanceElement(subchain, index);

        EXPECT_EQ(element.Contact(), contact_bob_);
    }

    for (const auto& index : clearContact) {
        EXPECT_TRUE(api_.Crypto().Blockchain().AssignContact(
            nym, accountID, subchain, index, empty_id_));

        const auto& element = account.BalanceElement(subchain, index);

        EXPECT_EQ(element.Contact(), empty_id_);
    }

    for (const auto& [index, time] : unconfirmed) {
        const auto& txid = address_data_.txids_.emplace_back(random_tx());

        EXPECT_TRUE(api_.Crypto().Blockchain().Unconfirm(
            {accountID, subchain, index}, txid, address_data_.times_.at(time)));

        const auto& element = account.BalanceElement(subchain, index);
        const auto transactions = element.Unconfirmed();

        ASSERT_EQ(transactions.size(), 1);
        EXPECT_EQ(transactions.front(), txid);
    }

    ASSERT_GT(address_data_.txids_.size(), 0);

    auto tIndex = address_data_.txids_.size() - 1u;

    for (const auto& index : confirmed) {
        const auto& txid = address_data_.txids_.emplace_back(random_tx());

        EXPECT_TRUE(api_.Crypto().Blockchain().Unconfirm(
            {accountID, subchain, index}, txid));

        const auto& element = account.BalanceElement(subchain, index);
        const auto transactions = element.Unconfirmed();

        ASSERT_EQ(transactions.size(), 1);
        EXPECT_EQ(transactions.front(), txid);
    }

    for (const auto& index : confirmed) {
        const auto& txid = address_data_.txids_.at(++tIndex);

        EXPECT_TRUE(api_.Crypto().Blockchain().Confirm(
            {accountID, subchain, index}, txid));

        const auto& element = account.BalanceElement(subchain, index);
        const auto transactions = element.Confirmed();

        ASSERT_EQ(transactions.size(), 1);
        EXPECT_EQ(transactions.front(), txid);
        EXPECT_EQ(element.Unconfirmed().size(), 0);
    }
}

TEST_F(ApiCryptoBlockchain, reserve)
{
    const auto& nym = alex_;
    const auto chain = btc_chain_;
    const auto& accountID = account_8_id_.get();
    const auto& account =
        api_.Crypto().Blockchain().Account(nym, chain).GetHD().at(accountID);
    const auto subchain = Subchain::External;

    {
        auto index = account.Reserve(subchain, reason_);

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 3);
    }
    {
        auto index = account.Reserve(subchain, reason_);

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 11);
    }

    for (auto i = 24u; i < 44u; ++i) {
        auto index = account.Reserve(subchain, reason_);

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), i);
    }

    {
        auto index = account.Reserve(subchain, contact_bob_, reason_, "match");

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 1);
    }
    {
        auto index = account.Reserve(subchain, reason_);

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 4);
    }
    {
        auto index = account.Reserve(subchain, reason_);

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 44);
    }
    {
        auto index = account.Reserve(subchain, contact_bob_, reason_, "match");

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 6);
    }
    {
        auto index = account.Reserve(subchain, contact_bob_, reason_, "match");

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 9);
    }
    {
        auto index = account.Reserve(subchain, contact_bob_, reason_, "match");

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 45);
    }
}

TEST_F(ApiCryptoBlockchain, release)
{
    const auto& nym = alex_;
    const auto chain = btc_chain_;
    const auto& accountID = account_8_id_.get();
    const auto& bc = api_.Crypto().Blockchain();
    const auto& account = bc.Account(nym, chain).GetHD().at(accountID);
    const auto subchain = Subchain::External;

    EXPECT_FALSE(bc.Release({accountID, subchain, 7}));
    EXPECT_FALSE(bc.Release({accountID, subchain, 15}));
    EXPECT_FALSE(bc.Release({accountID, subchain, 23}));
    EXPECT_TRUE(bc.Release({accountID, subchain, 0}));
    EXPECT_TRUE(bc.Release({accountID, subchain, 2}));
    EXPECT_TRUE(bc.Release({accountID, subchain, 5}));
    EXPECT_TRUE(bc.Release({accountID, subchain, 8}));
    EXPECT_TRUE(bc.Release({accountID, subchain, 10}));
    EXPECT_TRUE(bc.Release({accountID, subchain, 12}));
    EXPECT_TRUE(bc.Release({accountID, subchain, 13}));
    EXPECT_TRUE(bc.Release({accountID, subchain, 14}));
    EXPECT_TRUE(bc.Release({accountID, subchain, 16}));
    EXPECT_TRUE(bc.Release({accountID, subchain, 17}));
    EXPECT_TRUE(bc.Release({accountID, subchain, 18}));
    EXPECT_TRUE(bc.Release({accountID, subchain, 19}));
    EXPECT_TRUE(bc.Release({accountID, subchain, 20}));
    EXPECT_TRUE(bc.Release({accountID, subchain, 21}));
    EXPECT_TRUE(bc.Release({accountID, subchain, 22}));

    {
        auto index = account.Reserve(subchain, reason_);

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 0);
    }
    {
        auto index = account.Reserve(subchain, reason_);

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 2);
    }
    {
        auto index = account.Reserve(subchain, reason_);

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 8);
    }
    {
        auto index = account.Reserve(subchain, reason_);

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 10);
    }
    {
        auto index = account.Reserve(subchain, reason_);

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 16);
    }
    {
        auto index = account.Reserve(subchain, reason_);

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 17);
    }
    {
        auto index = account.Reserve(subchain, reason_);

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 18);
    }
    {
        auto index = account.Reserve(subchain, reason_);

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 19);
    }
    {
        auto index = account.Reserve(subchain, reason_);

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 5);
    }
}

TEST_F(ApiCryptoBlockchain, floor)
{
    const auto& nym = alex_;
    const auto chain = btc_chain_;
    const auto& accountID = account_8_id_.get();
    const auto& bc = api_.Crypto().Blockchain();
    const auto& account = bc.Account(nym, chain).GetHD().at(accountID);
    const auto subchain = Subchain::External;
    auto& txids = address_data_.txids_;

    EXPECT_TRUE(
        bc.Confirm({accountID, subchain, 6}, txids.emplace_back(random_tx())));
    EXPECT_EQ(account.Floor(subchain).value_or(999), 0);
    EXPECT_TRUE(
        bc.Confirm({accountID, subchain, 5}, txids.emplace_back(random_tx())));
    EXPECT_EQ(account.Floor(subchain).value_or(999), 0);
    EXPECT_TRUE(
        bc.Confirm({accountID, subchain, 4}, txids.emplace_back(random_tx())));
    EXPECT_EQ(account.Floor(subchain).value_or(999), 0);
    EXPECT_TRUE(
        bc.Confirm({accountID, subchain, 3}, txids.emplace_back(random_tx())));
    EXPECT_EQ(account.Floor(subchain).value_or(999), 0);
    EXPECT_TRUE(
        bc.Confirm({accountID, subchain, 2}, txids.emplace_back(random_tx())));
    EXPECT_EQ(account.Floor(subchain).value_or(999), 0);
    EXPECT_TRUE(
        bc.Confirm({accountID, subchain, 1}, txids.emplace_back(random_tx())));
    EXPECT_EQ(account.Floor(subchain).value_or(999), 0);
    EXPECT_TRUE(
        bc.Confirm({accountID, subchain, 0}, txids.emplace_back(random_tx())));
    EXPECT_EQ(account.Floor(subchain).value_or(999), 8);
    ASSERT_EQ(account.BalanceElement(subchain, 7).Confirmed().size(), 1);
    EXPECT_TRUE(bc.Unconfirm(
        {accountID, subchain, 7},
        account.BalanceElement(subchain, 7).Confirmed().front()));
    EXPECT_EQ(account.BalanceElement(subchain, 7).Confirmed().size(), 0);
    EXPECT_EQ(account.Floor(subchain).value_or(999), 7);
    EXPECT_TRUE(bc.Confirm({accountID, subchain, 7}, txids.at(0)));
    EXPECT_EQ(account.Floor(subchain).value_or(999), 8);
    EXPECT_TRUE(bc.Confirm({accountID, subchain, 7}, txids.at(1)));
    EXPECT_EQ(account.Floor(subchain).value_or(999), 8);
    EXPECT_TRUE(bc.Unconfirm({accountID, subchain, 7}, txids.at(1)));
    EXPECT_EQ(account.Floor(subchain).value_or(999), 8);
    ASSERT_EQ(account.BalanceElement(subchain, 15).Confirmed().size(), 1);
    EXPECT_TRUE(bc.Unconfirm(
        {accountID, subchain, 15},
        account.BalanceElement(subchain, 15).Confirmed().front()));
    EXPECT_EQ(account.BalanceElement(subchain, 15).Confirmed().size(), 0);
    EXPECT_EQ(account.Floor(subchain).value_or(999), 8);
    ASSERT_EQ(account.BalanceElement(subchain, 0).Confirmed().size(), 1);
    EXPECT_TRUE(bc.Unconfirm(
        {accountID, subchain, 0},
        account.BalanceElement(subchain, 0).Confirmed().front()));
    EXPECT_EQ(account.BalanceElement(subchain, 0).Confirmed().size(), 0);
    EXPECT_EQ(account.Floor(subchain).value_or(999), 0);
    EXPECT_TRUE(
        bc.Confirm({accountID, subchain, 0}, txids.emplace_back(random_tx())));
    EXPECT_EQ(account.Floor(subchain).value_or(999), 8);

    {
        auto index = account.Reserve(subchain, reason_);

        ASSERT_TRUE(index.has_value());
        EXPECT_EQ(index.value(), 12);
    }
}

TEST_F(ApiCryptoBlockchain, paymentcode)
{
    const auto& nym = alex_;
    const auto& chain = btc_chain_;
    const auto pNym = api_.Wallet().Nym(nym);
    auto bytes = ot::Space{};
    const auto accountID = api_.Crypto().Blockchain().NewPaymentCodeSubaccount(
        nym,
        api_.Factory().PaymentCodeFromBase58(pNym->PaymentCode()),
        api_.Factory().PaymentCodeFromBase58(
            "PD1jFsimY3DQUe7qGtx3z8BohTaT6r4kwJMCYXwp7uY8z6BSaFrpM"sv),
        [&] {
            pNym->PaymentCodePath(ot::writer(bytes));
            return ot::reader(bytes);
        }(),
        chain,
        reason_);

    ASSERT_FALSE(accountID.empty());

    const auto& account = api_.Crypto()
                              .Blockchain()
                              .Account(nym, chain)
                              .GetPaymentCode()
                              .at(accountID);

    EXPECT_EQ(account.ID(), accountID);
    EXPECT_TRUE(check_initial_state(account, Subchain::Outgoing));
    EXPECT_TRUE(check_initial_state(account, Subchain::Incoming));

    const auto index = account.Reserve(Subchain::Outgoing, reason_);

    ASSERT_TRUE(index.has_value());
    EXPECT_EQ(index.value(), 0u);
}

TEST_F(ApiCryptoBlockchain, batch)
{
    const auto& nym = alex_;
    const auto chain = bch_chain_;
    account_9_id_.set_value(api_.Crypto().Blockchain().NewHDSubaccount(
        nym, ot::blockchain::crypto::HDProtocol::BIP_44, chain, reason_));
    const auto& accountID = account_9_id_.get();

    ASSERT_FALSE(accountID.empty());

    auto list = api_.Crypto().Blockchain().SubaccountList(nym, chain);

    EXPECT_EQ(list.count(accountID), 1);

    const auto& account =
        api_.Crypto().Blockchain().Account(nym, chain).GetHD().at(accountID);

    ASSERT_EQ(account.ID(), accountID);

    constexpr auto count{1000u};
    constexpr auto subchain{Subchain::External};
    const auto indices = account.Reserve(subchain, count, reason_);
    const auto gen = account.LastGenerated(subchain);

    ASSERT_TRUE(gen.has_value());
    EXPECT_EQ(gen.value(), count - 1u);
    ASSERT_TRUE(indices.size() == count);

    for (auto i{0u}; i < count; ++i) {
        const auto index = indices.at(i);

        EXPECT_EQ(index, i);

        try {
            account.BalanceElement(subchain, index);

            EXPECT_TRUE(true);
        } catch (...) {
            EXPECT_TRUE(false);
        }
    }
}
}  // namespace ottest
