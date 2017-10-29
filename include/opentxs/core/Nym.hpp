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

#ifndef OPENTXS_CORE_OTPSEUDONYM_HPP
#define OPENTXS_CORE_OTPSEUDONYM_HPP

#include "opentxs/core/crypto/CredentialSet.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Proto.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/network/ZMQ.hpp"

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

class ContactData;
class Credential;
class Item;
class Ledger;
class Message;
class NymIDSource;
class NymParameters;
class OTAsymmetricKey;
class OTKeypair;
class OTPassword;
class OTPasswordData;
class OTPayment;
class OTTransaction;
class ServerContract;
class Tag;
class Wallet;

typedef std::deque<Message*> dequeOfMail;
typedef std::deque<std::int64_t> dequeOfTransNums;
typedef std::map<std::string, Identifier> mapOfIdentifiers;
typedef std::map<std::string, CredentialSet*> mapOfCredentialSets;
typedef std::list<OTAsymmetricKey*> listOfAsymmetricKeys;
typedef proto::CredentialIndex serializedCredentialIndex;
typedef bool CredentialIndexModeFlag;

class Nym
{
    friend class Wallet;

public:
    static const CredentialIndexModeFlag ONLY_IDS = true;
    static const CredentialIndexModeFlag FULL_CREDS = false;

    // CALLER is responsible to delete the Nym ptr being returned
    // in this functions!
    EXPORT static Nym* LoadPrivateNym(
        const Identifier& NYM_ID,
        bool bChecking = false,
        const String* pstrName = nullptr,
        const char* szFuncName = nullptr,
        const OTPasswordData* pPWData = nullptr,
        const OTPassword* pImportPassword = nullptr);

    // Whenever a Nym sends a payment, a copy is dropped std::into his
    // Outpayments.
    // (Payments screen.)
    // A payments message is the original OTMessage that this Nym sent.
    EXPORT void AddOutpayments(Message& theMessage);
    EXPORT std::string Alias() const;
    EXPORT const serializedCredentialIndex asPublicNym() const;
    EXPORT std::shared_ptr<const proto::Credential> ChildCredentialContents(
        const std::string& masterID,
        const std::string& childID) const;
    EXPORT std::int32_t ChildCredentialCount(const std::string& masterID) const;
    EXPORT const class ContactData& Claims() const;
    EXPORT bool CompareID(const Nym& RHS) const;
    EXPORT const Credential* GetChildCredential(
        const String& strMasterID,
        const String& strChildCredID) const;
    EXPORT std::string ChildCredentialID(
        const std::string& masterID,
        const std::uint32_t index) const;
    EXPORT bool CompareID(const Identifier& theIdentifier) const
    {
        return (theIdentifier == m_nymID);
    }
    EXPORT const Identifier& GetConstID() const { return m_nymID; }
    EXPORT const String& GetDescription() const { return m_strDescription; }
    EXPORT bool GetInboxHash(
        const std::string& acct_id,
        Identifier& theOutput) const;  // client-side
    EXPORT const CredentialSet* GetMasterCredentialByIndex(
        std::int32_t nIndex) const;
    EXPORT void GetIdentifier(Identifier& theIdentifier) const;
    EXPORT void GetIdentifier(String& theIdentifier) const;
    EXPORT std::size_t GetMasterCredentialCount() const;
    EXPORT bool GetOutboxHash(
        const std::string& acct_id,
        Identifier& theOutput) const;  // client-side
    EXPORT Message* GetOutpaymentsByIndex(const std::int32_t nIndex) const;
    EXPORT Message* GetOutpaymentsByTransNum(
        const std::int64_t lTransNum,
        std::unique_ptr<OTPayment>* pReturnPayment = nullptr,
        std::int32_t* pnReturnIndex = nullptr) const;
    EXPORT std::int32_t GetOutpaymentsCount() const;
    EXPORT const OTAsymmetricKey& GetPrivateAuthKey() const;
    EXPORT const OTAsymmetricKey& GetPrivateEncrKey() const;
    EXPORT const OTAsymmetricKey& GetPrivateSignKey() const;
    EXPORT const OTAsymmetricKey& GetPublicAuthKey() const;
    EXPORT const OTAsymmetricKey& GetPublicEncrKey() const;
    // OT uses the signature's metadata to narrow down its search for the
    // correct public key.
    // 'S' (signing key) or
    // 'E' (encryption key) OR
    // 'A' (authentication key)
    EXPORT std::int32_t GetPublicKeysBySignature(
        listOfAsymmetricKeys& listOutput,
        const OTSignature& theSignature,
        char cKeyType = '0') const;
    EXPORT const OTAsymmetricKey& GetPublicSignKey() const;
    EXPORT const CredentialSet* GetRevokedCredentialByIndex(
        std::int32_t nIndex) const;
    EXPORT std::size_t GetRevokedCredentialCount() const;
    EXPORT const std::int64_t& GetUsageCredits() const
    {
        return m_lUsageCredits;
    }
    EXPORT bool hasCapability(const NymCapability& capability) const;
    EXPORT const Identifier& ID() const { return m_nymID; }
    EXPORT bool IsMarkedForDeletion() const { return m_bMarkForDeletion; }
    EXPORT std::shared_ptr<const proto::Credential> MasterCredentialContents(
        const std::string& id) const;
    EXPORT std::string Name() const;
    EXPORT bool Path(proto::HDPath& output) const;
    EXPORT std::uint64_t Revision() const;
    EXPORT std::shared_ptr<const proto::Credential> RevokedCredentialContents(
        const std::string& id) const;
    EXPORT bool SavePseudonymWallet(Tag& parent) const;
    EXPORT const NymIDSource& Source() const { return *source_; }
    EXPORT zcert_t* TransportKey() const;
    EXPORT std::unique_ptr<proto::VerificationSet> VerificationSet() const;
    EXPORT bool VerifyPseudonym() const;
    EXPORT bool WriteCredentials() const;

    EXPORT std::string AddChildKeyCredential(
        const Identifier& strMasterID,
        const NymParameters& nymParameters);
    EXPORT bool AddClaim(const Claim& claim);
    EXPORT bool AddPaymentCode(
        const class PaymentCode& code,
        const proto::ContactItemType currency,
        const bool primary,
        const bool active = true);
    EXPORT bool AddPreferredOTServer(const Identifier& id, const bool primary);
    EXPORT bool DeleteClaim(const Identifier& id);
    EXPORT void DisplayStatistics(String& strOutput);
    EXPORT void GetPrivateCredentials(
        String& strCredList,
        String::Map* pmapCredFiles = nullptr);
    EXPORT CredentialSet* GetRevokedCredential(const String& strID);
    EXPORT std::set<std::string>& GetSetAssetAccounts()
    {
        return m_setAccounts;
    }
    EXPORT bool LoadCredentialIndex(const serializedCredentialIndex& index);
    EXPORT bool LoadCredentials(
        bool bLoadPrivate = false,
        const OTPasswordData* pPWData = nullptr,
        const OTPassword* pImportPassword = nullptr);
    // pMapCredentials can be passed, if you prefer to use a specific set,
    // instead of just loading the actual set from storage (such as during
    // registration, when the credentials have been sent inside a message.)
    EXPORT bool LoadNymFromString(
        const String& strNym,
        bool& converted,
        String::Map* pMapCredentials = nullptr,
        String* pstrReason = nullptr,
        const OTPassword* pImportPassword = nullptr);
    EXPORT bool LoadPublicKey();
    // The signer is whoever wanted to make sure these nym files haven't
    // changed. Usually that means the server nym.  Most of the time,
    // m_nymServer will be used as signer.
    EXPORT bool LoadSignedNymfile(const Nym& SIGNER_NYM);
    EXPORT void MarkAsUndeleted() { m_bMarkForDeletion = false; }
    EXPORT void MarkForDeletion() { m_bMarkForDeletion = true; }
    EXPORT std::string PaymentCode() const;
    // Like for when you are exporting a Nym from the wallet.
    EXPORT bool ReEncryptPrivateCredentials(
        bool bImporting,
        const OTPasswordData* pPWData = nullptr,
        const OTPassword* pImportPassword = nullptr);

    // IMPORTANT NOTE: Not all outpayments have a transaction num!
    // Imagine if you sent a cash purse to someone, for example.
    // The cash withdrawal had a transNum, and the eventual cash
    // deposit will have a transNum, but the purse itself does NOT.
    // That's okay in your outpayments box since it's like an outmail
    // box. It's not a ledger, so the items inside don't need a txn#.
    EXPORT bool RemoveOutpaymentsByIndex(
        const std::int32_t nIndex,
        bool bDeleteIt = true);
    EXPORT bool RemoveOutpaymentsByTransNum(
        const std::int64_t lTransNum,
        bool bDeleteIt = true);
    // ** ResyncWithServer **
    //
    // Not for normal use! (Since you should never get out of sync with the
    // server in the first place.)
    // However, in testing, or if some bug messes up some data, or whatever, and
    // you absolutely need to
    // re-sync with a server, and you trust that server not to lie to you, then
    // this function will do the trick.
    // NOTE: Before calling this, you need to do a getNymbox() to download the
    // latest Nymbox, and you need to do
    // a registerNym() to download the server's copy of your Nym. You then
    // need to load that Nymbox from
    // local storage, and you need to load the server's message Nym out of the
    // registerNymResponse reply, so that
    // you can pass both of those objects std::into this function, which must
    // assume
    // that those pieces were already done
    // just prior to this call.
    EXPORT bool ResyncWithServer(
        const Ledger& theNymbox,
        const Nym& theMessageNym);
    EXPORT bool SavePseudonym(String& strNym);
    EXPORT bool SaveSignedNymfile(const Nym& SIGNER_NYM);
    EXPORT bool SetAlias(const std::string& alias);
    EXPORT bool SetCommonName(const std::string& name);
    EXPORT bool SetContactData(const proto::ContactData& data);
    EXPORT void SetDescription(const String& strLocation)
    {
        m_strDescription = strLocation;
    }
    EXPORT void SetIdentifier(const Identifier& theIdentifier);
    EXPORT void SetIdentifier(const String& theIdentifier);
    EXPORT bool SetInboxHash(
        const std::string& acct_id,
        const Identifier& theInput);  // client-side
    EXPORT bool SetOutboxHash(
        const std::string& acct_id,
        const Identifier& theInput);  // client-side
    EXPORT bool SetScope(
        const proto::ContactItemType type,
        const std::string& name,
        const bool primary);
    EXPORT void SetUsageCredits(const std::int64_t& lUsage)
    {
        m_lUsageCredits = lUsage;
    }
    EXPORT bool SetVerificationSet(const proto::VerificationSet& data);

    EXPORT Nym();
    EXPORT explicit Nym(const NymParameters& nymParameters);
    EXPORT explicit Nym(const Identifier& nymID);
    EXPORT explicit Nym(const String& strNymID);
    EXPORT Nym(const Nym&) = default;
    EXPORT ~Nym();

    template <class T>
    bool SignProto(
        T& input,
        proto::Signature& signature,
        const OTPasswordData* pPWData = nullptr) const
    {
        bool haveSig = false;

        for (auto& it : m_mapCredentialSets) {
            if (nullptr != it.second) {
                bool success = it.second->SignProto(input, signature, pPWData);

                if (success) {
                    haveSig = true;
                    break;
                } else {
                    otErr << __FUNCTION__ << ": Credential set "
                          << it.second->GetMasterCredID() << " could not "
                          << "sign protobuf." << std::endl;
                }
            }

            otErr << __FUNCTION__ << ": Did not find any credential sets "
                  << "capable of signing on this nym." << std::endl;
        }

        return haveSig;
    }

    template <class T>
    bool VerifyProto(T& input, proto::Signature& signature) const
    {
        proto::Signature signatureCopy;
        signatureCopy.CopyFrom(signature);
        signature.clear_signature();

        return Verify(proto::ProtoAsData<T>(input), signatureCopy);
    }

private:
    std::int32_t version_{0};
    std::int32_t index_{0};
    // (SERVER side.)
    std::int64_t m_lUsageCredits{-1};
    bool m_bMarkForDeletion{false};
    std::string alias_{""};
    mutable std::mutex lock_{};
    std::atomic<std::uint64_t> revision_{0};
    proto::CredentialIndexMode mode_{proto::CREDINDEX_ERROR};
    String m_strNymfile{""};
    String m_strVersion{""};
    String m_strDescription{""};
    Identifier m_nymID{};
    std::shared_ptr<NymIDSource> source_{nullptr};
    mutable std::unique_ptr<class ContactData> contact_data_;

    // The credentials for this Nym. (Each with a master key credential and
    // various child credentials.)
    mapOfCredentialSets m_mapCredentialSets{};
    mapOfCredentialSets m_mapRevokedSets{};
    // Revoked child credential IDs
    String::List m_listRevokedIDs{};
    // Whenever client downloads Inbox, its hash is stored here. (When
    // downloading account, can compare ITS inbox hash to this one, to see if I
    // already have latest one.)
    mapOfIdentifiers m_mapInboxHash{};
    // Whenever client downloads Outbox, its hash is stored here. (When
    // downloading account, can compare ITS outbox hash to this one, to see if I
    // already have latest one.)
    mapOfIdentifiers m_mapOutboxHash{};
    // Any outoing payments sent by this Nym. (And not yet deleted.) (payments
    // screen.)
    dequeOfMail m_dequeOutpayments{};
    // (SERVER side)
    // A list of asset account IDs. Server side only (client side uses wallet;
    // has multiple servers.)
    std::set<std::string> m_setAccounts{};

    bool GetHash(
        const mapOfIdentifiers& the_map,
        const std::string& str_id,
        Identifier& theOutput) const;
    void init_claims(const Lock& lock) const;
    const CredentialSet* MasterCredential(const String& strID) const;
    bool SaveCredentialIDs() const;
    void SaveCredentialsToTag(
        Tag& parent,
        String::Map* pmapPubInfo = nullptr,
        String::Map* pmapPriInfo = nullptr) const;
    serializedCredentialIndex SerializeCredentialIndex(
        const CredentialIndexModeFlag mode = ONLY_IDS) const;
    bool set_contact_data(const Lock& lock, const proto::ContactData& data);
    void SerializeNymIDSource(Tag& parent) const;
    bool Verify(const Data& plaintext, const proto::Signature& sig) const;
    bool verify_lock(const Lock& lock) const;

    bool add_contact_credential(
        const Lock& lock,
        const proto::ContactData& data);
    bool add_verification_credential(
        const Lock& lock,
        const proto::VerificationSet& data);
    void ClearAll();
    void ClearCredentials();
    void ClearOutpayments();
    CredentialSet* GetMasterCredential(const String& strID);
    void RemoveAllNumbers(const String* pstrNotaryID = nullptr);
    void revoke_contact_credentials(const Lock& lock);
    void revoke_verification_credentials(const Lock& lock);
    bool SavePseudonym(const char* szFoldername, const char* szFilename);
    bool SetHash(
        mapOfIdentifiers& the_map,
        const std::string& str_id,
        const Identifier& theInput);
    bool update_nym(const Lock& lock);

    Nym(const String& name,
        const String& filename,
        const Identifier& nymID,
        const proto::CredentialIndexMode mode = proto::CREDINDEX_ERROR);
    Nym(const String& name, const String& filename, const String& nymID);
    Nym& operator=(const Nym&) = delete;
};
}  // namespace opentxs

#endif  // OPENTXS_CORE_OTPSEUDONYM_HPP
