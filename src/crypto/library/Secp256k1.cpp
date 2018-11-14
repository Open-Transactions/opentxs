// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#if OT_CRYPTO_USING_LIBSECP256K1
#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Hash.hpp"
#include "opentxs/api/crypto/Util.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/core/crypto/OTEnvelope.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#if OT_CRYPTO_USING_LIBBITCOIN
#include "opentxs/crypto/library/Bitcoin.hpp"
#endif
#include "opentxs/crypto/key/Secp256k1.hpp"
#include "opentxs/crypto/library/Secp256k1.hpp"
#include "opentxs/crypto/library/SymmetricProvider.hpp"
#if OT_CRYPTO_USING_TREZOR
#include "opentxs/crypto/library/Trezor.hpp"
#endif
#include "opentxs/OT.hpp"
#include "opentxs/Proto.hpp"

#include "AsymmetricProvider.hpp"
#include "EcdsaProvider.hpp"

extern "C" {
#include "secp256k1.h"
}

#include <cstdint>
#include <ostream>

#include "Secp256k1.hpp"

#define OT_METHOD "opentxs::Secp256k1::"

namespace opentxs
{
crypto::Secp256k1* Factory::Secp256k1(
    const api::Crypto& crypto,
    const api::crypto::Util& util,
    const crypto::EcdsaProvider& ecdsa)
{
    return new crypto::implementation::Secp256k1(crypto, util, ecdsa);
}
}  // namespace opentxs

namespace opentxs::crypto::implementation
{
bool Secp256k1::Initialized_ = false;

Secp256k1::Secp256k1(
    const api::Crypto& crypto,
    const api::crypto::Util& ssl,
    const crypto::EcdsaProvider& ecdsa)
    : EcdsaProvider(crypto)
    , context_(secp256k1_context_create(
          SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY))
    , ecdsa_(ecdsa)
    , ssl_(ssl)
{
}

bool Secp256k1::RandomKeypair(OTPassword& privateKey, Data& publicKey) const
{
    if (nullptr == context_) { return false; }

    bool validPrivkey = false;
    std::uint8_t candidateKey[PrivateKeySize]{};
    std::uint8_t nullKey[PrivateKeySize]{};
    std::uint8_t counter = 0;

    while (!validPrivkey) {
        privateKey.randomizeMemory_uint8(candidateKey, sizeof(candidateKey));
        // We add the random key to a zero value key because
        // secp256k1_privkey_tweak_add checks the result to make sure it's in
        // the correct range for secp256k1.
        //
        // This loop should almost always run exactly one time (about 1/(2^128)
        // chance of randomly generating an invalid key thus requiring a second
        // attempt)
        validPrivkey =
            secp256k1_ec_privkey_tweak_add(context_, candidateKey, nullKey);

        OT_ASSERT(3 > ++counter);
    }
    privateKey.setMemory(candidateKey, sizeof(candidateKey));

    return ScalarBaseMultiply(privateKey, publicKey);
}

bool Secp256k1::Sign(
    const Data& plaintext,
    const key::Asymmetric& theKey,
    const proto::HashType hashType,
    Data& signature,  // output
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
        secp256k1_ecdsa_signature ecdsaSignature;

        bool signatureCreated = secp256k1_ecdsa_sign(
            context_,
            &ecdsaSignature,
            reinterpret_cast<const unsigned char*>(hash->data()),
            reinterpret_cast<const unsigned char*>(privKey.getMemory()),
            nullptr,
            nullptr);

        if (signatureCreated) {
            signature.Assign(
                ecdsaSignature.data, sizeof(secp256k1_ecdsa_signature));
            return true;
        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Call to secp256k1_ecdsa_sign() failed.")
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

bool Secp256k1::Verify(
    const Data& plaintext,
    const key::Asymmetric& theKey,
    const Data& signature,
    const proto::HashType hashType,
    [[maybe_unused]] const OTPasswordData* pPWData) const
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

    secp256k1_pubkey point;
    const bool pubkeyParsed = ParsePublicKey(ecdsaPubkey, point);

    if (!pubkeyParsed) { return false; }

    secp256k1_ecdsa_signature ecdsaSignature;
    const bool haveSignature = DataToECSignature(signature, ecdsaSignature);

    if (!haveSignature) { return false; }

    return secp256k1_ecdsa_verify(
        context_,
        &ecdsaSignature,
        reinterpret_cast<const unsigned char*>(hash->data()),
        &point);
}

bool Secp256k1::DataToECSignature(
    const Data& inSignature,
    secp256k1_ecdsa_signature& outSignature) const
{
    const std::uint8_t* sigStart =
        static_cast<const std::uint8_t*>(inSignature.data());

    if (nullptr != sigStart) {

        if (sizeof(secp256k1_ecdsa_signature) == inSignature.size()) {
            secp256k1_ecdsa_signature ecdsaSignature;

            for (std::uint32_t i = 0; i < inSignature.size(); i++) {
                ecdsaSignature.data[i] = *(sigStart + i);
            }

            outSignature = ecdsaSignature;

            return true;
        }
    }
    return false;
}

bool Secp256k1::ECDH(
    const Data& publicKey,
    const OTPassword& privateKey,
    OTPassword& secret) const
{
#if OT_CRYPTO_USING_LIBBITCOIN
    return dynamic_cast<const Bitcoin&>(ecdsa_).ECDH(
        publicKey, privateKey, secret);
#elif OT_CRYPTO_USING_TREZOR
    return dynamic_cast<const Trezor&>(ecdsa_).ECDH(
        publicKey, privateKey, secret);
#else
    return false;
#endif
}

void Secp256k1::Init()
{
    OT_ASSERT(false == Initialized_);
    std::uint8_t randomSeed[32]{};
    ssl_.RandomizeMemory(randomSeed, 32);

    OT_ASSERT(nullptr != context_);

    [[maybe_unused]] int randomize =
        secp256k1_context_randomize(context_, randomSeed);
    Initialized_ = true;
}

bool Secp256k1::ParsePublicKey(const Data& input, secp256k1_pubkey& output)
    const
{
    if (nullptr == context_) { return false; }

    return secp256k1_ec_pubkey_parse(
        context_,
        &output,
        reinterpret_cast<const unsigned char*>(input.data()),
        input.size());
}

bool Secp256k1::ScalarBaseMultiply(
    const OTPassword& privateKey,
    Data& publicKey) const
{
    if (nullptr == context_) { return false; }

    if (!privateKey.isMemory()) { return false; }

    secp256k1_pubkey key;

    const auto created = secp256k1_ec_pubkey_create(
        context_,
        &key,
        static_cast<const unsigned char*>(privateKey.getMemory()));

    if (1 != created) { return false; }

    unsigned char output[PublicKeySize]{};
    size_t outputSize = sizeof(output);

    const auto serialized = secp256k1_ec_pubkey_serialize(
        context_, output, &outputSize, &key, SECP256K1_EC_COMPRESSED);

    if (1 != serialized) { return false; }

    publicKey.Assign(output, outputSize);

    return true;
}

Secp256k1::~Secp256k1()
{
    if (nullptr != context_) {
        secp256k1_context_destroy(context_);
        context_ = nullptr;
    }
    Initialized_ = false;
}
}  // namespace opentxs::crypto::implementation
#endif
