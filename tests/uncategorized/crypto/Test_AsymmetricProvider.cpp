// Copyright (c) 2010-2023 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include <gtest/gtest.h>
#include <opentxs/opentxs.hpp>
#include <string_view>

#include "internal/api/Crypto.hpp"
#include "ottest/fixtures/crypto/AsymmetricProvider.hpp"

namespace ottest
{
using namespace std::literals;

TEST_F(Signatures, RSA_unsupported_hash)
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

TEST_F(Signatures, RSA_detect_invalid_signature)
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

TEST_F(Signatures, RSA_supported_hashes)
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

TEST_F(Signatures, RSA_DH)
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

TEST_F(Signatures, Ed25519_unsupported_hash)
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

TEST_F(Signatures, Ed25519_detect_invalid_signature)
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

TEST_F(Signatures, Ed25519_supported_hashes)
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

TEST_F(Signatures, Ed25519_ECDH)
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

TEST_F(Signatures, Secp256k1_detect_invalid_signature)
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

TEST_F(Signatures, Secp256k1_supported_hashes)
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

TEST_F(Signatures, Secp256k1_ECDH)
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
