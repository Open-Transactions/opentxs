// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <atomic>
#include <initializer_list>
#include <optional>
#include <span>
#include <utility>

#include "ottest/data/crypto/PaymentCodeV3.hpp"
#include "ottest/fixtures/blockchain/Common.hpp"
#include "ottest/fixtures/blockchain/ScanListener.hpp"
#include "ottest/fixtures/blockchain/regtest/MultiplePaymentCode.hpp"
#include "ottest/fixtures/common/Counter.hpp"
#include "ottest/fixtures/common/User.hpp"
#include "ottest/fixtures/rpc/Helpers.hpp"
#include "ottest/fixtures/ui/AccountActivity.hpp"
#include "ottest/fixtures/ui/AccountList.hpp"
#include "ottest/fixtures/ui/AccountTree.hpp"
#include "ottest/fixtures/ui/ContactActivity.hpp"
#include "ottest/fixtures/ui/ContactList.hpp"

namespace ottest
{
using namespace opentxs::literals;

Counter account_activity_alex_{};
Counter account_activity_bob_{};
Counter account_activity_chris_{};
Counter account_activity_daniel_{};
Counter account_list_alex_{};
Counter account_list_bob_{};
Counter account_list_chris_{};
Counter account_list_daniel_{};
Counter account_tree_alex_{};
Counter account_tree_bob_{};
Counter account_tree_chris_{};
Counter account_tree_daniel_{};
Counter contact_activity_alex_bob_{};
Counter contact_activity_alex_chris_{};
Counter contact_activity_alex_daniel_{};
Counter contact_activity_bob_alex_{};
Counter contact_activity_chris_alex_{};
Counter contact_activity_daniel_alex_{};
Counter contact_list_alex_{};
Counter contact_list_bob_{};
Counter contact_list_chris_{};
Counter contact_list_daniel_{};

TEST_F(Regtest_multiple_payment_code, init_opentxs) {}

TEST_F(Regtest_multiple_payment_code, start_chains) { EXPECT_TRUE(Start()); }

TEST_F(Regtest_multiple_payment_code, connect_peers) { EXPECT_TRUE(Connect()); }

TEST_F(Regtest_multiple_payment_code, init_ui_models)
{
    account_activity_alex_.expected_ += 0;
    account_activity_bob_.expected_ += 0;
    account_activity_chris_.expected_ += 0;
    account_activity_daniel_.expected_ += 0;
    account_list_alex_.expected_ += 1;
    account_list_bob_.expected_ += 1;
    account_list_chris_.expected_ += 1;
    account_list_daniel_.expected_ += 1;
    account_tree_alex_.expected_ += 2;
    account_tree_bob_.expected_ += 2;
    account_tree_chris_.expected_ += 2;
    account_tree_daniel_.expected_ += 2;
    contact_list_alex_.expected_ += 1;
    contact_list_bob_.expected_ += 3;
    contact_list_chris_.expected_ += 3;
    contact_list_daniel_.expected_ += 3;

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

    init_account_activity(
        chris_, ChrisHD().Parent().AccountID(), account_activity_chris_);
    init_account_list(chris_, account_list_chris_);
    init_account_tree(chris_, account_tree_chris_);
    init_contact_list(chris_, contact_list_chris_);

    init_account_activity(
        daniel_, DanielHD().Parent().AccountID(), account_activity_daniel_);
    init_account_list(daniel_, account_list_daniel_);
    init_account_tree(daniel_, account_tree_daniel_);
    init_contact_list(daniel_, contact_list_daniel_);
}

TEST_F(Regtest_multiple_payment_code, alex_contact_list_initial)
{
    const auto expected = ContactListData{{
        {true, alex_.name_, alex_.name_, "ME", ""},
    }};
    wait_for_counter(contact_list_alex_, false);

    EXPECT_TRUE(check_contact_list(alex_, expected));
    EXPECT_TRUE(check_contact_list_qt(alex_, expected));
}

TEST_F(Regtest_multiple_payment_code, alex_account_activity_initial)
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

TEST_F(Regtest_multiple_payment_code, alex_account_list_initial)
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

TEST_F(Regtest_multiple_payment_code, alex_account_tree_initial)
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

TEST_F(Regtest_multiple_payment_code, bob_contact_list_initial)
{
    const auto expected = ContactListData{{
        {true, bob_.name_, bob_.name_, "ME", ""},
        {false, chris_.name_, chris_.name_, "C", ""},
        {false, daniel_.name_, daniel_.name_, "D", ""},
    }};
    wait_for_counter(contact_list_bob_, false);

    EXPECT_TRUE(check_contact_list(bob_, expected));
    EXPECT_TRUE(check_contact_list_qt(bob_, expected));
}

TEST_F(Regtest_multiple_payment_code, bob_account_activity_initial)
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

TEST_F(Regtest_multiple_payment_code, bob_account_list_initial)
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

TEST_F(Regtest_multiple_payment_code, bob_account_tree_initial)
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

TEST_F(Regtest_multiple_payment_code, chris_contact_list_initial)
{
    const auto expected = ContactListData{{
        {true, chris_.name_, chris_.name_, "ME", ""},
        {false, bob_.name_, bob_.name_, "B", ""},
        {false, daniel_.name_, daniel_.name_, "D", ""},
    }};
    wait_for_counter(contact_list_chris_, false);

    EXPECT_TRUE(check_contact_list(chris_, expected));
    EXPECT_TRUE(check_contact_list_qt(chris_, expected));
}

TEST_F(Regtest_multiple_payment_code, chris_account_activity_initial)
{
    const auto& id = ChrisHD().Parent().AccountID();
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
    wait_for_counter(account_activity_chris_, false);

    EXPECT_TRUE(check_account_activity(chris_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(chris_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(chris_, id, expected));
}

TEST_F(Regtest_multiple_payment_code, chris_account_list_initial)
{
    const auto expected = AccountListData{{
        {ChrisHD().Parent().AccountID().asBase58(ot_.Crypto()),
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
    wait_for_counter(account_list_chris_, false);

    EXPECT_TRUE(check_account_list(chris_, expected));
    EXPECT_TRUE(check_account_list_qt(chris_, expected));
    EXPECT_TRUE(check_account_list_rpc(chris_, expected));
}

TEST_F(Regtest_multiple_payment_code, chris_account_tree_initial)
{
    const auto expected = AccountTreeData{
        {{expected_unit_type_,
          expected_notary_name_,
          {
              {ChrisHD().Parent().AccountID().asBase58(ot_.Crypto()),
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
    wait_for_counter(account_tree_chris_, false);

    EXPECT_TRUE(check_account_tree(chris_, expected));
    EXPECT_TRUE(check_account_tree_qt(chris_, expected));
}

TEST_F(Regtest_multiple_payment_code, daniel_contact_list_initial)
{
    const auto expected = ContactListData{{
        {true, daniel_.name_, daniel_.name_, "ME", ""},
        {false, bob_.name_, bob_.name_, "B", ""},
        {false, chris_.name_, chris_.name_, "C", ""},
    }};
    wait_for_counter(contact_list_daniel_, false);

    EXPECT_TRUE(check_contact_list(daniel_, expected));
    EXPECT_TRUE(check_contact_list_qt(daniel_, expected));
}

TEST_F(Regtest_multiple_payment_code, daniel_account_activity_initial)
{
    const auto& id = DanielHD().Parent().AccountID();
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
    wait_for_counter(account_activity_daniel_, false);

    EXPECT_TRUE(check_account_activity(daniel_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(daniel_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(daniel_, id, expected));
}

TEST_F(Regtest_multiple_payment_code, daniel_account_list_initial)
{
    const auto expected = AccountListData{{
        {DanielHD().Parent().AccountID().asBase58(ot_.Crypto()),
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
    wait_for_counter(account_list_daniel_, false);

    EXPECT_TRUE(check_account_list(daniel_, expected));
    EXPECT_TRUE(check_account_list_qt(daniel_, expected));
    EXPECT_TRUE(check_account_list_rpc(daniel_, expected));
}

TEST_F(Regtest_multiple_payment_code, daniel_account_tree_initial)
{
    const auto expected = AccountTreeData{
        {{expected_unit_type_,
          expected_notary_name_,
          {
              {DanielHD().Parent().AccountID().asBase58(ot_.Crypto()),
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
    wait_for_counter(account_tree_daniel_, false);

    EXPECT_TRUE(check_account_tree(daniel_, expected));
    EXPECT_TRUE(check_account_tree_qt(daniel_, expected));
}

TEST_F(Regtest_multiple_payment_code, mine_initial_balance)
{
    constexpr auto orphan{0};
    constexpr auto count{1};
    const auto start = height_ - orphan;
    const auto end{start + count};
    auto future = listener_alex_.get_future(SendHD(), Subchain::External, end);
    account_activity_alex_.expected_ += 2;
    account_activity_bob_.expected_ += 2;
    account_activity_chris_.expected_ += 2;
    account_activity_daniel_.expected_ += 2;
    account_list_alex_.expected_ += 0;
    account_list_bob_.expected_ += 0;
    account_list_chris_.expected_ += 0;
    account_list_daniel_.expected_ += 0;
    account_tree_alex_.expected_ += 0;
    account_tree_bob_.expected_ += 0;
    account_tree_chris_.expected_ += 0;
    account_tree_daniel_.expected_ += 0;
    contact_list_alex_.expected_ += 0;
    contact_list_bob_.expected_ += 0;
    contact_list_chris_.expected_ += 0;
    contact_list_daniel_.expected_ += 0;

    EXPECT_EQ(start, 0);
    EXPECT_EQ(end, 1);
    EXPECT_TRUE(Mine(start, count, mine_to_alex_));
    EXPECT_TRUE(listener_alex_.wait(future));
}

TEST_F(Regtest_multiple_payment_code, mature_initial_balance)
{
    constexpr auto orphan{0};
    const auto count = static_cast<int>(MaturationInterval());
    const auto start = height_ - orphan;
    const auto end{start + count};
    auto future = listener_alex_.get_future(SendHD(), Subchain::External, end);
    account_activity_alex_.expected_ += 18;
    account_activity_bob_.expected_ += 11;
    account_activity_chris_.expected_ += 11;
    account_activity_daniel_.expected_ += 11;
    account_list_alex_.expected_ += 1;
    account_list_bob_.expected_ += 0;
    account_list_chris_.expected_ += 0;
    account_list_daniel_.expected_ += 0;
    account_tree_alex_.expected_ += 1;
    account_tree_bob_.expected_ += 0;
    account_tree_chris_.expected_ += 0;
    account_tree_daniel_.expected_ += 0;
    contact_list_alex_.expected_ += 0;
    contact_list_bob_.expected_ += 0;
    contact_list_chris_.expected_ += 0;
    contact_list_daniel_.expected_ += 0;

    EXPECT_EQ(start, 1);
    EXPECT_EQ(end, 11);
    EXPECT_TRUE(Mine(start, count));
    EXPECT_TRUE(listener_alex_.wait(future));
}

TEST_F(Regtest_multiple_payment_code, first_block)
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

TEST_F(Regtest_multiple_payment_code, alex_account_activity_initial_receive)
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

TEST_F(Regtest_multiple_payment_code, alex_account_list_initial_receive)
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

TEST_F(Regtest_multiple_payment_code, alex_account_tree_initial_receive)
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

TEST_F(Regtest_multiple_payment_code, bob_contact_list_initial_receive)
{
    const auto expected = ContactListData{{
        {true, bob_.name_, bob_.name_, "ME", ""},
        {true, chris_.name_, chris_.name_, "C", ""},
        {true, daniel_.name_, daniel_.name_, "D", ""},
    }};
    wait_for_counter(contact_list_bob_, false);

    EXPECT_TRUE(check_contact_list(bob_, expected));
    EXPECT_TRUE(check_contact_list_qt(bob_, expected));
}

TEST_F(Regtest_multiple_payment_code, bob_account_activity_initial_receive)
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
        100,
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

TEST_F(Regtest_multiple_payment_code, bob_account_list_initial_receive)
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

TEST_F(Regtest_multiple_payment_code, bob_account_tree_initial_receive)
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

TEST_F(Regtest_multiple_payment_code, chris_contact_list_initial_receive)
{
    const auto expected = ContactListData{{
        {true, chris_.name_, chris_.name_, "ME", ""},
        {true, bob_.name_, bob_.name_, "B", ""},
        {true, daniel_.name_, daniel_.name_, "D", ""},
    }};
    wait_for_counter(contact_list_chris_, false);

    EXPECT_TRUE(check_contact_list(chris_, expected));
    EXPECT_TRUE(check_contact_list_qt(chris_, expected));
}

TEST_F(Regtest_multiple_payment_code, chris_account_activity_initial_receive)
{
    const auto& id = ChrisHD().Parent().AccountID();
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
        {},
    };
    wait_for_counter(account_activity_chris_, false);

    EXPECT_TRUE(check_account_activity(chris_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(chris_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(chris_, id, expected));
}

TEST_F(Regtest_multiple_payment_code, chris_account_list_initial_receive)
{
    const auto expected = AccountListData{{
        {ChrisHD().Parent().AccountID().asBase58(ot_.Crypto()),
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
    wait_for_counter(account_list_chris_, false);

    EXPECT_TRUE(check_account_list(chris_, expected));
    EXPECT_TRUE(check_account_list_qt(chris_, expected));
    EXPECT_TRUE(check_account_list_rpc(chris_, expected));
}

TEST_F(Regtest_multiple_payment_code, chris_account_tree_initial_receive)
{
    const auto expected = AccountTreeData{
        {{expected_unit_type_,
          expected_notary_name_,
          {
              {ChrisHD().Parent().AccountID().asBase58(ot_.Crypto()),
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
    wait_for_counter(account_tree_chris_, false);

    EXPECT_TRUE(check_account_tree(chris_, expected));
    EXPECT_TRUE(check_account_tree_qt(chris_, expected));
}

TEST_F(Regtest_multiple_payment_code, daniel_contact_list_initial_receive)
{
    const auto expected = ContactListData{{
        {true, daniel_.name_, daniel_.name_, "ME", ""},
        {true, bob_.name_, bob_.name_, "B", ""},
        {true, chris_.name_, chris_.name_, "C", ""},
    }};
    wait_for_counter(contact_list_daniel_, false);

    EXPECT_TRUE(check_contact_list(daniel_, expected));
    EXPECT_TRUE(check_contact_list_qt(daniel_, expected));
}

TEST_F(Regtest_multiple_payment_code, daniel_account_activity_initial_receive)
{
    const auto& id = DanielHD().Parent().AccountID();
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
        {},
    };
    wait_for_counter(account_activity_daniel_, false);

    EXPECT_TRUE(check_account_activity(daniel_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(daniel_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(daniel_, id, expected));
}

TEST_F(Regtest_multiple_payment_code, daniel_account_list_initial_receive)
{
    const auto expected = AccountListData{{
        {DanielHD().Parent().AccountID().asBase58(ot_.Crypto()),
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
    wait_for_counter(account_list_daniel_, false);

    EXPECT_TRUE(check_account_list(daniel_, expected));
    EXPECT_TRUE(check_account_list_qt(daniel_, expected));
    EXPECT_TRUE(check_account_list_rpc(daniel_, expected));
}

TEST_F(Regtest_multiple_payment_code, daniel_account_tree_initial_receive)
{
    const auto expected = AccountTreeData{
        {{expected_unit_type_,
          expected_notary_name_,
          {
              {DanielHD().Parent().AccountID().asBase58(ot_.Crypto()),
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
    wait_for_counter(account_tree_daniel_, false);

    EXPECT_TRUE(check_account_tree(daniel_, expected));
    EXPECT_TRUE(check_account_tree_qt(daniel_, expected));
}

TEST_F(Regtest_multiple_payment_code, send_to_bob)
{
    account_activity_alex_.expected_ += 2;
    account_activity_bob_.expected_ += 2;
    account_activity_chris_.expected_ += 1;
    account_activity_daniel_.expected_ += 1;
    account_list_alex_.expected_ += 2;
    account_list_bob_.expected_ += 1;
    account_list_chris_.expected_ += 0;
    account_list_daniel_.expected_ += 0;
    account_tree_alex_.expected_ += 7;
    account_tree_bob_.expected_ += 1;
    account_tree_chris_.expected_ += 0;
    account_tree_daniel_.expected_ += 0;
    contact_list_alex_.expected_ += 4;
    contact_list_bob_.expected_ += 1;
    contact_list_chris_.expected_ += 1;
    contact_list_daniel_.expected_ += 1;
    const auto handle = client_1_.Network().Blockchain().GetChain(test_chain_);

    ASSERT_TRUE(handle);

    const auto notify = std::initializer_list<opentxs::PaymentCode>{
        chris_.PaymentCode(),
        daniel_.PaymentCode(),
    };
    const auto& network = handle.get();
    auto future = network.SendToPaymentCode(
        alex_.nym_id_, bob_.PaymentCode(), 1000000000, memo_outgoing_, notify);
    const auto& txid = transactions_.emplace_back(future.get().second);

    ASSERT_FALSE(txid.empty());
}

TEST_F(Regtest_multiple_payment_code, first_outgoing_transaction)
{
    const auto& api = client_1_;
    const auto& blockchain = api.Crypto().Blockchain();
    const auto handle = api.Network().Blockchain().GetChain(test_chain_);

    ASSERT_TRUE(handle);

    const auto& chain = handle.get();
    const auto& contact = api.Contacts();
    const auto& me = alex_.nym_id_;
    const auto self = contact.ContactID(me);
    const auto other = contact.ContactID(bob_.nym_id_);
    const auto& txid = transactions_.at(1);
    const auto tx = blockchain.LoadTransaction(txid).asBitcoin();

    EXPECT_TRUE(tx.IsValid());

    {
        const auto nyms = tx.AssociatedLocalNyms(api.Crypto().Blockchain(), {});

        EXPECT_GT(nyms.size(), 0);

        if (0 < nyms.size()) {
            EXPECT_EQ(nyms.size(), 1);
            EXPECT_EQ(*nyms.cbegin(), me);
        }
    }
    {
        const auto contacts = tx.AssociatedRemoteContacts(api, me, {});

        EXPECT_GT(contacts.size(), 0);

        if (0 < contacts.size()) {
            EXPECT_EQ(contacts.size(), 1);
            EXPECT_EQ(*contacts.cbegin(), other);
        }
    }
    {
        ASSERT_EQ(tx.Inputs().size(), 1u);

        const auto& script = tx.Inputs()[0u].Script();
        const auto elements = script.get();

        ASSERT_EQ(elements.size(), 1u);

        const auto& sig = elements[0];

        ASSERT_TRUE(sig.data_.has_value());
        EXPECT_GE(sig.data_.value().size(), 70u);
        EXPECT_LE(sig.data_.value().size(), 74u);
    }
    {
        ASSERT_EQ(tx.Outputs().size(), 4u);

        const auto& payment = tx.Outputs()[0];
        const auto& change1 = tx.Outputs()[1];
        const auto& change2 = tx.Outputs()[2];
        const auto& change3 = tx.Outputs()[3];

        EXPECT_EQ(payment.Payer(), self);
        EXPECT_EQ(payment.Payee(), other);
        EXPECT_EQ(change1.Payer(), self);
        EXPECT_EQ(change1.Payee(), self);
        EXPECT_EQ(change2.Payer(), self);
        EXPECT_EQ(change2.Payee(), self);
        EXPECT_EQ(change3.Payer(), self);
        EXPECT_EQ(change3.Payee(), self);

        const auto tags = chain.Wallet().GetTags({tx.ID().Bytes(), 1});
        using Tag = ot::blockchain::node::TxoTag;

        EXPECT_EQ(tags.size(), 3);
        EXPECT_EQ(tags.count(Tag::Normal), 1);
        EXPECT_EQ(tags.count(Tag::Notification), 1);
        EXPECT_EQ(tags.count(Tag::Change), 1);
    }
}

TEST_F(Regtest_multiple_payment_code, alex_contact_list_first_spend_unconfirmed)
{
    const auto expected = ContactListData{{
        {true, alex_.name_, alex_.name_, "ME", ""},
        {false, daniel_.name_, daniel_.payment_code_, "P", ""},
        {false, bob_.name_, bob_.payment_code_, "P", ""},
        {false, chris_.name_, chris_.payment_code_, "P", ""},
    }};
    wait_for_counter(contact_list_alex_, false);

    EXPECT_TRUE(check_contact_list(alex_, expected));
    EXPECT_TRUE(check_contact_list_qt(alex_, expected));
    EXPECT_TRUE(CheckContactID(
        alex_, bob_, GetPaymentCodeVector3().bob_.payment_code_));
}

TEST_F(
    Regtest_multiple_payment_code,
    alex_account_activity_first_spend_unconfirmed)
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
        8999999456,
        u8"89.999\u202F994\u202F56 units"_cstr,
        "",
        {},
        {test_chain_},
        100,
        {height_, height_},
        {},
        {{u8"89.99999456"_cstr, u8"89.999\u202F994\u202F56 units"_cstr}},
        {
            {
                ot::otx::client::StorageBox::BLOCKCHAIN,
                -1,
                -1000000544,
                u8"-10.000\u202F005\u202F44 units"_cstr,
                {expectedContact.asBase58(ot_.Crypto())},
                "",
                "",
                "Outgoing Unit Test Simulation transaction to "
                "PD1jFsimY3DQUe7qGtx3z8BohTaT6r4kwJMCYXwp7uY8z6BSaFrpM",
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

TEST_F(Regtest_multiple_payment_code, alex_account_list_first_spend_unconfirmed)
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
         8999999456,
         u8"89.999\u202F994\u202F56 units"_cstr},
    }};
    wait_for_counter(account_list_alex_, false);

    EXPECT_TRUE(check_account_list(alex_, expected));
    EXPECT_TRUE(check_account_list_qt(alex_, expected));
    EXPECT_TRUE(check_account_list_rpc(alex_, expected));
}

TEST_F(Regtest_multiple_payment_code, alex_account_tree_first_spend_unconfirmed)
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
               8999999456,
               u8"89.999\u202F994\u202F56 units"_cstr},
          }}}};
    wait_for_counter(account_tree_alex_, false);

    EXPECT_TRUE(check_account_tree(alex_, expected));
    EXPECT_TRUE(check_account_tree_qt(alex_, expected));
}

TEST_F(
    Regtest_multiple_payment_code,
    alex_contact_activity_first_spend_unconfirmed)
{
    contact_activity_alex_bob_.expected_ += 3;
    init_contact_activity(alex_, bob_, contact_activity_alex_bob_);
    const auto& contact = alex_.Contact(bob_.name_);
    const auto expected = ContactActivityData{
        false,
        contact.asBase58(ot_.Crypto()),
        bob_.payment_code_,
        "",
        contact.asBase58(ot_.Crypto()),
        {{ot::UnitType::Regtest, bob_.payment_code_}},
        {
            {
                false,
                false,
                true,
                -1,
                -1000000544,
                u8"-10.000\u202F005\u202F44 units"_cstr,
                alex_.name_,
                "Outgoing Unit Test Simulation transaction to "
                "PD1jFsimY3DQUe7qGtx3z8BohTaT6r4kwJMCYXwp7uY8z6BSaFrpM",
                "",
                ot::otx::client::StorageBox::BLOCKCHAIN,
                std::nullopt,
                ot::blockchain::HashToNumber(transactions_.at(1)),
            },
        },
    };
    wait_for_counter(contact_activity_alex_bob_, false);

    EXPECT_TRUE(check_contact_activity(alex_, contact, expected));
    EXPECT_TRUE(check_contact_activity_qt(alex_, contact, expected));
}

TEST_F(
    Regtest_multiple_payment_code,
    bob_contact_list_first_unconfirmed_incoming)
{
    const auto expected = ContactListData{{
        {true, bob_.name_, bob_.name_, "ME", ""},
        {true, chris_.name_, chris_.name_, "C", ""},
        {true, daniel_.name_, daniel_.name_, "D", ""},
        {false, alex_.name_, alex_.payment_code_, "P", ""},
    }};
    wait_for_counter(contact_list_bob_, false);

    EXPECT_TRUE(check_contact_list(bob_, expected));
    EXPECT_TRUE(check_contact_list_qt(bob_, expected));
    EXPECT_TRUE(CheckContactID(
        bob_, alex_, GetPaymentCodeVector3().alice_.payment_code_));
}

TEST_F(
    Regtest_multiple_payment_code,
    bob_account_activity_first_unconfirmed_incoming)
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
                "Incoming Unit Test Simulation transaction from "
                "PD1jTsa1rjnbMMLVbj5cg2c8KkFY32KWtPRqVVpSBkv1jf8zjHJVu",
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

TEST_F(
    Regtest_multiple_payment_code,
    bob_account_list_first_unconfirmed_incoming)
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

TEST_F(
    Regtest_multiple_payment_code,
    bob_account_tree_first_unconfirmed_incoming)
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

TEST_F(
    Regtest_multiple_payment_code,
    bob_contact_activity_first_unconfirmed_incoming)
{
    contact_activity_bob_alex_.expected_ += 3;
    init_contact_activity(bob_, alex_, contact_activity_bob_alex_);
    const auto& contact = bob_.Contact(alex_.name_);
    const auto expected = ContactActivityData{
        false,
        contact.asBase58(ot_.Crypto()),
        alex_.payment_code_,
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
                alex_.payment_code_,
                "Incoming Unit Test Simulation transaction from "
                "PD1jTsa1rjnbMMLVbj5cg2c8KkFY32KWtPRqVVpSBkv1jf8zjHJVu",
                "",
                ot::otx::client::StorageBox::BLOCKCHAIN,
                std::nullopt,
                ot::blockchain::HashToNumber(transactions_.at(1)),
            },
        },
    };
    wait_for_counter(contact_activity_bob_alex_, false);

    EXPECT_TRUE(check_contact_activity(bob_, contact, expected));
    EXPECT_TRUE(check_contact_activity_qt(bob_, contact, expected));
}

TEST_F(
    Regtest_multiple_payment_code,
    chris_contact_list_first_unconfirmed_incoming)
{
    const auto expected = ContactListData{{
        {true, chris_.name_, chris_.name_, "ME", ""},
        {true, bob_.name_, bob_.name_, "B", ""},
        {true, daniel_.name_, daniel_.name_, "D", ""},
        {false, alex_.name_, alex_.payment_code_, "P", ""},
    }};
    wait_for_counter(contact_list_bob_, false);

    EXPECT_TRUE(check_contact_list(chris_, expected));
    EXPECT_TRUE(check_contact_list_qt(chris_, expected));
    EXPECT_TRUE(CheckContactID(
        chris_, alex_, GetPaymentCodeVector3().alice_.payment_code_));
}

TEST_F(
    Regtest_multiple_payment_code,
    chris_account_list_first_unconfirmed_incoming)
{
    const auto expected = AccountListData{{
        {ChrisHD().Parent().AccountID().asBase58(ot_.Crypto()),
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
    wait_for_counter(account_list_chris_, false);

    EXPECT_TRUE(check_account_list(chris_, expected));
    EXPECT_TRUE(check_account_list_qt(chris_, expected));
    EXPECT_TRUE(check_account_list_rpc(chris_, expected));
}

TEST_F(
    Regtest_multiple_payment_code,
    chris_account_tree_first_unconfirmed_incoming)
{
    const auto expected = AccountTreeData{
        {{expected_unit_type_,
          expected_notary_name_,
          {
              {ChrisHD().Parent().AccountID().asBase58(ot_.Crypto()),
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
    wait_for_counter(account_tree_chris_, false);

    EXPECT_TRUE(check_account_tree(chris_, expected));
    EXPECT_TRUE(check_account_tree_qt(chris_, expected));
}

TEST_F(
    Regtest_multiple_payment_code,
    chris_contact_activity_first_unconfirmed_incoming)
{
    contact_activity_chris_alex_.expected_ += 1;
    init_contact_activity(chris_, alex_, contact_activity_chris_alex_);
    const auto& contact = chris_.Contact(alex_.name_);
    const auto expected = ContactActivityData{
        false,
        contact.asBase58(ot_.Crypto()),
        alex_.payment_code_,
        "",
        contact.asBase58(ot_.Crypto()),
        {{ot::UnitType::Regtest, alex_.payment_code_}},
        {},
    };
    wait_for_counter(contact_activity_chris_alex_, false);

    EXPECT_TRUE(check_contact_activity(chris_, contact, expected));
    EXPECT_TRUE(check_contact_activity_qt(chris_, contact, expected));
}

TEST_F(
    Regtest_multiple_payment_code,
    daniel_contact_list_first_unconfirmed_incoming)
{
    const auto expected = ContactListData{{
        {true, daniel_.name_, daniel_.name_, "ME", ""},
        {true, bob_.name_, bob_.name_, "B", ""},
        {true, chris_.name_, chris_.name_, "C", ""},
        {false, alex_.name_, alex_.payment_code_, "P", ""},
    }};
    wait_for_counter(contact_list_bob_, false);

    EXPECT_TRUE(check_contact_list(daniel_, expected));
    EXPECT_TRUE(check_contact_list_qt(daniel_, expected));
    EXPECT_TRUE(CheckContactID(
        daniel_, alex_, GetPaymentCodeVector3().alice_.payment_code_));
}

TEST_F(
    Regtest_multiple_payment_code,
    daniel_account_list_first_unconfirmed_incoming)
{
    const auto expected = AccountListData{{
        {DanielHD().Parent().AccountID().asBase58(ot_.Crypto()),
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
    wait_for_counter(account_list_daniel_, false);

    EXPECT_TRUE(check_account_list(daniel_, expected));
    EXPECT_TRUE(check_account_list_qt(daniel_, expected));
    EXPECT_TRUE(check_account_list_rpc(daniel_, expected));
}

TEST_F(
    Regtest_multiple_payment_code,
    daniel_account_tree_first_unconfirmed_incoming)
{
    const auto expected = AccountTreeData{
        {{expected_unit_type_,
          expected_notary_name_,
          {
              {DanielHD().Parent().AccountID().asBase58(ot_.Crypto()),
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
    wait_for_counter(account_tree_daniel_, false);

    EXPECT_TRUE(check_account_tree(daniel_, expected));
    EXPECT_TRUE(check_account_tree_qt(daniel_, expected));
}

TEST_F(
    Regtest_multiple_payment_code,
    daniel_contact_activity_first_unconfirmed_incoming)
{
    contact_activity_daniel_alex_.expected_ += 1;
    init_contact_activity(daniel_, alex_, contact_activity_daniel_alex_);
    const auto& contact = daniel_.Contact(alex_.name_);
    const auto expected = ContactActivityData{
        false,
        contact.asBase58(ot_.Crypto()),
        alex_.payment_code_,
        "",
        contact.asBase58(ot_.Crypto()),
        {{ot::UnitType::Regtest, alex_.payment_code_}},
        {},
    };
    wait_for_counter(contact_activity_daniel_alex_, false);

    EXPECT_TRUE(check_contact_activity(daniel_, contact, expected));
    EXPECT_TRUE(check_contact_activity_qt(daniel_, contact, expected));
}

TEST_F(Regtest_multiple_payment_code, shutdown) { Shutdown(); }
}  // namespace ottest
