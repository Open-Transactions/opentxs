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

#include "opentxs/consensus/TransactionStatement.hpp"
#include "opentxs/core/crypto/CredentialSet.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/NymIDSource.hpp"
#include "opentxs/core/Proto.hpp"
#include "opentxs/core/Types.hpp"
#include "opentxs/network/ZMQ.hpp"

#include <atomic>
#include <cstdint>
#include <deque>
#include <list>
#include <map>
#include <memory>
#include <set>

namespace opentxs
{

class OTAsymmetricKey;
class Item;
class OTKeypair;
class Ledger;
class Message;
class OTPassword;
class OTPasswordData;
class ServerContract;
class Credential;
class OTTransaction;
class Tag;
class Wallet;

typedef std::deque<Message*> dequeOfMail;
typedef std::deque<int64_t> dequeOfTransNums;
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
    Nym(const Nym&) = default;

private:
    std::string alias_;
    uint32_t version_{0};
    std::uint32_t index_{0};
    std::atomic<std::uint64_t> revision_;
    proto::CredentialIndexMode mode_{proto::CREDINDEX_ERROR};

    Nym& operator=(const Nym&);

    bool m_bMarkForDeletion;  // Default FALSE. When set to true, saves a
                              // "DELETED" flag with this Nym,
    // for easy cleanup later when the server is doing some maintenance.
    String m_strNymfile;  // This contains the request numbers and other user
                          // acct info. XML.
    String m_strVersion;  // This goes with the Nymfile
    std::shared_ptr<NymIDSource> source_;  // Hash this to form the NymID. Can
                                           // be a
    // public key, or a URL, or DN info from a
    // cert, etc.
    String m_strDescription;
    Identifier m_nymID;  // Hashed-ID formed by hashing the Nym's public key.
    mapOfIdentifiers m_mapInboxHash;   // Whenever client downloads Inbox, its
                                       // hash is stored here. (When downloading
                                       // account, can compare ITS inbox hash to
                                       // this one, to see if I already have
                                       // latest one.)
    mapOfIdentifiers m_mapOutboxHash;  // Whenever client downloads Outbox, its
                                       // hash is stored here. (When downloading
    // account, can compare ITS outbox hash to
    // this one, to see if I already have
    // latest one.)

    // NOTE: these dequeOfMail objects are only currently stored in the Nym for
    // convenience.
    // They don't have to be stored in here.
    dequeOfMail m_dequeOutpayments; // Any outoing payments sent by this Nym.
                                    // (And not yet deleted.) (payments screen.)
    // (SERVER side)
    // Using strings here to avoid juggling memory crap.
    std::set<std::string> m_setAccounts;  // A list of asset account IDs. Server
                                          // side only (client side uses wallet;
                                          // has multiple servers.)
    // (SERVER side.)
    int64_t m_lUsageCredits;  // Server-side. The usage credits available for
                              // this Nym. Infinite if negative.
    mapOfCredentialSets m_mapCredentialSets;  // The credentials for this Nym.
                                              // (Each with a master key
                                              // credential and various
                                              // child credentials.)
    mapOfCredentialSets m_mapRevokedSets;     // We keep track of old master
    // credentials after they are revoked.
    String::List m_listRevokedIDs;  // std::string list, any revoked Credential
                                    // IDs. (Mainly for child credentials)
public:
    EXPORT std::string Alias() const;
    EXPORT bool SetAlias(const std::string& alias);
    EXPORT std::uint64_t Revision() const;
    EXPORT void GetPrivateCredentials(
        String& strCredList,
        String::Map* pmapCredFiles = nullptr);
    EXPORT const serializedCredentialIndex asPublicNym() const;
    EXPORT size_t GetMasterCredentialCount() const;
    EXPORT size_t GetRevokedCredentialCount() const;
    EXPORT CredentialSet* GetRevokedCredential(const String& strID);
    EXPORT const CredentialSet* GetMasterCredentialByIndex(
        int32_t nIndex) const;
    EXPORT const CredentialSet* GetRevokedCredentialByIndex(
        int32_t nIndex) const;
    EXPORT const Credential* GetChildCredential(
        const String& strMasterID,
        const String& strChildCredID) const;
    EXPORT std::shared_ptr<const proto::Credential> MasterCredentialContents(
        const std::string& id) const;
    EXPORT std::shared_ptr<const proto::Credential> RevokedCredentialContents(
        const std::string& id) const;
    EXPORT int32_t ChildCredentialCount(const std::string& masterID) const;
    EXPORT std::string ChildCredentialID(
        const std::string& masterID,
        const uint32_t index) const;
    EXPORT std::shared_ptr<const proto::Credential> ChildCredentialContents(
        const std::string& masterID,
        const std::string& childID) const;
    EXPORT bool hasCapability(const NymCapability& capability) const;

private:
    const CredentialSet* MasterCredential(const String& strID) const;
    CredentialSet* GetMasterCredential(const String& strID);
    // Generic function used by the below functions.
    bool GetHash(
        const mapOfIdentifiers& the_map,
        const std::string& str_id,
        Identifier& theOutput) const;  // client-side
    bool SetHash(
        mapOfIdentifiers& the_map,
        const std::string& str_id,
        const Identifier& theInput);  // client-side

public:
    // This functions are for the latest downloaded inbox's hash.
    // (If the hash that appears in the account is different, then
    // your inbox is old -- download it again.)
    //
    // This saves you having to download it many times when it has not even
    // changed.
    //
    EXPORT bool GetInboxHash(
        const std::string& acct_id,
        Identifier& theOutput) const;  // client-side
    EXPORT bool SetInboxHash(
        const std::string& acct_id,
        const Identifier& theInput);  // client-side
    // This functions are for the latest downloaded outbox's hash.
    // (If the hash that appears in the account is different, then
    // your outbox is old -- download it again.)
    //
    // This saves you having to download it many times when it has not even
    // changed.
    //
    EXPORT bool GetOutboxHash(
        const std::string& acct_id,
        Identifier& theOutput) const;  // client-side
    EXPORT bool SetOutboxHash(
        const std::string& acct_id,
        const Identifier& theInput);  // client-side
    EXPORT const int64_t& GetUsageCredits() const { return m_lUsageCredits; }
    EXPORT void SetUsageCredits(const int64_t& lUsage)
    {
        m_lUsageCredits = lUsage;
    }
    inline void MarkForDeletion() { m_bMarkForDeletion = true; }
    inline bool IsMarkedForDeletion() const { return m_bMarkForDeletion; }
    inline void MarkAsUndeleted() { m_bMarkForDeletion = false; }
    inline std::set<std::string>& GetSetAssetAccounts()
    {
        return m_setAccounts;
    }  // stores acct IDs as std::string
    EXPORT Nym();
    EXPORT explicit Nym(const NymParameters& nymParameters);
    EXPORT explicit Nym(const Identifier& nymID);
    EXPORT explicit Nym(const String& strNymID);

private:
    EXPORT Nym(const String& name, const String& filename, const String& nymID);

public:
    EXPORT virtual ~Nym();
    EXPORT void Initialize();
    EXPORT bool VerifyPseudonym() const;

    // CALLER is responsible to delete the Nym ptr being returned
    // in this functions!
    //
    EXPORT static Nym* LoadPrivateNym(
        const Identifier& NYM_ID,
        bool bChecking = false,
        const String* pstrName = nullptr,
        const char* szFuncName = nullptr,
        const OTPasswordData* pPWData = nullptr,
        const OTPassword* pImportPassword = nullptr);
    EXPORT const OTAsymmetricKey& GetPublicAuthKey() const;  // Authentication
    const OTAsymmetricKey& GetPrivateAuthKey() const;
    EXPORT const OTAsymmetricKey& GetPublicEncrKey() const;  // Encryption
    const OTAsymmetricKey& GetPrivateEncrKey() const;
    EXPORT const OTAsymmetricKey& GetPublicSignKey() const;  // Signing
    const OTAsymmetricKey& GetPrivateSignKey() const;
    // OT uses the signature's metadata to narrow down its search for the
    // correct public key.
    EXPORT int32_t GetPublicKeysBySignature(
        listOfAsymmetricKeys& listOutput,
        const OTSignature& theSignature,
        char cKeyType = '0') const;  // 'S' (signing key) or
                                     // 'E' (encryption key)
                                     // or 'A'
                                     // (authentication key)
    EXPORT bool SaveCredentialIDs() const;

private:
    EXPORT void SaveCredentialsToTag(
        Tag& parent,
        String::Map* pmapPubInfo = nullptr,
        String::Map* pmapPriInfo = nullptr) const;
    serializedCredentialIndex SerializeCredentialIndex(
        const CredentialIndexModeFlag mode = ONLY_IDS) const;

public:
    bool LoadCredentialIndex(const serializedCredentialIndex& index);
    EXPORT bool LoadCredentials(
        bool bLoadPrivate = false,  // Loads public
                                    // credentials by
        // default. For private, pass true.
        const OTPasswordData* pPWData = nullptr,
        const OTPassword* pImportPassword = nullptr);
    // Like for when you are exporting a Nym from the wallet.
    EXPORT bool ReEncryptPrivateCredentials(
        bool bImporting,
        const OTPasswordData* pPWData = nullptr,  // bImporting=true, or
                                                  // false if exporting.
        const OTPassword* pImportPassword = nullptr);
    // The signer is whoever wanted to make sure these nym files haven't
    // changed.
    // Usually that means the server nym.  Most of the time, m_nymServer will be
    // used as signer.
    EXPORT bool LoadSignedNymfile(const Nym& SIGNER_NYM);
    EXPORT bool SaveSignedNymfile(const Nym& SIGNER_NYM);
    EXPORT bool LoadNymFromString(
        const String& strNym,
        bool& converted,
        String::Map* pMapCredentials = nullptr,  // pMapCredentials can be
                                                 // passed, if
        // you prefer to use a specific set,
        // instead of just loading the actual
        // set from storage (such as during
        // registration, when the credentials
        // have been sent inside a message.)
        String* pstrReason = nullptr,
        const OTPassword* pImportPassword = nullptr);
    EXPORT bool LoadPublicKey();
    EXPORT bool SavePseudonymWallet(Tag& parent) const;
    EXPORT bool SavePseudonym();  // saves to filename m_strNymfile

protected:
    EXPORT bool SavePseudonym(const char* szFoldername, const char* szFilename);

public:
    EXPORT bool SavePseudonym(String& strNym);
    EXPORT bool CompareID(const Identifier& theIdentifier) const
    {
        return (theIdentifier == m_nymID);
    }

    EXPORT bool CompareID(const Nym& RHS) const;
    EXPORT const NymIDSource& Source() const
    {
        return *source_;
    }  // Source for NymID for this credential. (Hash it to get ID.)

    EXPORT std::string PaymentCode() const;

    EXPORT const String& GetDescription() const
    {
        return m_strDescription;
    }  // Alternate download location for Nym's credential IDs. (Primary
       // location
       // being the source itself, but sometimes that's not feasible.)

    EXPORT void SetSource(const NymIDSource& source)
    {
        std::shared_ptr<NymIDSource> pSource =
            std::make_shared<NymIDSource>(source);
        source_ = pSource;
    }
    EXPORT void SetDescription(const String& strLocation)
    {
        m_strDescription = strLocation;
    }

private:
    EXPORT void SerializeNymIDSource(Tag& parent) const;

public:
    EXPORT const Identifier& GetConstID() const
    {
        return m_nymID;
    }  // CONST VERSION

    EXPORT void GetIdentifier(Identifier& theIdentifier) const;  // BINARY
                                                                 // VERSION
    EXPORT const Identifier& ID() const { return m_nymID; }
    EXPORT void SetIdentifier(const Identifier& theIdentifier);
    EXPORT void GetIdentifier(String& theIdentifier) const;  // STRING VERSION
    EXPORT void SetIdentifier(const String& theIdentifier);
    EXPORT void RemoveAllNumbers(const String* pstrNotaryID = nullptr);
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
    // you can pass both of those objects into this function, which must assume
    // that those pieces were already done
    // just prior to this call.
    EXPORT bool ResyncWithServer(
        const Ledger& theNymbox,
        const Nym& theMessageNym);

    // Whenever a Nym sends a payment, a copy is dropped into his Outpayments.
    // (Payments screen.)
    //
    EXPORT void AddOutpayments(Message& theMessage);  // a payments message is
                                                      // the original OTMessage
                                                      // that this Nym sent.
    EXPORT int32_t GetOutpaymentsCount() const;       // How many outpayments
                                                      // messages does
    // this Nym currently store?
    EXPORT Message* GetOutpaymentsByIndex(int32_t nIndex) const;  // Get a
                                                                  // specific
                                                                  // piece of
    // outpayments,
    // at a
    // specific index.
    EXPORT bool RemoveOutpaymentsByIndex(
        int32_t nIndex,
        bool bDeleteIt = true);  // if returns
                                 // false,
    // outpayments index was bad
    // (or something else must
    // have gone seriously
    // wrong.)

    EXPORT void ClearOutpayments();  // called by the destructor. (Not intended
                                     // to erase messages from local storage.)

    std::string AddChildKeyCredential(
        const Identifier& strMasterID,
        const NymParameters& nymParameters);

    void ClearCredentials();
    void ClearAll();
    EXPORT void DisplayStatistics(String& strOutput);

    EXPORT bool WriteCredentials() const;
    std::unique_ptr<proto::ContactData> ContactData() const;
    bool SetContactData(const proto::ContactData& data);

    std::unique_ptr<proto::VerificationSet> VerificationSet() const;
    bool SetVerificationSet(const proto::VerificationSet& data);

    bool Verify(const Data& plaintext, const proto::Signature& sig) const;
    zcert_t* TransportKey() const;

    template<class T>
    bool SignProto(
        T& input,
        proto::Signature& signature,
        const OTPasswordData* pPWData = nullptr) const {
            bool haveSig = false;

            for (auto& it: m_mapCredentialSets) {
                if (nullptr != it.second) {
                    bool success = it.second->SignProto(
                        input,
                        signature,
                        pPWData);

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

    template<class T>
    bool VerifyProto(T& input, proto::Signature& signature) const {
        proto::Signature signatureCopy;
        signatureCopy.CopyFrom(signature);
        signature.clear_signature();
        return Verify(proto::ProtoAsData<T>(input), signatureCopy);
    }
};

}  // namespace opentxs

#endif  // OPENTXS_CORE_OTPSEUDONYM_HPP
