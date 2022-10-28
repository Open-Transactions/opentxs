// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstddef>
#include <memory>
#include <optional>
#include <utility>

#include "ottest/fixtures/blind/Lucre.hpp"

namespace ottest
{
TEST_F(Lucre, generateMint) { EXPECT_TRUE(GenerateMint()); }

TEST_F(Lucre, requestPurse)
{
    ASSERT_TRUE(alice_);
    ASSERT_TRUE(bob_);
    ASSERT_TRUE(mint_);
    ASSERT_TRUE(NewPurse());

    auto& purse = request_purse_.value();

    ASSERT_TRUE(purse.IsUnlocked());
    EXPECT_EQ(
        ot::Clock::to_time_t(purse.EarliestValidTo()),
        ot::Clock::to_time_t(valid_to_));
    EXPECT_EQ(
        ot::Clock::to_time_t(purse.LatestValidFrom()),
        ot::Clock::to_time_t(valid_from_));
    EXPECT_EQ(server_id_, purse.Notary());
    EXPECT_EQ(purse.State(), ot::otx::blind::PurseType::Request);
    EXPECT_EQ(purse.Type(), ot::otx::blind::CashType::Lucre);
    EXPECT_EQ(unit_id_, purse.Unit());
    EXPECT_EQ(purse.Value(), REQUEST_PURSE_VALUE);
    ASSERT_EQ(purse.size(), 2);

    auto& token1 = purse.at(0);

    EXPECT_EQ(server_id_, token1.Notary());
    EXPECT_EQ(token1.Series(), 0);
    EXPECT_EQ(token1.State(), ot::otx::blind::TokenState::Blinded);
    EXPECT_EQ(token1.Type(), ot::otx::blind::CashType::Lucre);
    EXPECT_EQ(unit_id_, token1.Unit());
    EXPECT_EQ(
        ot::Clock::to_time_t(token1.ValidFrom()),
        ot::Clock::to_time_t(valid_from_));
    EXPECT_EQ(
        ot::Clock::to_time_t(token1.ValidTo()),
        ot::Clock::to_time_t(valid_to_));
    EXPECT_EQ(token1.Value(), 10000);

    auto& token2 = purse.at(1);

    EXPECT_EQ(server_id_, token2.Notary());
    EXPECT_EQ(token2.Series(), 0);
    EXPECT_EQ(token2.State(), ot::otx::blind::TokenState::Blinded);
    EXPECT_EQ(token2.Type(), ot::otx::blind::CashType::Lucre);
    EXPECT_EQ(unit_id_, token2.Unit());
    EXPECT_EQ(
        ot::Clock::to_time_t(token2.ValidFrom()),
        ot::Clock::to_time_t(valid_from_));
    EXPECT_EQ(
        ot::Clock::to_time_t(token2.ValidTo()),
        ot::Clock::to_time_t(valid_to_));
    EXPECT_EQ(token2.Value(), 10000);
}

TEST_F(Lucre, serialize_deserialize)
{
    ASSERT_TRUE(request_purse_);

    request_purse_->Serialize(opentxs::writer(serialized_bytes_));

    auto restored = DeserializePurse();

    ASSERT_TRUE(restored);

    EXPECT_EQ(
        ot::Clock::to_time_t(request_purse_->EarliestValidTo()),
        ot::Clock::to_time_t(restored.EarliestValidTo()));
    EXPECT_EQ(
        ot::Clock::to_time_t(request_purse_->LatestValidFrom()),
        ot::Clock::to_time_t(restored.LatestValidFrom()));
    EXPECT_EQ(request_purse_->Notary(), restored.Notary());
    EXPECT_EQ(request_purse_->State(), restored.State());
    EXPECT_EQ(request_purse_->Type(), restored.Type());
    EXPECT_EQ(request_purse_->Unit(), restored.Unit());
    EXPECT_EQ(request_purse_->Value(), restored.Value());

    EXPECT_EQ(2, restored.size());

    for (std::size_t i = 0; i < restored.size(); ++i) {
        auto& token_a = request_purse_->at(i);
        auto& token_b = restored.at(i);

        EXPECT_EQ(token_a.Notary(), token_b.Notary());
        EXPECT_EQ(token_a.Series(), token_b.Series());
        EXPECT_EQ(token_a.State(), token_b.State());
        EXPECT_EQ(token_a.Type(), token_b.Type());
        EXPECT_EQ(token_a.Unit(), token_b.Unit());
        EXPECT_EQ(
            ot::Clock::to_time_t(token_a.ValidFrom()),
            ot::Clock::to_time_t(token_b.ValidFrom()));
        EXPECT_EQ(
            ot::Clock::to_time_t(token_a.ValidTo()),
            ot::Clock::to_time_t(token_b.ValidTo()));
        EXPECT_EQ(token_a.Value(), token_b.Value());
    }
}

TEST_F(Lucre, sign)
{
    auto requestPurse = DeserializePurse();

    ASSERT_TRUE(requestPurse);

    ASSERT_TRUE(mint_);
    ASSERT_TRUE(alice_);
    ASSERT_TRUE(bob_);

    const auto& bob = *bob_;

    EXPECT_TRUE(requestPurse.Unlock(bob, reason_));
    ASSERT_TRUE(requestPurse.IsUnlocked());
    ASSERT_TRUE(IssuePurse(requestPurse));

    auto& issuePurse = *issue_purse_;

    EXPECT_TRUE(issuePurse.IsUnlocked());

    auto token = requestPurse.Pop();
    const auto added = issuePurse.AddNym(bob, reason_);

    EXPECT_TRUE(added);

    while (token) {
        EXPECT_TRUE(Sign(token));
        EXPECT_TRUE(ot::otx::blind::TokenState::Signed == token.State());

        const auto push = issuePurse.Push(std::move(token), reason_);

        EXPECT_TRUE(push);

        token = requestPurse.Pop();
    }

    EXPECT_TRUE(ot::otx::blind::PurseType::Issue == issuePurse.State());
    EXPECT_EQ(
        issuePurse.Notary().asBase58(api_.Crypto()),
        requestPurse.Notary().asBase58(api_.Crypto()));
    EXPECT_EQ(issuePurse.Type(), requestPurse.Type());
    EXPECT_EQ(
        issuePurse.Unit().asBase58(api_.Crypto()),
        requestPurse.Unit().asBase58(api_.Crypto()));
    EXPECT_EQ(issuePurse.Value(), REQUEST_PURSE_VALUE);
    EXPECT_EQ(requestPurse.Value(), 0);
}

TEST_F(Lucre, process)
{
    ASSERT_TRUE(issue_purse_);
    ASSERT_TRUE(mint_);
    ASSERT_TRUE(alice_);

    auto& issuePurse = *issue_purse_;

    auto bytes = ot::Space{};
    issuePurse.Serialize(opentxs::writer(bytes));
    auto purse = DeserializePurse(ot::reader(bytes));

    ASSERT_TRUE(purse);

    const auto& alice = *alice_;

    EXPECT_TRUE(purse.Unlock(alice, reason_));
    ASSERT_TRUE(purse.IsUnlocked());
    EXPECT_TRUE(Process(purse));
    EXPECT_TRUE(ot::otx::blind::PurseType::Normal == purse.State());

    issue_purse_.emplace(std::move(purse));
}

TEST_F(Lucre, verify)
{
    ASSERT_TRUE(issue_purse_);
    ASSERT_TRUE(mint_);
    ASSERT_TRUE(bob_);

    auto& issuePurse = *issue_purse_;
    const auto& bob = *bob_;

    EXPECT_TRUE(issuePurse.Unlock(bob, reason_));
    ASSERT_TRUE(issuePurse.IsUnlocked());

    auto bytes = ot::Space{};
    issuePurse.Serialize(opentxs::writer(bytes));
    auto purse = DeserializePurse(ot::reader(bytes));

    ASSERT_TRUE(purse);
    EXPECT_TRUE(purse.Unlock(bob, reason_));
    ASSERT_TRUE(purse.IsUnlocked());

    for (const auto& token : purse) {
        EXPECT_FALSE(token.IsSpent(reason_));
        EXPECT_TRUE(Verify(token));
    }

    issue_purse_.emplace(std::move(purse));
}

TEST_F(Lucre, wallet)
{
    {
        const auto& purse =
            api_.Wallet().Purse(alice_nym_id_, server_id_, unit_id_, true);

        EXPECT_FALSE(purse);
    }

    MakePurse();

    {
        const auto& purse =
            api_.Wallet().Purse(alice_nym_id_, server_id_, unit_id_, false);

        EXPECT_TRUE(purse);
    }
}

TEST_F(Lucre, PushPop) { EXPECT_TRUE(ReceivePurse()); }
}  // namespace ottest
