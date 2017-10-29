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

#ifndef OPENTXS_CORE_OTLEDGER_HPP
#define OPENTXS_CORE_OTLEDGER_HPP

#include "opentxs/core/Contract.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/OTTransactionType.hpp"

#include <cstdint>
#include <map>
#include <set>

namespace opentxs
{

class Account;
class Cheque;
class Identifier;
class Item;
class Nym;
class ServerContext;
class String;

// transaction ID is a int64_t, assigned by the server. Each transaction has
// one.
// FIRST the server issues the ID. THEN we create the blank transaction object
// with the
// ID in it and store it in our inbox. THEN if we want to send a transaction, we
// use
// the blank to do so. If there is no blank available, we message the server and
// request one.

typedef std::map<int64_t, OTTransaction*> mapOfTransactions;

// the "inbox" and "outbox" functionality is implemented in this class
class Ledger : public OTTransactionType
{
private:  // Private prevents erroneous use by other classes.
    typedef OTTransactionType ot_super;

    friend OTTransactionType* OTTransactionType::TransactionFactory(
        String strInput);

private:
    mapOfTransactions m_mapTransactions;  // a ledger contains a map of
                                          // transactions.

protected:
    // return -1 if error, 0 if nothing, and 1 if the node was processed.
    int32_t ProcessXMLNode(irr::io::IrrXMLReader*& xml) override;
    void UpdateContents() override;  // Before transmission or serialization,
                                     // this is where the ledger saves its
                                     // contents

    Ledger();  // Hopefully stays here.

public:
    enum ledgerType {
        nymbox,  // the nymbox is per user account (versus per asset account)
                 // and is used to receive new transaction numbers (and
                 // messages.)
        inbox,   // each asset account has an inbox, with pending transfers as
                 // well as receipts inside.
        outbox,  // if you SEND a pending transfer, it sits in your outbox until
                 // it's accepted, rejected, or canceled.
        message,  // used in OTMessages, to send various lists of transactions
                  // back and forth.
        paymentInbox,  // Used for client-side-only storage of incoming cheques,
                       // invoices, payment plan requests, etc. (Coming in from
                       // the Nymbox.)
        recordBox,  // Used for client-side-only storage of completed items from
                    // the inbox, and the paymentInbox.
        expiredBox,  // Used for client-side-only storage of expired items from
                     // the paymentInbox.
        error_state
    };  // If you add any types to this list, update the list of strings at the
        // top of the .CPP file.
    ledgerType m_Type{error_state};

    bool m_bLoadedLegacyData{
        false};  // So the server can tell if it just loaded a
                 // legacy box or a hashed box. (Legacy boxes
                 // stored ALL of the receipts IN the box. No
                 // more.)

protected:
    bool LoadGeneric(ledgerType theType, const String* pString = nullptr);
    bool SaveGeneric(ledgerType theType);

public:
    inline ledgerType GetType() const { return m_Type; }

    EXPORT bool LoadedLegacyData() const { return m_bLoadedLegacyData; }

    // This function assumes that this is an INBOX.
    // If you don't use an INBOX to call this method, then it will return
    // nullptr immediately. If you DO use an inbox, then it will create a
    // balanceStatement item to go onto your transaction.  (Transactions require
    // balance statements. And when you get the atBalanceStatement reply from
    // the server, KEEP THAT RECEIPT. Well, OT will do that for you.) You only
    // have to keep the latest receipt, unlike systems that don't store balance
    // agreement.  We also store a list of issued transactions, the new balance,
    // and the outbox hash.
    EXPORT Item* GenerateBalanceStatement(
        std::int64_t lAdjustment,
        const OTTransaction& theOwner,
        const ServerContext& context,
        const Account& theAccount,
        Ledger& theOutbox) const;
    EXPORT Item* GenerateBalanceStatement(
        std::int64_t lAdjustment,
        const OTTransaction& theOwner,
        const ServerContext& context,
        const Account& theAccount,
        Ledger& theOutbox,
        const std::set<TransactionNumber>& without) const;

    EXPORT void ProduceOutboxReport(Item& theBalanceItem);

    EXPORT bool AddTransaction(OTTransaction& theTransaction);
    EXPORT bool RemoveTransaction(
        int64_t lTransactionNum,
        bool bDeleteIt = true);  // if false,
                                 // transaction wasn't
                                 // found.

    EXPORT std::set<int64_t> GetTransactionNums(
        const std::set<int32_t>* pOnlyForIndices = nullptr) const;

    EXPORT OTTransaction* GetTransaction(
        OTTransaction::transactionType theType);
    EXPORT OTTransaction* GetTransaction(int64_t lTransactionNum) const;
    EXPORT OTTransaction* GetTransactionByIndex(int32_t nIndex) const;
    EXPORT OTTransaction* GetFinalReceipt(int64_t lReferenceNum);
    EXPORT OTTransaction* GetTransferReceipt(int64_t lNumberOfOrigin);
    EXPORT OTTransaction* GetChequeReceipt(
        int64_t lChequeNum,
        Cheque** ppChequeOut = nullptr);  // CALLER RESPONSIBLE
                                          // TO DELETE.
    EXPORT int32_t GetTransactionIndex(int64_t lTransactionNum);  // if not
                                                                  // found,
                                                                  // returns -1
    EXPORT OTTransaction* GetReplyNotice(const int64_t& lRequestNum);

    // This calls OTTransactionType::VerifyAccount(), which calls
    // VerifyContractID() as well as VerifySignature().
    //
    // But first, this OTLedger version also loads the box receipts,
    // if doing so is appropriate. (message ledger == not appropriate.)
    //
    // Use this method instead of Contract::VerifyContract, which
    // expects/uses a pubkey from inside the contract in order to verify
    // it.
    //
    EXPORT bool VerifyAccount(const Nym& theNym) override;
    // For ALL abbreviated transactions, load the actual box receipt for each.
    EXPORT bool LoadBoxReceipts(
        std::set<int64_t>* psetUnloaded = nullptr);  // if psetUnloaded passed
                                                     // in, then use it to
                                                     // return the #s that
                                                     // weren't there.
    EXPORT bool SaveBoxReceipts();  // For all "full version" transactions, save
                                    // the actual box receipt for each.
    // Verifies the abbreviated form exists first, and then loads the
    // full version and compares the two. Returns success / fail.
    //
    EXPORT bool LoadBoxReceipt(const int64_t& lTransactionNum);
    // Saves the Box Receipt separately.
    EXPORT bool SaveBoxReceipt(const int64_t& lTransactionNum);
    // "Deletes" it by adding MARKED_FOR_DELETION to the bottom of the file.
    EXPORT bool DeleteBoxReceipt(const int64_t& lTransactionNum);
    EXPORT bool LoadInbox();
    EXPORT bool SaveInbox(Identifier* pInboxHash = nullptr);  // If you pass
                                                              // the
                                                              // identifier in,
                                                              // the hash is
                                                              // recorded there
    EXPORT bool LoadNymbox();
    EXPORT bool SaveNymbox(Identifier* pNymboxHash = nullptr);  // If you pass
                                                                // the
    // identifier in,
    // the hash is
    // recorded there.
    EXPORT bool LoadOutbox();
    EXPORT bool SaveOutbox(Identifier* pOutboxHash = nullptr);  // If you pass
                                                                // the
    // identifier in,
    // the hash is
    // recorded there

    EXPORT bool CalculateHash(Identifier& theOutput);
    EXPORT bool CalculateInboxHash(Identifier& theOutput);
    EXPORT bool CalculateOutboxHash(Identifier& theOutput);
    EXPORT bool CalculateNymboxHash(Identifier& theOutput);
    EXPORT bool SavePaymentInbox();
    EXPORT bool LoadPaymentInbox();

    EXPORT bool SaveRecordBox();
    EXPORT bool LoadRecordBox();

    EXPORT bool SaveExpiredBox();
    EXPORT bool LoadExpiredBox();
    EXPORT bool LoadLedgerFromString(const String& theStr);  // Auto-detects
                                                             // ledger type.
    // (message/nymbox/inbox/outbox)
    EXPORT bool LoadInboxFromString(const String& strBox);
    EXPORT bool LoadOutboxFromString(const String& strBox);
    EXPORT bool LoadNymboxFromString(const String& strBox);
    EXPORT bool LoadPaymentInboxFromString(const String& strBox);
    EXPORT bool LoadRecordBoxFromString(const String& strBox);
    EXPORT bool LoadExpiredBoxFromString(const String& strBox);
    // inline for the top one only.
    inline int32_t GetTransactionCount() const
    {
        return static_cast<int32_t>(m_mapTransactions.size());
    }
    EXPORT int32_t GetTransactionCountInRefTo(int64_t lReferenceNum) const;
    EXPORT int64_t GetTotalPendingValue();  // for inbox only, allows you to
                                            // lookup the total value of pending
                                            // transfers within.
    EXPORT const mapOfTransactions& GetTransactionMap() const;
    EXPORT Ledger(
        const Identifier& theNymID,
        const Identifier& theAccountID,
        const Identifier& theNotaryID);
    EXPORT virtual ~Ledger();

    EXPORT void Release() override;
    EXPORT void Release_Ledger();

    EXPORT void ReleaseTransactions();
    // ONLY call this if you need to load a ledger where you don't already know
    // the person's NymID
    // For example, if you need to load someone ELSE's inbox in order to send
    // them a transfer, then
    // you only know their account number, not their user ID. So you call this
    // function to get it
    // loaded up, and the NymID will hopefully be loaded up with the rest of
    // it.
    EXPORT Ledger(
        const Identifier& theAccountID,
        const Identifier& theNotaryID);
    EXPORT void InitLedger();
    EXPORT static Ledger* GenerateLedger(
        const Identifier& theNymID,
        const Identifier& theAcctID,
        const Identifier& theNotaryID,
        ledgerType theType,
        bool bCreateFile = false);

    EXPORT bool GenerateLedger(
        const Identifier& theAcctID,
        const Identifier& theNotaryID,
        ledgerType theType,
        bool bCreateFile = false);

    EXPORT static char const* _GetTypeString(ledgerType theType);
    EXPORT char const* GetTypeString() const { return _GetTypeString(m_Type); }
};

}  // namespace opentxs

#endif  // OPENTXS_CORE_OTLEDGER_HPP
