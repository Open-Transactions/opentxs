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

class Test_CreateNymHD : public ::testing::Test
{
public:
    std::string SeedA_;
    std::string SeedB_;
    std::string AliceID, BobID, EveID, FrankID;
    std::string Alice, Bob;
  
    Test_CreateNymHD()
        // these fingerprints are deterministic so we can share them among tests
        : SeedA_(opentxs::OT::App().API().Exec().Wallet_ImportSeed(
              "spike nominee miss inquiry fee nothing belt list other daughter "
              "leave valley twelve gossip paper",
              ""))
        , SeedB_(opentxs::OT::App().API().Exec().Wallet_ImportSeed(
              "glimpse destroy nation advice seven useless candy move number "
              "toast insane anxiety proof enjoy lumber",
              ""))
        ,
        Alice(opentxs::OT::App().API().Exec().CreateNymHD(proto::CITEMTYPE_INDIVIDUAL,
                                                          "Alice", SeedA_, 0))
        , AliceID("ot24XFA1wKynjaAB59dx7PwEzGg37U8Q2yXG")
        ,
        Bob(opentxs::OT::App().API().Exec().CreateNymHD(proto::CITEMTYPE_INDIVIDUAL,
                                                        "Bob", SeedB_, 0))
        , BobID("ot274uRuN1VezD47R7SqAH27s2WKP1U5jKWk")
        , EveID("otwz4jCuiVg7UF2i1NgCSvTWeDS29EAHeL6")
        , FrankID("ot2BqchYuY5r747PnGK3SuM4A8bCLtuGASqY")
    {
    }
};

TEST_F(Test_CreateNymHD, TestNym_DeterministicIDs)
{
  
  EXPECT_STREQ(AliceID.c_str(), Alice.c_str());
  EXPECT_STREQ(BobID.c_str(), Bob.c_str());
}

TEST_F(Test_CreateNymHD, TestNym_ABCD)
{
    auto Charly = opentxs::OT::App().API().Exec().CreateNymHD(
        proto::CITEMTYPE_INDIVIDUAL, "Charly", SeedA_, 1);
    auto Dave = opentxs::OT::App().API().Exec().CreateNymHD(
        proto::CITEMTYPE_INDIVIDUAL, "Dave", SeedB_, 1);

    const ConstNym NymA = opentxs::OT::App().Wallet().Nym(Identifier(Alice));
    const ConstNym NymB = opentxs::OT::App().Wallet().Nym(Identifier(Bob));
    const ConstNym NymC = opentxs::OT::App().Wallet().Nym(Identifier(Charly));
    const ConstNym NymD = opentxs::OT::App().Wallet().Nym(Identifier(Dave));

    // Alice
    proto::HDPath pathA;
    EXPECT_TRUE(NymA.get()->Path(pathA));
    EXPECT_STREQ(pathA.root().c_str(), SeedA_.c_str());
    EXPECT_EQ(2, pathA.child_size());

    EXPECT_EQ(
        static_cast<std::uint32_t>(Bip43Purpose::NYM) |
            static_cast<std::uint32_t>(Bip32Child::HARDENED),
        pathA.child(0));

    EXPECT_EQ(
        0 | static_cast<std::uint32_t>(Bip32Child::HARDENED), pathA.child(1));

    // Bob
    proto::HDPath pathB;
    EXPECT_TRUE(NymB.get()->Path(pathB));
    EXPECT_STREQ(pathB.root().c_str(), SeedB_.c_str());
    EXPECT_EQ(2, pathB.child_size());

    EXPECT_EQ(
        static_cast<std::uint32_t>(Bip43Purpose::NYM) |
            static_cast<std::uint32_t>(Bip32Child::HARDENED),
        pathB.child(0));

    EXPECT_EQ(
        0 | static_cast<std::uint32_t>(Bip32Child::HARDENED), pathB.child(1));

    // Charly
    proto::HDPath pathC;
    EXPECT_TRUE(NymC.get()->Path(pathC));
    EXPECT_STREQ(pathC.root().c_str(), SeedA_.c_str());
    EXPECT_EQ(2, pathC.child_size());

    EXPECT_EQ(
        static_cast<std::uint32_t>(Bip43Purpose::NYM) |
            static_cast<std::uint32_t>(Bip32Child::HARDENED),
        pathC.child(0));

    EXPECT_EQ(
        1 | static_cast<std::uint32_t>(Bip32Child::HARDENED), pathC.child(1));
}

TEST_F(Test_CreateNymHD, TestNym_Dave)
{
    const auto Dave = opentxs::OT::App().API().Exec().CreateNymHD(
        proto::CITEMTYPE_INDIVIDUAL, "Dave", SeedB_, 1);
    const ConstNym NymD = opentxs::OT::App().Wallet().Nym(Identifier(Dave));

    proto::HDPath pathD;
    EXPECT_TRUE(NymD.get()->Path(pathD));
    EXPECT_STREQ(pathD.root().c_str(), SeedB_.c_str());
    EXPECT_EQ(2, pathD.child_size());

    EXPECT_EQ(
        static_cast<std::uint32_t>(Bip43Purpose::NYM) |
            static_cast<std::uint32_t>(Bip32Child::HARDENED),
        pathD.child(0));

    EXPECT_EQ(
        1 | static_cast<std::uint32_t>(Bip32Child::HARDENED), pathD.child(1));
}

TEST_F(Test_CreateNymHD, TestNym_Eve)
{

    // EXPECT_STREQ(EveID.c_str(), Eve.c_str());
    auto NewEve = opentxs::OT::App().API().Exec().CreateNymHD(
        proto::CITEMTYPE_INDIVIDUAL, "Eve", SeedB_, 2);
    EXPECT_STREQ(EveID.c_str(), NewEve.c_str());

    const ConstNym NymE = opentxs::OT::App().Wallet().Nym(Identifier(NewEve));

    proto::HDPath pathE;
    EXPECT_TRUE(NymE.get()->Path(pathE));
    EXPECT_STREQ(pathE.root().c_str(), SeedB_.c_str());
    EXPECT_EQ(2, pathE.child_size());

    EXPECT_EQ(
        static_cast<std::uint32_t>(Bip43Purpose::NYM) |
            static_cast<std::uint32_t>(Bip32Child::HARDENED),
        pathE.child(0));

    EXPECT_EQ(
        2 | static_cast<std::uint32_t>(Bip32Child::HARDENED), pathE.child(1));
}

TEST_F(Test_CreateNymHD, TestNym_Frank)
{
    auto Frank = opentxs::OT::App().API().Exec().CreateNymHD(
        proto::CITEMTYPE_INDIVIDUAL, "Frank", SeedB_, 3);
    auto Frank2 = opentxs::OT::App().API().Exec().CreateNymHD(
        proto::CITEMTYPE_INDIVIDUAL, "Frank", SeedA_, 3);

    EXPECT_STRNE(Frank.c_str(), Frank2.c_str());
    EXPECT_STREQ(FrankID.c_str(), Frank.c_str());

    const ConstNym NymF = opentxs::OT::App().Wallet().Nym(Identifier(Frank));
    const ConstNym NymF2 = opentxs::OT::App().Wallet().Nym(Identifier(Frank2));

    proto::HDPath pathF, pathF2;
    EXPECT_TRUE(NymF.get()->Path(pathF));
    EXPECT_TRUE(NymF2.get()->Path(pathF2));

    EXPECT_STREQ(pathF.root().c_str(), SeedB_.c_str());
    EXPECT_STREQ(pathF2.root().c_str(), SeedA_.c_str());

    EXPECT_EQ(2, pathF.child_size());
    EXPECT_EQ(2, pathF2.child_size());

    EXPECT_EQ(
        static_cast<std::uint32_t>(Bip43Purpose::NYM) |
            static_cast<std::uint32_t>(Bip32Child::HARDENED),
        pathF.child(0));

    EXPECT_EQ(
        3 | static_cast<std::uint32_t>(Bip32Child::HARDENED), pathF.child(1));
    EXPECT_EQ(
        3 | static_cast<std::uint32_t>(Bip32Child::HARDENED), pathF2.child(1));
}

}  // namespace
