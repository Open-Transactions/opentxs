// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <memory>
#include <optional>

#include "ottest/fixtures/blockchain/Activity.hpp"

namespace ottest
{
using Subchain = ot::blockchain::crypto::Subchain;

ot::UnallocatedCString txid_1_{};
ot::UnallocatedCString txid_2_{};
ot::crypto::Bip32Index first_index_{};
ot::crypto::Bip32Index second_index_{};
ot::crypto::Bip32Index third_index_{};
ot::crypto::Bip32Index fourth_index_{};

TEST_F(Test_BlockchainActivity, init)
{
    EXPECT_FALSE(nym_1_id().empty());
    EXPECT_FALSE(nym_2_id().empty());
    EXPECT_FALSE(account_1_id().empty());
    EXPECT_FALSE(account_2_id().empty());
    EXPECT_FALSE(contact_1_id().empty());
    EXPECT_FALSE(contact_2_id().empty());
    EXPECT_FALSE(contact_5_id().empty());
    EXPECT_FALSE(contact_6_id().empty());

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

    auto bytes = ot::Space{};
    auto thread1 =
        api_.Activity().Thread(nym_1_id(), contact_3_id(), ot::writer(bytes));
    auto thread2 =
        api_.Activity().Thread(nym_1_id(), contact_4_id(), ot::writer(bytes));
    auto thread3 =
        api_.Activity().Thread(nym_1_id(), contact_5_id(), ot::writer(bytes));
    auto thread4 =
        api_.Activity().Thread(nym_1_id(), contact_6_id(), ot::writer(bytes));

    EXPECT_FALSE(thread1);
    EXPECT_FALSE(thread2);
    EXPECT_FALSE(thread3);
    EXPECT_FALSE(thread4);
}

TEST_F(Test_BlockchainActivity, setup)
{
    const auto& account =
        api_.Crypto().Blockchain().HDSubaccount(nym_1_id(), account_1_id());
    const auto indexOne = account.Reserve(Subchain::External, reason_);
    const auto indexTwo = account.Reserve(Subchain::External, reason_);
    const auto indexThree = account.Reserve(Subchain::External, reason_);
    const auto indexFour = account.Reserve(Subchain::External, reason_);

    ASSERT_TRUE(indexOne.has_value());
    ASSERT_TRUE(indexTwo.has_value());
    ASSERT_TRUE(indexThree.has_value());
    ASSERT_TRUE(indexFour.has_value());

    first_index_ = indexOne.value();
    second_index_ = indexTwo.value();
    third_index_ = indexThree.value();
    fourth_index_ = indexFour.value();
    const auto& keyOne =
        account.BalanceElement(Subchain::External, first_index_);
    const auto& keyTwo =
        account.BalanceElement(Subchain::External, second_index_);
    const auto& keyThree =
        account.BalanceElement(Subchain::External, third_index_);
    const auto& keyFour =
        account.BalanceElement(Subchain::External, fourth_index_);

    EXPECT_TRUE(api_.Crypto().Blockchain().AssignContact(
        nym_1_id(),
        account_1_id(),
        Subchain::External,
        first_index_,
        contact_5_id()));
    EXPECT_TRUE(api_.Crypto().Blockchain().AssignContact(
        nym_1_id(),
        account_1_id(),
        Subchain::External,
        third_index_,
        contact_6_id()));

    const auto tx1 = get_test_transaction(keyOne, keyTwo);
    const auto tx2 = get_test_transaction(keyThree, keyFour);

    EXPECT_TRUE(tx1.IsValid());
    EXPECT_TRUE(tx2.IsValid());

    txid_1_ = tx1.ID().asHex();
    txid_2_ = tx2.ID().asHex();

    // ASSERT_TRUE(api_.Crypto().Blockchain().Internal().ProcessTransaction(
    //     ot::blockchain::Type::Bitcoin, *tx1, reason_));
    // ASSERT_TRUE(api_.Crypto().Blockchain().Internal().ProcessTransaction(
    //     ot::blockchain::Type::Bitcoin, *tx2, reason_));

    auto bytes = ot::Space{};
    auto thread1 =
        api_.Activity().Thread(nym_1_id(), contact_5_id(), ot::writer(bytes));
    auto thread2 =
        api_.Activity().Thread(nym_1_id(), contact_6_id(), ot::writer(bytes));

    ASSERT_TRUE(thread1);
    ASSERT_TRUE(thread2);

    //    EXPECT_TRUE(check_thread(*thread1, txid_1_));
    //    EXPECT_TRUE(check_thread(*thread2, txid_2_));
}

TEST_F(Test_BlockchainActivity, merge)
{
    // TODO ASSERT_TRUE(api_.Contacts().Merge(contact_5_id(), contact_6_id()));
}
}  // namespace ottest
