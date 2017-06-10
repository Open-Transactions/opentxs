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

#include "opentxs/core/crypto/Ecdsa.hpp"

#include "opentxs/api/OT.hpp"
#include "opentxs/core/crypto/AsymmetricKeyEC.hpp"
#include "opentxs/core/crypto/Crypto.hpp"
#include "opentxs/core/crypto/CryptoEngine.hpp"
#include "opentxs/core/crypto/CryptoHash.hpp"
#include "opentxs/core/crypto/CryptoHashEngine.hpp"
#include "opentxs/core/crypto/CryptoSymmetric.hpp"
#include "opentxs/core/crypto/CryptoSymmetricEngine.hpp"
#include "opentxs/core/crypto/CryptoUtil.hpp"
#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/OTPasswordData.hpp"
#include "opentxs/core/crypto/SymmetricKey.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"

namespace opentxs
{
bool Ecdsa::AsymmetricKeyToECPrivatekey(
    const AsymmetricKeyEC& asymmetricKey,
    const OTPasswordData& passwordData,
    OTPassword& privkey) const
{
    proto::Ciphertext dataPrivkey;
    const bool havePrivateKey = asymmetricKey.GetKey(dataPrivkey);

    if (!havePrivateKey) { return false; }

    return AsymmetricKeyToECPrivkey(
        dataPrivkey, passwordData, privkey);
}

bool Ecdsa::AsymmetricKeyToECPrivkey(
    const proto::Ciphertext& asymmetricKey,
    const OTPasswordData& passwordData,
    OTPassword& privkey) const
{
    BinarySecret masterPassword(
        OT::App().Crypto().AES().InstantiateBinarySecretSP());

    return ImportECPrivatekey(asymmetricKey, passwordData, privkey);
}

bool Ecdsa::AsymmetricKeyToECPubkey(
    const AsymmetricKeyEC& asymmetricKey,
    Data& pubkey) const
{
    return asymmetricKey.GetKey(pubkey);
}

bool Ecdsa::DecryptPrivateKey(
    const proto::Ciphertext& encryptedKey,
    const OTPasswordData& password,
    OTPassword& plaintextKey)
{
    auto key = OT::App().Crypto().Symmetric().Key(
        encryptedKey.key(),
        encryptedKey.mode());

    if (!key) {
        return false;
    }

    return key->Decrypt(encryptedKey, password, plaintextKey);
}

bool Ecdsa::DecryptPrivateKey(
    const proto::Ciphertext& encryptedKey,
    const proto::Ciphertext& encryptedChaincode,
    const OTPasswordData& password,
    OTPassword& key,
    OTPassword& chaincode)
{
    auto sessionKey = OT::App().Crypto().Symmetric().Key(
        encryptedKey.key(),
        encryptedKey.mode());

    const bool keyDecrypted =
        sessionKey->Decrypt(encryptedKey, password, key);
    const bool chaincodeDecrypted =
        sessionKey->Decrypt(encryptedChaincode, password, chaincode);

    return (keyDecrypted && chaincodeDecrypted);
}

bool Ecdsa::DecryptSessionKeyECDH(
    const AsymmetricKeyEC& privateKey,
    const AsymmetricKeyEC& publicKey,
    const OTPasswordData& password,
    SymmetricKey& sessionKey) const
{
    Data publicDHKey;

    if (!publicKey.GetKey(publicDHKey)) {
        otErr << __FUNCTION__ << ": Failed to get public key."
              << std::endl;

        return false;
    }

    OTPassword privateDHKey;

    if (!AsymmetricKeyToECPrivatekey(privateKey, password, privateDHKey)) {
        otErr << __FUNCTION__ << ": Failed to get private key."
              << std::endl;

        return false;
    }

    // Calculate ECDH shared secret
    BinarySecret ECDHSecret(
        OT::App().Crypto().AES().InstantiateBinarySecretSP());
    bool haveECDH = ECDH(publicDHKey, privateDHKey, *ECDHSecret);

    if (!haveECDH) {
        otErr << __FUNCTION__ << ": ECDH shared secret negotiation failed."
              << std::endl;

        return false;
    }

    OTPasswordData unlockPassword("");
    unlockPassword.SetOverride(*ECDHSecret);

    return sessionKey.Unlock(unlockPassword) ;
}

bool Ecdsa::ECPrivatekeyToAsymmetricKey(
    const OTPassword& privkey,
    const OTPasswordData& passwordData,
    AsymmetricKeyEC& asymmetricKey) const
{
    return ExportECPrivatekey(privkey, passwordData, asymmetricKey);
}

bool Ecdsa::ECPubkeyToAsymmetricKey(
    std::unique_ptr<Data>& pubkey,
    AsymmetricKeyEC& asymmetricKey) const
{
    if (!pubkey) { return false; }

    return asymmetricKey.SetKey(pubkey);
}

bool Ecdsa::EncryptPrivateKey(
    const OTPassword& plaintextKey,
    const OTPasswordData& password,
    proto::Ciphertext& encryptedKey)
{
    auto key = OT::App().Crypto().Symmetric().Key(password);

    if (!key) { return false; }

    Data blank;

    return key->Encrypt(
        plaintextKey,
        blank,
        password,
        encryptedKey,
        true);
}

bool Ecdsa::EncryptPrivateKey(
    const OTPassword& key,
    const OTPassword& chaincode,
    const OTPasswordData& password,
    proto::Ciphertext& encryptedKey,
    proto::Ciphertext& encryptedChaincode)
{
    auto sessionKey = OT::App().Crypto().Symmetric().Key(password);

    if (!sessionKey) { return false; }

    Data blank;

    const bool keyEncrypted = sessionKey->Encrypt(
        key,
        blank,
        password,
        encryptedKey,
        true);

    const bool chaincodeEncrypted = sessionKey->Encrypt(
        chaincode,
        blank,
        password,
        encryptedChaincode,
        false);

    return (keyEncrypted && chaincodeEncrypted);
}

bool Ecdsa::EncryptSessionKeyECDH(
    const AsymmetricKeyEC& privateKey,
    const AsymmetricKeyEC& publicKey,
    const OTPasswordData& passwordData,
    SymmetricKey& sessionKey,
    OTPassword& newKeyPassword) const
{
    std::unique_ptr<Data> dhPublicKey(new Data);

    OT_ASSERT(dhPublicKey);

    const auto havePubkey = publicKey.GetKey(*dhPublicKey);

    if (!havePubkey) {
        otErr << __FUNCTION__ << ": Failed to get public key." << std::endl;

        return false;
    }

    BinarySecret dhPrivateKey(
        OT::App().Crypto().AES().InstantiateBinarySecretSP());

    OT_ASSERT(dhPrivateKey);

    OTPasswordData privatePassword("");
    const bool havePrivateKey = AsymmetricKeyToECPrivatekey(
        privateKey,
        privatePassword,
        *dhPrivateKey);

    if (!havePrivateKey) {
        otErr << __FUNCTION__ << ": Failed to get private key." << std::endl;

        return false;
    }

    // Calculate ECDH shared secret
    const bool haveECDH = ECDH(*dhPublicKey, *dhPrivateKey, newKeyPassword);

    if (!haveECDH) {
        otErr << __FUNCTION__ << ": ECDH shared secret negotiation failed."
              << std::endl;

        return false;
    }

    const bool encrypted = sessionKey.ChangePassword(
        passwordData,
        newKeyPassword);

    if (!encrypted) {
        otErr << __FUNCTION__ << ": Session key encryption failed."
              << std::endl;

        return false;
    }

    return true;
}

bool Ecdsa::ExportECPrivatekey(
    const OTPassword& privkey,
    const OTPasswordData& password,
    AsymmetricKeyEC& asymmetricKey) const
{
    std::unique_ptr<proto::Ciphertext> encryptedKey(new proto::Ciphertext);

    EncryptPrivateKey(privkey, password, *encryptedKey);

    return asymmetricKey.SetKey(encryptedKey);
}

bool Ecdsa::ImportECPrivatekey(
    const proto::Ciphertext& asymmetricKey,
    const OTPasswordData& password,
    OTPassword& privkey) const
{
    return DecryptPrivateKey(asymmetricKey, password, privkey);
}

bool Ecdsa::PrivateToPublic(
    const proto::AsymmetricKey& privateKey,
    proto::AsymmetricKey& publicKey) const
{
    publicKey.CopyFrom(privateKey);
    publicKey.clear_chaincode();
    publicKey.clear_key();
    publicKey.set_mode(proto::KEYMODE_PUBLIC);
    BinarySecret plaintextKey(
        OT::App().Crypto().AES().InstantiateBinarySecretSP());
    OTPasswordData password(__FUNCTION__);
    const bool decrypted = DecryptPrivateKey(
        privateKey.encryptedkey(),
        password,
        *plaintextKey);

    if (!decrypted) { return false; }

    Data key;
    const bool output = ScalarBaseMultiply(*plaintextKey, key);

    if (output) {
        publicKey.set_key(key.GetPointer(), key.GetSize());

        return true;
    }

    return false;
}

bool Ecdsa::SeedToCurveKey(
    __attribute__((unused)) const OTPassword& seed,
    __attribute__((unused)) OTPassword& privateKey,
    __attribute__((unused)) Data& publicKey) const
{
    otErr << __FUNCTION__ << ": this engine does not support curve25519."
          << std::endl;

    return false;
}
} // namespace opentxs
