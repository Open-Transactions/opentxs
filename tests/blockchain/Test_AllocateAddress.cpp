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

class Test_AllocateAddress : public ::testing::Test
{
public:
    std::string SeedA_;
    std::string SeedB_;
    std::string SeedC_;

    // these fingerprints are deterministic so we can share them among tests
    Test_AllocateAddress()
        : SeedA_(opentxs::OT::App().API().Exec().Wallet_ImportSeed(
              "response seminar brave tip suit recall often sound stick owner "
              "lottery motion",
              ""))
        , SeedB_(opentxs::OT::App().API().Exec().Wallet_ImportSeed(
              "reward upper indicate eight swift arch injury crystal super "
              "wrestle already dentist",
              ""))
        , SeedC_(opentxs::OT::App().API().Exec().Wallet_ImportSeed(
              "predict cinnamon gauge spoon media food nurse improve employ "
              "similar own kid genius seed ghost",
              ""))
    {
    }
};

TEST_F(Test_AllocateAddress, testBip32_SeedA)
{
    const auto Alice = opentxs::OT::App().API().Exec().CreateNymHD(
        proto::CITEMTYPE_INDIVIDUAL, "Alice", SeedA_, 0);

    // Check m / 0'
    const ConstNym NymA = opentxs::OT::App().Wallet().Nym(Identifier(Alice));
    proto::HDPath pathA;
    ASSERT_TRUE(NymA.get()->Path(pathA));
    ASSERT_EQ(
        0 | static_cast<std::uint32_t>(Bip32Child::HARDENED), pathA.child(1));

    OTIdentifier AccountID = OT::App().Blockchain().NewAccount(
        Identifier(Alice),
        BlockchainAccountType::BIP32,
        static_cast<proto::ContactItemType>(proto::CITEMTYPE_BTC));
    std::shared_ptr<proto::Bip44Account> Account =
        OT::App().Blockchain().Account(Identifier(Alice), AccountID);
    ASSERT_EQ((*Account.get()).internalindex(), 0);
    ASSERT_EQ((*Account.get()).externalindex(), 0);

    // Check deposit address m / 0' / 0 / 0
    const auto AddressOut = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Alice), Identifier(AccountID), "Deposit 1", EXTERNAL_CHAIN);
    EXPECT_STREQ(
        (*AddressOut.get()).address().c_str(),
        "1K9teXNg8iKYwUPregT8QTmMepb376oTuX");
    ASSERT_EQ((*AddressOut.get()).index(), 0);

    // Check change address m / 0' / 1 / 0

    const auto AddressIn = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Alice), Identifier(AccountID), "Change 1", INTERNAL_CHAIN);
    EXPECT_STREQ(
        (*AddressIn.get()).address().c_str(),
        "179XLYWcaHiPMnPUsSdrPiAwNcybx2vpaa");
    ASSERT_EQ((*AddressIn.get()).index(), 0);

    // Check deposit address m / 0' / 0 / 1

    const auto AddressOut2 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Alice), Identifier(AccountID), "Deposit 2", EXTERNAL_CHAIN);
    EXPECT_STREQ(
        (*AddressOut2.get()).address().c_str(),
        "1GgpoMuPBfaa4ZT6ZeKaTY8NH9Ldx4Q89t");

    // Check change address m / 0' / 1 / 0

    const auto AddressIn2 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Alice), Identifier(AccountID), "Change 2", INTERNAL_CHAIN);
    EXPECT_STREQ(
        (*AddressIn2.get()).address().c_str(),
        "1FPoX1BUe9a6ugobnQkzFyn1Uycyns4Ejp");

    // Check deposit addresses m / 0' / 0 / 2 ... m / 0' / 0 / 9

    const auto AddressOut3 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Alice), Identifier(AccountID), "Deposit 3", EXTERNAL_CHAIN);
    EXPECT_STREQ(
        (*AddressOut3.get()).address().c_str(),
        "1FXb97adaza32zYQ5U29nxHZS4FmiCfXAJ");

    const auto AddressOut4 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Alice), Identifier(AccountID), "Deposit 4", EXTERNAL_CHAIN);
    EXPECT_STREQ(
        (*AddressOut4.get()).address().c_str(),
        "1Dx4k7daUS1VNNeoDtZe1ujpt99YeW7Yz");

    const auto AddressOut5 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Alice), Identifier(AccountID), "Deposit 5", EXTERNAL_CHAIN);
    EXPECT_STREQ(
        (*AddressOut5.get()).address().c_str(),
        "19KhniSVj1CovZWg1P5JvoM199nQR3gkhp");

    const auto AddressOut6 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Alice), Identifier(AccountID), "Deposit 6", EXTERNAL_CHAIN);
    EXPECT_STREQ(
        (*AddressOut6.get()).address().c_str(),
        "1CBnxZdo58Vu3upwEt96uTMZLAxVx4Xeg9");

    const auto AddressOut7 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Alice), Identifier(AccountID), "Deposit 7", EXTERNAL_CHAIN);
    EXPECT_STREQ(
        (*AddressOut7.get()).address().c_str(),
        "12vm2SqQ7RhhYPi6bJqqQzyJomV6H3j4AX");

    const auto AddressOut8 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Alice), Identifier(AccountID), "Deposit 8", EXTERNAL_CHAIN);
    EXPECT_STREQ(
        (*AddressOut8.get()).address().c_str(),
        "1D2fNJYjyWL1jn5qRhJZL6EbGzeyBjHuP3");

    const auto AddressOut9 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Alice), Identifier(AccountID), "Deposit 9", EXTERNAL_CHAIN);
    EXPECT_STREQ(
        (*AddressOut9.get()).address().c_str(),
        "19w4gVEse89JjE7TroavXZ9pyfJ78h4arG");

    const auto AddressOut10 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Alice), Identifier(AccountID), "Deposit 10", EXTERNAL_CHAIN);
    EXPECT_STREQ(
        (*AddressOut10.get()).address().c_str(),
        "1DVYvYAmTNtvML7vBrhBBhyePaEDVCCNaw");
    ASSERT_EQ((*AddressOut10.get()).index(), 9);

    // Check change addresses m / 0' / 1 / 2 ... m / 0' / 1 / 9
    const auto AddressIn3 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Alice), Identifier(AccountID), "Change 3", INTERNAL_CHAIN);
    EXPECT_STREQ(
        (*AddressIn3.get()).address().c_str(),
        "17jfyBx8ZHJ3DT9G2WehYEPKwT7Zv3kcLs");

    const auto AddressIn4 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Alice), Identifier(AccountID), "Change 4", INTERNAL_CHAIN);
    EXPECT_STREQ(
        (*AddressIn4.get()).address().c_str(),
        "15zErgibP264JkEMqihXQDp4Kb7vpvDpd5");

    const auto AddressIn5 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Alice), Identifier(AccountID), "Change 5", INTERNAL_CHAIN);
    EXPECT_STREQ(
        (*AddressIn5.get()).address().c_str(),
        "1KvRA5nngc4aA8y57A6TuS83Gud4xR5oPK");

    const auto AddressIn6 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Alice), Identifier(AccountID), "Change 6", INTERNAL_CHAIN);
    EXPECT_STREQ(
        (*AddressIn6.get()).address().c_str(),
        "14wC1Ph9z6S82QJA6yTaDaSZQjng9kDihT");

    const auto AddressIn7 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Alice), Identifier(AccountID), "Change 7", INTERNAL_CHAIN);
    EXPECT_STREQ(
        (*AddressIn7.get()).address().c_str(),
        "1FjW1pENbM6g5PAUpCdjQQykBYH6bzs5hU");

    const auto AddressIn8 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Alice), Identifier(AccountID), "Change 8", INTERNAL_CHAIN);
    EXPECT_STREQ(
        (*AddressIn8.get()).address().c_str(),
        "1Bt6BP3bXfRJbKUEFS15BrWa6Hca8G9W1L");

    const auto AddressIn9 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Alice), Identifier(AccountID), "Change 9", INTERNAL_CHAIN);
    EXPECT_STREQ(
        (*AddressIn9.get()).address().c_str(),
        "197TU7ptMMnhufMLFrY1o2Sgi5zcw2e3qv");

    const auto AddressIn10 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Alice), Identifier(AccountID), "Change 10", INTERNAL_CHAIN);
    EXPECT_STREQ(
        (*AddressIn10.get()).address().c_str(),
        "176aRLv3W94vyWPZDPY9csUrLNrqDFrzCs");
    ASSERT_EQ((*AddressIn10.get()).index(), 9);

    std::shared_ptr<proto::Bip44Account> AccountReloaded =
        OT::App().Blockchain().Account(Identifier(Alice), AccountID);
    ASSERT_EQ((*AccountReloaded.get()).internalindex(), 10);
    ASSERT_EQ((*AccountReloaded.get()).externalindex(), 10);
}

TEST_F(Test_AllocateAddress, testBip32_SeedB)
{
    // create account
    const auto Bob = opentxs::OT::App().API().Exec().CreateNymHD(
        proto::CITEMTYPE_INDIVIDUAL, "Bob", SeedB_, 0);

    // Check m / 0'
    const ConstNym NymB = opentxs::OT::App().Wallet().Nym(Identifier(Bob));
    proto::HDPath pathB;
    ASSERT_TRUE(NymB.get()->Path(pathB));
    ASSERT_EQ(
        0 | static_cast<std::uint32_t>(Bip32Child::HARDENED), pathB.child(1));

    OTIdentifier AccountID = OT::App().Blockchain().NewAccount(
        Identifier(Bob),
        BlockchainAccountType::BIP32,
        static_cast<proto::ContactItemType>(proto::CITEMTYPE_BTC));
    std::shared_ptr<proto::Bip44Account> Account =
        OT::App().Blockchain().Account(Identifier(Bob), AccountID);
    ASSERT_EQ((*Account.get()).internalindex(), 0);
    ASSERT_EQ((*Account.get()).externalindex(), 0);

    // Check deposit address m / 0' / 0 / 0
    const auto AddressOut = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Bob), Identifier(AccountID), "Deposit 1", EXTERNAL_CHAIN);
    EXPECT_STREQ(
        (*AddressOut.get()).address().c_str(),
        "1AngXb5xQoQ4nT8Bn6dDdr6AFS4yMZU2y");
    ASSERT_EQ((*AddressOut.get()).index(), 0);

    // Check change address m / 0' / 1 / 0

    const auto AddressIn = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Bob), Identifier(AccountID), "Change 1", INTERNAL_CHAIN);
    EXPECT_STREQ(
        (*AddressIn.get()).address().c_str(),
        "1GXj4LrpYKugu4ps7BvYHkUgJLErjBcZc");
    ASSERT_EQ((*AddressIn.get()).index(), 0);

    // Check deposit address m / 0' / 0 / 1

    const auto AddressOut2 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Bob), Identifier(AccountID), "Deposit 2", EXTERNAL_CHAIN);
    EXPECT_STREQ(
        (*AddressOut2.get()).address().c_str(),
        "1FQMy3HkD5C3gGZZHeeH9rjHgyqurxC44q");

    // Check change address m / 0' / 1 / 0

    const auto AddressIn2 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Bob), Identifier(AccountID), "Change 2", INTERNAL_CHAIN);
    EXPECT_STREQ(
        (*AddressIn2.get()).address().c_str(),
        "18yFFsUUe7ATjku2NfKizdnNfZGx99LmLJ");

    // Check deposit addresses m / 0' / 0 / 2 ... m / 0' / 0 / 9

    const auto Address3 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Bob), Identifier(AccountID), "Deposit 3", EXTERNAL_CHAIN);
    EXPECT_STREQ(
        (*Address3.get()).address().c_str(),
        "1APXZ5bCTbj2ZRV3ZHyAa59CmsXRP4HkTh");
    EXPECT_EQ((*Address3.get()).index(), 2);

    const auto Address4 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Bob), Identifier(AccountID), "Deposit 4", EXTERNAL_CHAIN);
    EXPECT_STREQ(
        (*Address4.get()).address().c_str(),
        "1M966pvtChYbceTsou73eB2hutwoZ7QtVv");

    const auto Address5 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Bob), Identifier(AccountID), "Deposit 5", EXTERNAL_CHAIN);
    EXPECT_STREQ(
        (*Address5.get()).address().c_str(),
        "1HcN6BWFZKLNEdBo15oUPQGXpDJ26SVKQE");

    const auto Address6 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Bob), Identifier(AccountID), "Deposit 6", EXTERNAL_CHAIN);
    EXPECT_STREQ(
        (*Address6.get()).address().c_str(),
        "1NcaLRLFr4edY4hUcR81aNMpveHaRqzxPR");

    const auto Address7 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Bob), Identifier(AccountID), "Deposit 7", EXTERNAL_CHAIN);
    EXPECT_STREQ(
        (*Address7.get()).address().c_str(),
        "1CT86ZmqRFZW57aztRscjWuzkhJjgHjiMS");

    const auto Address8 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Bob), Identifier(AccountID), "Deposit 8", EXTERNAL_CHAIN);
    EXPECT_STREQ(
        (*Address8.get()).address().c_str(),
        "1CXT6sU5s4mxP4UattFA6fGN7yW4dkkARn");

    const auto Address9 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Bob), Identifier(AccountID), "Deposit 9", EXTERNAL_CHAIN);
    EXPECT_STREQ(
        (*Address9.get()).address().c_str(),
        "12hwhKpxTyfiSGDdQw63SWVzefRuRxrFqb");

    const auto Address10 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Bob), Identifier(AccountID), "Deposit 10", EXTERNAL_CHAIN);
    EXPECT_STREQ(
        (*Address10.get()).address().c_str(),
        "18SRAzD6bZ2GsTK4J4RohhYneEyZAUvyqp");

    // Check change addresses m / 0' / 1 / 2 ... m / 0' / 1 / 9
    const auto AddressIn3 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Bob), Identifier(AccountID), "Change 3", INTERNAL_CHAIN);
    EXPECT_STREQ(
        (*AddressIn3.get()).address().c_str(),
        "19hDov3sMJdXkgrinhfD2seaKhcb6FiDKL");

    const auto AddressIn4 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Bob), Identifier(AccountID), "Change 4", INTERNAL_CHAIN);
    EXPECT_STREQ(
        (*AddressIn4.get()).address().c_str(),
        "1W9fEcakg5ZshPuAt5j2vTYkV6txNoiwq");

    const auto AddressIn5 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Bob), Identifier(AccountID), "Change 5", INTERNAL_CHAIN);
    EXPECT_STREQ(
        (*AddressIn5.get()).address().c_str(),
        "1EPTv3qdCJTbgqUZw83nUbjoKBmy4sHbhd");

    const auto AddressIn6 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Bob), Identifier(AccountID), "Change 6", INTERNAL_CHAIN);
    EXPECT_STREQ(
        (*AddressIn6.get()).address().c_str(),
        "17mcj9bmcuBfSZqc2mQnjLiT1mtPxGD1yu");

    const auto AddressIn7 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Bob), Identifier(AccountID), "Change 7", INTERNAL_CHAIN);
    EXPECT_STREQ(
        (*AddressIn7.get()).address().c_str(),
        "1LT2ZEnj1kmpgDbBQodiXVrAj6nRBmWUcH");

    const auto AddressIn8 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Bob), Identifier(AccountID), "Change 8", INTERNAL_CHAIN);
    EXPECT_STREQ(
        (*AddressIn8.get()).address().c_str(),
        "1HZmwsMWU87WFJxYDNQbnCW52KqUoLiCqZ");

    const auto AddressIn9 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Bob), Identifier(AccountID), "Change 9", INTERNAL_CHAIN);
    EXPECT_STREQ(
        (*AddressIn9.get()).address().c_str(),
        "16SdtUXrRey55j49Ae84YwVVNZXwGL2tLU");

    const auto AddressIn10 = opentxs::OT::App().Blockchain().AllocateAddress(
        Identifier(Bob), Identifier(AccountID), "Change 10", INTERNAL_CHAIN);
    EXPECT_STREQ(
        (*AddressIn10.get()).address().c_str(),
        "1N2Y3mM828N4JQGLzDfxNjU2WK9CMMekVg");
    ASSERT_EQ((*AddressIn10.get()).index(), 9);

    std::shared_ptr<proto::Bip44Account> AccountReloaded =
        OT::App().Blockchain().Account(Identifier(Bob), AccountID);
    ASSERT_EQ((*AccountReloaded.get()).internalindex(), 10);
    ASSERT_EQ((*AccountReloaded.get()).externalindex(), 10);
}

TEST_F(Test_AllocateAddress, testBip44_SeedC)
{
    const auto Charly = OT::App().API().Exec().CreateNymHD(
        opentxs::proto::CITEMTYPE_INDIVIDUAL, "Charly", SeedC_, 0);
    Identifier BTCAccountID = OT::App().Blockchain().NewAccount(
        Identifier(Charly),
        BlockchainAccountType::BIP44,
        static_cast<proto::ContactItemType>(proto::CITEMTYPE_BTC));

    Identifier BCHAccountID = OT::App().Blockchain().NewAccount(
        Identifier(Charly),
        BlockchainAccountType::BIP44,
        static_cast<proto::ContactItemType>(proto::CITEMTYPE_BCH));

    Identifier LTCAccountID = OT::App().Blockchain().NewAccount(
        Identifier(Charly),
        BlockchainAccountType::BIP44,
        static_cast<proto::ContactItemType>(proto::CITEMTYPE_LTC));

    // Check m / 44 ' / 0'
    const ConstNym NymC = opentxs::OT::App().Wallet().Nym(Identifier(Charly));
    proto::HDPath pathC;
    ASSERT_TRUE(NymC.get()->Path(pathC));
    ASSERT_EQ(
        0 | static_cast<std::uint32_t>(Bip32Child::HARDENED), pathC.child(1));

    std::shared_ptr<proto::Bip44Account> BTCAccount_ =
        OT::App().Blockchain().Account(Identifier(Charly), BTCAccountID);

    ASSERT_EQ((*BTCAccount_.get()).internalindex(), 0);
    ASSERT_EQ((*BTCAccount_.get()).externalindex(), 0);

    std::shared_ptr<proto::Bip44Account> LTCAccount_ =
        OT::App().Blockchain().Account(Identifier(Charly), LTCAccountID);

    ASSERT_EQ((*LTCAccount_.get()).internalindex(), 0);
    ASSERT_EQ((*LTCAccount_.get()).externalindex(), 0);

    // m / 44' / 0' / 0' / 0 / 0
    EXPECT_STREQ(
        opentxs::OT::App()
            .Blockchain()
            .AllocateAddress(
                Identifier(Charly),
                BTCAccountID,
                "BTC Deposit 1",
                EXTERNAL_CHAIN)
            .get()
            ->address()
            .c_str(),
        "1MWZN5PtYjfHA7WC1czB43HK9NjTKig1rA");
    // m / 44' / 0' / 0' / 1 / 0
    EXPECT_STREQ(
        opentxs::OT::App()
            .Blockchain()
            .AllocateAddress(
                Identifier(Charly),
                BTCAccountID,
                "BTC Change 1",
                INTERNAL_CHAIN)
            .get()
            ->address()
            .c_str(),
        "1PsjtCRUQ32t5F18W2K8Zzpn1aVmuRmTdB");

    // m / 44' / 0' / 0' / 0 / 1
    EXPECT_STREQ(
        opentxs::OT::App()
            .Blockchain()
            .AllocateAddress(
                Identifier(Charly),
                BTCAccountID,
                "BTC Deposit 1",
                EXTERNAL_CHAIN)
            .get()
            ->address()
            .c_str(),
        "16Ach28pUQbWDpVhe75AjwoCJws144Nd25");
    // m / 44' / 0' / 0' / 1 / 1
    EXPECT_STREQ(
        opentxs::OT::App()
            .Blockchain()
            .AllocateAddress(
                Identifier(Charly),
                BTCAccountID,
                "BTC Change 2",
                INTERNAL_CHAIN)
            .get()
            ->address()
            .c_str(),
        "15xi7Z3kVPg88ZYA82V8zPyodnQnamSZvN");

    // m / 44' / 145' / 0' / 0 / 0
    EXPECT_STREQ(
        opentxs::OT::App()
            .Blockchain()
            .AllocateAddress(
                Identifier(Charly),
                BCHAccountID,
                "BCH Deposit 1",
                EXTERNAL_CHAIN)
            .get()
            ->address()
            .c_str(),
        "14Et9A6QnwpnUH2Ym9kZ4Zz1FN2GixG9qS");
    // m / 44' / 145' / 0' / 1 / 0
    EXPECT_STREQ(
        opentxs::OT::App()
            .Blockchain()
            .AllocateAddress(
                Identifier(Charly),
                BCHAccountID,
                "BCH Change 1",
                INTERNAL_CHAIN)
            .get()
            ->address()
            .c_str(),
        "1FkAAgJWW1YWSqa5ByvHFe8dQvfNLT2rQN");

    // m / 44' / 145' / 0' / 0 / 1
    EXPECT_STREQ(
        opentxs::OT::App()
            .Blockchain()
            .AllocateAddress(
                Identifier(Charly),
                BCHAccountID,
                "BCH Deposit 2",
                EXTERNAL_CHAIN)
            .get()
            ->address()
            .c_str(),
        "17u11yKTfr13Xkm4k7h4bx3o3ssz4HSwGJ");
    // m / 44' / 145' / 0' / 1 / 1
    EXPECT_STREQ(
        opentxs::OT::App()
            .Blockchain()
            .AllocateAddress(
                Identifier(Charly),
                BCHAccountID,
                "BCH Change 2",
                INTERNAL_CHAIN)
            .get()
            ->address()
            .c_str(),
        "1HyweNdaw2QoRU1YfuJQWcZKUAVqMXyJsj");

    // m / 44' / 2' / 0' / 0 / 0
    EXPECT_STREQ(
        (opentxs::OT::App().Blockchain().AllocateAddress(
             Identifier(Charly), LTCAccountID, "LTC Deposit 1", EXTERNAL_CHAIN))
            .get()
            ->address()
            .c_str(),
        "LWDn8duKKwbP9hhCWpmX9o8BxywgCSTg41");
    // m / 44' / 2' / 0' / 1 / 0
    EXPECT_STREQ(
        (opentxs::OT::App().Blockchain().AllocateAddress(
             Identifier(Charly), LTCAccountID, "LTC Change 1", INTERNAL_CHAIN))
            .get()
            ->address()
            .c_str(),
        "LX3FAVopX2moW5h2ZwAKcrCKTChTyWqWze");

    // m / 44' / 2' / 0' / 0 / 1
    EXPECT_STREQ(
        (opentxs::OT::App().Blockchain().AllocateAddress(
             Identifier(Charly), LTCAccountID, "LTC Deposit 2", EXTERNAL_CHAIN))
            .get()
            ->address()
            .c_str(),
        "LSyrWGpCUm457F9TaXWAhvZs7Vu5g7a4Do");

    // m / 44' / 2' / 0' / 1 / 1
    EXPECT_STREQ(
        (opentxs::OT::App().Blockchain().AllocateAddress(
             Identifier(Charly), LTCAccountID, "LTC Change 2", INTERNAL_CHAIN))
            .get()
            ->address()
            .c_str(),
        "LMoZuWNnoTEJ1FjxQ4NXTcNbMK3croGpaF");
}
}  // namespace
