// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#if OT_QT
#include <QDateTime>
#include <QList>
#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#endif  // OT_QT
#include <gtest/gtest.h>
#include <atomic>
#include <memory>
#include <optional>
#include <string>
#include <vector>

#include "Helpers.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/UI.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/contact/Contact.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/core/UnitType.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/ui/AccountActivity.hpp"
#include "opentxs/ui/AccountList.hpp"
#include "opentxs/ui/AccountListItem.hpp"
#include "opentxs/ui/ActivitySummary.hpp"
#include "opentxs/ui/ActivitySummaryItem.hpp"
#include "opentxs/ui/ActivityThread.hpp"
#include "opentxs/ui/ActivityThreadItem.hpp"
#include "opentxs/ui/BalanceItem.hpp"
#include "opentxs/ui/BlockchainSelection.hpp"
#include "opentxs/ui/BlockchainSelectionItem.hpp"
#include "opentxs/ui/Blockchains.hpp"
#if OT_QT
#include "opentxs/ui/qt/AccountActivity.hpp"
#include "opentxs/ui/qt/AccountList.hpp"
#if OT_BLOCKCHAIN
#include "opentxs/ui/qt/BlockchainSelection.hpp"
#endif  // OT_BLOCKCHAIN
#endif  // OT_QT
#include "opentxs/util/SharedPimpl.hpp"
#include "opentxs/util/Time.hpp"
#include "ui/Helpers.hpp"

using Subchain = ot::blockchain::crypto::Subchain;

namespace ottest
{
const auto time_1_ = ot::Clock::from_time_t(1592661306);
const auto time_2_ = ot::Clock::from_time_t(1592663862);
const auto time_3_ = ot::Clock::from_time_t(1592664462);
const auto time_4_ = ot::Clock::from_time_t(1592675062);
std::string txid_1_{};
std::string txid_2_{};
std::string txid_3_{};
std::string txid_4_{};
ot::Bip32Index first_index_{};
ot::Bip32Index second_index_{};
ot::Bip32Index third_index_{};
ot::Bip32Index fourth_index_{};
ot::Bip32Index fifth_index_{};
ot::Bip32Index sixth_index_{};
Counter account_list_{};
Counter account_activity_{};
Counter account_summary_{};
Counter activity_summary_{};
Counter activity_thread_1_{};
Counter activity_thread_2_{};
Counter activity_thread_3_{};

TEST_F(Test_BlockchainActivity, init)
{
    EXPECT_FALSE(nym_1_id().empty());
    EXPECT_FALSE(contact_1_id().empty());
    EXPECT_FALSE(contact_2_id().empty());
    EXPECT_FALSE(contact_5_id().empty());
    EXPECT_FALSE(contact_6_id().empty());
    EXPECT_FALSE(contact_7_id().empty());

    {
        const auto contact = api_.Contacts().Contact(contact_1_id());

        ASSERT_TRUE(contact);
        EXPECT_EQ(contact->Label(), nym_1_name_);
        EXPECT_EQ(api_.Contacts().ContactName(contact_1_id()), nym_1_name_);
    }

    {
        const auto contact = api_.Contacts().Contact(contact_2_id());

        ASSERT_TRUE(contact);
        EXPECT_EQ(contact->Label(), nym_2_name_);
        EXPECT_EQ(api_.Contacts().ContactName(contact_2_id()), nym_2_name_);
    }

    {
        const auto contact = api_.Contacts().Contact(contact_5_id());

        ASSERT_TRUE(contact);
        EXPECT_EQ(contact->Label(), contact_5_name_);
        EXPECT_EQ(api_.Contacts().ContactName(contact_5_id()), contact_5_name_);
    }

    {
        const auto contact = api_.Contacts().Contact(contact_6_id());

        ASSERT_TRUE(contact);
        EXPECT_EQ(contact->Label(), contact_6_name_);
        EXPECT_EQ(api_.Contacts().ContactName(contact_6_id()), contact_6_name_);
    }

    {
        const auto contact = api_.Contacts().Contact(contact_7_id());

        ASSERT_TRUE(contact);
        EXPECT_EQ(contact->Label(), contact_7_name_);
        EXPECT_EQ(api_.Contacts().ContactName(contact_7_id()), contact_7_name_);
    }
}

TEST_F(Test_BlockchainActivity, blank_account_list)
{
    const auto& widget = api_.UI().AccountList(
        nym_1_id(), make_cb(account_list_, u8"account_list_"));
    auto row = widget.First();

    ASSERT_FALSE(row->Valid());
}

#if OT_QT
TEST_F(Test_BlockchainActivity, blank_account_list_qt)
{
    const auto* pWidget = api_.UI().AccountListQt(nym_1_id());

    ASSERT_NE(pWidget, nullptr);

    const auto& widget = *pWidget;

    EXPECT_EQ(widget.columnCount(), 4);
    EXPECT_EQ(widget.rowCount(), 0);

    {
        EXPECT_FALSE(widget.hasIndex(0, 0));
        EXPECT_FALSE(widget.hasIndex(0, 1));
        EXPECT_FALSE(widget.hasIndex(0, 2));
        EXPECT_FALSE(widget.hasIndex(0, 3));
    }
}
#endif  // OT_QT

TEST_F(Test_BlockchainActivity, blockchain_selection)
{
    auto counter = Counter{};
    counter.expected_ = 7;
    const auto& widget = api_.UI().BlockchainSelection(
        ot::ui::Blockchains::All, make_cb(counter, "Blockchain selector"));

    ASSERT_TRUE(wait_for_counter(counter));

    auto row = widget.First();
    auto expected{ot::blockchain::Type::Bitcoin};

    ASSERT_TRUE(row->Valid());

    EXPECT_EQ(row->Name(), ot::blockchain::DisplayString(expected));
    EXPECT_EQ(row->Type(), expected);
    EXPECT_FALSE(row->IsEnabled());
    EXPECT_FALSE(row->IsTestnet());

    ASSERT_FALSE(row->Last());

    row = widget.Next();
    expected = ot::blockchain::Type::BitcoinCash;

    EXPECT_EQ(row->Name(), ot::blockchain::DisplayString(expected));
    EXPECT_EQ(row->Type(), expected);
    EXPECT_FALSE(row->IsEnabled());
    EXPECT_FALSE(row->IsTestnet());

    ASSERT_FALSE(row->Last());

    row = widget.Next();
    expected = ot::blockchain::Type::Litecoin;

    EXPECT_EQ(row->Name(), ot::blockchain::DisplayString(expected));
    EXPECT_EQ(row->Type(), expected);
    EXPECT_FALSE(row->IsEnabled());
    EXPECT_FALSE(row->IsTestnet());

    ASSERT_FALSE(row->Last());

    row = widget.Next();
    expected = ot::blockchain::Type::PKT;

    EXPECT_EQ(row->Name(), ot::blockchain::DisplayString(expected));
    EXPECT_EQ(row->Type(), expected);
    EXPECT_FALSE(row->IsEnabled());
    EXPECT_FALSE(row->IsTestnet());

    ASSERT_FALSE(row->Last());

    row = widget.Next();
    expected = ot::blockchain::Type::Bitcoin_testnet3;

    EXPECT_EQ(row->Name(), ot::blockchain::DisplayString(expected));
    EXPECT_EQ(row->Type(), expected);
    EXPECT_FALSE(row->IsEnabled());
    EXPECT_TRUE(row->IsTestnet());

    ASSERT_FALSE(row->Last());

    row = widget.Next();
    expected = ot::blockchain::Type::BitcoinCash_testnet3;

    EXPECT_EQ(row->Name(), ot::blockchain::DisplayString(expected));
    EXPECT_EQ(row->Type(), expected);
    EXPECT_FALSE(row->IsEnabled());
    EXPECT_TRUE(row->IsTestnet());

    ASSERT_FALSE(row->Last());

    row = widget.Next();
    expected = ot::blockchain::Type::Litecoin_testnet4;

    EXPECT_EQ(row->Name(), ot::blockchain::DisplayString(expected));
    EXPECT_EQ(row->Type(), expected);
    EXPECT_FALSE(row->IsEnabled());
    EXPECT_TRUE(row->IsTestnet());

    EXPECT_TRUE(row->Last());
}

#if OT_QT
TEST_F(Test_BlockchainActivity, blockchain_selection_qt)
{
    const auto* pWidget =
        api_.UI().BlockchainSelectionQt(ot::ui::Blockchains::All);

    ASSERT_NE(pWidget, nullptr);

    const auto& widget = *pWidget;

    EXPECT_EQ(widget.columnCount(), 1);
    EXPECT_EQ(widget.rowCount(), 7);

    using Model = ot::ui::BlockchainSelectionQt;

    {
        const auto row{0};
        const auto expected{ot::blockchain::Type::Bitcoin};

        ASSERT_TRUE(widget.hasIndex(row, 0));

        const auto type = widget.data(widget.index(row, 0), Model::TypeRole);
        const auto name = widget.data(widget.index(row, 0), Qt::DisplayRole);
        const auto enabled =
            widget.data(widget.index(row, 0), Qt::CheckStateRole);
        const auto testnet =
            widget.data(widget.index(row, 0), Model::IsTestnet);

        EXPECT_EQ(type.toInt(), static_cast<int>(expected));
        EXPECT_EQ(
            name.toString().toStdString(),
            ot::blockchain::DisplayString(expected));
        EXPECT_FALSE(enabled.toBool());
        EXPECT_FALSE(testnet.toBool());
    }
    {
        const auto row{1};
        const auto expected{ot::blockchain::Type::BitcoinCash};

        ASSERT_TRUE(widget.hasIndex(row, 0));

        const auto type = widget.data(widget.index(row, 0), Model::TypeRole);
        const auto name = widget.data(widget.index(row, 0), Qt::DisplayRole);
        const auto enabled =
            widget.data(widget.index(row, 0), Qt::CheckStateRole);
        const auto testnet =
            widget.data(widget.index(row, 0), Model::IsTestnet);

        EXPECT_EQ(type.toInt(), static_cast<int>(expected));
        EXPECT_EQ(
            name.toString().toStdString(),
            ot::blockchain::DisplayString(expected));
        EXPECT_FALSE(enabled.toBool());
        EXPECT_FALSE(testnet.toBool());
    }
    {
        const auto row{2};
        const auto expected{ot::blockchain::Type::Litecoin};

        ASSERT_TRUE(widget.hasIndex(row, 0));

        const auto type = widget.data(widget.index(row, 0), Model::TypeRole);
        const auto name = widget.data(widget.index(row, 0), Qt::DisplayRole);
        const auto enabled =
            widget.data(widget.index(row, 0), Qt::CheckStateRole);
        const auto testnet =
            widget.data(widget.index(row, 0), Model::IsTestnet);

        EXPECT_EQ(type.toInt(), static_cast<int>(expected));
        EXPECT_EQ(
            name.toString().toStdString(),
            ot::blockchain::DisplayString(expected));
        EXPECT_FALSE(enabled.toBool());
        EXPECT_FALSE(testnet.toBool());
    }
    {
        const auto row{3};
        const auto expected{ot::blockchain::Type::PKT};

        ASSERT_TRUE(widget.hasIndex(row, 0));

        const auto type = widget.data(widget.index(row, 0), Model::TypeRole);
        const auto name = widget.data(widget.index(row, 0), Qt::DisplayRole);
        const auto enabled =
            widget.data(widget.index(row, 0), Qt::CheckStateRole);
        const auto testnet =
            widget.data(widget.index(row, 0), Model::IsTestnet);

        EXPECT_EQ(type.toInt(), static_cast<int>(expected));
        EXPECT_EQ(
            name.toString().toStdString(),
            ot::blockchain::DisplayString(expected));
        EXPECT_FALSE(enabled.toBool());
        EXPECT_FALSE(testnet.toBool());
    }
    {
        const auto row{4};
        const auto expected{ot::blockchain::Type::Bitcoin_testnet3};

        ASSERT_TRUE(widget.hasIndex(row, 0));

        const auto type = widget.data(widget.index(row, 0), Model::TypeRole);
        const auto name = widget.data(widget.index(row, 0), Qt::DisplayRole);
        const auto enabled =
            widget.data(widget.index(row, 0), Qt::CheckStateRole);
        const auto testnet =
            widget.data(widget.index(row, 0), Model::IsTestnet);

        EXPECT_EQ(type.toInt(), static_cast<int>(expected));
        EXPECT_EQ(
            name.toString().toStdString(),
            ot::blockchain::DisplayString(expected));
        EXPECT_FALSE(enabled.toBool());
        EXPECT_TRUE(testnet.toBool());
    }
    {
        const auto row{5};
        const auto expected{ot::blockchain::Type::BitcoinCash_testnet3};

        ASSERT_TRUE(widget.hasIndex(row, 0));

        const auto type = widget.data(widget.index(row, 0), Model::TypeRole);
        const auto name = widget.data(widget.index(row, 0), Qt::DisplayRole);
        const auto enabled =
            widget.data(widget.index(row, 0), Qt::CheckStateRole);
        const auto testnet =
            widget.data(widget.index(row, 0), Model::IsTestnet);

        EXPECT_EQ(type.toInt(), static_cast<int>(expected));
        EXPECT_EQ(
            name.toString().toStdString(),
            ot::blockchain::DisplayString(expected));
        EXPECT_FALSE(enabled.toBool());
        EXPECT_TRUE(testnet.toBool());
    }
    {
        const auto row{6};
        const auto expected{ot::blockchain::Type::Litecoin_testnet4};

        ASSERT_TRUE(widget.hasIndex(row, 0));

        const auto type = widget.data(widget.index(row, 0), Model::TypeRole);
        const auto name = widget.data(widget.index(row, 0), Qt::DisplayRole);
        const auto enabled =
            widget.data(widget.index(row, 0), Qt::CheckStateRole);
        const auto testnet =
            widget.data(widget.index(row, 0), Model::IsTestnet);

        EXPECT_EQ(type.toInt(), static_cast<int>(expected));
        EXPECT_EQ(
            name.toString().toStdString(),
            ot::blockchain::DisplayString(expected));
        EXPECT_FALSE(enabled.toBool());
        EXPECT_TRUE(testnet.toBool());
    }
}
#endif  // OT_QT

TEST_F(Test_BlockchainActivity, setup_blockchain_account)
{
    account_list_.expected_ += 1;

    EXPECT_FALSE(account_1_id().empty());

    const auto& account =
        api_.Crypto().Blockchain().HDSubaccount(nym_1_id(), account_1_id());
    const auto indexOne =
        account.Reserve(Subchain::External, reason_, contact_5_id());
    const auto indexTwo = account.Reserve(Subchain::External, reason_);
    const auto indexThree = account.Reserve(
        Subchain::External, reason_, contact_6_id(), u8"Free Ross");
    const auto indexFour = account.Reserve(Subchain::External, reason_);
    const auto indexFive = account.Reserve(Subchain::External, reason_);
    const auto indexSix = account.Reserve(Subchain::External, reason_);

    ASSERT_TRUE(indexOne.has_value());
    ASSERT_TRUE(indexTwo.has_value());
    ASSERT_TRUE(indexThree.has_value());
    ASSERT_TRUE(indexFour.has_value());
    ASSERT_TRUE(indexFive.has_value());
    ASSERT_TRUE(indexSix.has_value());

    first_index_ = indexOne.value();
    second_index_ = indexTwo.value();
    third_index_ = indexThree.value();
    fourth_index_ = indexFour.value();
    fifth_index_ = indexFour.value();
    sixth_index_ = indexFour.value();

    ASSERT_TRUE(wait_for_counter(account_list_));
}

TEST_F(Test_BlockchainActivity, setup_ui)
{
    account_activity_.expected_ += 0;
    account_summary_.expected_ += 0;
    activity_summary_.expected_ += 0;
    activity_thread_1_.expected_ += 2;
    activity_thread_2_.expected_ += 2;
    activity_thread_3_.expected_ += 2;
    api_.UI().AccountActivity(
        nym_1_id(),
        api_.Factory().IdentifierFromBase58(btc_account_id_),
        make_cb(account_activity_, u8"account_activity_"));
    api_.UI().AccountSummary(
        nym_1_id(),
        ot::core::UnitType::BTC,
        make_cb(account_summary_, u8"account_summary"));
    api_.UI().ActivitySummary(
        nym_1_id(), make_cb(activity_summary_, u8"activity_summary"));
    api_.UI().ActivityThread(
        nym_1_id(),
        contact_5_id(),
        make_cb(activity_thread_1_, u8"activity_thread_1"));
    api_.UI().ActivityThread(
        nym_1_id(),
        contact_6_id(),
        make_cb(activity_thread_2_, u8"activity_thread_2"));
    api_.UI().ActivityThread(
        nym_1_id(),
        contact_7_id(),
        make_cb(activity_thread_3_, u8"activity_thread_3"));
}

TEST_F(Test_BlockchainActivity, initial_state_account_list)
{
    ASSERT_TRUE(wait_for_counter(account_list_));

    const auto& widget = api_.UI().AccountList(nym_1_id());
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->AccountID(), btc_account_id_);
    EXPECT_EQ(row->Balance(), 0);
    EXPECT_EQ(row->ContractID(), btc_unit_id_);
    EXPECT_EQ(row->DisplayBalance(), u8"0 ₿");
    EXPECT_EQ(row->DisplayUnit(), u8"BTC");
    EXPECT_EQ(row->Name(), u8"This device");
    EXPECT_EQ(row->NotaryID(), btc_notary_id_);
    EXPECT_EQ(row->NotaryName(), u8"Bitcoin");
    EXPECT_EQ(row->Type(), ot::AccountType::Blockchain);
    EXPECT_EQ(row->Unit(), ot::core::UnitType::BTC);
    EXPECT_TRUE(row->Last());
}

#if OT_QT
TEST_F(Test_BlockchainActivity, initial_state_account_list_qt)
{
    ASSERT_TRUE(wait_for_counter(account_list_));

    const auto* pWidget = api_.UI().AccountListQt(nym_1_id());

    ASSERT_NE(pWidget, nullptr);

    const auto& widget = *pWidget;

    EXPECT_EQ(widget.columnCount(), 4);
    EXPECT_EQ(widget.rowCount(), 1);

    using Model = ot::ui::AccountListQt;

    {
        ASSERT_TRUE(widget.hasIndex(0, 0));
        ASSERT_TRUE(widget.hasIndex(0, 1));
        ASSERT_TRUE(widget.hasIndex(0, 2));
        ASSERT_TRUE(widget.hasIndex(0, 3));

        const auto notaryID =
            widget.data(widget.index(0, 0), Model::NotaryIDRole);
        const auto unit = widget.data(widget.index(0, 0), Model::UnitRole);
        const auto accountID =
            widget.data(widget.index(0, 0), Model::AccountIDRole);
        const auto balance =
            widget.data(widget.index(0, 0), Model::BalanceRole);
        const auto polarity =
            widget.data(widget.index(0, 0), Model::PolarityRole);
        const auto type =
            widget.data(widget.index(0, 0), Model::AccountTypeRole);
        const auto contractID =
            widget.data(widget.index(0, 0), Model::ContractIdRole);
        const auto notaryName = widget.data(widget.index(0, 0));
        const auto unitName = widget.data(widget.index(0, 1));
        const auto accountName = widget.data(widget.index(0, 2));
        const auto displayBalance = widget.data(widget.index(0, 3));

        EXPECT_EQ(notaryID.toString().toStdString(), btc_notary_id_);
        EXPECT_EQ(unit.toInt(), static_cast<int>(ot::core::UnitType::BTC));
        EXPECT_EQ(accountID.toString().toStdString(), btc_account_id_);
        EXPECT_EQ(balance.toInt(), 0);
        EXPECT_EQ(polarity.toInt(), 0);
        EXPECT_EQ(type.toInt(), static_cast<int>(ot::AccountType::Blockchain));
        EXPECT_EQ(contractID.toString().toStdString(), btc_unit_id_);
        EXPECT_EQ(notaryName.toString(), u8"Bitcoin");
        EXPECT_EQ(unitName.toString(), u8"BTC");
        EXPECT_EQ(accountName.toString(), u8"This device");
        EXPECT_EQ(displayBalance.toString(), u8"0 ₿");
    }
}
#endif  // OT_QT

TEST_F(Test_BlockchainActivity, initial_state_account_activity)
{
    ASSERT_TRUE(wait_for_counter(account_activity_));

    const auto& widget = api_.UI().AccountActivity(
        nym_1_id(), api_.Factory().IdentifierFromBase58(btc_account_id_));

    EXPECT_EQ(widget.AccountID(), btc_account_id_);
    EXPECT_EQ(widget.Balance(), 0);
    EXPECT_EQ(widget.BalancePolarity(), 0);
    EXPECT_EQ(widget.ContractID(), btc_unit_id_);

    const auto chains = widget.DepositChains();

    ASSERT_EQ(chains.size(), 1);
    EXPECT_EQ(chains.at(0), ot::blockchain::Type::Bitcoin);
    EXPECT_EQ(widget.DisplayBalance(), u8"0 ₿");
    EXPECT_EQ(widget.DisplayUnit(), u8"BTC");
    EXPECT_EQ(widget.Name(), u8"This device");
    EXPECT_EQ(widget.NotaryID(), btc_notary_id_);
    EXPECT_EQ(widget.NotaryName(), u8"Bitcoin");
    EXPECT_EQ(widget.Type(), ot::AccountType::Blockchain);
    EXPECT_EQ(widget.Unit(), ot::core::UnitType::BTC);

    auto row = widget.First();

    EXPECT_FALSE(row->Valid());
}

#if OT_QT
TEST_F(Test_BlockchainActivity, initial_state_account_activity_qt)
{
    ASSERT_TRUE(wait_for_counter(account_activity_));

    const auto* pWidget = api_.UI().AccountActivityQt(
        nym_1_id(), api_.Factory().Identifier(std::string{btc_account_id_}));

    ASSERT_NE(pWidget, nullptr);

    const auto& widget = *pWidget;

    EXPECT_EQ(widget.columnCount(), 5);
    EXPECT_EQ(widget.rowCount(), 0);
    EXPECT_EQ(widget.accountID().toStdString(), btc_account_id_);
    EXPECT_EQ(widget.balancePolarity(), 0);
    EXPECT_EQ(widget.displayBalance(), u8"0 ₿");

    {
        const auto chains = widget.depositChains();

        ASSERT_EQ(chains.size(), 1);
        EXPECT_EQ(
            *chains.begin(), static_cast<int>(ot::blockchain::Type::Bitcoin));
    }
}
#endif  // OT_QT

TEST_F(Test_BlockchainActivity, initial_state_account_summary)
{
    ASSERT_TRUE(wait_for_counter(account_summary_));
    // FIXME
}

TEST_F(Test_BlockchainActivity, initial_state_activity_summary)
{
    ASSERT_TRUE(wait_for_counter(activity_summary_));

    const auto& widget = api_.UI().ActivitySummary(nym_1_id());
    auto row = widget.First();

    EXPECT_FALSE(row->Valid());
}

TEST_F(Test_BlockchainActivity, initial_state_activity_thread_1)
{
    ASSERT_TRUE(wait_for_counter(activity_thread_1_));

    const auto& widget = api_.UI().ActivityThread(nym_1_id(), contact_5_id());
    auto row = widget.First();

    EXPECT_FALSE(row->Valid());
}

TEST_F(Test_BlockchainActivity, initial_state_activity_thread_2)
{
    ASSERT_TRUE(wait_for_counter(activity_thread_2_));

    const auto& widget = api_.UI().ActivityThread(nym_1_id(), contact_6_id());
    auto row = widget.First();

    EXPECT_FALSE(row->Valid());
}

TEST_F(Test_BlockchainActivity, initial_state_activity_thread_3)
{
    ASSERT_TRUE(wait_for_counter(activity_thread_3_));

    const auto& widget = api_.UI().ActivityThread(nym_1_id(), contact_7_id());
    auto row = widget.First();

    EXPECT_FALSE(row->Valid());
}

TEST_F(Test_BlockchainActivity, receive_assigned)
{
    account_list_.expected_ += 0;
    account_activity_.expected_ += 2;
    account_summary_.expected_ += 0;
    activity_summary_.expected_ += 4;
    activity_thread_1_.expected_ += 1;
    activity_thread_2_.expected_ += 1;
    activity_thread_3_.expected_ += 0;
    const auto& account =
        api_.Crypto().Blockchain().HDSubaccount(nym_1_id(), account_1_id());
    const auto& keyOne =
        account.BalanceElement(Subchain::External, first_index_);
    const auto& keyTwo =
        account.BalanceElement(Subchain::External, second_index_);
    const auto& keyThree =
        account.BalanceElement(Subchain::External, third_index_);
    const auto& keyFour =
        account.BalanceElement(Subchain::External, fourth_index_);

    const auto tx1 = get_test_transaction(keyOne, keyTwo, time_1_);
    const auto tx2 = get_test_transaction(keyThree, keyFour, time_2_);

    ASSERT_TRUE(tx1);
    ASSERT_TRUE(tx2);

    txid_1_ = tx1->ID().asHex();
    txid_2_ = tx2->ID().asHex();

    ASSERT_TRUE(api_.Crypto().Blockchain().Internal().ProcessTransaction(
        ot::blockchain::Type::Bitcoin, *tx1, reason_));
    ASSERT_TRUE(api_.Crypto().Blockchain().Internal().ProcessTransaction(
        ot::blockchain::Type::Bitcoin, *tx2, reason_));
}

TEST_F(Test_BlockchainActivity, receive_assigned_account_list)
{
    ASSERT_TRUE(wait_for_counter(account_list_));

    const auto& widget = api_.UI().AccountList(nym_1_id());
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->AccountID(), btc_account_id_);
    EXPECT_EQ(row->Balance(), 0);  // FIXME
    EXPECT_EQ(row->ContractID(), btc_unit_id_);
    EXPECT_EQ(row->DisplayBalance(), u8"0 ₿");  // FIXME
    EXPECT_EQ(row->DisplayUnit(), u8"BTC");
    EXPECT_EQ(row->Name(), u8"This device");
    EXPECT_EQ(row->NotaryID(), btc_notary_id_);
    EXPECT_EQ(row->NotaryName(), u8"Bitcoin");
    EXPECT_EQ(row->Type(), ot::AccountType::Blockchain);
    EXPECT_EQ(row->Unit(), ot::core::UnitType::BTC);
    EXPECT_TRUE(row->Last());
}

TEST_F(Test_BlockchainActivity, receive_assigned_account_activity)
{
    ASSERT_TRUE(wait_for_counter(account_activity_));

    const auto& widget = api_.UI().AccountActivity(
        nym_1_id(), api_.Factory().IdentifierFromBase58(btc_account_id_));

    EXPECT_EQ(widget.AccountID(), btc_account_id_);
    EXPECT_EQ(widget.Balance(), 0);          // FIXME
    EXPECT_EQ(widget.BalancePolarity(), 0);  // FIXME
    EXPECT_EQ(widget.ContractID(), btc_unit_id_);

    const auto chains = widget.DepositChains();

    ASSERT_EQ(chains.size(), 1);
    EXPECT_EQ(chains.at(0), ot::blockchain::Type::Bitcoin);
    EXPECT_EQ(widget.DisplayBalance(), u8"0 ₿");  // FIXME
    EXPECT_EQ(widget.DisplayUnit(), u8"BTC");
    EXPECT_EQ(widget.Name(), u8"This device");
    EXPECT_EQ(widget.NotaryID(), btc_notary_id_);
    EXPECT_EQ(widget.NotaryName(), u8"Bitcoin");
    EXPECT_EQ(widget.Type(), ot::AccountType::Blockchain);
    EXPECT_EQ(widget.Unit(), ot::core::UnitType::BTC);

    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->Amount(), 1380959);

    auto contacts = row->Contacts();

    ASSERT_EQ(contacts.size(), 1);
    EXPECT_EQ(contacts.at(0), contact_6_id().str());
    EXPECT_EQ(row->DisplayAmount(), u8"0.013\u202F809\u202F59 ₿");
    EXPECT_EQ(row->Memo(), u8"Free Ross");
    EXPECT_EQ(row->Workflow(), u8"");
    EXPECT_EQ(row->Text(), u8"Incoming Bitcoin transaction: Free Ross");
    EXPECT_EQ(row->Timestamp(), time_2_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_FALSE(row->UUID().empty());
    ASSERT_FALSE(row->Last());

    row = widget.Next();

    EXPECT_EQ(row->Amount(), 1380959);

    contacts = row->Contacts();

    ASSERT_EQ(contacts.size(), 1);
    EXPECT_EQ(contacts.at(0), contact_5_id().str());
    EXPECT_EQ(row->DisplayAmount(), u8"0.013\u202F809\u202F59 ₿");
    EXPECT_EQ(row->Memo(), u8"");
    EXPECT_EQ(row->Workflow(), u8"");
    EXPECT_EQ(row->Text(), u8"Incoming Bitcoin transaction");
    EXPECT_EQ(row->Timestamp(), time_1_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_FALSE(row->UUID().empty());
    EXPECT_TRUE(row->Last());
}

#if OT_QT
TEST_F(Test_BlockchainActivity, receive_assigned_account_activity_qt)
{
    ASSERT_TRUE(wait_for_counter(account_activity_));

    const auto* pWidget = api_.UI().AccountActivityQt(
        nym_1_id(), api_.Factory().Identifier(std::string{btc_account_id_}));

    ASSERT_NE(pWidget, nullptr);

    const auto& widget = *pWidget;

    EXPECT_EQ(widget.columnCount(), 5);
    EXPECT_EQ(widget.rowCount(), 2);
    EXPECT_EQ(widget.accountID().toStdString(), btc_account_id_);
    EXPECT_EQ(widget.balancePolarity(), 0);         // FIXME
    EXPECT_EQ(widget.displayBalance(), u8"0 ₿");  // FIXME

    {
        const auto chains = widget.depositChains();

        ASSERT_EQ(chains.size(), 1);
        EXPECT_EQ(
            *chains.begin(), static_cast<int>(ot::blockchain::Type::Bitcoin));
    }

    using Model = ot::ui::AccountActivityQt;

    {
        constexpr auto row{0};

        ASSERT_TRUE(widget.hasIndex(row, 0));
        ASSERT_TRUE(widget.hasIndex(row, 1));
        ASSERT_TRUE(widget.hasIndex(row, 2));
        ASSERT_TRUE(widget.hasIndex(row, 3));
        ASSERT_TRUE(widget.hasIndex(row, 4));

        const auto contacts =
            widget.data(widget.index(row, 0), Model::ContactsRole);
        const auto polarity =
            widget.data(widget.index(row, 0), Model::PolarityRole);
        const auto workflow =
            widget.data(widget.index(row, 0), Model::WorkflowRole);
        const auto type = widget.data(widget.index(row, 0), Model::TypeRole);
        const auto amount = widget.data(widget.index(row, 0));
        const auto text = widget.data(widget.index(row, 1));
        const auto memo = widget.data(widget.index(row, 2));
        const auto time = widget.data(widget.index(row, 3));
        const auto uuid = widget.data(widget.index(row, 4));

        {
            const auto ids = contacts.toStringList();

            ASSERT_EQ(ids.size(), 1);
            EXPECT_EQ(ids.at(0).toStdString(), contact_6_id().str());
        }

        EXPECT_EQ(polarity.toInt(), 1);
        EXPECT_EQ(workflow.toString(), u8"");
        EXPECT_EQ(type.toInt(), static_cast<int>(ot::StorageBox::BLOCKCHAIN));
        EXPECT_EQ(amount.toString(), u8"0.013\u202F809\u202F59 ₿");
        EXPECT_EQ(text.toString(), u8"Incoming Bitcoin transaction: Free Ross");
        EXPECT_EQ(memo.toString(), u8"Free Ross");

        {
            auto expected = QDateTime{};
            expected.setSecsSinceEpoch(ot::Clock::to_time_t(time_2_));

            EXPECT_EQ(time.toDateTime(), expected);
        }

        EXPECT_FALSE(uuid.toString().isEmpty());
    }
    {
        constexpr auto row{1};

        ASSERT_TRUE(widget.hasIndex(row, 0));
        ASSERT_TRUE(widget.hasIndex(row, 1));
        ASSERT_TRUE(widget.hasIndex(row, 2));
        ASSERT_TRUE(widget.hasIndex(row, 3));
        ASSERT_TRUE(widget.hasIndex(row, 4));

        const auto contacts =
            widget.data(widget.index(row, 0), Model::ContactsRole);
        const auto polarity =
            widget.data(widget.index(row, 0), Model::PolarityRole);
        const auto workflow =
            widget.data(widget.index(row, 0), Model::WorkflowRole);
        const auto type = widget.data(widget.index(row, 0), Model::TypeRole);
        const auto amount = widget.data(widget.index(row, 0));
        const auto text = widget.data(widget.index(row, 1));
        const auto memo = widget.data(widget.index(row, 2));
        const auto time = widget.data(widget.index(row, 3));
        const auto uuid = widget.data(widget.index(row, 4));

        {
            const auto ids = contacts.toStringList();

            ASSERT_EQ(ids.size(), 1);
            EXPECT_EQ(ids.at(0).toStdString(), contact_5_id().str());
        }

        EXPECT_EQ(polarity.toInt(), 1);
        EXPECT_EQ(workflow.toString(), u8"");
        EXPECT_EQ(type.toInt(), static_cast<int>(ot::StorageBox::BLOCKCHAIN));
        EXPECT_EQ(amount.toString(), u8"0.013\u202F809\u202F59 ₿");
        EXPECT_EQ(text.toString(), u8"Incoming Bitcoin transaction");
        EXPECT_EQ(memo.toString(), u8"");

        {
            auto expected = QDateTime{};
            expected.setSecsSinceEpoch(ot::Clock::to_time_t(time_1_));

            EXPECT_EQ(time.toDateTime(), expected);
        }

        EXPECT_FALSE(uuid.toString().isEmpty());
    }
}
#endif  // OT_QT

TEST_F(Test_BlockchainActivity, receive_assigned_account_summary)
{
    ASSERT_TRUE(wait_for_counter(account_summary_));
    // FIXME
}

TEST_F(Test_BlockchainActivity, receive_assigned_activity_summary)
{
    ASSERT_TRUE(wait_for_counter(activity_summary_));

    const auto& widget = api_.UI().ActivitySummary(nym_1_id());
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->DisplayName(), contact_6_name_);
    EXPECT_EQ(row->ImageURI(), u8"");
    EXPECT_EQ(row->Text(), u8"Incoming Bitcoin transaction: Free Ross");
    EXPECT_EQ(row->ThreadID(), contact_6_id().str());
    EXPECT_EQ(row->Timestamp(), time_2_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    ASSERT_FALSE(row->Last());

    row = widget.Next();

    EXPECT_EQ(row->DisplayName(), contact_5_name_);
    EXPECT_EQ(row->ImageURI(), u8"");
    EXPECT_EQ(row->Text(), u8"Incoming Bitcoin transaction");
    EXPECT_EQ(row->ThreadID(), contact_5_id().str());
    EXPECT_EQ(row->Timestamp(), time_1_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_TRUE(row->Last());
}

TEST_F(Test_BlockchainActivity, receive_assigned_activity_thread_1)
{
    ASSERT_TRUE(wait_for_counter(activity_thread_1_));

    const auto& widget = api_.UI().ActivityThread(nym_1_id(), contact_5_id());
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->Amount(), 1380959);
    EXPECT_FALSE(row->Deposit());
    EXPECT_EQ(row->DisplayAmount(), u8"0.013\u202F809\u202F59 ₿");
    EXPECT_FALSE(row->Loading());
    EXPECT_TRUE(row->MarkRead());
    EXPECT_EQ(row->Memo(), u8"");
    EXPECT_FALSE(row->Pending());
    EXPECT_EQ(row->Text(), u8"Incoming Bitcoin transaction");
    EXPECT_EQ(row->Timestamp(), time_1_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_TRUE(row->Last());
}

TEST_F(Test_BlockchainActivity, receive_assigned_activity_thread_2)
{
    ASSERT_TRUE(wait_for_counter(activity_thread_2_));

    const auto& widget = api_.UI().ActivityThread(nym_1_id(), contact_6_id());
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->Amount(), 1380959);
    EXPECT_FALSE(row->Deposit());
    EXPECT_EQ(row->DisplayAmount(), u8"0.013\u202F809\u202F59 ₿");
    EXPECT_FALSE(row->Loading());
    EXPECT_TRUE(row->MarkRead());
    EXPECT_EQ(row->Memo(), u8"Free Ross");
    EXPECT_FALSE(row->Pending());
    EXPECT_EQ(row->Text(), u8"Incoming Bitcoin transaction: Free Ross");
    EXPECT_EQ(row->Timestamp(), time_2_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_TRUE(row->Last());
}

// TEST_F(Test_BlockchainActivity, send)
//{
//    account_list_.expected_ += 0;
//    account_activity_.expected_ += 1;
//    account_summary_.expected_ += 0;
//    activity_summary_.expected_ += 2;
//    activity_thread_1_.expected_ += 0;
//    activity_thread_2_.expected_ += 0;
//    activity_thread_3_.expected_ += 1;
//
//    const auto& account =
//        api_.Crypto().Blockchain().HDSubaccount(nym_1_id(), account_1_id());
//    const auto& keyOne = account.BalanceElement(
//        Subchain::External, first_index_);  // 0.0033045 ₿
//    const auto& keyTwo = account.BalanceElement(
//        Subchain::External, second_index_);  // 0.01050509 ₿
//    const auto& keyThree = account.BalanceElement(
//        Subchain::External, third_index_);  // 0.0033045 ₿
//    const auto& keyFour =
//        account.BalanceElement(Subchain::External, fourth_index_);
//    const auto tx1 =
//    api_.Crypto().Blockchain().LoadTransactionBitcoin(txid_1_); const auto tx2
//    = api_.Crypto().Blockchain().LoadTransactionBitcoin(txid_2_);
//
//    ASSERT_TRUE(tx1);
//    ASSERT_TRUE(tx2);
//
//    auto input1 = ot::proto::BlockchainTransactionOutput{};
//    auto input2 = ot::proto::BlockchainTransactionOutput{};
//    auto input3 = ot::proto::BlockchainTransactionOutput{};
//
//    ASSERT_TRUE(tx1->Outputs().at(1).Serialize(api_.Crypto().Blockchain(),
//    input1));
//    ASSERT_TRUE(tx1->Outputs().at(0).Serialize(api_.Crypto().Blockchain(),
//    input2));
//    ASSERT_TRUE(tx2->Outputs().at(0).Serialize(api_.Crypto().Blockchain(),
//    input3));
//
//    input2.set_index(18);
//    input3.set_index(27);
//
//    const auto address =
//        api_.Crypto().Blockchain().DecodeAddress("17VZNX1SN5NtKa8UQFxwQbFeFc3iqRYhem");
//    const auto tx3 = get_test_transaction(
//        keyTwo,
//        keyOne,
//        keyThree,
//        keyFour,
//        input1,
//        input2,
//        input3,
//        std::get<0>(address)->asHex(),
//        time_3_);
//
//    ASSERT_TRUE(tx3);
//
//    txid_3_ = tx3->ID().asHex();
//
//    ASSERT_TRUE(api_.Crypto().Blockchain().ProcessTransaction(
//        ot::blockchain::Type::Bitcoin, *tx3, reason_));
//}

TEST_F(Test_BlockchainActivity, send_account_activity)
{
    ASSERT_TRUE(wait_for_counter(account_activity_));

    const auto& widget = api_.UI().AccountActivity(
        nym_1_id(), api_.Factory().IdentifierFromBase58(btc_account_id_));

    EXPECT_EQ(widget.AccountID(), btc_account_id_);
    EXPECT_EQ(widget.Balance(), 0);          // FIXME
    EXPECT_EQ(widget.BalancePolarity(), 0);  // FIXME
    EXPECT_EQ(widget.ContractID(), btc_unit_id_);

    const auto chains = widget.DepositChains();

    ASSERT_EQ(chains.size(), 1);
    EXPECT_EQ(chains.at(0), ot::blockchain::Type::Bitcoin);
    EXPECT_EQ(widget.DisplayBalance(), u8"0 ₿");  // FIXME
    EXPECT_EQ(widget.DisplayUnit(), u8"BTC");
    EXPECT_EQ(widget.Name(), u8"This device");
    EXPECT_EQ(widget.NotaryID(), btc_notary_id_);
    EXPECT_EQ(widget.NotaryName(), u8"Bitcoin");
    EXPECT_EQ(widget.Type(), ot::AccountType::Blockchain);
    EXPECT_EQ(widget.Unit(), ot::core::UnitType::BTC);

    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->Amount(), -1380959);

    auto contacts = row->Contacts();

    ASSERT_EQ(contacts.size(), 1);
    EXPECT_EQ(contacts.at(0), contact_7_id().str());
    EXPECT_EQ(row->DisplayAmount(), u8"-0.013\u202F809\u202F59 ₿");
    EXPECT_EQ(row->Memo(), u8"");
    EXPECT_EQ(row->Workflow(), u8"");
    EXPECT_EQ(row->Text(), u8"Outgoing Bitcoin transaction");
    EXPECT_EQ(row->Timestamp(), time_3_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_FALSE(row->UUID().empty());
    ASSERT_FALSE(row->Last());

    row = widget.Next();

    EXPECT_EQ(row->Amount(), 1380959);

    contacts = row->Contacts();

    ASSERT_EQ(contacts.size(), 1);
    EXPECT_EQ(contacts.at(0), contact_6_id().str());
    EXPECT_EQ(row->DisplayAmount(), u8"0.013\u202F809\u202F59 ₿");
    EXPECT_EQ(row->Memo(), u8"Free Ross");
    EXPECT_EQ(row->Workflow(), u8"");
    EXPECT_EQ(row->Text(), u8"Incoming Bitcoin transaction: Free Ross");
    EXPECT_EQ(row->Timestamp(), time_2_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_FALSE(row->UUID().empty());
    ASSERT_FALSE(row->Last());

    row = widget.Next();

    EXPECT_EQ(row->Amount(), 1380959);

    contacts = row->Contacts();

    ASSERT_EQ(contacts.size(), 1);
    EXPECT_EQ(contacts.at(0), contact_5_id().str());
    EXPECT_EQ(row->DisplayAmount(), u8"0.013\u202F809\u202F59 ₿");
    EXPECT_EQ(row->Memo(), u8"");
    EXPECT_EQ(row->Workflow(), u8"");
    EXPECT_EQ(row->Text(), u8"Incoming Bitcoin transaction");
    EXPECT_EQ(row->Timestamp(), time_1_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_FALSE(row->UUID().empty());
    EXPECT_TRUE(row->Last());
}

#if OT_QT
TEST_F(Test_BlockchainActivity, send_account_activity_qt)
{
    ASSERT_TRUE(wait_for_counter(account_activity_));

    const auto* pWidget = api_.UI().AccountActivityQt(
        nym_1_id(), api_.Factory().Identifier(std::string{btc_account_id_}));

    ASSERT_NE(pWidget, nullptr);

    const auto& widget = *pWidget;

    EXPECT_EQ(widget.columnCount(), 5);
    EXPECT_EQ(widget.rowCount(), 3);
    EXPECT_EQ(widget.accountID().toStdString(), btc_account_id_);
    EXPECT_EQ(widget.balancePolarity(), 0);         // FIXME
    EXPECT_EQ(widget.displayBalance(), u8"0 ₿");  // FIXME

    {
        const auto chains = widget.depositChains();

        ASSERT_EQ(chains.size(), 1);
        EXPECT_EQ(
            *chains.begin(), static_cast<int>(ot::blockchain::Type::Bitcoin));
    }

    using Model = ot::ui::AccountActivityQt;

    {
        constexpr auto row{0};

        ASSERT_TRUE(widget.hasIndex(row, 0));
        ASSERT_TRUE(widget.hasIndex(row, 1));
        ASSERT_TRUE(widget.hasIndex(row, 2));
        ASSERT_TRUE(widget.hasIndex(row, 3));
        ASSERT_TRUE(widget.hasIndex(row, 4));

        const auto contacts =
            widget.data(widget.index(row, 0), Model::ContactsRole);
        const auto polarity =
            widget.data(widget.index(row, 0), Model::PolarityRole);
        const auto workflow =
            widget.data(widget.index(row, 0), Model::WorkflowRole);
        const auto type = widget.data(widget.index(row, 0), Model::TypeRole);
        const auto amount = widget.data(widget.index(row, 0));
        const auto text = widget.data(widget.index(row, 1));
        const auto memo = widget.data(widget.index(row, 2));
        const auto time = widget.data(widget.index(row, 3));
        const auto uuid = widget.data(widget.index(row, 4));

        {
            const auto ids = contacts.toStringList();

            ASSERT_EQ(ids.size(), 1);
            EXPECT_EQ(ids.at(0).toStdString(), contact_7_id().str());
        }

        EXPECT_EQ(polarity.toInt(), -1);
        EXPECT_EQ(workflow.toString(), u8"");
        EXPECT_EQ(type.toInt(), static_cast<int>(ot::StorageBox::BLOCKCHAIN));
        EXPECT_EQ(amount.toString(), u8"-0.013\u202F809\u202F59 ₿");
        EXPECT_EQ(text.toString(), u8"Outgoing Bitcoin transaction");
        EXPECT_EQ(memo.toString(), u8"");

        {
            auto expected = QDateTime{};
            expected.setSecsSinceEpoch(ot::Clock::to_time_t(time_3_));

            EXPECT_EQ(time.toDateTime(), expected);
        }

        EXPECT_FALSE(uuid.toString().isEmpty());
    }
    {
        constexpr auto row{1};

        ASSERT_TRUE(widget.hasIndex(row, 0));
        ASSERT_TRUE(widget.hasIndex(row, 1));
        ASSERT_TRUE(widget.hasIndex(row, 2));
        ASSERT_TRUE(widget.hasIndex(row, 3));
        ASSERT_TRUE(widget.hasIndex(row, 4));

        const auto contacts =
            widget.data(widget.index(row, 0), Model::ContactsRole);
        const auto polarity =
            widget.data(widget.index(row, 0), Model::PolarityRole);
        const auto workflow =
            widget.data(widget.index(row, 0), Model::WorkflowRole);
        const auto type = widget.data(widget.index(row, 0), Model::TypeRole);
        const auto amount = widget.data(widget.index(row, 0));
        const auto text = widget.data(widget.index(row, 1));
        const auto memo = widget.data(widget.index(row, 2));
        const auto time = widget.data(widget.index(row, 3));
        const auto uuid = widget.data(widget.index(row, 4));

        {
            const auto ids = contacts.toStringList();

            ASSERT_EQ(ids.size(), 1);
            EXPECT_EQ(ids.at(0).toStdString(), contact_6_id().str());
        }

        EXPECT_EQ(polarity.toInt(), 1);
        EXPECT_EQ(workflow.toString(), u8"");
        EXPECT_EQ(type.toInt(), static_cast<int>(ot::StorageBox::BLOCKCHAIN));
        EXPECT_EQ(amount.toString(), u8"0.013\u202F809\u202F59 ₿");
        EXPECT_EQ(text.toString(), u8"Incoming Bitcoin transaction: Free Ross");
        EXPECT_EQ(memo.toString(), u8"Free Ross");

        {
            auto expected = QDateTime{};
            expected.setSecsSinceEpoch(ot::Clock::to_time_t(time_2_));

            EXPECT_EQ(time.toDateTime(), expected);
        }

        EXPECT_FALSE(uuid.toString().isEmpty());
    }
    {
        constexpr auto row{2};

        ASSERT_TRUE(widget.hasIndex(row, 0));
        ASSERT_TRUE(widget.hasIndex(row, 1));
        ASSERT_TRUE(widget.hasIndex(row, 2));
        ASSERT_TRUE(widget.hasIndex(row, 3));
        ASSERT_TRUE(widget.hasIndex(row, 4));

        const auto contacts =
            widget.data(widget.index(row, 0), Model::ContactsRole);
        const auto polarity =
            widget.data(widget.index(row, 0), Model::PolarityRole);
        const auto workflow =
            widget.data(widget.index(row, 0), Model::WorkflowRole);
        const auto type = widget.data(widget.index(row, 0), Model::TypeRole);
        const auto amount = widget.data(widget.index(row, 0));
        const auto text = widget.data(widget.index(row, 1));
        const auto memo = widget.data(widget.index(row, 2));
        const auto time = widget.data(widget.index(row, 3));
        const auto uuid = widget.data(widget.index(row, 4));

        {
            const auto ids = contacts.toStringList();

            ASSERT_EQ(ids.size(), 1);
            EXPECT_EQ(ids.at(0).toStdString(), contact_5_id().str());
        }

        EXPECT_EQ(polarity.toInt(), 1);
        EXPECT_EQ(workflow.toString(), u8"");
        EXPECT_EQ(type.toInt(), static_cast<int>(ot::StorageBox::BLOCKCHAIN));
        EXPECT_EQ(amount.toString(), u8"0.013\u202F809\u202F59 ₿");
        EXPECT_EQ(text.toString(), u8"Incoming Bitcoin transaction");
        EXPECT_EQ(memo.toString(), u8"");

        {
            auto expected = QDateTime{};
            expected.setSecsSinceEpoch(ot::Clock::to_time_t(time_1_));

            EXPECT_EQ(time.toDateTime(), expected);
        }

        EXPECT_FALSE(uuid.toString().isEmpty());
    }
}
#endif  // OT_QT

TEST_F(Test_BlockchainActivity, send_activity_summary)
{
    ASSERT_TRUE(wait_for_counter(activity_summary_));

    const auto& widget = api_.UI().ActivitySummary(nym_1_id());
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());

    EXPECT_EQ(row->DisplayName(), contact_7_name_);
    EXPECT_EQ(row->ImageURI(), u8"");
    EXPECT_EQ(row->Text(), u8"Outgoing Bitcoin transaction");
    EXPECT_EQ(row->ThreadID(), contact_7_id().str());
    EXPECT_EQ(row->Timestamp(), time_3_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    ASSERT_FALSE(row->Last());

    row = widget.Next();

    EXPECT_EQ(row->DisplayName(), contact_6_name_);
    EXPECT_EQ(row->ImageURI(), u8"");
    EXPECT_EQ(row->Text(), u8"Incoming Bitcoin transaction: Free Ross");
    EXPECT_EQ(row->ThreadID(), contact_6_id().str());
    EXPECT_EQ(row->Timestamp(), time_2_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    ASSERT_FALSE(row->Last());

    row = widget.Next();

    EXPECT_EQ(row->DisplayName(), contact_5_name_);
    EXPECT_EQ(row->ImageURI(), u8"");
    EXPECT_EQ(row->Text(), u8"Incoming Bitcoin transaction");
    EXPECT_EQ(row->ThreadID(), contact_5_id().str());
    EXPECT_EQ(row->Timestamp(), time_1_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_TRUE(row->Last());
}

TEST_F(Test_BlockchainActivity, send_activity_thread_3)
{
    ASSERT_TRUE(wait_for_counter(activity_thread_3_));

    const auto& widget = api_.UI().ActivityThread(nym_1_id(), contact_7_id());
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->Amount(), -1380959);
    EXPECT_FALSE(row->Deposit());
    EXPECT_EQ(row->DisplayAmount(), u8"-0.013\u202F809\u202F59 ₿");
    EXPECT_FALSE(row->Loading());
    EXPECT_TRUE(row->MarkRead());
    EXPECT_EQ(row->Memo(), u8"");
    EXPECT_FALSE(row->Pending());
    EXPECT_EQ(row->Text(), u8"Outgoing Bitcoin transaction");
    EXPECT_EQ(row->Timestamp(), time_3_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_TRUE(row->Last());
}

TEST_F(Test_BlockchainActivity, receive_unassigned)
{
    account_list_.expected_ += 0;
    account_activity_.expected_ += 1;
    account_summary_.expected_ += 0;
    activity_summary_.expected_ += 0;
    activity_thread_1_.expected_ += 0;
    activity_thread_2_.expected_ += 0;
    activity_thread_3_.expected_ += 0;
    const auto& account =
        api_.Crypto().Blockchain().HDSubaccount(nym_1_id(), account_1_id());
    const auto& keyFive =
        account.BalanceElement(Subchain::External, fifth_index_);
    const auto& keySix =
        account.BalanceElement(Subchain::External, sixth_index_);

    const auto tx = get_test_transaction(keyFive, keySix, time_4_);

    ASSERT_TRUE(tx);

    txid_4_ = tx->ID().asHex();

    ASSERT_TRUE(api_.Crypto().Blockchain().Internal().ProcessTransaction(
        ot::blockchain::Type::Bitcoin, *tx, reason_));
}

TEST_F(Test_BlockchainActivity, receive_unassigned_account_activity)
{
    ASSERT_TRUE(wait_for_counter(account_activity_));

    const auto& widget = api_.UI().AccountActivity(
        nym_1_id(), api_.Factory().IdentifierFromBase58(btc_account_id_));

    EXPECT_EQ(widget.AccountID(), btc_account_id_);
    EXPECT_EQ(widget.Balance(), 0);          // FIXME
    EXPECT_EQ(widget.BalancePolarity(), 0);  // FIXME
    EXPECT_EQ(widget.ContractID(), btc_unit_id_);

    const auto chains = widget.DepositChains();

    ASSERT_EQ(chains.size(), 1);
    EXPECT_EQ(chains.at(0), ot::blockchain::Type::Bitcoin);
    EXPECT_EQ(widget.DisplayBalance(), u8"0 ₿");  // FIXME
    EXPECT_EQ(widget.DisplayUnit(), u8"BTC");
    EXPECT_EQ(widget.Name(), u8"This device");
    EXPECT_EQ(widget.NotaryID(), btc_notary_id_);
    EXPECT_EQ(widget.NotaryName(), u8"Bitcoin");
    EXPECT_EQ(widget.Type(), ot::AccountType::Blockchain);
    EXPECT_EQ(widget.Unit(), ot::core::UnitType::BTC);

    auto row = widget.First();

    ASSERT_TRUE(row->Valid());
    EXPECT_EQ(row->Amount(), 1380959);

    auto contacts = row->Contacts();

    EXPECT_EQ(contacts.size(), 0);
    EXPECT_EQ(row->DisplayAmount(), u8"0.013\u202F809\u202F59 ₿");
    EXPECT_EQ(row->Memo(), u8"");
    EXPECT_EQ(row->Workflow(), u8"");
    EXPECT_EQ(row->Text(), u8"Incoming Bitcoin transaction");
    EXPECT_EQ(row->Timestamp(), time_4_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_FALSE(row->UUID().empty());
    ASSERT_FALSE(row->Last());

    row = widget.Next();

    EXPECT_EQ(row->Amount(), -1380959);

    contacts = row->Contacts();

    ASSERT_EQ(contacts.size(), 1);
    EXPECT_EQ(contacts.at(0), contact_7_id().str());
    EXPECT_EQ(row->DisplayAmount(), u8"-0.013\u202F809\u202F59 ₿");
    EXPECT_EQ(row->Memo(), u8"");
    EXPECT_EQ(row->Workflow(), u8"");
    EXPECT_EQ(row->Text(), u8"Outgoing Bitcoin transaction");
    EXPECT_EQ(row->Timestamp(), time_3_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_FALSE(row->UUID().empty());
    ASSERT_FALSE(row->Last());

    row = widget.Next();

    EXPECT_EQ(row->Amount(), 1380959);

    contacts = row->Contacts();

    ASSERT_EQ(contacts.size(), 1);
    EXPECT_EQ(contacts.at(0), contact_6_id().str());
    EXPECT_EQ(row->DisplayAmount(), u8"0.013\u202F809\u202F59 ₿");
    EXPECT_EQ(row->Memo(), u8"Free Ross");
    EXPECT_EQ(row->Workflow(), u8"");
    EXPECT_EQ(row->Text(), u8"Incoming Bitcoin transaction: Free Ross");
    EXPECT_EQ(row->Timestamp(), time_2_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_FALSE(row->UUID().empty());
    ASSERT_FALSE(row->Last());

    row = widget.Next();

    EXPECT_EQ(row->Amount(), 1380959);

    contacts = row->Contacts();

    ASSERT_EQ(contacts.size(), 1);
    EXPECT_EQ(contacts.at(0), contact_5_id().str());
    EXPECT_EQ(row->DisplayAmount(), u8"0.013\u202F809\u202F59 ₿");
    EXPECT_EQ(row->Memo(), u8"");
    EXPECT_EQ(row->Workflow(), u8"");
    EXPECT_EQ(row->Text(), u8"Incoming Bitcoin transaction");
    EXPECT_EQ(row->Timestamp(), time_1_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_FALSE(row->UUID().empty());
    EXPECT_TRUE(row->Last());
}

#if OT_QT
TEST_F(Test_BlockchainActivity, receive_unassigned_account_activity_qt)
{
    ASSERT_TRUE(wait_for_counter(account_activity_));

    const auto* pWidget = api_.UI().AccountActivityQt(
        nym_1_id(), api_.Factory().Identifier(std::string{btc_account_id_}));

    ASSERT_NE(pWidget, nullptr);

    const auto& widget = *pWidget;

    EXPECT_EQ(widget.columnCount(), 5);
    EXPECT_EQ(widget.rowCount(), 4);
    EXPECT_EQ(widget.accountID().toStdString(), btc_account_id_);
    EXPECT_EQ(widget.balancePolarity(), 0);         // FIXME
    EXPECT_EQ(widget.displayBalance(), u8"0 ₿");  // FIXME

    {
        const auto chains = widget.depositChains();

        ASSERT_EQ(chains.size(), 1);
        EXPECT_EQ(
            *chains.begin(), static_cast<int>(ot::blockchain::Type::Bitcoin));
    }

    using Model = ot::ui::AccountActivityQt;

    {
        constexpr auto row{0};

        ASSERT_TRUE(widget.hasIndex(row, 0));
        ASSERT_TRUE(widget.hasIndex(row, 1));
        ASSERT_TRUE(widget.hasIndex(row, 2));
        ASSERT_TRUE(widget.hasIndex(row, 3));
        ASSERT_TRUE(widget.hasIndex(row, 4));

        const auto contacts =
            widget.data(widget.index(row, 0), Model::ContactsRole);
        const auto polarity =
            widget.data(widget.index(row, 0), Model::PolarityRole);
        const auto workflow =
            widget.data(widget.index(row, 0), Model::WorkflowRole);
        const auto type = widget.data(widget.index(row, 0), Model::TypeRole);
        const auto amount = widget.data(widget.index(row, 0));
        const auto text = widget.data(widget.index(row, 1));
        const auto memo = widget.data(widget.index(row, 2));
        const auto time = widget.data(widget.index(row, 3));
        const auto uuid = widget.data(widget.index(row, 4));

        {
            const auto ids = contacts.toStringList();

            ASSERT_EQ(ids.size(), 0);
        }

        EXPECT_EQ(polarity.toInt(), 1);
        EXPECT_EQ(workflow.toString(), u8"");
        EXPECT_EQ(type.toInt(), static_cast<int>(ot::StorageBox::BLOCKCHAIN));
        EXPECT_EQ(amount.toString(), u8"0.013\u202F809\u202F59 ₿");
        EXPECT_EQ(text.toString(), u8"Incoming Bitcoin transaction");
        EXPECT_EQ(memo.toString(), u8"");

        {
            auto expected = QDateTime{};
            expected.setSecsSinceEpoch(ot::Clock::to_time_t(time_4_));

            EXPECT_EQ(time.toDateTime(), expected);
        }

        EXPECT_FALSE(uuid.toString().isEmpty());
    }
    {
        constexpr auto row{1};

        ASSERT_TRUE(widget.hasIndex(row, 0));
        ASSERT_TRUE(widget.hasIndex(row, 1));
        ASSERT_TRUE(widget.hasIndex(row, 2));
        ASSERT_TRUE(widget.hasIndex(row, 3));
        ASSERT_TRUE(widget.hasIndex(row, 4));

        const auto contacts =
            widget.data(widget.index(row, 0), Model::ContactsRole);
        const auto polarity =
            widget.data(widget.index(row, 0), Model::PolarityRole);
        const auto workflow =
            widget.data(widget.index(row, 0), Model::WorkflowRole);
        const auto type = widget.data(widget.index(row, 0), Model::TypeRole);
        const auto amount = widget.data(widget.index(row, 0));
        const auto text = widget.data(widget.index(row, 1));
        const auto memo = widget.data(widget.index(row, 2));
        const auto time = widget.data(widget.index(row, 3));
        const auto uuid = widget.data(widget.index(row, 4));

        {
            const auto ids = contacts.toStringList();

            ASSERT_EQ(ids.size(), 1);
            EXPECT_EQ(ids.at(0).toStdString(), contact_7_id().str());
        }

        EXPECT_EQ(polarity.toInt(), -1);
        EXPECT_EQ(workflow.toString(), u8"");
        EXPECT_EQ(type.toInt(), static_cast<int>(ot::StorageBox::BLOCKCHAIN));
        EXPECT_EQ(amount.toString(), u8"-0.013\u202F809\u202F59 ₿");
        EXPECT_EQ(text.toString(), u8"Outgoing Bitcoin transaction");
        EXPECT_EQ(memo.toString(), u8"");

        {
            auto expected = QDateTime{};
            expected.setSecsSinceEpoch(ot::Clock::to_time_t(time_3_));

            EXPECT_EQ(time.toDateTime(), expected);
        }

        EXPECT_FALSE(uuid.toString().isEmpty());
    }
    {
        constexpr auto row{2};

        ASSERT_TRUE(widget.hasIndex(row, 0));
        ASSERT_TRUE(widget.hasIndex(row, 1));
        ASSERT_TRUE(widget.hasIndex(row, 2));
        ASSERT_TRUE(widget.hasIndex(row, 3));
        ASSERT_TRUE(widget.hasIndex(row, 4));

        const auto contacts =
            widget.data(widget.index(row, 0), Model::ContactsRole);
        const auto polarity =
            widget.data(widget.index(row, 0), Model::PolarityRole);
        const auto workflow =
            widget.data(widget.index(row, 0), Model::WorkflowRole);
        const auto type = widget.data(widget.index(row, 0), Model::TypeRole);
        const auto amount = widget.data(widget.index(row, 0));
        const auto text = widget.data(widget.index(row, 1));
        const auto memo = widget.data(widget.index(row, 2));
        const auto time = widget.data(widget.index(row, 3));
        const auto uuid = widget.data(widget.index(row, 4));

        {
            const auto ids = contacts.toStringList();

            ASSERT_EQ(ids.size(), 1);
            EXPECT_EQ(ids.at(0).toStdString(), contact_6_id().str());
        }

        EXPECT_EQ(polarity.toInt(), 1);
        EXPECT_EQ(workflow.toString(), u8"");
        EXPECT_EQ(type.toInt(), static_cast<int>(ot::StorageBox::BLOCKCHAIN));
        EXPECT_EQ(amount.toString(), u8"0.013\u202F809\u202F59 ₿");
        EXPECT_EQ(text.toString(), u8"Incoming Bitcoin transaction: Free Ross");
        EXPECT_EQ(memo.toString(), u8"Free Ross");

        {
            auto expected = QDateTime{};
            expected.setSecsSinceEpoch(ot::Clock::to_time_t(time_2_));

            EXPECT_EQ(time.toDateTime(), expected);
        }

        EXPECT_FALSE(uuid.toString().isEmpty());
    }
    {
        constexpr auto row{3};

        ASSERT_TRUE(widget.hasIndex(row, 0));
        ASSERT_TRUE(widget.hasIndex(row, 1));
        ASSERT_TRUE(widget.hasIndex(row, 2));
        ASSERT_TRUE(widget.hasIndex(row, 3));
        ASSERT_TRUE(widget.hasIndex(row, 4));

        const auto contacts =
            widget.data(widget.index(row, 0), Model::ContactsRole);
        const auto polarity =
            widget.data(widget.index(row, 0), Model::PolarityRole);
        const auto workflow =
            widget.data(widget.index(row, 0), Model::WorkflowRole);
        const auto type = widget.data(widget.index(row, 0), Model::TypeRole);
        const auto amount = widget.data(widget.index(row, 0));
        const auto text = widget.data(widget.index(row, 1));
        const auto memo = widget.data(widget.index(row, 2));
        const auto time = widget.data(widget.index(row, 3));
        const auto uuid = widget.data(widget.index(row, 4));

        {
            const auto ids = contacts.toStringList();

            ASSERT_EQ(ids.size(), 1);
            EXPECT_EQ(ids.at(0).toStdString(), contact_5_id().str());
        }

        EXPECT_EQ(polarity.toInt(), 1);
        EXPECT_EQ(workflow.toString(), u8"");
        EXPECT_EQ(type.toInt(), static_cast<int>(ot::StorageBox::BLOCKCHAIN));
        EXPECT_EQ(amount.toString(), u8"0.013\u202F809\u202F59 ₿");
        EXPECT_EQ(text.toString(), u8"Incoming Bitcoin transaction");
        EXPECT_EQ(memo.toString(), u8"");

        {
            auto expected = QDateTime{};
            expected.setSecsSinceEpoch(ot::Clock::to_time_t(time_1_));

            EXPECT_EQ(time.toDateTime(), expected);
        }

        EXPECT_FALSE(uuid.toString().isEmpty());
    }
}

TEST_F(Test_BlockchainActivity, receive_unassigned_activity_summary)
{
    ASSERT_TRUE(wait_for_counter(activity_summary_));

    const auto& widget = api_.UI().ActivitySummary(nym_1_id());
    auto row = widget.First();

    ASSERT_TRUE(row->Valid());

    EXPECT_EQ(row->DisplayName(), contact_7_name_);
    EXPECT_EQ(row->ImageURI(), u8"");
    EXPECT_EQ(row->Text(), u8"Outgoing Bitcoin transaction");
    EXPECT_EQ(row->ThreadID(), contact_7_id().str());
    EXPECT_EQ(row->Timestamp(), time_3_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    ASSERT_FALSE(row->Last());

    row = widget.Next();

    EXPECT_EQ(row->DisplayName(), contact_6_name_);
    EXPECT_EQ(row->ImageURI(), u8"");
    EXPECT_EQ(row->Text(), u8"Incoming Bitcoin transaction: Free Ross");
    EXPECT_EQ(row->ThreadID(), contact_6_id().str());
    EXPECT_EQ(row->Timestamp(), time_2_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    ASSERT_FALSE(row->Last());

    row = widget.Next();

    EXPECT_EQ(row->DisplayName(), contact_5_name_);
    EXPECT_EQ(row->ImageURI(), u8"");
    EXPECT_EQ(row->Text(), u8"Incoming Bitcoin transaction");
    EXPECT_EQ(row->ThreadID(), contact_5_id().str());
    EXPECT_EQ(row->Timestamp(), time_1_);
    EXPECT_EQ(row->Type(), ot::StorageBox::BLOCKCHAIN);
    EXPECT_TRUE(row->Last());
}
#endif  // OT_QT

TEST_F(Test_BlockchainActivity, shutdown)
{
    EXPECT_EQ(account_list_.expected_, account_list_.updated_);
    EXPECT_EQ(account_activity_.expected_, account_activity_.updated_);
    EXPECT_EQ(account_summary_.expected_, account_summary_.updated_);
    EXPECT_EQ(activity_summary_.expected_, activity_summary_.updated_);
    EXPECT_EQ(activity_thread_1_.expected_, activity_thread_1_.updated_);
    EXPECT_EQ(activity_thread_2_.expected_, activity_thread_2_.updated_);
    EXPECT_EQ(activity_thread_3_.expected_, activity_thread_3_.updated_);
}
}  // namespace ottest
