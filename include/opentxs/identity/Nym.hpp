// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_IDENTITY_NYM_HPP
#define OPENTXS_IDENTITY_NYM_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/crypto/key/Keypair.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <cstdint>
#include <memory>
#include <set>
#include <string>

namespace opentxs
{
namespace identity
{
class Nym
{
public:
    using Serialized = proto::CredentialIndex;

    EXPORT static const VersionNumber DefaultVersion;
    EXPORT static const VersionNumber MaxVersion;

    EXPORT virtual std::string Alias() const = 0;
    EXPORT virtual const Serialized asPublicNym() const = 0;
    EXPORT virtual std::string BestEmail() const = 0;
    EXPORT virtual std::string BestPhoneNumber() const = 0;
    EXPORT virtual std::string BestSocialMediaProfile(
        const proto::ContactItemType type) const = 0;
    EXPORT virtual const class ContactData& Claims() const = 0;
    EXPORT virtual bool CompareID(const Nym& RHS) const = 0;
    EXPORT virtual bool CompareID(const identifier::Nym& rhs) const = 0;
    EXPORT virtual VersionNumber ContactCredentialVersion() const = 0;
    EXPORT virtual VersionNumber ContactDataVersion() const = 0;
    EXPORT virtual std::set<OTIdentifier> Contracts(
        const proto::ContactItemType currency,
        const bool onlyActive) const = 0;
    EXPORT virtual std::string EmailAddresses(bool active = true) const = 0;
    EXPORT virtual const String& GetDescription() const = 0;
    EXPORT virtual void GetIdentifier(identifier::Nym& theIdentifier) const = 0;
    EXPORT virtual void GetIdentifier(String& theIdentifier) const = 0;
    EXPORT virtual const crypto::key::Asymmetric& GetPrivateAuthKey(
        proto::AsymmetricKeyType keytype = proto::AKEYTYPE_NULL) const = 0;
    EXPORT virtual const crypto::key::Asymmetric& GetPrivateEncrKey(
        proto::AsymmetricKeyType keytype = proto::AKEYTYPE_NULL) const = 0;
    EXPORT virtual const crypto::key::Asymmetric& GetPrivateSignKey(
        proto::AsymmetricKeyType keytype = proto::AKEYTYPE_NULL) const = 0;

    EXPORT virtual const crypto::key::Asymmetric& GetPublicAuthKey(
        proto::AsymmetricKeyType keytype = proto::AKEYTYPE_NULL) const = 0;
    EXPORT virtual const crypto::key::Asymmetric& GetPublicEncrKey(
        proto::AsymmetricKeyType keytype = proto::AKEYTYPE_NULL) const = 0;
    // OT uses the signature's metadata to narrow down its search for the
    // correct public key.
    // 'S' (signing key) or
    // 'E' (encryption key) OR
    // 'A' (authentication key)
    EXPORT virtual std::int32_t GetPublicKeysBySignature(
        crypto::key::Keypair::Keys& listOutput,
        const Signature& theSignature,
        char cKeyType = '0') const = 0;
    EXPORT virtual const crypto::key::Asymmetric& GetPublicSignKey(
        proto::AsymmetricKeyType keytype = proto::AKEYTYPE_NULL) const = 0;
    EXPORT virtual bool HasCapability(
        const NymCapability& capability) const = 0;
    EXPORT virtual const identifier::Nym& ID() const = 0;

    /* Encrypt a symmetric key's password
     *
     *  \param[in]  password     The password for the key
     *  \param[in]  key          The symmetric key whose password will be
     *                           encrypted
     *  \param[out] output       The encrypted form of password
     */
    EXPORT virtual bool Lock(
        const OTPassword& password,
        crypto::key::Symmetric& key,
        proto::Ciphertext& output) const = 0;
    EXPORT virtual std::string Name() const = 0;

    /* Decrypt a symmetric key's password, then use that password to decrypt the
     * symmetric key itself
     *
     *  \param[in]    input        The encrypted password
     *  \param[inout] key          The symmetric key to be unlocked
     *  \param[out]   password     The decrypted password
     */
    EXPORT virtual bool Open(
        const proto::SessionKey& input,
        crypto::key::Symmetric& key,
        OTPassword& password,
        const PasswordPrompt& reason) const = 0;
    EXPORT virtual bool Path(proto::HDPath& output) const = 0;
    EXPORT virtual std::string PaymentCode(
        const PasswordPrompt& reason) const = 0;
    EXPORT virtual std::string PhoneNumbers(bool active = true) const = 0;
    EXPORT virtual std::uint64_t Revision() const = 0;

    /* Encrypt a symmetric key's password
     *
     *  \param[in]  password     The password for the key
     *  \param[in]  key          The symmetric key whose password will be
     *                           encrypted
     *  \param[out] output       The encrypted form of password
     */
    EXPORT virtual bool Seal(
        const OTPassword& password,
        crypto::key::Symmetric& key,
        proto::SessionKey& output,
        const PasswordPrompt& reason) const = 0;
    EXPORT virtual void SerializeNymIDSource(Tag& parent) const = 0;
    EXPORT virtual bool Sign(
        const ProtobufType& input,
        const proto::SignatureRole role,
        proto::Signature& signature,
        const PasswordPrompt& reason,
        const proto::HashType hash = proto::HASHTYPE_ERROR) const = 0;
    EXPORT virtual std::string SocialMediaProfiles(
        const proto::ContactItemType type,
        bool active = true) const = 0;
    EXPORT virtual const std::set<proto::ContactItemType>
    SocialMediaProfileTypes() const = 0;
    EXPORT virtual const NymIDSource& Source() const = 0;
    EXPORT virtual std::unique_ptr<OTPassword> TransportKey(
        Data& pubkey,
        const PasswordPrompt& reason) const = 0;

    /* Decrypt a symmetric key's password, then use that password to decrypt the
     * symmetric key itself
     *
     *  \param[in]    input        The encrypted password
     *  \param[inout] key          The symmetric key to be unlocked
     *  \param[out]   password     The decrypted password
     */
    EXPORT virtual bool Unlock(
        const proto::Ciphertext& input,
        crypto::key::Symmetric& key,
        OTPassword& password) const = 0;
    EXPORT virtual VersionNumber VerificationCredentialVersion() const = 0;
    EXPORT virtual std::unique_ptr<proto::VerificationSet> VerificationSet()
        const = 0;
    EXPORT virtual bool Verify(
        const ProtobufType& input,
        proto::Signature& signature,
        const PasswordPrompt& reason) const = 0;
    EXPORT virtual bool VerifyPseudonym(const PasswordPrompt& reason) const = 0;

    EXPORT virtual std::string AddChildKeyCredential(
        const Identifier& strMasterID,
        const NymParameters& nymParameters,
        const PasswordPrompt& reason) = 0;
    EXPORT virtual bool AddClaim(
        const Claim& claim,
        const PasswordPrompt& reason) = 0;
    EXPORT virtual bool AddContract(
        const identifier::UnitDefinition& instrumentDefinitionID,
        const proto::ContactItemType currency,
        const PasswordPrompt& reason,
        const bool primary,
        const bool active = true) = 0;
    EXPORT virtual bool AddEmail(
        const std::string& value,
        const PasswordPrompt& reason,
        const bool primary,
        const bool active) = 0;
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    EXPORT virtual bool AddPaymentCode(
        const class PaymentCode& code,
        const proto::ContactItemType currency,
        const PasswordPrompt& reason,
        const bool primary,
        const bool active = true) = 0;
#endif
    EXPORT virtual bool AddPhoneNumber(
        const std::string& value,
        const PasswordPrompt& reason,
        const bool primary,
        const bool active) = 0;
    EXPORT virtual bool AddPreferredOTServer(
        const Identifier& id,
        const PasswordPrompt& reason,
        const bool primary) = 0;
    EXPORT virtual bool AddSocialMediaProfile(
        const std::string& value,
        const proto::ContactItemType type,
        const PasswordPrompt& reason,
        const bool primary,
        const bool active) = 0;
    EXPORT virtual bool DeleteClaim(
        const Identifier& id,
        const PasswordPrompt& reason) = 0;
    EXPORT virtual bool SetCommonName(
        const std::string& name,
        const PasswordPrompt& reason) = 0;
    EXPORT virtual bool SetContactData(
        const proto::ContactData& data,
        const PasswordPrompt& reason) = 0;
    EXPORT virtual void SetDescription(const String& strLocation) = 0;
    EXPORT virtual bool SetScope(
        const proto::ContactItemType type,
        const std::string& name,
        const PasswordPrompt& reason,
        const bool primary) = 0;
    EXPORT virtual bool SetVerificationSet(
        const proto::VerificationSet& data,
        const PasswordPrompt& reason) = 0;

    EXPORT virtual ~Nym() = default;

protected:
    Nym() = default;

private:
    Nym(const Nym&) = delete;
    Nym(Nym&&) = delete;
    Nym& operator=(const Nym&) = delete;
    Nym& operator=(Nym&&) = delete;
};
}  // namespace identity
}  // namespace opentxs
#endif
