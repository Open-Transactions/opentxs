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

#ifndef OPENTXS_CORE_CRYPTO_CREDENTIAL_HPP
#define OPENTXS_CORE_CRYPTO_CREDENTIAL_HPP

#include "opentxs/core/contract/Signable.hpp"
#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Proto.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/Types.hpp"

#include <stdint.h>
#include <memory>
#include <string>

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

class Contract;
class CredentialSet;
class Identifier;
class NymParameters;
class OTPassword;
class OTPasswordData;

typedef std::shared_ptr<proto::Credential> serializedCredential;

class Credential : public Signable
{
public:
    static String CredentialTypeToString(proto::CredentialType credentialType);
    static serializedCredential ExtractArmoredCredential(
        const String& stringCredential);
    static serializedCredential ExtractArmoredCredential(
        const OTASCIIArmor& armoredCredential);
    static std::unique_ptr<Credential> Factory(
        CredentialSet& parent,
        const proto::Credential& serialized,
        const proto::KeyMode& mode,
        const proto::CredentialRole& role = proto::CREDROLE_ERROR);

    template<class C>
    static std::unique_ptr<C> Create(
        CredentialSet& owner,
        const NymParameters& nymParameters)
    {
        std::unique_ptr<C> credential;

        credential.reset(new C(owner, nymParameters));

        if (!credential) {
            otErr << __FUNCTION__ << ": Failed to construct credential."
                  << std::endl;

            return nullptr;
        }

        if (!credential->New(nymParameters)) {
            otErr << __FUNCTION__ << ": Failed to sign credential."
                  << std::endl;

            return nullptr;
        }

        if (!credential->Save()) {
            otErr << __FUNCTION__ << ": Failed to save credential."
                  << std::endl;

            return nullptr;
        }

        return credential;
    }

private:
    typedef Signable ot_super;
    Credential() = delete;

    // Syntax (non cryptographic) validation
    bool isValid(const Lock& lock) const;
    // Returns the serialized form to prevent unnecessary serializations
    bool isValid(const Lock& lock, serializedCredential& credential) const;

    Identifier GetID(const Lock& lock) const override;
    std::string Name() const override { return String(id_).Get(); }
    bool VerifyMasterID() const;
    bool VerifyNymID() const;
    bool verify_master_signature(const Lock& lock) const;

protected:
    proto::CredentialType type_ = proto::CREDTYPE_ERROR;
    proto::CredentialRole role_ = proto::CREDROLE_ERROR;
    proto::KeyMode mode_ = proto::KEYMODE_ERROR;
    CredentialSet* owner_backlink_ = nullptr; // Do not cleanup.
    String master_id_;
    String nym_id_;

    virtual serializedCredential serialize(
        const Lock& lock,
        const SerializationModeFlag asPrivate,
        const SerializationSignatureFlag asSigned) const;
    bool validate(const Lock& lock) const override;
    virtual bool verify_internally(const Lock& lock) const;

    bool AddMasterSignature(const Lock& lock);
    virtual bool New(const NymParameters& nymParameters);

    Credential(CredentialSet& owner, const proto::Credential& serializedCred);
    Credential(
        CredentialSet& owner,
        const std::uint32_t version,
        const NymParameters& nymParameters);

public:
    const String& MasterID() const { return master_id_; }
    const String& NymID() const { return nym_id_; }
    const proto::CredentialRole& Role() const { return role_; }
    SerializedSignature MasterSignature() const;
    const proto::KeyMode& Mode() const { return mode_; }
    bool Private() const { return (proto::KEYMODE_PRIVATE == mode_); }
    SerializedSignature SelfSignature(
        CredentialModeFlag version = PUBLIC_VERSION) const;
    SerializedSignature SourceSignature() const;
    proto::CredentialType Type() const;

    std::string asString(const bool asPrivate = false) const;
    virtual serializedCredential Serialized(
        const SerializationModeFlag asPrivate,
        const SerializationSignatureFlag asSigned) const;
    virtual bool hasCapability(const NymCapability& capability) const;

    virtual void ReleaseSignatures(const bool onlyPrivate);
    virtual bool Save() const;
    Data Serialize() const override;

    virtual bool GetContactData(
        std::unique_ptr<proto::ContactData>& contactData) const;
    virtual bool GetVerificationSet(
        std::unique_ptr<proto::VerificationSet>& verificationSet) const;

    bool Validate() const;
    virtual bool Verify(
        const Data& plaintext,
        const proto::Signature& sig,
        const proto::KeyRole key = proto::KEYROLE_SIGN) const;
    virtual bool Verify(
        const proto::Credential& credential,
        const proto::CredentialRole& role,
        const Identifier& masterID,
        const proto::Signature& masterSig) const;
    virtual bool TransportKey(Data& publicKey, OTPassword& privateKey) const;

    virtual ~Credential() = default;
};

} // namespace opentxs

#endif // OPENTXS_CORE_CRYPTO_CREDENTIAL_HPP
