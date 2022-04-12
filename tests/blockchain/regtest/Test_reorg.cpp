// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Helpers.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <chrono>
#include <utility>

#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/bitcoin/Outputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/node/BlockOracle.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/blockchain/node/TxoState.hpp"
#include "opentxs/blockchain/node/TxoTag.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/otx/client/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "rpc/Helpers.hpp"
#include "ui/Helpers.hpp"

namespace ottest
{
using namespace std::literals::chrono_literals;

Counter account_list_{};
Counter account_activity_{};

TEST_F(Regtest_fixture_hd, init_opentxs) {}

TEST_F(Regtest_fixture_hd, start_chains) { EXPECT_TRUE(Start()); }

TEST_F(Regtest_fixture_hd, connect_peers) { EXPECT_TRUE(Connect()); }

TEST_F(Regtest_fixture_hd, init_ui_models)
{
    account_activity_.expected_ += 0;
    account_list_.expected_ += 1;
    init_account_activity(
        alice_, SendHD().Parent().AccountID(), account_activity_);
    init_account_list(alice_, account_list_);
}

TEST_F(Regtest_fixture_hd, account_activity_initial)
{
    const auto& id = SendHD().Parent().AccountID();
    const auto expected = AccountActivityData{
        expected_account_type_,
        id.str(),
        expected_account_name_,
        expected_unit_type_,
        expected_unit_.str(),
        expected_display_unit_,
        expected_notary_.str(),
        expected_notary_name_,
        0,
        0,
        u8"0 units",
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
        {{u8"0", u8"0 units"},
         {u8"10", u8"10 units"},
         {u8"25", u8"25 units"},
         {u8"300", u8"300 units"},
         {u8"4000", u8"4,000 units"},
         {u8"50000", u8"50,000 units"},
         {u8"600000", u8"600,000 units"},
         {u8"7000000", u8"7,000,000 units"},
         {u8"1000000000000000001", u8"1,000,000,000,000,000,001 units"},
         {u8"74.99999448", u8"74.999\u202F994\u202F48 units"},
         {u8"86.00002652", u8"86.000\u202F026\u202F52 units"},
         {u8"89.99999684", u8"89.999\u202F996\u202F84 units"},
         {u8"100.0000495", u8"100.000\u202F049\u202F5 units"}},
        {},
    };

    EXPECT_TRUE(wait_for_counter(account_activity_));
    EXPECT_TRUE(check_account_activity(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(alice_, id, expected));
}

TEST_F(Regtest_fixture_hd, account_list_initial)
{
    const auto expected = AccountListData{{
        {SendHD().Parent().AccountID().str(),
         expected_unit_.str(),
         expected_display_unit_,
         expected_account_name_,
         expected_notary_.str(),
         expected_notary_name_,
         expected_account_type_,
         expected_unit_type_,
         0,
         0,
         u8"0 units"},
    }};

    EXPECT_TRUE(wait_for_counter(account_list_));
    EXPECT_TRUE(check_account_list(alice_, expected));
    EXPECT_TRUE(check_account_list_qt(alice_, expected));
    EXPECT_TRUE(check_account_list_rpc(alice_, expected));
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
    account_activity_.expected_ += (count + 1);

    EXPECT_EQ(start, 0);
    EXPECT_EQ(end, 1);
    EXPECT_TRUE(Mine(start, count, hd_generator_));
    EXPECT_TRUE(listener_.wait(future1));
    EXPECT_TRUE(listener_.wait(future2));
    EXPECT_TRUE(txos_.Mature(end));
}

TEST_F(Regtest_fixture_hd, first_block)
{
    const auto& blockchain =
        client_1_.Network().Blockchain().GetChain(test_chain_);
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
        id.str(),
        expected_account_name_,
        expected_unit_type_,
        expected_unit_.str(),
        expected_display_unit_,
        expected_notary_.str(),
        expected_notary_name_,
        0,
        0,
        u8"0 units",
        "",
        {},
        {test_chain_},
        100,
        {height_, height_},
        {},
        {{u8"0", u8"0 units"}},
        {
            {
                ot::otx::client::StorageBox::BLOCKCHAIN,
                1,
                10000004950,
                u8"100.000\u202F049\u202F5 units",
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

    EXPECT_TRUE(wait_for_counter(account_activity_));
    EXPECT_TRUE(check_account_activity(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(alice_, id, expected));
}

TEST_F(Regtest_fixture_hd, account_list_immature)
{
    const auto expected = AccountListData{{
        {SendHD().Parent().AccountID().str(),
         expected_unit_.str(),
         expected_display_unit_,
         expected_account_name_,
         expected_notary_.str(),
         expected_notary_name_,
         expected_account_type_,
         expected_unit_type_,
         0,
         0,
         u8"0 units"},
    }};

    EXPECT_TRUE(wait_for_counter(account_list_));
    EXPECT_TRUE(check_account_list(alice_, expected));
    EXPECT_TRUE(check_account_list_qt(alice_, expected));
    EXPECT_TRUE(check_account_list_rpc(alice_, expected));
}

TEST_F(Regtest_fixture_hd, txodb_immature) { EXPECT_TRUE(CheckTXODB()); }

TEST_F(Regtest_fixture_hd, mature)
{
    constexpr auto orphan{0};
    const auto count = static_cast<int>(MaturationInterval()) + 1;
    const auto start = height_ - orphan;
    const auto end{start + count};
    auto future1 = listener_.get_future(SendHD(), Subchain::External, end);
    auto future2 = listener_.get_future(SendHD(), Subchain::Internal, end);
    account_list_.expected_ += 1;
    account_activity_.expected_ += ((2u * count) + 1);

    EXPECT_EQ(start, 1);
    EXPECT_EQ(end, 12);
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
        const auto& chain =
            client_1_.Network().Blockchain().GetChain(test_chain_);
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
        id.str(),
        expected_account_name_,
        expected_unit_type_,
        expected_unit_.str(),
        expected_display_unit_,
        expected_notary_.str(),
        expected_notary_name_,
        1,
        10000004950,
        u8"100.000\u202F049\u202F5 units",
        "",
        {},
        {test_chain_},
        100,
        {height_, height_},
        {},
        {{u8"100.0000495", u8"100.000\u202F049\u202F5 units"}},
        {
            {
                ot::otx::client::StorageBox::BLOCKCHAIN,
                1,
                10000004950,
                u8"100.000\u202F049\u202F5 units",
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

    EXPECT_TRUE(wait_for_counter(account_activity_));
    EXPECT_TRUE(check_account_activity(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(alice_, id, expected));
}

TEST_F(Regtest_fixture_hd, account_list_mature)
{
    const auto expected = AccountListData{{
        {SendHD().Parent().AccountID().str(),
         expected_unit_.str(),
         expected_display_unit_,
         expected_account_name_,
         expected_notary_.str(),
         expected_notary_name_,
         expected_account_type_,
         expected_unit_type_,
         1,
         10000004950,
         u8"100.000\u202F049\u202F5 units"},
    }};

    EXPECT_TRUE(wait_for_counter(account_list_));
    EXPECT_TRUE(check_account_list(alice_, expected));
    EXPECT_TRUE(check_account_list_qt(alice_, expected));
    EXPECT_TRUE(check_account_list_rpc(alice_, expected));
}

TEST_F(Regtest_fixture_hd, txodb_inital_mature) { EXPECT_TRUE(CheckTXODB()); }

TEST_F(Regtest_fixture_hd, reorg)
{
    constexpr auto orphan{1};
    constexpr auto count{2};
    const auto start = height_ - orphan;
    const auto end{start + count};
    auto future1 = listener_.get_future(SendHD(), Subchain::External, end);
    auto future2 = listener_.get_future(SendHD(), Subchain::Internal, end);
    account_list_.expected_ += 0;
    account_activity_.expected_ += 3;

    EXPECT_EQ(start, 11);
    EXPECT_EQ(end, 13);
    EXPECT_TRUE(Mine(start, count));
    EXPECT_TRUE(listener_.wait(future1));
    EXPECT_TRUE(listener_.wait(future2));
}

TEST_F(Regtest_fixture_hd, account_activity_reorg)
{
    const auto& id = SendHD().Parent().AccountID();
    const auto expected = AccountActivityData{
        expected_account_type_,
        id.str(),
        expected_account_name_,
        expected_unit_type_,
        expected_unit_.str(),
        expected_display_unit_,
        expected_notary_.str(),
        expected_notary_name_,
        1,
        10000004950,
        u8"100.000\u202F049\u202F5 units",
        "",
        {},
        {test_chain_},
        100,
        {height_, height_},
        {},
        {{u8"100.0000495", u8"100.000\u202F049\u202F5 units"}},
        {
            {
                ot::otx::client::StorageBox::BLOCKCHAIN,
                1,
                10000004950,
                u8"100.000\u202F049\u202F5 units",
                {},
                "",
                "",
                "Incoming Unit Test Simulation transaction",
                ot::blockchain::HashToNumber(transactions_.at(0)),
                std::nullopt,
                13,
            },
        },
    };

    EXPECT_TRUE(wait_for_counter(account_activity_));
    EXPECT_TRUE(check_account_activity(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(alice_, id, expected));
}

TEST_F(Regtest_fixture_hd, account_list_reorg)
{
    const auto expected = AccountListData{{
        {SendHD().Parent().AccountID().str(),
         expected_unit_.str(),
         expected_display_unit_,
         expected_account_name_,
         expected_notary_.str(),
         expected_notary_name_,
         expected_account_type_,
         expected_unit_type_,
         1,
         10000004950,
         u8"100.000\u202F049\u202F5 units"},
    }};

    EXPECT_TRUE(wait_for_counter(account_list_));
    EXPECT_TRUE(check_account_list(alice_, expected));
    EXPECT_TRUE(check_account_list_qt(alice_, expected));
    EXPECT_TRUE(check_account_list_rpc(alice_, expected));
}

TEST_F(Regtest_fixture_hd, txodb_reorg) { EXPECT_TRUE(CheckTXODB()); }

TEST_F(Regtest_fixture_hd, advance_after_reorg)
{
    constexpr auto orphan{0};
    const auto count = 5;
    const auto start = height_ - orphan;
    const auto end{start + count};
    auto future1 = listener_.get_future(SendHD(), Subchain::External, end);
    auto future2 = listener_.get_future(SendHD(), Subchain::Internal, end);
    account_list_.expected_ += 0;
    account_activity_.expected_ += (2u * count);

    EXPECT_EQ(start, 13);
    EXPECT_EQ(end, 18);
    EXPECT_TRUE(Mine(start, count));
    EXPECT_TRUE(listener_.wait(future1));
    EXPECT_TRUE(listener_.wait(future2));
    EXPECT_TRUE(txos_.Mature(end));
}

TEST_F(Regtest_fixture_hd, account_activity_final)
{
    const auto& id = SendHD().Parent().AccountID();
    const auto expected = AccountActivityData{
        expected_account_type_,
        id.str(),
        expected_account_name_,
        expected_unit_type_,
        expected_unit_.str(),
        expected_display_unit_,
        expected_notary_.str(),
        expected_notary_name_,
        1,
        10000004950,
        u8"100.000\u202F049\u202F5 units",
        "",
        {},
        {test_chain_},
        100,
        {height_, height_},
        {},
        {{u8"100.0000495", u8"100.000\u202F049\u202F5 units"}},
        {
            {
                ot::otx::client::StorageBox::BLOCKCHAIN,
                1,
                10000004950,
                u8"100.000\u202F049\u202F5 units",
                {},
                "",
                "",
                "Incoming Unit Test Simulation transaction",
                ot::blockchain::HashToNumber(transactions_.at(0)),
                std::nullopt,
                18,
            },
        },
    };

    EXPECT_TRUE(wait_for_counter(account_activity_));
    EXPECT_TRUE(check_account_activity(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(alice_, id, expected));
}

TEST_F(Regtest_fixture_hd, account_list_final)
{
    const auto expected = AccountListData{{
        {SendHD().Parent().AccountID().str(),
         expected_unit_.str(),
         expected_display_unit_,
         expected_account_name_,
         expected_notary_.str(),
         expected_notary_name_,
         expected_account_type_,
         expected_unit_type_,
         1,
         10000004950,
         u8"100.000\u202F049\u202F5 units"},
    }};

    EXPECT_TRUE(wait_for_counter(account_list_));
    EXPECT_TRUE(check_account_list(alice_, expected));
    EXPECT_TRUE(check_account_list_qt(alice_, expected));
    EXPECT_TRUE(check_account_list_rpc(alice_, expected));
}

TEST_F(Regtest_fixture_hd, txodb_final) { EXPECT_TRUE(CheckTXODB()); }

TEST_F(Regtest_fixture_hd, shutdown)
{
    EXPECT_EQ(account_list_.expected_, account_list_.updated_);
    EXPECT_EQ(account_activity_.expected_, account_activity_.updated_);

    Shutdown();
}
}  // namespace ottest
