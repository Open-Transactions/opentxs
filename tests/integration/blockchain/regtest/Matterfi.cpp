// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <algorithm>
#include <atomic>
#include <future>
#include <optional>
#include <span>
#include <string_view>
#include <tuple>
#include <utility>

#include "internal/blockchain/block/Types.hpp"
#include "internal/util/P0330.hpp"
#include "ottest/data/crypto/PaymentCodeV3.hpp"
#include "ottest/fixtures/blockchain/Common.hpp"
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
#include "ottest/fixtures/ui/ContactActivity.hpp"
#include "ottest/fixtures/ui/ContactList.hpp"

namespace ottest
{
using namespace opentxs::literals;
using namespace std::literals;

static constexpr auto contact1 =
    "PD1icC1WChenjc2WEw52oKR127qCzLRFyo7DFdo6yJrZ5DKsrp42v";
static constexpr auto contact2 =
    "PD1jz7V6rqZyfdhb9pxrjx44BHtFfPornR2qb7KH5MmENhDg8HkQp";

Counter contact_list_alex_{};

TEST_F(Regtest_payment_code, init_opentxs) {}

TEST_F(Regtest_payment_code, add_contacts)
{
    client_1_.Contacts().PaymentCodeToContact(
        client_1_.Factory().PaymentCodeFromBase58(contact1), test_chain_);
    client_1_.Contacts().PaymentCodeToContact(
        client_1_.Factory().PaymentCodeFromBase58(contact2), test_chain_);
}

TEST_F(Regtest_payment_code, start_chains) { EXPECT_TRUE(Start()); }

TEST_F(Regtest_payment_code, connect_peers) { EXPECT_TRUE(Connect()); }

TEST_F(Regtest_payment_code, init_ui_models)
{
    contact_list_alex_.expected_ += 3;
    init_contact_list(alex_, contact_list_alex_);
}

TEST_F(Regtest_payment_code, alex_contact_list_initial)
{
    const auto expected = ContactListData{{
        {true, alex_.name_, alex_.name_, "ME", ""},
        {false, "contact1", contact1, "P", ""},
        {false, "contact2", contact2, "P", ""},
    }};
    wait_for_counter(contact_list_alex_, false);

    EXPECT_TRUE(check_contact_list(alex_, expected));
    EXPECT_TRUE(check_contact_list_qt(alex_, expected));
}

TEST_F(Regtest_payment_code, mine_initial_balance)
{
    constexpr auto orphan{0};
    constexpr auto count{1};
    const auto start = height_ - orphan;
    const auto end{start + count};
    auto future = listener_alex_.get_future(SendHD(), Subchain::External, end);

    EXPECT_EQ(start, 0);
    EXPECT_EQ(end, 1);
    EXPECT_TRUE(Mine(start, count, mine_to_alex_));
    EXPECT_TRUE(listener_alex_.wait(future));
    EXPECT_TRUE(txos_alex_.Mature(end));
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

TEST_F(Regtest_payment_code, mature_initial_balance)
{
    constexpr auto orphan{0};
    const auto count = static_cast<int>(MaturationInterval());
    const auto start = height_ - orphan;
    const auto end{start + count};
    auto future = listener_alex_.get_future(SendHD(), Subchain::External, end);

    EXPECT_EQ(start, 1);
    EXPECT_EQ(end, 11);
    EXPECT_TRUE(Mine(start, count));
    EXPECT_TRUE(listener_alex_.wait(future));
    EXPECT_TRUE(txos_alex_.Mature(end));
}

TEST_F(Regtest_payment_code, alex_txodb_inital_receive)
{
    EXPECT_TRUE(CheckTXODBAlex());
}

TEST_F(Regtest_payment_code, send_to_bob)
{
    contact_list_alex_.expected_ += 1;
    const auto handle = client_1_.Network().Blockchain().GetChain(test_chain_);

    ASSERT_TRUE(handle);

    const auto& wallet = handle.get().Wallet();
    const auto bob = client_1_.Factory().PaymentCodeFromBase58(
        GetPaymentCodeVector3().bob_.payment_code_);
    auto spend = wallet.CreateSpend(alex_.nym_id_);

    if (false == spend.SetUseEnhancedNotifications(true)) { ADD_FAILURE(); }

    if (false == spend.SetMemo(memo_outgoing_)) { ADD_FAILURE(); }

    if (false == spend.SendToPaymentCode(bob, 1000000000)) { ADD_FAILURE(); }

    auto future = wallet.Execute(spend);
    const auto& txid = transactions_.emplace_back(future.get().second);

    ASSERT_FALSE(txid.IsNull());

    {
        const auto tx = client_1_.Crypto().Blockchain().LoadTransaction(txid);

        EXPECT_TRUE(tx.IsValid());
        EXPECT_TRUE(
            txos_alex_.SpendUnconfirmed({transactions_.at(0).Bytes(), 0}));
        ASSERT_EQ(tx.asBitcoin().Outputs().size(), 5_uz);
        EXPECT_TRUE(txos_alex_.AddUnconfirmed(tx, 1, SendHD()));
        EXPECT_TRUE(txos_alex_.AddUnconfirmed(tx, 2, SendHD()));
        EXPECT_TRUE(txos_alex_.AddUnconfirmed(tx, 3, SendHD()));
        EXPECT_TRUE(txos_alex_.AddUnconfirmed(tx, 4, SendHD()));
    }

    {
        const auto& element = SendPC().BalanceElement(Subchain::Outgoing, 0);
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
        const auto& element = SendHD().BalanceElement(Subchain::Internal, 3);
        const auto amount = ot::Amount{2249999835};
        expected_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(txid.Bytes(), 1),
            std::forward_as_tuple(
                client_1_.Factory().DataFromBytes(element.Key().PublicKey()),
                amount,
                Pattern::PayToMultisig));
    }
    {
        const auto& element = SendHD().BalanceElement(Subchain::Internal, 2);
        const auto amount = ot::Amount{2249999835};
        expected_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(txid.Bytes(), 2),
            std::forward_as_tuple(
                client_1_.Factory().DataFromBytes(element.Key().PublicKey()),
                amount,
                Pattern::PayToMultisig));
    }
    {
        const auto& element = SendHD().BalanceElement(Subchain::Internal, 1);
        const auto amount = ot::Amount{2249999836};
        expected_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(txid.Bytes(), 3),
            std::forward_as_tuple(
                client_1_.Factory().DataFromBytes(element.Key().PublicKey()),
                amount,
                Pattern::PayToMultisig));
    }
    {
        const auto& element = SendHD().BalanceElement(Subchain::Internal, 0);
        const auto amount = ot::Amount{2249999836};
        expected_.emplace(
            std::piecewise_construct,
            std::forward_as_tuple(txid.Bytes(), 4),
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
        const auto outputs = tx.Outputs();

        EXPECT_EQ(outputs.size(), 5u);

        const auto& payment = outputs[0];

        EXPECT_EQ(payment.Payer(), self);
        EXPECT_EQ(payment.Payee(), other);

        const auto check_change = [&](const auto& change) {
            EXPECT_EQ(change.Payer(), self);
            EXPECT_EQ(change.Payee(), self);

            const auto tags = chain.Wallet().GetTags({tx.ID().Bytes(), 1});
            using Tag = ot::blockchain::node::TxoTag;

            EXPECT_EQ(tags.size(), 3);
            EXPECT_EQ(tags.count(Tag::Normal), 1);
            EXPECT_EQ(tags.count(Tag::Notification), 1);
            EXPECT_EQ(tags.count(Tag::Change), 1);
        };
        std::for_each(
            std::next(outputs.begin(), 1), outputs.end(), check_change);
    }
}

TEST_F(Regtest_payment_code, check_notification_transactions_sender)
{
    const auto& account =
        client_1_.Crypto().Blockchain().Account(alex_.nym_id_, test_chain_);
    const auto& pc = account.GetPaymentCode();

    EXPECT_EQ(pc.size(), 3_uz);

    const auto check_notifications = [&](const auto& subaccount) {
        const auto [incoming, outgoing] = subaccount.NotificationCount();

        EXPECT_EQ(subaccount.IncomingNotificationCount(), 0_uz);
        EXPECT_EQ(subaccount.OutgoingNotificationCount(), 1_uz);
        EXPECT_EQ(incoming, 0_uz);
        EXPECT_EQ(outgoing, 1_uz);
    };
    std::for_each(pc.begin(), pc.end(), check_notifications);
}

TEST_F(Regtest_payment_code, alex_contact_list_first_spend_unconfirmed)
{
    const auto expected = ContactListData{{
        {true, alex_.name_, alex_.name_, "ME", ""},
        {true, "contact1", contact1, "P", ""},
        {false, bob_.name_, bob_.payment_code_, "P", ""},
        {true, "contact2", contact2, "P", ""},
    }};
    wait_for_counter(contact_list_alex_, false);

    EXPECT_TRUE(check_contact_list(alex_, expected));
    EXPECT_TRUE(check_contact_list_qt(alex_, expected));
    EXPECT_TRUE(CheckContactID(
        alex_, bob_, GetPaymentCodeVector3().bob_.payment_code_));
}

TEST_F(Regtest_payment_code, alex_txodb_first_spend_unconfirmed)
{
    EXPECT_TRUE(CheckTXODBAlex());
}

TEST_F(Regtest_payment_code, shutdown) { Shutdown(); }
}  // namespace ottest
