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
#if OT_CRYPTO_USING_TREZOR
#include "opentxs/core/stdafx.hpp"

#include "opentxs/core/crypto/TrezorCrypto.hpp"

#include "opentxs/api/OT.hpp"
#include "opentxs/core/crypto/CryptoEngine.hpp"
#include "opentxs/core/crypto/CryptoHashEngine.hpp"
#include "opentxs/core/crypto/CryptoSymmetric.hpp"
#include "opentxs/core/crypto/Ecdsa.hpp"
#include "opentxs/core/crypto/OTAsymmetricKey.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Proto.hpp"
#include "opentxs/core/String.hpp"

extern "C" {
#if OT_CRYPTO_WITH_BIP39
#include <trezor-crypto/bip39.h>
#if OT_CRYPTO_WITH_BIP32
#include <trezor-crypto/bignum.h>
#include <trezor-crypto/curves.h>
#endif
#endif
}

#include <stdint.h>
#include <array>

#define OT_METHOD "opentxs::TrezorCrypto::"

namespace opentxs
{
#if OT_CRYPTO_WITH_BIP39
bool TrezorCrypto::toWords(const OTPassword& seed, OTPassword& words) const
{
    return words.setPassword(std::string(::mnemonic_from_data(
        static_cast<const uint8_t*>(seed.getMemory()), seed.getMemorySize())));
}

void TrezorCrypto::WordsToSeed(
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
        static_cast<uint8_t*>(seed.getMemoryWritable()),
        nullptr);
}
#endif  // OT_CRYPTO_WITH_BIP39

#if OT_CRYPTO_WITH_BIP32
TrezorCrypto::TrezorCrypto()
    : secp256k1_(get_curve_by_name(CurveName(EcdsaCurve::SECP256K1).c_str()))
{
    OT_ASSERT(nullptr != secp256k1_);
}

std::string TrezorCrypto::SeedToFingerprint(
    const EcdsaCurve& curve,
    const OTPassword& seed) const
{
    auto node = InstantiateHDNode(curve, seed);

    if (node) {
        Data pubkey(
            static_cast<void*>(node->public_key), sizeof(node->public_key));
        Identifier identifier;
        identifier.CalculateDigest(pubkey);
        String fingerprint(identifier);

        return fingerprint.Get();
    }

    return "";
}

serializedAsymmetricKey TrezorCrypto::SeedToPrivateKey(
    const EcdsaCurve& curve,
    const OTPassword& seed) const
{
    serializedAsymmetricKey derivedKey;
    auto node = InstantiateHDNode(curve, seed);

    OT_ASSERT_MSG(node, "Derivation of root node failed.");

    if (node) {
        derivedKey = HDNodeToSerialized(
            CryptoAsymmetric::CurveToKeyType(curve),
            *node,
            TrezorCrypto::DERIVE_PRIVATE);

        if (derivedKey) {
            OTPassword root;
            OT::App().Crypto().Hash().Digest(
                proto::HASHTYPE_BLAKE2B160, seed, root);
            derivedKey->mutable_path()->set_root(
                root.getMemory(), root.getMemorySize());
        }
    }

    return derivedKey;
}

serializedAsymmetricKey TrezorCrypto::GetChild(
    const proto::AsymmetricKey& parent,
    const uint32_t index) const
{
    auto node = SerializedToHDNode(parent);

    if (proto::KEYMODE_PRIVATE == parent.mode()) {
        hdnode_private_ckd(node.get(), index);
    } else {
        hdnode_public_ckd(node.get(), index);
    }
    serializedAsymmetricKey key =
        HDNodeToSerialized(parent.type(), *node, TrezorCrypto::DERIVE_PRIVATE);

    return key;
}

std::unique_ptr<HDNode> TrezorCrypto::GetChild(
    const HDNode& parent,
    const uint32_t index,
    const DerivationMode privateVersion)
{
    std::unique_ptr<HDNode> output;
    output.reset(new HDNode(parent));

    if (!output) {
        OT_FAIL;
    }

    if (privateVersion) {
        hdnode_private_ckd(output.get(), index);
    } else {
        hdnode_public_ckd(output.get(), index);
    }

    return output;
}

std::unique_ptr<HDNode> TrezorCrypto::DeriveChild(
    const EcdsaCurve& curve,
    const OTPassword& seed,
    proto::HDPath& path) const
{
    uint32_t depth = path.child_size();

    if (0 == depth) {

        return InstantiateHDNode(curve, seed);
    } else {
        proto::HDPath newpath = path;
        newpath.mutable_child()->RemoveLast();
        auto parentnode = DeriveChild(curve, seed, newpath);
        std::unique_ptr<HDNode> output;

        if (parentnode) {
            const auto child = path.child(depth - 1);
            output = GetChild(*parentnode, child, DERIVE_PRIVATE);
        }

        return output;
    }
}

serializedAsymmetricKey TrezorCrypto::GetHDKey(
    const EcdsaCurve& curve,
    const OTPassword& seed,
    proto::HDPath& path) const
{
    serializedAsymmetricKey output;
    auto node = DeriveChild(curve, seed, path);

    if (!node) {
        return output;
    }

    output = HDNodeToSerialized(
        CryptoAsymmetric::CurveToKeyType(curve),
        *node,
        TrezorCrypto::DERIVE_PRIVATE);

    if (output) {
        *(output->mutable_path()) = path;
    }

    return output;
}

serializedAsymmetricKey TrezorCrypto::HDNodeToSerialized(
    const proto::AsymmetricKeyType& type,
    const HDNode& node,
    const DerivationMode privateVersion) const
{
    serializedAsymmetricKey key = std::make_shared<proto::AsymmetricKey>();

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

        Ecdsa::EncryptPrivateKey(
            privateKey, publicKey, password, encryptedKey, chaincode);
    } else {
        key->set_mode(proto::KEYMODE_PUBLIC);
        key->set_key(node.public_key, sizeof(node.public_key));
    }

    return key;
}

std::unique_ptr<HDNode> TrezorCrypto::InstantiateHDNode(const EcdsaCurve& curve)
{
    auto entropy = OT::App().Crypto().AES().InstantiateBinarySecretSP();

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

std::unique_ptr<HDNode> TrezorCrypto::InstantiateHDNode(
    const EcdsaCurve& curve,
    const OTPassword& seed)
{
    std::unique_ptr<HDNode> output;
    output.reset(new HDNode);

    OT_ASSERT_MSG(output, "Instantiation of HD node failed.");

    auto curveName = CurveName(curve);

    if (1 > curveName.size()) {
        return output;
    }

    int result = ::hdnode_from_seed(
        static_cast<const uint8_t*>(seed.getMemory()),
        seed.getMemorySize(),
        CurveName(curve).c_str(),
        output.get());

    OT_ASSERT_MSG((1 == result), "Setup of HD node failed.");

    ::hdnode_fill_public_key(output.get());

    return output;
}

std::unique_ptr<HDNode> TrezorCrypto::SerializedToHDNode(
    const proto::AsymmetricKey& serialized) const
{
    auto node =
        InstantiateHDNode(CryptoAsymmetric::KeyTypeToCurve(serialized.type()));

    if (proto::KEYMODE_PRIVATE == serialized.mode()) {
        OTPassword key, chaincode;
        OTPasswordData password(__FUNCTION__);

        OT_ASSERT(!serialized.encryptedkey().text());
        OT_ASSERT(!serialized.chaincode().text());

        Ecdsa::DecryptPrivateKey(
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

std::string TrezorCrypto::CurveName(const EcdsaCurve& curve)
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

bool TrezorCrypto::RandomKeypair(OTPassword& privateKey, Data& publicKey) const
{
    bool valid = false;

    do {
        privateKey.randomizeMemory(256 / 8);

        if (ValidPrivateKey(privateKey)) {
            valid = true;
        }
    } while (false == valid);

    return ScalarBaseMultiply(privateKey, publicKey);
}

bool TrezorCrypto::ValidPrivateKey(const OTPassword& key) const
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
bool TrezorCrypto::ECDH(
    const Data& publicKey,
    const OTPassword& privateKey,
    OTPassword& secret) const
{
    OT_ASSERT(secp256k1_);

    curve_point point;

    const bool havePublic = ecdsa_read_pubkey(
        secp256k1_->params,
        static_cast<const uint8_t*>(publicKey.GetPointer()),
        &point);

    if (!havePublic) {
        otWarn << OT_METHOD << __FUNCTION__ << ": Invalid public key."
               << std::endl;

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

bool TrezorCrypto::ScalarBaseMultiply(
    const OTPassword& privateKey,
    Data& publicKey) const
{
    std::array<std::uint8_t, 33> blank{};
    publicKey.Assign(blank.data(), blank.size());

    OT_ASSERT(secp256k1_);

    ecdsa_get_public_key33(
        secp256k1_->params,
        privateKey.getMemory_uint8(),
        static_cast<std::uint8_t*>(const_cast<void*>(publicKey.GetPointer())));

    curve_point notUsed;

    return (
        1 == ecdsa_read_pubkey(
                 secp256k1_->params,
                 static_cast<const std::uint8_t*>(publicKey.GetPointer()),
                 &notUsed));
}
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1

std::string TrezorCrypto::Base58CheckEncode(
    const std::uint8_t* inputStart,
    const std::size_t& inputSize) const
{
    std::string output;

    if (0 == inputSize) {
        return output;
    }

    if (128 < inputSize) {
        otWarn << OT_METHOD << __FUNCTION__ << ": Input too long." << std::endl;

        return output;
    }

    const std::size_t bufferSize = inputSize + 32 + 4;
    output.resize(bufferSize, 0x0);
    const std::size_t outputSize = ::base58_encode_check(
        inputStart,
        inputSize,
        const_cast<char*>(output.c_str()),
        output.size());

    OT_ASSERT(outputSize <= bufferSize);

    output.resize(outputSize);

    return output;
}

bool TrezorCrypto::Base58CheckDecode(const std::string&& input, RawData& output)
    const
{
    const std::size_t inputSize = input.size();

    if (0 == inputSize) {
        return false;
    }

    if (128 < inputSize) {
        otWarn << OT_METHOD << __FUNCTION__ << ": Input too long." << std::endl;

        return false;
    }

    std::size_t outputSize = inputSize;
    output.resize(outputSize, 0x0);
    outputSize =
        ::base58_decode_check(input.data(), output.data(), output.size());

    if (0 == outputSize) {
        otWarn << OT_METHOD << __FUNCTION__ << ": Decoding failed."
               << std::endl;

        return false;
    }

    OT_ASSERT(outputSize <= output.size());

    output.resize(outputSize);

    return true;
}
}  // namespace opentxs
#endif  // OT_CRYPTO_USING_TREZOR
