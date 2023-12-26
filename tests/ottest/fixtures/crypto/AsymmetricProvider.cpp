// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/crypto/AsymmetricProvider.hpp"  // IWYU pragma: associated

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstdint>
#include <string_view>

#include "internal/crypto/library/AsymmetricProvider.hpp"
#include "ottest/env/OTTestEnvironment.hpp"
#include "util/HDIndex.hpp"

namespace ottest
{
namespace ot = opentxs;

using namespace std::literals;

const bool Signatures::have_hd_{ot::api::crypto::HaveHDKeys()};
const bool Signatures::have_rsa_{
    ot::api::crypto::HaveSupport(ot::crypto::asymmetric::Algorithm::Legacy)};
const bool Signatures::have_secp256k1_{
    ot::api::crypto::HaveSupport(ot::crypto::asymmetric::Algorithm::Secp256k1)};
const bool Signatures::have_ed25519_{
    ot::api::crypto::HaveSupport(ot::crypto::asymmetric::Algorithm::ED25519)};

Signatures::Signatures()
    : api_(dynamic_cast<const ot::api::session::Client&>(
          OTTestEnvironment::GetOT().StartClientSession(0)))
    , seed_id_(api_.Crypto().Seed().ImportSeed(
          api_.Factory().SecretFromText(
              "response seminar brave tip suit recall often sound stick owner lottery motion"sv),
          api_.Factory().SecretFromText(""sv),
          opentxs::crypto::SeedStyle::BIP39,
          opentxs::crypto::Language::en,
          api_.Factory().PasswordPrompt("Importing a BIP-39 seed")))
    , ed_(get_key(api_, ot::crypto::EcdsaCurve::ed25519, Role::Sign))
    , ed_hd_([&] {
        if (have_hd_) {

            return get_hd_key(api_, seed_id_, ot::crypto::EcdsaCurve::ed25519);
        } else {

            return get_key(api_, ot::crypto::EcdsaCurve::ed25519, Role::Sign);
        }
    }())
    , ed_2_([&] {
        if (have_hd_) {

            return get_hd_key(
                api_, seed_id_, ot::crypto::EcdsaCurve::ed25519, 1);
        } else {

            return get_key(api_, ot::crypto::EcdsaCurve::ed25519, Role::Sign);
        }
    }())
    , secp_(get_key(api_, ot::crypto::EcdsaCurve::secp256k1, Role::Sign))
    , secp_hd_([&] {
        if (have_hd_) {

            return get_hd_key(
                api_, seed_id_, ot::crypto::EcdsaCurve::secp256k1);
        } else {

            return get_key(api_, ot::crypto::EcdsaCurve::secp256k1, Role::Sign);
        }
    }())
    , secp_2_([&] {
        if (have_hd_) {

            return get_hd_key(
                api_, seed_id_, ot::crypto::EcdsaCurve::secp256k1, 1);
        } else {

            return get_key(api_, ot::crypto::EcdsaCurve::secp256k1, Role::Sign);
        }
    }())
    , rsa_sign_1_(get_key(api_, ot::crypto::EcdsaCurve::invalid, Role::Sign))
    , rsa_sign_2_(get_key(api_, ot::crypto::EcdsaCurve::invalid, Role::Sign))
{
    opentxs::assert_true(plaintext_1_ != plaintext_2_);
}

auto Signatures::get_hd_key(
    const ot::api::session::Client& api,
    const ot::crypto::SeedID& seedID,
    const ot::crypto::EcdsaCurve& curve,
    const std::uint32_t index) -> ot::crypto::asymmetric::Key
{
    auto reason = api.Factory().PasswordPrompt(__func__);

    return api.Crypto().Seed().GetHDKey(
        seedID,
        curve,
        {ot::HDIndex{
             ot::crypto::Bip43Purpose::NYM, ot::crypto::Bip32Child::HARDENED},
         ot::HDIndex{0, ot::crypto::Bip32Child::HARDENED},
         ot::HDIndex{0, ot::crypto::Bip32Child::HARDENED},
         ot::HDIndex{index, ot::crypto::Bip32Child::HARDENED},
         ot::HDIndex{
             ot::crypto::Bip32Child::SIGN_KEY,
             ot::crypto::Bip32Child::HARDENED}},
        reason);
}

auto Signatures::get_key(
    const ot::api::session::Client& api,
    const ot::crypto::EcdsaCurve curve,
    const Role role) -> ot::crypto::asymmetric::Key
{
    const auto reason = api.Factory().PasswordPrompt(__func__);
    const auto params = [&] {
        if (ot::crypto::EcdsaCurve::secp256k1 == curve) {

            return ot::crypto::Parameters{
                api.Factory(), ot::crypto::ParameterType::secp256k1};
        } else if (ot::crypto::EcdsaCurve::ed25519 == curve) {

            return ot::crypto::Parameters{
                api.Factory(), ot::crypto::ParameterType::ed25519};
        } else {

            return ot::crypto::Parameters{api.Factory(), 1024};
        }
    }();

    return api.Factory().AsymmetricKey(role, params, reason);
}

auto Signatures::test_dh(
    const ot::crypto::AsymmetricProvider& lib,
    const ot::crypto::asymmetric::Key& keyOne,
    const ot::crypto::asymmetric::Key& keyTwo,
    const ot::Data& expected) -> bool
{
    constexpr auto style = ot::crypto::SecretStyle::Default;
    auto reason = api_.Factory().PasswordPrompt(__func__);
    auto secret1 = api_.Factory().Secret(0);
    auto secret2 = api_.Factory().Secret(0);
    auto output = lib.SharedSecret(
        keyOne.PublicKey(), keyTwo.PrivateKey(reason), style, secret1);

    if (false == output) { return output; }

    output = lib.SharedSecret(
        keyTwo.PublicKey(), keyOne.PrivateKey(reason), style, secret2);

    EXPECT_TRUE(output);

    if (false == output) { return output; }

    EXPECT_EQ(secret1, secret2);

    if (0 < expected.size()) {
        EXPECT_EQ(secret1.Bytes(), expected.Bytes());

        output &= (secret1.Bytes() == expected.Bytes());
    }

    return output;
}

auto Signatures::test_signature(
    const ot::Data& plaintext,
    const ot::crypto::AsymmetricProvider& lib,
    const ot::crypto::asymmetric::Key& key,
    const ot::crypto::HashType hash) -> bool
{
    auto reason = api_.Factory().PasswordPrompt(__func__);
    auto sig = ot::Space{};
    const auto pubkey = key.PublicKey();
    const auto seckey = key.PrivateKey(reason);

    EXPECT_TRUE(key.HasPrivate());
    EXPECT_NE(pubkey.size(), 0);
    EXPECT_NE(seckey.size(), 0);

    if (false == key.HasPrivate()) { return false; }
    if ((0 == pubkey.size()) || (0 == seckey.size())) { return false; }

    const auto haveSig =
        lib.Sign(plaintext.Bytes(), seckey, hash, ot::writer(sig));
    const auto verified =
        lib.Verify(plaintext.Bytes(), pubkey, ot::reader(sig), hash);

    return haveSig && verified;
}

auto Signatures::bad_signature(
    const ot::crypto::AsymmetricProvider& lib,
    const ot::crypto::asymmetric::Key& key,
    const ot::crypto::HashType hash) -> bool
{
    auto reason = api_.Factory().PasswordPrompt(__func__);
    auto sig1 = ot::ByteArray{};
    auto sig2 = ot::ByteArray{};
    const auto pubkey = key.PublicKey();
    const auto seckey = key.PrivateKey(reason);

    EXPECT_TRUE(key.HasPrivate());
    EXPECT_NE(pubkey.size(), 0);
    EXPECT_NE(seckey.size(), 0);

    if (false == key.HasPrivate()) { return false; }
    if ((0 == pubkey.size()) || (0 == seckey.size())) { return false; }

    const auto haveSig1 = lib.Sign(
        plaintext_1_.Bytes(), key.PrivateKey(reason), hash, sig1.WriteInto());

    EXPECT_TRUE(haveSig1);

    if (false == haveSig1) { return false; }

    const auto haveSig2 = lib.Sign(
        plaintext_2_.Bytes(), key.PrivateKey(reason), hash, sig2.WriteInto());

    EXPECT_TRUE(haveSig2);

    if (false == haveSig2) { return false; }

    EXPECT_NE(sig1, sig2);

    if (sig1 == sig2) { return false; }

    const auto check1 =
        lib.Verify(plaintext_2_.Bytes(), key.PublicKey(), sig1.Bytes(), hash);

    EXPECT_FALSE(check1);

    if (check1) { return false; }

    const auto check2 =
        lib.Verify(plaintext_1_.Bytes(), key.PublicKey(), sig2.Bytes(), hash);

    EXPECT_FALSE(check2);

    if (check2) { return false; }

    return true;
}
}  // namespace ottest
