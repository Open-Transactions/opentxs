// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <cstddef>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>

#include "Helpers.hpp"
#include "VectorsV3.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/api/crypto/Encode.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/PasswordPrompt.hpp"
#include "opentxs/core/PaymentCode.hpp"
#include "opentxs/crypto/Types.hpp"
#include "opentxs/crypto/key/EllipticCurve.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace ottest
{
class Test_PaymentCode_v3 : public PC_Fixture_Base
{
public:
    static constexpr auto version_ = std::uint8_t{3};

    const ot::crypto::key::EllipticCurve& alice_blind_secret_;
    const ot::crypto::key::EllipticCurve& alice_blind_public_;
    const ot::crypto::key::EllipticCurve& bob_blind_secret_;
    const ot::crypto::key::EllipticCurve& bob_blind_public_;

    Test_PaymentCode_v3()
        : PC_Fixture_Base(
              version_,
              version_,
              GetVectors3().alice_.words_,
              GetVectors3().bob_.words_,
              GetVectors3().alice_.payment_code_,
              GetVectors3().bob_.payment_code_)
        , alice_blind_secret_(user_1_.blinding_key_secret(
              api_,
              GetVectors3().bob_.receive_chain_,
              reason_))
        , alice_blind_public_(user_1_.blinding_key_public())
        , bob_blind_secret_(user_2_.blinding_key_secret(
              api_,
              GetVectors3().alice_.receive_chain_,
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

    const auto decode1 =
        api_.Crypto().Encode().IdentifierDecode(alice_pc_secret_.asBase58());
    const auto decode2 =
        api_.Crypto().Encode().IdentifierDecode(alice_pc_public_.asBase58());
    auto data1 =
        api_.Factory().Data(ot::ReadView{decode1.data(), decode1.size()});
    auto data2 =
        api_.Factory().Data(ot::ReadView{decode2.data(), decode2.size()});

    EXPECT_EQ(alice_pc_secret_.asBase58(), alice_pc_public_.asBase58());
    EXPECT_EQ(alice_pc_secret_.asBase58(), GetVectors3().alice_.payment_code_);
    EXPECT_EQ(bob_pc_secret_.asBase58(), bob_pc_public_.asBase58());
    EXPECT_EQ(bob_pc_secret_.asBase58(), GetVectors3().bob_.payment_code_);
}

TEST_F(Test_PaymentCode_v3, locators)
{
    for (auto i = std::uint8_t{1}; i < 4u; ++i) {
        auto pub = api_.Factory().Data();
        auto sec = api_.Factory().Data();

        EXPECT_TRUE(alice_pc_public_.Locator(pub->WriteInto(), i));
        EXPECT_TRUE(alice_pc_secret_.Locator(sec->WriteInto(), i));
        EXPECT_EQ(pub->asHex(), GetVectors3().alice_.locators_.at(i - 1u));
        EXPECT_EQ(sec->asHex(), GetVectors3().alice_.locators_.at(i - 1u));
    }
    {
        auto got = api_.Factory().Data();

        EXPECT_TRUE(alice_pc_secret_.Locator(got->WriteInto()));
        EXPECT_EQ(got->asHex(), GetVectors3().alice_.locators_.back());
    }

    for (auto i = std::uint8_t{1}; i < 4u; ++i) {
        auto pub = api_.Factory().Data();
        auto sec = api_.Factory().Data();

        EXPECT_TRUE(bob_pc_public_.Locator(pub->WriteInto(), i));
        EXPECT_TRUE(bob_pc_secret_.Locator(sec->WriteInto(), i));
        EXPECT_EQ(pub->asHex(), GetVectors3().bob_.locators_.at(i - 1u));
        EXPECT_EQ(sec->asHex(), GetVectors3().bob_.locators_.at(i - 1u));
    }
    {
        auto got = api_.Factory().Data();

        EXPECT_TRUE(bob_pc_secret_.Locator(got->WriteInto()));
        EXPECT_EQ(got->asHex(), GetVectors3().bob_.locators_.back());
    }
}

TEST_F(Test_PaymentCode_v3, outgoing_btc)
{
    for (auto i = ot::Bip32Index{0}; i < 10u; ++i) {
        const auto pKey = bob_pc_secret_.Outgoing(
            alice_pc_public_,
            i,
            GetVectors3().alice_.receive_chain_,
            reason_,
            version_);

        ASSERT_TRUE(pKey);

        const auto& key = *pKey;
        const auto expect = api_.Factory().Data(
            GetVectors3().alice_.receive_keys_.at(i), ot::StringStyle::Hex);

        EXPECT_NE(
            GetVectors3().alice_.receive_keys_.at(i),
            GetVectors3().bob_.receive_keys_.at(i));
        EXPECT_EQ(expect->Bytes(), key.PublicKey());
    }
}

TEST_F(Test_PaymentCode_v3, incoming_btc)
{
    for (auto i = ot::Bip32Index{0}; i < 10u; ++i) {
        const auto pKey = alice_pc_secret_.Incoming(
            bob_pc_public_,
            i,
            GetVectors3().alice_.receive_chain_,
            reason_,
            version_);

        ASSERT_TRUE(pKey);

        const auto& key = *pKey;
        const auto expect = api_.Factory().Data(
            GetVectors3().alice_.receive_keys_.at(i), ot::StringStyle::Hex);

        EXPECT_NE(
            GetVectors3().alice_.receive_keys_.at(i),
            GetVectors3().bob_.receive_keys_.at(i));
        EXPECT_EQ(expect->Bytes(), key.PublicKey());
    }
}

TEST_F(Test_PaymentCode_v3, outgoing_testnet)
{
    for (auto i = ot::Bip32Index{0}; i < 10u; ++i) {
        const auto pKey = alice_pc_secret_.Outgoing(
            bob_pc_public_,
            i,
            GetVectors3().bob_.receive_chain_,
            reason_,
            version_);

        ASSERT_TRUE(pKey);

        const auto& key = *pKey;
        const auto expect = api_.Factory().Data(
            GetVectors3().bob_.receive_keys_.at(i), ot::StringStyle::Hex);

        EXPECT_NE(
            GetVectors3().bob_.receive_keys_.at(i),
            GetVectors3().alice_.receive_keys_.at(i));
        EXPECT_EQ(expect->Bytes(), key.PublicKey());
    }
}

TEST_F(Test_PaymentCode_v3, incoming_testnet)
{
    for (auto i = ot::Bip32Index{0}; i < 10u; ++i) {
        const auto pKey = bob_pc_secret_.Incoming(
            alice_pc_public_,
            i,
            GetVectors3().bob_.receive_chain_,
            reason_,
            version_);

        ASSERT_TRUE(pKey);

        const auto& key = *pKey;
        const auto expect = api_.Factory().Data(
            GetVectors3().bob_.receive_keys_.at(i), ot::StringStyle::Hex);

        EXPECT_NE(
            GetVectors3().bob_.receive_keys_.at(i),
            GetVectors3().alice_.receive_keys_.at(i));
        EXPECT_EQ(expect->Bytes(), key.PublicKey());
    }
}

TEST_F(Test_PaymentCode_v3, avoid_cross_chain_address_reuse)
{
    for (auto i = ot::Bip32Index{0}; i < 10u; ++i) {
        const auto pKey = bob_pc_secret_.Outgoing(
            alice_pc_public_,
            i,
            ot::blockchain::Type::Litecoin,
            reason_,
            version_);

        ASSERT_TRUE(pKey);

        const auto& key = *pKey;
        const auto expect = api_.Factory().Data(
            GetVectors3().alice_.receive_keys_.at(i), ot::StringStyle::Hex);

        EXPECT_NE(expect->Bytes(), key.PublicKey());
    }
}

TEST_F(Test_PaymentCode_v3, blind_alice)
{
    const auto sec = api_.Factory().Data(
        GetVectors3().alice_.change_key_secret_, ot::StringStyle::Hex);
    const auto pub = api_.Factory().Data(
        GetVectors3().alice_.change_key_public_, ot::StringStyle::Hex);

    EXPECT_EQ(sec->Bytes(), alice_blind_secret_.PrivateKey(reason_));
    EXPECT_EQ(pub->Bytes(), alice_blind_public_.PublicKey());
    EXPECT_EQ(alice_blind_secret_.PublicKey(), alice_blind_public_.PublicKey());

    {
        const auto expect = api_.Factory().Data(
            GetVectors3().alice_.blinded_payment_code_, ot::StringStyle::Hex);
        auto blinded = api_.Factory().Data();

        EXPECT_TRUE(alice_pc_secret_.BlindV3(
            bob_pc_public_,
            alice_blind_secret_,
            blinded->WriteInto(),
            reason_));
        EXPECT_EQ(expect.get(), blinded.get());

        const auto alice = bob_pc_secret_.UnblindV3(
            version_, blinded->Bytes(), alice_blind_public_, reason_);

        EXPECT_GT(alice.Version(), 0u);
        EXPECT_EQ(alice.asBase58(), GetVectors3().alice_.payment_code_);
    }

    const auto elements = alice_pc_secret_.GenerateNotificationElements(
        bob_pc_public_, alice_blind_secret_, reason_);

    ASSERT_EQ(elements.size(), 3);

    const auto& A = elements.at(0);
    const auto& F = elements.at(1);
    const auto& G = elements.at(2);

    {
        const auto got = api_.Factory().Data(ot::reader(A));

        EXPECT_EQ(got->Bytes(), alice_blind_public_.PublicKey());
    }
    {
        const auto expect =
            api_.Factory().Data(GetVectors3().alice_.F_, ot::StringStyle::Hex);
        const auto got = api_.Factory().Data(ot::reader(F));

        EXPECT_EQ(expect.get(), got.get());
    }
    {
        const auto expect =
            api_.Factory().Data(GetVectors3().alice_.G_, ot::StringStyle::Hex);
        const auto got = api_.Factory().Data(ot::reader(G));
        constexpr auto ignore = std::size_t{16};
        const auto v1 =
            ot::ReadView{expect->Bytes().data(), expect->size() - ignore};
        const auto v2 = ot::ReadView{got->Bytes().data(), got->size() - ignore};

        EXPECT_EQ(v1, v2);
    }

    using Elements = std::pmr::vector<ot::Space>;
    {
        const auto alice = bob_pc_secret_.DecodeNotificationElements(
            version_, Elements{A, F, G}, reason_);

        EXPECT_GT(alice.Version(), 0u);
        EXPECT_EQ(alice.asBase58(), GetVectors3().alice_.payment_code_);
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
    const auto sec = api_.Factory().Data(
        GetVectors3().bob_.change_key_secret_, ot::StringStyle::Hex);
    const auto pub = api_.Factory().Data(
        GetVectors3().bob_.change_key_public_, ot::StringStyle::Hex);

    EXPECT_EQ(sec->Bytes(), bob_blind_secret_.PrivateKey(reason_));
    EXPECT_EQ(pub->Bytes(), bob_blind_public_.PublicKey());
    EXPECT_EQ(bob_blind_secret_.PublicKey(), bob_blind_public_.PublicKey());

    {
        const auto expect = api_.Factory().Data(
            GetVectors3().bob_.blinded_payment_code_, ot::StringStyle::Hex);
        auto blinded = api_.Factory().Data();

        EXPECT_TRUE(bob_pc_secret_.BlindV3(
            alice_pc_public_,
            bob_blind_secret_,
            blinded->WriteInto(),
            reason_));
        EXPECT_EQ(expect.get(), blinded.get());

        const auto bob = alice_pc_secret_.UnblindV3(
            version_, blinded->Bytes(), bob_blind_public_, reason_);

        EXPECT_GT(bob.Version(), 0u);
        EXPECT_EQ(bob.asBase58(), GetVectors3().bob_.payment_code_);
    }

    const auto elements = bob_pc_secret_.GenerateNotificationElements(
        alice_pc_public_, bob_blind_secret_, reason_);

    ASSERT_EQ(elements.size(), 3);

    const auto& A = elements.at(0);
    const auto& F = elements.at(1);
    const auto& G = elements.at(2);

    {
        const auto got = api_.Factory().Data(ot::reader(A));

        EXPECT_EQ(got->Bytes(), bob_blind_public_.PublicKey());
    }
    {
        const auto expect =
            api_.Factory().Data(GetVectors3().bob_.F_, ot::StringStyle::Hex);
        const auto got = api_.Factory().Data(ot::reader(F));

        EXPECT_EQ(expect.get(), got.get());
    }
    {
        const auto expect =
            api_.Factory().Data(GetVectors3().bob_.G_, ot::StringStyle::Hex);
        const auto got = api_.Factory().Data(ot::reader(G));
        constexpr auto ignore = std::size_t{16};
        const auto v1 =
            ot::ReadView{expect->Bytes().data(), expect->size() - ignore};
        const auto v2 = ot::ReadView{got->Bytes().data(), got->size() - ignore};

        EXPECT_EQ(v1, v2);
    }

    using Elements = std::pmr::vector<ot::Space>;
    {
        const auto bob = alice_pc_secret_.DecodeNotificationElements(
            version_, Elements{A, F, G}, reason_);

        EXPECT_GT(bob.Version(), 0u);
        EXPECT_EQ(bob.asBase58(), GetVectors3().bob_.payment_code_);
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
