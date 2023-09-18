// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <memory>

#include "ottest/data/crypto/PaymentCodeV3.hpp"
#include "ottest/fixtures/paymentcode/PaymentCodeAPI.hpp"

namespace ottest
{
namespace ot = opentxs;

TEST_F(PaymentCodeAPI, init) {}

TEST_F(PaymentCodeAPI, alice)
{
    const auto& vector = GetPaymentCodeVector3().alice_;
    const auto& remote = GetPaymentCodeVector3().bob_;
    constexpr auto receiveChain{ot::blockchain::Type::Bitcoin};
    constexpr auto sendChain{ot::blockchain::Type::Bitcoin_testnet3};
    const auto reason = alice_.Factory().PasswordPrompt(__func__);
    const auto seedID = [&] {
        const auto words = alice_.Factory().SecretFromText(vector.words_);
        const auto phrase = alice_.Factory().Secret(0);

        return alice_.Crypto().Seed().ImportSeed(
            words,
            phrase,
            ot::crypto::SeedStyle::BIP39,
            ot::crypto::Language::en,
            reason);
    }();

    EXPECT_FALSE(seedID.empty());

    const auto pNym =
        alice_.Wallet().Nym({alice_.Factory(), seedID, 0}, reason, "Alice");

    ASSERT_TRUE(pNym);

    const auto& nym = *pNym;
    const auto localPC = nym.PaymentCodeSecret(reason);
    const auto remotePC =
        alice_.Factory().PaymentCodeFromBase58(remote.payment_code_);

    EXPECT_EQ(localPC.Version(), 3);
    EXPECT_EQ(remotePC.Version(), 3);
    EXPECT_EQ(localPC.asBase58(), vector.payment_code_);

    const auto path = [&] {
        auto out = ot::Space{};
        nym.PaymentCodePath(ot::writer(out));

        return out;
    }();
    const auto& account1 = alice_.Crypto().Blockchain().LoadOrCreateSubaccount(
        nym.ID(), remotePC, receiveChain, reason);
    const auto& account2 = alice_.Crypto().Blockchain().LoadOrCreateSubaccount(
        nym.ID(), remotePC, sendChain, reason);

    EXPECT_TRUE(account1);
    EXPECT_TRUE(account2);

    using Subchain = ot::blockchain::crypto::Subchain;
    const auto populate = [&](const auto& account, const auto& target) {
        const auto generate = [&](const auto subchain) {
            auto index = account.LastGenerated(subchain);

            while (!index.has_value() || index.value() < target.size()) {
                index = account.GenerateNext(subchain, reason);
            }
        };

        generate(Subchain::Incoming);
        generate(Subchain::Outgoing);
    };
    populate(account1, vector.receive_keys_);
    populate(account2, remote.receive_keys_);

    for (auto i{0u}; i < vector.receive_keys_.size(); ++i) {
        const auto expected =
            alice_.Factory().DataFromHex(vector.receive_keys_.at(i));
        const auto& element = account1.BalanceElement(Subchain::Incoming, i);
        const auto& key = element.Key();

        ASSERT_TRUE(key.IsValid());
        EXPECT_EQ(expected.Bytes(), key.PublicKey());

        const auto& element2 =
            alice_.Crypto().Blockchain().GetKey(element.KeyID());

        EXPECT_EQ(element.KeyID(), element2.KeyID());
    }

    for (auto i{0u}; i < remote.receive_keys_.size(); ++i) {
        const auto expected =
            alice_.Factory().DataFromHex(remote.receive_keys_.at(i));
        const auto& element = account2.BalanceElement(Subchain::Outgoing, i);
        const auto& key = element.Key();

        ASSERT_TRUE(key.IsValid());
        EXPECT_EQ(expected.Bytes(), key.PublicKey());

        const auto& element2 =
            alice_.Crypto().Blockchain().GetKey(element.KeyID());

        EXPECT_EQ(element.KeyID(), element2.KeyID());
    }
}

TEST_F(PaymentCodeAPI, bob)
{
    const auto& vector = GetPaymentCodeVector3().bob_;
    const auto& remote = GetPaymentCodeVector3().alice_;
    constexpr auto receiveChain{ot::blockchain::Type::Bitcoin_testnet3};
    constexpr auto sendChain{ot::blockchain::Type::Bitcoin};
    const auto reason = bob_.Factory().PasswordPrompt(__func__);
    const auto seedID = [&] {
        const auto words = bob_.Factory().SecretFromText(vector.words_);
        const auto phrase = bob_.Factory().Secret(0);

        return bob_.Crypto().Seed().ImportSeed(
            words,
            phrase,
            ot::crypto::SeedStyle::BIP39,
            ot::crypto::Language::en,
            reason);
    }();

    EXPECT_FALSE(seedID.empty());

    const auto pNym =
        bob_.Wallet().Nym({bob_.Factory(), seedID, 0}, reason, "Bob");

    ASSERT_TRUE(pNym);

    const auto& nym = *pNym;
    const auto localPC = nym.PaymentCodeSecret(reason);
    const auto remotePC =
        bob_.Factory().PaymentCodeFromBase58(remote.payment_code_);

    EXPECT_EQ(localPC.Version(), 3);
    EXPECT_EQ(remotePC.Version(), 3);
    EXPECT_EQ(localPC.asBase58(), vector.payment_code_);

    const auto path = [&] {
        auto out = ot::Space{};
        nym.PaymentCodePath(ot::writer(out));

        return out;
    }();
    const auto& account1 = bob_.Crypto().Blockchain().LoadOrCreateSubaccount(
        nym.ID(), remotePC, receiveChain, reason);
    const auto& account2 = bob_.Crypto().Blockchain().LoadOrCreateSubaccount(
        nym.ID(), remotePC, sendChain, reason);

    EXPECT_TRUE(account1.IsValid());
    EXPECT_TRUE(account2.IsValid());

    using Subchain = ot::blockchain::crypto::Subchain;
    const auto populate = [&](const auto& account, const auto& target) {
        const auto generate = [&](const auto subchain) {
            auto index = account.LastGenerated(subchain);

            while (!index.has_value() || index.value() < target.size()) {
                index = account.GenerateNext(subchain, reason);
            }
        };

        generate(Subchain::Incoming);
        generate(Subchain::Outgoing);
    };
    populate(account1, vector.receive_keys_);
    populate(account2, remote.receive_keys_);

    for (auto i{0u}; i < vector.receive_keys_.size(); ++i) {
        const auto expected =
            bob_.Factory().DataFromHex(vector.receive_keys_.at(i));
        const auto& element = account1.BalanceElement(Subchain::Incoming, i);
        const auto& key = element.Key();

        ASSERT_TRUE(key.IsValid());
        EXPECT_EQ(expected.Bytes(), key.PublicKey());

        const auto& element2 =
            bob_.Crypto().Blockchain().GetKey(element.KeyID());

        EXPECT_EQ(element.KeyID(), element2.KeyID());
    }

    for (auto i{0u}; i < remote.receive_keys_.size(); ++i) {
        const auto expected =
            bob_.Factory().DataFromHex(remote.receive_keys_.at(i));
        const auto& element = account2.BalanceElement(Subchain::Outgoing, i);
        const auto& key = element.Key();

        ASSERT_TRUE(key.IsValid());
        EXPECT_EQ(expected.Bytes(), key.PublicKey());

        const auto& element2 =
            bob_.Crypto().Blockchain().GetKey(element.KeyID());

        EXPECT_EQ(element.KeyID(), element2.KeyID());
    }
}
}  // namespace ottest
