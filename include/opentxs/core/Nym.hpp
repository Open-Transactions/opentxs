// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#ifndef OPENTXS_CORE_NYM_HPP
#define OPENTXS_CORE_NYM_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/core/crypto/CredentialSet.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/NymFile.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/crypto/key/Keypair.hpp"
#include "opentxs/Proto.hpp"
#include "opentxs/Types.hpp"

#include <atomic>
#include <cstdint>
#include <deque>
#include <list>
#include <map>
#include <memory>
#include <mutex>
#include <set>

namespace opentxs
{
namespace api
{
namespace implementation
{
class Wallet;
}  // namespace implementation
}  // namespace api

typedef std::deque<Message*> dequeOfMail;
typedef std::deque<std::int64_t> dequeOfTransNums;
typedef std::map<std::string, OTIdentifier> mapOfIdentifiers;
typedef std::map<std::string, CredentialSet*> mapOfCredentialSets;
typedef bool CredentialIndexModeFlag;

class Nym : Lockable
{
public:
    static const CredentialIndexModeFlag ONLY_IDS = true;
    static const CredentialIndexModeFlag FULL_CREDS = false;

    EXPORT bool AddEmail(
        const std::string& value,
        const bool primary,
        const bool active);
    EXPORT bool AddPhoneNumber(
        const std::string& value,
        const bool primary,
        const bool active);
    EXPORT bool AddSocialMediaProfile(
        const std::string& value,
        const proto::ContactItemType type,
        const bool primary,
        const bool active);
    EXPORT std::string Alias() const;
    EXPORT const serializedCredentialIndex asPublicNym() const;
    EXPORT std::string BestEmail() const;
    EXPORT std::string BestPhoneNumber() const;
    EXPORT std::string BestSocialMediaProfile(
        const proto::ContactItemType type) const;
    EXPORT std::shared_ptr<const proto::Credential> ChildCredentialContents(
        const std::string& masterID,
        const std::string& childID) const;
    EXPORT const class ContactData& Claims() const;
    EXPORT bool CompareID(const Nym& RHS) const;
    EXPORT bool CompareID(const Identifier& rhs) const;
    EXPORT std::set<OTIdentifier> Contracts(
        const proto::ContactItemType currency,
        const bool onlyActive) const;
    EXPORT void DisplayStatistics(String& strOutput) const;
    EXPORT const std::vector<OTIdentifier> GetChildCredentialIDs(
        const std::string& masterID) const;
    EXPORT std::string EmailAddresses(bool active = true) const;
    EXPORT const String& GetDescription() const { return m_strDescription; }
    EXPORT const std::vector<OTIdentifier> GetMasterCredentialIDs() const;
    EXPORT void GetIdentifier(Identifier& theIdentifier) const;
    EXPORT void GetIdentifier(String& theIdentifier) const;
    EXPORT const crypto::key::Asymmetric& GetPrivateAuthKey(
        proto::AsymmetricKeyType keytype = proto::AKEYTYPE_NULL) const;
    EXPORT const crypto::key::Asymmetric& GetPrivateEncrKey(
        proto::AsymmetricKeyType keytype = proto::AKEYTYPE_NULL) const;
    EXPORT const crypto::key::Asymmetric& GetPrivateSignKey(
        proto::AsymmetricKeyType keytype = proto::AKEYTYPE_NULL) const;
    EXPORT const crypto::key::Asymmetric& GetPublicAuthKey(
        proto::AsymmetricKeyType keytype = proto::AKEYTYPE_NULL) const;
    EXPORT const crypto::key::Asymmetric& GetPublicEncrKey(
        proto::AsymmetricKeyType keytype = proto::AKEYTYPE_NULL) const;
    // OT uses the signature's metadata to narrow down its search for the
    // correct public key.
    // 'S' (signing key) or
    // 'E' (encryption key) OR
    // 'A' (authentication key)
    EXPORT std::int32_t GetPublicKeysBySignature(
        crypto::key::Keypair::Keys& listOutput,
        const Signature& theSignature,
        char cKeyType = '0') const;
    EXPORT const crypto::key::Asymmetric& GetPublicSignKey(
        proto::AsymmetricKeyType keytype = proto::AKEYTYPE_NULL) const;
    EXPORT const std::vector<OTIdentifier> GetRevokedCredentialIDs() const;
    EXPORT bool HasCapability(const NymCapability& capability) const;
    EXPORT const identifier::Nym& ID() const { return m_nymID; }

    /* Encrypt a symmetric key's password
     *
     *  \param[in]  password     The password for the key
     *  \param[in]  key          The symmetric key whose password will be
     *                           encrypted
     *  \param[out] output       The encrypted form of password
     */
    EXPORT bool Lock(
        const OTPassword& password,
        crypto::key::Symmetric& key,
        proto::Ciphertext& output) const;
    EXPORT std::shared_ptr<const proto::Credential> MasterCredentialContents(
        const std::string& id) const;
    EXPORT std::string Name() const;

    /* Decrypt a symmetric key's password, then use that password to decrypt the
     * symmetric key itself
     *
     *  \param[in]    input        The encrypted password
     *  \param[inout] key          The symmetric key to be unlocked
     *  \param[out]   password     The decrypted password
     */
    EXPORT bool Open(
        const proto::SessionKey& input,
        crypto::key::Symmetric& key,
        OTPassword& password) const;
    EXPORT bool Path(proto::HDPath& output) const;
    EXPORT std::string PhoneNumbers(bool active = true) const;
    EXPORT std::uint64_t Revision() const;
    EXPORT std::shared_ptr<const proto::Credential> RevokedCredentialContents(
        const std::string& id) const;
    EXPORT bool SavePseudonymWallet(Tag& parent) const;

    /* Encrypt a symmetric key's password
     *
     *  \param[in]  password     The password for the key
     *  \param[in]  key          The symmetric key whose password will be
     *                           encrypted
     *  \param[out] output       The encrypted form of password
     */
    EXPORT bool Seal(
        const OTPassword& password,
        crypto::key::Symmetric& key,
        proto::SessionKey& output) const;
    EXPORT void SerializeNymIDSource(Tag& parent) const;
    EXPORT std::string SocialMediaProfiles(
        const proto::ContactItemType type,
        bool active = true) const;
    const std::set<proto::ContactItemType> SocialMediaProfileTypes() const;
    EXPORT const NymIDSource& Source() const { return *source_; }
    EXPORT std::unique_ptr<OTPassword> TransportKey(Data& pubkey) const;

    /* Decrypt a symmetric key's password, then use that password to decrypt the
     * symmetric key itself
     *
     *  \param[in]    input        The encrypted password
     *  \param[inout] key          The symmetric key to be unlocked
     *  \param[out]   password     The decrypted password
     */
    EXPORT bool Unlock(
        const proto::Ciphertext& input,
        crypto::key::Symmetric& key,
        OTPassword& password) const;
    EXPORT std::unique_ptr<proto::VerificationSet> VerificationSet() const;
    EXPORT bool VerifyPseudonym() const;
    EXPORT bool WriteCredentials() const;

    EXPORT std::string AddChildKeyCredential(
        const Identifier& strMasterID,
        const NymParameters& nymParameters);
    EXPORT bool AddClaim(const Claim& claim);
    EXPORT bool AddContract(
        const Identifier& instrumentDefinitionID,
        const proto::ContactItemType currency,
        const bool primary,
        const bool active = true);
#if OT_CRYPTO_SUPPORTED_SOURCE_BIP47
    EXPORT bool AddPaymentCode(
        const class PaymentCode& code,
        const proto::ContactItemType currency,
        const bool primary,
        const bool active = true);
#endif
    EXPORT bool AddPreferredOTServer(const Identifier& id, const bool primary);
    EXPORT bool DeleteClaim(const Identifier& id);
    EXPORT void GetPrivateCredentials(
        String& strCredList,
        String::Map* pmapCredFiles = nullptr) const;
    EXPORT CredentialSet* GetRevokedCredential(const String& strID);
    EXPORT bool LoadCredentialIndex(const serializedCredentialIndex& index);
    EXPORT bool LoadCredentials(
        bool bLoadPrivate = false,
        const OTPasswordData* pPWData = nullptr,
        const OTPassword* pImportPassword = nullptr);
    EXPORT bool LoadPublicKey();
    EXPORT std::string PaymentCode() const;
    // Like for when you are exporting a Nym from the wallet.
    EXPORT bool ReEncryptPrivateCredentials(
        bool bImporting,
        const OTPasswordData* pPWData = nullptr,
        const OTPassword* pImportPassword = nullptr) const;

    EXPORT bool SetCommonName(const std::string& name);
    EXPORT bool SetContactData(const proto::ContactData& data);
    EXPORT void SetDescription(const String& strLocation)
    {
        eLock lock(shared_lock_);

        m_strDescription = strLocation;
    }
    EXPORT bool SetScope(
        const proto::ContactItemType type,
        const std::string& name,
        const bool primary);
    EXPORT bool SetVerificationSet(const proto::VerificationSet& data);

    template <class T>
    bool SignProto(
        T& input,
        proto::Signature& signature,
        const OTPasswordData* pPWData = nullptr) const
    {
        sLock lock(shared_lock_);

        bool haveSig = false;

        for (auto& it : m_mapCredentialSets) {
            if (nullptr != it.second) {
                bool success = it.second->SignProto(input, signature, pPWData);

                if (success) {
                    haveSig = true;
                    break;
                } else {
                    LogOutput(": Credential set ")(
                        it.second->GetMasterCredID())(" could not "
                                                      "sign protobuf.")
                        .Flush();
                }
            }

            LogOutput(": Did not find any credential sets "
                      "capable of signing on this nym.")
                .Flush();
        }

        return haveSig;
    }

    template <class T>
    bool VerifyProto(T& input, proto::Signature& signature) const
    {
        sLock lock(shared_lock_);

        proto::Signature signatureCopy;
        signatureCopy.CopyFrom(signature);
        signature.clear_signature();

        return Verify(proto::ProtoAsData<T>(input), signatureCopy);
    }

    EXPORT ~Nym();

private:
    friend api::implementation::Wallet;

    const api::Core& api_;
    std::int32_t version_{0};
    std::uint32_t index_{0};
    std::string alias_;
    std::atomic<std::uint64_t> revision_{0};
    proto::CredentialIndexMode mode_{proto::CREDINDEX_ERROR};
    OTString m_strVersion;
    OTString m_strDescription;
    const OTNymID m_nymID;
    std::shared_ptr<NymIDSource> source_{nullptr};
    mutable std::unique_ptr<class ContactData> contact_data_;

    // The credentials for this Nym. (Each with a master key credential and
    // various child credentials.)
    mapOfCredentialSets m_mapCredentialSets;
    mapOfCredentialSets m_mapRevokedSets;
    // Revoked child credential IDs
    String::List m_listRevokedIDs;

    template <typename T>
    const crypto::key::Asymmetric& get_private_auth_key(
        const T& lock,
        proto::AsymmetricKeyType keytype = proto::AKEYTYPE_NULL) const;
    template <typename T>
    const crypto::key::Asymmetric& get_private_sign_key(
        const T& lock,
        proto::AsymmetricKeyType keytype = proto::AKEYTYPE_NULL) const;
    template <typename T>
    const crypto::key::Asymmetric& get_public_sign_key(
        const T& lock,
        proto::AsymmetricKeyType keytype = proto::AKEYTYPE_NULL) const;
    bool has_capability(const eLock& lock, const NymCapability& capability)
        const;
    void init_claims(const eLock& lock) const;
    const CredentialSet* MasterCredential(const String& strID) const;
    void SaveCredentialsToTag(
        Tag& parent,
        String::Map* pmapPubInfo = nullptr,
        String::Map* pmapPriInfo = nullptr) const;
    serializedCredentialIndex SerializeCredentialIndex(
        const CredentialIndexModeFlag mode = ONLY_IDS) const;
    bool set_contact_data(const eLock& lock, const proto::ContactData& data);
    bool Verify(const Data& plaintext, const proto::Signature& sig) const;
    bool verify_pseudonym(const eLock& lock) const;

    bool add_contact_credential(
        const eLock& lock,
        const proto::ContactData& data);
    bool add_verification_credential(
        const eLock& lock,
        const proto::VerificationSet& data);
    void clear_credentials(const eLock& lock);
    void ClearCredentials();
    CredentialSet* GetMasterCredential(const String& strID);
    bool load_credentials(
        const eLock& lock,
        bool bLoadPrivate = false,
        const OTPasswordData* pPWData = nullptr,
        const OTPassword* pImportPassword = nullptr);
    bool load_credential_index(
        const eLock& lock,
        const serializedCredentialIndex& index);
    void revoke_contact_credentials(const eLock& lock);
    void revoke_verification_credentials(const eLock& lock);
    void SetAlias(const std::string& alias);
    bool update_nym(const eLock& lock, const std::int32_t version);

    Nym(const api::Core& api, const NymParameters& nymParameters);
    Nym(const api::Core& api,
        const identifier::Nym& nymID,
        const proto::CredentialIndexMode mode = proto::CREDINDEX_ERROR);
    Nym() = delete;
    Nym(const Nym&) = delete;
    Nym(Nym&&) = delete;
    Nym& operator=(const Nym&) = delete;
    Nym& operator=(Nym&&) = delete;
};
}  // namespace opentxs

#endif
