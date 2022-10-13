// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <algorithm>
#include <atomic>
#include <chrono>
#include <future>
#include <memory>
#include <optional>
#include <tuple>
#include <utility>

#include "internal/api/session/UI.hpp"
#include "internal/interface/ui/AccountActivity.hpp"
#include "internal/util/LogMacros.hpp"
#include "ottest/fixtures/blockchain/Common.hpp"
#include "ottest/fixtures/blockchain/ScanListener.hpp"
#include "ottest/fixtures/blockchain/TXOs.hpp"
#include "ottest/fixtures/blockchain/regtest/Base.hpp"
#include "ottest/fixtures/blockchain/regtest/HD.hpp"
#include "ottest/fixtures/common/Counter.hpp"
#include "ottest/fixtures/common/User.hpp"
#include "ottest/fixtures/rpc/Helpers.hpp"
#include "ottest/fixtures/ui/AccountActivity.hpp"
#include "ottest/fixtures/ui/AccountList.hpp"
#include "ottest/fixtures/ui/BlockchainAccountStatus.hpp"

namespace ottest
{
using namespace std::literals::chrono_literals;
using namespace opentxs::literals;

using Protocol = ot::blockchain::crypto::HDProtocol;
using Subaccount = ot::blockchain::crypto::SubaccountType;
using Subchain = ot::blockchain::crypto::Subchain;

Counter account_list_{};
Counter account_activity_{};
Counter account_status_{};

TEST_F(Regtest_fixture_hd, init_opentxs) {}

TEST_F(Regtest_fixture_hd, start_chains) { EXPECT_TRUE(Start()); }

TEST_F(Regtest_fixture_hd, connect_peers) { EXPECT_TRUE(Connect()); }

TEST_F(Regtest_fixture_hd, init_ui_models)
{
    account_activity_.expected_ += 0;
    account_list_.expected_ += 1;
    account_status_.expected_ += 7;
    init_account_activity(
        alice_, SendHD().Parent().AccountID(), account_activity_);
    init_account_list(alice_, account_list_);
    init_blockchain_account_status(alice_, test_chain_, account_status_);
}

TEST_F(Regtest_fixture_hd, account_activity_initial)
{
    const auto& id = SendHD().Parent().AccountID();
    const auto expected = AccountActivityData{
        expected_account_type_,
        id.asBase58(ot_.Crypto()),
        expected_account_name_,
        expected_unit_type_,
        expected_unit_.asBase58(ot_.Crypto()),
        expected_display_unit_,
        expected_notary_.asBase58(ot_.Crypto()),
        expected_notary_name_,
        0,
        0,
        u8"0 units"_cstr,
        "",
        {},
        {test_chain_},
        0,
        {height_, height_},
        {
            {"mipcBbFg9gMiCh81Kj8tqqdgoZub1ZJRfn", true},
            {"2MzQwSSnBHWHqSAqtTVQ6v47XtaisrJa1Vc", true},
            {"2N2PJEucf6QY2kNFuJ4chQEBoyZWszRQE16", true},
            {"17VZNX1SN5NtKa8UQFxwQbFeFc3iqRYhem", false},
            {"3EktnHQD7RiAE6uzMj2ZifT9YgRrkSgzQX", false},
            {"LM2WMpR1Rp6j3Sa59cMXMs1SPzj9eXpGc1", false},
            {"MTf4tP1TCNBn8dNkyxeBVoPrFCcVzxJvvh", false},
            {"QVk4MvUu7Wb7tZ1wvAeiUvdF7wxhvpyLLK", false},
            {"pS8EA1pKEVBvv3kGsSGH37R8YViBmuRCPn", false},
        },
        {{u8"0"_cstr, u8"0 units"_cstr},
         {u8"10"_cstr, u8"10 units"_cstr},
         {u8"25"_cstr, u8"25 units"_cstr},
         {u8"300"_cstr, u8"300 units"_cstr},
         {u8"4000"_cstr, u8"4,000 units"_cstr},
         {u8"50000"_cstr, u8"50,000 units"_cstr},
         {u8"600000"_cstr, u8"600,000 units"_cstr},
         {u8"7000000"_cstr, u8"7,000,000 units"_cstr},
         {u8"1000000000000000001"_cstr,
          u8"1,000,000,000,000,000,001 units"_cstr},
         {u8"74.99999448"_cstr, u8"74.999\u202F994\u202F48 units"_cstr},
         {u8"86.00002652"_cstr, u8"86.000\u202F026\u202F52 units"_cstr},
         {u8"89.99999684"_cstr, u8"89.999\u202F996\u202F84 units"_cstr},
         {u8"100.0000495"_cstr, u8"100.000\u202F049\u202F5 units"_cstr}},
        {},
    };
    wait_for_counter(account_activity_, false);

    EXPECT_TRUE(check_account_activity(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(alice_, id, expected));
}

TEST_F(Regtest_fixture_hd, account_list_initial)
{
    const auto expected = AccountListData{{
        {SendHD().Parent().AccountID().asBase58(ot_.Crypto()),
         expected_unit_.asBase58(ot_.Crypto()),
         expected_display_unit_,
         expected_account_name_,
         expected_notary_.asBase58(ot_.Crypto()),
         expected_notary_name_,
         expected_account_type_,
         expected_unit_type_,
         0,
         0,
         u8"0 units"_cstr},
    }};
    wait_for_counter(account_list_, false);

    EXPECT_TRUE(check_account_list(alice_, expected));
    EXPECT_TRUE(check_account_list_qt(alice_, expected));
    EXPECT_TRUE(check_account_list_rpc(alice_, expected));
}

TEST_F(Regtest_fixture_hd, account_status_initial)
{
    const auto expected = BlockchainAccountStatusData{
        alice_.nym_id_.asBase58(ot_.Crypto()),
        test_chain_,
        {
            {"Unnamed seed: BIP-39 (default)",
             alice_.seed_id_,
             Subaccount::HD,
             {
                 {"BIP-44: m / 44' / 1' / 0'",
                  SendHD().ID().asBase58(ot_.Crypto()),
                  {
                      {"external subchain: 0 of 1 (0.000000 %)",
                       Subchain::External},
                      {"internal subchain: 0 of 1 (0.000000 %)",
                       Subchain::Internal},
                  }},
             }},
            {alice_.payment_code_ + " (local)",
             alice_.nym_id_.asBase58(ot_.Crypto()),
             Subaccount::PaymentCode,
             {
                 {"Notification transactions",
                  Account(alice_, test_chain_)
                      .GetNotification()
                      .at(0)
                      .ID()
                      .asBase58(ot_.Crypto()),
                  {
                      {"version 3 subchain: 0 of 1 (0.000000 %)",
                       Subchain::NotificationV3},
                  }},
             }},
        }};
    wait_for_counter(account_status_, false);

    EXPECT_TRUE(check_blockchain_account_status(alice_, test_chain_, expected));
    EXPECT_TRUE(
        check_blockchain_account_status_qt(alice_, test_chain_, expected));
}

TEST_F(Regtest_fixture_hd, txodb_initial) { EXPECT_TRUE(CheckTXODB()); }

TEST_F(Regtest_fixture_hd, generate)
{
    constexpr auto orphan{0};
    constexpr auto count{1};
    const auto start = height_ - orphan;
    const auto end{start + count};
    auto future1 = listener_.get_future(SendHD(), Subchain::External, end);
    auto future2 = listener_.get_future(SendHD(), Subchain::Internal, end);
    account_list_.expected_ += 0;
    account_activity_.expected_ += (2u * count) + 1u;
    account_status_.expected_ += (3u * count);

    EXPECT_EQ(start, 0);
    EXPECT_EQ(end, 1);
    EXPECT_TRUE(Mine(start, count, hd_generator_));
    EXPECT_TRUE(listener_.wait(future1));
    EXPECT_TRUE(listener_.wait(future2));
    EXPECT_TRUE(txos_.Mature(end));
}

TEST_F(Regtest_fixture_hd, first_block)
{
    const auto handle = client_1_.Network().Blockchain().GetChain(test_chain_);

    ASSERT_TRUE(handle);

    const auto& blockchain = handle.get();
    const auto blockHash = blockchain.HeaderOracle().BestHash(1);

    ASSERT_FALSE(blockHash.IsNull());

    const auto pBlock = blockchain.BlockOracle().LoadBitcoin(blockHash).get();

    ASSERT_TRUE(pBlock);

    const auto& block = *pBlock;

    ASSERT_EQ(block.size(), 1);

    const auto pTx = block.at(0);

    ASSERT_TRUE(pTx);

    const auto& tx = *pTx;

    EXPECT_EQ(tx.ID(), transactions_.at(0));
    EXPECT_EQ(tx.BlockPosition(), 0);
    EXPECT_EQ(tx.Outputs().size(), 100);
    EXPECT_TRUE(tx.IsGeneration());
}

TEST_F(Regtest_fixture_hd, account_activity_immature)
{
    const auto& id = SendHD().Parent().AccountID();
    const auto expected = AccountActivityData{
        expected_account_type_,
        id.asBase58(ot_.Crypto()),
        expected_account_name_,
        expected_unit_type_,
        expected_unit_.asBase58(ot_.Crypto()),
        expected_display_unit_,
        expected_notary_.asBase58(ot_.Crypto()),
        expected_notary_name_,
        0,
        0,
        u8"0 units"_cstr,
        "",
        {},
        {test_chain_},
        100,
        {height_, height_},
        {},
        {{u8"0"_cstr, u8"0 units"_cstr}},
        {
            {
                ot::otx::client::StorageBox::BLOCKCHAIN,
                1,
                10000004950,
                u8"100.000\u202F049\u202F5 units"_cstr,
                {},
                "",
                "",
                "Incoming Unit Test Simulation transaction",
                ot::blockchain::HashToNumber(transactions_.at(0)),
                std::nullopt,
                1,
            },
        },
    };
    wait_for_counter(account_activity_, false);

    EXPECT_TRUE(check_account_activity(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(alice_, id, expected));
}

TEST_F(Regtest_fixture_hd, account_list_immature)
{
    const auto expected = AccountListData{{
        {SendHD().Parent().AccountID().asBase58(ot_.Crypto()),
         expected_unit_.asBase58(ot_.Crypto()),
         expected_display_unit_,
         expected_account_name_,
         expected_notary_.asBase58(ot_.Crypto()),
         expected_notary_name_,
         expected_account_type_,
         expected_unit_type_,
         0,
         0,
         u8"0 units"_cstr},
    }};
    wait_for_counter(account_list_, false);

    EXPECT_TRUE(check_account_list(alice_, expected));
    EXPECT_TRUE(check_account_list_qt(alice_, expected));
    EXPECT_TRUE(check_account_list_rpc(alice_, expected));
}

TEST_F(Regtest_fixture_hd, account_status_immature)
{
    const auto expected = BlockchainAccountStatusData{
        alice_.nym_id_.asBase58(ot_.Crypto()),
        test_chain_,
        {
            {"Unnamed seed: BIP-39 (default)",
             alice_.seed_id_,
             Subaccount::HD,
             {
                 {"BIP-44: m / 44' / 1' / 0'",
                  SendHD().ID().asBase58(ot_.Crypto()),
                  {
                      {"external subchain: 1 of 1 (100.000000 %)",
                       Subchain::External},
                      {"internal subchain: 1 of 1 (100.000000 %)",
                       Subchain::Internal},
                  }},
             }},
            {alice_.payment_code_ + " (local)",
             alice_.nym_id_.asBase58(ot_.Crypto()),
             Subaccount::PaymentCode,
             {
                 {"Notification transactions",
                  Account(alice_, test_chain_)
                      .GetNotification()
                      .at(0)
                      .ID()
                      .asBase58(ot_.Crypto()),
                  {
                      {"version 3 subchain: 1 of 1 (100.000000 %)",
                       Subchain::NotificationV3},
                  }},
             }},
        }};
    wait_for_counter(account_status_, false);

    EXPECT_TRUE(check_blockchain_account_status(alice_, test_chain_, expected));
    EXPECT_TRUE(
        check_blockchain_account_status_qt(alice_, test_chain_, expected));
}

TEST_F(Regtest_fixture_hd, txodb_immature) { EXPECT_TRUE(CheckTXODB()); }

TEST_F(Regtest_fixture_hd, advance_test_chain_one_block_before_maturation)
{
    constexpr auto orphan{0};
    const auto count = static_cast<int>(MaturationInterval() - 1);
    const auto start = height_ - orphan;
    const auto end{start + count};
    auto future1 = listener_.get_future(SendHD(), Subchain::External, end);
    auto future2 = listener_.get_future(SendHD(), Subchain::Internal, end);
    account_list_.expected_ += 0;
    account_activity_.expected_ += (2u * count) + 1u;
    account_status_.expected_ += (6u * count);

    EXPECT_EQ(start, 1);
    EXPECT_EQ(end, 10);
    EXPECT_TRUE(Mine(start, count));
    EXPECT_TRUE(listener_.wait(future1));
    EXPECT_TRUE(listener_.wait(future2));
    EXPECT_TRUE(txos_.Mature(end));
}

TEST_F(Regtest_fixture_hd, account_activity_one_block_before_maturation)
{
    const auto& id = SendHD().Parent().AccountID();
    const auto expected = AccountActivityData{
        expected_account_type_,
        id.asBase58(ot_.Crypto()),
        expected_account_name_,
        expected_unit_type_,
        expected_unit_.asBase58(ot_.Crypto()),
        expected_display_unit_,
        expected_notary_.asBase58(ot_.Crypto()),
        expected_notary_name_,
        0,
        0,
        u8"0 units"_cstr,
        "",
        {},
        {test_chain_},
        100,
        {height_, height_},
        {},
        {{u8"0"_cstr, u8"0 units"_cstr}},
        {
            {
                ot::otx::client::StorageBox::BLOCKCHAIN,
                1,
                10000004950,
                u8"100.000\u202F049\u202F5 units"_cstr,
                {},
                "",
                "",
                "Incoming Unit Test Simulation transaction",
                ot::blockchain::HashToNumber(transactions_.at(0)),
                std::nullopt,
                10,
            },
        },
    };
    wait_for_counter(account_activity_, false);

    EXPECT_TRUE(check_account_activity(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(alice_, id, expected));
}

TEST_F(Regtest_fixture_hd, account_list_one_block_before_maturation)
{
    const auto expected = AccountListData{{
        {SendHD().Parent().AccountID().asBase58(ot_.Crypto()),
         expected_unit_.asBase58(ot_.Crypto()),
         expected_display_unit_,
         expected_account_name_,
         expected_notary_.asBase58(ot_.Crypto()),
         expected_notary_name_,
         expected_account_type_,
         expected_unit_type_,
         0,
         0,
         u8"0 units"_cstr},
    }};
    wait_for_counter(account_list_, false);

    EXPECT_TRUE(check_account_list(alice_, expected));
    EXPECT_TRUE(check_account_list_qt(alice_, expected));
    EXPECT_TRUE(check_account_list_rpc(alice_, expected));
}

TEST_F(Regtest_fixture_hd, account_status_one_block_before_maturation)
{
    const auto expected = BlockchainAccountStatusData{
        alice_.nym_id_.asBase58(ot_.Crypto()),
        test_chain_,
        {
            {"Unnamed seed: BIP-39 (default)",
             alice_.seed_id_,
             Subaccount::HD,
             {
                 {"BIP-44: m / 44' / 1' / 0'",
                  SendHD().ID().asBase58(ot_.Crypto()),
                  {
                      {"external subchain: 10 of 10 (100.000000 %)",
                       Subchain::External},
                      {"internal subchain: 10 of 10 (100.000000 %)",
                       Subchain::Internal},
                  }},
             }},
            {alice_.payment_code_ + " (local)",
             alice_.nym_id_.asBase58(ot_.Crypto()),
             Subaccount::PaymentCode,
             {
                 {"Notification transactions",
                  Account(alice_, test_chain_)
                      .GetNotification()
                      .at(0)
                      .ID()
                      .asBase58(ot_.Crypto()),
                  {
                      {"version 3 subchain: 10 of 10 (100.000000 %)",
                       Subchain::NotificationV3},
                  }},
             }},
        }};
    wait_for_counter(account_status_, false);

    EXPECT_TRUE(check_blockchain_account_status(alice_, test_chain_, expected));
    EXPECT_TRUE(
        check_blockchain_account_status_qt(alice_, test_chain_, expected));
}

TEST_F(Regtest_fixture_hd, txodb_one_block_before_maturation)
{
    EXPECT_TRUE(CheckTXODB());
}

TEST_F(Regtest_fixture_hd, mature)
{
    constexpr auto orphan{0};
    constexpr auto count{1};
    const auto start = height_ - orphan;
    const auto end{start + count};
    auto future1 = listener_.get_future(SendHD(), Subchain::External, end);
    auto future2 = listener_.get_future(SendHD(), Subchain::Internal, end);
    account_list_.expected_ += 1;
    account_activity_.expected_ += (2u * count) + 1u;
    account_status_.expected_ += (6u * count);

    EXPECT_EQ(start, 10);
    EXPECT_EQ(end, 11);
    EXPECT_TRUE(Mine(start, count));
    EXPECT_TRUE(listener_.wait(future1));
    EXPECT_TRUE(listener_.wait(future2));
    EXPECT_TRUE(txos_.Mature(end));
}

TEST_F(Regtest_fixture_hd, key_index)
{
    static constexpr auto count = 100u;
    static const auto baseAmount = ot::blockchain::Amount{100000000};
    const auto& account = SendHD();
    using Index = ot::Bip32Index;

    for (auto i = Index{0}; i < Index{count}; ++i) {
        using State = ot::blockchain::node::TxoState;
        using Tag = ot::blockchain::node::TxoTag;
        const auto& element = account.BalanceElement(Subchain::External, i);
        const auto key = element.KeyID();
        const auto handle =
            client_1_.Network().Blockchain().GetChain(test_chain_);

        ASSERT_TRUE(handle);

        const auto& chain = handle.get();
        const auto& wallet = chain.Wallet();
        const auto balance = wallet.GetBalance(key);
        const auto allOutputs = wallet.GetOutputs(key, State::All);
        const auto outputs = wallet.GetOutputs(key, State::ConfirmedNew);
        const auto& [confirmed, unconfirmed] = balance;

        EXPECT_EQ(confirmed, baseAmount + i);
        EXPECT_EQ(unconfirmed, baseAmount + i);
        EXPECT_EQ(allOutputs.size(), 1);
        EXPECT_EQ(outputs.size(), 1);

        for (const auto& outpoint : allOutputs) {
            const auto tags = wallet.GetTags(outpoint.first);

            EXPECT_EQ(tags.size(), 1);
            EXPECT_EQ(tags.count(Tag::Generation), 1);
        }
    }
}

TEST_F(Regtest_fixture_hd, account_activity_mature)
{
    const auto& id = SendHD().Parent().AccountID();
    const auto expected = AccountActivityData{
        expected_account_type_,
        id.asBase58(ot_.Crypto()),
        expected_account_name_,
        expected_unit_type_,
        expected_unit_.asBase58(ot_.Crypto()),
        expected_display_unit_,
        expected_notary_.asBase58(ot_.Crypto()),
        expected_notary_name_,
        1,
        10000004950,
        u8"100.000\u202F049\u202F5 units"_cstr,
        "",
        {},
        {test_chain_},
        100,
        {height_, height_},
        {},
        {{u8"100.0000495"_cstr, u8"100.000\u202F049\u202F5 units"_cstr}},
        {
            {
                ot::otx::client::StorageBox::BLOCKCHAIN,
                1,
                10000004950,
                u8"100.000\u202F049\u202F5 units"_cstr,
                {},
                "",
                "",
                "Incoming Unit Test Simulation transaction",
                ot::blockchain::HashToNumber(transactions_.at(0)),
                std::nullopt,
                11,
            },
        },
    };
    wait_for_counter(account_activity_, false);

    EXPECT_TRUE(check_account_activity(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(alice_, id, expected));
}

TEST_F(Regtest_fixture_hd, account_list_mature)
{
    const auto expected = AccountListData{{
        {SendHD().Parent().AccountID().asBase58(ot_.Crypto()),
         expected_unit_.asBase58(ot_.Crypto()),
         expected_display_unit_,
         expected_account_name_,
         expected_notary_.asBase58(ot_.Crypto()),
         expected_notary_name_,
         expected_account_type_,
         expected_unit_type_,
         1,
         10000004950,
         u8"100.000\u202F049\u202F5 units"_cstr},
    }};
    wait_for_counter(account_list_, false);

    EXPECT_TRUE(check_account_list(alice_, expected));
    EXPECT_TRUE(check_account_list_qt(alice_, expected));
    EXPECT_TRUE(check_account_list_rpc(alice_, expected));
}

TEST_F(Regtest_fixture_hd, account_status_mature)
{
    const auto expected = BlockchainAccountStatusData{
        alice_.nym_id_.asBase58(ot_.Crypto()),
        test_chain_,
        {
            {"Unnamed seed: BIP-39 (default)",
             alice_.seed_id_,
             Subaccount::HD,
             {
                 {"BIP-44: m / 44' / 1' / 0'",
                  SendHD().ID().asBase58(ot_.Crypto()),
                  {
                      {"external subchain: 11 of 11 (100.000000 %)",
                       Subchain::External},
                      {"internal subchain: 11 of 11 (100.000000 %)",
                       Subchain::Internal},
                  }},
             }},
            {alice_.payment_code_ + " (local)",
             alice_.nym_id_.asBase58(ot_.Crypto()),
             Subaccount::PaymentCode,
             {
                 {"Notification transactions",
                  Account(alice_, test_chain_)
                      .GetNotification()
                      .at(0)
                      .ID()
                      .asBase58(ot_.Crypto()),
                  {
                      {"version 3 subchain: 11 of 11 (100.000000 %)",
                       Subchain::NotificationV3},
                  }},
             }},
        }};
    wait_for_counter(account_status_, false);

    EXPECT_TRUE(check_blockchain_account_status(alice_, test_chain_, expected));
    EXPECT_TRUE(
        check_blockchain_account_status_qt(alice_, test_chain_, expected));
}

TEST_F(Regtest_fixture_hd, txodb_inital_mature) { EXPECT_TRUE(CheckTXODB()); }

TEST_F(Regtest_fixture_hd, failed_spend)
{
    account_list_.expected_ += 0;
    account_activity_.expected_ += 0;
    const auto handle = client_1_.Network().Blockchain().GetChain(test_chain_);

    ASSERT_TRUE(handle);

    const auto& network = handle.get();
    constexpr auto address{"mipcBbFg9gMiCh81Kj8tqqdgoZub1ZJRfn"};
    auto future = network.SendToAddress(
        alice_.nym_id_, address, 140000000000, memo_outgoing_);
    const auto txid = future.get().second;

    EXPECT_TRUE(txid.empty());

    // TODO ensure CancelProposal is finished processing with appropriate signal
    ot::Sleep(5s);
}

TEST_F(Regtest_fixture_hd, account_activity_failed_spend)
{
    const auto& id = SendHD().Parent().AccountID();
    const auto expected = AccountActivityData{
        expected_account_type_,
        id.asBase58(ot_.Crypto()),
        expected_account_name_,
        expected_unit_type_,
        expected_unit_.asBase58(ot_.Crypto()),
        expected_display_unit_,
        expected_notary_.asBase58(ot_.Crypto()),
        expected_notary_name_,
        1,
        10000004950,
        u8"100.000\u202F049\u202F5 units"_cstr,
        "",
        {},
        {test_chain_},
        100,
        {height_, height_},
        {},
        {{u8"100.0000495"_cstr, u8"100.000\u202F049\u202F5 units"_cstr}},
        {
            {
                ot::otx::client::StorageBox::BLOCKCHAIN,
                1,
                10000004950,
                u8"100.000\u202F049\u202F5 units"_cstr,
                {},
                "",
                "",
                "Incoming Unit Test Simulation transaction",
                ot::blockchain::HashToNumber(transactions_.at(0)),
                std::nullopt,
                11,
            },
        },
    };
    wait_for_counter(account_activity_, false);

    EXPECT_TRUE(check_account_activity(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(alice_, id, expected));
}

TEST_F(Regtest_fixture_hd, account_list_failed_spend)
{
    const auto expected = AccountListData{{
        {SendHD().Parent().AccountID().asBase58(ot_.Crypto()),
         expected_unit_.asBase58(ot_.Crypto()),
         expected_display_unit_,
         expected_account_name_,
         expected_notary_.asBase58(ot_.Crypto()),
         expected_notary_name_,
         expected_account_type_,
         expected_unit_type_,
         1,
         10000004950,
         u8"100.000\u202F049\u202F5 units"_cstr},
    }};
    wait_for_counter(account_list_, false);

    EXPECT_TRUE(check_account_list(alice_, expected));
    EXPECT_TRUE(check_account_list_qt(alice_, expected));
    EXPECT_TRUE(check_account_list_rpc(alice_, expected));
}

TEST_F(Regtest_fixture_hd, txodb_failed_spend) { EXPECT_TRUE(CheckTXODB()); }

TEST_F(Regtest_fixture_hd, spend)
{
    account_list_.expected_ += 1;
    account_activity_.expected_ += 2;
    const auto handle = client_1_.Network().Blockchain().GetChain(test_chain_);

    ASSERT_TRUE(handle);

    const auto& network = handle.get();
    const auto& widget = client_1_.UI().Internal().AccountActivity(
        alice_.nym_id_, SendHD().Parent().AccountID());
    constexpr auto sendAmount{"14 units"};
    constexpr auto address{"mipcBbFg9gMiCh81Kj8tqqdgoZub1ZJRfn"};

    ASSERT_FALSE(widget.ValidateAmount(sendAmount).empty());
    ASSERT_TRUE(widget.ValidateAddress(address));

    auto future = network.SendToAddress(
        alice_.nym_id_, address, 1400000000, memo_outgoing_);
    const auto& txid = transactions_.emplace_back(future.get().second);

    EXPECT_FALSE(txid.empty());

    const auto& element = SendHD().BalanceElement(Subchain::Internal, 0);
    const auto amount = ot::blockchain::Amount{99997807};
    expected_.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(txid.Bytes(), 0),
        std::forward_as_tuple(
            element.PubkeyHash(), amount, Pattern::PayToPubkeyHash));
    const auto pTX =
        client_1_.Crypto().Blockchain().LoadTransactionBitcoin(txid);

    ASSERT_TRUE(pTX);

    const auto& tx = *pTX;

    for (const auto& input : tx.Inputs()) {
        EXPECT_TRUE(txos_.SpendUnconfirmed(input.PreviousOutput()));
    }

    EXPECT_TRUE(txos_.AddUnconfirmed(tx, 0, SendHD()));
}

TEST_F(Regtest_fixture_hd, account_activity_unconfirmed_spend)
{
    const auto& id = SendHD().Parent().AccountID();
    const auto expected = AccountActivityData{
        expected_account_type_,
        id.asBase58(ot_.Crypto()),
        expected_account_name_,
        expected_unit_type_,
        expected_unit_.asBase58(ot_.Crypto()),
        expected_display_unit_,
        expected_notary_.asBase58(ot_.Crypto()),
        expected_notary_name_,
        1,
        8600002652,
        u8"86.000\u202F026\u202F52 units"_cstr,
        "",
        {},
        {test_chain_},
        100,
        {height_, height_},
        {},
        {{u8"86.00002652"_cstr, u8"86.000\u202F026\u202F52 units"_cstr}},
        {
            {
                ot::otx::client::StorageBox::BLOCKCHAIN,
                -1,
                -1400002298,
                u8"-14.000\u202F022\u202F98 units"_cstr,
                {},
                "",
                "",
                "Outgoing Unit Test Simulation transaction",
                ot::blockchain::HashToNumber(transactions_.at(1)),
                std::nullopt,
                0,
            },
            {
                ot::otx::client::StorageBox::BLOCKCHAIN,
                1,
                10000004950,
                u8"100.000\u202F049\u202F5 units"_cstr,
                {},
                "",
                "",
                "Incoming Unit Test Simulation transaction",
                ot::blockchain::HashToNumber(transactions_.at(0)),
                std::nullopt,
                11,
            },
        },
    };
    wait_for_counter(account_activity_, false);

    EXPECT_TRUE(check_account_activity(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(alice_, id, expected));
}

TEST_F(Regtest_fixture_hd, account_list_unconfirmed_spend)
{
    const auto expected = AccountListData{{
        {SendHD().Parent().AccountID().asBase58(ot_.Crypto()),
         expected_unit_.asBase58(ot_.Crypto()),
         expected_display_unit_,
         expected_account_name_,
         expected_notary_.asBase58(ot_.Crypto()),
         expected_notary_name_,
         expected_account_type_,
         expected_unit_type_,
         1,
         8600002652,
         u8"86.000\u202F026\u202F52 units"_cstr},
    }};
    wait_for_counter(account_list_, false);

    EXPECT_TRUE(check_account_list(alice_, expected));
    EXPECT_TRUE(check_account_list_qt(alice_, expected));
    EXPECT_TRUE(check_account_list_rpc(alice_, expected));
}

TEST_F(Regtest_fixture_hd, txodb_unconfirmed_spend)
{
    EXPECT_TRUE(CheckTXODB());
}

TEST_F(Regtest_fixture_hd, confirm)
{
    constexpr auto orphan{0};
    constexpr auto count{1};
    const auto start = height_ - orphan;
    const auto end{start + count};
    auto future1 = listener_.get_future(SendHD(), Subchain::External, end);
    auto future2 = listener_.get_future(SendHD(), Subchain::Internal, end);
    account_list_.expected_ += 2;
    account_activity_.expected_ += ((3 * count) + 5);
    account_status_.expected_ += (6u * count);
    const auto& txid = transactions_.at(1);
    const auto extra = [&] {
        auto output = ot::UnallocatedVector<Transaction>{};
        const auto& pTX = output.emplace_back(
            client_1_.Crypto().Blockchain().LoadTransactionBitcoin(txid));

        OT_ASSERT(pTX);

        return output;
    }();

    EXPECT_EQ(start, 11);
    EXPECT_EQ(end, 12);
    EXPECT_TRUE(Mine(start, count, default_, extra));
    EXPECT_TRUE(listener_.wait(future1));
    EXPECT_TRUE(listener_.wait(future2));
    EXPECT_TRUE(txos_.Mature(end));
    EXPECT_TRUE(txos_.Confirm(transactions_.at(0)));
    EXPECT_TRUE(txos_.Confirm(txid));
}

TEST_F(Regtest_fixture_hd, outgoing_transaction)
{
    const auto pTX = client_1_.Crypto().Blockchain().LoadTransactionBitcoin(
        transactions_.at(1));

    ASSERT_TRUE(pTX);

    const auto& tx = *pTX;

    EXPECT_FALSE(tx.IsGeneration());

    const auto handle = client_1_.Network().Blockchain().GetChain(test_chain_);

    ASSERT_TRUE(handle);

    const auto& chain = handle.get();
    const auto& wallet = chain.Wallet();
    using Tag = ot::blockchain::node::TxoTag;

    {
        const auto tags = wallet.GetTags({transactions_.at(1).Bytes(), 0});

        EXPECT_EQ(tags.size(), 2);
        EXPECT_EQ(tags.count(Tag::Normal), 1);
        EXPECT_EQ(tags.count(Tag::Change), 1);
    }
    {
        const auto tags = wallet.GetTags({transactions_.at(1).Bytes(), 1});

        EXPECT_EQ(tags.size(), 0);
    }
}

TEST_F(Regtest_fixture_hd, account_activity_confirmed_spend)
{
    const auto& id = SendHD().Parent().AccountID();
    const auto expected = AccountActivityData{
        expected_account_type_,
        id.asBase58(ot_.Crypto()),
        expected_account_name_,
        expected_unit_type_,
        expected_unit_.asBase58(ot_.Crypto()),
        expected_display_unit_,
        expected_notary_.asBase58(ot_.Crypto()),
        expected_notary_name_,
        1,
        8600002652,
        u8"86.000\u202F026\u202F52 units"_cstr,
        "",
        {},
        {test_chain_},
        100,
        {height_, height_},
        {},
        {{u8"86.00002652"_cstr, u8"86.000\u202F026\u202F52 units"_cstr}},
        {
            {
                ot::otx::client::StorageBox::BLOCKCHAIN,
                -1,
                -1400002298,
                u8"-14.000\u202F022\u202F98 units"_cstr,
                {},
                "",
                "",
                "Outgoing Unit Test Simulation transaction",
                ot::blockchain::HashToNumber(transactions_.at(1)),
                std::nullopt,
                1,
            },
            {
                ot::otx::client::StorageBox::BLOCKCHAIN,
                1,
                10000004950,
                u8"100.000\u202F049\u202F5 units"_cstr,
                {},
                "",
                "",
                "Incoming Unit Test Simulation transaction",
                ot::blockchain::HashToNumber(transactions_.at(0)),
                std::nullopt,
                12,
            },
        },
    };
    wait_for_counter(account_activity_, false);

    EXPECT_TRUE(check_account_activity(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(alice_, id, expected));
}

TEST_F(Regtest_fixture_hd, account_list_confirmed_spend)
{
    const auto expected = AccountListData{{
        {SendHD().Parent().AccountID().asBase58(ot_.Crypto()),
         expected_unit_.asBase58(ot_.Crypto()),
         expected_display_unit_,
         expected_account_name_,
         expected_notary_.asBase58(ot_.Crypto()),
         expected_notary_name_,
         expected_account_type_,
         expected_unit_type_,
         1,
         8600002652,
         u8"86.000\u202F026\u202F52 units"_cstr},
    }};
    wait_for_counter(account_list_, false);

    EXPECT_TRUE(check_account_list(alice_, expected));
    EXPECT_TRUE(check_account_list_qt(alice_, expected));
    EXPECT_TRUE(check_account_list_rpc(alice_, expected));
}

TEST_F(Regtest_fixture_hd, account_status_confirmed_spend)
{
    const auto expected = BlockchainAccountStatusData{
        alice_.nym_id_.asBase58(ot_.Crypto()),
        test_chain_,
        {
            {"Unnamed seed: BIP-39 (default)",
             alice_.seed_id_,
             Subaccount::HD,
             {
                 {"BIP-44: m / 44' / 1' / 0'",
                  SendHD().ID().asBase58(ot_.Crypto()),
                  {
                      {"external subchain: 12 of 12 (100.000000 %)",
                       Subchain::External},
                      {"internal subchain: 12 of 12 (100.000000 %)",
                       Subchain::Internal},
                  }},
             }},
            {alice_.payment_code_ + " (local)",
             alice_.nym_id_.asBase58(ot_.Crypto()),
             Subaccount::PaymentCode,
             {
                 {"Notification transactions",
                  Account(alice_, test_chain_)
                      .GetNotification()
                      .at(0)
                      .ID()
                      .asBase58(ot_.Crypto()),
                  {
                      {"version 3 subchain: 12 of 12 (100.000000 %)",
                       Subchain::NotificationV3},
                  }},
             }},
        }};

    wait_for_counter(account_status_, false);
    EXPECT_TRUE(check_blockchain_account_status(alice_, test_chain_, expected));
    EXPECT_TRUE(
        check_blockchain_account_status_qt(alice_, test_chain_, expected));
}

TEST_F(Regtest_fixture_hd, txodb_confirmed_spend) { EXPECT_TRUE(CheckTXODB()); }

// TEST_F(Regtest_fixture_hd, reorg_matured_coins)
// {
//     constexpr auto orphan{12};
//     constexpr auto count{13};
//     const auto start = height_ - orphan;
//     const auto end{start + count};
//     auto future1 = listener_.get_future(SendHD(), Subchain::External, end);
//     auto future2 = listener_.get_future(SendHD(), Subchain::Internal, end);
//     account_list_.expected_ += 0;
//     account_activity_.expected_ += 4;
//
//     EXPECT_EQ(start, 0);
//     EXPECT_EQ(end, 13);
//     EXPECT_TRUE(Mine(start, count));
//     EXPECT_TRUE(listener_.wait(future1));
//     EXPECT_TRUE(listener_.wait(future2));
//     EXPECT_TRUE(txos_.OrphanGeneration(transactions_.at(0)));
//     EXPECT_TRUE(txos_.Orphan(transactions_.at(1)));
//     EXPECT_TRUE(txos_.Mature(end));
// }

// TODO balances are not correctly calculated when ancestor transactions are
// invalidated by conflicts

TEST_F(Regtest_fixture_hd, shutdown)
{
    EXPECT_EQ(account_list_.expected_, account_list_.updated_);
    EXPECT_EQ(account_activity_.expected_, account_activity_.updated_);
    // TODO EXPECT_EQ(account_status_.expected_, account_status_.updated_);

    Shutdown();
}
}  // namespace ottest
