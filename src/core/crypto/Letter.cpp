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

#include "opentxs/core/crypto/Letter.hpp"

#include "opentxs/api/OT.hpp"
#include "opentxs/core/crypto/AsymmetricKeyEC.hpp"
#include "opentxs/core/crypto/AsymmetricKeyEd25519.hpp"
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#include "opentxs/core/crypto/AsymmetricKeySecp256k1.hpp"
#endif
#include "opentxs/core/crypto/CryptoEngine.hpp"
#include "opentxs/core/crypto/CryptoHash.hpp"
#include "opentxs/core/crypto/CryptoSymmetric.hpp"
#include "opentxs/core/crypto/CryptoSymmetricEngine.hpp"
#include "opentxs/core/crypto/CryptoUtil.hpp"
#include "opentxs/core/crypto/Ecdsa.hpp"
#if OT_CRYPTO_USING_LIBSECP256K1
#include "opentxs/core/crypto/Libsecp256k1.hpp"
#endif
#include "opentxs/core/crypto/Libsodium.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/OpenSSL.hpp"
#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/crypto/OTAsymmetricKey.hpp"
#include "opentxs/core/crypto/OTEnvelope.hpp"
#include "opentxs/core/crypto/OTKeypair.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/crypto/SymmetricKey.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/OTStringXML.hpp"
#include "opentxs/core/Proto.hpp"
#include "opentxs/core/String.hpp"

#include <irrxml/irrXML.hpp>
#include <stdint.h>
#include <ostream>
#include <string>

namespace opentxs
{
bool Letter::AddRSARecipients(
    __attribute__((unused)) const mapOfAsymmetricKeys& recipients,
    __attribute__((unused)) const SymmetricKey& sessionKey,
    __attribute__((unused)) proto::Envelope envelope)
{
#if OT_CRYPTO_SUPPORTED_KEY_RSA
#if OT_CRYPTO_USING_OPENSSL
    OpenSSL& engine = static_cast<OpenSSL&>(OT::App().Crypto().RSA());
#endif

    // Encrypt the session key to all RSA recipients and add the
    // encrypted key to the global list of session keys for this letter.
    Data encrypted;
    proto::SymmetricKey serializedSessionKey;
    const bool serialized = sessionKey.Serialize(serializedSessionKey);

    if (!serialized) {
        otErr << __FUNCTION__ << ": Session key serialization failed."
              <<  std::endl;

        return false;
    }

    Data binary = proto::ProtoAsData(serializedSessionKey);
    const bool haveSessionKey = engine.EncryptSessionKey(
        recipients,
        binary,
        encrypted);

    if (haveSessionKey) {
        envelope.set_rsakey(encrypted.GetPointer(), encrypted.GetSize());
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
    mapOfAsymmetricKeys& RSARecipients,
    __attribute__((unused)) mapOfECKeys& secp256k1Recipients,
    mapOfECKeys& ed25519Recipients)
{
    for (auto& it : recipients) {
        switch (it.second->keyType()) {
            case proto::AKEYTYPE_SECP256K1 :
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
                secp256k1Recipients.insert(
                    std::pair<std::string, const AsymmetricKeyEC*>(
                        it.first,
                        static_cast<const AsymmetricKeySecp256k1*>(it.second)));
#endif
                break;
            case proto::AKEYTYPE_ED25519 :
                ed25519Recipients.insert(
                    std::pair<std::string, const AsymmetricKeyEC*>(
                        it.first,
                        static_cast<const AsymmetricKeyEd25519*>(it.second)));
                break;
            case proto::AKEYTYPE_LEGACY :
                RSARecipients.insert(
                    std::pair<std::string, OTAsymmetricKey*>(
                        it.first,
                        it.second));
                break;
            default :
                otErr << __FUNCTION__
                      << ": Unknown recipient type." << std::endl;
                return false;
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
    const bool haveRecipientsED25519 = (0 < ed25519Recipients.size());

    OTPasswordData defaultPassword("");
    DefaultPassword(defaultPassword);
    auto sessionKey = OT::App().Crypto().Symmetric().Key(defaultPassword);

    proto::Envelope output;
    output.set_version(1);
    Data iv;
    const bool encrypted = sessionKey->Encrypt(
        theInput, iv, defaultPassword, *output.mutable_ciphertext(), false);

    if (!encrypted) {
        otErr << __FUNCTION__ << ": Encryption failed." << std::endl;

        return false;
    }

    if (0 < RSARecipients.size()) {
        if (!AddRSARecipients(RSARecipients, *sessionKey, output)) {
            return false;
        }
    }

    if (haveRecipientsECDSA) {
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
#if OT_CRYPTO_USING_LIBSECP256K1
        Ecdsa& engine =
            static_cast<Libsecp256k1&>(OT::App().Crypto().SECP256K1());
#endif
        std::unique_ptr<OTKeypair> dhKeypair;
        NymParameters parameters(proto::CREDTYPE_LEGACY);
        parameters.setNymParameterType(NymParameterType::SECP256K1);
        dhKeypair.reset(new OTKeypair(parameters, proto::KEYROLE_ENCRYPT));

        OT_ASSERT(dhKeypair);

        auto& newDhKey = *output.add_dhkey();
        newDhKey = *dhKeypair->Serialize(false);


        std::unique_ptr<AsymmetricKeyEC> dhPrivateKey;
        dhPrivateKey.reset(
            static_cast<AsymmetricKeySecp256k1*>(
                OTAsymmetricKey::KeyFactory(*dhKeypair->Serialize(true))));

        OT_ASSERT(dhPrivateKey);

        // Individually encrypt the session key to each recipient and add
        // the encrypted key to the global list of session keys for this
        // letter.
        for (auto it : secp256k1Recipients) {
            OTPassword newKeyPassword;
            const bool haveSessionKey = engine.EncryptSessionKeyECDH(
                *dhPrivateKey,
                *it.second,
                defaultPassword,
                *sessionKey,
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
                << "secp256k1 recipients without Libsecp256k1 support."
                << std::endl;

        return false;
#endif
    }

    if (haveRecipientsED25519) {
        Ecdsa& engine =
            static_cast<Libsodium&>(OT::App().Crypto().ED25519());
        std::unique_ptr<OTKeypair> dhKeypair;
        NymParameters parameters(proto::CREDTYPE_LEGACY);
        parameters.setNymParameterType(NymParameterType::ED25519);
        dhKeypair.reset(new OTKeypair(parameters, proto::KEYROLE_ENCRYPT));

        OT_ASSERT(dhKeypair);

        auto& newDhKey = *output.add_dhkey();
        newDhKey = *dhKeypair->Serialize(false);


        std::unique_ptr<AsymmetricKeyEC> dhPrivateKey;
        dhPrivateKey.reset(
            static_cast<AsymmetricKeyEd25519*>(
                OTAsymmetricKey::KeyFactory(*dhKeypair->Serialize(true))));

        OT_ASSERT(dhPrivateKey);

        // Individually encrypt the session key to each recipient and add
        // the encrypted key to the global list of session keys for this
        // letter.
        for (auto it : ed25519Recipients) {
            OTPassword newKeyPassword;
            const bool haveSessionKey = engine.EncryptSessionKeyECDH(
                *dhPrivateKey,
                *it.second,
                defaultPassword,
                *sessionKey,
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

    dataOutput = proto::ProtoAsData(output);

    return true;
}

bool Letter::Open(
    const Data& dataInput,
    const Nym& theRecipient,
    const OTPasswordData& keyPassword,
    String& theOutput)
{
    auto serialized = proto::DataToProto<proto::Envelope>(dataInput);

    const bool haveInput = proto::Check(serialized, 0, 0xFFFFFFFF);

    if (!haveInput) {
        otErr << __FUNCTION__ << " Could not decode input." << std::endl;

        return false;
    }

    // Attempt to decrypt the session key
    bool haveSessionKey = false;
    const OTAsymmetricKey& privateKey = theRecipient.GetPrivateEncrKey();
    const auto privateKeyType = privateKey.keyType();
#if OT_CRYPTO_SUPPORTED_KEY_RSA
    const bool rsa = privateKeyType == proto::AKEYTYPE_LEGACY;
#endif
    const bool ed25519 = (privateKeyType == proto::AKEYTYPE_ED25519);
    const bool secp256k1 = (privateKeyType == proto::AKEYTYPE_SECP256K1);
    const bool ec = (ed25519 || secp256k1);
    std::unique_ptr<SymmetricKey> key;

    if (ec) {
        const AsymmetricKeyEC* ecKey = nullptr;

        if (secp256k1) {
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
            ecKey = static_cast<const AsymmetricKeySecp256k1*>(&privateKey);
#endif
        } else if (ed25519) {
            ecKey = static_cast<const AsymmetricKeyEd25519*>(&privateKey);
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

        std::unique_ptr<AsymmetricKeyEC> dhPublicKey;
        if (secp256k1) {
#if OT_CRYPTO_SUPPORTED_KEY_SECP256K1
            dhPublicKey.reset(static_cast<AsymmetricKeySecp256k1*>
                (OTAsymmetricKey::KeyFactory(ephemeralPubkey)));
#endif
        } else if (ed25519) {
            dhPublicKey.reset(static_cast<AsymmetricKeyEd25519*>
                (OTAsymmetricKey::KeyFactory(ephemeralPubkey)));
        }

        // The only way to know which session key (might) belong to us to try
        // them all
        for (auto& it : serialized.sessionkey()) {
            key = OT::App().Crypto().Symmetric().Key(
                it,
                serialized.ciphertext().mode());
            haveSessionKey = ecKey->ECDSA().DecryptSessionKeyECDH(
                *ecKey,
                *dhPublicKey,
                keyPassword,
                *key);

            if (haveSessionKey) {
                break;
            }
        }
    }

#if OT_CRYPTO_SUPPORTED_KEY_RSA
    if (rsa) {
#if OT_CRYPTO_USING_OPENSSL
        OpenSSL& engine = static_cast<OpenSSL&>(OT::App().Crypto().RSA());
#endif
        Data serializedKey(
            serialized.rsakey().data(), serialized.rsakey().size());
        Data sessionKey;
        haveSessionKey = engine.DecryptSessionKey(
            serializedKey,
            theRecipient,
            sessionKey,
            nullptr);
    }
#endif

    if (haveSessionKey) {
        Data plaintext;
        OTPasswordData defaultPassword("");
        DefaultPassword(defaultPassword);
        const bool decrypted = key->Decrypt(
            serialized.ciphertext(),
            defaultPassword,
            plaintext);

        if (decrypted) {
            theOutput.Set(
                static_cast<const char*>(plaintext.GetPointer()),
                plaintext.GetSize());

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
} // namespace opentxs
