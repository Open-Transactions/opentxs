// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <atomic>
#include <future>
#include <memory>
#include <optional>
#include <span>
#include <utility>

#include "ottest/fixtures/blockchain/Common.hpp"
#include "ottest/fixtures/blockchain/FaucetListener.hpp"
#include "ottest/fixtures/blockchain/ScanListener.hpp"
#include "ottest/fixtures/blockchain/TXOs.hpp"
#include "ottest/fixtures/blockchain/regtest/PaymentCode.hpp"
#include "ottest/fixtures/common/Counter.hpp"
#include "ottest/fixtures/common/User.hpp"
#include "ottest/fixtures/integration/Helpers.hpp"
#include "ottest/fixtures/rpc/Helpers.hpp"
#include "ottest/fixtures/ui/AccountActivity.hpp"
#include "ottest/fixtures/ui/AccountList.hpp"
#include "ottest/fixtures/ui/AccountTree.hpp"
#include "ottest/fixtures/ui/ActivityThread.hpp"
#include "ottest/fixtures/ui/ContactList.hpp"

namespace ottest
{
using namespace opentxs::literals;

Counter account_activity_alex_{};
Counter account_activity_bob_{};
Counter account_list_alex_{};
Counter account_list_bob_{};
Counter account_tree_alex_{};
Counter account_tree_bob_{};
Counter activity_thread_alex_bob_{};
Counter activity_thread_bob_alex_{};
Counter contact_list_alex_{};
Counter contact_list_bob_{};

TEST_F(Regtest_payment_code, init_opentxs) {}

TEST_F(Regtest_payment_code, start_chains) { EXPECT_TRUE(Start()); }

TEST_F(Regtest_payment_code, connect_peers) { EXPECT_TRUE(Connect()); }

TEST_F(Regtest_payment_code, init_ui_models)
{
    account_activity_alex_.expected_ += 0;
    account_activity_bob_.expected_ += 0;
    account_list_alex_.expected_ += 1;
    account_list_bob_.expected_ += 1;
    account_tree_alex_.expected_ += 2;
    account_tree_bob_.expected_ += 2;
    contact_list_alex_.expected_ += 1;
    contact_list_bob_.expected_ += 1;
    init_account_activity(
        alex_, SendHD().Parent().AccountID(), account_activity_alex_);
    init_account_list(alex_, account_list_alex_);
    init_account_tree(alex_, account_tree_alex_);
    init_contact_list(alex_, contact_list_alex_);

    init_account_activity(
        bob_, ReceiveHD().Parent().AccountID(), account_activity_bob_);
    init_account_list(bob_, account_list_bob_);
    init_account_tree(bob_, account_tree_bob_);
    init_contact_list(bob_, contact_list_bob_);
}

TEST_F(Regtest_payment_code, alex_contact_list_initial)
{
    const auto expected = ContactListData{{
        {true, alex_.name_, alex_.name_, "ME", ""},
    }};
    wait_for_counter(contact_list_alex_, false);

    EXPECT_TRUE(check_contact_list(alex_, expected));
    EXPECT_TRUE(check_contact_list_qt(alex_, expected));
}

TEST_F(Regtest_payment_code, alex_account_activity_initial)
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
        {},
        {{u8"0"_cstr, u8"0 units"_cstr}},
        {},
    };
    wait_for_counter(account_activity_alex_, false);

    EXPECT_TRUE(check_account_activity(alex_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(alex_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(alex_, id, expected));
}

TEST_F(Regtest_payment_code, alex_account_list_initial)
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
    wait_for_counter(account_list_alex_, false);

    EXPECT_TRUE(check_account_list(alex_, expected));
    EXPECT_TRUE(check_account_list_qt(alex_, expected));
    EXPECT_TRUE(check_account_list_rpc(alex_, expected));
}

TEST_F(Regtest_payment_code, alex_account_tree_initial)
{
    const auto expected = AccountTreeData{
        {{expected_unit_type_,
          expected_notary_name_,
          {
              {SendHD().Parent().AccountID().asBase58(ot_.Crypto()),
               expected_unit_.asBase58(ot_.Crypto()),
               expected_display_unit_,
               expected_account_name_,
               expected_notary_.asBase58(ot_.Crypto()),
               expected_notary_name_,
               ot::AccountType::Blockchain,
               expected_unit_type_,
               0,
               0,
               "0 units"},
          }}}};
    wait_for_counter(account_tree_alex_, false);

    EXPECT_TRUE(check_account_tree(alex_, expected));
    EXPECT_TRUE(check_account_tree_qt(alex_, expected));
}

TEST_F(Regtest_payment_code, bob_contact_list_initial)
{
    const auto expected = ContactListData{{
        {true, bob_.name_, bob_.name_, "ME", ""},
    }};
    wait_for_counter(contact_list_bob_, false);

    EXPECT_TRUE(check_contact_list(bob_, expected));
    EXPECT_TRUE(check_contact_list_qt(bob_, expected));
}

TEST_F(Regtest_payment_code, bob_account_activity_initial)
{
    const auto& id = ReceiveHD().Parent().AccountID();
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
        {},
        {{u8"0"_cstr, u8"0 units"_cstr}},
        {},
    };
    wait_for_counter(account_activity_bob_, false);

    EXPECT_TRUE(check_account_activity(bob_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(bob_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(bob_, id, expected));
}

TEST_F(Regtest_payment_code, bob_account_list_initial)
{
    const auto expected = AccountListData{{
        {ReceiveHD().Parent().AccountID().asBase58(ot_.Crypto()),
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
    wait_for_counter(account_list_bob_, false);

    EXPECT_TRUE(check_account_list(bob_, expected));
    EXPECT_TRUE(check_account_list_qt(bob_, expected));
    EXPECT_TRUE(check_account_list_rpc(bob_, expected));
}

TEST_F(Regtest_payment_code, bob_account_tree_initial)
{
    const auto expected = AccountTreeData{
        {{expected_unit_type_,
          expected_notary_name_,
          {
              {ReceiveHD().Parent().AccountID().asBase58(ot_.Crypto()),
               expected_unit_.asBase58(ot_.Crypto()),
               expected_display_unit_,
               expected_account_name_,
               expected_notary_.asBase58(ot_.Crypto()),
               expected_notary_name_,
               ot::AccountType::Blockchain,
               expected_unit_type_,
               0,
               0,
               "0 units"},
          }}}};
    wait_for_counter(account_tree_bob_, false);

    EXPECT_TRUE(check_account_tree(bob_, expected));
    EXPECT_TRUE(check_account_tree_qt(bob_, expected));
}

TEST_F(Regtest_payment_code, mine_initial_balance)
{
    constexpr auto orphan{0};
    constexpr auto count{1};
    const auto start = height_ - orphan;
    const auto end{start + count};
    auto future = listener_alex_.get_future(SendHD(), Subchain::External, end);
    account_activity_alex_.expected_ += (count + 2);
    account_activity_bob_.expected_ += (count + 1);
    account_list_alex_.expected_ += 0;
    account_list_bob_.expected_ += 0;
    account_tree_alex_.expected_ += 0;
    account_tree_bob_.expected_ += 0;

    EXPECT_EQ(start, 0);
    EXPECT_EQ(end, 1);
    EXPECT_TRUE(Mine(start, count, mine_to_alex_));
    EXPECT_TRUE(listener_alex_.wait(future));
    EXPECT_TRUE(txos_alex_.Mature(end));
    EXPECT_TRUE(txos_bob_.Mature(end));
}

TEST_F(Regtest_payment_code, mature_initial_balance)
{
    constexpr auto orphan{0};
    const auto count = static_cast<int>(MaturationInterval());
    const auto start = height_ - orphan;
    const auto end{start + count};
    auto future = listener_alex_.get_future(SendHD(), Subchain::External, end);
    account_activity_alex_.expected_ += ((count * 2) + 1);
    account_activity_bob_.expected_ += count;
    account_list_alex_.expected_ += 1;
    account_list_bob_.expected_ += 0;
    account_tree_alex_.expected_ += 1;
    account_tree_bob_.expected_ += 0;

    EXPECT_EQ(start, 1);
    EXPECT_EQ(end, 11);
    EXPECT_TRUE(Mine(start, count));
    EXPECT_TRUE(listener_alex_.wait(future));
    EXPECT_TRUE(txos_alex_.Mature(end));
    EXPECT_TRUE(txos_bob_.Mature(end));
}

TEST_F(Regtest_payment_code, first_block)
{
    const auto handle = client_1_.Network().Blockchain().GetChain(test_chain_);

    ASSERT_TRUE(handle);

    const auto& blockchain = handle.get();
    const auto blockHash = blockchain.HeaderOracle().BestHash(1);

    ASSERT_FALSE(blockHash.IsNull());

    auto future = blockchain.BlockOracle().Load(blockHash);
    const auto& block = future.get().asBitcoin();

    EXPECT_TRUE(block.IsValid());
    ASSERT_EQ(block.size(), 1);

    const auto tx = block.get()[0].asBitcoin();

    EXPECT_TRUE(tx.IsValid());
    EXPECT_EQ(tx.ID(), transactions_.at(0));
    EXPECT_EQ(tx.BlockPosition(), 0);
    ASSERT_EQ(tx.Outputs().size(), 1);
    EXPECT_TRUE(tx.IsGeneration());
}

TEST_F(Regtest_payment_code, alex_account_activity_initial_receive)
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
        10000000000,
        u8"100 units"_cstr,
        "",
        {},
        {test_chain_},
        100,
        {height_, height_},
        {},
        {{u8"100"_cstr, u8"100 units"_cstr}},
        {
            {
                ot::otx::client::StorageBox::BLOCKCHAIN,
                1,
                10000000000,
                u8"100 units"_cstr,
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
    wait_for_counter(account_activity_alex_, false);

    EXPECT_TRUE(check_account_activity(alex_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(alex_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(alex_, id, expected));
}

TEST_F(Regtest_payment_code, alex_account_list_initial_receive)
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
         10000000000,
         u8"100 units"_cstr},
    }};
    wait_for_counter(account_list_alex_, false);

    EXPECT_TRUE(check_account_list(alex_, expected));
    EXPECT_TRUE(check_account_list_qt(alex_, expected));
    EXPECT_TRUE(check_account_list_rpc(alex_, expected));
}

TEST_F(Regtest_payment_code, alex_account_tree_initial_receive)
{
    const auto expected = AccountTreeData{
        {{expected_unit_type_,
          expected_notary_name_,
          {
              {SendHD().Parent().AccountID().asBase58(ot_.Crypto()),
               expected_unit_.asBase58(ot_.Crypto()),
               expected_display_unit_,
               expected_account_name_,
               expected_notary_.asBase58(ot_.Crypto()),
               expected_notary_name_,
               ot::AccountType::Blockchain,
               expected_unit_type_,
               1,
               10000000000,
               u8"100 units"_cstr},
          }}}};
    wait_for_counter(account_tree_alex_, false);

    EXPECT_TRUE(check_account_tree(alex_, expected));
    EXPECT_TRUE(check_account_tree_qt(alex_, expected));
}

TEST_F(Regtest_payment_code, alex_txodb_inital_receive)
{
    EXPECT_TRUE(CheckTXODBAlex());
}

TEST_F(Regtest_payment_code, register_on_notary)
{
    account_activity_alex_.expected_ += 0;
    account_activity_bob_.expected_ += 0;
    activity_thread_alex_bob_.expected_ += 0;
    activity_thread_bob_alex_.expected_ += 0;
    contact_list_alex_.expected_ += 0;
    contact_list_bob_.expected_ += 0;
    client_1_.OTX().StartIntroductionServer(alex_.nym_id_);
    client_2_.OTX().StartIntroductionServer(bob_.nym_id_);

    EXPECT_TRUE(CheckOTXResult(
        client_1_.OTX().RegisterNymPublic(alex_.nym_id_, server_1_.id_, true)));
    EXPECT_TRUE(CheckOTXResult(
        client_2_.OTX().RegisterNymPublic(bob_.nym_id_, server_1_.id_, true)));

    client_1_.OTX().ContextIdle(alex_.nym_id_, server_1_.id_).get();
    client_2_.OTX().ContextIdle(bob_.nym_id_, server_1_.id_).get();
}

TEST_F(Regtest_payment_code, import_nym)
{
    account_activity_alex_.expected_ += 0;
    account_activity_bob_.expected_ += 0;
    activity_thread_alex_bob_.expected_ += 0;
    activity_thread_bob_alex_.expected_ += 0;
    contact_list_alex_.expected_ += 0;
    contact_list_bob_.expected_ += 1;
    auto proto = ""s;

    EXPECT_TRUE(alex_.nym_->Serialize(ot::writer(proto)));

    auto imported = client_2_.Wallet().Nym(proto);

    EXPECT_TRUE(imported);
}

TEST_F(Regtest_payment_code, bob_contact_list_after_import)
{
    const auto expected = ContactListData{{
        {true, bob_.name_, bob_.name_, "ME", ""},
        {false, alex_.name_, alex_.name_, "A", ""},
    }};
    wait_for_counter(contact_list_bob_, false);

    EXPECT_TRUE(check_contact_list(bob_, expected));
    EXPECT_TRUE(check_contact_list_qt(bob_, expected));
    EXPECT_TRUE(CheckContactID(bob_, alex_, alex_.payment_code_));
}

TEST_F(Regtest_payment_code, send_faucet_request)
{
    auto faucet =
        FaucetListener{client_1_, alex_.nym_id_, "faucet"}.GetFuture();
    account_activity_alex_.expected_ += 2;
    account_activity_bob_.expected_ += 2;
    account_list_alex_.expected_ += 2;
    account_list_bob_.expected_ += 1;
    account_tree_alex_.expected_ += 3;
    account_tree_bob_.expected_ += 1;
    activity_thread_alex_bob_.expected_ += 0;
    activity_thread_bob_alex_.expected_ += 0;
    contact_list_alex_.expected_ += 2;
    contact_list_bob_.expected_ += 0;

    if (activity_thread_request_faucet(bob_, alex_)) {
        const auto txid = faucet.get();

        EXPECT_FALSE(txid.empty());

        transactions_.emplace_back(txid);
    } else {
        FAIL();
    }

    client_1_.OTX().ContextIdle(alex_.nym_id_, server_1_.id_).get();
    client_2_.OTX().ContextIdle(bob_.nym_id_, server_1_.id_).get();
}

TEST_F(Regtest_payment_code, alex_contact_list_first_spend_unconfirmed)
{
    const auto expected = ContactListData{{
        {true, alex_.name_, alex_.name_, "ME", ""},
        {false, bob_.name_, bob_.name_, "B", ""},
    }};
    wait_for_counter(contact_list_alex_, false);

    EXPECT_TRUE(check_contact_list(alex_, expected));
    EXPECT_TRUE(check_contact_list_qt(alex_, expected));
    EXPECT_TRUE(CheckContactID(alex_, bob_, bob_.payment_code_));
}

TEST_F(Regtest_payment_code, alex_account_activity_first_spend_unconfirmed)
{
    const auto expectedContact = client_1_.Contacts().ContactID(bob_.nym_id_);
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
        8999999684,
        u8"89.999\u202F996\u202F84 units"_cstr,
        "",
        {},
        {test_chain_},
        100,
        {height_, height_},
        {},
        {{u8"89.99999684"_cstr, u8"89.999\u202F996\u202F84 units"_cstr}},
        {
            {
                ot::otx::client::StorageBox::BLOCKCHAIN,
                -1,
                -1000000316,
                u8"-10.000\u202F003\u202F16 units"_cstr,
                {expectedContact.asBase58(ot_.Crypto())},
                "",
                "",
                "Outgoing Unit Test Simulation transaction to Bob",
                ot::blockchain::HashToNumber(transactions_.at(1)),
                std::nullopt,
                0,
            },
            {
                ot::otx::client::StorageBox::BLOCKCHAIN,
                1,
                10000000000,
                u8"100 units"_cstr,
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
    wait_for_counter(account_activity_alex_, false);

    EXPECT_TRUE(check_account_activity(alex_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(alex_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(alex_, id, expected));
}

TEST_F(Regtest_payment_code, alex_account_list_first_spend_unconfirmed)
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
         8999999684,
         u8"89.999\u202F996\u202F84 units"_cstr},
    }};
    wait_for_counter(account_list_alex_, false);

    EXPECT_TRUE(check_account_list(alex_, expected));
    EXPECT_TRUE(check_account_list_qt(alex_, expected));
    EXPECT_TRUE(check_account_list_rpc(alex_, expected));
}

TEST_F(Regtest_payment_code, alex_account_tree_first_spend_unconfirmed)
{
    const auto expected = AccountTreeData{
        {{expected_unit_type_,
          expected_notary_name_,
          {
              {SendHD().Parent().AccountID().asBase58(ot_.Crypto()),
               expected_unit_.asBase58(ot_.Crypto()),
               expected_display_unit_,
               expected_account_name_,
               expected_notary_.asBase58(ot_.Crypto()),
               expected_notary_name_,
               ot::AccountType::Blockchain,
               expected_unit_type_,
               1,
               8999999684,
               u8"89.999\u202F996\u202F84 units"_cstr},
          }}}};
    wait_for_counter(account_tree_alex_, false);

    EXPECT_TRUE(check_account_tree(alex_, expected));
    EXPECT_TRUE(check_account_tree_qt(alex_, expected));
}

TEST_F(Regtest_payment_code, alex_activity_thread_first_spend_unconfirmed)
{
    activity_thread_alex_bob_.expected_ += 3;
    init_activity_thread(alex_, bob_, activity_thread_alex_bob_);
    const auto& contact = alex_.Contact(bob_.name_);
    const auto expected = ActivityThreadData{
        true,
        contact.asBase58(ot_.Crypto()),
        bob_.name_,
        "",
        contact.asBase58(ot_.Crypto()),
        {{ot::UnitType::Regtest, bob_.payment_code_}},
        {
            {
                false,
                false,
                true,
                -1,
                -1000000316,
                u8"-10.000\u202F003\u202F16 units"_cstr,
                alex_.name_,
                "Outgoing Unit Test Simulation transaction to Bob",
                "",
                ot::otx::client::StorageBox::BLOCKCHAIN,
                std::nullopt,
            },
        },
    };
    wait_for_counter(activity_thread_alex_bob_, false);

    EXPECT_TRUE(check_activity_thread(alex_, contact, expected));
    EXPECT_TRUE(check_activity_thread_qt(alex_, contact, expected));
}

TEST_F(Regtest_payment_code, bob_contact_list_first_unconfirmed_incoming)
{
    const auto expected = ContactListData{{
        {true, bob_.name_, bob_.name_, "ME", ""},
        {true, alex_.name_, alex_.name_, "A", ""},
    }};
    wait_for_counter(contact_list_bob_, false);

    EXPECT_TRUE(check_contact_list(bob_, expected));
    EXPECT_TRUE(check_contact_list_qt(bob_, expected));
}

TEST_F(Regtest_payment_code, bob_account_activity_first_unconfirmed_incoming)
{
    const auto expectedContact = client_2_.Contacts().ContactID(alex_.nym_id_);
    const auto& id = ReceiveHD().Parent().AccountID();
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
        1000000000,
        u8"10 units"_cstr,
        "",
        {},
        {test_chain_},
        100,
        {height_, height_},
        {},
        {{u8"10"_cstr, u8"10 units"_cstr}},
        {
            {
                ot::otx::client::StorageBox::BLOCKCHAIN,
                1,
                1000000000,
                u8"10 units"_cstr,
                {expectedContact.asBase58(ot_.Crypto())},
                "",
                "",
                "Incoming Unit Test Simulation transaction from Alex",
                ot::blockchain::HashToNumber(transactions_.at(1)),
                std::nullopt,
                0,
            },
        },
    };
    wait_for_counter(account_activity_bob_, false);

    EXPECT_TRUE(check_account_activity(bob_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(bob_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(bob_, id, expected));
}

TEST_F(Regtest_payment_code, bob_account_list_first_unconfirmed_incoming)
{
    const auto expected = AccountListData{{
        {ReceiveHD().Parent().AccountID().asBase58(ot_.Crypto()),
         expected_unit_.asBase58(ot_.Crypto()),
         expected_display_unit_,
         expected_account_name_,
         expected_notary_.asBase58(ot_.Crypto()),
         expected_notary_name_,
         expected_account_type_,
         expected_unit_type_,
         1,
         1000000000,
         u8"10 units"_cstr},
    }};
    wait_for_counter(account_list_bob_, false);

    EXPECT_TRUE(check_account_list(bob_, expected));
    EXPECT_TRUE(check_account_list_qt(bob_, expected));
    EXPECT_TRUE(check_account_list_rpc(bob_, expected));
}

TEST_F(Regtest_payment_code, bob_account_tree_first_unconfirmed_incoming)
{
    const auto expected = AccountTreeData{
        {{expected_unit_type_,
          expected_notary_name_,
          {
              {ReceiveHD().Parent().AccountID().asBase58(ot_.Crypto()),
               expected_unit_.asBase58(ot_.Crypto()),
               expected_display_unit_,
               expected_account_name_,
               expected_notary_.asBase58(ot_.Crypto()),
               expected_notary_name_,
               ot::AccountType::Blockchain,
               expected_unit_type_,
               1,
               1000000000,
               u8"10 units"_cstr},
          }}}};
    wait_for_counter(account_tree_bob_, false);

    EXPECT_TRUE(check_account_tree(bob_, expected));
    EXPECT_TRUE(check_account_tree_qt(bob_, expected));
}

TEST_F(Regtest_payment_code, bob_activity_thread_first_unconfirmed_incoming)
{
    init_activity_thread(bob_, alex_, activity_thread_bob_alex_);
    const auto& contact = bob_.Contact(alex_.name_);
    const auto expected = ActivityThreadData{
        true,
        contact.asBase58(ot_.Crypto()),
        alex_.name_,
        "",
        contact.asBase58(ot_.Crypto()),
        {{ot::UnitType::Regtest, alex_.payment_code_}},
        {
            {
                false,
                false,
                false,
                1,
                1000000000,
                "10 units",
                alex_.name_,
                "Incoming Unit Test Simulation transaction from Alex",
                "",
                ot::otx::client::StorageBox::BLOCKCHAIN,
                std::nullopt,
            },
        },
    };
    wait_for_counter(activity_thread_bob_alex_, false);

    EXPECT_TRUE(check_activity_thread(bob_, contact, expected));
    EXPECT_TRUE(check_activity_thread_qt(bob_, contact, expected));
}

TEST_F(Regtest_payment_code, shutdown) { Shutdown(); }
}  // namespace ottest
