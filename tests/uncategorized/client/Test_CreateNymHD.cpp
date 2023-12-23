// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cassert>
#include <memory>

#include "ottest/fixtures/client/CreateNymHD.hpp"

namespace ottest
{
namespace ot = opentxs;

TEST_F(CreateNymHD, TestNym_DeterministicIDs)
{
    EXPECT_EQ(alice_, alice_expected_id_);
    EXPECT_EQ(bob_, bob_expected_id_);
}

TEST_F(CreateNymHD, TestNym_ABCD)
{
    const auto aliceID = api_.Factory().NymIDFromBase58(alice_);
    const auto bobID = api_.Factory().NymIDFromBase58(bob_);

    assert(false == aliceID.empty());
    assert(false == bobID.empty());

    const auto NymA = api_.Wallet().Nym(aliceID);
    const auto NymB = api_.Wallet().Nym(bobID);
    const auto NymC =
        api_.Wallet().Nym({api_.Factory(), seed_a_, 1}, reason_, "Charly");
    const auto NymD =
        api_.Wallet().Nym({api_.Factory(), seed_b_, 1}, reason_, "Dave");

    ASSERT_TRUE(NymA);
    ASSERT_TRUE(NymB);
    ASSERT_TRUE(NymC);
    ASSERT_TRUE(NymD);

    // Alice
    EXPECT_TRUE(NymA->HasPath());
    EXPECT_EQ(NymA->PathRoot(), seed_a_);
    EXPECT_EQ(2, NymA->PathChildSize());

    EXPECT_EQ(
        static_cast<ot::crypto::Bip32Index>(ot::crypto::Bip43Purpose::NYM) |
            static_cast<ot::crypto::Bip32Index>(
                ot::crypto::Bip32Child::HARDENED),
        NymA->PathChild(0));

    EXPECT_EQ(
        0 | static_cast<ot::crypto::Bip32Index>(
                ot::crypto::Bip32Child::HARDENED),
        NymA->PathChild(1));

    // Bob
    EXPECT_TRUE(NymB->HasPath());
    EXPECT_EQ(NymB->PathRoot(), seed_b_);
    EXPECT_EQ(2, NymB->PathChildSize());

    EXPECT_EQ(
        static_cast<ot::crypto::Bip32Index>(ot::crypto::Bip43Purpose::NYM) |
            static_cast<ot::crypto::Bip32Index>(
                ot::crypto::Bip32Child::HARDENED),
        NymB->PathChild(0));

    EXPECT_EQ(
        0 | static_cast<ot::crypto::Bip32Index>(
                ot::crypto::Bip32Child::HARDENED),
        NymB->PathChild(1));

    // Charly
    EXPECT_TRUE(NymC->HasPath());
    EXPECT_EQ(NymC->PathRoot(), seed_a_);
    EXPECT_EQ(2, NymC->PathChildSize());

    EXPECT_EQ(
        static_cast<ot::crypto::Bip32Index>(ot::crypto::Bip43Purpose::NYM) |
            static_cast<ot::crypto::Bip32Index>(
                ot::crypto::Bip32Child::HARDENED),
        NymC->PathChild(0));

    EXPECT_EQ(
        1 | static_cast<ot::crypto::Bip32Index>(
                ot::crypto::Bip32Child::HARDENED),
        NymC->PathChild(1));
}

TEST_F(CreateNymHD, TestNym_Dave)
{
    const auto NymD =
        api_.Wallet().Nym({api_.Factory(), seed_b_, 1}, reason_, "Dave");

    ASSERT_TRUE(NymD);

    EXPECT_TRUE(NymD->HasPath());
    EXPECT_EQ(NymD->PathRoot(), seed_b_);
    EXPECT_EQ(2, NymD->PathChildSize());

    EXPECT_EQ(
        static_cast<ot::crypto::Bip32Index>(ot::crypto::Bip43Purpose::NYM) |
            static_cast<ot::crypto::Bip32Index>(
                ot::crypto::Bip32Child::HARDENED),
        NymD->PathChild(0));

    EXPECT_EQ(
        1 | static_cast<ot::crypto::Bip32Index>(
                ot::crypto::Bip32Child::HARDENED),
        NymD->PathChild(1));
}

TEST_F(CreateNymHD, TestNym_Eve)
{
    const auto NymE =
        api_.Wallet().Nym({api_.Factory(), seed_b_, 2, 1}, reason_, "Eve");

    ASSERT_TRUE(NymE);

    EXPECT_EQ(eve_expected_id_, NymE->ID().asBase58(api_.Crypto()));

    EXPECT_TRUE(NymE->HasPath());
    EXPECT_EQ(NymE->PathRoot(), seed_b_);
    EXPECT_EQ(2, NymE->PathChildSize());

    EXPECT_EQ(
        static_cast<ot::crypto::Bip32Index>(ot::crypto::Bip43Purpose::NYM) |
            static_cast<ot::crypto::Bip32Index>(
                ot::crypto::Bip32Child::HARDENED),
        NymE->PathChild(0));

    EXPECT_EQ(
        2 | static_cast<ot::crypto::Bip32Index>(
                ot::crypto::Bip32Child::HARDENED),
        NymE->PathChild(1));
}

TEST_F(CreateNymHD, TestNym_Frank)
{
    const auto NymF =
        api_.Wallet().Nym({api_.Factory(), seed_b_, 3, 3}, reason_, "Frank");
    const auto NymF2 =
        api_.Wallet().Nym({api_.Factory(), seed_a_, 3, 3}, reason_, "Frank");

    EXPECT_NE(NymF->ID(), NymF2->ID());

    EXPECT_EQ(frank_expected_id_, NymF->ID().asBase58(api_.Crypto()));

    EXPECT_TRUE(NymF->HasPath());
    EXPECT_TRUE(NymF2->HasPath());

    EXPECT_EQ(NymF->PathRoot(), seed_b_);
    EXPECT_EQ(NymF2->PathRoot(), seed_a_);

    EXPECT_EQ(2, NymF->PathChildSize());
    EXPECT_EQ(2, NymF2->PathChildSize());

    EXPECT_EQ(
        static_cast<ot::crypto::Bip32Index>(ot::crypto::Bip43Purpose::NYM) |
            static_cast<ot::crypto::Bip32Index>(
                ot::crypto::Bip32Child::HARDENED),
        NymF->PathChild(0));

    EXPECT_EQ(
        3 | static_cast<ot::crypto::Bip32Index>(
                ot::crypto::Bip32Child::HARDENED),
        NymF->PathChild(1));
    EXPECT_EQ(
        3 | static_cast<ot::crypto::Bip32Index>(
                ot::crypto::Bip32Child::HARDENED),
        NymF2->PathChild(1));
}

TEST_F(CreateNymHD, TestNym_NonnegativeIndex)
{
    const auto Nym1 =
        api_.Wallet().Nym({api_.Factory(), seed_c_, 0}, reason_, "Nym1");
    const auto Nym2 =
        api_.Wallet().Nym({api_.Factory(), seed_c_, 0}, reason_, "Nym2");

    EXPECT_TRUE(Nym1->HasPath());
    EXPECT_TRUE(Nym2->HasPath());

    const auto nym1Index = Nym1->PathChild(1);
    const auto nym2Index = Nym2->PathChild(1);

    ASSERT_EQ(nym1Index, nym2Index);
}

TEST_F(CreateNymHD, TestNym_NegativeIndex)
{
    const auto Nym1 =
        api_.Wallet().Nym({api_.Factory(), seed_d_, -1}, reason_, "Nym1");
    const auto Nym2 =
        api_.Wallet().Nym({api_.Factory(), seed_d_, -1}, reason_, "Nym2");

    EXPECT_TRUE(Nym1->HasPath());
    EXPECT_TRUE(Nym2->HasPath());

    const auto nym1Index = Nym1->PathChild(1);
    const auto nym2Index = Nym2->PathChild(1);

    ASSERT_NE(nym1Index, nym2Index);
}
}  // namespace ottest
