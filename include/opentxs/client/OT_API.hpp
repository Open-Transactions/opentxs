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

#ifndef OPENTXS_CLIENT_OPENTRANSACTIONS_HPP
#define OPENTXS_CLIENT_OPENTRANSACTIONS_HPP

#include "opentxs/Forward.hpp"

#include "opentxs/api/client/Wallet.hpp"
#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/core/contract/peer/PeerObject.hpp"
#include "opentxs/core/crypto/NymParameters.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/Item.hpp"
#include "opentxs/core/Lockable.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/Types.hpp"

#include <cstdint>
#include <memory>
#include <mutex>
#include <set>
#include <tuple>

namespace opentxs
{
namespace api
{
namespace implementation
{
class Api;
}  // namespace implementation
}  // namespace api

/** AccountInfo: accountID, nymID, serverID, unitID */
using AccountInfo =
    std::tuple<OTIdentifier, OTIdentifier, OTIdentifier, OTIdentifier>;

// The C++ high-level interface to the Open Transactions client-side.
class OT_API : Lockable
{
public:
    /** AccountInfo: accountID, nymID, serverID, unitID*/
    typedef std::pair<std::unique_ptr<Ledger>, std::unique_ptr<Ledger>>
        ProcessInbox;

    EXPORT bool GetWalletFilename(String& strPath) const;
    EXPORT bool SetWalletFilename(const String& strPath) const;
    EXPORT OTWallet* GetWallet(const char* szFuncName = nullptr) const;

    inline OTClient* GetClient() const { return m_pClient.get(); }

    EXPORT bool SetWallet(const String& strFilename) const;
    EXPORT bool WalletExists() const;
    EXPORT bool LoadWallet() const;

    EXPORT time64_t GetTime() const;
    EXPORT bool NumList_Add(NumList& theList, const NumList& theNewNumbers)
        const;
    EXPORT bool NumList_Remove(NumList& theList, const NumList& theOldNumbers)
        const;
    EXPORT bool NumList_VerifyQuery(
        const NumList& theList,
        const NumList& theQueryNumbers) const;
    EXPORT bool NumList_VerifyAll(
        const NumList& theList,
        const NumList& theQueryNumbers) const;
    EXPORT std::int32_t NumList_Count(const NumList& theList) const;
    // Reading data about the local wallet.. presumably already loaded.

    EXPORT std::int32_t GetNymCount() const;
    EXPORT std::set<OTIdentifier> LocalNymList() const;
    EXPORT std::set<AccountInfo> Accounts() const;
    EXPORT std::int32_t GetAccountCount() const;

    EXPORT bool GetNym(
        std::int32_t iIndex,
        Identifier& NYM_ID,
        String& NYM_NAME) const;
    EXPORT bool GetAccount(
        std::int32_t iIndex,
        Identifier& THE_ID,
        String& THE_NAME) const;
    // In this case, the ID is input, the pointer is output.
    // Gets the data from Wallet.
    EXPORT const BasketContract* GetBasketContract(
        const Identifier& THE_ID,
        const char* szFuncName = nullptr) const;
    EXPORT std::shared_ptr<Account> GetAccount(
        const Identifier& THE_ID,
        const char* szFuncName = nullptr) const;

    EXPORT std::shared_ptr<Account> GetAccountPartialMatch(
        const std::string PARTIAL_ID,
        const char* szFuncName = nullptr) const;

    EXPORT static std::string NymIDFromPaymentCode(
        const std::string& paymentCode);
    /**   Add a single claim to the target nym's contact credential
     *    \param[in]  nymID the indentifier of the target nym
     *    \param[in]  section section containing the claim
     *    \param[in]  type claim type
     *    \param[in]  value claim value
     *    \param[in]  active true if the claim should have an active attribute
     *    \param[in]  primary true if the claim should have a primary attribute
     *    \param[in]  start beginning of valid time for the claim
     *    \param[in]  end end of valid time for the claim
     *    \return true for success, false for error
     */
    EXPORT bool AddClaim(
        NymData& nym,
        const proto::ContactSectionName& section,
        const proto::ContactItemType& type,
        const std::string& value,
        const bool primary = false,
        const bool active = true,
        const std::uint64_t start = 0,
        const std::uint64_t end = 0,
        const std::uint32_t version = 1) const;

    EXPORT std::shared_ptr<Account> GetOrLoadAccount(
        const Nym& theNym,
        const Identifier& ACCT_ID,
        const Identifier& NOTARY_ID,
        const char* szFuncName = nullptr) const;

    EXPORT std::shared_ptr<Account> GetOrLoadAccount(
        const Identifier& NYM_ID,
        const Identifier& ACCT_ID,
        const Identifier& NOTARY_ID,
        const char* szFuncName = nullptr) const;
    // The name is basically just a client-side label.
    // This function lets you change it.
    EXPORT bool SetNym_Alias(
        const Identifier& targetNymID,
        const Identifier& walletNymID,
        const String& name) const;

    EXPORT bool Rename_Nym(
        const Identifier& nymID,
        const std::string& name,
        const proto::ContactItemType type = proto::CITEMTYPE_ERROR,
        const bool primary = true) const;

    EXPORT bool SetAccount_Name(
        const Identifier& ACCT_ID,
        const Identifier& SIGNER_NYM_ID,
        const String& ACCT_NEW_NAME) const;

    // This works by checking to see if the Nym has a request number for the
    // given server.
    // That's why it's important, when registering at a specific server, to
    // immediately do a
    // "get request number" since that's what locks in the clients ability to be
    // able to tell
    // that it's registered there.
    EXPORT bool IsNym_RegisteredAtServer(
        const Identifier& NYM_ID,
        const Identifier& NOTARY_ID) const;
    EXPORT bool Wallet_ChangePassphrase() const;
    EXPORT std::string Wallet_GetPhrase() const;
    EXPORT std::string Wallet_GetSeed() const;
    EXPORT std::string Wallet_GetWords() const;
    EXPORT std::string Wallet_ImportSeed(
        const OTPassword& words,
        const OTPassword& passphrase) const;
    EXPORT bool Wallet_CanRemoveServer(const Identifier& NOTARY_ID) const;
    EXPORT bool Wallet_CanRemoveAssetType(
        const Identifier& INSTRUMENT_DEFINITION_ID) const;
    EXPORT bool Wallet_CanRemoveNym(const Identifier& NYM_ID) const;
    EXPORT bool Wallet_CanRemoveAccount(const Identifier& ACCOUNT_ID) const;
    // OT has the capability to export a Nym (normally stored in several files)
    // as an encoded
    // object (in base64-encoded form) and then import it again.
    //
    // Returns bool on success, and strOutput will contain the exported data.
    EXPORT bool Wallet_ExportNym(const Identifier& NYM_ID, String& strOutput)
        const;
    // OT has the capability to export a Nym (normally stored in several files)
    // as an encoded
    // object (in base64-encoded form) and then import it again.
    //
    // Returns bool on success, and if pNymID is passed in, will set it to the
    // new NymID.
    // Also on failure, if the Nym was already there with that ID, and if pNymID
    // is passed,
    // then it will be set to the ID that was already there.
    EXPORT bool Wallet_ImportNym(
        const String& FILE_CONTENTS,
        Identifier* pNymID = nullptr) const;
    // First three arguments denote the existing purse.
    // Fourth argument is the NEW purse being imported.
    // (Which may have a different owner Nym, or be protected
    // by a symmetric key instead of a Nym.)
    bool Wallet_ImportPurse(
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID,
        const Identifier& SIGNER_ID,  // We must know the SIGNER_ID in order to
                                      // know which "old purse" to load and
                                      // merge into. The New Purse may have a
                                      // different one, but its ownership will
                                      // be re-assigned in that case, as part
                                      // of the merging process, to SIGNER_ID.
                                      // Otherwise the New Purse might be
                                      // symmetrically encrypted (instead of
                                      // using a Nym) in which case again, its
                                      // ownership will be re-assigned from
                                      // that key, to SIGNER_ID, as part of the
                                      // merging process.
        const String& THE_PURSE,
        const String* pstrDisplay = nullptr) const;
    //
    // ENCODE, DECODE, SIGN, VERIFY, ENCRYPT, DECRYPT

    /** OT-encode a plaintext string.
     This will pack, compress, and base64-encode a plain string.
     Returns the base64-encoded string, or nullptr.
     */
    EXPORT bool Encode(
        const String& strPlaintext,
        String& strOutput,
        bool bLineBreaks = true) const;
    /** Decode an OT-encoded string (back to plaintext.)
    This will base64-decode, uncompress, and unpack an OT-encoded string.
    Returns the plaintext string, or nullptr.
    */
    EXPORT bool Decode(
        const String& strEncoded,
        String& strOutput,
        bool bLineBreaks = true) const;
    /** OT-ENCRYPT a plaintext string.
    This will encode, ENCRYPT, and encode a plain string.
    Returns the base64-encoded ciphertext, or nullptr.
    */
    EXPORT bool Encrypt(
        const Identifier& theRecipientNymID,
        const String& strPlaintext,
        String& strOutput) const;
    /** OT-DECRYPT an OT-encrypted string back to plaintext.
    Decrypts the base64-encoded ciphertext back into a normal string plaintext.
    Returns the plaintext string, or nullptr.
    */
    EXPORT bool Decrypt(
        const Identifier& theRecipientNymID,
        const String& strCiphertext,
        String& strOutput) const;
    /** OT-Sign a piece of flat text. (With no discernible bookends around it.)
        strType contains the OT type. For example, if you are trying to sign a
        ledger (which does not have any existing signatures on it) then you
       would
        pass LEDGER for strType, resulting in -----BEGIN OT SIGNED LEDGER-----
     */
    bool FlatSign(
        const Identifier& theSignerNymID,
        const String& strInput,
        const String& strContractType,
        String& strOutput) const;
    /** OT-Sign a CONTRACT.  (First signature)
    Tries to instantiate the contract object, based on the string passed in.
    Then it releases ALL signatures, and then signs the contract.
    Returns the signed contract, or nullptr if failure.
    */
    EXPORT bool SignContract(
        const Identifier& theSignerNymID,
        const String& strContract,
        String& strOutput) const;
    /** OT-Sign a CONTRACT.  (Add a signature)
    Tries to instantiate the contract object, based on the string passed in.
    Signs the contract, WITHOUT releasing any signatures that are already there.
    Returns the signed contract, or nullptr if failure.
    */
    EXPORT bool AddSignature(
        const Identifier& theSignerNymID,
        const String& strContract,
        String& strOutput) const;
    /** OT-Verify the signature on a CONTRACT.
     Returns true/false (success/fail)
     */
    EXPORT bool VerifySignature(
        const String& strContract,
        const Identifier& theSignerNymID,
        Contract** ppContract = nullptr) const;  // If you use this optional
                                                 // parameter, then YOU are
    // responsible to clean it up.

    /// Verify and Retrieve XML Contents.
    EXPORT bool VerifyAndRetrieveXMLContents(
        const String& strContract,
        const Identifier& theSignerNymID,
        String& strOutput) const;
    /// === Verify Account Receipt ===
    /// Returns bool. Verifies any asset account (intermediary files) against
    /// its own last signed receipt.
    /// Obviously this will fail for any new account that hasn't done any
    /// transactions yet, and thus has no receipts.
    EXPORT bool VerifyAccountReceipt(
        const Identifier& NOTARY_ID,
        const Identifier& NYM_ID,
        const Identifier& ACCOUNT_ID) const;

    // Returns an OTCheque pointer, or nullptr.
    // (Caller responsible to delete.)
    EXPORT Cheque* WriteCheque(
        const Identifier& NOTARY_ID,
        const std::int64_t& CHEQUE_AMOUNT,
        const time64_t& VALID_FROM,
        const time64_t& VALID_TO,
        const Identifier& SENDER_accountID,
        const Identifier& SENDER_NYM_ID,
        const String& CHEQUE_MEMO,
        const Identifier& pRECIPIENT_NYM_ID) const;

    // DISCARD CHEQUE (recover the transaction number for re-use, so the
    // cheque itself can be discarded.)
    EXPORT bool DiscardCheque(
        const Identifier& NOTARY_ID,
        const Identifier& NYM_ID,
        const Identifier& ACCT_ID,
        const String& THE_CHEQUE) const;

    // PROPOSE PAYMENT PLAN (called by Merchant)
    //
    // Returns an OTPaymentPlan pointer, or nullptr.
    // (Caller responsible to delete.)
    //
    // Payment Plan Delay, and Payment Plan Period, both default to 30 days (if
    // you pass 0),
    // measured in seconds.
    //
    // Payment Plan Length, and Payment Plan Max Payments, both default to 0,
    // which means
    // no maximum length and no maximum number of payments.
    EXPORT OTPaymentPlan* ProposePaymentPlan(
        const Identifier& NOTARY_ID,
        const time64_t& VALID_FROM,  // 0 defaults to the current time in
                                     // seconds
                                     // since Jan 1970.
        const time64_t& VALID_TO,  // 0 defaults to "no expiry." Otherwise this
                                   // value is ADDED to VALID_FROM. (It's a
                                   // length.)
        const Identifier* pSENDER_ACCT_ID,
        const Identifier& SENDER_NYM_ID,
        const String& PLAN_CONSIDERATION,  // like a memo.
        const Identifier& RECIPIENT_ACCT_ID,
        const Identifier& RECIPIENT_NYM_ID,
        // ----------------------------------------  // If it's above zero, the
        // initial
        const std::int64_t& INITIAL_PAYMENT_AMOUNT,  // amount will be processed
                                                     // after
        const time64_t& INITIAL_PAYMENT_DELAY,  // delay (seconds from now.)
        // ----------------------------------------  // AND SEPARATELY FROM
        // THIS...
        const std::int64_t& PAYMENT_PLAN_AMOUNT,  // The regular amount charged,
        const time64_t& PAYMENT_PLAN_DELAY,       // which begins occuring after
                                                  // delay
        const time64_t& PAYMENT_PLAN_PERIOD,  // (seconds from now) and happens
        // ----------------------------------------  // every period, ad
        // infinitum, until
        time64_t PAYMENT_PLAN_LENGTH = OT_TIME_ZERO,  // after the length (in
                                                      // seconds)
        std::int32_t PAYMENT_PLAN_MAX_PAYMENTS = 0    // expires, or after the
                                                      // maximum
        ) const;  // number of payments. These last

    // CONFIRM PAYMENT PLAN (called by Customer)
    EXPORT bool ConfirmPaymentPlan(
        const Identifier& NOTARY_ID,
        const Identifier& SENDER_NYM_ID,
        const Identifier& SENDER_ACCT_ID,
        const Identifier& RECIPIENT_NYM_ID,
        OTPaymentPlan& thePlan) const;
#if OT_CASH
    EXPORT Purse* LoadPurse(
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID,
        const Identifier& NYM_ID,
        const String* pstrDisplay = nullptr) const;
    EXPORT bool SavePurse(
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID,
        const Identifier& NYM_ID,
        Purse& THE_PURSE) const;
    EXPORT Purse* CreatePurse(
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID,
        const Identifier& OWNER_ID) const;
    EXPORT Purse* CreatePurse_Passphrase(
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID) const;
    // This is a low-level utility function. Probably should
    // make this private so people don't confuse it with the API.
    // All the purse functions use this.
    EXPORT OTNym_or_SymmetricKey* LoadPurseAndOwnerFromString(
        const Identifier& theNotaryID,
        const Identifier& theInstrumentDefinitionID,
        const String& strPurse,
        Purse& thePurse,          // output
        OTPassword& thePassword,  // Only used in the case of password-protected
                                  // purses. Passed in so it won't go out of
                                  // scope when return value has a member set to
                                  // point to it.
        bool bForEncrypting = true,  // true==encrypting,false==decrypting.
        const Identifier* pOWNER_ID = nullptr,  // This can be nullptr, **IF**
                                                // purse is password-protected.
                                                // (It's
        // just ignored in that case.) Otherwise MUST contain the
        // NymID for the Purse owner.
        const String* pstrDisplay1 = nullptr,
        const String* pstrDisplay2 = nullptr) const;
    EXPORT OTNym_or_SymmetricKey* LoadPurseAndOwnerForMerge(
        const String& strPurse,
        Purse& thePurse,          // output
        OTPassword& thePassword,  // Only used in the case of password-protected
                                  // purses. Passed in so it won't go out of
                                  // scope when pOwner is set to point to it.
        bool bCanBePublic = false,  // true==private nym isn't mandatory.
                                    // false==private nym IS mandatory.
                                    // (Only relevant if there's an owner.)
        const Identifier* pOWNER_ID = nullptr,  // This can be nullptr, **IF**
                                                // purse is password-protected.
                                                // (It's
        // just ignored in that case.) Otherwise if it's
        // Nym-protected, the purse will have a NymID on it already.
        // If not (it's optional), then pOWNER_ID is the ID it will
        // try next, before failing.
        const String* pstrDisplay = nullptr) const;
    EXPORT Token* Purse_Peek(
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID,
        const String& THE_PURSE,
        const Identifier* pOWNER_ID = nullptr,  // This can be nullptr, **IF**
                                                // purse is password-protected.
                                                // (It's
        // just ignored in that case.) Otherwise MUST contain the
        // NymID for the Purse owner (necessary to decrypt the token.)
        const String* pstrDisplay = nullptr) const;

    EXPORT Purse* Purse_Pop(
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID,
        const String& THE_PURSE,
        const Identifier* pOWNER_ID = nullptr,  // This can be nullptr, **IF**
                                                // purse
                                                // is
        // password-protected. (It's just
        // ignored in that case.) Otherwise MUST
        // contain the NymID for the Purse owner
        // (necessary to decrypt the token.)
        const String* pstrDisplay = nullptr) const;

    EXPORT Purse* Purse_Empty(
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID,
        const String& THE_PURSE,
        const String* pstrDisplay = nullptr) const;

    EXPORT Purse* Purse_Push(
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID,
        const String& THE_PURSE,
        const String& THE_TOKEN,
        const Identifier* pOWNER_ID = nullptr,  // This can be nullptr, **IF**
                                                // purse is password-protected.
                                                // (It's
        // just ignored in that case.) Otherwise MUST contain the
        // NymID for the Purse owner (recipient. necessary to encrypt
        // the token to him.)
        const String* pstrDisplay = nullptr) const;

    EXPORT Token* Token_ChangeOwner(
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID,
        const String& THE_TOKEN,
        const Identifier& SIGNER_NYM_ID,
        const String& OLD_OWNER,  // Pass a NymID here, or a purse.
        const String& NEW_OWNER,  // Pass a NymID here, or a purse.
        const String* pstrDisplay = nullptr) const;
    EXPORT Mint* LoadMint(
        const Identifier& NOTARY_ID,
        const Identifier& INSTRUMENT_DEFINITION_ID) const;
#endif  // OT_CASH
    EXPORT bool IsBasketCurrency(
        const Identifier& BASKET_INSTRUMENT_DEFINITION_ID) const;

    EXPORT std::int64_t GetBasketMinimumTransferAmount(
        const Identifier& BASKET_INSTRUMENT_DEFINITION_ID) const;

    EXPORT std::int32_t GetBasketMemberCount(
        const Identifier& BASKET_INSTRUMENT_DEFINITION_ID) const;

    EXPORT bool GetBasketMemberType(
        const Identifier& BASKET_INSTRUMENT_DEFINITION_ID,
        std::int32_t nIndex,
        Identifier& theOutputMemberType) const;

    EXPORT std::int64_t GetBasketMemberMinimumTransferAmount(
        const Identifier& BASKET_INSTRUMENT_DEFINITION_ID,
        std::int32_t nIndex) const;
    EXPORT std::shared_ptr<Account> LoadAssetAccount(
        const Identifier& NOTARY_ID,
        const Identifier& NYM_ID,
        const Identifier& ACCOUNT_ID) const;
    EXPORT Ledger* LoadNymbox(
        const Identifier& NOTARY_ID,
        const Identifier& NYM_ID) const;

    EXPORT Ledger* LoadNymboxNoVerify(
        const Identifier& NOTARY_ID,
        const Identifier& NYM_ID) const;

    EXPORT Ledger* LoadInbox(
        const Identifier& NOTARY_ID,
        const Identifier& NYM_ID,
        const Identifier& ACCOUNT_ID) const;

    EXPORT Ledger* LoadInboxNoVerify(
        const Identifier& NOTARY_ID,
        const Identifier& NYM_ID,
        const Identifier& ACCOUNT_ID) const;

    EXPORT Ledger* LoadOutbox(
        const Identifier& NOTARY_ID,
        const Identifier& NYM_ID,
        const Identifier& ACCOUNT_ID) const;

    EXPORT Ledger* LoadOutboxNoVerify(
        const Identifier& NOTARY_ID,
        const Identifier& NYM_ID,
        const Identifier& ACCOUNT_ID) const;
    EXPORT Ledger* LoadPaymentInbox(
        const Identifier& NOTARY_ID,
        const Identifier& NYM_ID) const;

    EXPORT Ledger* LoadPaymentInboxNoVerify(
        const Identifier& NOTARY_ID,
        const Identifier& NYM_ID) const;
    // LoadRecordBox
    // Note: depending on the record type, the Account ID may contain the User
    // ID.
    EXPORT Ledger* LoadRecordBox(
        const Identifier& NOTARY_ID,
        const Identifier& NYM_ID,
        const Identifier& ACCOUNT_ID) const;

    EXPORT Ledger* LoadRecordBoxNoVerify(
        const Identifier& NOTARY_ID,
        const Identifier& NYM_ID,
        const Identifier& ACCOUNT_ID) const;

    EXPORT bool ClearRecord(
        const Identifier& NOTARY_ID,
        const Identifier& NYM_ID,
        const Identifier& ACCOUNT_ID,  // NYM_ID can be passed here as well.
        std::int32_t nIndex,
        bool bClearAll = false  // if true, nIndex is ignored.
        ) const;
    EXPORT Ledger* LoadExpiredBox(
        const Identifier& NOTARY_ID,
        const Identifier& NYM_ID) const;

    EXPORT Ledger* LoadExpiredBoxNoVerify(
        const Identifier& NOTARY_ID,
        const Identifier& NYM_ID) const;

    EXPORT bool ClearExpired(
        const Identifier& NOTARY_ID,
        const Identifier& NYM_ID,
        std::int32_t nIndex,
        bool bClearAll = false  // if true, nIndex is
                                // ignored.
        ) const;

    EXPORT std::int32_t Ledger_GetCount(const Ledger& theLedger) const;
    EXPORT std::set<std::int64_t> Ledger_GetTransactionNums(
        const Ledger& theLedger) const;

    EXPORT ProcessInbox Ledger_CreateResponse(
        const Identifier& theNotaryID,
        const Identifier& theNymID,
        const Identifier& theAccountID) const;

    EXPORT bool Transaction_CreateResponse(
        const Identifier& theNotaryID,
        const Identifier& theNymID,
        const Identifier& theAcctID,
        Ledger& responseLedger,
        OTTransaction& originalTransaction,  // Responding to
        const bool& BOOL_DO_I_ACCEPT) const;

    EXPORT bool Ledger_FinalizeResponse(
        const Identifier& theNotaryID,
        const Identifier& theNymID,
        const Identifier& theAcctID,
        Ledger& responseLedger) const;

    EXPORT OTTransaction* Ledger_GetTransactionByIndex(
        Ledger& theLedger,
        const std::int32_t& nIndex) const;

    EXPORT OTTransaction* Ledger_GetTransactionByID(
        Ledger& theLedger,
        const std::int64_t& TRANSACTION_NUMBER) const;

    EXPORT std::unique_ptr<OTPayment> Ledger_GetInstrument(
        const Identifier& theNymID,
        const Ledger& theLedger,
        const std::int32_t& nIndex) const;
    // The functions immediately above and blow this comment
    // have good reason for having their parameters in a different order.
    EXPORT std::unique_ptr<OTPayment> Ledger_GetInstrumentByReceiptID(
        const Ledger& theLedger,
        const Identifier& theNymID,
        const std::int64_t& lReceiptId) const;

    EXPORT std::unique_ptr<OTPayment> Ledger_GetInstrumentByReceiptID(
        const Identifier& theNymID,
        const Ledger& theLedger,
        const std::int64_t& lReceiptId) const;

    EXPORT std::int64_t Ledger_GetTransactionIDByIndex(
        const Ledger& theLedger,
        const std::int32_t& nIndex) const;

    EXPORT bool Ledger_AddTransaction(
        const Identifier& theNymID,
        Ledger& theLedger,  // theLedger takes ownership of pTransaction.
        std::unique_ptr<OTTransaction>& pTransaction) const;

    // These 3 functions are lower level:

    EXPORT ProcessInbox CreateProcessInbox(
        const Identifier& accountID,
        const ServerContext& context) const;

    EXPORT bool IncludeResponse(
        const Identifier& accountID,
        const bool accept,
        ServerContext& context,
        OTTransaction& source,
        Ledger& processInbox) const;

    EXPORT bool FinalizeProcessInbox(
        const Identifier& accountID,
        ServerContext& context,
        Ledger& processInbox,
        Ledger& inbox) const;

    // Note: if instrument is expired BEFORE being recorded, it will go into the
    // expired box instead of the record box.
    EXPORT bool RecordPayment(
        const Identifier& TRANSPORT_NOTARY_ID,
        const Identifier& NYM_ID,
        bool bIsInbox,  // true == payments inbox. false == payments outbox.
        std::int32_t nIndex,  // removes payment instrument (from payments in or
                              // out
                              // box) and moves to record box.
        bool bSaveCopy) const;  // If false, copy of instrument will NOT be
                                // saved.
    // So the client side knows which ones he has in storage, vs which ones he
    // still needs to download.
    EXPORT bool DoesBoxReceiptExist(
        const Identifier& NOTARY_ID,
        const Identifier& NYM_ID,      // Unused here for now, but still
                                       // convention.
        const Identifier& ACCOUNT_ID,  // If for Nymbox (vs inbox/outbox) then
                                       // pass NYM_ID in this field also.
        std::int32_t nBoxType,         // 0/nymbox, 1/inbox, 2/outbox
        const TransactionNumber& lTransactionNum) const;
    // Outgoing
    EXPORT Message* GetSentMessage(
        const std::int64_t& lRequestNumber,
        const Identifier& NOTARY_ID,
        const Identifier& NYM_ID) const;
    EXPORT bool RemoveSentMessage(
        const std::int64_t& lRequestNumber,
        const Identifier& NOTARY_ID,
        const Identifier& NYM_ID) const;
    EXPORT void FlushSentMessages(
        bool bHarvestingForRetry,
        const Identifier& NOTARY_ID,
        const Identifier& NYM_ID,
        const Ledger& THE_NYMBOX) const;

    EXPORT bool HaveAlreadySeenReply(
        const Identifier& NOTARY_ID,
        const Identifier& NYM_ID,
        const RequestNumber& lRequestNumber) const;

    EXPORT bool ResyncNymWithServer(
        NymFile& theNym,
        const Ledger& theNymbox,
        const Nym& theMessageNym) const;

    // These commands below send messages to the server:

    EXPORT CommandResult registerNym(ServerContext& context) const;

    EXPORT CommandResult unregisterNym(ServerContext& context) const;

    EXPORT CommandResult
    checkNym(ServerContext& context, const Identifier& targetNymID) const;

    EXPORT CommandResult usageCredits(
        ServerContext& context,
        const Identifier& NYM_ID_CHECK,
        std::int64_t lAdjustment = 0) const;

    EXPORT CommandResult sendNymMessage(
        ServerContext& context,
        const Identifier& recipientNymID,
        const std::string& THE_MESSAGE,
        Identifier& messageID) const;

    EXPORT CommandResult sendNymObject(
        ServerContext& context,
        std::unique_ptr<Message>& request,
        const Identifier& recipientNymID,
        const PeerObject& object,
        const RequestNumber provided) const;

    EXPORT CommandResult registerContract(
        ServerContext& context,
        const ContractType TYPE,
        const Identifier& CONTRACT) const;

    EXPORT CommandResult sendNymInstrument(
        ServerContext& context,
        std::unique_ptr<Message>& request,
        const Identifier& recipientNymID,
        const OTPayment& instrument,
        bool storeOutpayment,
        const OTPayment* senderCopy = nullptr) const;

    EXPORT CommandResult registerInstrumentDefinition(
        ServerContext& context,
        const proto::UnitDefinition& THE_CONTRACT) const;

    EXPORT CommandResult getInstrumentDefinition(
        ServerContext& context,
        const Identifier& INSTRUMENT_DEFINITION_ID) const;

    EXPORT CommandResult getMint(
        ServerContext& context,
        const Identifier& INSTRUMENT_DEFINITION_ID) const;

    EXPORT CommandResult getBoxReceipt(
        ServerContext& context,
        const Identifier& ACCOUNT_ID,  // If for Nymbox (vs
                                       // inbox/outbox) then pass
                                       // NYM_ID in this field also.
        std::int32_t nBoxType,         // 0/nymbox, 1/inbox, 2/outbox
        const TransactionNumber& lTransactionNum) const;

    EXPORT CommandResult queryInstrumentDefinitions(
        ServerContext& context,
        const OTASCIIArmor& ENCODED_MAP) const;

    EXPORT CommandResult registerAccount(
        ServerContext& context,
        const Identifier& INSTRUMENT_DEFINITION_ID) const;

    EXPORT CommandResult deleteAssetAccount(
        ServerContext& context,
        const Identifier& ACCOUNT_ID) const;

    EXPORT CommandResult
    getAccountData(ServerContext& context, const Identifier& ACCT_ID) const;

    EXPORT bool AddBasketCreationItem(
        proto::UnitDefinition& basketTemplate,
        const String& currencyID,
        const std::uint64_t weight) const;

    EXPORT CommandResult issueBasket(
        ServerContext& context,
        const proto::UnitDefinition& basket) const;

    EXPORT Basket* GenerateBasketExchange(
        const Identifier& NOTARY_ID,
        const Identifier& NYM_ID,
        const Identifier& BASKET_INSTRUMENT_DEFINITION_ID,
        const Identifier& BASKET_ASSET_ACCT_ID,
        std::int32_t TRANSFER_MULTIPLE) const;  // 1            2             3
    // 5=2,3,4  OR  10=4,6,8  OR 15=6,9,12

    EXPORT bool AddBasketExchangeItem(
        const Identifier& NOTARY_ID,
        const Identifier& NYM_ID,
        Basket& theBasket,
        const Identifier& INSTRUMENT_DEFINITION_ID,
        const Identifier& ASSET_ACCT_ID) const;

    EXPORT CommandResult exchangeBasket(
        ServerContext& context,
        const Identifier& BASKET_INSTRUMENT_DEFINITION_ID,
        const String& BASKET_INFO,
        bool bExchangeInOrOut) const;

    EXPORT CommandResult getTransactionNumbers(ServerContext& context) const;

#if OT_CASH
    EXPORT CommandResult notarizeWithdrawal(
        ServerContext& context,
        const Identifier& ACCT_ID,
        const Amount amount) const;

    EXPORT CommandResult notarizeDeposit(
        ServerContext& context,
        const Identifier& ACCT_ID,
        const String& THE_PURSE) const;
#endif  // OT_CASH

    EXPORT CommandResult notarizeTransfer(
        ServerContext& context,
        const Identifier& ACCT_FROM,
        const Identifier& ACCT_TO,
        const Amount amount,
        const String& NOTE) const;

    EXPORT CommandResult getNymbox(ServerContext& context) const;

    EXPORT CommandResult processNymbox(ServerContext& context) const;

    EXPORT CommandResult processInbox(
        ServerContext& context,
        const Identifier& ACCT_ID,
        const String& ACCT_LEDGER) const;

    EXPORT CommandResult withdrawVoucher(
        ServerContext& context,
        const Identifier& ACCT_ID,
        const Identifier& RECIPIENT_NYM_ID,
        const String& CHEQUE_MEMO,
        const Amount amount) const;

    EXPORT CommandResult payDividend(
        ServerContext& context,
        const Identifier& DIVIDEND_FROM_ACCT_ID,  // if dollars paid for pepsi
                                                  // shares, then this is the
                                                  // issuer's dollars account.
        const Identifier& SHARES_INSTRUMENT_DEFINITION_ID,  // if dollars paid
                                                            // for pepsi
                                                            // shares,
        // then this is the pepsi shares
        // instrument definition id.
        const String& DIVIDEND_MEMO,  // user-configurable note that's added to
                                      // the
                                      // payout request message.
        const Amount& AMOUNT_PER_SHARE) const;  // number of dollars to be paid
                                                // out
    // PER SHARE (multiplied by total
    // number of shares issued.)

    EXPORT CommandResult depositCheque(
        ServerContext& context,
        const Identifier& ACCT_ID,
        const Cheque& cheque) const;

    EXPORT CommandResult triggerClause(
        ServerContext& context,
        const TransactionNumber& lTransactionNum,
        const String& strClauseName,
        const String* pStrParam = nullptr) const;

    EXPORT bool Create_SmartContract(
        const Identifier& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                          // signing at this point is only to
                                          // cause a save.)
        time64_t VALID_FROM,              // Default (0 or nullptr) == NOW
        time64_t VALID_TO,     // Default (0 or nullptr) == no expiry / cancel
                               // anytime
        bool SPECIFY_ASSETS,   // This means asset type IDs must be provided for
                               // every named account.
        bool SPECIFY_PARTIES,  // This means Nym IDs must be provided for every
                               // party.
        String& strOutput) const;

    EXPORT bool SmartContract_SetDates(
        const String& THE_CONTRACT,  // The contract, about to have the dates
                                     // changed on it.
        const Identifier& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                          // signing at this point is only to
                                          // cause a save.)
        time64_t VALID_FROM,              // Default (0 or nullptr) == NOW
        time64_t VALID_TO,  // Default (0 or nullptr) == no expiry / cancel
                            // anytime.
        String& strOutput) const;

    EXPORT bool Smart_ArePartiesSpecified(const String& THE_CONTRACT) const;

    EXPORT bool Smart_AreAssetTypesSpecified(const String& THE_CONTRACT) const;

    EXPORT bool SmartContract_AddBylaw(
        const String& THE_CONTRACT,  // The contract, about to have the bylaw
                                     // added to it.
        const Identifier& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                          // signing at this point is only to
                                          // cause a save.)
        const String& BYLAW_NAME,  // The Bylaw's NAME as referenced in the
                                   // smart contract. (And the scripts...)
        String& strOutput) const;

    EXPORT bool SmartContract_RemoveBylaw(
        const String& THE_CONTRACT,  // The contract, about to have the bylaw
                                     // removed from it.
        const Identifier& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                          // signing at this point is only to
                                          // cause a save.)
        const String& BYLAW_NAME,  // The Bylaw's NAME as referenced in the
                                   // smart contract. (And the scripts...)
        String& strOutput) const;

    EXPORT bool SmartContract_AddClause(
        const String& THE_CONTRACT,  // The contract, about to have the clause
                                     // added to it.
        const Identifier& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                          // signing at this point is only to
                                          // cause a save.)
        const String& BYLAW_NAME,   // Should already be on the contract. (This
                                    // way we can find it.)
        const String& CLAUSE_NAME,  // The Clause's name as referenced in the
                                    // smart contract. (And the scripts...)
        const String& SOURCE_CODE,  // The actual source code for the clause.
        String& strOutput) const;

    EXPORT bool SmartContract_UpdateClause(
        const String& THE_CONTRACT,  // The contract, about to have the clause
                                     // updated on it.
        const Identifier& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                          // signing at this point is only to
                                          // cause a save.)
        const String& BYLAW_NAME,   // Should already be on the contract. (This
                                    // way we can find it.)
        const String& CLAUSE_NAME,  // The Clause's name as referenced in the
                                    // smart contract. (And the scripts...)
        const String& SOURCE_CODE,  // The actual source code for the clause.
        String& strOutput) const;

    EXPORT bool SmartContract_RemoveClause(
        const String& THE_CONTRACT,  // The contract, about to have the clause
                                     // removed from it.
        const Identifier& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                          // signing at this point is only to
                                          // cause a save.)
        const String& BYLAW_NAME,   // Should already be on the contract. (This
                                    // way we can find it.)
        const String& CLAUSE_NAME,  // The Clause's name as referenced in the
                                    // smart contract. (And the scripts...)
        String& strOutput) const;

    EXPORT bool SmartContract_AddVariable(
        const String& THE_CONTRACT,  // The contract, about to have the variable
                                     // added to it.
        const Identifier& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                          // signing at this point is only to
                                          // cause a save.)
        const String& BYLAW_NAME,  // Should already be on the contract. (This
                                   // way we can find it.)
        const String& VAR_NAME,    // The Variable's name as referenced in the
                                   // smart contract. (And the scripts...)
        const String& VAR_ACCESS,  // "constant", "persistent", or "important".
        const String& VAR_TYPE,    // "string", "std::int64_t", or "bool"
        const String& VAR_VALUE,  // Contains a string. If type is std::int64_t,
                                  // atol() will be used to convert value to a
                                  // std::int64_t. If type is bool, the strings
                                  // "true" or "false" are expected here in
                                  // order to convert to a bool.
        String& strOutput) const;

    EXPORT bool SmartContract_RemoveVariable(
        const String& THE_CONTRACT,  // The contract, about to have the variable
                                     // removed from it.
        const Identifier& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                          // signing at this point is only to
                                          // cause a save.)
        const String& BYLAW_NAME,  // Should already be on the contract. (This
                                   // way we can find it.)
        const String& VAR_NAME,    // The Variable's name as referenced in the
                                   // smart contract. (And the scripts...)
        String& strOutput) const;

    EXPORT bool SmartContract_AddCallback(
        const String& THE_CONTRACT,  // The contract, about to have the callback
                                     // added to it.
        const Identifier& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                          // signing at this point is only to
                                          // cause a save.)
        const String& BYLAW_NAME,  // Should already be on the contract. (This
                                   // way we can find it.)
        const String& CALLBACK_NAME,  // The Callback's name as referenced in
                                      // the smart contract. (And the
                                      // scripts...)
        const String& CLAUSE_NAME,  // The actual clause that will be triggered
                                    // by the callback. (Must exist.)
        String& strOutput) const;

    EXPORT bool SmartContract_RemoveCallback(
        const String& THE_CONTRACT,  // The contract, about to have the callback
                                     // removed from it.
        const Identifier& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                          // signing at this point is only to
                                          // cause a save.)
        const String& BYLAW_NAME,  // Should already be on the contract. (This
                                   // way we can find it.)
        const String& CALLBACK_NAME,  // The Callback's name as referenced in
                                      // the smart contract. (And the
                                      // scripts...)
        String& strOutput) const;

    EXPORT bool SmartContract_AddHook(
        const String& THE_CONTRACT,  // The contract, about to have the hook
                                     // added to it.
        const Identifier& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                          // signing at this point is only to
                                          // cause a save.)
        const String& BYLAW_NAME,  // Should already be on the contract. (This
                                   // way we can find it.)
        const String& HOOK_NAME,   // The Hook's name as referenced in the smart
                                   // contract. (And the scripts...)
        const String& CLAUSE_NAME,  // The actual clause that will be triggered
                                    // by the hook. (You can call this multiple
                                    // times, and have multiple clauses trigger
                                    // on the same hook.)
        String& strOutput) const;

    EXPORT bool SmartContract_RemoveHook(
        const String& THE_CONTRACT,  // The contract, about to have the hook
                                     // removed from it.
        const Identifier& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                          // signing at this point is only to
                                          // cause a save.)
        const String& BYLAW_NAME,  // Should already be on the contract. (This
                                   // way we can find it.)
        const String& HOOK_NAME,   // The Hook's name as referenced in the smart
                                   // contract. (And the scripts...)
        const String& CLAUSE_NAME,  // The actual clause that will be triggered
                                    // by the hook. (You can call this multiple
                                    // times, and have multiple clauses trigger
                                    // on the same hook.)
        String& strOutput) const;

    EXPORT bool SmartContract_AddParty(
        const String& THE_CONTRACT,  // The contract, about to have the party
                                     // added to it.
        const Identifier& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                          // signing at this point is only to
                                          // cause a save.)
        const String& PARTY_NYM_ID,  // Optional. Some smart contracts require
                                     // the party's Nym to be specified in
                                     // advance.
        const String& PARTY_NAME,    // The Party's NAME as referenced in the
                                     // smart contract. (And the scripts...)
        const String& AGENT_NAME,    // An AGENT will be added by default for
                                     // this party. Need Agent NAME.
        String& strOutput) const;

    EXPORT bool SmartContract_RemoveParty(
        const String& THE_CONTRACT,  // The contract, about to have the party
                                     // removed from it.
        const Identifier& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                          // signing at this point is only to
                                          // cause a save.)
        const String& PARTY_NAME,  // The Party's NAME as referenced in the
                                   // smart contract. (And the scripts...)
        String& strOutput) const;

    EXPORT bool SmartContract_AddAccount(
        const String& THE_CONTRACT,  // The contract, about to have the account
                                     // added to it.
        const Identifier& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                          // signing at this point is only to
                                          // cause a save.)
        const String& PARTY_NAME,  // The Party's NAME as referenced in the
                                   // smart contract. (And the scripts...)
        const String& ACCT_NAME,   // The Account's name as referenced in the
                                   // smart contract
        const String& INSTRUMENT_DEFINITION_ID,  // Instrument Definition ID for
                                                 // the
                                                 // Account.
        String& strOutput) const;

    EXPORT bool SmartContract_RemoveAccount(
        const String& THE_CONTRACT,  // The contract, about to have the account
                                     // removed from it.
        const Identifier& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                          // signing at this point is only to
                                          // cause a save.)
        const String& PARTY_NAME,  // The Party's NAME as referenced in the
                                   // smart contract. (And the scripts...)
        const String& ACCT_NAME,   // The Account's name as referenced in the
                                   // smart contract
        String& strOutput) const;

    EXPORT std::int32_t SmartContract_CountNumsNeeded(
        const String& THE_CONTRACT,  // The contract, about to have the bylaw
                                     // added to it.
        const String& AGENT_NAME) const;  // An AGENT will be added by default
                                          // for
                                          // this party. Need Agent NAME.

    EXPORT bool SmartContract_ConfirmAccount(
        const String& THE_CONTRACT,
        const Identifier& SIGNER_NYM_ID,
        const String& PARTY_NAME,
        const String& ACCT_NAME,
        const String& AGENT_NAME,
        const String& ACCT_ID,
        String& strOutput) const;

    EXPORT bool SmartContract_ConfirmParty(
        const String& THE_CONTRACT,  // The smart contract, about to be changed
                                     // by this function.
        const String& PARTY_NAME,    // Should already be on the contract. This
                                     // way we can find it.
        const Identifier& NYM_ID,    // Nym ID for the party, the actual owner,
        const Identifier& NOTARY_ID,
        String& strOutput) const;  // ===> AS WELL AS for the default AGENT of
                                   // that
                                   // party. (For now, until I code entities)
    EXPORT bool Msg_HarvestTransactionNumbers(
        const Message& theMsg,
        const Identifier& NYM_ID,
        bool bHarvestingForRetry,
        bool bReplyWasSuccess,
        bool bReplyWasFailure,
        bool bTransactionWasSuccess,
        bool bTransactionWasFailure) const;

    EXPORT bool HarvestClosingNumbers(
        const Identifier& NOTARY_ID,
        const Identifier& NYM_ID,
        const String& THE_CRON_ITEM) const;

    EXPORT bool HarvestAllNumbers(
        const Identifier& NOTARY_ID,
        const Identifier& NYM_ID,
        const String& THE_CRON_ITEM) const;

    EXPORT CommandResult activateSmartContract(
        ServerContext& context,
        const String& THE_SMART_CONTRACT) const;

    EXPORT CommandResult depositPaymentPlan(
        ServerContext& context,
        const String& THE_PAYMENT_PLAN) const;
    EXPORT CommandResult issueMarketOffer(
        ServerContext& context,
        const Identifier& ASSET_ACCT_ID,
        const Identifier& CURRENCY_ACCT_ID,
        const std::int64_t& MARKET_SCALE,  // Defaults to minimum of 1. Market
                                           // granularity.
        const std::int64_t& MINIMUM_INCREMENT,  // This will be multiplied by
                                                // the
                                                // Scale. Min 1.
        const std::int64_t& TOTAL_ASSETS_ON_OFFER,  // Total assets available
                                                    // for
                                                    // sale
        // or purchase. Will be multiplied
        // by minimum increment.
        const Amount PRICE_LIMIT,  // Per Minimum Increment...
        bool bBuyingOrSelling,     // BUYING == false, SELLING == true.
        time64_t tLifespanInSeconds = OT_TIME_DAY_IN_SECONDS,  // 86400 seconds
                                                               // == 1 day.
        char STOP_SIGN = 0,  // For stop orders, set to '<' or '>'
        const Amount ACTIVATION_PRICE = 0) const;  // For stop orders, set the
                                                   // threshold price here.
    EXPORT CommandResult getMarketList(ServerContext& context) const;
    EXPORT CommandResult getMarketOffers(
        ServerContext& context,
        const Identifier& MARKET_ID,
        const std::int64_t& lDepth) const;
    EXPORT CommandResult getMarketRecentTrades(
        ServerContext& context,
        const Identifier& MARKET_ID) const;

    EXPORT CommandResult getNymMarketOffers(ServerContext& context) const;

    // For cancelling market offers and payment plans.
    EXPORT CommandResult cancelCronItem(
        ServerContext& context,
        const Identifier& ASSET_ACCT_ID,
        const TransactionNumber& lTransactionNum) const;

    EXPORT CommandResult initiatePeerRequest(
        ServerContext& context,
        const Identifier& recipient,
        const std::shared_ptr<PeerRequest>& request) const;

    EXPORT CommandResult initiatePeerReply(
        ServerContext& context,
        const Identifier& recipient,
        const Identifier& request,
        const std::shared_ptr<PeerReply>& reply) const;

    EXPORT CommandResult
    requestAdmin(ServerContext& context, const std::string& PASSWORD) const;

    EXPORT CommandResult serverAddClaim(
        ServerContext& context,
        const std::string& section,
        const std::string& type,
        const std::string& value,
        const bool primary) const;

    EXPORT ConnectionState CheckConnection(const std::string& server) const;

    EXPORT std::string AddChildKeyCredential(
        const Identifier& nymID,
        const Identifier& masterID,
        const NymParameters& nymParameters) const;

    EXPORT std::unique_ptr<proto::ContactData> GetContactData(
        const Identifier& nymID) const;

    EXPORT std::list<std::string> BoxItemCount(
        const Identifier& NYM_ID,
        const StorageBox box) const;
    EXPORT std::string BoxContents(
        const Identifier& NYM_ID,
        const Identifier& nIndex,
        const StorageBox box) const;

    EXPORT ~OT_API();  // calls Cleanup();

private:
    friend class api::implementation::Api;

    class Pid;

    const api::Activity& activity_;
    const api::Settings& config_;
    const api::ContactManager& contacts_;
    const api::Crypto& crypto_;
    const api::Identity& identity_;
    const api::storage::Storage& storage_;
    const api::client::Wallet& wallet_;
    const api::client::Workflow& workflow_;
    const api::network::ZMQ& zeromq_;

    bool m_bDefaultStore{false};

    String m_strDataPath;
    mutable String m_strWalletFilename;
    String m_strWalletFilePath;
    String m_strConfigFilename;
    String m_strConfigFilePath;

    std::unique_ptr<Pid> pid_;
    OTWallet* m_pWallet{nullptr};
    std::unique_ptr<OTClient> m_pClient;

    ContextLockCallback lock_callback_;

    bool add_accept_item(
        const Item::itemType type,
        const TransactionNumber originNumber,
        const TransactionNumber referenceNumber,
        const String& note,
        const Nym& nym,
        const Amount amount,
        OTTransaction& processInbox) const;
    bool find_cron(
        const ServerContext& context,
        const Item& item,
        OTTransaction& processInbox,
        OTTransaction& serverTransaction,
        Ledger& inbox,
        Amount& amount,
        std::set<TransactionNumber>& closing) const;
    bool find_standard(
        const ServerContext& context,
        const Item& item,
        const TransactionNumber number,
        OTTransaction& serverTransaction,
        Ledger& inbox,
        Amount& amount,
        std::set<TransactionNumber>& closing) const;
    OTTransaction* get_or_create_process_inbox(
        const Identifier& accountID,
        ServerContext& context,
        Ledger& response) const;
    TransactionNumber get_origin(
        const Identifier& notaryID,
        const OTTransaction& source,
        String& note) const;
    Item::itemType response_type(
        const OTTransaction::transactionType sourceType,
        const bool success) const;
    NetworkReplyMessage send_message(
        const std::set<ServerContext::ManagedNumber>& pending,
        ServerContext& context,
        const Message& message) const;

    std::set<std::unique_ptr<Cheque>> extract_cheques(
        const Identifier& nymID,
        const Identifier& accountID,
        const Identifier& serverID,
        const String& serializedProcessInbox,
        Ledger& inbox) const;

    bool Cleanup();
    bool Init();
    bool LoadConfigFile();

    OT_API(
        const api::Activity& activity,
        const api::Settings& config,
        const api::ContactManager& contacts,
        const api::Crypto& crypto,
        const api::Identity& identity,
        const api::storage::Storage& storage,
        const api::client::Wallet& wallet,
        const api::client::Workflow& workflow,
        const api::network::ZMQ& zmq,
        const ContextLockCallback& lockCallback);
    OT_API() = delete;
    OT_API(const OT_API&) = delete;
    OT_API(OT_API&&) = delete;
    OT_API operator=(const OT_API&) = delete;
    OT_API operator=(OT_API&&) = delete;
};
}  // namespace opentxs

#endif  // OPENTXS_CLIENT_OPENTRANSACTIONS_HPP
