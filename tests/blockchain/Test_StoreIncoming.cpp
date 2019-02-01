// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"

#include <gtest/gtest.h>

using namespace opentxs;

namespace
{

class Test_StoreIncoming : public ::testing::Test
{
public:
    const opentxs::api::client::Manager& client_;
    std::string Alice, Bob, Charly;
    OTIdentifier AccountID;

    // these fingerprints are deterministic so we can share them among tests
    Test_StoreIncoming()
        : client_(opentxs::OT::App().StartClient({}, 0))
        , Alice(client_.Exec().CreateNymHD(
              proto::CITEMTYPE_INDIVIDUAL,
              "testStoreIncoming_A",
              "",
              90))
        , Bob(client_.Exec().CreateNymHD(
              proto::CITEMTYPE_INDIVIDUAL,
              "testStoreIncoming_B",
              "",
              91))
        , Charly(client_.Exec().CreateNymHD(
              proto::CITEMTYPE_INDIVIDUAL,
              "testStoreIncoming_C",
              "",
              92))
        , AccountID(client_.Blockchain().NewAccount(
              identifier::Nym::Factory(Alice),
              BlockchainAccountType::BIP44,
              proto::CITEMTYPE_BTC))
    {
    }
};

proto::BlockchainTransaction MakeTransaction(const std::string id)
{
    proto::BlockchainTransaction Tx;
    Tx.set_version(1);
    Tx.set_txid(id);
    Tx.set_chain(proto::CITEMTYPE_BTC);
    Tx.set_txversion(1);
    Tx.set_fee(1827);
    Tx.set_memo("memo1");
    Tx.set_confirmations(7);

    return Tx;
}

TEST_F(Test_StoreIncoming, testIncomingDeposit1)
{
    std::cout << "testIncomingDeposit1 started. \n";
    std::cout << "Alice's Nym: " << Alice << "\n";
    std::cout << "Bob's Nym: " << Bob << "\n";
    std::cout << "Charly's Nym: " << Charly << "\n";

    // 1. allocate deposit address
    std::unique_ptr<proto::Bip44Address> Address =
        client_.Blockchain().AllocateAddress(
            identifier::Nym::Factory(Alice),
            Identifier::Factory(AccountID),
            "Deposit 1",
            EXTERNAL_CHAIN);

    // test: address allocated
    std::unique_ptr<proto::Bip44Address> AddrPtr =
        client_.Blockchain().LoadAddress(
            identifier::Nym::Factory(Alice),
            Identifier::Factory(AccountID),
            0,
            EXTERNAL_CHAIN);
    EXPECT_TRUE(bool(AddrPtr));
    EXPECT_EQ(AddrPtr->incoming_size(), 0);

    // 2. assign to Bob
    bool assigned = client_.Blockchain().AssignAddress(
        identifier::Nym::Factory(Alice),
        Identifier::Factory(AccountID),
        0,
        Identifier::Factory(Bob),
        EXTERNAL_CHAIN);
    EXPECT_TRUE(assigned);

    // 3. store incoming transaction
    auto Tx = MakeTransaction(
        "5ddfedaf76b3abd902e1860115e163957aa16f72fc56b1f61bf314fc37781618");
    bool Stored = client_.Blockchain().StoreIncoming(
        identifier::Nym::Factory(Alice),
        Identifier::Factory(AccountID),
        AddrPtr->index(),
        EXTERNAL_CHAIN,
        Tx);

    std::cout << "Stored incoming transaction " << Tx.txid() << "\n";
    EXPECT_TRUE(Stored);

    // test: transaction is saved
    std::string TXID = Tx.txid();
    std::shared_ptr<proto::BlockchainTransaction> StoredTx =
        client_.Blockchain().Transaction(Tx.txid());

    EXPECT_TRUE(bool(StoredTx));
    EXPECT_EQ(StoredTx->version(), 1);
    EXPECT_EQ(StoredTx->txversion(), 1);
    EXPECT_EQ(StoredTx->chain(), proto::CITEMTYPE_BTC);
    EXPECT_EQ(StoredTx->fee(), 1827);
    EXPECT_EQ(StoredTx->confirmations(), 7);
    EXPECT_STREQ(StoredTx->memo().c_str(), "memo1");

    // test: tx is associated in deposit address
    std::unique_ptr<proto::Bip44Address> NewAddrPtr =
        client_.Blockchain().LoadAddress(
            identifier::Nym::Factory(Alice),
            Identifier::Factory(AccountID),
            0,
            EXTERNAL_CHAIN);
    EXPECT_TRUE(bool(NewAddrPtr));
    EXPECT_EQ(NewAddrPtr->incoming_size(), 1);
    EXPECT_EQ(NewAddrPtr->index(), 0);
    EXPECT_STREQ(NewAddrPtr->incoming(0).c_str(), StoredTx->txid().c_str());

    // test: Activity::Thread has deposit
    std::shared_ptr<proto::StorageThread> Thread_AB = client_.Activity().Thread(
        identifier::Nym::Factory(Alice), Identifier::Factory(Bob));
    ASSERT_EQ(1, Thread_AB->item_size());
    EXPECT_EQ(1, Thread_AB->participant_size());
    EXPECT_STREQ(Bob.c_str(), Thread_AB->participant(0).c_str());
    EXPECT_EQ(1, Thread_AB->version());
    EXPECT_STREQ(Bob.c_str(), Thread_AB->id().c_str());

    proto::StorageThreadItem Deposit = Thread_AB->item(0);

    EXPECT_EQ(1, Deposit.version());
    EXPECT_STREQ(StoredTx->txid().c_str(), Deposit.id().c_str());
    EXPECT_EQ(0, Deposit.index());
    EXPECT_EQ(0, Deposit.time());
    EXPECT_EQ(10, Deposit.box());
    EXPECT_STREQ("", Deposit.account().c_str());
    EXPECT_TRUE(Deposit.unread());

    // testL a second deposit from Bob
    std::unique_ptr<proto::Bip44Address> Address2 =
        client_.Blockchain().AllocateAddress(
            identifier::Nym::Factory(Alice),
            Identifier::Factory(AccountID),
            "Deposit 2",
            EXTERNAL_CHAIN);

    bool assigned2 = client_.Blockchain().AssignAddress(
        identifier::Nym::Factory(Alice),
        Identifier::Factory(AccountID),
        Address2->index(),
        Identifier::Factory(Bob),
        EXTERNAL_CHAIN);
    EXPECT_TRUE(assigned2);

    auto Tx2 = MakeTransaction(
        "6ddfedaf76b3abd902e1860115e163957aa16f72fc56b1f61bf314fc37781616");
    bool Stored2 = client_.Blockchain().StoreIncoming(
        identifier::Nym::Factory(Alice),
        Identifier::Factory(AccountID),
        Address2->index(),
        EXTERNAL_CHAIN,
        Tx2);
    EXPECT_TRUE(Stored2);

    std::shared_ptr<proto::BlockchainTransaction> StoredTx2 =
        client_.Blockchain().Transaction(Tx2.txid());
    // test: tx is associated in deposit address
    std::unique_ptr<proto::Bip44Address> AddrPtr2 =
        client_.Blockchain().LoadAddress(
            identifier::Nym::Factory(Alice),
            Identifier::Factory(AccountID),
            Address2->index(),
            EXTERNAL_CHAIN);
    EXPECT_TRUE(bool(AddrPtr2));
    EXPECT_EQ(AddrPtr2->incoming_size(), 1);
    EXPECT_EQ(AddrPtr2->index(), 1);
    EXPECT_STREQ(AddrPtr2->incoming(0).c_str(), StoredTx2->txid().c_str());

    // test: Alice<->Bob events are 2
    std::shared_ptr<proto::StorageThread> Thread_AB_ =
        client_.Activity().Thread(
            identifier::Nym::Factory(Alice), Identifier::Factory(Bob));
    ASSERT_EQ(2, Thread_AB_->item_size());
}

TEST_F(Test_StoreIncoming, testIncomingDeposit_UnknownContact)
{
    std::cout << "testIncomingDeposit_UnknownContact started. \n";
    std::cout << "Alice's Nym: " << Alice << "\n";
    std::cout << "Bob's Nym: " << Bob << "\n";
    std::cout << "Charly's Nym: " << Charly << "\n";

    std::cout << "Started Uknown contact test !!!\n";

    // test: Alice has activity record with Bob
    ObjectList AThreads =
        client_.Activity().Threads(identifier::Nym::Factory(Alice), false);
    ASSERT_EQ(1, AThreads.size());

    std::shared_ptr<proto::Bip44Account> Account = client_.Blockchain().Account(
        identifier::Nym::Factory(Alice), AccountID);
    const std::int8_t NextDepositIndex = 2;

    // 1. allocate deposit address
    std::unique_ptr<proto::Bip44Address> Address =
        client_.Blockchain().AllocateAddress(
            identifier::Nym::Factory(Alice),
            Identifier::Factory(AccountID),
            "Deposit 2",
            EXTERNAL_CHAIN);
    std::cout << "Created Address " << Address->address() << "\n";
    EXPECT_EQ(NextDepositIndex, Address->index());

    // test: address allocated
    std::unique_ptr<proto::Bip44Address> AddrPtr =
        client_.Blockchain().LoadAddress(
            identifier::Nym::Factory(Alice),
            Identifier::Factory(AccountID),
            Address->index(),
            EXTERNAL_CHAIN);
    EXPECT_TRUE(bool(AddrPtr));
    EXPECT_EQ(AddrPtr->incoming_size(), 0);
    EXPECT_EQ(AddrPtr->index(), NextDepositIndex);

    // 2. store incoming transaction
    auto Tx = MakeTransaction(
        "5688c51b241770ff488eb1c425d608e9de0c25d4df31f0b49fa2b5a90dade126");
    bool Stored = client_.Blockchain().StoreIncoming(
        identifier::Nym::Factory(Alice),
        Identifier::Factory(AccountID),
        AddrPtr->index(),
        EXTERNAL_CHAIN,
        Tx);

    std::cout << "Stored incoming transaction " << Tx.txid() << "\n";
    EXPECT_TRUE(Stored);

    // test: transaction is saved
    std::string TXID = Tx.txid();
    std::shared_ptr<proto::BlockchainTransaction> StoredTx =
        client_.Blockchain().Transaction(Tx.txid());

    EXPECT_TRUE(bool(StoredTx));
    EXPECT_EQ(StoredTx->version(), 1);
    EXPECT_EQ(StoredTx->txversion(), 1);
    EXPECT_EQ(StoredTx->chain(), proto::CITEMTYPE_BTC);
    EXPECT_EQ(StoredTx->fee(), 1827);
    EXPECT_EQ(StoredTx->confirmations(), 7);
    EXPECT_STREQ(StoredTx->memo().c_str(), "memo1");

    // test: tx is associated in deposit address #2
    std::unique_ptr<proto::Bip44Address> NewAddrPtr =
        client_.Blockchain().LoadAddress(
            identifier::Nym::Factory(Alice),
            Identifier::Factory(AccountID),
            AddrPtr->index(),
            EXTERNAL_CHAIN);
    EXPECT_TRUE(bool(NewAddrPtr));
    EXPECT_EQ(NewAddrPtr->incoming_size(), 1);
    EXPECT_EQ(NewAddrPtr->index(), NextDepositIndex);
    EXPECT_STREQ(NewAddrPtr->incoming(0).c_str(), StoredTx->txid().c_str());

    // 3. Assign deposit address to contact
    bool assigned = client_.Blockchain().AssignAddress(
        identifier::Nym::Factory(Alice),
        Identifier::Factory(AccountID),
        AddrPtr->index(),
        Identifier::Factory(Charly),
        EXTERNAL_CHAIN);
    std::cout << "Assigned address: " << assigned
              << " index: " << AddrPtr->index() << " \n";
    ASSERT_TRUE(assigned);

    // test: deposit is included in Activity::Thread
    std::shared_ptr<proto::StorageThread> Thread_AC = client_.Activity().Thread(
        identifier::Nym::Factory(Alice), Identifier::Factory(Charly));
    ASSERT_EQ(1, Thread_AC->item_size());
    EXPECT_EQ(1, Thread_AC->participant_size());
    EXPECT_STREQ(Charly.c_str(), Thread_AC->participant(0).c_str());
    EXPECT_EQ(1, Thread_AC->version());
    EXPECT_STREQ(Charly.c_str(), Thread_AC->id().c_str());

    proto::StorageThreadItem Deposit = Thread_AC->item(0);

    EXPECT_EQ(1, Deposit.version());
    EXPECT_STREQ(StoredTx->txid().c_str(), Deposit.id().c_str());
    EXPECT_EQ(0, Deposit.index());
    EXPECT_EQ(0, Deposit.time());
    EXPECT_EQ(10, Deposit.box());
    EXPECT_STREQ("", Deposit.account().c_str());
    EXPECT_TRUE(Deposit.unread());
}
}  // namespace
