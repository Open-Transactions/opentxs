// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <optional>
#include <utility>

#include "ottest/Basic.hpp"
#include "ottest/data/crypto/PaymentCodeV3.hpp"

namespace ottest
{
auto id_ = std::optional<opentxs::crypto::SeedID>{std::nullopt};

TEST(HDSeed, create_seed)
{
    const auto& otx = ot::InitContext(Args(true));
    const auto& api = otx.StartClientSession(0);
    const auto& factory = api.Factory();
    const auto& crypto = api.Crypto().Seed();
    const auto reason = factory.PasswordPrompt(__func__);
    const auto& alex = GetPaymentCodeVector3().alice_;

    ASSERT_FALSE(id_.has_value());

    id_.emplace(crypto.ImportSeed(
        factory.SecretFromText(alex.words_),
        factory.SecretFromText(""),
        opentxs::crypto::SeedStyle::BIP39,
        opentxs::crypto::Language::en,
        reason));

    ASSERT_TRUE(id_.has_value());

    const auto& seedID = *id_;

    EXPECT_EQ(seedID, crypto.DefaultSeed().first);
    EXPECT_EQ(crypto.Words(seedID, reason), alex.words_);

    ot::Cleanup();
}

TEST(HDSeed, restart)
{
    const auto& otx = ot::InitContext(Args(true));
    const auto& api = otx.StartClientSession(0);
    const auto& factory = api.Factory();
    const auto& crypto = api.Crypto().Seed();
    const auto reason = factory.PasswordPrompt(__func__);
    const auto& alex = GetPaymentCodeVector3().alice_;

    ASSERT_TRUE(id_.has_value());

    const auto& seedID = *id_;

    EXPECT_EQ(seedID, crypto.DefaultSeed().first);
    EXPECT_EQ(crypto.Words(seedID, reason), alex.words_);

    ot::Cleanup();
}
}  // namespace ottest
