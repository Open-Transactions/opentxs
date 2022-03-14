// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <cassert>
#include <memory>

#include "internal/api/session/Client.hpp"
#include "internal/otx/client/obsolete/OTAPI_Exec.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/crypto/Bip32Child.hpp"
#include "opentxs/crypto/Bip43Purpose.hpp"
#include "opentxs/crypto/Parameters.hpp"  // IWYU pragma: keep
#include "opentxs/crypto/Types.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/PasswordPrompt.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace ot = opentxs;

namespace ottest
{
class Test_CreateNymHD : public ::testing::Test
{
public:
    static constexpr auto alice_expected_id_{
        "ot2xuVaYSXY9rZsm9oFHHPjy9uDrCJPG3DNyExF4uUHV2aCLuCvS8f3"};
    static constexpr auto bob_expected_id_{
        "ot2xuVSVeLyKCRrEpjPPwdra3UgwrZVLfhvxnZnA8oiy5FwiDDEW1KU"};
    static constexpr auto eve_expected_id_{
        "ot2xuVVxcrbQ8SsU3YQ2K6uEsTrF3xbLfwNq6f3BH5aUEXWSPuaANe9"};
    static constexpr auto frank_expected_id_{
        "ot2xuVYn8io5LpjK7itnUT7ujx8n5Rt3GKs5xXeh9nfZja2SwB5jEq6"};

    const ot::api::session::Client& api_;
    ot::OTPasswordPrompt reason_;
    ot::UnallocatedCString SeedA_;
    ot::UnallocatedCString SeedB_;
    ot::UnallocatedCString SeedC_;
    ot::UnallocatedCString SeedD_;
    ot::UnallocatedCString Alice, Bob;

    Test_CreateNymHD()
        : api_(ot::Context().StartClientSession(0))
        , reason_(api_.Factory().PasswordPrompt(__func__))
        // these fingerprints are deterministic so we can share them among tests
        , SeedA_(api_.InternalClient().Exec().Wallet_ImportSeed(
              "spike nominee miss inquiry fee nothing belt list other daughter "
              "leave valley twelve gossip paper",
              ""))
        , SeedB_(api_.InternalClient().Exec().Wallet_ImportSeed(
              "glimpse destroy nation advice seven useless candy move number "
              "toast insane anxiety proof enjoy lumber",
              ""))
        , SeedC_(api_.InternalClient().Exec().Wallet_ImportSeed(
              "park cabbage quit",
              ""))
        , SeedD_(api_.InternalClient().Exec().Wallet_ImportSeed(
              "federal dilemma rare",
              ""))
        , Alice(api_.Wallet().Nym({SeedA_, 0, 1}, reason_, "Alice")->ID().str())
        , Bob(api_.Wallet().Nym({SeedB_, 0, 1}, reason_, "Bob")->ID().str())
    {
        assert(false == Alice.empty());
        assert(false == Bob.empty());
    }
};

TEST_F(Test_CreateNymHD, TestNym_DeterministicIDs)
{
    EXPECT_EQ(Alice, alice_expected_id_);
    EXPECT_EQ(Bob, bob_expected_id_);
}

TEST_F(Test_CreateNymHD, TestNym_ABCD)
{
    const auto aliceID = api_.Factory().NymID(Alice);
    const auto bobID = api_.Factory().NymID(Bob);

    assert(false == aliceID->empty());
    assert(false == bobID->empty());

    const auto NymA = api_.Wallet().Nym(aliceID);
    const auto NymB = api_.Wallet().Nym(bobID);
    const auto NymC = api_.Wallet().Nym({SeedA_, 1}, reason_, "Charly");
    const auto NymD = api_.Wallet().Nym({SeedB_, 1}, reason_, "Dave");

    ASSERT_TRUE(NymA);
    ASSERT_TRUE(NymB);
    ASSERT_TRUE(NymC);
    ASSERT_TRUE(NymD);

    // Alice
    EXPECT_TRUE(NymA->HasPath());
    EXPECT_STREQ(NymA->PathRoot().c_str(), SeedA_.c_str());
    EXPECT_EQ(2, NymA->PathChildSize());

    EXPECT_EQ(
        static_cast<ot::Bip32Index>(ot::Bip43Purpose::NYM) |
            static_cast<ot::Bip32Index>(ot::Bip32Child::HARDENED),
        NymA->PathChild(0));

    EXPECT_EQ(
        0 | static_cast<ot::Bip32Index>(ot::Bip32Child::HARDENED),
        NymA->PathChild(1));

    // Bob
    EXPECT_TRUE(NymB->HasPath());
    EXPECT_STREQ(NymB->PathRoot().c_str(), SeedB_.c_str());
    EXPECT_EQ(2, NymB->PathChildSize());

    EXPECT_EQ(
        static_cast<ot::Bip32Index>(ot::Bip43Purpose::NYM) |
            static_cast<ot::Bip32Index>(ot::Bip32Child::HARDENED),
        NymB->PathChild(0));

    EXPECT_EQ(
        0 | static_cast<ot::Bip32Index>(ot::Bip32Child::HARDENED),
        NymB->PathChild(1));

    // Charly
    EXPECT_TRUE(NymC->HasPath());
    EXPECT_STREQ(NymC->PathRoot().c_str(), SeedA_.c_str());
    EXPECT_EQ(2, NymC->PathChildSize());

    EXPECT_EQ(
        static_cast<ot::Bip32Index>(ot::Bip43Purpose::NYM) |
            static_cast<ot::Bip32Index>(ot::Bip32Child::HARDENED),
        NymC->PathChild(0));

    EXPECT_EQ(
        1 | static_cast<ot::Bip32Index>(ot::Bip32Child::HARDENED),
        NymC->PathChild(1));
}

TEST_F(Test_CreateNymHD, TestNym_Dave)
{
    const auto NymD = api_.Wallet().Nym({SeedB_, 1}, reason_, "Dave");

    ASSERT_TRUE(NymD);

    EXPECT_TRUE(NymD->HasPath());
    EXPECT_STREQ(NymD->PathRoot().c_str(), SeedB_.c_str());
    EXPECT_EQ(2, NymD->PathChildSize());

    EXPECT_EQ(
        static_cast<ot::Bip32Index>(ot::Bip43Purpose::NYM) |
            static_cast<ot::Bip32Index>(ot::Bip32Child::HARDENED),
        NymD->PathChild(0));

    EXPECT_EQ(
        1 | static_cast<ot::Bip32Index>(ot::Bip32Child::HARDENED),
        NymD->PathChild(1));
}

TEST_F(Test_CreateNymHD, TestNym_Eve)
{
    const auto NymE = api_.Wallet().Nym({SeedB_, 2, 1}, reason_, "Eve");

    ASSERT_TRUE(NymE);

    EXPECT_EQ(eve_expected_id_, NymE->ID().str());

    EXPECT_TRUE(NymE->HasPath());
    EXPECT_STREQ(NymE->PathRoot().c_str(), SeedB_.c_str());
    EXPECT_EQ(2, NymE->PathChildSize());

    EXPECT_EQ(
        static_cast<ot::Bip32Index>(ot::Bip43Purpose::NYM) |
            static_cast<ot::Bip32Index>(ot::Bip32Child::HARDENED),
        NymE->PathChild(0));

    EXPECT_EQ(
        2 | static_cast<ot::Bip32Index>(ot::Bip32Child::HARDENED),
        NymE->PathChild(1));
}

TEST_F(Test_CreateNymHD, TestNym_Frank)
{
    const auto NymF = api_.Wallet().Nym({SeedB_, 3, 3}, reason_, "Frank");
    const auto NymF2 = api_.Wallet().Nym({SeedA_, 3, 3}, reason_, "Frank");

    EXPECT_NE(NymF->ID(), NymF2->ID());

    EXPECT_EQ(frank_expected_id_, NymF->ID().str());

    EXPECT_TRUE(NymF->HasPath());
    EXPECT_TRUE(NymF2->HasPath());

    EXPECT_STREQ(NymF->PathRoot().c_str(), SeedB_.c_str());
    EXPECT_STREQ(NymF2->PathRoot().c_str(), SeedA_.c_str());

    EXPECT_EQ(2, NymF->PathChildSize());
    EXPECT_EQ(2, NymF2->PathChildSize());

    EXPECT_EQ(
        static_cast<ot::Bip32Index>(ot::Bip43Purpose::NYM) |
            static_cast<ot::Bip32Index>(ot::Bip32Child::HARDENED),
        NymF->PathChild(0));

    EXPECT_EQ(
        3 | static_cast<ot::Bip32Index>(ot::Bip32Child::HARDENED),
        NymF->PathChild(1));
    EXPECT_EQ(
        3 | static_cast<ot::Bip32Index>(ot::Bip32Child::HARDENED),
        NymF2->PathChild(1));
}

TEST_F(Test_CreateNymHD, TestNym_NonnegativeIndex)
{
    const auto Nym1 = api_.Wallet().Nym({SeedC_, 0}, reason_, "Nym1");
    const auto Nym2 = api_.Wallet().Nym({SeedC_, 0}, reason_, "Nym2");

    EXPECT_TRUE(Nym1->HasPath());
    EXPECT_TRUE(Nym2->HasPath());

    const auto nym1Index = Nym1->PathChild(1);
    const auto nym2Index = Nym2->PathChild(1);

    ASSERT_EQ(nym1Index, nym2Index);
}

TEST_F(Test_CreateNymHD, TestNym_NegativeIndex)
{
    const auto Nym1 = api_.Wallet().Nym({SeedD_, -1}, reason_, "Nym1");
    const auto Nym2 = api_.Wallet().Nym({SeedD_, -1}, reason_, "Nym2");

    EXPECT_TRUE(Nym1->HasPath());
    EXPECT_TRUE(Nym2->HasPath());

    const auto nym1Index = Nym1->PathChild(1);
    const auto nym2Index = Nym2->PathChild(1);

    ASSERT_NE(nym1Index, nym2Index);
}
}  // namespace ottest
