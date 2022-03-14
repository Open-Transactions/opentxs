// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <memory>
#include <optional>

#include "Helpers.hpp"
#include "internal/api/crypto/Blockchain.hpp"
#include "opentxs/api/crypto/Blockchain.hpp"
#include "opentxs/api/session/Activity.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Contacts.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Storage.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/block/bitcoin/Transaction.hpp"
#include "opentxs/blockchain/crypto/HD.hpp"
#include "opentxs/blockchain/crypto/Subchain.hpp"
#include "opentxs/core/Contact.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/PasswordPrompt.hpp"

using Subchain = ot::blockchain::crypto::Subchain;

ot::UnallocatedCString txid_{};
ot::Bip32Index first_index_{};
ot::Bip32Index second_index_{};

namespace ottest
{
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

TEST_F(Test_BlockchainActivity, inputs)
{
    const auto& account =
        api_.Crypto().Blockchain().HDSubaccount(nym_1_id(), account_1_id());
    const auto indexOne = account.Reserve(Subchain::External, reason_);
    const auto indexTwo = account.Reserve(Subchain::External, reason_);

    ASSERT_TRUE(indexOne.has_value());
    ASSERT_TRUE(indexTwo.has_value());

    first_index_ = indexOne.value();
    second_index_ = indexTwo.value();
    const auto& keyOne =
        account.BalanceElement(Subchain::External, first_index_);
    const auto& keyTwo =
        account.BalanceElement(Subchain::External, second_index_);
    const auto incoming = get_test_transaction(keyOne, keyTwo);

    ASSERT_TRUE(incoming);

    txid_ = incoming->ID().asHex();

    auto list = api_.Storage().BlockchainThreadMap(nym_1_id(), incoming->ID());

    EXPECT_EQ(list.size(), 0);
    ASSERT_TRUE(api_.Crypto().Blockchain().Internal().ProcessTransaction(
        ot::blockchain::Type::Bitcoin, *incoming, reason_));

    auto bytes = ot::Space{};
    auto thread1 =
        api_.Activity().Thread(nym_1_id(), contact_3_id(), ot::writer(bytes));
    auto thread2 =
        api_.Activity().Thread(nym_1_id(), contact_4_id(), ot::writer(bytes));
    auto thread3 =
        api_.Activity().Thread(nym_1_id(), contact_5_id(), ot::writer(bytes));
    auto thread4 =
        api_.Activity().Thread(nym_1_id(), contact_6_id(), ot::writer(bytes));

    ASSERT_TRUE(thread1);
    ASSERT_TRUE(thread2);
    EXPECT_FALSE(thread3);
    EXPECT_FALSE(thread4);

    list = api_.Storage().BlockchainThreadMap(nym_1_id(), incoming->ID());

    EXPECT_EQ(list.size(), 2);
}

TEST_F(Test_BlockchainActivity, contact5)
{
    ASSERT_TRUE(api_.Crypto().Blockchain().AssignContact(
        nym_1_id(),
        account_1_id(),
        Subchain::External,
        first_index_,
        contact_5_id()));

    auto bytes = ot::Space{};
    auto thread1 =
        api_.Activity().Thread(nym_1_id(), contact_3_id(), ot::writer(bytes));
    auto thread2 =
        api_.Activity().Thread(nym_1_id(), contact_4_id(), ot::writer(bytes));
    auto thread3 =
        api_.Activity().Thread(nym_1_id(), contact_5_id(), ot::writer(bytes));
    auto thread4 =
        api_.Activity().Thread(nym_1_id(), contact_6_id(), ot::writer(bytes));

    ASSERT_TRUE(thread1);
    ASSERT_TRUE(thread2);
    ASSERT_TRUE(thread3);
    EXPECT_FALSE(thread4);
}

TEST_F(Test_BlockchainActivity, contact6)
{
    ASSERT_TRUE(api_.Crypto().Blockchain().AssignContact(
        nym_1_id(),
        account_1_id(),
        Subchain::External,
        second_index_,
        contact_6_id()));

    auto bytes = ot::Space{};
    auto thread1 =
        api_.Activity().Thread(nym_1_id(), contact_3_id(), ot::writer(bytes));
    auto thread2 =
        api_.Activity().Thread(nym_1_id(), contact_4_id(), ot::writer(bytes));
    auto thread3 =
        api_.Activity().Thread(nym_1_id(), contact_5_id(), ot::writer(bytes));
    auto thread4 =
        api_.Activity().Thread(nym_1_id(), contact_6_id(), ot::writer(bytes));

    ASSERT_TRUE(thread1);
    ASSERT_TRUE(thread2);
    ASSERT_TRUE(thread3);
    ASSERT_TRUE(thread4);
}

TEST_F(Test_BlockchainActivity, unassign)
{
    ASSERT_TRUE(api_.Crypto().Blockchain().AssignContact(
        nym_1_id(),
        account_1_id(),
        Subchain::External,
        second_index_,
        api_.Factory().Identifier()));

    auto bytes = ot::Space{};
    auto thread1 =
        api_.Activity().Thread(nym_1_id(), contact_3_id(), ot::writer(bytes));
    auto thread2 =
        api_.Activity().Thread(nym_1_id(), contact_4_id(), ot::writer(bytes));
    auto thread3 =
        api_.Activity().Thread(nym_1_id(), contact_5_id(), ot::writer(bytes));
    auto thread4 =
        api_.Activity().Thread(nym_1_id(), contact_6_id(), ot::writer(bytes));

    ASSERT_TRUE(thread1);
    ASSERT_TRUE(thread2);
    ASSERT_TRUE(thread3);
    ASSERT_TRUE(thread4);
}
}  // namespace ottest
