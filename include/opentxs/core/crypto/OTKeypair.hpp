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

#ifndef OPENTXS_CORE_CRYPTO_OTKEYPAIR_HPP
#define OPENTXS_CORE_CRYPTO_OTKEYPAIR_HPP

#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/OTAsymmetricKey.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Proto.hpp"
#include "opentxs/core/Types.hpp"

#include <cstdint>
#include <list>
#include <memory>

namespace opentxs
{

class Contract;
class Identifier;
class OTASCIIArmor;
class OTAsymmetricKey;
class OTPassword;
class OTPasswordData;
class OTSignature;
class OTSignatureMetadata;
class String;

typedef std::list<OTAsymmetricKey*> listOfAsymmetricKeys;

/** Encapsulates public/private key (though often there may only be a public key
 * present, unless the nym belongs to you.) */
class OTKeypair
{
    friend class LowLevelKeyGenerator;

private:
    EXPORT OTKeypair(){};
    std::shared_ptr<OTAsymmetricKey> m_pkeyPublic;   // This nym's public key
    std::shared_ptr<OTAsymmetricKey> m_pkeyPrivate;  // This nym's private key
    proto::KeyRole role_{proto::KEYROLE_ERROR};

public:
    EXPORT bool MakeNewKeypair(const NymParameters& nymParameters);
    EXPORT bool ReEncrypt(const OTPassword& theExportPassword, bool bImporting);
    // a Nym to/from the wallet.
    EXPORT bool CalculateID(Identifier& theOutput) const;

    // PRIVATE KEY functions
    EXPORT bool HasPrivateKey() const;

    // Return the private key as an OTAsymmetricKey object
    // TODO this violates encapsulation and should be deprecated
    EXPORT const OTAsymmetricKey& GetPrivateKey() const;

    EXPORT bool hasCapability(const NymCapability& capability) const;

    // PUBLIC KEY functions
    EXPORT bool HasPublicKey() const;
    // Return the public key as an OTAsymmetricKey object
    // TODO this violates encapsulation and should be deprecated
    EXPORT const OTAsymmetricKey& GetPublicKey() const;
    // Get a public key as an opentxs::String.
    // This form is used in all cases except for the NymIDSource
    // of a self-signed MasterCredential
    EXPORT bool GetPublicKey(String& strKey) const;

    // TODO this violates encapsulation and should be deprecated
    EXPORT int32_t GetPublicKeyBySignature(
        listOfAsymmetricKeys& listOutput,  // inclusive means, return keys when
                                           // theSignature has no metadata.
        const OTSignature& theSignature,
        bool bInclusive = false) const;
    EXPORT OTKeypair(
        const NymParameters& nymParameters,
        const proto::KeyRole role = proto::KEYROLE_ERROR);
    EXPORT OTKeypair(
        const proto::AsymmetricKey& serializedPubkey,
        const proto::AsymmetricKey& serializedPrivkey);
    EXPORT explicit OTKeypair(const proto::AsymmetricKey& serializedPubkey);
    EXPORT ~OTKeypair();

    serializedAsymmetricKey Serialize(bool privateKey = false) const;
    bool Verify(const Data& plaintext, const proto::Signature& sig) const;
    bool TransportKey(Data& publicKey, OTPassword& privateKey) const;

    template<class C>
    bool SignProto(
        C& serialized,
        proto::Signature& signature,
        const String& credID = String(""),
        const OTPasswordData* pPWData = nullptr) const
            {
                if (!m_pkeyPrivate) {
                    otErr << __FUNCTION__ << ": Missing private key. Can not "
                          << "sign." << std::endl;

                    return false;
                }

                return m_pkeyPrivate->SignProto<C>(
                    serialized,
                    signature,
                    credID,
                    pPWData);
            }
};

}  // namespace opentxs

#endif  // OPENTXS_CORE_CRYPTO_OTKEYPAIR_HPP
