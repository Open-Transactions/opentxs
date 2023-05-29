// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <cstdint>
#include <string_view>

#include "internal/api/Crypto.hpp"
#include "internal/api/session/Client.hpp"
#include "internal/crypto/library/AsymmetricProvider.hpp"
#include "internal/otx/client/obsolete/OTAPI_Exec.hpp"
#include "internal/util/LogMacros.hpp"  // IWYU pragma: keep
#include "ottest/env/OTTestEnvironment.hpp"
#include "util/HDIndex.hpp"  // IWYU pragma: keep

namespace ot = opentxs;

namespace ottest
{
class Test_Signatures : public ::testing::Test
{
public:
    using Role = ot::crypto::asymmetric::Role;
    using Type = ot::crypto::asymmetric::Algorithm;

    static const bool have_hd_;
    static const bool have_rsa_;
    static const bool have_secp256k1_;
    static const bool have_ed25519_;

    const ot::api::session::Client& api_;
    const ot::UnallocatedCString fingerprint_;
    const ot::crypto::HashType sha256_{ot::crypto::HashType::Sha256};
    const ot::crypto::HashType sha512_{ot::crypto::HashType::Sha512};
    const ot::crypto::HashType blake160_{ot::crypto::HashType::Blake2b160};
    const ot::crypto::HashType blake256_{ot::crypto::HashType::Blake2b256};
    const ot::crypto::HashType blake512_{ot::crypto::HashType::Blake2b512};
    const ot::crypto::HashType ripemd160_{ot::crypto::HashType::Ripemd160};
    const ot::UnallocatedCString plaintext_string_1_{"Test string"};
    const ot::UnallocatedCString plaintext_string_2_{"Another string"};
    const ot::ByteArray plaintext_1_{
        plaintext_string_1_.data(),
        plaintext_string_1_.size()};
    const ot::ByteArray plaintext_2_{
        plaintext_string_2_.data(),
        plaintext_string_2_.size()};
    ot::crypto::asymmetric::Key ed_;
    ot::crypto::asymmetric::Key ed_hd_;
    ot::crypto::asymmetric::Key ed_2_;
    ot::crypto::asymmetric::Key secp_;
    ot::crypto::asymmetric::Key secp_hd_;
    ot::crypto::asymmetric::Key secp_2_;
    ot::crypto::asymmetric::Key rsa_sign_1_;
    ot::crypto::asymmetric::Key rsa_sign_2_;

    [[maybe_unused]] Test_Signatures()
        : api_(dynamic_cast<const ot::api::session::Client&>(
              OTTestEnvironment::GetOT().StartClientSession(0)))
        , fingerprint_(api_.InternalClient().Exec().Wallet_ImportSeed(
              "response seminar brave tip suit recall often sound stick owner "
              "lottery motion",
              ""))
        , ed_(get_key(api_, ot::crypto::EcdsaCurve::ed25519, Role::Sign))
        , ed_hd_([&] {
            if (have_hd_) {

                return get_hd_key(
                    api_, fingerprint_, ot::crypto::EcdsaCurve::ed25519);
            } else {

                return get_key(
                    api_, ot::crypto::EcdsaCurve::ed25519, Role::Sign);
            }
        }())
        , ed_2_([&] {
            if (have_hd_) {

                return get_hd_key(
                    api_, fingerprint_, ot::crypto::EcdsaCurve::ed25519, 1);
            } else {

                return get_key(
                    api_, ot::crypto::EcdsaCurve::ed25519, Role::Sign);
            }
        }())
        , secp_(get_key(api_, ot::crypto::EcdsaCurve::secp256k1, Role::Sign))
        , secp_hd_([&] {
            if (have_hd_) {

                return get_hd_key(
                    api_, fingerprint_, ot::crypto::EcdsaCurve::secp256k1);
            } else {

                return get_key(
                    api_, ot::crypto::EcdsaCurve::secp256k1, Role::Sign);
            }
        }())
        , secp_2_([&] {
            if (have_hd_) {

                return get_hd_key(
                    api_, fingerprint_, ot::crypto::EcdsaCurve::secp256k1, 1);
            } else {

                return get_key(
                    api_, ot::crypto::EcdsaCurve::secp256k1, Role::Sign);
            }
        }())
        , rsa_sign_1_(
              get_key(api_, ot::crypto::EcdsaCurve::invalid, Role::Sign))
        , rsa_sign_2_(
              get_key(api_, ot::crypto::EcdsaCurve::invalid, Role::Sign))
    {
    }

    static auto get_hd_key(
        const ot::api::session::Client& api,
        const ot::UnallocatedCString& fingerprint,
        const ot::crypto::EcdsaCurve& curve,
        const std::uint32_t index = 0) -> ot::crypto::asymmetric::Key
    {
        auto reason = api.Factory().PasswordPrompt(__func__);
        ot::UnallocatedCString id{fingerprint};

        return api.Crypto().Seed().GetHDKey(
            id,
            curve,
            {ot::HDIndex{ot::Bip43Purpose::NYM, ot::Bip32Child::HARDENED},
             ot::HDIndex{0, ot::Bip32Child::HARDENED},
             ot::HDIndex{0, ot::Bip32Child::HARDENED},
             ot::HDIndex{index, ot::Bip32Child::HARDENED},
             ot::HDIndex{ot::Bip32Child::SIGN_KEY, ot::Bip32Child::HARDENED}},
            reason);
    }
    [[maybe_unused]] static auto get_key(
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

    [[maybe_unused]] auto test_dh(
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

    [[maybe_unused]] auto test_signature(
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

    [[maybe_unused]] auto bad_signature(
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

        const auto haveSig = lib.Sign(
            plaintext_1_.Bytes(),
            key.PrivateKey(reason),
            hash,
            ot::writer(sig));

        EXPECT_TRUE(haveSig);

        if (false == haveSig) { return false; }

        const auto verified = lib.Verify(
            plaintext_2_.Bytes(), key.PublicKey(), ot::reader(sig), hash);

        EXPECT_FALSE(verified);

        return !verified;
    }
};

const bool Test_Signatures::have_hd_{ot::api::crypto::HaveHDKeys()};
const bool Test_Signatures::have_rsa_{
    ot::api::crypto::HaveSupport(ot::crypto::asymmetric::Algorithm::Legacy)};
const bool Test_Signatures::have_secp256k1_{
    ot::api::crypto::HaveSupport(ot::crypto::asymmetric::Algorithm::Secp256k1)};
const bool Test_Signatures::have_ed25519_{
    ot::api::crypto::HaveSupport(ot::crypto::asymmetric::Algorithm::ED25519)};

TEST_F(Test_Signatures, RSA_unsupported_hash)
{
    if (have_rsa_) {
        const auto& provider =
            api_.Crypto().Internal().AsymmetricProvider(Type::Legacy);

        EXPECT_FALSE(
            test_signature(plaintext_1_, provider, rsa_sign_1_, blake160_));
        EXPECT_FALSE(
            test_signature(plaintext_1_, provider, rsa_sign_1_, blake256_));
        EXPECT_FALSE(
            test_signature(plaintext_1_, provider, rsa_sign_1_, blake512_));
    } else {
        // TODO
    }
}

TEST_F(Test_Signatures, RSA_detect_invalid_signature)
{
    if (have_rsa_) {
        const auto& provider =
            api_.Crypto().Internal().AsymmetricProvider(Type::Legacy);

        EXPECT_TRUE(bad_signature(provider, rsa_sign_1_, sha256_));
        EXPECT_TRUE(bad_signature(provider, rsa_sign_1_, sha512_));
        EXPECT_TRUE(bad_signature(provider, rsa_sign_1_, ripemd160_));
    } else {
        // TODO
    }
}

TEST_F(Test_Signatures, RSA_supported_hashes)
{
    if (have_rsa_) {
        const auto& provider =
            api_.Crypto().Internal().AsymmetricProvider(Type::Legacy);

        EXPECT_TRUE(
            test_signature(plaintext_1_, provider, rsa_sign_1_, sha256_));
        EXPECT_TRUE(
            test_signature(plaintext_1_, provider, rsa_sign_1_, sha512_));
        EXPECT_TRUE(
            test_signature(plaintext_1_, provider, rsa_sign_1_, ripemd160_));
    } else {
        // TODO
    }
}

TEST_F(Test_Signatures, RSA_DH)
{
    if (have_rsa_) {
        const auto& provider =
            api_.Crypto().Internal().AsymmetricProvider(Type::Legacy);
        const auto expected = api_.Factory().Data();

        // RSA does not use the same key for encryption/signing and DH so this
        // test should fail
        EXPECT_FALSE(test_dh(provider, rsa_sign_1_, rsa_sign_2_, expected));
    } else {
        // TODO
    }
}

TEST_F(Test_Signatures, Ed25519_unsupported_hash)
{
    if (have_ed25519_) {
        const auto& provider =
            api_.Crypto().Internal().AsymmetricProvider(Type::ED25519);

        if (have_hd_) {
            EXPECT_FALSE(
                test_signature(plaintext_1_, provider, ed_hd_, sha256_));
            EXPECT_FALSE(
                test_signature(plaintext_1_, provider, ed_hd_, sha512_));
            EXPECT_FALSE(
                test_signature(plaintext_1_, provider, ed_hd_, ripemd160_));
            EXPECT_FALSE(
                test_signature(plaintext_1_, provider, ed_hd_, blake160_));
            EXPECT_FALSE(
                test_signature(plaintext_1_, provider, ed_hd_, blake512_));
        } else {
            // TODO
        }

        EXPECT_FALSE(test_signature(plaintext_1_, provider, ed_, sha256_));
        EXPECT_FALSE(test_signature(plaintext_1_, provider, ed_, sha512_));
        EXPECT_FALSE(test_signature(plaintext_1_, provider, ed_, ripemd160_));
        EXPECT_FALSE(test_signature(plaintext_1_, provider, ed_, blake160_));
        EXPECT_FALSE(test_signature(plaintext_1_, provider, ed_, blake512_));
    } else {
        // TODO
    }
}

TEST_F(Test_Signatures, Ed25519_detect_invalid_signature)
{
    if (have_ed25519_) {
        const auto& provider =
            api_.Crypto().Internal().AsymmetricProvider(Type::ED25519);

        if (have_hd_) {
            EXPECT_TRUE(bad_signature(provider, ed_hd_, blake256_));
        } else {
            // TODO
        }

        EXPECT_TRUE(bad_signature(provider, ed_, blake256_));
    } else {
        // TODO
    }
}

TEST_F(Test_Signatures, Ed25519_supported_hashes)
{
    if (have_ed25519_) {
        const auto& provider =
            api_.Crypto().Internal().AsymmetricProvider(Type::ED25519);

        if (have_hd_) {
            EXPECT_TRUE(
                test_signature(plaintext_1_, provider, ed_hd_, blake256_));
        } else {
            // TODO
        }

        EXPECT_TRUE(test_signature(plaintext_1_, provider, ed_, blake256_));
    } else {
        // TODO
    }
}

TEST_F(Test_Signatures, Ed25519_ECDH)
{
    if (have_ed25519_) {
        const auto& provider =
            api_.Crypto().Internal().AsymmetricProvider(Type::ED25519);
        const auto expected = api_.Factory().Data();

        EXPECT_TRUE(test_dh(provider, ed_, ed_2_, expected));
    } else {
        // TODO
    }
}

TEST_F(Test_Signatures, Secp256k1_detect_invalid_signature)
{
    if (have_secp256k1_) {
        const auto& provider =
            api_.Crypto().Internal().AsymmetricProvider(Type::Secp256k1);

        if (have_hd_) {
            EXPECT_TRUE(bad_signature(provider, secp_hd_, sha256_));
            EXPECT_TRUE(bad_signature(provider, secp_hd_, sha512_));
            EXPECT_TRUE(bad_signature(provider, secp_hd_, blake160_));
            EXPECT_TRUE(bad_signature(provider, secp_hd_, blake256_));
            EXPECT_TRUE(bad_signature(provider, secp_hd_, blake512_));
            EXPECT_TRUE(bad_signature(provider, secp_hd_, ripemd160_));
        } else {
            // TODO
        }

        EXPECT_TRUE(bad_signature(provider, secp_, sha256_));
        EXPECT_TRUE(bad_signature(provider, secp_, sha512_));
        EXPECT_TRUE(bad_signature(provider, secp_, blake160_));
        EXPECT_TRUE(bad_signature(provider, secp_, blake256_));
        EXPECT_TRUE(bad_signature(provider, secp_, blake512_));
        EXPECT_TRUE(bad_signature(provider, secp_, ripemd160_));
    } else {
        // TODO
    }
}

TEST_F(Test_Signatures, Secp256k1_supported_hashes)
{
    if (have_secp256k1_) {
        const auto& provider =
            api_.Crypto().Internal().AsymmetricProvider(Type::Secp256k1);

        if (have_hd_) {
            EXPECT_TRUE(
                test_signature(plaintext_1_, provider, secp_hd_, sha256_));
            EXPECT_TRUE(
                test_signature(plaintext_1_, provider, secp_hd_, sha512_));
            EXPECT_TRUE(
                test_signature(plaintext_1_, provider, secp_hd_, blake160_));
            EXPECT_TRUE(
                test_signature(plaintext_1_, provider, secp_hd_, blake256_));
            EXPECT_TRUE(
                test_signature(plaintext_1_, provider, secp_hd_, blake512_));
            EXPECT_TRUE(
                test_signature(plaintext_1_, provider, secp_hd_, ripemd160_));
        } else {
            // TODO
        }

        EXPECT_TRUE(test_signature(plaintext_1_, provider, secp_, sha256_));
        EXPECT_TRUE(test_signature(plaintext_1_, provider, secp_, sha512_));
        EXPECT_TRUE(test_signature(plaintext_1_, provider, secp_, blake256_));
        EXPECT_TRUE(test_signature(plaintext_1_, provider, secp_, blake512_));
        EXPECT_TRUE(test_signature(plaintext_1_, provider, secp_, blake160_));
        EXPECT_TRUE(test_signature(plaintext_1_, provider, secp_, ripemd160_));
    } else {
        // TODO
    }
}

TEST_F(Test_Signatures, Secp256k1_ECDH)
{
    if (have_secp256k1_) {
        const auto& provider =
            api_.Crypto().Internal().AsymmetricProvider(Type::Secp256k1);

        const auto expected = api_.Factory().Data();

        EXPECT_TRUE(test_dh(provider, secp_, secp_2_, expected));
    } else {
        // TODO
    }
}
}  // namespace ottest
