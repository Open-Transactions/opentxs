// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/opentxs.hpp"

#include <gtest/gtest.h>

using namespace opentxs;

namespace
{

class Test_CreateNymHD : public ::testing::Test
{
public:
    const opentxs::api::client::Manager& client_;
    std::string SeedA_;
    std::string SeedB_;
    std::string SeedC_;
    std::string SeedD_;
    std::string AliceID, BobID, EveID, FrankID;
    std::string Alice, Bob;

    Test_CreateNymHD()
        : client_(opentxs::OT::App().StartClient({}, 0))
        // these fingerprints are deterministic so we can share them among tests
        , SeedA_(client_.Exec().Wallet_ImportSeed(
              "spike nominee miss inquiry fee nothing belt list other daughter "
              "leave valley twelve gossip paper",
              ""))
        , SeedB_(client_.Exec().Wallet_ImportSeed(
              "glimpse destroy nation advice seven useless candy move number "
              "toast insane anxiety proof enjoy lumber",
              ""))
        , SeedC_(client_.Exec().Wallet_ImportSeed("park cabbage quit", ""))
        , SeedD_(client_.Exec().Wallet_ImportSeed("federal dilemma rare", ""))
        , Alice(
              client_.Exec()
                  .CreateNymHD(proto::CITEMTYPE_INDIVIDUAL, "Alice", SeedA_, 0))
        , AliceID("ot24XFA1wKynjaAB59dx7PwEzGg37U8Q2yXG")
        , Bob(client_.Exec()
                  .CreateNymHD(proto::CITEMTYPE_INDIVIDUAL, "Bob", SeedB_, 0))
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
    auto Charly = client_.Exec().CreateNymHD(
        proto::CITEMTYPE_INDIVIDUAL, "Charly", SeedA_, 1);
    auto Dave = client_.Exec().CreateNymHD(
        proto::CITEMTYPE_INDIVIDUAL, "Dave", SeedB_, 1);

    const ConstNym NymA = client_.Wallet().Nym(identifier::Nym::Factory(Alice));
    const ConstNym NymB = client_.Wallet().Nym(identifier::Nym::Factory(Bob));
    const ConstNym NymC =
        client_.Wallet().Nym(identifier::Nym::Factory(Charly));
    const ConstNym NymD = client_.Wallet().Nym(identifier::Nym::Factory(Dave));

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
    const auto Dave = client_.Exec().CreateNymHD(
        proto::CITEMTYPE_INDIVIDUAL, "Dave", SeedB_, 1);
    const ConstNym NymD = client_.Wallet().Nym(identifier::Nym::Factory(Dave));

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
    auto NewEve = client_.Exec().CreateNymHD(
        proto::CITEMTYPE_INDIVIDUAL, "Eve", SeedB_, 2);
    EXPECT_STREQ(EveID.c_str(), NewEve.c_str());

    const ConstNym NymE =
        client_.Wallet().Nym(identifier::Nym::Factory(NewEve));

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
    auto Frank = client_.Exec().CreateNymHD(
        proto::CITEMTYPE_INDIVIDUAL, "Frank", SeedB_, 3);
    auto Frank2 = client_.Exec().CreateNymHD(
        proto::CITEMTYPE_INDIVIDUAL, "Frank", SeedA_, 3);

    EXPECT_STRNE(Frank.c_str(), Frank2.c_str());
    EXPECT_STREQ(FrankID.c_str(), Frank.c_str());

    const ConstNym NymF = client_.Wallet().Nym(identifier::Nym::Factory(Frank));
    const ConstNym NymF2 =
        client_.Wallet().Nym(identifier::Nym::Factory(Frank2));

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

TEST_F(Test_CreateNymHD, TestNym_NonnegativeIndex)
{
    auto NymID1 = client_.Exec().CreateNymHD(
        proto::CITEMTYPE_INDIVIDUAL, "Nym1", SeedC_, 0);

    auto NymID2 = client_.Exec().CreateNymHD(
        proto::CITEMTYPE_INDIVIDUAL, "Nym2", SeedC_, 0);

    const ConstNym Nym1 =
        client_.Wallet().Nym(identifier::Nym::Factory(NymID1));
    const ConstNym Nym2 =
        client_.Wallet().Nym(identifier::Nym::Factory(NymID2));

    proto::HDPath path1, path2;
    EXPECT_TRUE(Nym1.get()->Path(path1));
    EXPECT_TRUE(Nym2.get()->Path(path2));

    const auto nym1Index = path1.child(1);
    const auto nym2Index = path2.child(1);

    ASSERT_EQ(nym1Index, nym2Index);
}

TEST_F(Test_CreateNymHD, TestNym_NegativeIndex)
{
    auto NymID1 = client_.Exec().CreateNymHD(
        proto::CITEMTYPE_INDIVIDUAL, "Nym1", SeedD_, -1);

    auto NymID2 = client_.Exec().CreateNymHD(
        proto::CITEMTYPE_INDIVIDUAL, "Nym2", SeedD_, -1);

    const ConstNym Nym1 =
        client_.Wallet().Nym(identifier::Nym::Factory(NymID1));
    const ConstNym Nym2 =
        client_.Wallet().Nym(identifier::Nym::Factory(NymID2));

    proto::HDPath path1, path2;
    EXPECT_TRUE(Nym1.get()->Path(path1));
    EXPECT_TRUE(Nym2.get()->Path(path2));

    const auto nym1Index = path1.child(1);
    const auto nym2Index = path2.child(1);

    ASSERT_NE(nym1Index, nym2Index);
}

}  // namespace
