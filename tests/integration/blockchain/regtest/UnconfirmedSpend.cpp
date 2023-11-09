// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "ottest/fixtures/blockchain/SyncListener.hpp"
// IWYU pragma: no_include "ottest/fixtures/blockchain/TXOState.hpp"

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <atomic>
#include <optional>
#include <span>
#include <string_view>
#include <tuple>
#include <utility>

#include "internal/util/P0330.hpp"
#include "ottest/data/crypto/PaymentCodeV3.hpp"
#include "ottest/fixtures/blockchain/Common.hpp"
#include "ottest/fixtures/blockchain/ScanListener.hpp"
#include "ottest/fixtures/blockchain/TXOs.hpp"
#include "ottest/fixtures/blockchain/regtest/PaymentCode.hpp"
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
using namespace std::literals;
using enum opentxs::blockchain::crypto::SubaccountType;

Counter account_activity_alex_{};
Counter account_activity_bob_{};
Counter account_list_alex_{};
Counter account_list_bob_{};
Counter account_tree_alex_{};
Counter account_tree_bob_{};
Counter contact_activity_alex_bob_{};
Counter contact_activity_bob_alex_{};
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
        u8"0 units",
        "",
        {},
        {test_chain_},
        0,
        {height_, height_},
        {},
        {{u8"0", u8"0 units"}},
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
         u8"0 units"},
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
        u8"0 units",
        "",
        {},
        {test_chain_},
        0,
        {height_, height_},
        {},
        {{u8"0", u8"0 units"}},
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
         u8"0 units"},
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
        u8"100 units",
        "",
        {},
        {test_chain_},
        100,
        {height_, height_},
        {},
        {{u8"100", u8"100 units"}},
        {
            {
                ot::otx::client::StorageBox::BLOCKCHAIN,
                1,
                10000000000,
                u8"100 units",
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
         u8"100 units"},
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
               u8"100 units"},
          }}}};
    wait_for_counter(account_tree_alex_, false);

    EXPECT_TRUE(check_account_tree(alex_, expected));
    EXPECT_TRUE(check_account_tree_qt(alex_, expected));
}

TEST_F(Regtest_payment_code, alex_txodb_inital_receive)
{
    EXPECT_TRUE(CheckTXODBAlex());
}

TEST_F(Regtest_payment_code, send_to_bob)
{
    account_activity_alex_.expected_ += 2;
    account_activity_bob_.expected_ += 2;
    account_list_alex_.expected_ += 2;
    account_list_bob_.expected_ += 1;
    account_tree_alex_.expected_ += 3;
    account_tree_bob_.expected_ += 1;
    contact_list_alex_.expected_ += 1;
    contact_list_bob_.expected_ += 1;
    const auto handle = client_1_.Network().Blockchain().GetChain(test_chain_);

    ASSERT_TRUE(handle);

    const auto recipient = client_1_.Factory().PaymentCodeFromBase58(
        GetPaymentCodeVector3().bob_.payment_code_);
    const auto& wallet = handle.get().Wallet();
    auto spend = wallet.CreateSpend(alex_.nym_id_);

    if (false == spend.SetUseEnhancedNotifications(false)) { ADD_FAILURE(); }

    if (false == spend.SetMemo(memo_outgoing_)) { ADD_FAILURE(); }

    if (false == spend.SendToPaymentCode(recipient, 1000000000)) {
        ADD_FAILURE();
    }

    auto future = wallet.Execute(spend);
    const auto& txid = transactions_.emplace_back(future.get().second);

    ASSERT_FALSE(txid.IsNull());

    {
        const auto tx = client_1_.Crypto().Blockchain().LoadTransaction(txid);

        EXPECT_TRUE(tx.IsValid());
        EXPECT_TRUE(
            txos_alex_.SpendUnconfirmed({transactions_.at(0).Bytes(), 0}));
        EXPECT_TRUE(txos_alex_.AddUnconfirmed(tx, 1, SendHD()));
        // NOTE do not update Bob's txos since the recipient payment code
        // subaccount does not exist yet.
    }

    {
        const auto subaccount = SendPC().asDeterministic().asPaymentCode();
        const auto& element = subaccount.BalanceElement(Subchain::Outgoing, 0);
        const auto amount = ot::Amount{1000000000};
        expected_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(txid.Bytes(), 0),
            std::forward_as_tuple(
                client_1_.Factory().DataFromBytes(element.Key().PublicKey()),
                amount,
                Pattern::PayToPubkey));
    }
    {
        const auto subaccount = SendHD().asDeterministic().asHD();
        const auto& element = subaccount.BalanceElement(Subchain::Internal, 0);
        const auto amount = ot::Amount{8999999684};
        expected_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(txid.Bytes(), 1),
            std::forward_as_tuple(
                client_1_.Factory().DataFromBytes(element.Key().PublicKey()),
                amount,
                Pattern::PayToMultisig));
    }
}

TEST_F(Regtest_payment_code, first_outgoing_transaction)
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
        ASSERT_EQ(tx.Outputs().size(), 2u);

        const auto& payment = tx.Outputs()[0];
        const auto& change = tx.Outputs()[1];

        EXPECT_EQ(payment.Payer(), self);
        EXPECT_EQ(payment.Payee(), other);
        EXPECT_EQ(change.Payer(), self);
        EXPECT_EQ(change.Payee(), self);

        const auto tags = chain.Wallet().GetTags({tx.ID().Bytes(), 1});
        using Tag = ot::blockchain::node::TxoTag;

        EXPECT_EQ(tags.size(), 3);
        EXPECT_EQ(tags.count(Tag::Normal), 1);
        EXPECT_EQ(tags.count(Tag::Notification), 1);
        EXPECT_EQ(tags.count(Tag::Change), 1);
    }
}

TEST_F(Regtest_payment_code, check_notification_transactions_sender)
{
    const auto& account =
        client_1_.Crypto().Blockchain().Account(alex_.nym_id_, test_chain_);
    const auto pc = account.GetSubaccounts(PaymentCode);

    ASSERT_TRUE(0_uz < pc.size());

    const auto& subaccount = pc.at(0_uz).asDeterministic().asPaymentCode();
    const auto [incoming, outgoing] = subaccount.NotificationCount();

    EXPECT_EQ(subaccount.IncomingNotificationCount(), 0_uz);
    EXPECT_EQ(subaccount.OutgoingNotificationCount(), 1_uz);
    EXPECT_EQ(incoming, 0_uz);
    EXPECT_EQ(outgoing, 1_uz);
}

TEST_F(Regtest_payment_code, alex_contact_list_first_spend_unconfirmed)
{
    const auto expected = ContactListData{{
        {true, alex_.name_, alex_.name_, "ME", ""},
        {false, bob_.name_, bob_.payment_code_, "P", ""},
    }};
    wait_for_counter(contact_list_alex_, false);

    EXPECT_TRUE(check_contact_list(alex_, expected));
    EXPECT_TRUE(check_contact_list_qt(alex_, expected));
    EXPECT_TRUE(CheckContactID(
        alex_, bob_, GetPaymentCodeVector3().bob_.payment_code_));
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
        u8"89.999\u202F996\u202F84 units",
        "",
        {},
        {test_chain_},
        100,
        {height_, height_},
        {},
        {{u8"89.99999684", u8"89.999\u202F996\u202F84 units"}},
        {
            {
                ot::otx::client::StorageBox::BLOCKCHAIN,
                -1,
                -1000000316,
                u8"-10.000\u202F003\u202F16 units",
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
                u8"100 units",
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
         u8"89.999\u202F996\u202F84 units"},
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
               u8"89.999\u202F996\u202F84 units"},
          }}}};
    wait_for_counter(account_tree_alex_, false);

    EXPECT_TRUE(check_account_tree(alex_, expected));
    EXPECT_TRUE(check_account_tree_qt(alex_, expected));
}

TEST_F(Regtest_payment_code, alex_contact_activity_first_spend_unconfirmed)
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
                -1000000316,
                u8"-10.000\u202F003\u202F16 units",
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

TEST_F(Regtest_payment_code, alex_txodb_first_spend_unconfirmed)
{
    EXPECT_TRUE(CheckTXODBAlex());
}

TEST_F(Regtest_payment_code, bob_contact_list_first_unconfirmed_incoming)
{
    const auto expected = ContactListData{{
        {true, bob_.name_, bob_.name_, "ME", ""},
        {false, alex_.name_, alex_.payment_code_, "P", ""},
    }};
    wait_for_counter(contact_list_bob_, false);

    EXPECT_TRUE(check_contact_list(bob_, expected));
    EXPECT_TRUE(check_contact_list_qt(bob_, expected));
    EXPECT_TRUE(CheckContactID(
        bob_, alex_, GetPaymentCodeVector3().alice_.payment_code_));
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
        u8"10 units",
        "",
        {},
        {test_chain_},
        100,
        {height_, height_},
        {},
        {{u8"10", u8"10 units"}},
        {
            {
                ot::otx::client::StorageBox::BLOCKCHAIN,
                1,
                1000000000,
                u8"10 units",
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
         u8"10 units"},
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
               u8"10 units"},
          }}}};
    wait_for_counter(account_tree_bob_, false);

    EXPECT_TRUE(check_account_tree(bob_, expected));
    EXPECT_TRUE(check_account_tree_qt(bob_, expected));
}

TEST_F(Regtest_payment_code, bob_contact_activity_first_unconfirmed_incoming)
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

TEST_F(Regtest_payment_code, bob_txodb_first_unconfirmed_incoming)
{
    {
        // NOTE normally this would be done when the transaction was first sent
        // but the subaccount returned by ReceivePC() did not exist yet.
        const auto tx = client_1_.Crypto().Blockchain().LoadTransaction(
            transactions_.at(1));

        EXPECT_TRUE(tx.IsValid());
        EXPECT_TRUE(txos_bob_.AddUnconfirmed(tx, 0, ReceivePC()));
    }

    EXPECT_TRUE(CheckTXODBBob());
}

TEST_F(Regtest_payment_code, send_to_alex)
{
    account_activity_alex_.expected_ += 2;
    account_activity_bob_.expected_ += 2;
    account_list_alex_.expected_ += 1;
    account_list_bob_.expected_ += 1;
    account_tree_alex_.expected_ += 1;
    account_tree_bob_.expected_ += 1;
    contact_activity_alex_bob_.expected_ += 1;
    contact_activity_bob_alex_.expected_ += 1;
    const auto handle = client_2_.Network().Blockchain().GetChain(test_chain_);

    ASSERT_TRUE(handle);

    const auto recipient = client_2_.Factory().PaymentCodeFromBase58(
        GetPaymentCodeVector3().alice_.payment_code_);
    const auto& wallet = handle.get().Wallet();
    auto spend = wallet.CreateSpend(bob_.nym_id_);

    if (false == spend.SetUseEnhancedNotifications(false)) { ADD_FAILURE(); }

    if (false == spend.SetMemo(memo_outgoing_)) { ADD_FAILURE(); }

    if (false == spend.SendToPaymentCode(recipient, 100000000)) {
        ADD_FAILURE();
    }

    if (false == spend.SetSpendUnconfirmedChange(true)) { ADD_FAILURE(); }

    if (false == spend.SetSpendUnconfirmedIncoming(true)) { ADD_FAILURE(); }

    auto future = wallet.Execute(spend);
    const auto& txid = transactions_.emplace_back(future.get().second);

    ASSERT_FALSE(txid.IsNull());
}

TEST_F(Regtest_payment_code, alex_account_activity_after_unconfirmed_spend)
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
        9099999684,
        u8"90.999\u202F996\u202F84 units",
        "",
        {},
        {test_chain_},
        100,
        {height_, height_},
        {},
        {{u8"90.99999684", u8"90.999\u202F996\u202F84 units"}},
        {
            {
                ot::otx::client::StorageBox::BLOCKCHAIN,
                1,
                100000000,
                u8"1 units",
                {expectedContact.asBase58(ot_.Crypto())},
                "",
                "",
                "Incoming Unit Test Simulation transaction from "
                "PD1jFsimY3DQUe7qGtx3z8BohTaT6r4kwJMCYXwp7uY8z6BSaFrpM",
                ot::blockchain::HashToNumber(transactions_.at(2)),
                std::nullopt,
                0,
            },
            {
                ot::otx::client::StorageBox::BLOCKCHAIN,
                -1,
                -1000000316,
                u8"-10.000\u202F003\u202F16 units",
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
                u8"100 units",
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

TEST_F(Regtest_payment_code, alex_account_list_after_unconfirmed_spend)
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
         9099999684,
         u8"90.999\u202F996\u202F84 units"},
    }};
    wait_for_counter(account_list_alex_, false);

    EXPECT_TRUE(check_account_list(alex_, expected));
    EXPECT_TRUE(check_account_list_qt(alex_, expected));
    EXPECT_TRUE(check_account_list_rpc(alex_, expected));
}

TEST_F(Regtest_payment_code, alex_account_tree_after_unconfirmed_spend)
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
               9099999684,
               u8"90.999\u202F996\u202F84 units"},
          }}}};
    wait_for_counter(account_tree_alex_, false);

    EXPECT_TRUE(check_account_tree(alex_, expected));
    EXPECT_TRUE(check_account_tree_qt(alex_, expected));
}

TEST_F(Regtest_payment_code, alex_contact_activity_after_unconfirmed_spend)
{
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
                -1000000316,
                u8"-10.000\u202F003\u202F16 units",
                alex_.name_,
                "Outgoing Unit Test Simulation transaction to "
                "PD1jFsimY3DQUe7qGtx3z8BohTaT6r4kwJMCYXwp7uY8z6BSaFrpM",
                "",
                ot::otx::client::StorageBox::BLOCKCHAIN,
                std::nullopt,
                ot::blockchain::HashToNumber(transactions_.at(1)),
            },
            {
                false,
                false,
                false,
                1,
                100000000,
                u8"1 units",
                "PD1jFsimY3DQUe7qGtx3z8BohTaT6r4kwJMCYXwp7uY8z6BSaFrpM",
                "Incoming Unit Test Simulation transaction from "
                "PD1jFsimY3DQUe7qGtx3z8BohTaT6r4kwJMCYXwp7uY8z6BSaFrpM",
                "",
                ot::otx::client::StorageBox::BLOCKCHAIN,
                std::nullopt,
                ot::blockchain::HashToNumber(transactions_.at(2)),
            },
        },
    };
    wait_for_counter(contact_activity_alex_bob_, false);

    EXPECT_TRUE(check_contact_activity(alex_, contact, expected));
    EXPECT_TRUE(check_contact_activity_qt(alex_, contact, expected));
}

TEST_F(Regtest_payment_code, bob_account_activity_after_unconfirmed_spend)
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
        899999684,
        u8"8.999\u202F996\u202F84 units",
        "",
        {},
        {test_chain_},
        100,
        {height_, height_},
        {},
        {{u8"9", u8"9 units"}},
        {
            {
                ot::otx::client::StorageBox::BLOCKCHAIN,
                -1,
                -100000316,
                u8"-1.000\u202F003\u202F16 units",
                {expectedContact.asBase58(ot_.Crypto())},
                "",
                "",
                "Outgoing Unit Test Simulation transaction to "
                "PD1jTsa1rjnbMMLVbj5cg2c8KkFY32KWtPRqVVpSBkv1jf8zjHJVu",
                ot::blockchain::HashToNumber(transactions_.at(2)),
                std::nullopt,
                0,
            },
            {
                ot::otx::client::StorageBox::BLOCKCHAIN,
                1,
                1000000000,
                u8"10 units",
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

TEST_F(Regtest_payment_code, bob_account_list_after_unconfirmed_spend)
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
         899999684,
         u8"8.999\u202F996\u202F84 units"},
    }};
    wait_for_counter(account_list_bob_, false);

    EXPECT_TRUE(check_account_list(bob_, expected));
    EXPECT_TRUE(check_account_list_qt(bob_, expected));
    EXPECT_TRUE(check_account_list_rpc(bob_, expected));
}

TEST_F(Regtest_payment_code, bob_account_tree_after_unconfirmed_spend)
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
               899999684,
               u8"8.999\u202F996\u202F84 units"},
          }}}};
    wait_for_counter(account_tree_bob_, false);

    EXPECT_TRUE(check_account_tree(bob_, expected));
    EXPECT_TRUE(check_account_tree_qt(bob_, expected));
}

TEST_F(Regtest_payment_code, bob_contact_activity_after_unconfirmed_spend)
{
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
            {
                false,
                false,
                true,
                -1,
                -100000316,
                u8"-1.000\u202F003\u202F16 units",
                bob_.name_,
                "Outgoing Unit Test Simulation transaction to "
                "PD1jTsa1rjnbMMLVbj5cg2c8KkFY32KWtPRqVVpSBkv1jf8zjHJVu",
                "",
                ot::otx::client::StorageBox::BLOCKCHAIN,
                std::nullopt,
                ot::blockchain::HashToNumber(transactions_.at(2)),
            },
        },
    };
    wait_for_counter(contact_activity_bob_alex_, false);

    EXPECT_TRUE(check_contact_activity(bob_, contact, expected));
    EXPECT_TRUE(check_contact_activity_qt(bob_, contact, expected));
}

TEST_F(Regtest_payment_code, send_to_alex_again)
{
    account_activity_alex_.expected_ += 2;
    account_activity_bob_.expected_ += 2;
    account_list_alex_.expected_ += 1;
    account_list_bob_.expected_ += 1;
    account_tree_alex_.expected_ += 1;
    account_tree_bob_.expected_ += 1;
    contact_activity_alex_bob_.expected_ += 1;
    contact_activity_bob_alex_.expected_ += 1;
    const auto handle = client_2_.Network().Blockchain().GetChain(test_chain_);

    ASSERT_TRUE(handle);

    const auto recipient = client_2_.Factory().PaymentCodeFromBase58(
        GetPaymentCodeVector3().alice_.payment_code_);
    const auto& wallet = handle.get().Wallet();
    auto spend = wallet.CreateSpend(bob_.nym_id_);

    if (false == spend.SetUseEnhancedNotifications(false)) { ADD_FAILURE(); }

    if (false == spend.SetMemo(memo_outgoing_)) { ADD_FAILURE(); }

    if (false == spend.SendToPaymentCode(recipient, 100000000)) {
        ADD_FAILURE();
    }

    if (false == spend.SetSpendUnconfirmedChange(true)) { ADD_FAILURE(); }

    if (false == spend.SetSpendUnconfirmedIncoming(true)) { ADD_FAILURE(); }

    auto future = wallet.Execute(spend);
    const auto& txid = transactions_.emplace_back(future.get().second);

    ASSERT_FALSE(txid.IsNull());
}

TEST_F(Regtest_payment_code, confirm_send)
{
    constexpr auto orphan{0};
    constexpr auto count{1};
    const auto start = height_ - orphan;
    const auto end{start + count};
    auto future1 = listener_alex_.get_future(SendHD(), Subchain::External, end);
    auto future2 = listener_alex_.get_future(SendHD(), Subchain::Internal, end);
    auto future3 = listener_alex_.get_future(SendPC(), Subchain::Incoming, end);
    auto future4 =
        listener_bob_.get_future(ReceivePC(), Subchain::Incoming, end);
    auto future5 =
        listener_bob_.get_future(ReceiveHD(), Subchain::Internal, end);
    account_activity_alex_.expected_ += 9;
    account_activity_bob_.expected_ += 6;
    account_list_alex_.expected_ += 2;
    account_list_bob_.expected_ += 0;
    account_tree_alex_.expected_ += 2;
    account_tree_bob_.expected_ += 0;
    const auto& first = transactions_.at(1);
    const auto& second = transactions_.at(2);
    const auto& third = transactions_.at(3);
    const auto extra = [&] {
        auto output = ot::UnallocatedVector<Transaction>{};
        output.emplace_back(
            client_2_.Crypto().Blockchain().LoadTransaction(second));
        output.emplace_back(
            client_2_.Crypto().Blockchain().LoadTransaction(third));
        output.emplace_back(
            client_1_.Crypto().Blockchain().LoadTransaction(first));

        return output;
    }();

    EXPECT_EQ(start, 11);
    EXPECT_EQ(end, 12);
    EXPECT_TRUE(Mine(start, count, default_, extra));
    EXPECT_TRUE(listener_alex_.wait(future1));
    EXPECT_TRUE(listener_alex_.wait(future2));
    EXPECT_TRUE(listener_alex_.wait(future3));
    EXPECT_TRUE(listener_alex_.wait(future4));
    EXPECT_TRUE(listener_alex_.wait(future5));
}

TEST_F(Regtest_payment_code, alex_account_activity_after_confirmation)
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
        9199999684,
        u8"91.999\u202F996\u202F84 units",
        "",
        {},
        {test_chain_},
        100,
        {height_, height_},
        {},
        {{u8"91.99999684", u8"91.999\u202F996\u202F84 units"}},
        {
            {
                ot::otx::client::StorageBox::BLOCKCHAIN,
                1,
                100000000,
                u8"1 units",
                {expectedContact.asBase58(ot_.Crypto())},
                "",
                "",
                "Incoming Unit Test Simulation transaction from "
                "PD1jFsimY3DQUe7qGtx3z8BohTaT6r4kwJMCYXwp7uY8z6BSaFrpM",
                ot::blockchain::HashToNumber(transactions_.at(3)),
                std::nullopt,
                1,
            },
            {
                ot::otx::client::StorageBox::BLOCKCHAIN,
                1,
                100000000,
                u8"1 units",
                {expectedContact.asBase58(ot_.Crypto())},
                "",
                "",
                "Incoming Unit Test Simulation transaction from "
                "PD1jFsimY3DQUe7qGtx3z8BohTaT6r4kwJMCYXwp7uY8z6BSaFrpM",
                ot::blockchain::HashToNumber(transactions_.at(2)),
                std::nullopt,
                1,
            },
            {
                ot::otx::client::StorageBox::BLOCKCHAIN,
                -1,
                -1000000316,
                u8"-10.000\u202F003\u202F16 units",
                {expectedContact.asBase58(ot_.Crypto())},
                "",
                "",
                "Outgoing Unit Test Simulation transaction to "
                "PD1jFsimY3DQUe7qGtx3z8BohTaT6r4kwJMCYXwp7uY8z6BSaFrpM",
                ot::blockchain::HashToNumber(transactions_.at(1)),
                std::nullopt,
                1,
            },
            {
                ot::otx::client::StorageBox::BLOCKCHAIN,
                1,
                10000000000,
                u8"100 units",
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
    wait_for_counter(account_activity_alex_, false);

    EXPECT_TRUE(check_account_activity(alex_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(alex_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(alex_, id, expected));
}

TEST_F(Regtest_payment_code, alex_account_list_after_confirmation)
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
         9199999684,
         u8"91.999\u202F996\u202F84 units"},
    }};
    wait_for_counter(account_list_alex_, false);

    EXPECT_TRUE(check_account_list(alex_, expected));
    EXPECT_TRUE(check_account_list_qt(alex_, expected));
    EXPECT_TRUE(check_account_list_rpc(alex_, expected));
}

TEST_F(Regtest_payment_code, alex_account_tree_after_confirmation)
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
               9199999684,
               u8"91.999\u202F996\u202F84 units"},
          }}}};
    wait_for_counter(account_tree_alex_, false);

    EXPECT_TRUE(check_account_tree(alex_, expected));
    EXPECT_TRUE(check_account_tree_qt(alex_, expected));
}

TEST_F(Regtest_payment_code, alex_contact_activity_after_confirmation)
{
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
                -1000000316,
                u8"-10.000\u202F003\u202F16 units",
                alex_.name_,
                "Outgoing Unit Test Simulation transaction to "
                "PD1jFsimY3DQUe7qGtx3z8BohTaT6r4kwJMCYXwp7uY8z6BSaFrpM",
                "",
                ot::otx::client::StorageBox::BLOCKCHAIN,
                std::nullopt,
                ot::blockchain::HashToNumber(transactions_.at(1)),
            },
            {
                false,
                false,
                false,
                1,
                100000000,
                u8"1 units",
                "PD1jFsimY3DQUe7qGtx3z8BohTaT6r4kwJMCYXwp7uY8z6BSaFrpM",
                "Incoming Unit Test Simulation transaction from "
                "PD1jFsimY3DQUe7qGtx3z8BohTaT6r4kwJMCYXwp7uY8z6BSaFrpM",
                "",
                ot::otx::client::StorageBox::BLOCKCHAIN,
                std::nullopt,
                ot::blockchain::HashToNumber(transactions_.at(2)),
            },
            {
                false,
                false,
                false,
                1,
                100000000,
                u8"1 units",
                "PD1jFsimY3DQUe7qGtx3z8BohTaT6r4kwJMCYXwp7uY8z6BSaFrpM",
                "Incoming Unit Test Simulation transaction from "
                "PD1jFsimY3DQUe7qGtx3z8BohTaT6r4kwJMCYXwp7uY8z6BSaFrpM",
                "",
                ot::otx::client::StorageBox::BLOCKCHAIN,
                std::nullopt,
                ot::blockchain::HashToNumber(transactions_.at(3)),
            },
        },
    };
    wait_for_counter(contact_activity_alex_bob_, false);

    EXPECT_TRUE(check_contact_activity(alex_, contact, expected));
    EXPECT_TRUE(check_contact_activity_qt(alex_, contact, expected));
}

TEST_F(Regtest_payment_code, bob_account_activity_after_confirmation)
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
        799999448,
        u8"7.999\u202F994\u202F48 units",
        "",
        {},
        {test_chain_},
        100,
        {height_, height_},
        {},
        {{u8"9", u8"9 units"}},
        {
            {
                ot::otx::client::StorageBox::BLOCKCHAIN,
                -1,
                -100000236,
                u8"-1.000\u202F002\u202F36 units",
                {expectedContact.asBase58(ot_.Crypto())},
                "",
                "",
                "Outgoing Unit Test Simulation transaction to "
                "PD1jTsa1rjnbMMLVbj5cg2c8KkFY32KWtPRqVVpSBkv1jf8zjHJVu",
                ot::blockchain::HashToNumber(transactions_.at(3)),
                std::nullopt,
                1,
            },
            {
                ot::otx::client::StorageBox::BLOCKCHAIN,
                -1,
                -100000316,
                u8"-1.000\u202F003\u202F16 units",
                {expectedContact.asBase58(ot_.Crypto())},
                "",
                "",
                "Outgoing Unit Test Simulation transaction to "
                "PD1jTsa1rjnbMMLVbj5cg2c8KkFY32KWtPRqVVpSBkv1jf8zjHJVu",
                ot::blockchain::HashToNumber(transactions_.at(2)),
                std::nullopt,
                1,
            },
            {
                ot::otx::client::StorageBox::BLOCKCHAIN,
                1,
                1000000000,
                u8"10 units",
                {expectedContact.asBase58(ot_.Crypto())},
                "",
                "",
                "Incoming Unit Test Simulation transaction from "
                "PD1jTsa1rjnbMMLVbj5cg2c8KkFY32KWtPRqVVpSBkv1jf8zjHJVu",
                ot::blockchain::HashToNumber(transactions_.at(1)),
                std::nullopt,
                1,
            },
        },
    };
    wait_for_counter(account_activity_bob_, false);

    EXPECT_TRUE(check_account_activity(bob_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(bob_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(bob_, id, expected));
}

TEST_F(Regtest_payment_code, bob_account_list_after_confirmation)
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
         799999448,
         u8"7.999\u202F994\u202F48 units"},
    }};
    wait_for_counter(account_list_bob_, false);

    EXPECT_TRUE(check_account_list(bob_, expected));
    EXPECT_TRUE(check_account_list_qt(bob_, expected));
    EXPECT_TRUE(check_account_list_rpc(bob_, expected));
}

TEST_F(Regtest_payment_code, bob_account_tree_after_confirmation)
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
               799999448,
               u8"7.999\u202F994\u202F48 units"},
          }}}};
    wait_for_counter(account_tree_bob_, false);

    EXPECT_TRUE(check_account_tree(bob_, expected));
    EXPECT_TRUE(check_account_tree_qt(bob_, expected));
}

TEST_F(Regtest_payment_code, bob_contact_activity_after_confirmation)
{
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
            {
                false,
                false,
                true,
                -1,
                -100000316,
                u8"-1.000\u202F003\u202F16 units",
                bob_.name_,
                "Outgoing Unit Test Simulation transaction to "
                "PD1jTsa1rjnbMMLVbj5cg2c8KkFY32KWtPRqVVpSBkv1jf8zjHJVu",
                "",
                ot::otx::client::StorageBox::BLOCKCHAIN,
                std::nullopt,
                ot::blockchain::HashToNumber(transactions_.at(2)),
            },
            {
                false,
                false,
                true,
                -1,
                -100000236,
                u8"-1.000\u202F002\u202F36 units",
                bob_.name_,
                "Outgoing Unit Test Simulation transaction to "
                "PD1jTsa1rjnbMMLVbj5cg2c8KkFY32KWtPRqVVpSBkv1jf8zjHJVu",
                "",
                ot::otx::client::StorageBox::BLOCKCHAIN,
                std::nullopt,
                ot::blockchain::HashToNumber(transactions_.at(3)),
            },
        },
    };
    wait_for_counter(contact_activity_bob_alex_, false);

    EXPECT_TRUE(check_contact_activity(bob_, contact, expected));
    EXPECT_TRUE(check_contact_activity_qt(bob_, contact, expected));
}

TEST_F(Regtest_payment_code, shutdown) { Shutdown(); }
}  // namespace ottest
