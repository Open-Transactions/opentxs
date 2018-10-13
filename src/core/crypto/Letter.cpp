// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/crypto/Letter.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#include "opentxs/api/crypto/Symmetric.hpp"
#include "opentxs/api/crypto/Util.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/OTEnvelope.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/StringXML.hpp"
#include "opentxs/core/String.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
#include "opentxs/crypto/key/Ed25519.hpp"
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#include "opentxs/crypto/key/EllipticCurve.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#include "opentxs/crypto/key/Secp256k1.hpp"
#endif
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/crypto/library/EcdsaProvider.hpp"
#include "opentxs/crypto/library/HashingProvider.hpp"
#if OT_CRYPTO_USING_OPENSSL
#include "opentxs/crypto/library/OpenSSL.hpp"
#endif
#if OT_CRYPTO_USING_LIBSECP256K1
#include "opentxs/crypto/library/Secp256k1.hpp"
#endif
#include "opentxs/crypto/library/Sodium.hpp"
#include "opentxs/crypto/library/LegacySymmetricProvider.hpp"
#include "opentxs/crypto/key/Asymmetric.hpp"
#include "opentxs/crypto/key/Keypair.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Proto.hpp"

#include <irrxml/irrXML.hpp>

#include <cstdint>
#include <ostream>
#include <string>

namespace opentxs
{
bool Letter::AddRSARecipients(
    [[maybe_unused]] const mapOfAsymmetricKeys& recipients,
    [[maybe_unused]] const crypto::key::Symmetric& sessionKey,
    [[maybe_unused]] proto::Envelope& envelope)
{
#if OT_CRYPTO_SUPPORTED_KEY_RSA
#if OT_CRYPTO_USING_OPENSSL
    const crypto::OpenSSL& engine =
        dynamic_cast<const crypto::OpenSSL&>(OT::App().Crypto().RSA());
#endif

    // Encrypt the session key to all RSA recipients and add the
    // encrypted key to the global list of session keys for this letter.
    auto encrypted = Data::Factory();
    proto::SymmetricKey serializedSessionKey;
    const bool serialized = sessionKey.Serialize(serializedSessionKey);

    if (!serialized) {
        otErr << __FUNCTION__ << ": Session key serialization failed."
              << std::endl;

        return false;
    }

    auto binary = proto::ProtoAsData(serializedSessionKey);
    const bool haveSessionKey =
        engine.EncryptSessionKey(recipients, binary, encrypted);

    if (haveSessionKey) {
        envelope.set_rsakey(encrypted->data(), encrypted->size());
    } else {
        otErr << __FUNCTION__ << ": Session key encryption failed."
              << std::endl;

        return false;
    }

    return true;
#else
    otErr << __FUNCTION__ << ": Attempting to Seal to RSA recipients without "
          << "RSA support." << std::endl;

    return false;
#endif
}

bool Letter::DefaultPassword(OTPasswordData& password)
{
    OTPassword defaultPassword;
    defaultPassword.setPassword("opentxs");
    return password.SetOverride(defaultPassword);
}

bool Letter::SortRecipients(
    const mapOfAsymmetricKeys& recipients,
    [[maybe_unused]] mapOfAsymmetricKeys& RSARecipients,
    [[maybe_unused]] mapOfECKeys& secp256k1Recipients,
    [[maybe_unused]] mapOfECKeys& ed25519Recipients)
{
    for (auto& it : recipients) {
        switch (it.second->keyType()) {
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
            case proto::AKEYTYPE_SECP256K1: {
                secp256k1Recipients.insert(
                    std::pair<std::string, const crypto::key::EllipticCurve*>(
                        it.first,
                        dynamic_cast<const crypto::key::Secp256k1*>(
                            it.second)));
            } break;
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
            case proto::AKEYTYPE_ED25519: {
                ed25519Recipients.insert(
                    std::pair<std::string, const crypto::key::EllipticCurve*>(
                        it.first,
                        dynamic_cast<const crypto::key::Ed25519*>(it.second)));
            } break;
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
#if OT_CRYPTO_SUPPORTED_KEY_RSA
            case proto::AKEYTYPE_LEGACY: {
                RSARecipients.insert(
                    std::pair<std::string, crypto::key::Asymmetric*>(
                        it.first, it.second));
            } break;
#endif  // OT_CRYPTO_SUPPORTED_KEY_RSA
            default: {
                otErr << __FUNCTION__ << ": Unknown recipient type."
                      << std::endl;
                return false;
            }
        }
    }

    return true;
}

bool Letter::Seal(
    const mapOfAsymmetricKeys& RecipPubKeys,
    const String& theInput,
    Data& dataOutput)
{
    mapOfAsymmetricKeys RSARecipients;
    mapOfECKeys secp256k1Recipients;
    mapOfECKeys ed25519Recipients;

    if (!SortRecipients(
            RecipPubKeys,
            RSARecipients,
            secp256k1Recipients,
            ed25519Recipients)) {
        return false;
    }

    const bool haveRecipientsECDSA = (0 < secp256k1Recipients.size());
    [[maybe_unused]] const bool haveRecipientsED25519 =
        (0 < ed25519Recipients.size());

    OTPasswordData defaultPassword("");
    DefaultPassword(defaultPassword);
    auto sessionKey = OT::App().Crypto().Symmetric().Key(defaultPassword);

    proto::Envelope output;
    output.set_version(1);
    auto iv = Data::Factory();
    const bool encrypted = sessionKey->Encrypt(
        theInput, iv, defaultPassword, *output.mutable_ciphertext(), false);

    if (!encrypted) {
        otErr << __FUNCTION__ << ": Encryption failed." << std::endl;

        return false;
    }

    if (0 < RSARecipients.size()) {
        if (!AddRSARecipients(RSARecipients, sessionKey, output)) {
            return false;
        }
    }

    [[maybe_unused]] auto dhRawKey = crypto::key::Asymmetric::Factory();
    [[maybe_unused]] const crypto::key::EllipticCurve* dhPrivateKey { nullptr };

    if (haveRecipientsECDSA) {
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        const auto& engine = dynamic_cast<const crypto::EcdsaProvider&>(
            OT::App().Crypto().SECP256K1());
        NymParameters parameters(proto::CREDTYPE_LEGACY);
        parameters.setNymParameterType(NymParameterType::SECP256K1);
        auto dhKeypair =
            crypto::key::Keypair::Factory(parameters, proto::KEYROLE_ENCRYPT);
        auto& newDhKey = *output.add_dhkey();
        newDhKey = *dhKeypair->Serialize(false);
        dhRawKey =
            crypto::key::Asymmetric::Factory(*dhKeypair->Serialize(true));
        dhPrivateKey =
            dynamic_cast<const crypto::key::Secp256k1*>(&dhRawKey.get());

        OT_ASSERT(nullptr != dhPrivateKey);

        // Individually encrypt the session key to each recipient and add
        // the encrypted key to the global list of session keys for this
        // letter.
        for (auto it : secp256k1Recipients) {
            OTPassword newKeyPassword;
            const bool haveSessionKey = engine.EncryptSessionKeyECDH(
                *dhPrivateKey,
                *it.second,
                defaultPassword,
                sessionKey,
                newKeyPassword);

            if (haveSessionKey) {
                auto& serializedSessionKey = *output.add_sessionkey();
                sessionKey->Serialize(serializedSessionKey);
            } else {
                otErr << __FUNCTION__ << ": Session key encryption failed."
                      << std::endl;

                return false;
            }
        }
#else
        otErr << __FUNCTION__ << ": Attempting to Seal to "
              << "secp256k1 recipients without crypto::Secp256k1 support."
              << std::endl;

        return false;
#endif
    }

#if OT_CRYPTO_SUPPORTED_KEY_ED25519
    if (haveRecipientsED25519) {
        const crypto::EcdsaProvider& engine =
            dynamic_cast<const crypto::EcdsaProvider&>(
                OT::App().Crypto().ED25519());
        NymParameters parameters(proto::CREDTYPE_LEGACY);
        parameters.setNymParameterType(NymParameterType::ED25519);
        auto dhKeypair =
            crypto::key::Keypair::Factory(parameters, proto::KEYROLE_ENCRYPT);
        auto& newDhKey = *output.add_dhkey();
        newDhKey = *dhKeypair->Serialize(false);
        dhRawKey =
            crypto::key::Asymmetric::Factory(*dhKeypair->Serialize(true));
        dhPrivateKey =
            dynamic_cast<const crypto::key::Ed25519*>(&dhRawKey.get());

        OT_ASSERT(nullptr != dhPrivateKey);

        // Individually encrypt the session key to each recipient and add
        // the encrypted key to the global list of session keys for this
        // letter.
        for (auto it : ed25519Recipients) {
            OTPassword newKeyPassword;
            const bool haveSessionKey = engine.EncryptSessionKeyECDH(
                *dhPrivateKey,
                *it.second,
                defaultPassword,
                sessionKey,
                newKeyPassword);

            if (haveSessionKey) {
                auto& serializedSessionKey = *output.add_sessionkey();
                sessionKey->Serialize(serializedSessionKey);
            } else {
                otErr << __FUNCTION__ << ": Session key encryption failed."
                      << std::endl;

                return false;
            }
        }
    }
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519

    auto temp = proto::ProtoAsData(output);
    dataOutput.Assign(temp->data(), temp->size());

    return true;
}

bool Letter::Open(
    const Data& dataInput,
    const Nym& theRecipient,
    const OTPasswordData& keyPassword,
    String& theOutput)
{
    auto serialized = proto::DataToProto<proto::Envelope>(dataInput);

    const bool haveInput = proto::Validate(serialized, VERBOSE);

    if (!haveInput) {
        otErr << __FUNCTION__ << " Could not decode input." << std::endl;

        return false;
    }

    // Attempt to decrypt the session key
    bool haveSessionKey = false;
    const crypto::key::Asymmetric& privateKey =
        theRecipient.GetPrivateEncrKey();
    const auto privateKeyType = privateKey.keyType();
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    const bool rsa = privateKeyType == proto::AKEYTYPE_LEGACY;
#endif
    const bool ed25519 = (privateKeyType == proto::AKEYTYPE_ED25519);
    const bool secp256k1 = (privateKeyType == proto::AKEYTYPE_SECP256K1);
    const bool ec = (ed25519 || secp256k1);
    auto key = crypto::key::Symmetric::Factory();

    if (ec) {
        const crypto::key::EllipticCurve* ecKey = nullptr;

        if (secp256k1) {
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
            ecKey = dynamic_cast<const crypto::key::Secp256k1*>(&privateKey);
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        } else if (ed25519) {
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
            ecKey = dynamic_cast<const crypto::key::Ed25519*>(&privateKey);
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
        }

        if (nullptr == ecKey) {
            otErr << __FUNCTION__ << ": Unsupported key type." << std::endl;

            return false;
        }

        bool found = false;
        proto::AsymmetricKey ephemeralPubkey;

        for (auto& key : serialized.dhkey()) {
            if (privateKeyType == key.type()) {
                ephemeralPubkey = key;
                found = true;

                break;
            }
        }

        if (!found) {
            otErr << __FUNCTION__ << ": Need an ephemeral public key for ECDH, "
                  << "but the letter does not contain one." << std::endl;

            return false;
        }

        const auto dhRawKey = crypto::key::Asymmetric::Factory(ephemeralPubkey);
        const crypto::key::EllipticCurve* dhPublicKey{nullptr};

        if (secp256k1) {
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
            dhPublicKey =
                dynamic_cast<const crypto::key::Secp256k1*>(&dhRawKey.get());
#endif  // OT_CRYPTO_SUPPORTED_KEY_SECP256K1
        } else if (ed25519) {
#if OT_CRYPTO_SUPPORTED_KEY_ED25519
            dhPublicKey =
                dynamic_cast<const crypto::key::Ed25519*>(&dhRawKey.get());
#endif  // OT_CRYPTO_SUPPORTED_KEY_ED25519
        }

        OT_ASSERT(nullptr != dhPublicKey)

        // The only way to know which session key (might) belong to us to
        // try them all
        for (auto& it : serialized.sessionkey()) {
            key = OT::App().Crypto().Symmetric().Key(
                it, serialized.ciphertext().mode());
            haveSessionKey = ecKey->ECDSA().DecryptSessionKeyECDH(
                *ecKey, *dhPublicKey, keyPassword, key);

            if (haveSessionKey) { break; }
        }
    }

#if OT_CRYPTO_SUPPORTED_KEY_RSA
    if (rsa) {
#if OT_CRYPTO_USING_OPENSSL
        const crypto::OpenSSL& engine =
            dynamic_cast<const crypto::OpenSSL&>(OT::App().Crypto().RSA());
#endif
        auto serializedKey = Data::Factory(
            serialized.rsakey().data(), serialized.rsakey().size());
        auto sessionKey = Data::Factory();
        haveSessionKey = engine.DecryptSessionKey(
            serializedKey, theRecipient, sessionKey, nullptr);
    }
#endif

    if (haveSessionKey) {
        auto plaintext = Data::Factory();
        OTPasswordData defaultPassword("");
        DefaultPassword(defaultPassword);
        const bool decrypted =
            key->Decrypt(serialized.ciphertext(), defaultPassword, plaintext);

        if (decrypted) {
            theOutput.Set(
                static_cast<const char*>(plaintext->data()), plaintext->size());

            return true;
        } else {
            otErr << __FUNCTION__ << " Decryption failed." << std::endl;

            return false;
        }
    } else {
        otErr << __FUNCTION__ << " Could not decrypt any sessions key. "
              << "Was this message intended for us?" << std::endl;

        return false;
    }
}
}  // namespace opentxs
