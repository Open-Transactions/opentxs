// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/Ledger.hpp"

#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/consensus/TransactionStatement.hpp"
#include "opentxs/core/transaction/Helpers.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/OTFolders.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Item.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/NumList.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/StringXML.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/OTTransactionType.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/Types.hpp"

#include <stdlib.h>
#include <sys/types.h>
#include <cstdint>
#include <irrxml/irrXML.hpp>
#include <memory>
#include <ostream>
#include <set>
#include <string>
#include <utility>

#define OT_METHOD "opentxs::Ledger::"

namespace opentxs
{
char const* const __TypeStringsLedger[] = {
    "nymbox",  // the nymbox is per user account (versus per asset account) and
               // is used to receive new transaction numbers (and messages.)
    "inbox",  // each asset account has an inbox, with pending transfers as well
              // as receipts inside.
    "outbox",   // if you SEND a pending transfer, it sits in your outbox until
                // it's accepted, rejected, or canceled.
    "message",  // used in OTMessages, to send various lists of transactions
                // back
                // and forth.
    "paymentInbox",  // Used for client-side-only storage of incoming cheques,
    // invoices, payment plan requests, etc. (Coming in from the
    // Nymbox.)
    "recordBox",   // Used for client-side-only storage of completed items from
                   // the inbox, and the paymentInbox.
    "expiredBox",  // Used for client-side-only storage of expired items from
                   // the
                   // paymentInbox.
    "error_state"};

// ID refers to account ID.
// Since a ledger is normally used as an inbox for a specific account, in a
// specific file, then I've decided to restrict ledgers to a single account.
Ledger::Ledger(
    const api::Core& core,
    const Identifier& theNymID,
    const Identifier& theAccountID,
    const Identifier& theNotaryID)
    : OTTransactionType(core, theNymID, theAccountID, theNotaryID)
    , m_Type(ledgerType::message)
    , m_bLoadedLegacyData(false)
{
    InitLedger();
}

// ONLY call this if you need to load a ledger where you don't already know the
// person's NymID For example, if you need to load someone ELSE's inbox in
// order to send them a transfer, then you only know their account number, not
// their user ID. So you call this function to get it loaded up, and the NymID
// will hopefully be loaded up with the rest of it.
Ledger::Ledger(
    const api::Core& core,
    const Identifier& theAccountID,
    const Identifier& theNotaryID)
    : OTTransactionType(core)
    , m_Type(ledgerType::message)
    , m_bLoadedLegacyData(false)
{
    InitLedger();

    SetRealAccountID(theAccountID);
    SetRealNotaryID(theNotaryID);
}

// This is private now and hopefully will stay that way.
Ledger::Ledger(const api::Core& core)
    : OTTransactionType(core)
    , m_Type(ledgerType::message)
    , m_bLoadedLegacyData(false)
{
    InitLedger();
}

char const* Ledger::_GetTypeString(ledgerType theType)
{
    std::int32_t nType = static_cast<std::int32_t>(theType);
    return __TypeStringsLedger[nType];
}

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
bool Ledger::VerifyAccount(const Nym& theNym)
{
    switch (GetType()) {
        case ledgerType::message:  // message ledgers do not load Box Receipts.
                                   // (They
                                   // store full version internally already.)
            break;
        case ledgerType::nymbox:
        case ledgerType::inbox:
        case ledgerType::outbox:
        case ledgerType::paymentInbox:
        case ledgerType::recordBox:
        case ledgerType::expiredBox: {
            std::set<std::int64_t> setUnloaded;
            // if psetUnloaded passed in, then use it to return the #s that
            // weren't there as box receipts.
            //          bool bLoadedBoxReceipts =
            LoadBoxReceipts(&setUnloaded);  // Note: Also useful for suppressing
                                            // errors here.
        } break;
        default: {
            const std::int32_t nLedgerType =
                static_cast<std::int32_t>(GetType());
            const auto theNymID = Identifier::Factory(theNym);
            const auto strNymID = String::Factory(theNymID);
            auto strAccountID = String::Factory();
            GetIdentifier(strAccountID);
            otErr << "OTLedger::VerifyAccount: Failure: Bad ledger type: "
                  << nLedgerType << ", NymID: " << strNymID
                  << ", AcctID: " << strAccountID << "\n";
        }
            return false;
    }

    return OTTransactionType::VerifyAccount(theNym);
}

// This makes sure that ALL transactions inside the ledger are saved as box
// receipts
// in their full (not abbreviated) form (as separate files.)
//
bool Ledger::SaveBoxReceipts()  // For ALL full transactions, save the actual
                                // box receipt for each to its own place.
{
    bool bRetVal = true;
    for (auto& it : m_mapTransactions) {
        auto pTransaction = it.second;
        OT_ASSERT(false != bool(pTransaction));

        // We only save full versions of transactions as box receipts, not
        // abbreviated versions.
        // (If it's not abbreviated, therefore it's the full version.)
        //
        if (!pTransaction->IsAbbreviated())  // This way we won't see an
                                             // error if it's not
                                             // abbreviated.
            bRetVal = pTransaction->SaveBoxReceipt(*this);

        if (!bRetVal) {
            otErr << "OTLedger::SaveBoxReceipts: Failed calling SaveBoxReceipt "
                     "on transaction: "
                  << pTransaction->GetTransactionNum() << ".\n";
            break;
        }
    }
    return bRetVal;
}

bool Ledger::SaveBoxReceipt(const std::int64_t& lTransactionNum)
{

    // First, see if the transaction itself exists on this ledger.
    // Get a pointer to it.
    auto pTransaction = GetTransaction(lTransactionNum);

    if (false == bool(pTransaction)) {
        otOut << "OTLedger::SaveBoxReceipt: Unable to save box receipt "
              << lTransactionNum
              << ": "
                 "couldn't find the transaction on this ledger.\n";
        return false;
    }

    return pTransaction->SaveBoxReceipt(*this);
}

bool Ledger::DeleteBoxReceipt(const std::int64_t& lTransactionNum)
{

    // First, see if the transaction itself exists on this ledger.
    // Get a pointer to it.
    auto pTransaction = GetTransaction(lTransactionNum);

    if (false == bool(pTransaction)) {
        otOut << "OTLedger::DeleteBoxReceipt: Unable to delete (overwrite) box "
                 "receipt "
              << lTransactionNum
              << ": couldn't find the transaction on this ledger.\n";
        return false;
    }

    return pTransaction->DeleteBoxReceipt(*this);
}

// This makes sure that ALL transactions inside the ledger are loaded in their
// full (not abbreviated) form.
//
// For ALL abbreviated transactions, load the actual box receipt for each.
//
// For all failures to load the box receipt, if a set pointer was passed in,
// then add that transaction# to the set. (psetUnloaded)

// if psetUnloaded passed in, then use it to return the #s that weren't there.
bool Ledger::LoadBoxReceipts(std::set<std::int64_t>* psetUnloaded)
{
    // Grab a copy of all the transaction #s stored inside this ledger.
    //
    std::set<std::int64_t> the_set;

    for (auto& it : m_mapTransactions) {
        const auto pTransaction = it.second;
        OT_ASSERT(false != bool(pTransaction));
        the_set.insert(pTransaction->GetTransactionNum());
    }

    // Now iterate through those numbers and for each, load the box receipt.
    //
    bool bRetVal = true;

    for (auto& it : the_set) {
        std::int64_t lSetNum = it;

        const auto pTransaction = GetTransaction(lSetNum);
        OT_ASSERT(false != bool(pTransaction));

        // Failed loading the boxReceipt
        //
        if ((true == pTransaction->IsAbbreviated()) &&
            (false == LoadBoxReceipt(lSetNum))) {
            // WARNING: pTransaction must be re-Get'd below this point if
            // needed, since pointer
            // is bad if success on LoadBoxReceipt() call.
            //
            bRetVal = false;
            OTLogStream* pLog = &otOut;

            if (nullptr != psetUnloaded) {
                psetUnloaded->insert(lSetNum);
                pLog = &otLog3;
            }
            *pLog << "OTLedger::LoadBoxReceipts: Failed calling LoadBoxReceipt "
                     "on "
                     "abbreviated transaction number:"
                  << lSetNum << ".\n";
            // If psetUnloaded is passed in, then we don't want to break,
            // because we want to
            // populate it with the conmplete list of IDs that wouldn't load as
            // a Box Receipt.
            // Thus, we only break if psetUnloaded is nullptr, which is better
            // optimization in that case.
            // (If not building a list of all failures, then we can return at
            // first sign of failure.)
            //
            if (nullptr == psetUnloaded) break;
        }
        // else (success), no need for a block in that case.
    }

    // You might ask, why didn't I just iterate through the transactions
    // directly and just call
    // LoadBoxReceipt on each one? Answer: Because that function actually
    // deletes the transaction
    // and replaces it with a different object, if successful.

    return bRetVal;
}

/*
 While the box itself is stored at (for example) "nymbox/NOTARY_ID/NYM_ID"
 the box receipts for that box may be stored at: "nymbox/NOTARY_ID/NYM_ID.r"
 With a specific receipt denoted by transaction:
 "nymbox/NOTARY_ID/NYM_ID.r/TRANSACTION_ID.rct"
 */

bool Ledger::LoadBoxReceipt(const std::int64_t& lTransactionNum)
{
    // First, see if the transaction itself exists on this ledger.
    // Get a pointer to it.
    // Next, see if the appropriate file exists, and load it up from
    // local storage, into a string.
    // Finally, try to load the transaction from that string and see if
    // successful.
    // If it verifies, then replace the abbreviated receipt with the actual one.

    // First, see if the transaction itself exists on this ledger.
    // Get a pointer to it.
    //
    auto pTransaction = GetTransaction(lTransactionNum);

    if (false == bool(pTransaction)) {
        otOut
            << __FUNCTION__ << ": Unable to load box receipt "
            << lTransactionNum
            << ": couldn't find abbreviated version already on this ledger.\n";
        return false;
    }
    // Todo: security analysis. By this point we've verified the hash of the
    // transaction against the stored
    // hash inside the abbreviated version. (VerifyBoxReceipt) We've also
    // verified a few other values like transaction
    // number, and the "in ref to" display number. We're then assuming based on
    // those, that the adjustment and display
    // amount are correct. (The hash is actually a zero knowledge proof of this
    // already.) This is good for speedier
    // optimization but may be worth revisiting in case any security holes.
    // UPDATE: We'll save this for optimization needs in the future.
    //  pBoxReceipt->SetAbbrevAdjustment( pTransaction->GetAbbrevAdjustment() );
    //  pBoxReceipt->SetAbbrevDisplayAmount(
    // pTransaction->GetAbbrevDisplayAmount() );

    //  otOut << "DEBUGGING:  OTLedger::LoadBoxReceipt: ledger type: %s \n",
    // GetTypeString());

    // LoadBoxReceipt already checks pTransaction to see if it's
    // abbreviated
    // (which it must be.) So I don't bother checking twice.
    //
    auto pBoxReceipt = ::opentxs::LoadBoxReceipt(*pTransaction, *this);

    // success
    if (false != bool(pBoxReceipt)) {
        // Remove the existing, abbreviated receipt, and replace it with
        // the actual receipt.
        // (If this inbox/outbox/whatever is saved, it will later save in
        // abbreviated form again.)
        //
        RemoveTransaction(lTransactionNum);  // this deletes pTransaction
        std::shared_ptr<OTTransaction> receipt{pBoxReceipt.release()};
        AddTransaction(receipt);

        return true;
    }

    return false;
}

std::set<std::int64_t> Ledger::GetTransactionNums(
    const std::set<std::int32_t>* pOnlyForIndices /*=nullptr*/) const
{
    std::set<std::int64_t> the_set{};

    std::int32_t current_index{-1};

    for (const auto& it : m_mapTransactions) {
        ++current_index;  // 0 on first iteration.
        const auto pTransaction = it.second;
        OT_ASSERT(false != bool(pTransaction));
        const std::int64_t lTransNum = pTransaction->GetTransactionNum();
        if (nullptr == pOnlyForIndices) {
            the_set.insert(lTransNum);
            continue;
        }
        // -------------------------------
        std::set<std::int32_t>::const_iterator it_indices =
            pOnlyForIndices->find(current_index);
        if (pOnlyForIndices->end() != it_indices) { the_set.insert(lTransNum); }
    }
    return the_set;
}

// the below four functions (load/save in/outbox) assume that the ID
// is already set properly.
// Then it uses the ID to form the path for the file that is opened.
// Easy, right?

bool Ledger::LoadInbox()
{
    //  print_stacktrace();

    bool bRetVal = LoadGeneric(ledgerType::inbox);

    return bRetVal;
}

// TODO really should verify the NotaryID after loading the ledger.
// Perhaps just call "VerifyContract" and we'll make sure, for ledgers
// VerifyContract is overriden and explicitly checks the notaryID.
// Should also check the Type at the same time.

bool Ledger::LoadOutbox() { return LoadGeneric(ledgerType::outbox); }

bool Ledger::LoadNymbox() { return LoadGeneric(ledgerType::nymbox); }

bool Ledger::LoadInboxFromString(const String& strBox)
{
    return LoadGeneric(ledgerType::inbox, strBox);
}

bool Ledger::LoadOutboxFromString(const String& strBox)
{
    return LoadGeneric(ledgerType::outbox, strBox);
}

bool Ledger::LoadNymboxFromString(const String& strBox)
{
    return LoadGeneric(ledgerType::nymbox, strBox);
}

bool Ledger::LoadPaymentInbox()
{
    return LoadGeneric(ledgerType::paymentInbox);
}

bool Ledger::LoadRecordBox() { return LoadGeneric(ledgerType::recordBox); }

bool Ledger::LoadExpiredBox() { return LoadGeneric(ledgerType::expiredBox); }

bool Ledger::LoadPaymentInboxFromString(const String& strBox)
{
    return LoadGeneric(ledgerType::paymentInbox, strBox);
}

bool Ledger::LoadRecordBoxFromString(const String& strBox)
{
    return LoadGeneric(ledgerType::recordBox, strBox);
}

bool Ledger::LoadExpiredBoxFromString(const String& strBox)
{
    return LoadGeneric(ledgerType::expiredBox, strBox);
}

/**
  OTLedger::LoadGeneric is called by LoadInbox, LoadOutbox, and LoadNymbox.
  Does NOT VerifyAccount after loading -- caller is responsible to do that.

  pString -- optional argument, for when  you prefer to load from a string
  instead of from a file.
 */
bool Ledger::LoadGeneric(ledgerType theType, const String& pString)
{
    m_Type = theType;

    const char* pszType = GetTypeString();
    const char* pszFolder = nullptr;

    switch (theType) {
        case ledgerType::nymbox:
            pszFolder = OTFolders::Nymbox().Get();
            break;
        case ledgerType::inbox:
            pszFolder = OTFolders::Inbox().Get();
            break;
        case ledgerType::outbox:
            pszFolder = OTFolders::Outbox().Get();
            break;
        case ledgerType::paymentInbox:
            pszFolder = OTFolders::PaymentInbox().Get();
            break;
        case ledgerType::recordBox:
            pszFolder = OTFolders::RecordBox().Get();
            break;
        case ledgerType::expiredBox:
            pszFolder = OTFolders::ExpiredBox().Get();
            break;
        /* --- BREAK --- */
        default:
            otErr << "OTLedger::LoadGeneric: Error: unknown box type. (This "
                     "should "
                     "never happen.)\n";
            return false;
    }
    m_strFoldername = String::Factory(pszFolder);

    auto strID = String::Factory();
    GetIdentifier(strID);

    const auto strNotaryID = String::Factory(GetRealNotaryID());

    if (!m_strFilename->Exists())
        m_strFilename->Format(
            "%s%s%s", strNotaryID->Get(), Log::PathSeparator(), strID->Get());

    auto strFilename = String::Factory();
    strFilename = String::Factory(strID->Get());

    const char* szFolder1name =
        m_strFoldername->Get();  // "nymbox" (or "inbox" or "outbox")
    const char* szFolder2name = strNotaryID->Get();  // "nymbox/NOTARY_ID"
    const char* szFilename = strFilename->Get();  // "nymbox/NOTARY_ID/NYM_ID"
    // (or "inbox/NOTARY_ID/ACCT_ID"
    // or
    // "outbox/NOTARY_ID/ACCT_ID")

    auto strRawFile = String::Factory();

    if (pString.Exists())  // Loading FROM A STRING.
        strRawFile->Set(pString.Get());
    else  // Loading FROM A FILE.
    {
        if (!OTDB::Exists(
                api_.DataFolder(),
                szFolder1name,
                szFolder2name,
                szFilename,
                "")) {
            LogDebug(OT_METHOD)(__FUNCTION__)(
                ": does not exist in OTLedger::Load")(pszType)(": ")(
                szFolder1name)(Log::PathSeparator())(szFolder2name)(
                Log::PathSeparator())(szFilename)
                .Flush();
            return false;
        }

        // Try to load the ledger from local storage.
        //
        std::string strFileContents(OTDB::QueryPlainString(
            api_.DataFolder(),
            szFolder1name,
            szFolder2name,
            szFilename,
            ""));  // <=== LOADING
                   // FROM DATA STORE.

        if (strFileContents.length() < 2) {
            otErr << "OTLedger::LoadGeneric: Error reading file: "
                  << szFolder1name << Log::PathSeparator() << szFolder2name
                  << Log::PathSeparator() << szFilename << "\n";
            return false;
        }

        strRawFile->Set(strFileContents.c_str());
    }
    // NOTE: No need to deal with OT ARMORED INBOX file format here, since
    //       LoadContractFromString already handles that automatically.

    if (!strRawFile->Exists()) {
        otErr << "OTLedger::LoadGeneric: Unable to load box (" << szFolder1name
              << Log::PathSeparator() << szFolder2name << Log::PathSeparator()
              << szFilename << ") from empty string.\n";
        return false;
    }

    bool bSuccess = LoadContractFromString(strRawFile);

    if (!bSuccess) {
        otErr << "Failed loading " << pszType << " "
              << ((pString.Exists()) ? "from string" : "from file")
              << " in OTLedger::Load" << pszType << ": " << szFolder1name
              << Log::PathSeparator() << szFolder2name << Log::PathSeparator()
              << szFilename << "\n";
        return false;
    } else {
        LogVerbose(OT_METHOD)(__FUNCTION__)("Successfully loaded ")(pszType)(
            " ")((pString.Exists()) ? "from string" : "from file")(
            " in OTLedger::Load")(pszType)(": ")(szFolder1name)(
            Log::PathSeparator())(szFolder2name)(Log::PathSeparator())(
            szFilename)
            .Flush();
    }

    return bSuccess;
}

bool Ledger::SaveGeneric(ledgerType theType)
{
    m_Type = theType;

    const char* pszFolder = nullptr;
    const char* pszType = GetTypeString();

    switch (theType) {
        case ledgerType::nymbox:
            pszFolder = OTFolders::Nymbox().Get();
            break;
        case ledgerType::inbox:
            pszFolder = OTFolders::Inbox().Get();
            break;
        case ledgerType::outbox:
            pszFolder = OTFolders::Outbox().Get();
            break;
        case ledgerType::paymentInbox:
            pszFolder = OTFolders::PaymentInbox().Get();
            break;
        case ledgerType::recordBox:
            pszFolder = OTFolders::RecordBox().Get();
            break;
        case ledgerType::expiredBox:
            pszFolder = OTFolders::ExpiredBox().Get();
            break;
        /* --- BREAK --- */
        default:
            otErr << "OTLedger::SaveGeneric: Error: unknown box type. (This "
                     "should "
                     "never happen.)\n";
            return false;
    }

    m_strFoldername = String::Factory(pszFolder);  // <=======

    auto strID = String::Factory();
    GetIdentifier(strID);
    const auto strNotaryID = String::Factory(GetRealNotaryID());

    if (!m_strFilename->Exists())
        m_strFilename->Format(
            "%s%s%s", strNotaryID->Get(), Log::PathSeparator(), strID->Get());

    auto strFilename = String::Factory();
    strFilename = String::Factory(strID->Get());

    const char* szFolder1name =
        m_strFoldername->Get();  // "nymbox" (or "inbox" or "outbox")
    const char* szFolder2name = strNotaryID->Get();  // "nymbox/NOTARY_ID"
    const char* szFilename = strFilename->Get();  // "nymbox/NOTARY_ID/NYM_ID"
    // (or "inbox/NOTARY_ID/ACCT_ID"
    // or
    // "outbox/NOTARY_ID/ACCT_ID")

    OT_ASSERT(m_strFoldername->GetLength() > 2);
    OT_ASSERT(m_strFilename->GetLength() > 2);

    auto strRawFile = String::Factory();

    if (!SaveContractRaw(strRawFile)) {
        otErr << "OTLedger::SaveGeneric: Error saving " << pszType
              << " (to string):\n"
              << szFolder1name << Log::PathSeparator() << szFolder2name
              << Log::PathSeparator() << szFilename << "\n";
        return false;
    }

    auto strFinal = String::Factory();
    auto ascTemp = Armored::Factory(strRawFile);

    if (false ==
        ascTemp->WriteArmoredString(strFinal, m_strContractType->Get())) {
        otErr << "OTLedger::SaveGeneric: Error saving " << pszType
              << " (failed writing armored string):\n"
              << szFolder1name << Log::PathSeparator() << szFolder2name
              << Log::PathSeparator() << szFilename << "\n";
        return false;
    }

    bool bSaved = OTDB::StorePlainString(
        strFinal->Get(),
        api_.DataFolder(),
        szFolder1name,
        szFolder2name,
        szFilename,
        "");  // <=== SAVING TO DATA STORE.
    if (!bSaved) {
        otErr << "OTLedger::SaveGeneric: Error writing " << pszType
              << " to file: " << szFolder1name << Log::PathSeparator()
              << szFolder2name << Log::PathSeparator() << szFilename << "\n";
        return false;
    } else
        LogVerbose(OT_METHOD)(__FUNCTION__)("Successfully saved ")(pszType)(
            ": ")(szFolder1name)(Log::PathSeparator())(szFolder2name)(
            Log::PathSeparator())(szFilename)
            .Flush();

    return bSaved;
}

// If you know you have an inbox, outbox, or nymbox, then call
// CalculateInboxHash,
// CalculateOutboxHash, or CalculateNymboxHash. Otherwise, if in doubt, call
// this.
// It's more generic but warning: performs less verification.
//
bool Ledger::CalculateHash(Identifier& theOutput) const
{
    theOutput.Release();

    bool bCalcDigest = theOutput.CalculateDigest(m_xmlUnsigned);

    if (false == bCalcDigest) {
        theOutput.Release();
        otErr << OT_METHOD << __FUNCTION__
              << ": Failed trying to calculate hash (for a " << GetTypeString()
              << ")" << std::endl;
    }

    return bCalcDigest;
}

bool Ledger::CalculateInboxHash(Identifier& theOutput) const
{
    if (m_Type != ledgerType::inbox) {
        otErr << OT_METHOD << __FUNCTION__ << ": Wrong type." << std::endl;

        return false;
    }

    return CalculateHash(theOutput);
}

bool Ledger::CalculateOutboxHash(Identifier& theOutput) const
{
    if (m_Type != ledgerType::outbox) {
        otErr << OT_METHOD << __FUNCTION__ << ": Wrong type." << std::endl;

        return false;
    }

    return CalculateHash(theOutput);
}

bool Ledger::CalculateNymboxHash(Identifier& theOutput) const
{
    if (m_Type != ledgerType::nymbox) {
        otErr << OT_METHOD << __FUNCTION__ << ": Wrong type." << std::endl;

        return false;
    }

    return CalculateHash(theOutput);
}

bool Ledger::save_box(
    const ledgerType type,
    Identifier& hash,
    bool (Ledger::*calc)(Identifier&) const)
{
    OT_ASSERT(nullptr != calc)

    if (m_Type != type) {
        otErr << OT_METHOD << __FUNCTION__ << ": Wrong type." << std::endl;

        return false;
    }

    const bool saved = SaveGeneric(m_Type);

    if (saved) {
        hash.Release();

        if (false == (this->*calc)(hash)) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Failed trying to calculate box hash." << std::endl;
        }
    }

    return saved;
}

// If you're going to save this, make sure you sign it first.
bool Ledger::SaveNymbox()
{
    auto hash = Identifier::Factory();

    return SaveNymbox(hash);
}

// If you're going to save this, make sure you sign it first.
bool Ledger::SaveNymbox(Identifier& hash)
{
    return save_box(ledgerType::nymbox, hash, &Ledger::CalculateNymboxHash);
}

// If you're going to save this, make sure you sign it first.
bool Ledger::SaveInbox()
{
    auto hash = Identifier::Factory();

    return SaveInbox(hash);
}

// If you're going to save this, make sure you sign it first.
bool Ledger::SaveInbox(Identifier& hash)
{
    return save_box(ledgerType::inbox, hash, &Ledger::CalculateInboxHash);
}

// If you're going to save this, make sure you sign it first.
bool Ledger::SaveOutbox()
{
    auto hash = Identifier::Factory();

    return SaveOutbox(hash);
}

// If you're going to save this, make sure you sign it first.
bool Ledger::SaveOutbox(Identifier& hash)
{
    return save_box(ledgerType::outbox, hash, &Ledger::CalculateOutboxHash);
}

// If you're going to save this, make sure you sign it first.
bool Ledger::SavePaymentInbox()
{
    if (m_Type != ledgerType::paymentInbox) {
        otErr << "Wrong ledger type passed to OTLedger::SavePaymentInbox.\n";
        return false;
    }

    return SaveGeneric(m_Type);
}

// If you're going to save this, make sure you sign it first.
bool Ledger::SaveRecordBox()
{
    if (m_Type != ledgerType::recordBox) {
        otErr << "Wrong ledger type passed to OTLedger::SaveRecordBox.\n";
        return false;
    }

    return SaveGeneric(m_Type);
}

// If you're going to save this, make sure you sign it first.
bool Ledger::SaveExpiredBox()
{
    if (m_Type != ledgerType::expiredBox) {
        otErr << "Wrong ledger type passed to OTLedger::SaveExpiredBox.\n";
        return false;
    }

    return SaveGeneric(m_Type);
}

bool Ledger::generate_ledger(
    const Identifier& theNymID,
    const Identifier& theAcctID,
    const Identifier& theNotaryID,
    ledgerType theType,
    bool bCreateFile)
{
    // First we set the "Safe" ID and try to load the file, to make sure it
    // doesn't already exist.
    auto strID = String::Factory(theAcctID),
         strNotaryID = String::Factory(theNotaryID);

    switch (theType) {
        case ledgerType::nymbox:  // stored by NymID ONLY.
            m_strFoldername = String::Factory(OTFolders::Nymbox().Get());
            m_strFilename->Format(
                "%s%s%s",
                strNotaryID->Get(),
                Log::PathSeparator(),
                strID->Get());
            break;
        case ledgerType::inbox:  // stored by AcctID ONLY.
            m_strFoldername = String::Factory(OTFolders::Inbox().Get());
            m_strFilename->Format(
                "%s%s%s",
                strNotaryID->Get(),
                Log::PathSeparator(),
                strID->Get());
            break;
        case ledgerType::outbox:  // stored by AcctID ONLY.
            m_strFoldername = String::Factory(OTFolders::Outbox().Get());
            m_strFilename->Format(
                "%s%s%s",
                strNotaryID->Get(),
                Log::PathSeparator(),
                strID->Get());
            break;
        case ledgerType::paymentInbox:  // stored by NymID ONLY.
            m_strFoldername = String::Factory(OTFolders::PaymentInbox().Get());
            m_strFilename->Format(
                "%s%s%s",
                strNotaryID->Get(),
                Log::PathSeparator(),
                strID->Get());
            break;
        case ledgerType::recordBox:  // stored by Acct ID *and* Nym ID
                                     // (depending on the box.)
            m_strFoldername = String::Factory(OTFolders::RecordBox().Get());
            m_strFilename->Format(
                "%s%s%s",
                strNotaryID->Get(),
                Log::PathSeparator(),
                strID->Get());
            break;
        case ledgerType::expiredBox:  // stored by Nym ID only.
            m_strFoldername = String::Factory(OTFolders::ExpiredBox().Get());
            m_strFilename->Format(
                "%s%s%s",
                strNotaryID->Get(),
                Log::PathSeparator(),
                strID->Get());
            break;
        case ledgerType::message:
            LogTrace(OT_METHOD)(__FUNCTION__)("Generating message ledger...")
                .Flush();
            SetRealAccountID(theAcctID);
            SetPurportedAccountID(theAcctID);  // It's safe to set these the
                                               // same here, since we're
                                               // creating the ledger now.
            SetRealNotaryID(theNotaryID);
            SetPurportedNotaryID(theNotaryID);  // Always want the server ID on
            // anything that the server signs.
            m_Type = theType;
            return true;
        default:
            OT_FAIL_MSG("OTLedger::GenerateLedger: GenerateLedger is only for "
                        "message, nymbox, inbox, outbox, and paymentInbox "
                        "ledgers.\n");
    }

    m_Type = theType;  // Todo make this Get/Set methods

    SetRealAccountID(theAcctID);  // set this before calling LoadContract... (In
                                  // this case, will just be the Nym ID as
                                  // well...)
    SetRealNotaryID(theNotaryID);  // (Ledgers/transactions/items were
                                   // originally meant just for account-related
                                   // functions.)

    if (bCreateFile) {

        auto strFilename = String::Factory();
        strFilename = String::Factory(strID->Get());

        const char* szFolder1name =
            m_strFoldername->Get();  // "nymbox" (or "inbox" or "outbox")
        const char* szFolder2name = strNotaryID->Get();  // "nymbox/NOTARY_ID"
        const char* szFilename =
            strFilename->Get();  // "nymbox/NOTARY_ID/NYM_ID"  (or
                                 // "inbox/NOTARY_ID/ACCT_ID" or
                                 // "outbox/NOTARY_ID/ACCT_ID")

        if (OTDB::Exists(
                api_.DataFolder(),
                szFolder1name,
                szFolder2name,
                szFilename,
                "")) {
            otOut << "ERROR: trying to generate ledger that already exists: "
                  << szFolder1name << Log::PathSeparator() << szFolder2name
                  << Log::PathSeparator() << szFilename << "\n";
            return false;
        }

        // Okay, it doesn't already exist. Let's generate it.
        otOut << "Generating " << szFolder1name << Log::PathSeparator()
              << szFolder2name << Log::PathSeparator() << szFilename << "\n";
    }

    SetNymID(theNymID);
    SetPurportedAccountID(theAcctID);
    SetPurportedNotaryID(theNotaryID);

    // Notice I still don't actually create the file here.  The programmer still
    // has to call "SaveNymbox", "SaveInbox" or "SaveOutbox" or "SaveRecordBox"
    // or "SavePaymentInbox" to actually save the file. But he cannot do that
    // unless he generates it first here, and the "bCreateFile" parameter
    // insures that he isn't overwriting one that is already there (even if we
    // don't actually save the file in this function.)

    return true;
}

bool Ledger::GenerateLedger(
    const Identifier& theAcctID,
    const Identifier& theNotaryID,
    ledgerType theType,
    bool bCreateFile)
{
    auto nymID = Identifier::Factory();

    if ((ledgerType::inbox == theType) || (ledgerType::outbox == theType)) {
        // Have to look up the NymID here. No way around it. We need that ID.
        // Plus it helps verify things.
        auto account = api_.Wallet().Account(theAcctID);

        if (account) {
            nymID = Identifier::Factory(account.get().GetNymID());
        } else {
            otErr << "OTLedger::GenerateLedger: Failed in "
                     "OTAccount::LoadExistingAccount().\n";
            return false;
        }
    } else if (ledgerType::recordBox == theType) {
        // RecordBox COULD be by NymID OR AcctID. So we TRY to lookup the acct.
        auto account = api_.Wallet().Account(theAcctID);

        if (account) {
            nymID = Identifier::Factory(account.get().GetNymID());
        } else {
            // Must be based on NymID, not AcctID (like Nymbox. But RecordBox
            // can go either way.)
            nymID = Identifier::Factory(theAcctID);
            // In the case of nymbox, and sometimes with recordBox, the acct ID
            // IS the user ID.
        }
    } else {
        // In the case of paymentInbox, expired box, and nymbox, the acct ID IS
        // the user ID. (Should change it to "owner ID" to make it sound right
        // either way.)
        nymID = Identifier::Factory(theAcctID);
    }

    return generate_ledger(nymID, theAcctID, theNotaryID, theType, bCreateFile);
}

bool Ledger::CreateLedger(
    const Identifier& theNymID,
    const Identifier& theAcctID,
    const Identifier& theNotaryID,
    ledgerType theType,
    bool bCreateFile)
{
    return generate_ledger(
        theNymID, theAcctID, theNotaryID, theType, bCreateFile);
}

void Ledger::InitLedger()
{
    m_strContractType = String::Factory(
        "LEDGER");  // CONTRACT, MESSAGE, TRANSACTION, LEDGER, TRANSACTION ITEM

    // This is the default type for a ledger.
    // Inboxes and Outboxes are generated with the right type, with files.
    // Until the GenerateLedger function is called, message is the default type.
    m_Type = ledgerType::message;

    m_bLoadedLegacyData = false;
}

const mapOfTransactions& Ledger::GetTransactionMap() const
{
    return m_mapTransactions;
}

/// If transaction #87, in reference to #74, is in the inbox, you can remove it
/// by calling this function and passing in 87. Deletes.
///
bool Ledger::RemoveTransaction(std::int64_t lTransactionNum)
{
    // See if there's something there with that transaction number.
    auto it = m_mapTransactions.find(lTransactionNum);

    // If it's not already on the list, then there's nothing to remove.
    if (it == m_mapTransactions.end()) {
        otErr << "OTLedger::RemoveTransaction"
              << ": Attempt to remove Transaction from ledger, when "
                 "not already there: "
              << lTransactionNum << "\n";
        return false;
    }
    // Otherwise, if it WAS already there, remove it properly.
    else {
        m_mapTransactions.erase(it);

        return true;
    }
}

bool Ledger::AddTransaction(std::shared_ptr<OTTransaction> theTransaction)
{
    // See if there's something else already there with the same transaction
    // number.
    auto it = m_mapTransactions.find(theTransaction->GetTransactionNum());

    // If it's not already on the list, then add it...
    if (it == m_mapTransactions.end()) {
        m_mapTransactions[theTransaction->GetTransactionNum()] = theTransaction;
        theTransaction->SetParent(*this);  // for convenience
        return true;
    }
    // Otherwise, if it was already there, log an error.
    else {
        otErr << "Attempt to add Transaction to ledger when already there for "
                 "that number: "
              << theTransaction->GetTransactionNum() << "\n";
    }

    return false;
}

// Do NOT delete the return value, it's owned by the ledger.
std::shared_ptr<OTTransaction> Ledger::GetTransaction(transactionType theType)
{
    // loop through the items that make up this transaction

    for (auto& it : m_mapTransactions) {
        auto pTransaction = it.second;
        OT_ASSERT(false != bool(pTransaction));

        if (theType == pTransaction->GetType()) return pTransaction;
    }

    return nullptr;
}

// if not found, returns -1
std::int32_t Ledger::GetTransactionIndex(std::int64_t lTransactionNum)
{
    // loop through the transactions inside this ledger
    // If a specific transaction is found, returns its index inside the ledger
    //
    std::int32_t nIndex = -1;

    for (auto& it : m_mapTransactions) {
        const auto pTransaction = it.second;
        OT_ASSERT(false != bool(pTransaction));

        ++nIndex;  // 0 on first iteration.

        if (pTransaction->GetTransactionNum() == lTransactionNum) {
            return nIndex;
        }
    }
    return -1;
}

// Look up a transaction by transaction number and see if it is in the ledger.
// If it is, return a pointer to it, otherwise return nullptr.
//
// Do NOT delete the return value, it's owned by the ledger.
//
std::shared_ptr<OTTransaction> Ledger::GetTransaction(
    std::int64_t lTransactionNum) const
{
    auto it = m_mapTransactions.find(lTransactionNum);
    if (it != m_mapTransactions.end()) {  // found it
        auto pTransaction = it->second;
        OT_ASSERT(false != bool(pTransaction));
        if (pTransaction->GetTransactionNum() == lTransactionNum) {
            return pTransaction;
        }
        // TODO: Else log error here.
    }
    return nullptr;
}

// Return a count of all the transactions in this ledger that are IN REFERENCE
// TO a specific trans#.
//
// Might want to change this so that it only counts ACCEPTED receipts.
//
std::int32_t Ledger::GetTransactionCountInRefTo(
    std::int64_t lReferenceNum) const
{
    std::int32_t nCount{0};

    for (auto& it : m_mapTransactions) {
        const auto pTransaction = it.second;
        OT_ASSERT(false != bool(pTransaction));

        if (pTransaction->GetReferenceToNum() == lReferenceNum) nCount++;
    }

    return nCount;
}

// Look up a transaction by transaction number and see if it is in the ledger.
// If it is, return a pointer to it, otherwise return nullptr.
//
std::shared_ptr<OTTransaction> Ledger::GetTransactionByIndex(
    std::int32_t nIndex) const
{
    // Out of bounds.
    if ((nIndex < 0) || (nIndex >= GetTransactionCount())) return nullptr;

    std::int32_t nIndexCount = -1;

    for (auto& it : m_mapTransactions) {
        nIndexCount++;  // On first iteration, this is now 0, same as nIndex.
        auto pTransaction = it.second;
        OT_ASSERT(false != bool(pTransaction));  // Should always be good.

        // If this transaction is the one at the requested index
        if (nIndexCount == nIndex) return pTransaction;
    }

    return nullptr;  // Should never reach this point, since bounds are checked
                     // at the top.
}

// Nymbox-only.
// Looks up replyNotice by REQUEST NUMBER.
//
std::shared_ptr<OTTransaction> Ledger::GetReplyNotice(
    const std::int64_t& lRequestNum)
{
    // loop through the transactions that make up this ledger.
    for (auto& it : m_mapTransactions) {
        auto pTransaction = it.second;
        OT_ASSERT(false != bool(pTransaction));

        if (transactionType::replyNotice !=
            pTransaction->GetType())  // <=======
            continue;

        if (pTransaction->GetRequestNum() == lRequestNum) return pTransaction;
    }

    return nullptr;
}

std::shared_ptr<OTTransaction> Ledger::GetTransferReceipt(
    std::int64_t lNumberOfOrigin)
{
    // loop through the transactions that make up this ledger.
    for (auto& it : m_mapTransactions) {
        auto pTransaction = it.second;
        OT_ASSERT(false != bool(pTransaction));

        if (transactionType::transferReceipt == pTransaction->GetType()) {
            auto strReference = String::Factory();
            pTransaction->GetReferenceString(strReference);

            auto pOriginalItem{api_.Factory().Item(
                strReference,
                pTransaction->GetPurportedNotaryID(),
                pTransaction->GetReferenceToNum())};
            OT_ASSERT(false != bool(pOriginalItem));

            if (pOriginalItem->GetType() != itemType::acceptPending) {
                otErr << "OTLedger::" << __FUNCTION__
                      << ": Wrong item type attached to transferReceipt!\n";
                return nullptr;
            } else {
                // Note: the acceptPending USED to be "in reference to" whatever
                // the pending
                // was in reference to. (i.e. the original transfer.) But since
                // the KacTech
                // bug fix (for accepting multiple transfer receipts) the
                // acceptPending is now
                // "in reference to" the pending itself, instead of the original
                // transfer.
                //
                // It used to be that a caller of GetTransferReceipt would pass
                // in the InRefTo
                // expected from the pending in the outbox, and match it to the
                // InRefTo found
                // on the acceptPending (inside the transferReceipt) in the
                // inbox.
                // But this is no longer possible, since the acceptPending is no
                // longer InRefTo
                // whatever the pending is InRefTo.
                //
                // Therefore, in this place, it is now necessary to pass in the
                // NumberOfOrigin,
                // and compare it to the NumberOfOrigin, to find the match.
                //
                if (pOriginalItem->GetNumberOfOrigin() == lNumberOfOrigin)
                    //              if (pOriginalItem->GetReferenceToNum() ==
                    // lTransactionNum)
                    return pTransaction;  // FOUND IT!
            }
        }
    }

    return nullptr;
}

// This method loops through all the receipts in the ledger (inbox usually),
// to see if there's a chequeReceipt for a given cheque. For each cheque
// receipt,
// it will load up the original depositCheque item it references, and then load
// up
// the actual cheque which is attached to that item. At this point it can verify
// whether lChequeNum matches the transaction number on the cheque itself, and
// if
// so, return a pointer to the relevant chequeReceipt.
//
// The caller has the option of passing ppChequeOut if he wants the cheque
// returned
// (if he's going to load it anyway, no sense in loading it twice.) If the
// caller
// elects this option, he needs to delete the cheque when he's done with it.
// (But of course do NOT delete the OTTransaction that's returned, since that is
// owned by the ledger.)
//
std::shared_ptr<OTTransaction> Ledger::GetChequeReceipt(std::int64_t lChequeNum)
{
    for (auto& it : m_mapTransactions) {
        auto pCurrentReceipt = it.second;
        OT_ASSERT(nullptr != pCurrentReceipt);

        if ((pCurrentReceipt->GetType() != transactionType::chequeReceipt) &&
            (pCurrentReceipt->GetType() != transactionType::voucherReceipt))
            continue;

        auto strDepositChequeMsg = String::Factory();
        pCurrentReceipt->GetReferenceString(strDepositChequeMsg);

        auto pOriginalItem{api_.Factory().Item(
            strDepositChequeMsg,
            GetPurportedNotaryID(),
            pCurrentReceipt->GetReferenceToNum())};

        if (false == bool(pOriginalItem)) {
            otErr << __FUNCTION__
                  << ": Expected original depositCheque request item to be "
                     "inside the chequeReceipt "
                     "(but failed to load it...)\n";
        } else if (itemType::depositCheque != pOriginalItem->GetType()) {
            auto strItemType = String::Factory();
            pOriginalItem->GetTypeString(strItemType);
            otErr << __FUNCTION__
                  << ": Expected original depositCheque request item to be "
                     "inside the chequeReceipt, "
                     "but somehow what we found instead was a "
                  << strItemType << "...\n";
        } else {
            // Get the cheque from the Item and load it up into a Cheque object.
            //
            auto strCheque = String::Factory();
            pOriginalItem->GetAttachment(strCheque);

            auto pCheque{api_.Factory().Cheque()};
            OT_ASSERT(false != bool(pCheque));

            if (!((strCheque->GetLength() > 2) &&
                  pCheque->LoadContractFromString(strCheque))) {
                otErr << __FUNCTION__ << ": Error loading cheque from string:\n"
                      << strCheque << "\n";
            }
            // NOTE: Technically we don'T NEED to load up the cheque anymore,
            // since
            // we could just check the NumberOfOrigin, which should already
            // match the
            // transaction number on the cheque.
            // However, even that would have to load up the cheque once, if it
            // wasn't
            // already set, and this function already must RETURN a copy of the
            // cheque
            // (at least optionally), so we might as well just load it up,
            // verify it,
            // and return it. (That's why we are still loading the cheque here
            // instead
            // of checking the number of origin.)
            else {
                // Success loading the cheque.
                // Let's see if it's the right cheque...
                if (pCheque->GetTransactionNum() == lChequeNum) {

                    return pCurrentReceipt;
                }
            }
        }
    }

    return nullptr;
}

// Find the finalReceipt in this Inbox, that has lTransactionNum as its "in
// reference to".
// This is useful for cases where a marketReceipt or paymentReceipt has been
// found,
// yet the transaction # for that receipt isn't on my issued list... it's been
// closed.
// Normally this would be a problem: why is it in my inbox then? Because those
// receipts
// are still valid as long as there is a "FINAL RECEIPT" in the same inbox, that
// references
// the same original transaction that they do.  The below function makes it easy
// to find that
// final receipt, if it exists.
//
std::shared_ptr<OTTransaction> Ledger::GetFinalReceipt(
    std::int64_t lReferenceNum)
{
    // loop through the transactions that make up this ledger.
    for (auto& it : m_mapTransactions) {
        auto pTransaction = it.second;
        OT_ASSERT(false != bool(pTransaction));

        if (transactionType::finalReceipt !=
            pTransaction->GetType())  // <=======
            continue;

        if (pTransaction->GetReferenceToNum() == lReferenceNum)
            return pTransaction;
    }

    return nullptr;
}

/// Only if it is an inbox, a ledger will loop through the transactions
/// and produce the XML output for the report that's necessary during
/// a balance agreement. (Any balance agreement for an account must
/// include the list of transactions the nym has issued for use, as
/// well as a listing of the transactions in the inbox for that account.
/// This function does that last part :)
///
/// returns a new balance statement item containing the inbox report
/// CALLER IS RESPONSIBLE TO DELETE.
std::unique_ptr<Item> Ledger::GenerateBalanceStatement(
    std::int64_t lAdjustment,
    const OTTransaction& theOwner,
    const ServerContext& context,
    const Account& theAccount,
    Ledger& theOutbox) const
{
    return GenerateBalanceStatement(
        lAdjustment,
        theOwner,
        context,
        theAccount,
        theOutbox,
        std::set<TransactionNumber>());
}

std::unique_ptr<Item> Ledger::GenerateBalanceStatement(
    std::int64_t lAdjustment,
    const OTTransaction& theOwner,
    const ServerContext& context,
    const Account& theAccount,
    Ledger& theOutbox,
    const std::set<TransactionNumber>& without) const
{
    std::set<TransactionNumber> removing = without;

    if (ledgerType::inbox != GetType()) {
        otErr << "OTLedger::GenerateBalanceStatement: Wrong ledger type.\n";

        return nullptr;
    }

    if ((theAccount.GetPurportedAccountID() != GetPurportedAccountID()) ||
        (theAccount.GetPurportedNotaryID() != GetPurportedNotaryID()) ||
        (theAccount.GetNymID() != GetNymID())) {
        otErr << "Wrong Account passed in to "
                 "OTLedger::GenerateBalanceStatement.\n";

        return nullptr;
    }

    if ((theOutbox.GetPurportedAccountID() != GetPurportedAccountID()) ||
        (theOutbox.GetPurportedNotaryID() != GetPurportedNotaryID()) ||
        (theOutbox.GetNymID() != GetNymID())) {
        otErr << "Wrong Outbox passed in to "
                 "OTLedger::GenerateBalanceStatement.\n";

        return nullptr;
    }

    if ((context.Nym()->ID() != GetNymID())) {
        otErr << "Wrong Nym passed in to OTLedger::GenerateBalanceStatement.\n";

        return nullptr;
    }

    // theOwner is the withdrawal, or deposit, or whatever, that wants to change
    // the account balance, and thus that needs a new balance agreement signed.
    //
    auto pBalanceItem{api_.Factory().Item(
        theOwner,
        itemType::balanceStatement,
        Identifier::Factory())};  // <=== balanceStatement type, with
                                  // user ID, server ID, account ID,
                                  // transaction ID.

    // The above has an ASSERT, so this this will never actually happen.
    if (false == bool(pBalanceItem)) return nullptr;

    std::string itemType;
    const auto number = theOwner.GetTransactionNum();

    switch (theOwner.GetType()) {
        // These six options will remove the transaction number from the issued
        // list, SUCCESS OR FAIL. Server will expect the number to be missing
        // from the list, in the case of these. Therefore I remove it here in
        // order to generate a proper balance agreement, acceptable to the
        // server.
        case transactionType::processInbox: {
            itemType = "processInbox";
            LogDetail(OT_METHOD)(__FUNCTION__)(": Removing number ")(number)(
                " for ")(itemType)
                .Flush();
            removing.insert(number);
        } break;
        case transactionType::withdrawal: {
            itemType = "withdrawal";
            LogDetail(OT_METHOD)(__FUNCTION__)(": Removing number ")(number)(
                " for ")(itemType)
                .Flush();
            removing.insert(number);
        } break;
        case transactionType::deposit: {
            itemType = "deposit";
            LogDetail(OT_METHOD)(__FUNCTION__)(": Removing number ")(number)(
                " for ")(itemType)
                .Flush();
            removing.insert(number);
        } break;
        case transactionType::cancelCronItem: {
            itemType = "cancelCronItem";
            LogDetail(OT_METHOD)(__FUNCTION__)(": Removing number ")(number)(
                " for ")(itemType)
                .Flush();
            removing.insert(number);
        } break;
        case transactionType::exchangeBasket: {
            itemType = "exchangeBasket";
            LogDetail(OT_METHOD)(__FUNCTION__)(": Removing number ")(number)(
                " for ")(itemType)
                .Flush();
            removing.insert(number);
        } break;
        case transactionType::payDividend: {
            itemType = "payDividend";
            LogDetail(OT_METHOD)(__FUNCTION__)(": Removing number ")(number)(
                " for ")(itemType)
                .Flush();
            removing.insert(number);
        } break;
        case transactionType::transfer:
        case transactionType::marketOffer:
        case transactionType::paymentPlan:
        case transactionType::smartContract: {
            // Nothing removed here since the transaction is still in play.
            // (Assuming success.) If the server replies with rejection for any
            // of these three, then I can remove the transaction number from my
            // list of issued/signed for. But if success, then I am responsible
            // for the transaction number until I sign off on closing it. Since
            // the Balance Statement ANTICIPATES SUCCESS, NOT FAILURE, it
            // assumes the number to be "in play" here, and thus DOES NOT remove
            // it (vs the cases above, which do.)
        } break;
        default: {
            otErr << "OTLedger::" << __FUNCTION__
                  << ": wrong owner transaction type: "
                  << theOwner.GetTypeString() << "\n";
        } break;
    }

    std::set<TransactionNumber> adding;
    auto statement = context.Statement(adding, removing);

    if (!statement) { return nullptr; }

    pBalanceItem->SetAttachment(OTString(*statement));
    std::int64_t lCurrentBalance = theAccount.GetBalance();
    // The new (predicted) balance for after the transaction is complete.
    // (item.GetAmount)
    pBalanceItem->SetAmount(lCurrentBalance + lAdjustment);

    // loop through the INBOX transactions, and produce a sub-item onto
    // pBalanceItem for each, which will be a report on each transaction in this
    // inbox, therefore added to the balance item. (So the balance item contains
    // a complete report on the receipts in this inbox.)

    LogVerbose(OT_METHOD)(__FUNCTION__)(
        "About to loop through the inbox items and produce a report for ")(
        "each one... ")
        .Flush();

    for (auto& it : m_mapTransactions) {
        auto pTransaction = it.second;

        OT_ASSERT(false != bool(pTransaction));

        LogVerbose(OT_METHOD)(__FUNCTION__)("Producing a report... ").Flush();
        // This function adds a receipt sub-item to pBalanceItem, where
        // appropriate for INBOX items.
        pTransaction->ProduceInboxReportItem(*pBalanceItem);
    }

    theOutbox.ProduceOutboxReport(*pBalanceItem);
    pBalanceItem->SignContract(*context.Nym());
    pBalanceItem->SaveContract();

    return pBalanceItem;
}

// for inbox only, allows you to lookup the total value of pending transfers
// within the inbox.
// (And it really loads the items to check the amount, but does all this ONLY
// for pending transfers.)
//
std::int64_t Ledger::GetTotalPendingValue()
{
    std::int64_t lTotalPendingValue = 0;

    if (ledgerType::inbox != GetType()) {
        otErr << "OTLedger::GetTotalPendingValue: Wrong ledger type (expected "
                 "inbox).\n";
        return 0;
    }

    for (auto& it : m_mapTransactions) {
        const auto pTransaction = it.second;
        OT_ASSERT(false != bool(pTransaction));

        if (pTransaction->GetType() == transactionType::pending)
            lTotalPendingValue +=
                pTransaction->GetReceiptAmount();  // this actually loads up the
        // original item and reads the
        // amount.
    }

    return lTotalPendingValue;
}

// Called by the above function.
// This ledger is an outbox, and it is creating a report of itself,
// adding each report item to this balance item.
// DO NOT call this, it's meant to be used only by above function.
void Ledger::ProduceOutboxReport(Item& theBalanceItem)
{
    if (ledgerType::outbox != GetType()) {
        otErr << __FUNCTION__ << ": Wrong ledger type.\n";
        return;
    }

    // loop through the OUTBOX transactions, and produce a sub-item onto
    // theBalanceItem for each, which will
    // be a report on each pending transfer in this outbox, therefore added to
    // the balance item.
    // (So the balance item contains a complete report on the outoing transfers
    // in this outbox.)
    for (auto& it : m_mapTransactions) {
        auto pTransaction = it.second;
        OT_ASSERT(false != bool(pTransaction));

        // it only reports receipts where we don't yet have balance agreement.
        pTransaction->ProduceOutboxReportItem(
            theBalanceItem);  // <======= This function adds a pending transfer
                              // sub-item to theBalanceItem, where appropriate.
    }
}

// Auto-detects ledger type. (message/nymbox/inbox/outbox)
// Use this instead of LoadContractFromString (for ledgers,
// for when you don't know their type already.)
// Otherwise if you know the type, then use LoadNymboxFromString() etc.
//
bool Ledger::LoadLedgerFromString(const String& theStr)
{
    bool bLoaded = false;

    // Todo security: Look how this is done...
    // Any vulnerabilities?
    //
    if (theStr.Contains("type=\"nymbox\""))
        bLoaded = LoadNymboxFromString(theStr);
    else if (theStr.Contains("type=\"inbox\""))
        bLoaded = LoadInboxFromString(theStr);
    else if (theStr.Contains("type=\"outbox\""))
        bLoaded = LoadOutboxFromString(theStr);
    else if (theStr.Contains("type=\"paymentInbox\""))
        bLoaded = LoadPaymentInboxFromString(theStr);
    else if (theStr.Contains("type=\"recordBox\""))
        bLoaded = LoadRecordBoxFromString(theStr);
    else if (theStr.Contains("type=\"expiredBox\""))
        bLoaded = LoadExpiredBoxFromString(theStr);
    else if (theStr.Contains("type=\"message\"")) {
        m_Type = ledgerType::message;
        bLoaded = LoadContractFromString(theStr);
    }
    return bLoaded;
}

// SignContract will call this function at the right time.
void Ledger::UpdateContents()  // Before transmission or serialization, this is
                               // where the ledger saves its contents
{
    switch (GetType()) {
        case ledgerType::message:
        case ledgerType::nymbox:
        case ledgerType::inbox:
        case ledgerType::outbox:
        case ledgerType::paymentInbox:
        case ledgerType::recordBox:
        case ledgerType::expiredBox:
            break;
        default:
            otErr
                << "OTLedger::UpdateContents: Error: unexpected box type (1st "
                   "block). (This should never happen.)\n";
            return;
    }

    // Abbreviated for all types but OTLedger::message.
    // A message ledger stores the full receipts directly inside itself. (No
    // separate files.)
    // For other types: These store abbreviated versions of themselves, with the
    // actual receipts
    // in separate files. Those separate files are created on server side when
    // first added to the
    // box, and on client side when downloaded from the server. They must match
    // the hash that
    // appears in the box.
    bool bSavingAbbreviated = GetType() != ledgerType::message;

    // We store this, so we know how many abbreviated records to read back
    // later.
    std::int32_t nPartialRecordCount = 0;
    if (bSavingAbbreviated) {
        nPartialRecordCount =
            static_cast<std::int32_t>(m_mapTransactions.size());
    }

    // Notice I use the PURPORTED Account ID and Notary ID to create the output.
    // That's because I don't want to inadvertantly substitute the real ID
    // for a bad one and then sign it.
    // So if there's a bad one in there when I read it, THAT's the one that I
    // write as well!
    auto strType = String::Factory(GetTypeString()),
         strLedgerAcctID = String::Factory(GetPurportedAccountID()),
         strLedgerAcctNotaryID = String::Factory(GetPurportedNotaryID()),
         strNymID = String::Factory(GetNymID());

    // I release this because I'm about to repopulate it.
    m_xmlUnsigned->Release();

    Tag tag("accountLedger");

    tag.add_attribute("version", m_strVersion->Get());
    tag.add_attribute("type", strType->Get());
    tag.add_attribute("numPartialRecords", formatInt(nPartialRecordCount));
    tag.add_attribute("accountID", strLedgerAcctID->Get());
    tag.add_attribute("nymID", strNymID->Get());
    tag.add_attribute("notaryID", strLedgerAcctNotaryID->Get());

    // loop through the transactions and print them out here.
    for (auto& it : m_mapTransactions) {
        auto pTransaction = it.second;
        OT_ASSERT(false != bool(pTransaction));

        if (false == bSavingAbbreviated)  // only OTLedger::message uses this
                                          // block.
        {
            // Save the FULL version of the receipt inside the box, so
            // no separate files are necessary.
            auto strTransaction = String::Factory();

            pTransaction->SaveContractRaw(strTransaction);
            auto ascTransaction = Armored::Factory();
            ascTransaction->SetString(strTransaction, true);  // linebreaks =
                                                              // true

            tag.add_tag("transaction", ascTransaction->Get());
        } else  // true == bSavingAbbreviated
        {
            // ALL OTHER ledger types are
            // saved here in abbreviated form.

            switch (GetType()) {

                case ledgerType::nymbox:
                    pTransaction->SaveAbbreviatedNymboxRecord(tag);
                    break;
                case ledgerType::inbox:
                    pTransaction->SaveAbbreviatedInboxRecord(tag);
                    break;
                case ledgerType::outbox:
                    pTransaction->SaveAbbreviatedOutboxRecord(tag);
                    break;
                case ledgerType::paymentInbox:
                    pTransaction->SaveAbbrevPaymentInboxRecord(tag);
                    break;
                case ledgerType::recordBox:
                    pTransaction->SaveAbbrevRecordBoxRecord(tag);
                    break;
                case ledgerType::expiredBox:
                    pTransaction->SaveAbbrevExpiredBoxRecord(tag);
                    break;

                default
                    :  // todo: possibly change this to an OT_ASSERT. security.
                    otErr << "OTLedger::UpdateContents: Error: unexpected box "
                             "type "
                             "(2nd block). (This should never happen. "
                             "Skipping.)\n";

                    OT_FAIL_MSG("ASSERT: OTLedger::UpdateContents: Unexpected "
                                "ledger type.");

                    continue;
            }
        }
    }

    std::string str_result;
    tag.output(str_result);

    m_xmlUnsigned->Concatenate("%s", str_result.c_str());
}

// LoadContract will call this function at the right time.
// return -1 if error, 0 if nothing, and 1 if the node was processed.
std::int32_t Ledger::ProcessXMLNode(irr::io::IrrXMLReader*& xml)
{
    const char* szFunc = "OTLedger::ProcessXMLNode";

    const auto strNodeName = String::Factory(xml->getNodeName());

    if (strNodeName->Compare("accountLedger")) {
        auto strType = String::Factory(),               // ledger type
            strLedgerAcctID = String::Factory(),        // purported
            strLedgerAcctNotaryID = String::Factory(),  // purported
            strNymID = String::Factory(),
             strNumPartialRecords =
                 String::Factory();  // Ledger contains either full
                                     // receipts, or abbreviated
                                     // receipts with hashes and partial
                                     // data.

        strType = String::Factory(xml->getAttributeValue("type"));
        m_strVersion = String::Factory(xml->getAttributeValue("version"));

        if (strType->Compare("message"))  // These are used for sending
            // transactions in messages. (Withdrawal
            // request, etc.)
            m_Type = ledgerType::message;
        else if (strType->Compare("nymbox"))  // Used for receiving new
                                              // transaction numbers, and for
                                              // receiving notices.
            m_Type = ledgerType::nymbox;
        else if (strType->Compare("inbox"))  // These are used for storing the
                                             // receipts in your inbox. (That
                                             // server must store until
                                             // signed-off.)
            m_Type = ledgerType::inbox;
        else if (strType->Compare("outbox"))  // Outgoing, pending transfers.
            m_Type = ledgerType::outbox;
        else if (strType->Compare("paymentInbox"))  // Receiving invoices, etc.
            m_Type = ledgerType::paymentInbox;
        else if (strType->Compare("recordBox"))  // Where receipts go to die
                                                 // (awaiting user deletion,
                                                 // completed from other boxes
                                                 // already.)
            m_Type = ledgerType::recordBox;
        else if (strType->Compare("expiredBox"))  // Where expired payments go
                                                  // to die (awaiting user
                                                  // deletion, completed from
                                                  // other boxes already.)
            m_Type = ledgerType::expiredBox;
        else
            m_Type = ledgerType::error_state;  // Danger, Will Robinson.

        strLedgerAcctID = String::Factory(xml->getAttributeValue("accountID"));
        strLedgerAcctNotaryID =
            String::Factory(xml->getAttributeValue("notaryID"));
        strNymID = String::Factory(xml->getAttributeValue("nymID"));

        if (!strLedgerAcctID->Exists() || !strLedgerAcctNotaryID->Exists() ||
            !strNymID->Exists()) {
            otOut << szFunc << ": Failure: missing strLedgerAcctID ("
                  << strLedgerAcctID
                  << ") or "
                     "strLedgerAcctNotaryID ("
                  << strLedgerAcctNotaryID << ") or strNymID (" << strNymID
                  << ") while loading transaction "
                     "from "
                  << strType << " ledger. \n";
            return (-1);
        }

        auto ACCOUNT_ID = Identifier::Factory(strLedgerAcctID),
             NOTARY_ID = Identifier::Factory(strLedgerAcctNotaryID),
             NYM_ID = Identifier::Factory(strNymID);

        SetPurportedAccountID(ACCOUNT_ID);
        SetPurportedNotaryID(NOTARY_ID);
        SetNymID(NYM_ID);

        if (!m_bLoadSecurely) {
            SetRealAccountID(ACCOUNT_ID);
            SetRealNotaryID(NOTARY_ID);
            //            OTString str1(GetRealAccountID()),
            // str2(GetRealNotaryID());
            //            otErr << "DEBUGGING:\nReal Acct ID: %s\nReal Server
            // ID: %s\n",
            //                          str1.Get(), str2.Get());
        }

        // Load up the partial records, based on the expected count...
        //
        strNumPartialRecords =
            String::Factory(xml->getAttributeValue("numPartialRecords"));
        std::int32_t nPartialRecordCount =
            (strNumPartialRecords->Exists() ? atoi(strNumPartialRecords->Get())
                                            : 0);

        auto strExpected = String::Factory();  // The record type has a
                                               // different name for each box.
        NumList theNumList;
        NumList* pNumList = nullptr;
        switch (m_Type) {
            case ledgerType::nymbox:
                strExpected->Set("nymboxRecord");
                pNumList = &theNumList;
                break;
            case ledgerType::inbox:
                strExpected->Set("inboxRecord");
                break;
            case ledgerType::outbox:
                strExpected->Set("outboxRecord");
                break;
            case ledgerType::paymentInbox:
                strExpected->Set("paymentInboxRecord");
                break;
            case ledgerType::recordBox:
                strExpected->Set("recordBoxRecord");
                break;
            case ledgerType::expiredBox:
                strExpected->Set("expiredBoxRecord");
                break;
            /* --- BREAK --- */
            case ledgerType::message:
                if (nPartialRecordCount > 0) {
                    otErr << szFunc << ": Error: There are "
                          << nPartialRecordCount
                          << " unexpected abbreviated records in an "
                             "OTLedger::message type ledger. (Failed loading "
                             "ledger with accountID: "
                          << strLedgerAcctID << ")\n";
                    return (-1);
                }

                break;
            default:
                otErr << "OTLedger::ProcessXMLNode: Unexpected ledger type ("
                      << strType
                      << "). (Failed loading "
                         "ledger for account: "
                      << strLedgerAcctID << ")\n";
                return (-1);
        }  // switch (to set strExpected to the abbreviated record type.)

        if (nPartialRecordCount > 0)  // message ledger will never enter this
                                      // block due to switch block (above.)
        {

            // We iterate to read the expected number of partial records from
            // the xml.
            // (They had better be there...)
            //
            while (nPartialRecordCount-- > 0) {
                //                xml->read(); // <==================
                if (!SkipToElement(xml)) {
                    otOut << szFunc
                          << ": Failure: Unable to find element when "
                             "one was expected ("
                          << strExpected
                          << ") "
                             "for abbreviated record of receipt in "
                          << GetTypeString() << " box:\n\n"
                          << m_strRawFile << "\n\n";
                    return (-1);
                }

                // strExpected can be one of:
                //
                //                strExpected.Set("nymboxRecord");
                //                strExpected.Set("inboxRecord");
                //                strExpected.Set("outboxRecord");
                //
                // We're loading here either a nymboxRecord, inboxRecord, or
                // outboxRecord...
                //
                const auto strLoopNodeName =
                    String::Factory(xml->getNodeName());

                if (strLoopNodeName->Exists() &&
                    (xml->getNodeType() == irr::io::EXN_ELEMENT) &&
                    (strExpected->Compare(strLoopNodeName))) {
                    std::int64_t lNumberOfOrigin = 0;
                    originType theOriginType =
                        originType::not_applicable;  // default
                    std::int64_t lTransactionNum = 0;
                    std::int64_t lInRefTo = 0;
                    std::int64_t lInRefDisplay = 0;

                    time64_t the_DATE_SIGNED = OT_TIME_ZERO;
                    transactionType theType =
                        transactionType::error_state;  // default
                    auto strHash = String::Factory();

                    std::int64_t lAdjustment = 0;
                    std::int64_t lDisplayValue = 0;
                    std::int64_t lClosingNum = 0;
                    std::int64_t lRequestNum = 0;
                    bool bReplyTransSuccess = false;

                    std::int32_t nAbbrevRetVal = LoadAbbreviatedRecord(
                        xml,
                        lNumberOfOrigin,
                        theOriginType,
                        lTransactionNum,
                        lInRefTo,
                        lInRefDisplay,
                        the_DATE_SIGNED,
                        theType,
                        strHash,
                        lAdjustment,
                        lDisplayValue,
                        lClosingNum,
                        lRequestNum,
                        bReplyTransSuccess,
                        pNumList);  // This is for "transactionType::blank" and
                                    // "transactionType::successNotice",
                                    // otherwise nullptr.
                    if ((-1) == nAbbrevRetVal)
                        return (-1);  // The function already logs
                                      // appropriately.

                    //
                    // See if the same-ID transaction already exists in the
                    // ledger.
                    // (There can only be one.)
                    //
                    auto pExistingTrans = GetTransaction(lTransactionNum);
                    if (false != bool(pExistingTrans))  // Uh-oh, it's already
                                                        // there!
                    {
                        otOut << szFunc << ": Error loading transaction "
                              << lTransactionNum << " (" << strExpected
                              << "), since one was already there, in box for "
                                 "account: "
                              << strLedgerAcctID << ".\n";
                        return (-1);
                    }

                    // CONSTRUCT THE ABBREVIATED RECEIPT HERE...

                    // Set all the values we just loaded here during actual
                    // construction of transaction
                    // (as abbreviated transaction) i.e. make a special
                    // constructor for abbreviated transactions
                    // which is ONLY used here.
                    //
                    auto pTransaction{api_.Factory().Transaction(
                        NYM_ID,
                        ACCOUNT_ID,
                        NOTARY_ID,
                        lNumberOfOrigin,
                        static_cast<originType>(theOriginType),
                        lTransactionNum,
                        lInRefTo,  // lInRefTo
                        lInRefDisplay,
                        the_DATE_SIGNED,
                        static_cast<transactionType>(theType),
                        strHash,
                        lAdjustment,
                        lDisplayValue,
                        lClosingNum,
                        lRequestNum,
                        bReplyTransSuccess,
                        pNumList)};  // This is for "transactionType::blank" and
                                     // "transactionType::successNotice",
                                     // otherwise nullptr.
                    OT_ASSERT(false != bool(pTransaction));
                    //
                    // NOTE: For THIS CONSTRUCTOR ONLY, we DO set the purported
                    // AcctID and purported NotaryID.
                    // WHY? Normally you set the "real" IDs at construction, and
                    // then set the "purported" IDs
                    // when loading from string. But this constructor (only this
                    // one) is actually used when
                    // loading abbreviated receipts as you load their
                    // inbox/outbox/nymbox.
                    // Abbreviated receipts are not like real transactions,
                    // which have notaryID, AcctID, nymID,
                    // and signature attached, and the whole thing is
                    // base64-encoded and then added to the ledger
                    // as part of a list of contained objects. Rather, with
                    // abbreviated receipts, there are a series
                    // of XML records loaded up as PART OF the ledger itself.
                    // None of these individual XML records
                    // has its own signature, or its own record of the main IDs
                    // -- those are assumed to be on the parent
                    // ledger.
                    // That's the whole point: abbreviated records don't store
                    // redundant info, and don't each have their
                    // own signature, because we want them to be as small as
                    // possible inside their parent ledger.
                    // Therefore I will pass in the parent ledger's "real" IDs
                    // at construction, and immediately thereafter
                    // set the parent ledger's "purported" IDs onto the
                    // abbreviated transaction. That way, VerifyContractID()
                    // will still work and do its job properly with these
                    // abbreviated records.
                    //
                    // NOTE: Moved to OTTransaction constructor (for
                    // abbreviateds) for now.
                    //
                    //                    pTransaction->SetPurportedAccountID(
                    // GetPurportedAccountID());
                    //                    pTransaction->SetPurportedNotaryID(
                    // GetPurportedNotaryID());

                    // Add it to the ledger's list of transactions...
                    //

                    if (pTransaction->VerifyContractID()) {
                        // Add it to the ledger...
                        //
                        std::shared_ptr<OTTransaction> transaction{
                            pTransaction.release()};
                        m_mapTransactions[transaction->GetTransactionNum()] =
                            transaction;
                        transaction->SetParent(*this);
                    } else {
                        otErr << szFunc
                              << ": ERROR: verifying contract ID on "
                                 "abbreviated transaction "
                              << pTransaction->GetTransactionNum() << "\n";
                        return (-1);
                    }
                    //                    xml->read(); // <==================
                    // MIGHT need to add "skip after element" here.
                    //
                    // Update: Nope.
                } else {
                    otErr << szFunc
                          << ": Expected abbreviated record element.\n";
                    return (-1);  // error condition
                }
            }  // while
        }      // if (number of partial records > 0)

        LogTrace(OT_METHOD)(__FUNCTION__)(
            ": Loading account ledger of type \"")(strType)("\", version: ")(
            m_strVersion)
            .Flush();

        // Since we just loaded this stuff, let's verify it. We may have to
        // remove this verification here and do it outside this call. But for
        // now...

        if (VerifyContractID()) {
            return 1;
        } else {
            return (-1);
        }
    }

    // Todo: When loading abbreviated list of records, set the m_bAbbreviated to
    // true.
    // Then in THIS block below, if that is set to true, then seek an existing
    // transaction instead of
    // instantiating a new one. Then repopulate the new one and verify the new
    // values against the ones
    // that were already there before overwriting anything.

    // Hmm -- technically this code should only execute for OTLedger::message,
    // and thus only if
    // m_bIsAbbreviated is FALSE. When the complete receipt is loaded,
    // "LoadBoxReceipt()" will be
    // called, and it will directly load the transaction starting in
    // OTTransaction::ProcessXMLNode().
    // THAT is where we must check for abbreviated mode and expect it already
    // loaded etc etc. Whereas
    // here in this spot, we basically want to error out if it's not a message
    // ledger.
    // UPDATE: However, I must consider legacy data. For now, I'll allow this to
    // load in any type of box.
    // I also need to check and see if the box receipt already exists (since its
    // normal creation point
    // may not have happened, when taking legacy data into account.) If it
    // doesn't already exist, then I
    // should save it again at this point.
    //
    else if (strNodeName->Compare("transaction")) {
        auto strTransaction = String::Factory();
        auto ascTransaction = Armored::Factory();

        // go to the next node and read the text.
        //        xml->read(); // <==================
        if (!SkipToTextField(xml)) {
            otOut << __FUNCTION__
                  << ": Failure: Unable to find expected text field "
                     "containing receipt transaction in box. \n";
            return (-1);
        }

        if (irr::io::EXN_TEXT == xml->getNodeType()) {
            // the ledger contains a series of transactions.
            // Each transaction is initially stored as an Armored string.
            const auto strLoopNodeData = String::Factory(xml->getNodeData());

            if (strLoopNodeData->Exists())
                ascTransaction->Set(strLoopNodeData);  // Put the ascii-armored
                                                       // node data into the
                                                       // ascii-armor object

            // Decode that into strTransaction, so we can load the transaction
            // object from that string.
            if (!ascTransaction->Exists() ||
                !ascTransaction->GetString(strTransaction)) {
                otErr << __FUNCTION__
                      << ": ERROR: Missing expected transaction contents. "
                         "Ledger contents:\n\n"
                      << m_strRawFile << "\n\n";
                return (-1);
            }

            // I belive we're only supposed to use purported numbers when
            // loading/saving, and to compare them (as distrusted)
            // against a more-trusted source, in order to verify them. Whereas
            // when actually USING the numbers (such as here,
            // when "GetRealAccountID()" is being used to instantiate the
            // transaction, then you ONLY use numbers that you KNOW
            // are good (the number you were expecting) versus whatever number
            // was actually in the file.
            // But wait, you ask, how do I know they are the same number then?
            // Because you verified that when you first loaded
            // everything into memory. Right after "load" was a "verify" that
            // makes sure the "real" account ID and the "purported"
            // account ID are actually the same.
            //
            // UPDATE: If this ledger is loaded from string, there's no
            // guarantee that the real IDs have even been set.
            // In some cases (Factory...) they definitely have not been. It
            // makes sense here when loading, to set the member
            // transactions to the same account/server IDs that were actually
            // loaded for their parent ledger. Therefore, changing
            // back here to Purported values.
            //
            //            OTTransaction * pTransaction = new
            // OTTransaction(GetNymID(), GetRealAccountID(),
            // GetRealNotaryID());
            auto pTransaction{api_.Factory().Transaction(
                GetNymID(), GetPurportedAccountID(), GetPurportedNotaryID())};
            OT_ASSERT(false != bool(pTransaction));

            // Need this set before the LoadContractFromString().
            //
            if (!m_bLoadSecurely) pTransaction->SetLoadInsecure();

            // If we're able to successfully base64-decode the string and load
            // it up as
            // a transaction, then let's add it to the ledger's list of
            // transactions
            if (strTransaction->Exists() &&
                pTransaction->LoadContractFromString(strTransaction) &&
                pTransaction->VerifyContractID())
            // I responsible here to call pTransaction->VerifyContractID()
            // since
            // I am loading it here and adding it to the ledger. (So I do.)
            {

                auto pExistingTrans =
                    GetTransaction(pTransaction->GetTransactionNum());
                if (false != bool(pExistingTrans))  // Uh-oh, it's already
                                                    // there!
                {
                    const auto strPurportedAcctID =
                        String::Factory(GetPurportedAccountID());
                    otOut
                        << szFunc << ": Error loading full transaction "
                        << pTransaction->GetTransactionNum()
                        << ", since one was already there, in box for account: "
                        << strPurportedAcctID << ".\n";
                    return (-1);
                }

                // It's not already there on this ledger -- so add it!
                std::shared_ptr<OTTransaction> transaction{
                    pTransaction.release()};
                m_mapTransactions[transaction->GetTransactionNum()] =
                    transaction;
                transaction->SetParent(*this);

                switch (GetType()) {
                    case ledgerType::message:
                        break;
                    case ledgerType::nymbox:
                    case ledgerType::inbox:
                    case ledgerType::outbox:
                    case ledgerType::paymentInbox:
                    case ledgerType::recordBox:
                    case ledgerType::expiredBox: {
                        // For the sake of legacy data, check for existence of
                        // box
                        // receipt here,
                        // and re-save that box receipt if it doesn't exist.
                        //
                        otOut << "--- Apparently this is old data (the "
                                 "transaction "
                                 "is still stored inside the ledger itself)... "
                                 "\n";
                        m_bLoadedLegacyData =
                            true;  // Only place this is set true.

                        const std::int32_t nBoxType =
                            static_cast<std::int32_t>(GetType());

                        const bool bBoxReceiptAlreadyExists =
                            VerifyBoxReceiptExists(
                                api_.DataFolder(),
                                transaction->GetRealNotaryID(),
                                transaction->GetNymID(),
                                transaction->GetRealAccountID(),  // If Nymbox
                                                                  // (vs
                                // inbox/outbox)
                                // the NYM_ID
                                // will be in this
                                // field also.
                                nBoxType,  // 0/nymbox, 1/inbox, 2/outbox
                                transaction->GetTransactionNum());
                        if (!bBoxReceiptAlreadyExists)  // Doesn't already
                                                        // exist separately.
                        {
                            // Okay then, let's create it...
                            //
                            otOut << "--- The BoxReceipt doesn't exist "
                                     "separately "
                                     "(yet.) Creating it in local storage...\n";

                            const std::int64_t lBoxType =
                                static_cast<std::int64_t>(nBoxType);

                            if (false == transaction->SaveBoxReceipt(
                                             lBoxType))  //  <======== SAVE BOX
                                                         //  RECEIPT
                                otErr << "--- FAILED trying to save BoxReceipt "
                                         "from legacy data to local storage!\n";
                        }
                    } break;
                    default:
                        otErr << szFunc
                              << ": Unknown ledger type while loading "
                                 "transaction!"
                                 " (Should never happen.)\n";  // todo: assert
                                                               // here?
                                                               // "should never
                                                               // happen" ...
                        return (-1);
                }  // switch (GetType())

            }  // if transaction loads and verifies.
            else {
                otErr << "ERROR: loading or verifying transaction in "
                         "OTLedger::ProcessXMLNode\n";
                return (-1);
            }
        } else {
            otErr << "Error in OTLedger::ProcessXMLNode: transaction without "
                     "value.\n";
            return (-1);  // error condition
        }
        return 1;
    }

    return 0;
}

void Ledger::ReleaseTransactions()
{
    // If there were any dynamically allocated objects, clean them up here.

    m_mapTransactions.clear();
}

void Ledger::Release_Ledger() { ReleaseTransactions(); }

void Ledger::Release()
{
    Release_Ledger();

    ot_super::Release();  // since I've overridden the base class, I call it
                          // now...
}

Ledger::~Ledger() { Release_Ledger(); }
}  // namespace opentxs
