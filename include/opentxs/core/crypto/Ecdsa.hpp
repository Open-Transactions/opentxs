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

#ifndef OPENTXS_CORE_CRYPTO_ECDSA_HPP
#define OPENTXS_CORE_CRYPTO_ECDSA_HPP

#include "opentxs/core/crypto/CryptoSymmetric.hpp"
#include "opentxs/core/Proto.hpp"

namespace opentxs
{
class AsymmetricKeyEC;
class Data;
class OTPassword;
class OTPasswordData;
class SymmetricKey;

class Ecdsa
{
protected:
    Ecdsa() = default;

    virtual bool AsymmetricKeyToECPubkey(
        const AsymmetricKeyEC& asymmetricKey,
        Data& pubkey) const;
    virtual bool AsymmetricKeyToECPrivkey(
        const proto::Ciphertext& asymmetricKey,
        const OTPasswordData& passwordData,
        OTPassword& privkey) const;
    virtual bool ECDH(
        const Data& publicKey,
        const OTPassword& privateKey,
        OTPassword& secret) const = 0;
    virtual bool ScalarBaseMultiply(
        const OTPassword& privateKey,
        Data& publicKey) const = 0;

public:
    static bool DecryptPrivateKey(
        const proto::Ciphertext& encryptedKey,
        const OTPasswordData& password,
        OTPassword& plaintextKey);
    static bool DecryptPrivateKey(
        const proto::Ciphertext& encryptedKey,
        const proto::Ciphertext& encryptedChaincode,
        const OTPasswordData& password,
        OTPassword& key,
        OTPassword& chaincode);
    static bool EncryptPrivateKey(
        const OTPassword& plaintextKey,
        const OTPasswordData& password,
        proto::Ciphertext& encryptedKey);
    static bool EncryptPrivateKey(
        const OTPassword& key,
        const OTPassword& chaincode,
        const OTPasswordData& password,
        proto::Ciphertext& encryptedKey,
        proto::Ciphertext& encryptedChaincode);

    virtual bool AsymmetricKeyToECPrivatekey(
        const AsymmetricKeyEC& asymmetricKey,
        const OTPasswordData& passwordData,
        OTPassword& privkey) const;
    virtual bool DecryptSessionKeyECDH(
        const AsymmetricKeyEC& privateKey,
        const AsymmetricKeyEC& publicKey,
        const OTPasswordData& password,
        SymmetricKey& sessionKey) const;
    virtual bool ECPrivatekeyToAsymmetricKey(
        const OTPassword& privkey,
        const OTPasswordData& passwordData,
        AsymmetricKeyEC& asymmetricKey) const;
    virtual bool ECPubkeyToAsymmetricKey(
        std::unique_ptr<Data>& pubkey,
        AsymmetricKeyEC& asymmetricKey) const;
    virtual bool EncryptSessionKeyECDH(
        const AsymmetricKeyEC& privateKey,
        const AsymmetricKeyEC& publicKey,
        const OTPasswordData& passwordData,
        SymmetricKey& sessionKey,
        OTPassword& newKeyPassword) const;
    virtual bool ExportECPrivatekey(
        const OTPassword& privkey,
        const OTPasswordData& password,
        AsymmetricKeyEC& asymmetricKey) const;
    virtual bool ImportECPrivatekey(
        const proto::Ciphertext& asymmetricKey,
        const OTPasswordData& password,
        OTPassword& privkey) const;
    virtual bool PrivateToPublic(
        const proto::AsymmetricKey& privateKey,
        proto::AsymmetricKey& publicKey) const;
    virtual bool RandomKeypair(
        OTPassword& privateKey,
        Data& publicKey) const = 0;
    virtual bool SeedToCurveKey(
        const OTPassword& seed,
        OTPassword& privateKey,
        Data& publicKey) const;

    virtual ~Ecdsa() = default;
};
}  // namespace opentxs
#endif  // OPENTXS_CORE_CRYPTO_ECDSA_HPP
