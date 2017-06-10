/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#include "opentxs/core/stdafx.hpp"

#include "opentxs/core/crypto/Libsodium.hpp"

#include "opentxs/api/OT.hpp"
#include "opentxs/core/crypto/AsymmetricKeyEd25519.hpp"
#include "opentxs/core/crypto/CryptoEngine.hpp"
#include "opentxs/core/crypto/CryptoSymmetric.hpp"
#include "opentxs/core/crypto/OTAsymmetricKey.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"

#include <array>

extern "C" {
#include <sodium.h>
}

#define OT_METHOD "opentxs::Libsodium::"

namespace opentxs
{
void Libsodium::Init_Override() const
{
    auto result = ::sodium_init();

    OT_ASSERT(0 == result);
}

bool Libsodium::Decrypt(
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
        otErr << OT_METHOD << __FUNCTION__ << ": Incorrect key size."
              << std::endl;

        return false;
    }

    if (IvSize(mode) != nonce.size()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Incorrect nonce size."
              << std::endl;

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
            otErr << OT_METHOD << __FUNCTION__
                  << ": Unsupported encryption mode (" << mode << ")"
                  << std::endl;
        }
    }

    return false;
}

bool Libsodium::Derive(
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
        otErr << OT_METHOD << __FUNCTION__ << ": Incorrect salt size ("
              << saltSize << "). "
              << "Required: (" << requiredSize << ")." << std::endl;

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
                 crypto_pwhash_ALG_DEFAULT));
}

bool Libsodium::Digest(
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
                         CryptoHash::HashSize(hashType),
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

    otErr << OT_METHOD << __FUNCTION__ << ": Unsupported hash function."
          << std::endl;

    return false;
}

bool Libsodium::ECDH(
    const Data& publicKey,
    const OTPassword& seed,
    OTPassword& secret) const
{
    Data notUsed;
    OTPassword curvePrivate;

    if (!SeedToCurveKey(seed, curvePrivate, notUsed)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to expand private key."
              << std::endl;

        return false;
    }

    std::array<unsigned char, crypto_scalarmult_curve25519_BYTES> blank{};
    Data curvePublic(blank.data(), blank.size());
    secret.setMemory(blank.data(), blank.size());
    const bool havePublic = crypto_sign_ed25519_pk_to_curve25519(
        static_cast<unsigned char*>(
            const_cast<void*>(curvePublic.GetPointer())),
        static_cast<const unsigned char*>(publicKey.GetPointer()));

    if (0 != havePublic) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to convert public key from ed25519 to curve25519."
              << std::endl;

        return false;
    }

    const auto output = ::crypto_scalarmult(
        static_cast<unsigned char*>(secret.getMemoryWritable()),
        static_cast<const unsigned char*>(curvePrivate.getMemory()),
        static_cast<const unsigned char*>(curvePublic.GetPointer()));

    return (0 == output);
}

bool Libsodium::Encrypt(
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
        otErr << OT_METHOD << __FUNCTION__ << ": Incorrect mode." << std::endl;

        return result;
    }

    if (KeySize(mode) != keySize) {
        otErr << OT_METHOD << __FUNCTION__ << ": Incorrect key size."
              << std::endl;

        return result;
    }

    if (IvSize(mode) != nonce.size()) {
        otErr << OT_METHOD << __FUNCTION__ << ": Incorrect nonce size."
              << std::endl;

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
            otErr << OT_METHOD << __FUNCTION__
                  << ": Unsupported encryption mode (" << mode << ")"
                  << std::endl;
        }
    }

    return result;
}

bool Libsodium::ExpandSeed(
    const OTPassword& seed,
    OTPassword& privateKey,
    Data& publicKey) const
{
    if (!seed.isMemory()) {
        return false;
    }

    if (crypto_sign_SEEDBYTES != seed.getMemorySize()) {
        return false;
    }

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

bool Libsodium::HMAC(
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
                         CryptoHash::HashSize(hashType),
                         input,
                         inputSize,
                         key,
                         keySize));
        }
        case (proto::HASHTYPE_SHA256): {
            if (crypto_auth_hmacsha256_KEYBYTES != keySize) {
                otErr << OT_METHOD << __FUNCTION__ << ": Incorrect key size."
                      << std::endl;

                return false;
            }

            return (0 == crypto_auth_hmacsha256(output, input, inputSize, key));
        }
        case (proto::HASHTYPE_SHA512): {
            if (crypto_auth_hmacsha512_KEYBYTES != keySize) {
                otErr << OT_METHOD << __FUNCTION__ << ": Incorrect key size."
                      << std::endl;

                return false;
            }

            return (0 == crypto_auth_hmacsha512(output, input, inputSize, key));
        }
        default: {
        }
    }

    otErr << OT_METHOD << __FUNCTION__ << ": Unsupported hash function."
          << std::endl;

    return false;
}

std::size_t Libsodium::IvSize(const proto::SymmetricMode mode) const
{
    switch (mode) {
        case (proto::SMODE_CHACHA20POLY1305): {
            return crypto_aead_chacha20poly1305_IETF_NPUBBYTES;
        }
        default: {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Unsupported encryption mode (" << mode << ")"
                  << std::endl;
        }
    }
    return 0;
}

std::size_t Libsodium::KeySize(const proto::SymmetricMode mode) const
{
    switch (mode) {
        case (proto::SMODE_CHACHA20POLY1305): {
            return crypto_aead_chacha20poly1305_IETF_KEYBYTES;
        }
        default: {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Unsupported encryption mode (" << mode << ")"
                  << std::endl;
        }
    }
    return 0;
}

bool Libsodium::RandomKeypair(OTPassword& privateKey, Data& publicKey) const
{
    OTPassword notUsed;
    privateKey.randomizeMemory(crypto_sign_SEEDBYTES);

    return ExpandSeed(privateKey, notUsed, publicKey);
}

std::size_t Libsodium::SaltSize(const proto::SymmetricKeyType type) const
{
    switch (type) {
        case (proto::SKEYTYPE_ARGON2): {

            return crypto_pwhash_SALTBYTES;
        }
        default: {
            otErr << OT_METHOD << __FUNCTION__ << ": Unsupported key type ("
                  << type << ")" << std::endl;
        }
    }

    return 0;
}

bool Libsodium::ScalarBaseMultiply(const OTPassword& seed, Data& publicKey)
    const
{
    OTPassword notUsed;

    return ExpandSeed(seed, notUsed, publicKey);
}

bool Libsodium::SeedToCurveKey(
    const OTPassword& seed,
    OTPassword& privateKey,
    Data& publicKey) const
{
    Data intermediatePublic;
    OTPassword intermediatePrivate;

    if (!ExpandSeed(seed, intermediatePrivate, intermediatePublic)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to expand seed."
              << std::endl;

        return false;
    }

    std::array<unsigned char, crypto_scalarmult_curve25519_BYTES> blank{};
    privateKey.setMemory(blank.data(), blank.size());
    const bool havePrivate = crypto_sign_ed25519_sk_to_curve25519(
        static_cast<unsigned char*>(privateKey.getMemoryWritable()),
        static_cast<const unsigned char*>(intermediatePrivate.getMemory()));

    if (0 != havePrivate) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to convert private key from ed25519 to curve25519."
              << std::endl;

        return false;
    }

    const bool havePublic = crypto_sign_ed25519_pk_to_curve25519(
        blank.data(),
        static_cast<const unsigned char*>(intermediatePublic.GetPointer()));

    if (0 != havePublic) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed to convert public key from ed25519 to curve25519."
              << std::endl;

        return false;
    }

    publicKey.Assign(blank.data(), blank.size());

    OT_ASSERT(crypto_scalarmult_BYTES == publicKey.GetSize());
    OT_ASSERT(crypto_scalarmult_SCALARBYTES == privateKey.getMemorySize());

    return true;
}

bool Libsodium::Sign(
    const Data& plaintext,
    const OTAsymmetricKey& theKey,
    const proto::HashType hashType,
    Data& signature,
    const OTPasswordData* pPWData,
    const OTPassword* exportPassword) const
{
    if (proto::HASHTYPE_BLAKE2B256 != hashType) {
        otErr << __FUNCTION__ << ": Invalid hash function: "
              << CryptoHash::HashTypeToString(hashType) << std::endl;

        return false;
    }

    OTPassword seed;
    bool havePrivateKey = false;

    // FIXME
    OT_ASSERT_MSG(nullptr == exportPassword, "This case is not yet handled.");

    const AsymmetricKeyEC* key =
        dynamic_cast<const AsymmetricKeyEd25519*>(&theKey);

    if (nullptr == key) {
        return false;
    }

    if (nullptr == pPWData) {
        OTPasswordData passwordData(
            "Please enter your password to sign this  document.");
        havePrivateKey = AsymmetricKeyToECPrivatekey(*key, passwordData, seed);
    } else {
        havePrivateKey = AsymmetricKeyToECPrivatekey(*key, *pPWData, seed);
    }

    if (!havePrivateKey) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Can not extract ed25519 private key seed "
              << "from OTAsymmetricKey." << std::endl;

        return false;
    }

    Data notUsed;
    OTPassword privKey;
    const bool keyExpanded = ExpandSeed(seed, privKey, notUsed);

    if (!keyExpanded) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Can not expand ed25519 private key from seed." << std::endl;

        return false;
    }

    std::array<unsigned char, crypto_sign_BYTES> sig{};
    const auto output = ::crypto_sign_detached(
        sig.data(),
        nullptr,
        static_cast<const unsigned char*>(plaintext.GetPointer()),
        plaintext.GetSize(),
        static_cast<const unsigned char*>(privKey.getMemory()));

    if (0 == output) {
        signature.Assign(sig.data(), sig.size());

        return true;
    }

    otErr << OT_METHOD << __FUNCTION__ << ": Failed to sign plaintext."
          << std::endl;

    return false;
}

std::size_t Libsodium::TagSize(const proto::SymmetricMode mode) const
{
    switch (mode) {
        case (proto::SMODE_CHACHA20POLY1305): {
            return crypto_aead_chacha20poly1305_IETF_ABYTES;
        }
        default: {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Unsupported encryption mode (" << mode << ")"
                  << std::endl;
        }
    }
    return 0;
}

bool Libsodium::Verify(
    const Data& plaintext,
    const OTAsymmetricKey& theKey,
    const Data& signature,
    const proto::HashType hashType,
    __attribute__((unused)) const OTPasswordData* pPWData) const
{
    if (proto::HASHTYPE_BLAKE2B256 != hashType) {
        otErr << OT_METHOD << __FUNCTION__ << ": Invalid hash function: "
              << CryptoHash::HashTypeToString(hashType) << std::endl;

        return false;
    }

    const AsymmetricKeyEC* key =
        dynamic_cast<const AsymmetricKeyEd25519*>(&theKey);

    if (nullptr == key) {
        return false;
    }

    Data pubkey;
    const bool havePublicKey = AsymmetricKeyToECPubkey(*key, pubkey);

    if (!havePublicKey) {
        otErr << OT_METHOD << __FUNCTION__
              << ": Can not extract ed25519 public key from OTAsymmetricKey."
              << std::endl;

        return false;
    }

    const auto output = ::crypto_sign_verify_detached(
        static_cast<const unsigned char*>(signature.GetPointer()),
        static_cast<const unsigned char*>(plaintext.GetPointer()),
        plaintext.GetSize(),
        static_cast<const unsigned char*>(pubkey.GetPointer()));

    if (0 == output) {
        return true;
    }

    otErr << OT_METHOD << __FUNCTION__ << ": Failed to verify signature."
          << std::endl;

    return false;
}
}  // namespace opentxs
