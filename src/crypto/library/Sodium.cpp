// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/Ed25519.hpp"
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#include "opentxs/crypto/library/Sodium.hpp"
#include "opentxs/OT.hpp"

#if OT_CRYPTO_SUPPORTED_KEY_ED25519
#include "AsymmetricProvider.hpp"
#include "EcdsaProvider.hpp"
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519

extern "C" {
#include <sodium.h>
}

#include <array>

#include "Sodium.hpp"

#define OT_METHOD "opentxs::Sodium::"

namespace opentxs
{
crypto::Sodium* Factory::Sodium(const api::Crypto& crypto)
{
    return new crypto::implementation::Sodium(crypto);
}
}  // namespace opentxs

namespace opentxs::crypto::implementation
{
Sodium::Sodium(const api::Crypto& crypto)
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
    : AsymmetricProvider()
    , EcdsaProvider(crypto)
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
{
    const auto result = ::sodium_init();

    OT_ASSERT(-1 != result);
}

bool Sodium::Decrypt(
    const proto::Ciphertext& ciphertext,
    const std::uint8_t* key,
    const std::size_t keySize,
    std::uint8_t* plaintext) const
{
    const auto& message = ciphertext.data();
    const auto& nonce = ciphertext.iv();
    const auto& mac = ciphertext.tag();
    const auto& mode = ciphertext.mode();

    if (KeySize(mode) != keySize) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect key size.").Flush();

        return false;
    }

    if (IvSize(mode) != nonce.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect nonce size.").Flush();

        return false;
    }

    switch (ciphertext.mode()) {
        case (proto::SMODE_CHACHA20POLY1305): {
            return (
                0 == crypto_aead_chacha20poly1305_ietf_decrypt_detached(
                         plaintext,
                         nullptr,
                         reinterpret_cast<const unsigned char*>(message.data()),
                         message.size(),
                         reinterpret_cast<const unsigned char*>(mac.data()),
                         nullptr,
                         0,
                         reinterpret_cast<const unsigned char*>(nonce.data()),
                         key));
        }
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Unsupported encryption mode (")(mode)(").")
                .Flush();
        }
    }

    return false;
}

bool Sodium::Derive(
    const std::uint8_t* input,
    const std::size_t inputSize,
    const std::uint8_t* salt,
    const std::size_t saltSize,
    const std::uint64_t operations,
    const std::uint64_t difficulty,
    const proto::SymmetricKeyType type,
    std::uint8_t* output,
    std::size_t outputSize) const
{
    const auto requiredSize = SaltSize(type);

    if (requiredSize != saltSize) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect salt size (")(saltSize)(
            "). Required: (")(requiredSize)(").")
            .Flush();

        return false;
    }

    return (
        0 == crypto_pwhash(
                 output,
                 outputSize,
                 reinterpret_cast<const char*>(input),
                 inputSize,
                 salt,
                 operations,
                 difficulty,
                 crypto_pwhash_ALG_ARGON2I13));
}

bool Sodium::Digest(
    const proto::HashType hashType,
    const std::uint8_t* input,
    const size_t inputSize,
    std::uint8_t* output) const
{
    switch (hashType) {
        case (proto::HASHTYPE_BLAKE2B160):
        case (proto::HASHTYPE_BLAKE2B256):
        case (proto::HASHTYPE_BLAKE2B512): {
            return (
                0 == crypto_generichash(
                         output,
                         HashingProvider::HashSize(hashType),
                         input,
                         inputSize,
                         nullptr,
                         0));
        }
        case (proto::HASHTYPE_SHA256): {
            return (0 == crypto_hash_sha256(output, input, inputSize));
        }
        case (proto::HASHTYPE_SHA512): {
            return (0 == crypto_hash_sha512(output, input, inputSize));
        }
        default: {
        }
    }

    LogOutput(OT_METHOD)(__FUNCTION__)(": Unsupported hash function.").Flush();

    return false;
}

#if OT_CRYPTO_SUPPORTED_KEY_ED25519
bool Sodium::ECDH(
    const Data& publicKey,
    const OTPassword& seed,
    OTPassword& secret) const
{
    auto notUsed = Data::Factory();
    OTPassword curvePrivate;

    if (!SeedToCurveKey(seed, curvePrivate, notUsed)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to expand private key.")
            .Flush();

        return false;
    }

    std::array<unsigned char, crypto_scalarmult_curve25519_BYTES> blank{};
    auto curvePublic = Data::Factory(blank.data(), blank.size());
    secret.setMemory(blank.data(), blank.size());
    const bool havePublic = crypto_sign_ed25519_pk_to_curve25519(
        static_cast<unsigned char*>(const_cast<void*>(curvePublic->data())),
        static_cast<const unsigned char*>(publicKey.data()));

    if (0 != havePublic) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to convert public key from ed25519 to curve25519.")
            .Flush();

        return false;
    }

    const auto output = ::crypto_scalarmult(
        static_cast<unsigned char*>(secret.getMemoryWritable()),
        static_cast<const unsigned char*>(curvePrivate.getMemory()),
        static_cast<const unsigned char*>(curvePublic->data()));

    return (0 == output);
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519

bool Sodium::Encrypt(
    const std::uint8_t* input,
    const std::size_t inputSize,
    const std::uint8_t* key,
    const std::size_t keySize,
    proto::Ciphertext& ciphertext) const
{
    OT_ASSERT(nullptr != input);
    OT_ASSERT(nullptr != key);

    const auto& mode = ciphertext.mode();
    const auto& nonce = ciphertext.iv();
    auto& tag = *ciphertext.mutable_tag();
    auto& output = *ciphertext.mutable_data();

    bool result = false;

    if (mode == proto::SMODE_ERROR) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect mode.").Flush();

        return result;
    }

    if (KeySize(mode) != keySize) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect key size.").Flush();

        return result;
    }

    if (IvSize(mode) != nonce.size()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect nonce size.").Flush();

        return result;
    }

    ciphertext.set_version(1);
    tag.resize(TagSize(mode), 0x0);
    output.resize(inputSize, 0x0);

    switch (mode) {
        case (proto::SMODE_CHACHA20POLY1305): {
            return (
                0 == crypto_aead_chacha20poly1305_ietf_encrypt_detached(
                         reinterpret_cast<unsigned char*>(&output.front()),
                         reinterpret_cast<unsigned char*>(&tag.front()),
                         nullptr,
                         input,
                         inputSize,
                         nullptr,
                         0,
                         nullptr,
                         reinterpret_cast<const unsigned char*>(&nonce.front()),
                         key));

            break;
        }
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Unsupported encryption mode (")(mode)(").")
                .Flush();
        }
    }

    return result;
}

#if OT_CRYPTO_SUPPORTED_KEY_ED25519
bool Sodium::ExpandSeed(
    const OTPassword& seed,
    OTPassword& privateKey,
    Data& publicKey) const
{
    if (!seed.isMemory()) { return false; }

    if (crypto_sign_SEEDBYTES != seed.getMemorySize()) { return false; }

    std::array<unsigned char, crypto_sign_SECRETKEYBYTES> secretKeyBlank{};
    privateKey.setMemory(secretKeyBlank.data(), secretKeyBlank.size());
    std::array<unsigned char, crypto_sign_PUBLICKEYBYTES> publicKeyBlank{};
    const auto output = ::crypto_sign_seed_keypair(
        publicKeyBlank.data(),
        static_cast<unsigned char*>(privateKey.getMemoryWritable()),
        static_cast<const unsigned char*>(seed.getMemory()));
    publicKey.Assign(publicKeyBlank.data(), publicKeyBlank.size());

    return (0 == output);
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519

bool Sodium::HMAC(
    const proto::HashType hashType,
    const std::uint8_t* input,
    const size_t inputSize,
    const std::uint8_t* key,
    const size_t keySize,
    std::uint8_t* output) const
{
    switch (hashType) {
        case (proto::HASHTYPE_BLAKE2B160):
        case (proto::HASHTYPE_BLAKE2B256):
        case (proto::HASHTYPE_BLAKE2B512): {
            return (
                0 == crypto_generichash(
                         output,
                         HashingProvider::HashSize(hashType),
                         input,
                         inputSize,
                         key,
                         keySize));
        }
        case (proto::HASHTYPE_SHA256): {
            if (crypto_auth_hmacsha256_KEYBYTES != keySize) {
                LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect key size.")
                    .Flush();

                return false;
            }

            return (0 == crypto_auth_hmacsha256(output, input, inputSize, key));
        }
        case (proto::HASHTYPE_SHA512): {
            if (crypto_auth_hmacsha512_KEYBYTES != keySize) {
                LogOutput(OT_METHOD)(__FUNCTION__)(": Incorrect key size.")
                    .Flush();

                return false;
            }

            return (0 == crypto_auth_hmacsha512(output, input, inputSize, key));
        }
        default: {
        }
    }

    LogOutput(OT_METHOD)(__FUNCTION__)(": Unsupported hash function.").Flush();

    return false;
}

std::size_t Sodium::IvSize(const proto::SymmetricMode mode) const
{
    switch (mode) {
        case (proto::SMODE_CHACHA20POLY1305): {
            return crypto_aead_chacha20poly1305_IETF_NPUBBYTES;
        }
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Unsupported encryption mode (")(mode)(").")
                .Flush();
        }
    }
    return 0;
}

std::size_t Sodium::KeySize(const proto::SymmetricMode mode) const
{
    switch (mode) {
        case (proto::SMODE_CHACHA20POLY1305): {
            return crypto_aead_chacha20poly1305_IETF_KEYBYTES;
        }
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Unsupported encryption mode (")(mode)(").")
                .Flush();
        }
    }
    return 0;
}

#if OT_CRYPTO_SUPPORTED_KEY_ED25519
bool Sodium::RandomKeypair(OTPassword& privateKey, Data& publicKey) const
{
    OTPassword notUsed;
    privateKey.randomizeMemory(crypto_sign_SEEDBYTES);

    return ExpandSeed(privateKey, notUsed, publicKey);
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519

bool Sodium::RandomizeMemory(void* destination, const std::size_t size) const
{
    ::randombytes_buf(destination, size);

    return true;
}

std::size_t Sodium::SaltSize(const proto::SymmetricKeyType type) const
{
    switch (type) {
        case (proto::SKEYTYPE_ARGON2): {

            return crypto_pwhash_SALTBYTES;
        }
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unsupported key type (")(
                type)(").")
                .Flush();
        }
    }

    return 0;
}

#if OT_CRYPTO_SUPPORTED_KEY_ED25519
bool Sodium::ScalarBaseMultiply(const OTPassword& seed, Data& publicKey) const
{
    OTPassword notUsed;

    return ExpandSeed(seed, notUsed, publicKey);
}

bool Sodium::SeedToCurveKey(
    const OTPassword& seed,
    OTPassword& privateKey,
    Data& publicKey) const
{
    auto intermediatePublic = Data::Factory();
    ;
    OTPassword intermediatePrivate;

    if (!ExpandSeed(seed, intermediatePrivate, intermediatePublic)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to expand seed.").Flush();

        return false;
    }

    std::array<unsigned char, crypto_scalarmult_curve25519_BYTES> blank{};
    privateKey.setMemory(blank.data(), blank.size());
    const bool havePrivate = crypto_sign_ed25519_sk_to_curve25519(
        static_cast<unsigned char*>(privateKey.getMemoryWritable()),
        static_cast<const unsigned char*>(intermediatePrivate.getMemory()));

    if (0 != havePrivate) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to convert private key from ed25519 to curve25519.")
            .Flush();

        return false;
    }

    const bool havePublic = crypto_sign_ed25519_pk_to_curve25519(
        blank.data(),
        static_cast<const unsigned char*>(intermediatePublic->data()));

    if (0 != havePublic) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to convert public key from ed25519 to curve25519.")
            .Flush();

        return false;
    }

    publicKey.Assign(blank.data(), blank.size());

    OT_ASSERT(crypto_scalarmult_BYTES == publicKey.size());
    OT_ASSERT(crypto_scalarmult_SCALARBYTES == privateKey.getMemorySize());

    return true;
}

bool Sodium::Sign(
    const Data& plaintext,
    const key::Asymmetric& theKey,
    const proto::HashType hashType,
    Data& signature,
    const OTPasswordData* pPWData,
    const OTPassword* exportPassword) const
{
    if (proto::HASHTYPE_BLAKE2B256 != hashType) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid hash function: ")(
            hashType)(".")
            .Flush();

        return false;
    }

    OTPassword seed;
    bool havePrivateKey = false;

    // FIXME
    OT_ASSERT_MSG(nullptr == exportPassword, "This case is not yet handled.");

    const crypto::key::EllipticCurve* key =
        dynamic_cast<const key::Ed25519*>(&theKey);

    if (nullptr == key) { return false; }

    if (nullptr == pPWData) {
        OTPasswordData passwordData(
            "Please enter your password to sign this  document.");
        havePrivateKey = AsymmetricKeyToECPrivatekey(*key, passwordData, seed);
    } else {
        havePrivateKey = AsymmetricKeyToECPrivatekey(*key, *pPWData, seed);
    }

    if (!havePrivateKey) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Can not extract ed25519 private key seed "
            "from Asymmetric.")
            .Flush();

        return false;
    }

    auto notUsed = Data::Factory();
    OTPassword privKey;
    const bool keyExpanded = ExpandSeed(seed, privKey, notUsed);

    if (!keyExpanded) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Can not expand ed25519 private key from seed.")
            .Flush();

        return false;
    }

    std::array<unsigned char, crypto_sign_BYTES> sig{};
    const auto output = ::crypto_sign_detached(
        sig.data(),
        nullptr,
        static_cast<const unsigned char*>(plaintext.data()),
        plaintext.size(),
        static_cast<const unsigned char*>(privKey.getMemory()));

    if (0 == output) {
        signature.Assign(sig.data(), sig.size());

        return true;
    }

    LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to sign plaintext.").Flush();

    return false;
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519

std::size_t Sodium::TagSize(const proto::SymmetricMode mode) const
{
    switch (mode) {
        case (proto::SMODE_CHACHA20POLY1305): {
            return crypto_aead_chacha20poly1305_IETF_ABYTES;
        }
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Unsupported encryption mode (")(mode)(").")
                .Flush();
        }
    }
    return 0;
}

#if OT_CRYPTO_SUPPORTED_KEY_ED25519
bool Sodium::Verify(
    const Data& plaintext,
    const key::Asymmetric& theKey,
    const Data& signature,
    const proto::HashType hashType,
    __attribute__((unused)) const OTPasswordData* pPWData) const
{
    if (proto::HASHTYPE_BLAKE2B256 != hashType) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Invalid hash function: ")(
            hashType)(".")
            .Flush();

        return false;
    }

    const crypto::key::EllipticCurve* key =
        dynamic_cast<const key::Ed25519*>(&theKey);

    if (nullptr == key) { return false; }

    auto pubkey = Data::Factory();
    ;
    const bool havePublicKey = AsymmetricKeyToECPubkey(*key, pubkey);

    if (!havePublicKey) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Can not extract ed25519 public key from "
            "Asymmetric.")
            .Flush();

        return false;
    }

    const auto output = ::crypto_sign_verify_detached(
        static_cast<const unsigned char*>(signature.data()),
        static_cast<const unsigned char*>(plaintext.data()),
        plaintext.size(),
        static_cast<const unsigned char*>(pubkey->data()));

    if (0 == output) { return true; }

    // I made this "info" since it's not necessarily an
    // error. Perhaps someone tried to verify 3 signatures
    // so he could find the right one. Metadata can be used
    // to avoid these extra, unnecessary sig verifications.
    //
    LogVerbose(OT_METHOD)(__FUNCTION__)(": Failed to verify signature.")
        .Flush();

    return false;
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
}  // namespace opentxs::crypto::implementation
