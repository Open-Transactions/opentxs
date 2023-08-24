// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstdint>
#include <string_view>

#include "internal/util/P0330.hpp"
#include "ottest/data/crypto/PaymentCodeV3.hpp"
#include "ottest/fixtures/paymentcode/PaymentCode_v1_v3.hpp"

namespace ottest
{
using namespace opentxs::literals;
using namespace std::literals;

TEST_F(PaymentCode_v1_v3, generate)
{
    EXPECT_EQ(alice_pc_secret_.Version(), alice_version_);
    EXPECT_EQ(alice_pc_public_.Version(), alice_version_);
    EXPECT_EQ(bob_pc_secret_.Version(), bob_version_);
    EXPECT_EQ(bob_pc_public_.Version(), bob_version_);

    auto data1 = ot::ByteArray{};
    auto data2 = ot::ByteArray{};

    EXPECT_TRUE(api_.Crypto().Encode().Base58CheckDecode(
        alice_pc_secret_.asBase58(), data1.WriteInto()));
    EXPECT_TRUE(api_.Crypto().Encode().Base58CheckDecode(
        alice_pc_public_.asBase58(), data2.WriteInto()));
    EXPECT_EQ(alice_pc_secret_.asBase58(), alice_pc_public_.asBase58());
    EXPECT_EQ(
        alice_pc_secret_.asBase58(),
        GetPaymentCodeVector3().chris_.payment_code_);
    EXPECT_EQ(bob_pc_secret_.asBase58(), bob_pc_public_.asBase58());
    EXPECT_EQ(
        bob_pc_secret_.asBase58(),
        GetPaymentCodeVector3().daniel_.payment_code_);
}

TEST_F(PaymentCode_v1_v3, locators)
{
    for (auto i = std::uint8_t{1}; i < 4u; ++i) {
        auto got = api_.Factory().Data();

        EXPECT_FALSE(alice_pc_public_.Locator(got.WriteInto(), i));
        EXPECT_FALSE(alice_pc_secret_.Locator(got.WriteInto(), i));
    }
    {
        auto got = api_.Factory().Data();

        EXPECT_FALSE(alice_pc_secret_.Locator(got.WriteInto()));
    }

    for (auto i = std::uint8_t{1}; i < 4u; ++i) {
        auto pub = api_.Factory().Data();
        auto sec = api_.Factory().Data();

        EXPECT_TRUE(bob_pc_public_.Locator(pub.WriteInto(), i));
        EXPECT_TRUE(bob_pc_secret_.Locator(sec.WriteInto(), i));
        EXPECT_EQ(
            pub.asHex(), GetPaymentCodeVector3().daniel_.locators_.at(i - 1u));
        EXPECT_EQ(
            sec.asHex(), GetPaymentCodeVector3().daniel_.locators_.at(i - 1u));
    }
    {
        auto got = api_.Factory().Data();

        EXPECT_TRUE(bob_pc_secret_.Locator(got.WriteInto()));
        EXPECT_EQ(
            got.asHex(), GetPaymentCodeVector3().daniel_.locators_.back());
    }
}

TEST_F(PaymentCode_v1_v3, outgoing_btc)
{
    for (auto i = ot::Bip32Index{0}; i < 10u; ++i) {
        const auto key = bob_pc_secret_.Outgoing(
            alice_pc_public_,
            i,
            GetPaymentCodeVector3().chris_.receive_chain_,
            reason_);

        ASSERT_TRUE(key.IsValid());

        const auto expect = api_.Factory().DataFromHex(
            GetPaymentCodeVector3().chris_.receive_keys_.at(i));

        EXPECT_EQ(expect.Bytes(), key.PublicKey());
    }
}

TEST_F(PaymentCode_v1_v3, incoming_btc)
{
    for (auto i = ot::Bip32Index{0}; i < 10u; ++i) {
        const auto key = alice_pc_secret_.Incoming(
            bob_pc_public_,
            i,
            GetPaymentCodeVector3().chris_.receive_chain_,
            reason_);

        ASSERT_TRUE(key.IsValid());

        const auto expect = api_.Factory().DataFromHex(
            GetPaymentCodeVector3().chris_.receive_keys_.at(i));

        EXPECT_EQ(expect.Bytes(), key.PublicKey());
    }
}

TEST_F(PaymentCode_v1_v3, outgoing_testnet)
{
    for (auto i = ot::Bip32Index{0}; i < 10u; ++i) {
        const auto key = alice_pc_secret_.Outgoing(
            bob_pc_public_,
            i,
            GetPaymentCodeVector3().daniel_.receive_chain_,
            reason_);

        ASSERT_TRUE(key.IsValid());

        const auto expect = api_.Factory().DataFromHex(
            GetPaymentCodeVector3().daniel_.receive_keys_.at(i));

        EXPECT_EQ(expect.Bytes(), key.PublicKey());
    }
}

TEST_F(PaymentCode_v1_v3, incoming_testnet)
{
    for (auto i = ot::Bip32Index{0}; i < 10u; ++i) {
        const auto key = bob_pc_secret_.Incoming(
            alice_pc_public_,
            i,
            GetPaymentCodeVector3().daniel_.receive_chain_,
            reason_);

        ASSERT_TRUE(key.IsValid());

        const auto expect = api_.Factory().DataFromHex(
            GetPaymentCodeVector3().daniel_.receive_keys_.at(i));

        EXPECT_NE(
            GetPaymentCodeVector3().daniel_.receive_keys_.at(i),
            GetPaymentCodeVector3().chris_.receive_keys_.at(i));
        EXPECT_EQ(expect.Bytes(), key.PublicKey());
    }
}

TEST_F(PaymentCode_v1_v3, cross_chain_address_reuse)
{
    for (auto i = ot::Bip32Index{0}; i < 10u; ++i) {
        const auto key = bob_pc_secret_.Outgoing(
            alice_pc_public_, i, ot::blockchain::Type::Litecoin, reason_);

        ASSERT_TRUE(key.IsValid());

        const auto expect = api_.Factory().DataFromHex(
            GetPaymentCodeVector3().chris_.receive_keys_.at(i));

        EXPECT_EQ(expect.Bytes(), key.PublicKey());
    }
}

TEST_F(PaymentCode_v1_v3, blind_alice)
{
    const auto sec = api_.Factory().DataFromHex(
        GetPaymentCodeVector3().chris_.change_key_secret_);
    const auto pub = api_.Factory().DataFromHex(
        GetPaymentCodeVector3().chris_.change_key_public_);

    EXPECT_EQ(sec.Bytes(), alice_blind_secret_.PrivateKey(reason_));
    EXPECT_EQ(pub.Bytes(), alice_blind_public_.PublicKey());
    EXPECT_EQ(alice_blind_secret_.PublicKey(), alice_blind_public_.PublicKey());

    {
        const auto expect = api_.Factory().DataFromHex(
            GetPaymentCodeVector3().chris_.blinded_payment_code_);
        auto blinded = api_.Factory().Data();

        EXPECT_TRUE(alice_pc_secret_.BlindV3(
            bob_pc_public_, alice_blind_secret_, blinded.WriteInto(), reason_));
        EXPECT_EQ(expect, blinded);

        const auto alice = bob_pc_secret_.UnblindV3(
            alice_version_, blinded.Bytes(), alice_blind_public_, reason_);

        EXPECT_GT(alice.Version(), 0u);
        EXPECT_EQ(
            alice.asBase58(), GetPaymentCodeVector3().chris_.payment_code_);
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
            api_.Factory().DataFromHex(GetPaymentCodeVector3().chris_.f_);
        const auto got = api_.Factory().DataFromBytes(ot::reader(F));

        EXPECT_EQ(expect, got);
    }
    {
        const auto expect =
            api_.Factory().DataFromHex(GetPaymentCodeVector3().chris_.g_);
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
            alice_version_, Elements{A, F, G}, reason_);

        EXPECT_GT(alice.Version(), 0u);
        EXPECT_EQ(
            alice.asBase58(), GetPaymentCodeVector3().chris_.payment_code_);
    }
}

TEST_F(PaymentCode_v1_v3, blind_bob)
{
    {
        const auto sec = api_.Factory().DataFromHex(
            GetPaymentCodeVector3().daniel_.change_key_secret_);
        const auto pub = api_.Factory().DataFromHex(
            GetPaymentCodeVector3().daniel_.change_key_public_);

        EXPECT_EQ(sec.Bytes(), bob_blind_secret_.PrivateKey(reason_));
        EXPECT_EQ(pub.Bytes(), bob_blind_public_.PublicKey());
        EXPECT_EQ(bob_blind_secret_.PublicKey(), bob_blind_public_.PublicKey());
    }
    {
        const auto elements = bob_pc_secret_.GenerateNotificationElements(
            alice_pc_public_, bob_blind_secret_, reason_);

        EXPECT_EQ(elements.size(), 0);

        auto blinded = api_.Factory().Data();

        EXPECT_FALSE(bob_pc_secret_.BlindV3(
            alice_pc_public_, bob_blind_secret_, blinded.WriteInto(), reason_));
    }

    const auto outpoint =
        api_.Factory().DataFromHex(GetPaymentCodeVector3().outpoint_);
    const auto blinded = api_.Factory().DataFromHex(
        GetPaymentCodeVector3().daniel_.blinded_payment_code_);

    {
        auto data = api_.Factory().Data();
        const auto blind = bob_pc_secret_.Blind(
            alice_pc_public_,
            bob_blind_secret_,
            outpoint.Bytes(),
            data.WriteInto(),
            reason_);

        ASSERT_TRUE(blind);
        EXPECT_EQ(data, blinded);

        const auto pPC = alice_pc_secret_.Unblind(
            blinded.Bytes(), bob_blind_public_, outpoint.Bytes(), reason_);

        EXPECT_GT(pPC.Version(), 0u);
        EXPECT_EQ(
            pPC.asBase58(), GetPaymentCodeVector3().daniel_.payment_code_);
    }
}

TEST_F(PaymentCode_v1_v3, shutdown) { Shutdown(); }
}  // namespace ottest
