// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#if OT_CRYPTO_USING_TREZOR
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/crypto/Util.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/Secp256k1.hpp"
#include "opentxs/crypto/library/AsymmetricProvider.hpp"
#include "opentxs/crypto/library/EcdsaProvider.hpp"
#include "opentxs/crypto/library/LegacySymmetricProvider.hpp"
#include "opentxs/crypto/library/Trezor.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#if OT_CRYPTO_WITH_BIP32
#include "crypto/Bip32.hpp"
#endif
#include "AsymmetricProvider.hpp"
#include "EcdsaProvider.hpp"

extern "C" {
#if OT_CRYPTO_WITH_BIP39
#include <trezor-crypto/bip39.h>
#if OT_CRYPTO_WITH_BIP32
#include <trezor-crypto/bignum.h>
#include <trezor-crypto/bip32.h>
#include <trezor-crypto/curves.h>
#endif
#endif
#include <trezor-crypto/base58.h>
#include <trezor-crypto/ecdsa.h>
#include <trezor-crypto/rand.h>
#include <trezor-crypto/ripemd160.h>
}

#include <array>
#include <cstdint>
#include <memory>
#include <string>

#include "Trezor.hpp"

#define OT_METHOD "opentxs::crypto::implementation::Trezor::"

extern "C" {
uint32_t random32(void)
{
    uint32_t output{0};
    const auto done = opentxs::OT::App().Crypto().Util().RandomizeMemory(
        &output, sizeof(output));

    OT_ASSERT(done)

    return output;
}
}

namespace opentxs
{
crypto::Trezor* Factory::Trezor(const api::Crypto& crypto)
{
    return new crypto::implementation::Trezor(crypto);
}
}  // namespace opentxs

namespace opentxs::crypto::implementation
{
Trezor::Trezor(const api::Crypto& crypto)
#if OT_CRYPTO_WITH_BIP32
    : Bip32()
#endif
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#if OT_CRYPTO_WITH_BIP32
    ,
#else
    :
#endif
    EcdsaProvider(crypto)
#endif
{
#if OT_CRYPTO_WITH_BIP32
    secp256k1_ = get_curve_by_name(CurveName(EcdsaCurve::SECP256K1).c_str());
    OT_ASSERT(nullptr != secp256k1_);
#endif
}

#if OT_CRYPTO_WITH_BIP39
bool Trezor::SeedToWords(const OTPassword& seed, OTPassword& words) const
{
    return words.setPassword(std::string(::mnemonic_from_data(
        static_cast<const std::uint8_t*>(seed.getMemory()),
        seed.getMemorySize())));
}

void Trezor::WordsToSeed(
    const OTPassword& words,
    OTPassword& seed,
    const OTPassword& passphrase) const
{
    OT_ASSERT(words.isPassword());
    OT_ASSERT(passphrase.isPassword());

    seed.SetSize(512 / 8);

    ::mnemonic_to_seed(
        words.getPassword(),
        passphrase.getPassword(),
        static_cast<std::uint8_t*>(seed.getMemoryWritable()),
        nullptr);
}
#endif  // OT_CRYPTO_WITH_BIP39

#if OT_CRYPTO_WITH_BIP32
std::string Trezor::SeedToFingerprint(
    const EcdsaCurve& curve,
    const OTPassword& seed) const
{
    auto node = InstantiateHDNode(curve, seed);

    if (node) {
        auto pubkey = Data::Factory(
            static_cast<void*>(node->public_key), sizeof(node->public_key));
        auto identifier = Identifier::Factory();
        identifier->CalculateDigest(pubkey);
        auto fingerprint = String::Factory(identifier);

        return fingerprint->Get();
    }

    return "";
}

std::shared_ptr<proto::AsymmetricKey> Trezor::SeedToPrivateKey(
    const EcdsaCurve& curve,
    const OTPassword& seed) const
{
    std::shared_ptr<proto::AsymmetricKey> derivedKey;
    auto node = InstantiateHDNode(curve, seed);

    OT_ASSERT_MSG(node, "Derivation of root node failed.");

    if (node) {
        derivedKey = HDNodeToSerialized(
            AsymmetricProvider::CurveToKeyType(curve),
            *node,
            Trezor::DERIVE_PRIVATE);

        if (derivedKey) {
            OTPassword root;
            crypto_.Hash().Digest(proto::HASHTYPE_BLAKE2B160, seed, root);
            derivedKey->mutable_path()->set_root(
                root.getMemory(), root.getMemorySize());
        }
    }

    return derivedKey;
}

std::shared_ptr<proto::AsymmetricKey> Trezor::GetChild(
    const proto::AsymmetricKey& parent,
    const std::uint32_t index) const
{
    auto node = SerializedToHDNode(parent);

    if (proto::KEYMODE_PRIVATE == parent.mode()) {
        hdnode_private_ckd(node.get(), index);
    } else {
        hdnode_public_ckd(node.get(), index);
    }
    std::shared_ptr<proto::AsymmetricKey> key =
        HDNodeToSerialized(parent.type(), *node, Trezor::DERIVE_PRIVATE);

    return key;
}

std::unique_ptr<HDNode> Trezor::GetChild(
    const HDNode& parent,
    const std::uint32_t index,
    const DerivationMode privateVersion)
{
    std::unique_ptr<HDNode> output;
    output.reset(new HDNode(parent));

    if (!output) { OT_FAIL; }

    if (privateVersion) {
        hdnode_private_ckd(output.get(), index);
    } else {
        hdnode_public_ckd(output.get(), index);
    }

    return output;
}

std::unique_ptr<HDNode> Trezor::DeriveChild(
    const EcdsaCurve& curve,
    const OTPassword& seed,
    proto::HDPath& path) const
{
    std::uint32_t depth = path.child_size();

    if (0 == depth) {

        return InstantiateHDNode(curve, seed);
    } else {
        proto::HDPath newpath = path;
        newpath.mutable_child()->RemoveLast();
        auto parentnode = DeriveChild(curve, seed, newpath);
        std::unique_ptr<HDNode> output{nullptr};

        if (parentnode) {
            const auto child = path.child(depth - 1);
            output = GetChild(*parentnode, child, DERIVE_PRIVATE);
        } else {
            OT_FAIL;
        }

        return output;
    }
}

std::shared_ptr<proto::AsymmetricKey> Trezor::GetHDKey(
    const EcdsaCurve& curve,
    const OTPassword& seed,
    proto::HDPath& path) const
{
    LogVerbose(OT_METHOD)(__FUNCTION__)(": Deriving child: ")(Print(path))
        .Flush();
    std::shared_ptr<proto::AsymmetricKey> output{nullptr};
    auto node = DeriveChild(curve, seed, path);

    if (!node) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to derive child.").Flush();

        return output;
    }

    output = HDNodeToSerialized(
        AsymmetricProvider::CurveToKeyType(curve),
        *node,
        Trezor::DERIVE_PRIVATE);

    if (output) { *(output->mutable_path()) = path; }

    return output;
}

std::shared_ptr<proto::AsymmetricKey> Trezor::HDNodeToSerialized(
    const proto::AsymmetricKeyType& type,
    const HDNode& node,
    const DerivationMode privateVersion) const
{
    std::shared_ptr<proto::AsymmetricKey> key =
        std::make_shared<proto::AsymmetricKey>();

    key->set_version(1);
    key->set_type(type);

    if (privateVersion) {
        key->set_mode(proto::KEYMODE_PRIVATE);
        auto& encryptedKey = *key->mutable_encryptedkey();
        auto& chaincode = *key->mutable_chaincode();

        OTPasswordData password(__FUNCTION__);
        OTPassword privateKey, publicKey;
        privateKey.setMemory(node.private_key, sizeof(node.private_key));
        publicKey.setMemory(node.chain_code, sizeof(node.chain_code));

        EcdsaProvider::EncryptPrivateKey(
            privateKey, publicKey, password, encryptedKey, chaincode);
    } else {
        key->set_mode(proto::KEYMODE_PUBLIC);
        key->set_key(node.public_key, sizeof(node.public_key));
    }

    return key;
}

std::unique_ptr<HDNode> Trezor::InstantiateHDNode(const EcdsaCurve& curve) const
{
    auto entropy = crypto_.AES().InstantiateBinarySecretSP();

    OT_ASSERT_MSG(entropy, "Failed to obtain entropy.");

    entropy->randomizeMemory(256 / 8);

    auto output = InstantiateHDNode(curve, *entropy);

    OT_ASSERT(output);

    output->depth = 0;
    output->child_num = 0;
    OTPassword::zeroMemory(output->chain_code, sizeof(output->chain_code));
    OTPassword::zeroMemory(output->private_key, sizeof(output->private_key));
    OTPassword::zeroMemory(output->public_key, sizeof(output->public_key));

    return output;
}

std::unique_ptr<HDNode> Trezor::InstantiateHDNode(
    const EcdsaCurve& curve,
    const OTPassword& seed)
{
    std::unique_ptr<HDNode> output;
    output.reset(new HDNode);

    OT_ASSERT_MSG(output, "Instantiation of HD node failed.");

    auto curveName = CurveName(curve);

    if (1 > curveName.size()) { return output; }

    int result = ::hdnode_from_seed(
        static_cast<const std::uint8_t*>(seed.getMemory()),
        seed.getMemorySize(),
        CurveName(curve).c_str(),
        output.get());

    OT_ASSERT_MSG((1 == result), "Setup of HD node failed.");

    ::hdnode_fill_public_key(output.get());

    return output;
}

std::unique_ptr<HDNode> Trezor::SerializedToHDNode(
    const proto::AsymmetricKey& serialized) const
{
    auto node = InstantiateHDNode(
        AsymmetricProvider::KeyTypeToCurve(serialized.type()));

    if (proto::KEYMODE_PRIVATE == serialized.mode()) {
        OTPassword key, chaincode;
        OTPasswordData password(__FUNCTION__);

        OT_ASSERT(!serialized.encryptedkey().text());
        OT_ASSERT(!serialized.chaincode().text());

        DecryptPrivateKey(
            serialized.encryptedkey(),
            serialized.chaincode(),
            password,
            key,
            chaincode);

        OT_ASSERT(key.isMemory());
        OT_ASSERT(chaincode.isMemory());

        OTPassword::safe_memcpy(
            &(node->private_key[0]),
            sizeof(node->private_key),
            key.getMemory(),
            key.getMemorySize(),
            false);

        OTPassword::safe_memcpy(
            &(node->chain_code[0]),
            sizeof(node->chain_code),
            chaincode.getMemory(),
            chaincode.getMemorySize(),
            false);
    } else {
        OTPassword::safe_memcpy(
            &(node->public_key[0]),
            sizeof(node->public_key),
            serialized.key().c_str(),
            serialized.key().size(),
            false);
    }

    return node;
}

std::string Trezor::CurveName(const EcdsaCurve& curve)
{
    switch (curve) {
        case (EcdsaCurve::SECP256K1): {
            return ::SECP256K1_NAME;
        }
        case (EcdsaCurve::ED25519): {
            return ::ED25519_NAME;
        }
        default: {
        }
    }

    return "";
}

bool Trezor::RandomKeypair(OTPassword& privateKey, Data& publicKey) const
{
    bool valid = false;

    do {
        privateKey.randomizeMemory(256 / 8);

        if (ValidPrivateKey(privateKey)) { valid = true; }
    } while (false == valid);

    return ScalarBaseMultiply(privateKey, publicKey);
}

bool Trezor::ValidPrivateKey(const OTPassword& key) const
{
    std::unique_ptr<bignum256> input(new bignum256);
    std::unique_ptr<bignum256> max(new bignum256);

    OT_ASSERT(input);
    OT_ASSERT(max);

    bn_read_be(key.getMemory_uint8(), input.get());
    bn_normalize(input.get());

    bn_read_be(KeyMax, max.get());
    bn_normalize(max.get());

    const bool zero = bn_is_zero(input.get());

    const bool size = bn_is_less(input.get(), max.get());

    return (!zero && size);
}
#endif  // OT_CRYPTO_WITH_BIP32

#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
bool Trezor::ECDH(
    const Data& publicKey,
    const OTPassword& privateKey,
    OTPassword& secret) const
{
    OT_ASSERT(secp256k1_);

    curve_point point;

    const bool havePublic = ecdsa_read_pubkey(
        secp256k1_->params,
        static_cast<const std::uint8_t*>(publicKey.data()),
        &point);

    if (!havePublic) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Invalid public key.").Flush();

        return false;
    }

    bignum256 scalar;
    bn_read_be(privateKey.getMemory_uint8(), &scalar);

    curve_point sharedSecret;
    point_multiply(secp256k1_->params, &scalar, &point, &sharedSecret);

    std::array<std::uint8_t, 32> output{};
    secret.setMemory(output.data(), sizeof(output));

    OT_ASSERT(32 == secret.getMemorySize());

    bn_write_be(
        &sharedSecret.x,
        static_cast<std::uint8_t*>(secret.getMemoryWritable()));

    return true;
}

bool Trezor::ScalarBaseMultiply(const OTPassword& privateKey, Data& publicKey)
    const
{
    std::array<std::uint8_t, 33> blank{};
    publicKey.Assign(blank.data(), blank.size());

    OT_ASSERT(secp256k1_);

    ecdsa_get_public_key33(
        secp256k1_->params,
        privateKey.getMemory_uint8(),
        static_cast<std::uint8_t*>(const_cast<void*>(publicKey.data())));

    curve_point notUsed;

    return (
        1 == ecdsa_read_pubkey(
                 secp256k1_->params,
                 static_cast<const std::uint8_t*>(publicKey.data()),
                 &notUsed));
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1

std::string Trezor::Base58CheckEncode(
    const std::uint8_t* inputStart,
    const std::size_t& inputSize) const
{
    std::string output;

    if (0 == inputSize) { return output; }

    if (128 < inputSize) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Input too long.").Flush();

        return output;
    }

    const std::size_t bufferSize = inputSize + 32 + 4;
    output.resize(bufferSize, 0x0);
    const std::size_t outputSize = ::base58_encode_check(
        inputStart,
        inputSize,
        HASHER_SHA2D,
        const_cast<char*>(output.c_str()),
        output.size());

    OT_ASSERT(outputSize <= bufferSize);

    output.resize(outputSize);

    return output;
}

bool Trezor::Base58CheckDecode(const std::string&& input, RawData& output) const
{
    const std::size_t inputSize = input.size();

    if (0 == inputSize) { return false; }

    if (128 < inputSize) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Input too long.").Flush();

        return false;
    }

    std::size_t outputSize = inputSize;
    output.resize(outputSize, 0x0);
    outputSize = ::base58_decode_check(
        input.data(), HASHER_SHA2D, output.data(), output.size());

    if (0 == outputSize) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Decoding failed.").Flush();

        return false;
    }

    OT_ASSERT(outputSize <= output.size());

    output.resize(outputSize);

    return true;
}

bool Trezor::RIPEMD160(
    const std::uint8_t* input,
    const size_t inputSize,
    std::uint8_t* output) const
{
    ripemd160(input, inputSize, output);

    return true;
}

bool Trezor::Sign(
    const Data& plaintext,
    const key::Asymmetric& theKey,
    const proto::HashType hashType,
    Data& signature,
    const OTPasswordData* pPWData,
    const OTPassword* exportPassword) const
{
    auto hash = Data::Factory();
    bool haveDigest = crypto_.Hash().Digest(hashType, plaintext, hash);

    if (!haveDigest) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failed to obtain the contract hash.")
            .Flush();

        return false;
    }
    OTPassword privKey;
    bool havePrivateKey;

    // FIXME
    OT_ASSERT_MSG(nullptr == exportPassword, "This case is not yet handled.");

    const crypto::key::EllipticCurve* key =
        dynamic_cast<const crypto::key::Secp256k1*>(&theKey);

    if (nullptr == key) { return false; }

    if (nullptr == pPWData) {
        OTPasswordData passwordData(
            "Please enter your password to sign this document.");
        havePrivateKey =
            AsymmetricKeyToECPrivatekey(*key, passwordData, privKey);
    } else {
        havePrivateKey = AsymmetricKeyToECPrivatekey(*key, *pPWData, privKey);
    }

    if (havePrivateKey) {
        std::array<std::uint8_t, 64> blank;
        signature.Assign(blank.data(), blank.size());

        const bool signatureCreated =
            (0 == ecdsa_sign_digest(
                      secp256k1_->params,
                      static_cast<const std::uint8_t*>(privKey.getMemory()),
                      static_cast<const std::uint8_t*>(hash->data()),
                      static_cast<std::uint8_t*>(
                          const_cast<void*>(signature.data())),
                      nullptr,
                      nullptr));

        if (signatureCreated) {

            return true;
        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Call to ecdsa_sign_digest() failed.")
                .Flush();

            return false;
        }
    } else {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Can not extract ecdsa private key from "
            "Asymmetric.")
            .Flush();

        return false;
    }
}

bool Trezor::Verify(
    const Data& plaintext,
    const key::Asymmetric& theKey,
    const Data& signature,
    const proto::HashType hashType,
    const OTPasswordData* pPWData) const
{
    auto hash = Data::Factory();
    bool haveDigest = crypto_.Hash().Digest(hashType, plaintext, hash);

    if (!haveDigest) { return false; }

    const crypto::key::EllipticCurve* key =
        dynamic_cast<const crypto::key::Secp256k1*>(&theKey);

    if (nullptr == key) { return false; }

    auto ecdsaPubkey = Data::Factory();
    const bool havePublicKey = AsymmetricKeyToECPubkey(*key, ecdsaPubkey);

    if (!havePublicKey) { return false; }

    const bool output =
        (0 == ecdsa_verify_digest(
                  secp256k1_->params,
                  static_cast<const std::uint8_t*>(ecdsaPubkey->data()),
                  static_cast<const std::uint8_t*>(signature.data()),
                  static_cast<const std::uint8_t*>(hash->data())));

    return output;
}
}  // namespace opentxs::crypto::implementation
#endif  // OT_CRYPTO_USING_TREZOR
