/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#include "opentxs/opentxs.hpp"

#include <gtest/gtest.h>

using namespace opentxs;

namespace
{

class Test_StoreOutgoing : public ::testing::Test
{
public:
  std::string Alice, Bob, Charly;
  OTIdentifier AccountID;
  
  // these fingerprints are deterministic so we can share them among tests
  Test_StoreOutgoing()
    : Alice(opentxs::OT::App().API().Exec().CreateNymHD(proto::CITEMTYPE_INDIVIDUAL, "testStoreOutgoing_A", "", 100))
    , Bob(opentxs::OT::App().API().Exec().CreateNymHD(proto::CITEMTYPE_INDIVIDUAL, "testStoreOutgoing_B", "", 101))
    , Charly(opentxs::OT::App().API().Exec().CreateNymHD(proto::CITEMTYPE_INDIVIDUAL, "testStoreOutgoing_C", "", 102))
    , AccountID(OT::App().Blockchain().NewAccount(Identifier(Alice),
                                                  BlockchainAccountType::BIP44,
                                                  static_cast<proto::ContactItemType>(proto::CITEMTYPE_BTC)))
  {
  }
};
  
proto::BlockchainTransaction* MakeTransaction(const std::string id)
{
  proto::BlockchainTransaction* Tx = new proto::BlockchainTransaction;
  Tx->set_version(1);
  Tx->set_txid(id);
  Tx->set_chain(static_cast<proto::ContactItemType>(proto::CITEMTYPE_BTC));
  Tx->set_txversion(1);
  Tx->set_fee(1827);
  Tx->set_memo("memo1");
  Tx->set_confirmations(7);
  return Tx;
}

TEST_F(Test_StoreOutgoing, testDeposit)
{
  // test: Alice has no activity records
  ObjectList AThreads = OT::App().Activity().Threads(Identifier(Alice), false);
  ASSERT_EQ(0, AThreads.size());

  // test:: Activity::Thread has deposit
  std::shared_ptr<proto::Bip44Account> Account = OT::App().Blockchain().Account(Identifier(Alice), AccountID);
  // test: no outgoing transactions
  ASSERT_EQ((*Account.get()).outgoing_size(), 0);

  // 1. Allocate deposit address
  std::unique_ptr<proto::Bip44Address> Address = opentxs::OT::App().Blockchain().AllocateAddress(Identifier(Alice),
                                                                                                 Identifier(AccountID),
                                                                                                 "Deposit 1", EXTERNAL_CHAIN);

  // 2. Assign to Bob
  bool assigned = opentxs::OT::App().Blockchain().AssignAddress(Identifier(Alice), Identifier(AccountID), Address->index(), Identifier(Bob), EXTERNAL_CHAIN);
  // test: deposit address associated to bob
  ASSERT_TRUE(assigned);

  // 3. Store outgoing transaction: from Alice to Bob
  proto::BlockchainTransaction* Tx = MakeTransaction("ff041ccd67dd63b88a55f4681229108363c7615932ccbe73b68f4fffd1697ac6");
  bool Stored= opentxs::OT::App().Blockchain().StoreOutgoing(Identifier(Alice), Identifier(AccountID), Identifier(Bob), *Tx);
  std::cout << "Stored outgoing transaction " << Tx->txid() << " !!\n";
  EXPECT_TRUE(Stored);

  // test: transaction is saved
  std::string TXID = Tx->txid();
  std::shared_ptr<proto::BlockchainTransaction> StoredOutgoingTx = opentxs::OT::App().Blockchain().Transaction(Tx->txid());
  proto::BlockchainTransaction& StoredTx = *StoredOutgoingTx.get();
  EXPECT_TRUE(bool(StoredOutgoingTx));

  // test: transaction associated in account
  std::shared_ptr<proto::Bip44Account> ReloadedAccount = OT::App().Blockchain().Account(Identifier(Alice), AccountID);
  ASSERT_EQ(ReloadedAccount->outgoing_size(), 1);
  ASSERT_STREQ(ReloadedAccount->outgoing(0).c_str(), Tx->txid().c_str());

  // test: Activity::Thread contains deposit item
  std::shared_ptr<proto::StorageThread> Thread_AB = opentxs::OT::App().Activity().Thread(Identifier(Alice), Identifier(Bob));
  ASSERT_EQ(1, Thread_AB->item_size());
  EXPECT_EQ(1, Thread_AB->participant_size());
  EXPECT_STREQ(Bob.c_str(), Thread_AB->participant(0).c_str());
  EXPECT_EQ(1, Thread_AB->version());
  EXPECT_STREQ(Bob.c_str(), Thread_AB->id().c_str());

  proto::StorageThreadItem Deposit = Thread_AB->item(0);

  EXPECT_EQ(1, Deposit.version());
  EXPECT_STREQ(StoredTx.txid().c_str(), Deposit.id().c_str());
  EXPECT_EQ(0, Deposit.index());
  EXPECT_EQ(0, Deposit.time());
  EXPECT_EQ(11, Deposit.box());
  EXPECT_STREQ("", Deposit.account().c_str());
  EXPECT_FALSE(Deposit.unread());
}

TEST_F(Test_StoreOutgoing, testDeposit_UnknownContact)
{
  // test: Alice has acvitiy with previous contact
  ObjectList AThreads = OT::App().Activity().Threads(Identifier(Alice), false);
  EXPECT_EQ(1, AThreads.size());

  // test:: account contains an outgoing tx
  std::shared_ptr<proto::Bip44Account> Account = OT::App().Blockchain().Account(Identifier(Alice), AccountID);
  ASSERT_EQ((*Account.get()).outgoing_size(), 1);

  // 1. Allocate deposit address
  std::unique_ptr<proto::Bip44Address> Address = opentxs::OT::App().Blockchain().AllocateAddress(Identifier(Alice),
                                                                                                 Identifier(AccountID),
                                                                                                 "Deposit 1", EXTERNAL_CHAIN);

  // 2. Store outgoing transaction: from Alice to Charly
  proto::BlockchainTransaction* Tx = MakeTransaction("855cd591c6502d1c81cfe38db8e0d8404ca09c2c3bc878e07f4cd0ca3afd7793");
  bool Stored= opentxs::OT::App().Blockchain().StoreOutgoing(Identifier(Alice), Identifier(AccountID), Identifier(Charly), *Tx);
  std::cout << "Stored outgoing transaction " << Tx->txid() << " !!\n";
  EXPECT_TRUE(Stored);

  // test: Activity::Thread contains deposit item
  std::shared_ptr<proto::StorageThread> Thread_AB_ = opentxs::OT::App().Activity().Thread(Identifier(Alice), Identifier(Charly));
  ASSERT_EQ(1, Thread_AB_->item_size());

  // 3. Assign to Charly
  bool assigned = opentxs::OT::App().Blockchain().AssignAddress(Identifier(Alice), Identifier(AccountID), Address->index(), Identifier(Charly), EXTERNAL_CHAIN);
  // test: deposit address associated to Charly
  ASSERT_TRUE(assigned);

  // test: transaction is saved
  std::string TXID = Tx->txid();
  std::shared_ptr<proto::BlockchainTransaction> StoredOutgoingTx = opentxs::OT::App().Blockchain().Transaction(Tx->txid());
  proto::BlockchainTransaction& StoredTx = *StoredOutgoingTx.get();
  EXPECT_TRUE(bool(StoredOutgoingTx));

  // test: transaction associated in account
  std::shared_ptr<proto::Bip44Account> ReloadedAccount = OT::App().Blockchain().Account(Identifier(Alice), AccountID);
  ASSERT_EQ(ReloadedAccount->outgoing_size(), 2);
  ASSERT_STREQ(ReloadedAccount->outgoing(1).c_str(), Tx->txid().c_str());

  // test: Activity::Thread contains deposit item
  OTIdentifier CharlyContactID = Identifier(Charly);
  //OTIdentifier CharlyContactID = OT::App().Contact().ContactID(Identifier(Charly));
  std::shared_ptr<proto::StorageThread> Thread_AC = opentxs::OT::App().Activity().Thread(Identifier(Alice), Identifier(CharlyContactID));
  ASSERT_EQ(1, Thread_AC->item_size());
  EXPECT_EQ(1, Thread_AC->participant_size());
  EXPECT_STREQ(Charly.c_str(), Thread_AC->participant(0).c_str());
  EXPECT_EQ(1, Thread_AC->version());
  EXPECT_STREQ(Charly.c_str(), Thread_AC->id().c_str());

  // test: Alice has acvitiy with Bob (from previous test) and Charly
  ObjectList AThreads_ = OT::App().Activity().Threads(Identifier(Alice), false);
  EXPECT_EQ(2, AThreads_.size());

  proto::StorageThreadItem DepositToCharly = Thread_AC->item(0);

  EXPECT_EQ(1, DepositToCharly.version());
  EXPECT_STREQ(StoredTx.txid().c_str(), DepositToCharly.id().c_str());
  EXPECT_EQ(0, DepositToCharly.index());
  EXPECT_EQ(0, DepositToCharly.time());
  EXPECT_EQ(11, DepositToCharly.box());
  EXPECT_STREQ("", DepositToCharly.account().c_str());
  EXPECT_FALSE(DepositToCharly.unread());
}
}  // namespace
