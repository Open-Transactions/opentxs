// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

#include "internal/api/Crypto.hpp"
#include "internal/api/session/Client.hpp"
#include "internal/otx/client/obsolete/OTAPI_Exec.hpp"
#include "internal/util/LogMacros.hpp"  // IWYU pragma: keep
#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/api/Context.hpp"
#include "opentxs/api/crypto/Config.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/api/session/Client.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Secret.hpp"
#include "opentxs/crypto/Bip32Child.hpp"
#include "opentxs/crypto/Bip43Purpose.hpp"
#include "opentxs/crypto/HashType.hpp"
#include "opentxs/crypto/ParameterType.hpp"
#include "opentxs/crypto/Parameters.hpp"
#include "opentxs/crypto/SecretStyle.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/HD.hpp"
#include "opentxs/crypto/key/asymmetric/Algorithm.hpp"
#include "opentxs/crypto/key/asymmetric/Role.hpp"
#include "opentxs/crypto/library/AsymmetricProvider.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "util/HDIndex.hpp"  // IWYU pragma: keep

using namespace opentxs;

namespace ot = opentxs;

namespace ottest
{
class Test_Signatures : public ::testing::Test
{
public:
    using Role = ot::crypto::key::asymmetric::Role;
    using Type = ot::crypto::key::asymmetric::Algorithm;

    static const bool have_hd_;
    static const bool have_rsa_;
    static const bool have_secp256k1_;
    static const bool have_ed25519_;

    const ot::api::session::Client& api_;
    const std::string fingerprint_;
    const crypto::HashType sha256_{crypto::HashType::Sha256};
    const crypto::HashType sha512_{crypto::HashType::Sha512};
    const crypto::HashType blake160_{crypto::HashType::Blake2b160};
    const crypto::HashType blake256_{crypto::HashType::Blake2b256};
    const crypto::HashType blake512_{crypto::HashType::Blake2b512};
    const crypto::HashType ripemd160_{crypto::HashType::Ripemd160};
    const std::string plaintext_string_1_{"Test string"};
    const std::string plaintext_string_2_{"Another string"};
    const OTData plaintext_1{
        Data::Factory(plaintext_string_1_.data(), plaintext_string_1_.size())};
    const OTData plaintext_2{
        Data::Factory(plaintext_string_2_.data(), plaintext_string_2_.size())};
    OTAsymmetricKey ed_;
    OTAsymmetricKey ed_hd_;
    OTAsymmetricKey ed_2_;
    OTAsymmetricKey secp_;
    OTAsymmetricKey secp_hd_;
    OTAsymmetricKey secp_2_;
    OTAsymmetricKey rsa_sign_1_;
    OTAsymmetricKey rsa_sign_2_;

    [[maybe_unused]] Test_Signatures()
        : api_(dynamic_cast<const ot::api::session::Client&>(
              ot::Context().StartClientSession(0)))
        , fingerprint_(api_.InternalClient().Exec().Wallet_ImportSeed(
              "response seminar brave tip suit recall often sound stick owner "
              "lottery motion",
              ""))
        , ed_(get_key(api_, EcdsaCurve::ed25519, Role::Sign))
        , ed_hd_([&] {
            if (have_hd_) {

                return get_hd_key(api_, fingerprint_, EcdsaCurve::ed25519);
            } else {

                return get_key(api_, EcdsaCurve::ed25519, Role::Sign);
            }
        }())
        , ed_2_([&] {
            if (have_hd_) {

                return get_hd_key(api_, fingerprint_, EcdsaCurve::ed25519, 1);
            } else {

                return get_key(api_, EcdsaCurve::ed25519, Role::Sign);
            }
        }())
        , secp_(get_key(api_, EcdsaCurve::secp256k1, Role::Sign))
        , secp_hd_([&] {
            if (have_hd_) {

                return get_hd_key(api_, fingerprint_, EcdsaCurve::secp256k1);
            } else {

                return get_key(api_, EcdsaCurve::secp256k1, Role::Sign);
            }
        }())
        , secp_2_([&] {
            if (have_hd_) {

                return get_hd_key(api_, fingerprint_, EcdsaCurve::secp256k1, 1);
            } else {

                return get_key(api_, EcdsaCurve::secp256k1, Role::Sign);
            }
        }())
        , rsa_sign_1_(get_key(api_, EcdsaCurve::invalid, Role::Sign))
        , rsa_sign_2_(get_key(api_, EcdsaCurve::invalid, Role::Sign))
    {
    }

    static OTAsymmetricKey get_hd_key(
        const ot::api::session::Client& api,
        const std::string& fingerprint,
        const EcdsaCurve& curve,
        const std::uint32_t index = 0)
    {
        auto reason = api.Factory().PasswordPrompt(__func__);
        std::string id{fingerprint};

        return OTAsymmetricKey{
            api.Crypto()
                .Seed()
                .GetHDKey(
                    id,
                    curve,
                    {HDIndex{Bip43Purpose::NYM, Bip32Child::HARDENED},
                     HDIndex{0, Bip32Child::HARDENED},
                     HDIndex{0, Bip32Child::HARDENED},
                     HDIndex{index, Bip32Child::HARDENED},
                     HDIndex{Bip32Child::SIGN_KEY, Bip32Child::HARDENED}},
                    reason)
                .release()};
    }
    [[maybe_unused]] static OTAsymmetricKey get_key(
        const ot::api::session::Client& api,
        const EcdsaCurve curve,
        const Role role)
    {
        const auto reason = api.Factory().PasswordPrompt(__func__);
        const auto params = [&] {
            if (EcdsaCurve::secp256k1 == curve) {

                return ot::crypto::Parameters{crypto::ParameterType::secp256k1};
            } else if (EcdsaCurve::ed25519 == curve) {

                return ot::crypto::Parameters{crypto::ParameterType::ed25519};
            } else {

                return ot::crypto::Parameters{1024};
            }
        }();

        return api.Factory().AsymmetricKey(params, reason, role);
    }

    [[maybe_unused]] bool test_dh(
        const crypto::AsymmetricProvider& lib,
        const crypto::key::Asymmetric& keyOne,
        const crypto::key::Asymmetric& keyTwo,
        const ot::Data& expected)
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
            EXPECT_EQ(secret1->Bytes(), expected.Bytes());

            output &= (secret1->Bytes() == expected.Bytes());
        }

        return output;
    }

    [[maybe_unused]] bool test_signature(
        const Data& plaintext,
        const crypto::AsymmetricProvider& lib,
        const crypto::key::Asymmetric& key,
        const crypto::HashType hash)
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
            lib.Sign(plaintext.Bytes(), seckey, hash, writer(sig));
        const auto verified =
            lib.Verify(plaintext.Bytes(), pubkey, reader(sig), hash);

        return haveSig && verified;
    }

    [[maybe_unused]] bool bad_signature(
        const crypto::AsymmetricProvider& lib,
        const crypto::key::Asymmetric& key,
        const crypto::HashType hash)
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
            plaintext_1->Bytes(), key.PrivateKey(reason), hash, writer(sig));

        EXPECT_TRUE(haveSig);

        if (false == haveSig) { return false; }

        const auto verified = lib.Verify(
            plaintext_2->Bytes(), key.PublicKey(), reader(sig), hash);

        EXPECT_FALSE(verified);

        return !verified;
    }
};

const bool Test_Signatures::have_hd_{ot::api::crypto::HaveHDKeys()};
const bool Test_Signatures::have_rsa_{ot::api::crypto::HaveSupport(
    ot::crypto::key::asymmetric::Algorithm::Legacy)};
const bool Test_Signatures::have_secp256k1_{ot::api::crypto::HaveSupport(
    ot::crypto::key::asymmetric::Algorithm::Secp256k1)};
const bool Test_Signatures::have_ed25519_{ot::api::crypto::HaveSupport(
    ot::crypto::key::asymmetric::Algorithm::ED25519)};

TEST_F(Test_Signatures, RSA_unsupported_hash)
{
    if (have_rsa_) {
        const auto& provider =
            api_.Crypto().Internal().AsymmetricProvider(Type::Legacy);

        EXPECT_FALSE(
            test_signature(plaintext_1, provider, rsa_sign_1_, blake160_));
        EXPECT_FALSE(
            test_signature(plaintext_1, provider, rsa_sign_1_, blake256_));
        EXPECT_FALSE(
            test_signature(plaintext_1, provider, rsa_sign_1_, blake512_));
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
            test_signature(plaintext_1, provider, rsa_sign_1_, sha256_));
        EXPECT_TRUE(
            test_signature(plaintext_1, provider, rsa_sign_1_, sha512_));
        EXPECT_TRUE(
            test_signature(plaintext_1, provider, rsa_sign_1_, ripemd160_));
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
                test_signature(plaintext_1, provider, ed_hd_, sha256_));
            EXPECT_FALSE(
                test_signature(plaintext_1, provider, ed_hd_, sha512_));
            EXPECT_FALSE(
                test_signature(plaintext_1, provider, ed_hd_, ripemd160_));
            EXPECT_FALSE(
                test_signature(plaintext_1, provider, ed_hd_, blake160_));
            EXPECT_FALSE(
                test_signature(plaintext_1, provider, ed_hd_, blake512_));
        } else {
            // TODO
        }

        EXPECT_FALSE(test_signature(plaintext_1, provider, ed_, sha256_));
        EXPECT_FALSE(test_signature(plaintext_1, provider, ed_, sha512_));
        EXPECT_FALSE(test_signature(plaintext_1, provider, ed_, ripemd160_));
        EXPECT_FALSE(test_signature(plaintext_1, provider, ed_, blake160_));
        EXPECT_FALSE(test_signature(plaintext_1, provider, ed_, blake512_));
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
                test_signature(plaintext_1, provider, ed_hd_, blake256_));
        } else {
            // TODO
        }

        EXPECT_TRUE(test_signature(plaintext_1, provider, ed_, blake256_));
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
                test_signature(plaintext_1, provider, secp_hd_, sha256_));
            EXPECT_TRUE(
                test_signature(plaintext_1, provider, secp_hd_, sha512_));
            EXPECT_TRUE(
                test_signature(plaintext_1, provider, secp_hd_, blake160_));
            EXPECT_TRUE(
                test_signature(plaintext_1, provider, secp_hd_, blake256_));
            EXPECT_TRUE(
                test_signature(plaintext_1, provider, secp_hd_, blake512_));
            EXPECT_TRUE(
                test_signature(plaintext_1, provider, secp_hd_, ripemd160_));
        } else {
            // TODO
        }

        EXPECT_TRUE(test_signature(plaintext_1, provider, secp_, sha256_));
        EXPECT_TRUE(test_signature(plaintext_1, provider, secp_, sha512_));
        EXPECT_TRUE(test_signature(plaintext_1, provider, secp_, blake256_));
        EXPECT_TRUE(test_signature(plaintext_1, provider, secp_, blake512_));
        EXPECT_TRUE(test_signature(plaintext_1, provider, secp_, blake160_));
        EXPECT_TRUE(test_signature(plaintext_1, provider, secp_, ripemd160_));
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
