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

#ifndef OPENTXS_CORE_CRYPTO_KEYCREDENTIAL_HPP
#define OPENTXS_CORE_CRYPTO_KEYCREDENTIAL_HPP

#include "opentxs/core/crypto/Credential.hpp"
#include "opentxs/core/crypto/OTKeypair.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/Proto.hpp"
#include "opentxs/core/Types.hpp"

#include <memory>

// A nym contains a list of credential sets.
// The whole purpose of a Nym is to be an identity, which can have
// master credentials.
//
// Each CredentialSet contains list of Credentials. One of the
// Credentials is a MasterCredential, and the rest are ChildCredentials
// signed by the MasterCredential.
//
// A Credential may contain keys, in which case it is a KeyCredential.
//
// Credentials without keys might be an interface to a hardware device
// or other kind of external encryption and authentication system.
//
// Non-key Credentials are not yet implemented.
//
// Each KeyCredential has 3 OTKeypairs: encryption, signing, and authentication.
// Each OTKeypair has 2 OTAsymmetricKeys (public and private.)
//
// A MasterCredential must be a KeyCredential, and is only used to sign
// ChildCredentials
//
// ChildCredentials are used for all other actions, and never sign other
// Credentials

namespace opentxs
{

class OTAsymmetricKey;
class CredentialSet;
class OTPassword;
class OTPasswordData;

typedef std::list<OTAsymmetricKey*> listOfAsymmetricKeys;

/// KeyCredential
/// A form of Credential that contains 3 key pairs: signing,
/// authentication, and encryption.
/// We won't use KeyCredential directly but only as a common base class for
/// ChildKeyCredential and MasterCredential.
///
class KeyCredential : public Credential
{
private:
    typedef Credential ot_super;
    friend class CredentialSet;
    KeyCredential() = delete;

    bool addKeytoSerializedKeyCredential(
        proto::KeyCredential& credential,
        const bool getPrivate,
        const proto::KeyRole role) const;
    bool addKeyCredentialtoSerializedCredential(
        serializedCredential credential,
        const bool addPrivate) const;
    bool VerifySig(
        const Lock& lock,
        const proto::Signature& sig,
        const CredentialModeFlag asPrivate = PRIVATE_VERSION) const;
    bool VerifySignedBySelf(const Lock& lock) const;

#if OT_CRYPTO_SUPPORTED_KEY_HD
    std::shared_ptr<OTKeypair> DeriveHDKeypair(
        const OTPassword& seed,
        const std::string& fingerprint,
        const uint32_t nym,
        const uint32_t credset,
        const uint32_t credindex,
        const EcdsaCurve& curve,
        const proto::KeyRole role);
#endif

protected:
    serializedCredential serialize(
        const Lock& lock,
        const SerializationModeFlag asPrivate,
        const SerializationSignatureFlag asSigned) const override;
    bool verify_internally(const Lock& lock) const override;

    bool New(const NymParameters& nymParameters) override;
    virtual bool SelfSign(
        const OTPassword* exportPassword = nullptr,
        const OTPasswordData* pPWData = nullptr,
        const bool onlyPrivate = false);

    KeyCredential(
        CredentialSet& owner,
        const NymParameters& nymParameters);
    KeyCredential(
        CredentialSet& owner,
        const proto::Credential& serializedCred);

public:
    std::shared_ptr<OTKeypair> m_SigningKey;
    std::shared_ptr<OTKeypair> m_AuthentKey;
    std::shared_ptr<OTKeypair> m_EncryptKey;

    bool ReEncryptKeys(const OTPassword& theExportPassword, bool bImporting);
    EXPORT int32_t GetPublicKeysBySignature(
        listOfAsymmetricKeys& listOutput, const OTSignature& theSignature,
        char cKeyType = '0') const; // 'S' (signing key) or
                                    // 'E' (encryption key)
                                    // or 'A'
                                    // (authentication key)

    bool hasCapability(const NymCapability& capability) const override;

    using ot_super::Verify;
    bool Verify(
        const Data& plaintext,
        const proto::Signature& sig,
        const proto::KeyRole key = proto::KEYROLE_SIGN) const override;
    bool TransportKey(Data& publicKey, OTPassword& privateKey) const override;

    virtual ~KeyCredential() = default;

    template<class C>
    bool SignProto(
        C& serialized,
        proto::Signature& signature,
        proto::KeyRole key = proto::KEYROLE_SIGN,
        const OTPasswordData* pPWData = nullptr) const
            {
                const OTKeypair* keyToUse = nullptr;

                switch (key) {
                    case (proto::KEYROLE_AUTH) :
                        keyToUse = m_AuthentKey.get();
                        break;
                    case (proto::KEYROLE_SIGN) :
                        keyToUse = m_SigningKey.get();
                        break;
                    default :
                        otErr << __FUNCTION__ << ": Can not sign with the "
                              << "specified key." << std::endl;
                        return false;
                }

                if (nullptr != keyToUse) {
                    return keyToUse->SignProto<C>(
                        serialized,
                        signature,
                        String(id_),
                        pPWData);
                }

                return false;
            }
};
} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_KEYCREDENTIAL_HPP
