// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstdint>

#include "internal/util/P0330.hpp"
#include "ottest/data/crypto/PaymentCodeV3.hpp"
#include "ottest/fixtures/paymentcode/Helpers.hpp"

namespace ottest
{
using namespace opentxs::literals;

class Test_PaymentCode_v3 : public PC_Fixture_Base
{
public:
    static constexpr auto version_ = std::uint8_t{3};

    const ot::crypto::asymmetric::key::EllipticCurve& alice_blind_secret_;
    const ot::crypto::asymmetric::key::EllipticCurve& alice_blind_public_;
    const ot::crypto::asymmetric::key::EllipticCurve& bob_blind_secret_;
    const ot::crypto::asymmetric::key::EllipticCurve& bob_blind_public_;

    Test_PaymentCode_v3()
        : PC_Fixture_Base(
              version_,
              version_,
              GetPaymentCodeVector3().alice_.words_,
              GetPaymentCodeVector3().bob_.words_,
              GetPaymentCodeVector3().alice_.payment_code_,
              GetPaymentCodeVector3().bob_.payment_code_)
        , alice_blind_secret_(user_1_.blinding_key_secret(
              api_,
              GetPaymentCodeVector3().bob_.receive_chain_,
              reason_))
        , alice_blind_public_(user_1_.blinding_key_public())
        , bob_blind_secret_(user_2_.blinding_key_secret(
              api_,
              GetPaymentCodeVector3().alice_.receive_chain_,
              reason_))
        , bob_blind_public_(user_2_.blinding_key_public())
    {
    }
};

TEST_F(Test_PaymentCode_v3, generate)
{
    EXPECT_EQ(alice_pc_secret_.Version(), version_);
    EXPECT_EQ(alice_pc_public_.Version(), version_);
    EXPECT_EQ(bob_pc_secret_.Version(), version_);
    EXPECT_EQ(bob_pc_public_.Version(), version_);

    auto data1 = ot::ByteArray{};
    auto data2 = ot::ByteArray{};

    EXPECT_TRUE(api_.Crypto().Encode().Base58CheckDecode(
        alice_pc_secret_.asBase58(), data1.WriteInto()));
    EXPECT_TRUE(api_.Crypto().Encode().Base58CheckDecode(
        alice_pc_public_.asBase58(), data2.WriteInto()));
    EXPECT_EQ(alice_pc_secret_.asBase58(), alice_pc_public_.asBase58());
    EXPECT_EQ(
        alice_pc_secret_.asBase58(),
        GetPaymentCodeVector3().alice_.payment_code_);
    EXPECT_EQ(bob_pc_secret_.asBase58(), bob_pc_public_.asBase58());
    EXPECT_EQ(
        bob_pc_secret_.asBase58(), GetPaymentCodeVector3().bob_.payment_code_);
}

TEST_F(Test_PaymentCode_v3, locators)
{
    for (auto i = std::uint8_t{1}; i < 4u; ++i) {
        auto pub = api_.Factory().Data();
        auto sec = api_.Factory().Data();

        EXPECT_TRUE(alice_pc_public_.Locator(pub.WriteInto(), i));
        EXPECT_TRUE(alice_pc_secret_.Locator(sec.WriteInto(), i));
        EXPECT_EQ(
            pub.asHex(), GetPaymentCodeVector3().alice_.locators_.at(i - 1u));
        EXPECT_EQ(
            sec.asHex(), GetPaymentCodeVector3().alice_.locators_.at(i - 1u));
    }
    {
        auto got = api_.Factory().Data();

        EXPECT_TRUE(alice_pc_secret_.Locator(got.WriteInto()));
        EXPECT_EQ(got.asHex(), GetPaymentCodeVector3().alice_.locators_.back());
    }

    for (auto i = std::uint8_t{1}; i < 4u; ++i) {
        auto pub = api_.Factory().Data();
        auto sec = api_.Factory().Data();

        EXPECT_TRUE(bob_pc_public_.Locator(pub.WriteInto(), i));
        EXPECT_TRUE(bob_pc_secret_.Locator(sec.WriteInto(), i));
        EXPECT_EQ(
            pub.asHex(), GetPaymentCodeVector3().bob_.locators_.at(i - 1u));
        EXPECT_EQ(
            sec.asHex(), GetPaymentCodeVector3().bob_.locators_.at(i - 1u));
    }
    {
        auto got = api_.Factory().Data();

        EXPECT_TRUE(bob_pc_secret_.Locator(got.WriteInto()));
        EXPECT_EQ(got.asHex(), GetPaymentCodeVector3().bob_.locators_.back());
    }
}

TEST_F(Test_PaymentCode_v3, outgoing_btc)
{
    for (auto i = ot::Bip32Index{0}; i < 10u; ++i) {
        const auto key = bob_pc_secret_.Outgoing(
            alice_pc_public_,
            i,
            GetPaymentCodeVector3().alice_.receive_chain_,
            reason_,
            version_);

        ASSERT_TRUE(key.IsValid());

        const auto expect = api_.Factory().DataFromHex(
            GetPaymentCodeVector3().alice_.receive_keys_.at(i));

        EXPECT_NE(
            GetPaymentCodeVector3().alice_.receive_keys_.at(i),
            GetPaymentCodeVector3().bob_.receive_keys_.at(i));
        EXPECT_EQ(expect.Bytes(), key.PublicKey());
    }
}

TEST_F(Test_PaymentCode_v3, incoming_btc)
{
    for (auto i = ot::Bip32Index{0}; i < 10u; ++i) {
        const auto key = alice_pc_secret_.Incoming(
            bob_pc_public_,
            i,
            GetPaymentCodeVector3().alice_.receive_chain_,
            reason_,
            version_);

        ASSERT_TRUE(key.IsValid());

        const auto expect = api_.Factory().DataFromHex(
            GetPaymentCodeVector3().alice_.receive_keys_.at(i));

        EXPECT_NE(
            GetPaymentCodeVector3().alice_.receive_keys_.at(i),
            GetPaymentCodeVector3().bob_.receive_keys_.at(i));
        EXPECT_EQ(expect.Bytes(), key.PublicKey());
    }
}

TEST_F(Test_PaymentCode_v3, outgoing_testnet)
{
    for (auto i = ot::Bip32Index{0}; i < 10u; ++i) {
        const auto key = alice_pc_secret_.Outgoing(
            bob_pc_public_,
            i,
            GetPaymentCodeVector3().bob_.receive_chain_,
            reason_,
            version_);

        ASSERT_TRUE(key.IsValid());

        const auto expect = api_.Factory().DataFromHex(
            GetPaymentCodeVector3().bob_.receive_keys_.at(i));

        EXPECT_NE(
            GetPaymentCodeVector3().bob_.receive_keys_.at(i),
            GetPaymentCodeVector3().alice_.receive_keys_.at(i));
        EXPECT_EQ(expect.Bytes(), key.PublicKey());
    }
}

TEST_F(Test_PaymentCode_v3, incoming_testnet)
{
    for (auto i = ot::Bip32Index{0}; i < 10u; ++i) {
        const auto key = bob_pc_secret_.Incoming(
            alice_pc_public_,
            i,
            GetPaymentCodeVector3().bob_.receive_chain_,
            reason_,
            version_);

        ASSERT_TRUE(key.IsValid());

        const auto expect = api_.Factory().DataFromHex(
            GetPaymentCodeVector3().bob_.receive_keys_.at(i));

        EXPECT_NE(
            GetPaymentCodeVector3().bob_.receive_keys_.at(i),
            GetPaymentCodeVector3().alice_.receive_keys_.at(i));
        EXPECT_EQ(expect.Bytes(), key.PublicKey());
    }
}

TEST_F(Test_PaymentCode_v3, avoid_cross_chain_address_reuse)
{
    for (auto i = ot::Bip32Index{0}; i < 10u; ++i) {
        const auto key = bob_pc_secret_.Outgoing(
            alice_pc_public_,
            i,
            ot::blockchain::Type::Litecoin,
            reason_,
            version_);

        ASSERT_TRUE(key.IsValid());

        const auto expect = api_.Factory().DataFromHex(
            GetPaymentCodeVector3().alice_.receive_keys_.at(i));

        EXPECT_NE(expect.Bytes(), key.PublicKey());
    }
}

TEST_F(Test_PaymentCode_v3, blind_alice)
{
    const auto sec = api_.Factory().DataFromHex(
        GetPaymentCodeVector3().alice_.change_key_secret_);
    const auto pub = api_.Factory().DataFromHex(
        GetPaymentCodeVector3().alice_.change_key_public_);

    EXPECT_EQ(sec.Bytes(), alice_blind_secret_.PrivateKey(reason_));
    EXPECT_EQ(pub.Bytes(), alice_blind_public_.PublicKey());
    EXPECT_EQ(alice_blind_secret_.PublicKey(), alice_blind_public_.PublicKey());

    {
        const auto expect = api_.Factory().DataFromHex(
            GetPaymentCodeVector3().alice_.blinded_payment_code_);
        auto blinded = api_.Factory().Data();

        EXPECT_TRUE(alice_pc_secret_.BlindV3(
            bob_pc_public_, alice_blind_secret_, blinded.WriteInto(), reason_));
        EXPECT_EQ(expect, blinded);

        const auto alice = bob_pc_secret_.UnblindV3(
            version_, blinded.Bytes(), alice_blind_public_, reason_);

        EXPECT_GT(alice.Version(), 0u);
        EXPECT_EQ(
            alice.asBase58(), GetPaymentCodeVector3().alice_.payment_code_);
    }

    const auto elements = alice_pc_secret_.GenerateNotificationElements(
        bob_pc_public_, alice_blind_secret_, reason_);

    ASSERT_EQ(elements.size(), 3);

    const auto& A = elements.at(0);
    const auto& F = elements.at(1);
    const auto& G = elements.at(2);

    {
        const auto got = api_.Factory().DataFromBytes(ot::reader(A));

        EXPECT_EQ(got.Bytes(), alice_blind_public_.PublicKey());
    }
    {
        const auto expect =
            api_.Factory().DataFromHex(GetPaymentCodeVector3().alice_.f_);
        const auto got = api_.Factory().DataFromBytes(ot::reader(F));

        EXPECT_EQ(expect, got);
    }
    {
        const auto expect =
            api_.Factory().DataFromHex(GetPaymentCodeVector3().alice_.g_);
        const auto got = api_.Factory().DataFromBytes(ot::reader(G));
        constexpr auto ignore = 16_uz;
        const auto v1 =
            ot::ReadView{expect.Bytes().data(), expect.size() - ignore};
        const auto v2 = ot::ReadView{got.Bytes().data(), got.size() - ignore};

        EXPECT_EQ(v1, v2);
    }

    using Elements = ot::UnallocatedVector<ot::Space>;
    {
        const auto alice = bob_pc_secret_.DecodeNotificationElements(
            version_, Elements{A, F, G}, reason_);

        EXPECT_GT(alice.Version(), 0u);
        EXPECT_EQ(
            alice.asBase58(), GetPaymentCodeVector3().alice_.payment_code_);
    }
    {
        const auto alice = bob_pc_secret_.DecodeNotificationElements(
            version_, Elements{A, G, F}, reason_);

        EXPECT_EQ(alice.Version(), 0u);
    }
    {
        const auto alice = bob_pc_secret_.DecodeNotificationElements(
            version_, Elements{F, G, A}, reason_);

        EXPECT_EQ(alice.Version(), 0u);
    }
    {
        const auto alice = bob_pc_secret_.DecodeNotificationElements(
            version_, Elements{F, A, G}, reason_);

        EXPECT_EQ(alice.Version(), 0u);
    }
    {
        const auto alice = bob_pc_secret_.DecodeNotificationElements(
            version_, Elements{G, A, F}, reason_);

        EXPECT_EQ(alice.Version(), 0u);
    }
    {
        const auto alice = bob_pc_secret_.DecodeNotificationElements(
            version_, Elements{G, F, A}, reason_);

        EXPECT_EQ(alice.Version(), 0u);
    }
}

TEST_F(Test_PaymentCode_v3, blind_bob)
{
    const auto sec = api_.Factory().DataFromHex(
        GetPaymentCodeVector3().bob_.change_key_secret_);
    const auto pub = api_.Factory().DataFromHex(
        GetPaymentCodeVector3().bob_.change_key_public_);

    EXPECT_EQ(sec.Bytes(), bob_blind_secret_.PrivateKey(reason_));
    EXPECT_EQ(pub.Bytes(), bob_blind_public_.PublicKey());
    EXPECT_EQ(bob_blind_secret_.PublicKey(), bob_blind_public_.PublicKey());

    {
        const auto expect = api_.Factory().DataFromHex(
            GetPaymentCodeVector3().bob_.blinded_payment_code_);
        auto blinded = api_.Factory().Data();

        EXPECT_TRUE(bob_pc_secret_.BlindV3(
            alice_pc_public_, bob_blind_secret_, blinded.WriteInto(), reason_));
        EXPECT_EQ(expect, blinded);

        const auto bob = alice_pc_secret_.UnblindV3(
            version_, blinded.Bytes(), bob_blind_public_, reason_);

        EXPECT_GT(bob.Version(), 0u);
        EXPECT_EQ(bob.asBase58(), GetPaymentCodeVector3().bob_.payment_code_);
    }

    const auto elements = bob_pc_secret_.GenerateNotificationElements(
        alice_pc_public_, bob_blind_secret_, reason_);

    ASSERT_EQ(elements.size(), 3);

    const auto& A = elements.at(0);
    const auto& F = elements.at(1);
    const auto& G = elements.at(2);

    {
        const auto got = api_.Factory().DataFromBytes(ot::reader(A));

        EXPECT_EQ(got.Bytes(), bob_blind_public_.PublicKey());
    }
    {
        const auto expect =
            api_.Factory().DataFromHex(GetPaymentCodeVector3().bob_.f_);
        const auto got = api_.Factory().DataFromBytes(ot::reader(F));

        EXPECT_EQ(expect, got);
    }
    {
        const auto expect =
            api_.Factory().DataFromHex(GetPaymentCodeVector3().bob_.g_);
        const auto got = api_.Factory().DataFromBytes(ot::reader(G));
        constexpr auto ignore = 16_uz;
        const auto v1 =
            ot::ReadView{expect.Bytes().data(), expect.size() - ignore};
        const auto v2 = ot::ReadView{got.Bytes().data(), got.size() - ignore};

        EXPECT_EQ(v1, v2);
    }

    using Elements = ot::UnallocatedVector<ot::Space>;
    {
        const auto bob = alice_pc_secret_.DecodeNotificationElements(
            version_, Elements{A, F, G}, reason_);

        EXPECT_GT(bob.Version(), 0u);
        EXPECT_EQ(bob.asBase58(), GetPaymentCodeVector3().bob_.payment_code_);
    }
    {
        const auto bob = alice_pc_secret_.DecodeNotificationElements(
            version_, Elements{A, G, F}, reason_);

        EXPECT_EQ(bob.Version(), 0u);
    }
    {
        const auto bob = alice_pc_secret_.DecodeNotificationElements(
            version_, Elements{F, G, A}, reason_);

        EXPECT_EQ(bob.Version(), 0u);
    }
    {
        const auto bob = alice_pc_secret_.DecodeNotificationElements(
            version_, Elements{F, A, G}, reason_);

        EXPECT_EQ(bob.Version(), 0u);
    }
    {
        const auto bob = alice_pc_secret_.DecodeNotificationElements(
            version_, Elements{G, A, F}, reason_);

        EXPECT_EQ(bob.Version(), 0u);
    }
    {
        const auto bob = alice_pc_secret_.DecodeNotificationElements(
            version_, Elements{G, F, A}, reason_);

        EXPECT_EQ(bob.Version(), 0u);
    }
}

TEST_F(Test_PaymentCode_v3, shutdown) { Shutdown(); }
}  // namespace ottest
