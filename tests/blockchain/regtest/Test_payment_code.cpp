// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "Helpers.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <algorithm>
#include <optional>
#include <string>
#include <utility>

#include "integration/Helpers.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/network/Blockchain.hpp"
#include "opentxs/api/network/Network.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/OTX.hpp"
#include "opentxs/blockchain/FilterType.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/GCS.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/bitcoin/Block.hpp"
#include "opentxs/blockchain/block/bitcoin/Input.hpp"
#include "opentxs/blockchain/block/bitcoin/Inputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Output.hpp"
#include "opentxs/blockchain/block/bitcoin/Outputs.hpp"
#include "opentxs/blockchain/block/bitcoin/Script.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/blockchain/crypto/Account.hpp"
#include "opentxs/blockchain/crypto/Element.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/PaymentCode.hpp"
#include "opentxs/blockchain/crypto/SubaccountType.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/BlockOracle.hpp"
#include "opentxs/blockchain/node/FilterOracle.hpp"
#include "opentxs/blockchain/node/HeaderOracle.hpp"
#include "opentxs/blockchain/node/Manager.hpp"
#include "opentxs/blockchain/node/TxoTag.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/Server.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/otx/LastReplyStatus.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/Time.hpp"
#include "paymentcode/VectorsV3.hpp"
#include "rpc/Helpers.hpp"
#include "ui/Helpers.hpp"

namespace ottest
{
Counter account_activity_alice_{};
Counter account_activity_bob_{};
Counter account_list_alice_{};
Counter account_list_bob_{};
Counter contact_list_alice_{};
Counter contact_list_bob_{};
Counter activity_thread_alice_bob_{};
Counter activity_thread_bob_alice_{};

TEST_F(Regtest_payment_code, init_opentxs) {}

TEST_F(Regtest_payment_code, start_chains) { EXPECT_TRUE(Start()); }

TEST_F(Regtest_payment_code, connect_peers)
{
    // TODO find out why inproc peers sometimes time out
    const auto connected = Connect();

    EXPECT_TRUE(connected);

    OT_ASSERT(connected);
}

TEST_F(Regtest_payment_code, init_ui_models)
{
    account_activity_alice_.expected_ += 0;
    account_activity_bob_.expected_ += 0;
    account_list_alice_.expected_ += 1;
    account_list_bob_.expected_ += 1;
    contact_list_alice_.expected_ += 1;
    contact_list_bob_.expected_ += 1;
    init_account_activity(
        alice_, SendHD().Parent().AccountID(), account_activity_alice_);
    init_account_list(alice_, account_list_alice_);
    init_contact_list(alice_, contact_list_alice_);

    init_account_activity(
        bob_, ReceiveHD().Parent().AccountID(), account_activity_bob_);
    init_account_list(bob_, account_list_bob_);
    init_contact_list(bob_, contact_list_bob_);
}

TEST_F(Regtest_payment_code, alice_contact_list_initial)
{
    const auto expected = ContactListData{{
        {true, alice_.name_, alice_.name_, "ME", ""},
    }};

    ASSERT_TRUE(wait_for_counter(contact_list_alice_, false));
    EXPECT_TRUE(check_contact_list(alice_, expected));
    EXPECT_TRUE(check_contact_list_qt(alice_, expected));
}

TEST_F(Regtest_payment_code, alice_account_activity_initial)
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
        {},
        {{u8"0", u8"0 units"}},
        {},
    };

    ASSERT_TRUE(wait_for_counter(account_activity_alice_, false));
    EXPECT_TRUE(check_account_activity(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(alice_, id, expected));
}

TEST_F(Regtest_payment_code, alice_account_list_initial)
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

    ASSERT_TRUE(wait_for_counter(account_list_alice_, false));
    EXPECT_TRUE(check_account_list(alice_, expected));
    EXPECT_TRUE(check_account_list_qt(alice_, expected));
    EXPECT_TRUE(check_account_list_rpc(alice_, expected));
}

TEST_F(Regtest_payment_code, bob_contact_list_initial)
{
    const auto expected = ContactListData{{
        {true, bob_.name_, bob_.name_, "ME", ""},
    }};

    ASSERT_TRUE(wait_for_counter(contact_list_bob_, false));
    EXPECT_TRUE(check_contact_list(bob_, expected));
    EXPECT_TRUE(check_contact_list_qt(bob_, expected));
}

TEST_F(Regtest_payment_code, bob_account_activity_initial)
{
    const auto& id = ReceiveHD().Parent().AccountID();
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
        {},
        {{u8"0", u8"0 units"}},
        {},
    };

    ASSERT_TRUE(wait_for_counter(account_activity_bob_, false));
    EXPECT_TRUE(check_account_activity(bob_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(bob_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(bob_, id, expected));
}

TEST_F(Regtest_payment_code, bob_account_list_initial)
{
    const auto expected = AccountListData{{
        {ReceiveHD().Parent().AccountID().str(),
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

    ASSERT_TRUE(wait_for_counter(account_list_bob_, false));
    EXPECT_TRUE(check_account_list(bob_, expected));
    EXPECT_TRUE(check_account_list_qt(bob_, expected));
    EXPECT_TRUE(check_account_list_rpc(bob_, expected));
}

TEST_F(Regtest_payment_code, mine_initial_balance)
{
    constexpr auto orphan{0};
    constexpr auto count{1};
    const auto start = height_ - orphan;
    const auto end{start + count};
    auto future = listener_alice_.get_future(SendHD(), Subchain::External, end);
    account_activity_alice_.expected_ += (count + 1);
    account_activity_bob_.expected_ += count;
    account_list_alice_.expected_ += 0;
    account_list_bob_.expected_ += 0;

    EXPECT_EQ(start, 0);
    EXPECT_EQ(end, 1);
    EXPECT_TRUE(Mine(start, count, mine_to_alice_));
    EXPECT_TRUE(listener_alice_.wait(future));
    EXPECT_TRUE(txos_alice_.Mature(end));
    EXPECT_TRUE(txos_bob_.Mature(end));
}

TEST_F(Regtest_payment_code, mature_initial_balance)
{
    constexpr auto orphan{0};
    const auto count = static_cast<int>(MaturationInterval());
    const auto start = height_ - orphan;
    const auto end{start + count};
    auto future = listener_alice_.get_future(SendHD(), Subchain::External, end);
    account_activity_alice_.expected_ += ((2 * count) + 1);
    account_activity_bob_.expected_ += count;
    account_list_alice_.expected_ += 1;
    account_list_bob_.expected_ += 0;

    EXPECT_EQ(start, 1);
    EXPECT_EQ(end, 11);
    EXPECT_TRUE(Mine(start, count));
    EXPECT_TRUE(listener_alice_.wait(future));
    EXPECT_TRUE(txos_alice_.Mature(end));
    EXPECT_TRUE(txos_bob_.Mature(end));
}

TEST_F(Regtest_payment_code, first_block)
{
    const auto& blockchain =
        client_1_.Network().Blockchain().GetChain(test_chain_);
    const auto blockHash = blockchain.HeaderOracle().BestHash(1);

    ASSERT_FALSE(blockHash->empty());

    const auto pBlock = blockchain.BlockOracle().LoadBitcoin(blockHash).get();

    ASSERT_TRUE(pBlock);

    const auto& block = *pBlock;

    ASSERT_EQ(block.size(), 1);

    const auto pTx = block.at(0);

    ASSERT_TRUE(pTx);

    const auto& tx = *pTx;

    EXPECT_EQ(tx.ID(), transactions_.at(0));
    EXPECT_EQ(tx.BlockPosition(), 0);
    ASSERT_EQ(tx.Outputs().size(), 1);
    EXPECT_TRUE(tx.IsGeneration());
}

TEST_F(Regtest_payment_code, alice_account_activity_initial_receive)
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
                ot::StorageBox::BLOCKCHAIN,
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

    ASSERT_TRUE(wait_for_counter(account_activity_alice_, false));
    EXPECT_TRUE(check_account_activity(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(alice_, id, expected));
}

TEST_F(Regtest_payment_code, alice_account_list_initial_receive)
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
         10000000000,
         u8"100 units"},
    }};

    ASSERT_TRUE(wait_for_counter(account_list_alice_, false));
    EXPECT_TRUE(check_account_list(alice_, expected));
    EXPECT_TRUE(check_account_list_qt(alice_, expected));
    EXPECT_TRUE(check_account_list_rpc(alice_, expected));
}

TEST_F(Regtest_payment_code, alice_txodb_inital_receive)
{
    EXPECT_TRUE(CheckTXODBAlice());
}

TEST_F(Regtest_payment_code, send_to_bob)
{
    account_activity_alice_.expected_ += 2;
    account_activity_bob_.expected_ += 2;
    account_list_alice_.expected_ += 1;
    account_list_bob_.expected_ += 1;
    contact_list_alice_.expected_ += 1;
    contact_list_bob_.expected_ += 1;
    const auto& network =
        client_1_.Network().Blockchain().GetChain(test_chain_);
    auto future = network.SendToPaymentCode(
        alice_.nym_id_,
        client_1_.Factory().PaymentCode(GetVectors3().bob_.payment_code_),
        1000000000,
        memo_outgoing_);
    const auto& txid = transactions_.emplace_back(future.get().second);

    ASSERT_FALSE(txid->empty());

    {
        const auto pTX =
            client_1_.Crypto().Blockchain().LoadTransactionBitcoin(txid);

        ASSERT_TRUE(pTX);

        const auto& tx = *pTX;
        EXPECT_TRUE(
            txos_alice_.SpendUnconfirmed({transactions_.at(0)->Bytes(), 0}));
        EXPECT_TRUE(txos_alice_.AddUnconfirmed(tx, 1, SendHD()));
        // NOTE do not update Bob's txos since the recipient payment code
        // subaccount does not exist yet.
    }

    {
        const auto& element = SendPC().BalanceElement(Subchain::Outgoing, 0);
        const auto amount = ot::blockchain::Amount{1000000000};
        expected_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(txid->Bytes(), 0),
            std::forward_as_tuple(
                client_1_.Factory().Data(element.Key()->PublicKey()),
                amount,
                Pattern::PayToPubkey));
    }
    {
        const auto& element = SendHD().BalanceElement(Subchain::Internal, 0);
        const auto amount = ot::blockchain::Amount{8999999684};
        expected_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(txid->Bytes(), 1),
            std::forward_as_tuple(
                client_1_.Factory().Data(element.Key()->PublicKey()),
                amount,
                Pattern::PayToMultisig));
    }
}

TEST_F(Regtest_payment_code, first_outgoing_transaction)
{
    const auto& api = client_1_;
    const auto& blockchain = api.Crypto().Blockchain();
    const auto& chain = api.Network().Blockchain().GetChain(test_chain_);
    const auto& contact = api.Contacts();
    const auto& me = alice_.nym_id_;
    const auto self = contact.ContactID(me);
    const auto other = contact.ContactID(bob_.nym_id_);
    const auto& txid = transactions_.at(1).get();
    const auto pTX = blockchain.LoadTransactionBitcoin(txid);

    ASSERT_TRUE(pTX);

    const auto& tx = *pTX;

    {
        const auto nyms = tx.AssociatedLocalNyms();

        EXPECT_GT(nyms.size(), 0);

        if (0 < nyms.size()) {
            EXPECT_EQ(nyms.size(), 1);
            EXPECT_EQ(nyms.front(), me);
        }
    }
    {
        const auto contacts = tx.AssociatedRemoteContacts(contact, me);

        EXPECT_GT(contacts.size(), 0);

        if (0 < contacts.size()) {
            EXPECT_EQ(contacts.size(), 1);
            EXPECT_EQ(contacts.front(), other);
        }
    }
    {
        ASSERT_EQ(tx.Inputs().size(), 1u);

        const auto& script = tx.Inputs().at(0u).Script();

        ASSERT_EQ(script.size(), 1u);

        const auto& sig = script.at(0u);

        ASSERT_TRUE(sig.data_.has_value());
        EXPECT_GE(sig.data_.value().size(), 70u);
        EXPECT_LE(sig.data_.value().size(), 74u);
    }
    {
        ASSERT_EQ(tx.Outputs().size(), 2u);

        const auto& payment = tx.Outputs().at(0);
        const auto& change = tx.Outputs().at(1);

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

TEST_F(Regtest_payment_code, alice_contact_list_first_spend_unconfirmed)
{
    const auto expected = ContactListData{{
        {true, alice_.name_, alice_.name_, "ME", ""},
        {false, bob_.name_, bob_.payment_code_, "P", ""},
    }};

    ASSERT_TRUE(wait_for_counter(contact_list_alice_, false));
    EXPECT_TRUE(check_contact_list(alice_, expected));
    EXPECT_TRUE(check_contact_list_qt(alice_, expected));
    EXPECT_TRUE(CheckContactID(alice_, bob_, GetVectors3().bob_.payment_code_));
}

TEST_F(Regtest_payment_code, alice_account_activity_first_spend_unconfirmed)
{
    const auto expectedContact = client_1_.Contacts().ContactID(bob_.nym_id_);
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
                ot::StorageBox::BLOCKCHAIN,
                -1,
                -1000000316,
                u8"-10.000\u202F003\u202F16 units",
                {expectedContact->str()},
                "",
                "",
                "Outgoing Unit Test Simulation transaction to "
                "PD1jFsimY3DQUe7qGtx3z8BohTaT6r4kwJMCYXwp7uY8z6BSaFrpM",
                ot::blockchain::HashToNumber(transactions_.at(1)),
                std::nullopt,
                0,
            },
            {
                ot::StorageBox::BLOCKCHAIN,
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

    ASSERT_TRUE(wait_for_counter(account_activity_alice_, false));
    EXPECT_TRUE(check_account_activity(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(alice_, id, expected));
}

TEST_F(Regtest_payment_code, alice_account_list_first_spend_unconfirmed)
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
         8999999684,
         u8"89.999\u202F996\u202F84 units"},
    }};

    ASSERT_TRUE(wait_for_counter(account_list_alice_, false));
    EXPECT_TRUE(check_account_list(alice_, expected));
    EXPECT_TRUE(check_account_list_qt(alice_, expected));
    EXPECT_TRUE(check_account_list_rpc(alice_, expected));
}

TEST_F(Regtest_payment_code, alice_activity_thread_first_spend_unconfirmed)
{
    activity_thread_alice_bob_.expected_ += 3;
    init_activity_thread(alice_, bob_, activity_thread_alice_bob_);
    const auto& contact = alice_.Contact(bob_.name_);
    const auto expected = ActivityThreadData{
        false,
        contact.str(),
        bob_.payment_code_,
        "",
        contact.str(),
        {{ot::core::UnitType::Regtest, bob_.payment_code_}},
        {
            {
                false,
                false,
                true,
                -1,
                -1000000316,
                u8"-10.000\u202F003\u202F16 units",
                alice_.name_,
                "Outgoing Unit Test Simulation transaction to "
                "PD1jFsimY3DQUe7qGtx3z8BohTaT6r4kwJMCYXwp7uY8z6BSaFrpM",
                "",
                ot::StorageBox::BLOCKCHAIN,
                std::nullopt,
            },
        },
    };

    ASSERT_TRUE(wait_for_counter(activity_thread_alice_bob_, false));
    EXPECT_TRUE(check_activity_thread(alice_, contact, expected));
    EXPECT_TRUE(check_activity_thread_qt(alice_, contact, expected));
}

TEST_F(Regtest_payment_code, alice_txodb_first_spend_unconfirmed)
{
    EXPECT_TRUE(CheckTXODBAlice());
}

TEST_F(Regtest_payment_code, bob_contact_list_first_unconfirmed_incoming)
{
    const auto expected = ContactListData{{
        {true, bob_.name_, bob_.name_, "ME", ""},
        {false, alice_.name_, alice_.payment_code_, "P", ""},
    }};

    ASSERT_TRUE(wait_for_counter(contact_list_bob_, false));
    EXPECT_TRUE(check_contact_list(bob_, expected));
    EXPECT_TRUE(check_contact_list_qt(bob_, expected));
    EXPECT_TRUE(
        CheckContactID(bob_, alice_, GetVectors3().alice_.payment_code_));
}

TEST_F(Regtest_payment_code, bob_account_activity_first_unconfirmed_incoming)
{
    const auto expectedContact = client_2_.Contacts().ContactID(alice_.nym_id_);
    const auto& id = ReceiveHD().Parent().AccountID();
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
                ot::StorageBox::BLOCKCHAIN,
                1,
                1000000000,
                u8"10 units",
                {expectedContact->str()},
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

    ASSERT_TRUE(wait_for_counter(account_activity_bob_, false));
    EXPECT_TRUE(check_account_activity(bob_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(bob_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(bob_, id, expected));
}

TEST_F(Regtest_payment_code, bob_account_list_first_unconfirmed_incoming)
{
    const auto expected = AccountListData{{
        {ReceiveHD().Parent().AccountID().str(),
         expected_unit_.str(),
         expected_display_unit_,
         expected_account_name_,
         expected_notary_.str(),
         expected_notary_name_,
         expected_account_type_,
         expected_unit_type_,
         1,
         1000000000,
         u8"10 units"},
    }};

    ASSERT_TRUE(wait_for_counter(account_list_bob_, false));
    EXPECT_TRUE(check_account_list(bob_, expected));
    EXPECT_TRUE(check_account_list_qt(bob_, expected));
    EXPECT_TRUE(check_account_list_rpc(bob_, expected));
}

TEST_F(Regtest_payment_code, bob_activity_thread_first_unconfirmed_incoming)
{
    activity_thread_bob_alice_.expected_ += 3;
    init_activity_thread(bob_, alice_, activity_thread_bob_alice_);
    const auto& contact = bob_.Contact(alice_.name_);
    const auto expected = ActivityThreadData{
        false,
        contact.str(),
        alice_.payment_code_,
        "",
        contact.str(),
        {{ot::core::UnitType::Regtest, alice_.payment_code_}},
        {
            {
                false,
                false,
                false,
                1,
                1000000000,
                "10 units",
                alice_.payment_code_,
                "Incoming Unit Test Simulation transaction from "
                "PD1jTsa1rjnbMMLVbj5cg2c8KkFY32KWtPRqVVpSBkv1jf8zjHJVu",
                "",
                ot::StorageBox::BLOCKCHAIN,
                std::nullopt,
            },
        },
    };

    ASSERT_TRUE(wait_for_counter(activity_thread_bob_alice_, false));
    EXPECT_TRUE(check_activity_thread(bob_, contact, expected));
    EXPECT_TRUE(check_activity_thread_qt(bob_, contact, expected));
}

TEST_F(Regtest_payment_code, bob_txodb_first_unconfirmed_incoming)
{
    {
        // NOTE normally this would be done when the transaction was first sent
        // but the subaccount returned by ReceivePC() did not exist yet.
        const auto pTX = client_1_.Crypto().Blockchain().LoadTransactionBitcoin(
            transactions_.at(1));

        ASSERT_TRUE(pTX);

        const auto& tx = *pTX;
        EXPECT_TRUE(txos_bob_.AddUnconfirmed(tx, 0, ReceivePC()));
    }

    EXPECT_TRUE(CheckTXODBBob());
}

TEST_F(Regtest_payment_code, confirm_send)
{
    constexpr auto orphan{0};
    constexpr auto count{1};
    const auto start = height_ - orphan;
    const auto end{start + count};
    auto future1 =
        listener_alice_.get_future(SendHD(), Subchain::External, end);
    auto future2 =
        listener_alice_.get_future(SendHD(), Subchain::Internal, end);
    account_activity_alice_.expected_ += ((2 * count) + 4);
    account_activity_bob_.expected_ += (count + 2);
    account_list_alice_.expected_ += 2;
    account_list_bob_.expected_ += 0;
    const auto& txid = transactions_.at(1).get();
    const auto extra = [&] {
        auto output = std::pmr::vector<Transaction>{};
        const auto pTX = output.emplace_back(
            client_1_.Crypto().Blockchain().LoadTransactionBitcoin(txid));

        OT_ASSERT(pTX);

        return output;
    }();

    EXPECT_EQ(start, 11);
    EXPECT_EQ(end, 12);
    EXPECT_TRUE(Mine(start, count, default_, extra));
    EXPECT_TRUE(listener_alice_.wait(future1));
    EXPECT_TRUE(listener_alice_.wait(future2));
    EXPECT_TRUE(txos_alice_.Mature(end));
    EXPECT_TRUE(txos_alice_.Confirm(transactions_.at(0)));
    EXPECT_TRUE(txos_alice_.Confirm(txid));
    EXPECT_TRUE(txos_bob_.Mature(end));
    EXPECT_TRUE(txos_bob_.Confirm(txid));
}

TEST_F(Regtest_payment_code, second_block)
{
    const auto& blockchain =
        client_1_.Network().Blockchain().GetChain(test_chain_);
    const auto blockHash = blockchain.HeaderOracle().BestHash(height_);
    auto expected = std::pmr::vector<ot::Space>{};

    ASSERT_FALSE(blockHash->empty());

    const auto pBlock = blockchain.BlockOracle().LoadBitcoin(blockHash).get();

    ASSERT_TRUE(pBlock);

    const auto& block = *pBlock;

    ASSERT_EQ(block.size(), 2);

    {
        const auto pTx = block.at(0);

        ASSERT_TRUE(pTx);

        const auto& tx = *pTx;
        expected.emplace_back(ot::space(tx.ID().Bytes()));

        EXPECT_EQ(tx.BlockPosition(), 0);
        EXPECT_EQ(tx.Outputs().size(), 1);
    }

    {
        const auto pTx = block.at(1);

        ASSERT_TRUE(pTx);

        const auto& tx = *pTx;
        expected.emplace_back(ot::space(tx.ID().Bytes()));

        EXPECT_EQ(tx.ID(), transactions_.at(1));
        EXPECT_EQ(tx.BlockPosition(), 1);
        EXPECT_FALSE(tx.IsGeneration());
        ASSERT_EQ(tx.Inputs().size(), 1);

        {
            const auto& input = tx.Inputs().at(0);
            expected.emplace_back(ot::space(input.PreviousOutput().Bytes()));
        }

        ASSERT_EQ(tx.Outputs().size(), 2);

        {
            const auto& output = tx.Outputs().at(0);
            const auto& script = output.Script();

            ASSERT_EQ(script.Type(), Pattern::PayToPubkey);

            const auto bytes = script.Pubkey();

            ASSERT_TRUE(bytes.has_value());

            expected.emplace_back(ot::space(bytes.value()));
        }
        {
            const auto& output = tx.Outputs().at(1);
            const auto& script = output.Script();

            ASSERT_EQ(script.Type(), Pattern::PayToMultisig);

            for (auto i{0u}; i < 3u; ++i) {
                const auto bytes = script.MultisigPubkey(i);

                ASSERT_TRUE(bytes.has_value());

                expected.emplace_back(ot::space(bytes.value()));
            }
        }
    }

    {
        auto elements = block.Internal().ExtractElements(FilterType::ES);
        std::sort(elements.begin(), elements.end());
        std::sort(expected.begin(), expected.end());

        EXPECT_EQ(expected.size(), 7);
        EXPECT_EQ(elements.size(), expected.size());
        EXPECT_EQ(elements, expected);
    }

    const auto pFilter =
        blockchain.FilterOracle().LoadFilter(FilterType::ES, blockHash);

    ASSERT_TRUE(pFilter);

    const auto& filter = *pFilter;

    for (const auto& element : expected) {
        EXPECT_TRUE(filter.Test(ot::reader(element)));
    }
}

TEST_F(Regtest_payment_code, alice_account_activity_first_spend_confirmed)
{
    const auto expectedContact = client_1_.Contacts().ContactID(bob_.nym_id_);
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
                ot::StorageBox::BLOCKCHAIN,
                -1,
                -1000000316,
                u8"-10.000\u202F003\u202F16 units",
                {expectedContact->str()},
                "",
                "",
                "Outgoing Unit Test Simulation transaction to "
                "PD1jFsimY3DQUe7qGtx3z8BohTaT6r4kwJMCYXwp7uY8z6BSaFrpM",
                ot::blockchain::HashToNumber(transactions_.at(1)),
                std::nullopt,
                1,
            },
            {
                ot::StorageBox::BLOCKCHAIN,
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

    ASSERT_TRUE(wait_for_counter(account_activity_alice_, false));
    EXPECT_TRUE(check_account_activity(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(alice_, id, expected));

    const auto& tree =
        client_1_.Crypto().Blockchain().Account(alice_.nym_id_, test_chain_);
    const auto& pc = tree.GetPaymentCode();

    ASSERT_EQ(pc.size(), 1);

    const auto& account = pc.at(0);
    const auto lookahead = account.Lookahead() - 1;

    EXPECT_EQ(account.Type(), bca::SubaccountType::PaymentCode);
    EXPECT_TRUE(account.IsNotified());

    {
        constexpr auto subchain{Subchain::Incoming};

        const auto gen = account.LastGenerated(subchain);

        ASSERT_TRUE(gen.has_value());
        EXPECT_EQ(gen.value(), lookahead);
    }
    {
        constexpr auto subchain{Subchain::Outgoing};

        const auto gen = account.LastGenerated(subchain);

        ASSERT_TRUE(gen.has_value());
        EXPECT_EQ(gen.value(), lookahead);
    }
    {
        constexpr auto subchain{Subchain::External};

        const auto gen = account.LastGenerated(subchain);

        EXPECT_FALSE(gen.has_value());
    }
    {
        constexpr auto subchain{Subchain::Internal};

        const auto gen = account.LastGenerated(subchain);

        EXPECT_FALSE(gen.has_value());
    }
}

TEST_F(Regtest_payment_code, alice_account_list_first_spend_confirmed)
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
         8999999684,
         u8"89.999\u202F996\u202F84 units"},
    }};

    ASSERT_TRUE(wait_for_counter(account_list_alice_, false));
    EXPECT_TRUE(check_account_list(alice_, expected));
    EXPECT_TRUE(check_account_list_qt(alice_, expected));
    EXPECT_TRUE(check_account_list_rpc(alice_, expected));
}

TEST_F(Regtest_payment_code, alice_txodb_first_spend_confirmed)
{
    EXPECT_TRUE(CheckTXODBAlice());
}

TEST_F(Regtest_payment_code, bob_account_activity_first_spend_confirmed)
{
    const auto expectedContact = client_2_.Contacts().ContactID(alice_.nym_id_);
    const auto& id = ReceiveHD().Parent().AccountID();
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
                ot::StorageBox::BLOCKCHAIN,
                1,
                1000000000,
                u8"10 units",
                {expectedContact->str()},
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

    ASSERT_TRUE(wait_for_counter(account_activity_bob_, false));
    EXPECT_TRUE(check_account_activity(bob_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(bob_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(bob_, id, expected));

    const auto& tree =
        client_2_.Crypto().Blockchain().Account(bob_.nym_id_, test_chain_);
    const auto& pc = tree.GetPaymentCode();

    ASSERT_EQ(pc.size(), 1);

    const auto& account = pc.at(0);
    const auto lookahead = account.Lookahead() - 1u;

    EXPECT_EQ(account.Type(), bca::SubaccountType::PaymentCode);
    EXPECT_FALSE(account.IsNotified());

    {
        constexpr auto subchain{Subchain::Incoming};

        const auto gen = account.LastGenerated(subchain);

        ASSERT_TRUE(gen.has_value());
        EXPECT_EQ(gen.value(), lookahead);
    }
    {
        constexpr auto subchain{Subchain::Outgoing};

        const auto gen = account.LastGenerated(subchain);

        ASSERT_TRUE(gen.has_value());
        EXPECT_EQ(gen.value(), lookahead);
    }
    {
        constexpr auto subchain{Subchain::External};

        const auto gen = account.LastGenerated(subchain);

        EXPECT_FALSE(gen.has_value());
    }
    {
        constexpr auto subchain{Subchain::Internal};

        const auto gen = account.LastGenerated(subchain);

        EXPECT_FALSE(gen.has_value());
    }
}

TEST_F(Regtest_payment_code, bob_account_list_first_spend_confirmed)
{
    const auto expected = AccountListData{{
        {ReceiveHD().Parent().AccountID().str(),
         expected_unit_.str(),
         expected_display_unit_,
         expected_account_name_,
         expected_notary_.str(),
         expected_notary_name_,
         expected_account_type_,
         expected_unit_type_,
         1,
         1000000000,
         u8"10 units"},
    }};

    ASSERT_TRUE(wait_for_counter(account_list_bob_, false));
    EXPECT_TRUE(check_account_list(bob_, expected));
    EXPECT_TRUE(check_account_list_qt(bob_, expected));
    EXPECT_TRUE(check_account_list_rpc(bob_, expected));
}

TEST_F(Regtest_payment_code, bob_first_incoming_transaction)
{
    const auto& api = client_2_;
    const auto& blockchain = api.Crypto().Blockchain();
    const auto& contact = api.Contacts();
    const auto& me = bob_.nym_id_;
    const auto self = contact.ContactID(me);
    const auto other = contact.ContactID(alice_.nym_id_);
    const auto& txid = transactions_.at(1).get();
    const auto pTX = blockchain.LoadTransactionBitcoin(txid);

    ASSERT_TRUE(pTX);

    const auto& tx = *pTX;

    {
        const auto nyms = tx.AssociatedLocalNyms();

        EXPECT_GT(nyms.size(), 0);

        if (0 < nyms.size()) {
            EXPECT_EQ(nyms.size(), 1);
            EXPECT_EQ(nyms.front(), me);
        }
    }
    {
        const auto contacts = tx.AssociatedRemoteContacts(contact, me);

        EXPECT_GT(contacts.size(), 0);

        if (0 < contacts.size()) {
            EXPECT_EQ(contacts.size(), 1);
            EXPECT_EQ(contacts.front(), other);
        }
    }
    {
        ASSERT_EQ(tx.Inputs().size(), 1u);

        const auto& script = tx.Inputs().at(0u).Script();

        ASSERT_EQ(script.size(), 1u);

        const auto& sig = script.at(0u);

        ASSERT_TRUE(sig.data_.has_value());
        EXPECT_GE(sig.data_.value().size(), 70u);
        EXPECT_LE(sig.data_.value().size(), 74u);
    }
    {
        ASSERT_EQ(tx.Outputs().size(), 2u);

        const auto& payment = tx.Outputs().at(0);
        const auto& change = tx.Outputs().at(1);

        EXPECT_EQ(payment.Payer(), other);
        EXPECT_EQ(payment.Payee(), self);
        EXPECT_TRUE(change.Payer()->empty());
        EXPECT_TRUE(change.Payee()->empty());
    }
}

TEST_F(Regtest_payment_code, bob_txodb_first_spend_confirmed)
{
    EXPECT_TRUE(CheckTXODBBob());
}

TEST_F(Regtest_payment_code, send_to_bob_again)
{
    account_activity_alice_.expected_ += 2;
    account_activity_bob_.expected_ += 2;
    account_list_alice_.expected_ += 1;
    account_list_bob_.expected_ += 1;
    activity_thread_alice_bob_.expected_ += 1;
    activity_thread_bob_alice_.expected_ += 1;
    const auto& network =
        client_1_.Network().Blockchain().GetChain(test_chain_);
    auto future = network.SendToPaymentCode(
        alice_.nym_id_,
        client_1_.Factory().PaymentCode(GetVectors3().bob_.payment_code_),
        1500000000,
        memo_outgoing_);
    const auto& txid = transactions_.emplace_back(future.get().second);

    ASSERT_FALSE(txid->empty());

    {
        const auto pTX =
            client_1_.Crypto().Blockchain().LoadTransactionBitcoin(txid);

        ASSERT_TRUE(pTX);

        const auto& tx = *pTX;
        EXPECT_TRUE(
            txos_alice_.SpendUnconfirmed({transactions_.at(1)->Bytes(), 1}));
        EXPECT_TRUE(txos_bob_.AddUnconfirmed(tx, 0, ReceivePC()));
        EXPECT_TRUE(txos_alice_.AddUnconfirmed(tx, 1, SendHD()));
    }

    {
        const auto& element = SendPC().BalanceElement(Subchain::Outgoing, 1);
        const auto amount = ot::blockchain::Amount{1500000000};
        expected_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(txid->Bytes(), 0),
            std::forward_as_tuple(
                client_1_.Factory().Data(element.Key()->PublicKey()),
                amount,
                Pattern::PayToPubkey));
    }
    {
        const auto& element = SendHD().BalanceElement(Subchain::Internal, 1);
        const auto amount = ot::blockchain::Amount{7499999448};
        expected_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(txid->Bytes(), 1),
            std::forward_as_tuple(
                element.PubkeyHash(), amount, Pattern::PayToPubkeyHash));
    }
}

TEST_F(Regtest_payment_code, alice_account_activity_second_spend_unconfirmed)
{
    const auto expectedContact = client_1_.Contacts().ContactID(bob_.nym_id_);
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
        7499999448,
        u8"74.999\u202F994\u202F48 units",
        "",
        {},
        {test_chain_},
        100,
        {height_, height_},
        {},
        {{u8"74.99999448", u8"74.999\u202F994\u202F48 units"}},
        {
            {
                ot::StorageBox::BLOCKCHAIN,
                -1,
                -1500000236,
                u8"-15.000\u202F002\u202F36 units",
                {expectedContact->str()},
                "",
                "",
                "Outgoing Unit Test Simulation transaction to "
                "PD1jFsimY3DQUe7qGtx3z8BohTaT6r4kwJMCYXwp7uY8z6BSaFrpM",
                ot::blockchain::HashToNumber(transactions_.at(2)),
                std::nullopt,
                0,
            },
            {
                ot::StorageBox::BLOCKCHAIN,
                -1,
                -1000000316,
                u8"-10.000\u202F003\u202F16 units",
                {expectedContact->str()},
                "",
                "",
                "Outgoing Unit Test Simulation transaction to "
                "PD1jFsimY3DQUe7qGtx3z8BohTaT6r4kwJMCYXwp7uY8z6BSaFrpM",
                ot::blockchain::HashToNumber(transactions_.at(1)),
                std::nullopt,
                1,
            },
            {
                ot::StorageBox::BLOCKCHAIN,
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

    ASSERT_TRUE(wait_for_counter(account_activity_alice_, false));
    EXPECT_TRUE(check_account_activity(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(alice_, id, expected));

    const auto& tree =
        client_1_.Crypto().Blockchain().Account(alice_.nym_id_, test_chain_);
    const auto& pc = tree.GetPaymentCode();

    ASSERT_EQ(pc.size(), 1);

    const auto& account = pc.at(0);
    const auto lookahead = account.Lookahead() - 1u;

    EXPECT_EQ(account.Type(), bca::SubaccountType::PaymentCode);
    EXPECT_TRUE(account.IsNotified());

    {
        constexpr auto subchain{Subchain::Incoming};

        const auto gen = account.LastGenerated(subchain);

        ASSERT_TRUE(gen.has_value());
        EXPECT_EQ(gen.value(), lookahead);
    }
    {
        constexpr auto subchain{Subchain::Outgoing};

        const auto gen = account.LastGenerated(subchain);

        ASSERT_TRUE(gen.has_value());
        EXPECT_EQ(gen.value(), lookahead);
    }
    {
        constexpr auto subchain{Subchain::External};

        const auto gen = account.LastGenerated(subchain);

        EXPECT_FALSE(gen.has_value());
    }
    {
        constexpr auto subchain{Subchain::Internal};

        const auto gen = account.LastGenerated(subchain);

        EXPECT_FALSE(gen.has_value());
    }
}

TEST_F(Regtest_payment_code, alice_account_list_second_spend_unconfirmed)
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
         7499999448,
         u8"74.999\u202F994\u202F48 units"},
    }};

    ASSERT_TRUE(wait_for_counter(account_list_alice_, false));
    EXPECT_TRUE(check_account_list(alice_, expected));
    EXPECT_TRUE(check_account_list_qt(alice_, expected));
    EXPECT_TRUE(check_account_list_rpc(alice_, expected));
}

TEST_F(Regtest_payment_code, alice_activity_thread_second_spend_unconfirmed)
{
    const auto& contact = alice_.Contact(bob_.name_);
    const auto expected = ActivityThreadData{
        false,
        contact.str(),
        bob_.payment_code_,
        "",
        contact.str(),
        {{ot::core::UnitType::Regtest, bob_.payment_code_}},
        {
            {
                false,
                false,
                true,
                -1,
                -1000000316,
                u8"-10.000\u202F003\u202F16 units",
                alice_.name_,
                "Outgoing Unit Test Simulation transaction to "
                "PD1jFsimY3DQUe7qGtx3z8BohTaT6r4kwJMCYXwp7uY8z6BSaFrpM",
                "",
                ot::StorageBox::BLOCKCHAIN,
                std::nullopt,
            },
            {
                false,
                false,
                true,
                -1,
                -1500000236,
                u8"-15.000\u202F002\u202F36 units",
                alice_.name_,
                "Outgoing Unit Test Simulation transaction to "
                "PD1jFsimY3DQUe7qGtx3z8BohTaT6r4kwJMCYXwp7uY8z6BSaFrpM",
                "",
                ot::StorageBox::BLOCKCHAIN,
                std::nullopt,
            },
        },
    };

    ASSERT_TRUE(wait_for_counter(activity_thread_alice_bob_, false));
    EXPECT_TRUE(check_activity_thread(alice_, contact, expected));
    EXPECT_TRUE(check_activity_thread_qt(alice_, contact, expected));
}

TEST_F(Regtest_payment_code, alice_second_outgoing_transaction)
{
    const auto& api = client_1_;
    const auto& blockchain = api.Crypto().Blockchain();
    const auto& contact = api.Contacts();
    const auto& me = alice_.nym_id_;
    const auto self = contact.ContactID(me);
    const auto other = contact.ContactID(bob_.nym_id_);
    const auto& txid = transactions_.at(2).get();
    const auto pTX = blockchain.LoadTransactionBitcoin(txid);

    ASSERT_TRUE(pTX);

    const auto& tx = *pTX;

    {
        const auto nyms = tx.AssociatedLocalNyms();

        EXPECT_GT(nyms.size(), 0);

        if (0 < nyms.size()) {
            EXPECT_EQ(nyms.size(), 1);
            EXPECT_EQ(nyms.front(), me);
        }
    }
    {
        const auto contacts = tx.AssociatedRemoteContacts(contact, me);

        EXPECT_GT(contacts.size(), 0);

        if (0 < contacts.size()) {
            EXPECT_EQ(contacts.size(), 1);
            EXPECT_EQ(contacts.front(), other);
        }
    }
    {
        ASSERT_EQ(tx.Inputs().size(), 1u);

        const auto& script = tx.Inputs().at(0u).Script();

        ASSERT_EQ(script.size(), 2u);

        const auto& placeholder = script.at(0u);
        using OP = ot::blockchain::block::bitcoin::OP;

        EXPECT_EQ(placeholder.opcode_, OP::ZERO);

        const auto& sig = script.at(1u);

        ASSERT_TRUE(sig.data_.has_value());
        EXPECT_GE(sig.data_.value().size(), 70u);
        EXPECT_LE(sig.data_.value().size(), 74u);
    }
    {
        ASSERT_EQ(tx.Outputs().size(), 2u);

        const auto& payment = tx.Outputs().at(0);
        const auto& change = tx.Outputs().at(1);

        EXPECT_EQ(payment.Payer(), self);
        EXPECT_EQ(payment.Payee(), other);
        EXPECT_EQ(change.Payer(), self);
        EXPECT_EQ(change.Payee(), self);
    }
}

TEST_F(Regtest_payment_code, alice_txodb_second_spend_unconfirmed)
{
    EXPECT_TRUE(CheckTXODBAlice());
}

TEST_F(Regtest_payment_code, bob_account_activity_second_unconfirmed_incoming)
{
    const auto expectedContact = client_2_.Contacts().ContactID(alice_.nym_id_);
    const auto& id = ReceiveHD().Parent().AccountID();
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
        2500000000,
        u8"25 units",
        "",
        {},
        {test_chain_},
        100,
        {height_, height_},
        {},
        {{u8"25", u8"25 units"}},
        {
            {
                ot::StorageBox::BLOCKCHAIN,
                1,
                1500000000,
                u8"15 units",
                {expectedContact->str()},
                "",
                "",
                "Incoming Unit Test Simulation transaction from "
                "PD1jTsa1rjnbMMLVbj5cg2c8KkFY32KWtPRqVVpSBkv1jf8zjHJVu",
                ot::blockchain::HashToNumber(transactions_.at(2)),
                std::nullopt,
                0,
            },
            {
                ot::StorageBox::BLOCKCHAIN,
                1,
                1000000000,
                u8"10 units",
                {expectedContact->str()},
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

    ASSERT_TRUE(wait_for_counter(account_activity_bob_, false));
    EXPECT_TRUE(check_account_activity(bob_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(bob_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(bob_, id, expected));

    const auto& tree =
        client_2_.Crypto().Blockchain().Account(bob_.nym_id_, test_chain_);
    const auto& pc = tree.GetPaymentCode();

    ASSERT_EQ(pc.size(), 1);

    const auto& account = pc.at(0);
    const auto lookahead = account.Lookahead() - 1u;

    EXPECT_EQ(account.Type(), bca::SubaccountType::PaymentCode);
    EXPECT_FALSE(account.IsNotified());

    {
        constexpr auto subchain{Subchain::Incoming};

        const auto gen = account.LastGenerated(subchain);

        ASSERT_TRUE(gen.has_value());
        EXPECT_EQ(gen.value(), lookahead);
    }
    {
        constexpr auto subchain{Subchain::Outgoing};

        const auto gen = account.LastGenerated(subchain);

        ASSERT_TRUE(gen.has_value());
        EXPECT_EQ(gen.value(), lookahead);
    }
    {
        constexpr auto subchain{Subchain::External};

        const auto gen = account.LastGenerated(subchain);

        EXPECT_FALSE(gen.has_value());
    }
    {
        constexpr auto subchain{Subchain::Internal};

        const auto gen = account.LastGenerated(subchain);

        EXPECT_FALSE(gen.has_value());
    }
}

TEST_F(Regtest_payment_code, bob_account_list_second_unconfirmed_incoming)
{
    const auto expected = AccountListData{{
        {ReceiveHD().Parent().AccountID().str(),
         expected_unit_.str(),
         expected_display_unit_,
         expected_account_name_,
         expected_notary_.str(),
         expected_notary_name_,
         expected_account_type_,
         expected_unit_type_,
         1,
         2500000000,
         u8"25 units"},
    }};

    ASSERT_TRUE(wait_for_counter(account_list_bob_, false));
    EXPECT_TRUE(check_account_list(bob_, expected));
    EXPECT_TRUE(check_account_list_qt(bob_, expected));
    EXPECT_TRUE(check_account_list_rpc(bob_, expected));
}

TEST_F(Regtest_payment_code, bob_activity_thread_second_unconfirmed_incoming)
{
    const auto& contact = bob_.Contact(alice_.name_);
    const auto expected = ActivityThreadData{
        false,
        contact.str(),
        alice_.payment_code_,
        "",
        contact.str(),
        {{ot::core::UnitType::Regtest, alice_.payment_code_}},
        {
            {
                false,
                false,
                false,
                1,
                1000000000,
                "10 units",
                alice_.payment_code_,
                "Incoming Unit Test Simulation transaction from "
                "PD1jTsa1rjnbMMLVbj5cg2c8KkFY32KWtPRqVVpSBkv1jf8zjHJVu",
                "",
                ot::StorageBox::BLOCKCHAIN,
                std::nullopt,
            },
            {
                false,
                false,
                false,
                1,
                1500000000,
                "15 units",
                alice_.payment_code_,
                "Incoming Unit Test Simulation transaction from "
                "PD1jTsa1rjnbMMLVbj5cg2c8KkFY32KWtPRqVVpSBkv1jf8zjHJVu",
                "",
                ot::StorageBox::BLOCKCHAIN,
                std::nullopt,
            },
        },
    };

    ASSERT_TRUE(wait_for_counter(activity_thread_bob_alice_, false));
    EXPECT_TRUE(check_activity_thread(bob_, contact, expected));
    EXPECT_TRUE(check_activity_thread_qt(bob_, contact, expected));
}

TEST_F(Regtest_payment_code, bob_txodb_second_spend_unconfirmed)
{
    EXPECT_TRUE(CheckTXODBBob());
}

TEST_F(Regtest_payment_code, update_contacts)
{
    activity_thread_alice_bob_.expected_ += 4;
    activity_thread_bob_alice_.expected_ += 4;
    account_activity_alice_.expected_ += 2;
    account_activity_bob_.expected_ += 2;
    contact_list_alice_.expected_ += 1;
    contact_list_bob_.expected_ += 1;
    client_1_.OTX().StartIntroductionServer(alice_.nym_id_);
    client_2_.OTX().StartIntroductionServer(bob_.nym_id_);
    auto task1 =
        client_1_.OTX().RegisterNymPublic(alice_.nym_id_, server_1_.id_, true);
    auto task2 =
        client_2_.OTX().RegisterNymPublic(bob_.nym_id_, server_1_.id_, true);

    ASSERT_NE(0, task1.first);
    ASSERT_NE(0, task2.first);
    EXPECT_EQ(
        ot::otx::LastReplyStatus::MessageSuccess, task1.second.get().first);
    EXPECT_EQ(
        ot::otx::LastReplyStatus::MessageSuccess, task2.second.get().first);

    client_1_.OTX().ContextIdle(alice_.nym_id_, server_1_.id_).get();
    client_2_.OTX().ContextIdle(bob_.nym_id_, server_1_.id_).get();
}

TEST_F(Regtest_payment_code, alice_contact_list_after_otx)
{
    const auto expected = ContactListData{{
        {true, alice_.name_, alice_.name_, "ME", ""},
        {true, bob_.name_, bob_.name_, "B", ""},
    }};

    ASSERT_TRUE(wait_for_counter(contact_list_alice_, false));
    EXPECT_TRUE(check_contact_list(alice_, expected));
    EXPECT_TRUE(check_contact_list_qt(alice_, expected));
}

TEST_F(Regtest_payment_code, alice_account_activity_after_otx)
{
    const auto expectedContact = client_1_.Contacts().ContactID(bob_.nym_id_);
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
        7499999448,
        u8"74.999\u202F994\u202F48 units",
        "",
        {},
        {test_chain_},
        100,
        {height_, height_},
        {},
        {{u8"74.99999448", u8"74.999\u202F994\u202F48 units"}},
        {
            {
                ot::StorageBox::BLOCKCHAIN,
                -1,
                -1500000236,
                u8"-15.000\u202F002\u202F36 units",
                {expectedContact->str()},
                "",
                "",
                "Outgoing Unit Test Simulation transaction to Bob",
                ot::blockchain::HashToNumber(transactions_.at(2)),
                std::nullopt,
                0,
            },
            {
                ot::StorageBox::BLOCKCHAIN,
                -1,
                -1000000316,
                u8"-10.000\u202F003\u202F16 units",
                {expectedContact->str()},
                "",
                "",
                "Outgoing Unit Test Simulation transaction to Bob",
                ot::blockchain::HashToNumber(transactions_.at(1)),
                std::nullopt,
                1,
            },
            {
                ot::StorageBox::BLOCKCHAIN,
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

    ASSERT_TRUE(wait_for_counter(account_activity_alice_, false));
    EXPECT_TRUE(check_account_activity(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(alice_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(alice_, id, expected));
}

TEST_F(Regtest_payment_code, alice_account_list_after_otx)
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
         7499999448,
         u8"74.999\u202F994\u202F48 units"},
    }};

    ASSERT_TRUE(wait_for_counter(account_list_alice_, false));
    EXPECT_TRUE(check_account_list(alice_, expected));
    EXPECT_TRUE(check_account_list_qt(alice_, expected));
    EXPECT_TRUE(check_account_list_rpc(alice_, expected));
}

TEST_F(Regtest_payment_code, alice_activity_thread_after_otx)
{
    const auto& contact = alice_.Contact(bob_.name_);
    const auto expected = ActivityThreadData{
        true,
        contact.str(),
        bob_.name_,
        "",
        contact.str(),
        {{ot::core::UnitType::Regtest, bob_.payment_code_}},
        {
            {
                false,
                false,
                true,
                -1,
                -1000000316,
                u8"-10.000\u202F003\u202F16 units",
                alice_.name_,
                "Outgoing Unit Test Simulation transaction to Bob",
                "",
                ot::StorageBox::BLOCKCHAIN,
                std::nullopt,
            },
            {
                false,
                false,
                true,
                -1,
                -1500000236,
                u8"-15.000\u202F002\u202F36 units",
                alice_.name_,
                "Outgoing Unit Test Simulation transaction to Bob",
                "",
                ot::StorageBox::BLOCKCHAIN,
                std::nullopt,
            },
        },
    };

    ASSERT_TRUE(wait_for_counter(activity_thread_alice_bob_, false));
    EXPECT_TRUE(check_activity_thread(alice_, contact, expected));
    EXPECT_TRUE(check_activity_thread_qt(alice_, contact, expected));
}

TEST_F(Regtest_payment_code, bob_contact_list_after_otx)
{
    const auto expected = ContactListData{{
        {true, bob_.name_, bob_.name_, "ME", ""},
        {true, alice_.name_, alice_.name_, "A", ""},
    }};

    ASSERT_TRUE(wait_for_counter(contact_list_bob_, false));
    EXPECT_TRUE(check_contact_list(bob_, expected));
    EXPECT_TRUE(check_contact_list_qt(bob_, expected));
}

TEST_F(Regtest_payment_code, bob_account_activity_after_otx)
{
    const auto expectedContact = client_2_.Contacts().ContactID(alice_.nym_id_);
    const auto& id = ReceiveHD().Parent().AccountID();
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
        2500000000,
        u8"25 units",
        "",
        {},
        {test_chain_},
        100,
        {height_, height_},
        {},
        {{u8"25", u8"25 units"}},
        {
            {
                ot::StorageBox::BLOCKCHAIN,
                1,
                1500000000,
                u8"15 units",
                {expectedContact->str()},
                "",
                "",
                "Incoming Unit Test Simulation transaction from Alice",
                ot::blockchain::HashToNumber(transactions_.at(2)),
                std::nullopt,
                0,
            },
            {
                ot::StorageBox::BLOCKCHAIN,
                1,
                1000000000,
                u8"10 units",
                {expectedContact->str()},
                "",
                "",
                "Incoming Unit Test Simulation transaction from Alice",
                ot::blockchain::HashToNumber(transactions_.at(1)),
                std::nullopt,
                1,
            },
        },
    };

    ASSERT_TRUE(wait_for_counter(account_activity_bob_, false));
    EXPECT_TRUE(check_account_activity(bob_, id, expected));
    EXPECT_TRUE(check_account_activity_qt(bob_, id, expected));
    EXPECT_TRUE(check_account_activity_rpc(bob_, id, expected));

    const auto& tree =
        client_2_.Crypto().Blockchain().Account(bob_.nym_id_, test_chain_);
    const auto& pc = tree.GetPaymentCode();

    ASSERT_EQ(pc.size(), 1);

    const auto& account = pc.at(0);
    const auto lookahead = account.Lookahead() - 1u;

    EXPECT_EQ(account.Type(), bca::SubaccountType::PaymentCode);
    EXPECT_FALSE(account.IsNotified());

    {
        constexpr auto subchain{Subchain::Incoming};

        const auto gen = account.LastGenerated(subchain);

        ASSERT_TRUE(gen.has_value());
        EXPECT_EQ(gen.value(), lookahead);
    }
    {
        constexpr auto subchain{Subchain::Outgoing};

        const auto gen = account.LastGenerated(subchain);

        ASSERT_TRUE(gen.has_value());
        EXPECT_EQ(gen.value(), lookahead);
    }
    {
        constexpr auto subchain{Subchain::External};

        const auto gen = account.LastGenerated(subchain);

        EXPECT_FALSE(gen.has_value());
    }
    {
        constexpr auto subchain{Subchain::Internal};

        const auto gen = account.LastGenerated(subchain);

        EXPECT_FALSE(gen.has_value());
    }
}

TEST_F(Regtest_payment_code, bob_account_list_after_otx)
{
    const auto expected = AccountListData{{
        {ReceiveHD().Parent().AccountID().str(),
         expected_unit_.str(),
         expected_display_unit_,
         expected_account_name_,
         expected_notary_.str(),
         expected_notary_name_,
         expected_account_type_,
         expected_unit_type_,
         1,
         2500000000,
         u8"25 units"},
    }};

    ASSERT_TRUE(wait_for_counter(account_list_bob_, false));
    EXPECT_TRUE(check_account_list(bob_, expected));
    EXPECT_TRUE(check_account_list_qt(bob_, expected));
    EXPECT_TRUE(check_account_list_rpc(bob_, expected));
}

TEST_F(Regtest_payment_code, bob_activity_thread_after_otx)
{
    const auto& contact = bob_.Contact(alice_.name_);
    const auto expected = ActivityThreadData{
        true,
        contact.str(),
        alice_.name_,
        "",
        contact.str(),
        {{ot::core::UnitType::Regtest, alice_.payment_code_}},
        {
            {
                false,
                false,
                false,
                1,
                1000000000,
                "10 units",
                alice_.name_,
                "Incoming Unit Test Simulation transaction from Alice",
                "",
                ot::StorageBox::BLOCKCHAIN,
                std::nullopt,
            },
            {
                false,
                false,
                false,
                1,
                1500000000,
                "15 units",
                alice_.name_,
                "Incoming Unit Test Simulation transaction from Alice",
                "",
                ot::StorageBox::BLOCKCHAIN,
                std::nullopt,
            },
        },
    };

    ASSERT_TRUE(wait_for_counter(activity_thread_bob_alice_, false));
    EXPECT_TRUE(check_activity_thread(bob_, contact, expected));
    EXPECT_TRUE(check_activity_thread_qt(bob_, contact, expected));
}

TEST_F(Regtest_payment_code, send_message_to_alice)
{
    activity_thread_alice_bob_.expected_ += 1;
    activity_thread_bob_alice_.expected_ += 2;

    EXPECT_FALSE(activity_thread_send_message(bob_, alice_));
    EXPECT_TRUE(activity_thread_send_message(bob_, alice_, message_text_));
}

TEST_F(Regtest_payment_code, alice_activity_thread_after_message)
{
    const auto& contact = alice_.Contact(bob_.name_);
    const auto expected = ActivityThreadData{
        true,
        contact.str(),
        bob_.name_,
        "",
        contact.str(),
        {{ot::core::UnitType::Regtest, bob_.payment_code_}},
        {
            {
                false,
                false,
                true,
                -1,
                -1000000316,
                u8"-10.000\u202F003\u202F16 units",
                alice_.name_,
                "Outgoing Unit Test Simulation transaction to Bob",
                "",
                ot::StorageBox::BLOCKCHAIN,
                std::nullopt,
            },
            {
                false,
                false,
                true,
                -1,
                -1500000236,
                u8"-15.000\u202F002\u202F36 units",
                alice_.name_,
                "Outgoing Unit Test Simulation transaction to Bob",
                "",
                ot::StorageBox::BLOCKCHAIN,
                std::nullopt,
            },
            {
                false,
                false,
                false,
                0,
                0,
                "",
                bob_.name_,
                message_text_,
                "",
                ot::StorageBox::MAILINBOX,
                std::nullopt,
            },
        },
    };

    ASSERT_TRUE(wait_for_counter(activity_thread_alice_bob_, false));
    EXPECT_TRUE(check_activity_thread(alice_, contact, expected));
    EXPECT_TRUE(check_activity_thread_qt(alice_, contact, expected));
}

TEST_F(Regtest_payment_code, bob_activity_thread_after_message)
{
    const auto& contact = bob_.Contact(alice_.name_);
    const auto expected = ActivityThreadData{
        true,
        contact.str(),
        alice_.name_,
        "",
        contact.str(),
        {{ot::core::UnitType::Regtest, alice_.payment_code_}},
        {
            {
                false,
                false,
                false,
                1,
                1000000000,
                "10 units",
                alice_.name_,
                "Incoming Unit Test Simulation transaction from Alice",
                "",
                ot::StorageBox::BLOCKCHAIN,
                std::nullopt,
            },
            {
                false,
                false,
                false,
                1,
                1500000000,
                "15 units",
                alice_.name_,
                "Incoming Unit Test Simulation transaction from Alice",
                "",
                ot::StorageBox::BLOCKCHAIN,
                std::nullopt,
            },
            {
                false,
                false,
                true,
                0,
                0,
                "",
                bob_.name_,
                message_text_,
                "",
                ot::StorageBox::MAILOUTBOX,
                std::nullopt,
            },
        },
    };

    ASSERT_TRUE(wait_for_counter(activity_thread_bob_alice_, false));
    EXPECT_TRUE(check_activity_thread(bob_, contact, expected));
    EXPECT_TRUE(check_activity_thread_qt(bob_, contact, expected));
}

TEST_F(Regtest_payment_code, shutdown) { Shutdown(); }
}  // namespace ottest
