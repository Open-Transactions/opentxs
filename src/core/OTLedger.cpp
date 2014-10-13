/************************************************************
 *
 *  OTLedger.cpp
 *
 */

/************************************************************
 -----BEGIN PGP SIGNED MESSAGE-----
 Hash: SHA1

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
 *  Copyright (C) 2010-2013 by "Fellow Traveler" (A pseudonym)
 *
 *  EMAIL:
 *  FellowTraveler@rayservers.net
 *
 *  BITCOIN:  1NtTPVVjDsUfDWybS4BwvHpG2pdS9RnYyQ
 *
 *  KEY FINGERPRINT (PGP Key in license file):
 *  9DD5 90EB 9292 4B48 0484  7910 0308 00ED F951 BB8E
 *
 *  OFFICIAL PROJECT WIKI(s):
 *  https://github.com/FellowTraveler/Moneychanger
 *  https://github.com/FellowTraveler/Open-Transactions/wiki
 *
 *  WEBSITE:
 *  http://www.OpenTransactions.org/
 *
 *  Components and licensing:
 *   -- Moneychanger..A Java client GUI.....LICENSE:.....GPLv3
 *   -- otlib.........A class library.......LICENSE:...LAGPLv3
 *   -- otapi.........A client API..........LICENSE:...LAGPLv3
 *   -- opentxs/ot....Command-line client...LICENSE:...LAGPLv3
 *   -- otserver......Server Application....LICENSE:....AGPLv3
 *  Github.com/FellowTraveler/Open-Transactions/wiki/Components
 *
 *  All of the above OT components were designed and written by
 *  Fellow Traveler, with the exception of Moneychanger, which
 *  was contracted out to Vicky C (bitcointrader4@gmail.com).
 *  The open-source community has since actively contributed.
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This program is free software: you can redistribute it
 *   and/or modify it under the terms of the GNU Affero
 *   General Public License as published by the Free Software
 *   Foundation, either version 3 of the License, or (at your
 *   option) any later version.
 *
 *   ADDITIONAL PERMISSION under the GNU Affero GPL version 3
 *   section 7: (This paragraph applies only to the LAGPLv3
 *   components listed above.) If you modify this Program, or
 *   any covered work, by linking or combining it with other
 *   code, such other code is not for that reason alone subject
 *   to any of the requirements of the GNU Affero GPL version 3.
 *   (==> This means if you are only using the OT API, then you
 *   don't have to open-source your code--only your changes to
 *   Open-Transactions itself must be open source. Similar to
 *   LGPLv3, except it applies to software-as-a-service, not
 *   just to distributing binaries.)
 *
 *   Extra WAIVER for OpenSSL, Lucre, and all other libraries
 *   used by Open Transactions: This program is released under
 *   the AGPL with the additional exemption that compiling,
 *   linking, and/or using OpenSSL is allowed. The same is true
 *   for any other open source libraries included in this
 *   project: complete waiver from the AGPL is hereby granted to
 *   compile, link, and/or use them with Open-Transactions,
 *   according to their own terms, as long as the rest of the
 *   Open-Transactions terms remain respected, with regard to
 *   the Open-Transactions code itself.
 *
 *   Lucre License:
 *   This code is also "dual-license", meaning that Ben Lau-
 *   rie's license must also be included and respected, since
 *   the code for Lucre is also included with Open Transactions.
 *   See Open-Transactions/src/otlib/lucre/LUCRE_LICENSE.txt
 *   The Laurie requirements are light, but if there is any
 *   problem with his license, simply remove the Lucre code.
 *   Although there are no other blind token algorithms in Open
 *   Transactions (yet. credlib is coming), the other functions
 *   will continue to operate.
 *   See Lucre on Github:  https://github.com/benlaurie/lucre
 *   -----------------------------------------------------
 *   You should have received a copy of the GNU Affero General
 *   Public License along with this program.  If not, see:
 *   http://www.gnu.org/licenses/
 *
 *   If you would like to use this software outside of the free
 *   software license, please contact FellowTraveler.
 *   (Unfortunately many will run anonymously and untraceably,
 *   so who could really stop them?)
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will be
 *   useful, but WITHOUT ANY WARRANTY; without even the implied
 *   warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *   PURPOSE.  See the GNU Affero General Public License for
 *   more details.

 -----BEGIN PGP SIGNATURE-----
 Version: GnuPG v1.4.9 (Darwin)

 iQIcBAEBAgAGBQJRSsfJAAoJEAMIAO35UbuOQT8P/RJbka8etf7wbxdHQNAY+2cC
 vDf8J3X8VI+pwMqv6wgTVy17venMZJa4I4ikXD/MRyWV1XbTG0mBXk/7AZk7Rexk
 KTvL/U1kWiez6+8XXLye+k2JNM6v7eej8xMrqEcO0ZArh/DsLoIn1y8p8qjBI7+m
 aE7lhstDiD0z8mwRRLKFLN2IH5rAFaZZUvj5ERJaoYUKdn4c+RcQVei2YOl4T0FU
 LWND3YLoH8naqJXkaOKEN4UfJINCwxhe5Ke9wyfLWLUO7NamRkWD2T7CJ0xocnD1
 sjAzlVGNgaFDRflfIF4QhBx1Ddl6wwhJfw+d08bjqblSq8aXDkmFA7HeunSFKkdn
 oIEOEgyj+veuOMRJC5pnBJ9vV+7qRdDKQWaCKotynt4sWJDGQ9kWGWm74SsNaduN
 TPMyr9kNmGsfR69Q2Zq/FLcLX/j8ESxU+HYUB4vaARw2xEOu2xwDDv6jt0j3Vqsg
 x7rWv4S/Eh18FDNDkVRChiNoOIilLYLL6c38uMf1pnItBuxP3uhgY6COm59kVaRh
 nyGTYCDYD2TK+fI9o89F1297uDCwEJ62U0Q7iTDp5QuXCoxkPfv8/kX6lS6T3y9G
 M9mqIoLbIQ1EDntFv7/t6fUTS2+46uCrdZWbQ5RjYXdrzjij02nDmJAm2BngnZvd
 kamH0Y/n11lCvo1oQxM+
 =uSzz
 -----END PGP SIGNATURE-----
 **************************************************************/

#include <opentxs/core/stdafx.hpp>

#include <opentxs/core/OTLedger.hpp>
#include <opentxs/core/OTAccount.hpp>
#include <opentxs/core/OTCheque.hpp>
#include <opentxs/core/crypto/OTEnvelope.hpp>
#include <opentxs/core/util/OTFolders.hpp>
#include <opentxs/core/OTLog.hpp>
#include <opentxs/core/OTMessage.hpp>
#include <opentxs/core/OTPseudonym.hpp>
#include <opentxs/core/OTStorage.hpp>
#include <opentxs/core/transaction/Helpers.hpp>

#include <irrxml/irrXML.hpp>

#include <memory>

namespace opentxs
{

char const* const __TypeStringsLedger[] = {
    "nymbox", // the nymbox is per user account (versus per asset account) and
              // is used to receive new transaction numbers (and messages.)
    "inbox",  // each asset account has an inbox, with pending transfers as well
              // as receipts inside.
    "outbox", // if you SEND a pending transfer, it sits in your outbox until
              // it's accepted, rejected, or canceled.
    "message", // used in OTMessages, to send various lists of transactions back
               // and forth.
    "paymentInbox", // Used for client-side-only storage of incoming cheques,
                    // invoices, payment plan requests, etc. (Coming in from the
                    // Nymbox.)
    "recordBox",    // Used for client-side-only storage of completed items from
                    // the inbox, and the paymentInbox.
    "expiredBox", // Used for client-side-only storage of expired items from the
                  // paymentInbox.
    "error_state"};

char const* OTLedger::_GetTypeString(ledgerType theType)
{
    int32_t nType = static_cast<int32_t>(theType);
    return __TypeStringsLedger[nType];
}

// This calls OTTransactionType::VerifyAccount(), which calls
// VerifyContractID() as well as VerifySignature().
//
// But first, this OTLedger version also loads the box receipts,
// if doing so is appropriate. (message ledger == not appropriate.)
//
// Use this method instead of OTContract::VerifyContract, which
// expects/uses a pubkey from inside the contract in order to verify
// it.
//
bool OTLedger::VerifyAccount(const OTPseudonym& theNym)
{
    switch (GetType()) {
    case OTLedger::message: // message ledgers do not load Box Receipts. (They
                            // store full version internally already.)
        break;
    case OTLedger::nymbox:
    case OTLedger::inbox:
    case OTLedger::outbox:
    case OTLedger::paymentInbox:
    case OTLedger::recordBox:
    case OTLedger::expiredBox: {
        std::set<int64_t> setUnloaded;
        // if psetUnloaded passed in, then use it to return the #s that weren't
        // there as box receipts.
        //            bool bLoadedBoxReceipts =
        LoadBoxReceipts(
            &setUnloaded); // Note: Also useful for suppressing errors here.
    } break;
    default: {
        const int32_t nLedgerType = static_cast<int32_t>(GetType());
        const OTIdentifier theNymID(theNym);
        const OTString strNymID(theNymID);
        OTString strAccountID;
        GetIdentifier(strAccountID);
        otErr << "OTLedger::VerifyAccount: Failure: Bad ledger type: "
              << nLedgerType << ", UserID: " << strNymID
              << ", AcctID: " << strAccountID << "\n";
    }
        return false;
    }

    return OTTransactionType::VerifyAccount(theNym);
}
/*
 bool OTTransactionType::VerifyAccount(OTPseudonym& theNym)
{
    // Make sure that the supposed AcctID matches the one read from the file.
    //
    if (!VerifyContractID())
    {
        otErr << "Error verifying account ID in
OTTransactionType::VerifyAccount\n";
        return false;
    }
    else if (!VerifySignature(theNym))
    {
        otErr << "Error verifying signature in
OTTransactionType::VerifyAccount.\n";
        return false;
    }

    otLog4 << "\nWe now know that...\n"
            "1) The expected Account ID matches the ID that was found on the
object.\n"
            "2) The SIGNATURE VERIFIED on the object.\n\n");
    return true;
}
*/

// This makes sure that ALL transactions inside the ledger are saved as box
// receipts
// in their full (not abbreviated) form (as separate files.)
//
bool OTLedger::SaveBoxReceipts() // For ALL full transactions, save the actual
                                 // box receipt for each to its own place.
{
    bool bRetVal = true;
    for (auto& it : m_mapTransactions) {
        OTTransaction* pTransaction = it.second;
        OT_ASSERT(nullptr != pTransaction);

        // We only save full versions of transactions as box receipts, not
        // abbreviated versions.
        // (If it's not abbreviated, therefore it's the full version.)
        //
        if (!pTransaction->IsAbbreviated()) // This way we won't see an
                                            // error if it's not
                                            // abbreviated.
            bRetVal = pTransaction->SaveBoxReceipt(*this);

        if (!bRetVal) {
            otErr << "OTLedger::SaveBoxReceipts: Failed calling SaveBoxReceipt "
                     "on transaction: " << pTransaction->GetTransactionNum()
                  << ".\n";
            break;
        }
    }
    return bRetVal;
}

bool OTLedger::SaveBoxReceipt(const int64_t& lTransactionNum)
{

    // First, see if the transaction itself exists on this ledger.
    // Get a pointer to it.
    OTTransaction* pTransaction = GetTransaction(lTransactionNum);

    if (nullptr == pTransaction) {
        otOut << "OTLedger::SaveBoxReceipt: Unable to save box receipt "
              << lTransactionNum
              << ": "
                 "couldn't find the transaction on this ledger.\n";
        return false;
    }

    return pTransaction->SaveBoxReceipt(*this);
}

bool OTLedger::DeleteBoxReceipt(const int64_t& lTransactionNum)
{

    // First, see if the transaction itself exists on this ledger.
    // Get a pointer to it.
    OTTransaction* pTransaction = GetTransaction(lTransactionNum);

    if (nullptr == pTransaction) {
        otOut << "OTLedger::DeleteBoxReceipt: Unable to delete (overwrite) box "
                 "receipt " << lTransactionNum
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
bool OTLedger::LoadBoxReceipts(std::set<int64_t>* psetUnloaded)
{
    // Grab a copy of all the transaction #s stored inside this ledger.
    //
    std::set<int64_t> the_set;

    for (auto& it : m_mapTransactions) {
        OTTransaction* pTransaction = it.second;
        OT_ASSERT(nullptr != pTransaction);
        the_set.insert(pTransaction->GetTransactionNum());
    }

    // Now iterate through those numbers and for each, load the box receipt.
    //
    bool bRetVal = true;

    for (auto& it : the_set) {
        int64_t lSetNum = it;

        OTTransaction* pTransaction = GetTransaction(lSetNum);
        OT_ASSERT(nullptr != pTransaction);

        // Failed loading the boxReceipt
        //
        if ((true == pTransaction->IsAbbreviated()) &&
            (false == LoadBoxReceipt(lSetNum))) {
            // WARNING: pTransaction must be re-Get'd below this point if
            // needed, since pointer
            // is bad if success on LoadBoxReceipt() call.
            //
            pTransaction = nullptr;
            bRetVal = false;
            OTLogStream* pLog = &otOut;

            if (nullptr != psetUnloaded) {
                psetUnloaded->insert(lSetNum);
                pLog = &otLog3;
            }
            *pLog << "OTLedger::LoadBoxReceipts: Failed calling LoadBoxReceipt "
                     "on "
                     "abbreviated transaction number:" << lSetNum << ".\n";
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
 While the box itself is stored at (for example) "nymbox/SERVER_ID/USER_ID"
 the box receipts for that box may be stored at: "nymbox/SERVER_ID/USER_ID.r"
 With a specific receipt denoted by transaction:
 "nymbox/SERVER_ID/USER_ID.r/TRANSACTION_ID.rct"
 */

bool OTLedger::LoadBoxReceipt(const int64_t& lTransactionNum)
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
    OTTransaction* pTransaction = GetTransaction(lTransactionNum);

    if (nullptr == pTransaction) {
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
    OTTransaction* pBoxReceipt =
        ::opentxs::LoadBoxReceipt(*pTransaction, *this);

    // success
    if (nullptr != pBoxReceipt) {
        //  Remove the existing, abbreviated receipt, and replace it with the
        // actual receipt.
        //  (If this inbox/outbox/whatever is saved, it will later save in
        // abbreviated form
        //    again.)
        //
        RemoveTransaction(lTransactionNum); // this deletes pTransaction
        pTransaction = nullptr;
        AddTransaction(*pBoxReceipt); // takes ownership.

        return true;
    }

    return false;
}

// the below four functions (load/save in/outbox) assume that the ID is already
// set
// properly.
// Then it uses the ID to form the path for the file that is opened. Easy,
// right?

bool OTLedger::LoadInbox()
{
    //    print_stacktrace();

    bool bRetVal = LoadGeneric(OTLedger::inbox);

    return bRetVal;
}

// TODO really should verify the ServerID after loading the ledger.
// Perhaps just call "VerifyContract" and we'll make sure, for ledgers
// VerifyContract is overriden and explicitly checks the serverID.
// Should also check the Type at the same time.

bool OTLedger::LoadOutbox()
{
    return LoadGeneric(OTLedger::outbox);
}

bool OTLedger::LoadNymbox()
{
    return LoadGeneric(OTLedger::nymbox);
}

bool OTLedger::LoadInboxFromString(const OTString& strBox)
{
    return LoadGeneric(OTLedger::inbox, &strBox);
}

bool OTLedger::LoadOutboxFromString(const OTString& strBox)
{
    return LoadGeneric(OTLedger::outbox, &strBox);
}

bool OTLedger::LoadNymboxFromString(const OTString& strBox)
{
    return LoadGeneric(OTLedger::nymbox, &strBox);
}

bool OTLedger::LoadPaymentInbox()
{
    return LoadGeneric(OTLedger::paymentInbox);
}

bool OTLedger::LoadRecordBox()
{
    return LoadGeneric(OTLedger::recordBox);
}

bool OTLedger::LoadExpiredBox()
{
    return LoadGeneric(OTLedger::expiredBox);
}

bool OTLedger::LoadPaymentInboxFromString(const OTString& strBox)
{
    return LoadGeneric(OTLedger::paymentInbox, &strBox);
}

bool OTLedger::LoadRecordBoxFromString(const OTString& strBox)
{
    return LoadGeneric(OTLedger::recordBox, &strBox);
}

bool OTLedger::LoadExpiredBoxFromString(const OTString& strBox)
{
    return LoadGeneric(OTLedger::expiredBox, &strBox);
}

/**
  OTLedger::LoadGeneric is called by LoadInbox, LoadOutbox, and LoadNymbox.
  Does NOT VerifyAccount after loading -- caller is responsible to do that.

  pString -- optional argument, for when  you prefer to load from a string
  instead of from a file.
 */
bool OTLedger::LoadGeneric(OTLedger::ledgerType theType,
                           const OTString* pString)
{
    m_Type = theType;

    const char* pszType = GetTypeString();
    const char* pszFolder = nullptr;

    switch (theType) {
    case OTLedger::nymbox:
        pszFolder = OTFolders::Nymbox().Get();
        break;
    case OTLedger::inbox:
        pszFolder = OTFolders::Inbox().Get();
        break;
    case OTLedger::outbox:
        pszFolder = OTFolders::Outbox().Get();
        break;
    case OTLedger::paymentInbox:
        pszFolder = OTFolders::PaymentInbox().Get();
        break;
    case OTLedger::recordBox:
        pszFolder = OTFolders::RecordBox().Get();
        break;
    case OTLedger::expiredBox:
        pszFolder = OTFolders::ExpiredBox().Get();
        break;
    /* --- BREAK --- */
    default:
        otErr << "OTLedger::LoadGeneric: Error: unknown box type. (This should "
                 "never happen.)\n";
        return false;
    }
    m_strFoldername = pszFolder;

    OTString strID;
    GetIdentifier(strID);

    const OTString strServerID(GetRealServerID());

    if (!m_strFilename.Exists())
        m_strFilename.Format("%s%s%s", strServerID.Get(),
                             OTLog::PathSeparator(), strID.Get());

    OTString strFilename;
    strFilename = strID.Get();

    const char* szFolder1name =
        m_strFoldername.Get(); // "nymbox" (or "inbox" or "outbox")
    const char* szFolder2name = strServerID.Get(); // "nymbox/SERVER_ID"
    const char* szFilename = strFilename.Get();    // "nymbox/SERVER_ID/USER_ID"
    // (or "inbox/SERVER_ID/ACCT_ID"
    // or
    // "outbox/SERVER_ID/ACCT_ID")

    OTString strRawFile;

    if (nullptr != pString) // Loading FROM A STRING.
        strRawFile.Set(*pString);
    else // Loading FROM A FILE.
    {
        if (!OTDB::Exists(szFolder1name, szFolder2name, szFilename)) {
            otLog3 << pszType << " does not exist in OTLedger::Load" << pszType
                   << ": " << szFolder1name << OTLog::PathSeparator()
                   << szFolder2name << OTLog::PathSeparator() << szFilename
                   << "\n";
            return false;
        }

        // Try to load the ledger from local storage.
        //
        std::string strFileContents(OTDB::QueryPlainString(
            szFolder1name, szFolder2name,
            szFilename)); // <=== LOADING FROM DATA STORE.

        if (strFileContents.length() < 2) {
            otErr << "OTLedger::LoadGeneric: Error reading file: "
                  << szFolder1name << OTLog::PathSeparator() << szFolder2name
                  << OTLog::PathSeparator() << szFilename << "\n";
            return false;
        }

        strRawFile.Set(strFileContents.c_str());
    }
    // NOTE: No need to deal with OT ARMORED INBOX file format here, since
    //       LoadContractFromString already handles that automatically.

    if (!strRawFile.Exists()) {
        otErr << "OTLedger::LoadGeneric: Unable to load box (" << szFolder1name
              << OTLog::PathSeparator() << szFolder2name
              << OTLog::PathSeparator() << szFilename
              << ") from empty string.\n";
        return false;
    }

    bool bSuccess = LoadContractFromString(strRawFile);

    if (!bSuccess) {
        otErr << "Failed loading " << pszType << " "
              << ((nullptr != pString) ? "from string" : "from file")
              << " in OTLedger::Load" << pszType << ": " << szFolder1name
              << OTLog::PathSeparator() << szFolder2name
              << OTLog::PathSeparator() << szFilename << "\n";
        return false;
    }
    else {
        otInfo << "Successfully loaded " << pszType << " "
               << ((nullptr != pString) ? "from string" : "from file")
               << " in OTLedger::Load" << pszType << ": " << szFolder1name
               << OTLog::PathSeparator() << szFolder2name
               << OTLog::PathSeparator() << szFilename << "\n";
    }

    return bSuccess;
}

bool OTLedger::SaveGeneric(OTLedger::ledgerType theType)
{
    m_Type = theType;

    const char* pszFolder = nullptr;
    const char* pszType = GetTypeString();

    switch (theType) {
    case OTLedger::nymbox:
        pszFolder = OTFolders::Nymbox().Get();
        break;
    case OTLedger::inbox:
        pszFolder = OTFolders::Inbox().Get();
        break;
    case OTLedger::outbox:
        pszFolder = OTFolders::Outbox().Get();
        break;
    case OTLedger::paymentInbox:
        pszFolder = OTFolders::PaymentInbox().Get();
        break;
    case OTLedger::recordBox:
        pszFolder = OTFolders::RecordBox().Get();
        break;
    case OTLedger::expiredBox:
        pszFolder = OTFolders::ExpiredBox().Get();
        break;
    /* --- BREAK --- */
    default:
        otErr << "OTLedger::SaveGeneric: Error: unknown box type. (This should "
                 "never happen.)\n";
        return false;
    }

    m_strFoldername = pszFolder; // <=======

    OTString strID;
    GetIdentifier(strID);
    const OTString strServerID(GetRealServerID());

    if (!m_strFilename.Exists())
        m_strFilename.Format("%s%s%s", strServerID.Get(),
                             OTLog::PathSeparator(), strID.Get());

    OTString strFilename;
    strFilename = strID.Get();

    const char* szFolder1name =
        m_strFoldername.Get(); // "nymbox" (or "inbox" or "outbox")
    const char* szFolder2name = strServerID.Get(); // "nymbox/SERVER_ID"
    const char* szFilename = strFilename.Get();    // "nymbox/SERVER_ID/USER_ID"
    // (or "inbox/SERVER_ID/ACCT_ID"
    // or
    // "outbox/SERVER_ID/ACCT_ID")

    OT_ASSERT(m_strFoldername.GetLength() > 2);
    OT_ASSERT(m_strFilename.GetLength() > 2);

    OTString strRawFile;

    if (!SaveContractRaw(strRawFile)) {
        otErr << "OTLedger::SaveGeneric: Error saving " << pszType
              << " (to string):\n" << szFolder1name << OTLog::PathSeparator()
              << szFolder2name << OTLog::PathSeparator() << szFilename << "\n";
        return false;
    }

    OTString strFinal;
    OTASCIIArmor ascTemp(strRawFile);

    if (false ==
        ascTemp.WriteArmoredString(strFinal, m_strContractType.Get())) {
        otErr << "OTLedger::SaveGeneric: Error saving " << pszType
              << " (failed writing armored string):\n" << szFolder1name
              << OTLog::PathSeparator() << szFolder2name
              << OTLog::PathSeparator() << szFilename << "\n";
        return false;
    }

    bool bSaved =
        OTDB::StorePlainString(strFinal.Get(), szFolder1name, szFolder2name,
                               szFilename); // <=== SAVING TO DATA STORE.
    if (!bSaved) {
        otErr << "OTLedger::SaveGeneric: Error writing " << pszType
              << " to file: " << szFolder1name << OTLog::PathSeparator()
              << szFolder2name << OTLog::PathSeparator() << szFilename << "\n";
        return false;
    }
    else
        otInfo << "Successfully saved " << pszType << ": " << szFolder1name
               << OTLog::PathSeparator() << szFolder2name
               << OTLog::PathSeparator() << szFilename << "\n";

    return bSaved;
}

// If you know you have an inbox, outbox, or nymbox, then call
// CalculateInboxHash,
// CalculateOutboxHash, or CalculateNymboxHash. Otherwise, if in doubt, call
// this.
// It's more generic but warning: performs less verification.
//
bool OTLedger::CalculateHash(OTIdentifier& theOutput)
{
    theOutput.clear();

    bool bCalcDigest = theOutput.CalculateDigest(m_xmlUnsigned);
    if (!bCalcDigest) {
        theOutput.clear();
        otErr << "OTLedger::CalculateHash: Failed trying to calculate hash "
                 "(for a " << GetTypeString() << ")\n";
    }

    return bCalcDigest;
}

bool OTLedger::CalculateInboxHash(OTIdentifier& theOutput)
{
    if (m_Type != OTLedger::inbox) {
        otErr << "Wrong ledger type passed to OTLedger::CalculateInboxHash.\n";
        return false;
    }

    return CalculateHash(theOutput);
}

bool OTLedger::CalculateOutboxHash(OTIdentifier& theOutput)
{
    if (m_Type != OTLedger::outbox) {
        otErr << "Wrong ledger type passed to OTLedger::CalculateOutboxHash.\n";
        return false;
    }

    return CalculateHash(theOutput);
}

bool OTLedger::CalculateNymboxHash(OTIdentifier& theOutput)
{
    if (m_Type != OTLedger::nymbox) {
        otErr << "Wrong ledger type passed to OTLedger::CalculateNymboxHash.\n";
        return false;
    }

    return CalculateHash(theOutput);
}

// If you're going to save this, make sure you sign it first.
bool OTLedger::SaveNymbox(OTIdentifier* pNymboxHash) // If you pass
                                                     // the identifier
                                                     // in, the hash
                                                     // is recorded
                                                     // there.
{
    if (m_Type != OTLedger::nymbox) {
        otErr << "Wrong ledger type passed to OTLedger::SaveNymbox.\n";
        return false;
    }

    const bool bSaved = SaveGeneric(m_Type);

    // Sometimes the caller, when saving the Nymbox, wants to know what the
    // latest Nymbox hash is. FYI, the NymboxHash is calculated on the UNSIGNED
    // contents of the Nymbox. So if pNymboxHash is not nullptr, then that is
    // where
    // I will put the new hash, as output for the caller of this function.
    //
    if (bSaved && (nullptr != pNymboxHash)) {
        pNymboxHash->clear();

        if (!CalculateNymboxHash(*pNymboxHash))
            otErr << "OTLedger::SaveNymbox: Failed trying to calculate nymbox "
                     "hash.\n";
        //
        //        if (!pNymboxHash->CalculateDigest(m_xmlUnsigned))
        //            otErr << "OTLedger::SaveNymbox: Failed trying to calculate
        // nymbox hash.\n";
    }

    return bSaved;
}

// If you're going to save this, make sure you sign it first.
bool OTLedger::SaveInbox(OTIdentifier* pInboxHash) // If you pass the
                                                   // identifier in,
                                                   // the hash is
                                                   // recorded there.
{

    //    OTString strTempBlah, strTempBlah2(*this);
    //    GetIdentifier(strTempBlah);
    //    otErr << "OTLedger::SaveInbox: DEBUGGING: SAVING INBOX for account:
    // %s, number of transactions currently in the inbox: %d \n\n STACK
    // TRACE:\n\n",
    //                  strTempBlah.Get(), GetTransactionCount());
    //
    //    print_stacktrace();

    //    otErr << "OTLedger::SaveInbox: (CONTINUED DEBUGGING):  INBOX CONTENTS:
    // \n\n%s\n\n",
    //                  strTempBlah2.Get());

    if (m_Type != OTLedger::inbox) {
        otErr << "Wrong ledger type passed to OTLedger::SaveInbox.\n";
        return false;
    }

    const bool bSaved = SaveGeneric(m_Type);

    // Sometimes the caller, when saving the Inbox, wants to know what the
    // latest Inbox hash is. FYI, the InboxHash is calculated on the UNSIGNED
    // contents of the Inbox. So if pInboxHash is not nullptr, then that is
    // where
    // I will put the new hash, as output for the caller of this function.
    //
    if (bSaved && (nullptr != pInboxHash)) {
        pInboxHash->clear();

        if (!CalculateInboxHash(*pInboxHash))
            //      if (!pInboxHash->CalculateDigest(m_xmlUnsigned))
            otErr << "OTLedger::SaveInbox: Failed trying to calculate Inbox "
                     "hash.\n";
    }

    return bSaved;
}

// If you're going to save this, make sure you sign it first.
bool OTLedger::SaveOutbox(OTIdentifier* pOutboxHash) // If you pass
                                                     // the identifier
                                                     // in, the hash
                                                     // is recorded
                                                     // there.
{
    if (m_Type != OTLedger::outbox) {
        otErr << "Wrong ledger type passed to OTLedger::SaveOutbox.\n";
        return false;
    }

    const bool bSaved = SaveGeneric(m_Type);

    // Sometimes the caller, when saving the Outbox, wants to know what the
    // latest Outbox hash is. FYI, the OutboxHash is calculated on the UNSIGNED
    // contents of the Outbox. So if pOutboxHash is not nullptr, then that is
    // where
    // I will put the new hash, as output for the caller of this function.
    //
    if (bSaved && (nullptr != pOutboxHash)) {
        pOutboxHash->clear();

        if (!CalculateOutboxHash(*pOutboxHash))
            //      if (!pOutboxHash->CalculateDigest(m_xmlUnsigned))
            otErr << "OTLedger::SaveOutbox: Failed trying to calculate Outbox "
                     "hash.\n";
    }

    return bSaved;
}

// If you're going to save this, make sure you sign it first.
bool OTLedger::SavePaymentInbox()
{
    if (m_Type != OTLedger::paymentInbox) {
        otErr << "Wrong ledger type passed to OTLedger::SavePaymentInbox.\n";
        return false;
    }

    return SaveGeneric(m_Type);
}

// If you're going to save this, make sure you sign it first.
bool OTLedger::SaveRecordBox()
{
    if (m_Type != OTLedger::recordBox) {
        otErr << "Wrong ledger type passed to OTLedger::SaveRecordBox.\n";
        return false;
    }

    return SaveGeneric(m_Type);
}

// If you're going to save this, make sure you sign it first.
bool OTLedger::SaveExpiredBox()
{
    if (m_Type != OTLedger::expiredBox) {
        otErr << "Wrong ledger type passed to OTLedger::SaveExpiredBox.\n";
        return false;
    }

    return SaveGeneric(m_Type);
}

// static
OTLedger* OTLedger::GenerateLedger(
    const OTIdentifier& theUserID,
    const OTIdentifier& theAcctID, // AcctID should be "OwnerID" since could be
                                   // acct OR Nym (with nymbox)
    const OTIdentifier& theServerID, ledgerType theType, bool bCreateFile)
{
    OTLedger* pLedger = new OTLedger(theUserID, theAcctID, theServerID);
    OT_ASSERT(nullptr != pLedger);

    pLedger->GenerateLedger(theAcctID, theServerID, theType, bCreateFile);
    pLedger->SetUserID(theUserID);

    return pLedger;
}

bool OTLedger::GenerateLedger(const OTIdentifier& theAcctID,
                              const OTIdentifier& theServerID,
                              ledgerType theType, bool bCreateFile)
{
    // First we set the "Safe" ID and try to load the file, to make sure it
    // doesn't already exist.
    OTString strID(theAcctID), strServerID(theServerID);

    switch (theType) {
    case OTLedger::nymbox: // stored by NymID ONLY.
        m_strFoldername = OTFolders::Nymbox().Get();
        m_strFilename.Format("%s%s%s", strServerID.Get(),
                             OTLog::PathSeparator(), strID.Get());
        break;
    case OTLedger::inbox: // stored by AcctID ONLY.
        m_strFoldername = OTFolders::Inbox().Get();
        m_strFilename.Format("%s%s%s", strServerID.Get(),
                             OTLog::PathSeparator(), strID.Get());
        break;
    case OTLedger::outbox: // stored by AcctID ONLY.
        m_strFoldername = OTFolders::Outbox().Get();
        m_strFilename.Format("%s%s%s", strServerID.Get(),
                             OTLog::PathSeparator(), strID.Get());
        break;
    case OTLedger::paymentInbox: // stored by NymID ONLY.
        m_strFoldername = OTFolders::PaymentInbox().Get();
        m_strFilename.Format("%s%s%s", strServerID.Get(),
                             OTLog::PathSeparator(), strID.Get());
        break;
    case OTLedger::recordBox: // stored by Acct ID *and* Nym ID (depending on
                              // the box.)
        m_strFoldername = OTFolders::RecordBox().Get();
        m_strFilename.Format("%s%s%s", strServerID.Get(),
                             OTLog::PathSeparator(), strID.Get());
        break;
    case OTLedger::expiredBox: // stored by Nym ID only.
        m_strFoldername = OTFolders::ExpiredBox().Get();
        m_strFilename.Format("%s%s%s", strServerID.Get(),
                             OTLog::PathSeparator(), strID.Get());
        break;
    case OTLedger::message:
        otLog4 << "Generating message ledger...\n";
        SetRealAccountID(theAcctID);
        SetPurportedAccountID(theAcctID); // It's safe to set these the same
                                          // here, since we're creating the
                                          // ledger now.
        SetRealServerID(theServerID);
        SetPurportedServerID(theServerID); // Always want the server ID on
                                           // anything that the server signs.
        m_Type = theType;
        return true;
    default:
        OT_FAIL_MSG("OTLedger::GenerateLedger: GenerateLedger is only for "
                    "message, nymbox, inbox, outbox, and paymentInbox "
                    "ledgers.\n");
    }

    m_Type = theType; // Todo make this Get/Set methods

    SetRealAccountID(theAcctID);  // set this before calling LoadContract... (In
                                  // this case, will just be the Nym ID as
                                  // well...)
    SetRealServerID(theServerID); // (Ledgers/transactions/items were originally
                                  // meant just for account-related functions.)

    if (bCreateFile) {

        OTString strFilename;
        strFilename = strID.Get();

        const char* szFolder1name =
            m_strFoldername.Get(); // "nymbox" (or "inbox" or "outbox")
        const char* szFolder2name = strServerID.Get(); // "nymbox/SERVER_ID"
        const char* szFilename =
            strFilename.Get(); // "nymbox/SERVER_ID/USER_ID"  (or
                               // "inbox/SERVER_ID/ACCT_ID" or
                               // "outbox/SERVER_ID/ACCT_ID")

        if (OTDB::Exists(szFolder1name, szFolder2name, szFilename)) {
            otOut << "ERROR: trying to generate ledger that already exists: "
                  << szFolder1name << OTLog::PathSeparator() << szFolder2name
                  << OTLog::PathSeparator() << szFilename << "\n";
            return false;
        }

        // Okay, it doesn't already exist. Let's generate it.
        otOut << "Generating " << szFolder1name << OTLog::PathSeparator()
              << szFolder2name << OTLog::PathSeparator() << szFilename << "\n";
    }

    if ((OTLedger::inbox == theType) || (OTLedger::outbox == theType)) {
        // Have to look up the UserID here. No way around it. We need that ID.
        // Plus it helps verify things.
        std::unique_ptr<OTAccount> pAccount(
            OTAccount::LoadExistingAccount(theAcctID, theServerID));

        if (nullptr != pAccount)
            SetUserID(pAccount->GetUserID());
        else {
            otErr << "OTLedger::GenerateLedger: Failed in "
                     "OTAccount::LoadExistingAccount().\n";
            return false;
        }
    }
    else if (OTLedger::recordBox == theType) {
        // RecordBox COULD be by NymID OR AcctID.
        // So we TRY to lookup the acct.
        //
        std::unique_ptr<OTAccount> pAccount(
            OTAccount::LoadExistingAccount(theAcctID, theServerID));

        if (nullptr != pAccount) // Found it!
            SetUserID(pAccount->GetUserID());
        else // Must be based on NymID, not AcctID (like Nymbox. But RecordBox
             // can go either way.)
        {
            SetUserID(theAcctID); // In the case of nymbox, and sometimes with
                                  // recordBox, the acct ID IS the user ID.
        }
    }
    else {
        // In the case of paymentInbox, expired box, and nymbox, the acct ID IS
        // the user ID.
        // (Should change it to "owner ID" to make it sound right either way.)
        //
        SetUserID(theAcctID);
    }

    // Notice I still don't actually create the file here.  The programmer still
    // has to call
    // "SaveNymbox", "SaveInbox" or "SaveOutbox" or "SaveRecordBox" or
    // "SavePaymentInbox" to
    // actually save the file. But he cannot do that unless he generates it
    // first here, and
    // the "bCreateFile" parameter insures that he isn't overwriting one that is
    // already there
    // (even if we don't actually save the file in this function.)
    //
    SetPurportedAccountID(theAcctID);
    SetPurportedServerID(theServerID);

    return true;
}

void OTLedger::InitLedger()
{
    m_strContractType =
        "LEDGER"; // CONTRACT, MESSAGE, TRANSACTION, LEDGER, TRANSACTION ITEM

    // This is the default type for a ledger.
    // Inboxes and Outboxes are generated with the right type, with files.
    // Until the GenerateLedger function is called, message is the default type.
    m_Type = OTLedger::message;

    m_bLoadedLegacyData = false;
}

// ID refers to account ID.
// Since a ledger is normally used as an inbox for a specific account, in a
// specific file,
// then I've decided to restrict ledgers to a single account.
OTLedger::OTLedger(const OTIdentifier& theUserID,
                   const OTIdentifier& theAccountID,
                   const OTIdentifier& theServerID)
    : OTTransactionType(theUserID, theAccountID, theServerID)
    , m_Type(OTLedger::message)
    , m_bLoadedLegacyData(false)
{
    InitLedger();
}

// ONLY call this if you need to load a ledger where you don't already know the
// person's UserID
// For example, if you need to load someone ELSE's inbox in order to send them a
// transfer, then
// you only know their account number, not their user ID. So you call this
// function to get it
// loaded up, and the UserID will hopefully be loaded up with the rest of it.
OTLedger::OTLedger(const OTIdentifier& theAccountID,
                   const OTIdentifier& theServerID)
    : OTTransactionType()
    , m_Type(OTLedger::message)
    , m_bLoadedLegacyData(false)
{
    InitLedger();

    SetRealAccountID(theAccountID);
    SetRealServerID(theServerID);
}

// This is private now and hopefully will stay that way.
OTLedger::OTLedger()
    : OTTransactionType()
    , m_Type(OTLedger::message)
    , m_bLoadedLegacyData(false)
{
    InitLedger();
}

const mapOfTransactions& OTLedger::GetTransactionMap() const
{
    return m_mapTransactions;
}

/// If transaction #87, in reference to #74, is in the inbox, you can remove it
/// by calling this function and passing in 87. Deletes.
///
bool OTLedger::RemoveTransaction(int64_t lTransactionNum, bool bDeleteIt)
{
    // See if there's something there with that transaction number.
    auto it = m_mapTransactions.find(lTransactionNum);

    // If it's not already on the list, then there's nothing to remove.
    if (it == m_mapTransactions.end()) {
        otErr << "OTLedger::RemoveTransaction"
              << ": Attempt to remove Transaction from ledger, when "
                 "not already there: " << lTransactionNum << "\n";
        return false;
    }
    // Otherwise, if it WAS already there, remove it properly.
    else {
        OTTransaction* pTransaction = it->second;
        OT_ASSERT(nullptr != pTransaction);
        m_mapTransactions.erase(it);

        if (bDeleteIt) {
            delete pTransaction;
            pTransaction = nullptr;
        }
        return true;
    }
}

bool OTLedger::AddTransaction(OTTransaction& theTransaction)
{
    // See if there's something else already there with the same transaction
    // number.
    auto it = m_mapTransactions.find(theTransaction.GetTransactionNum());

    // If it's not already on the list, then add it...
    if (it == m_mapTransactions.end()) {
        m_mapTransactions[theTransaction.GetTransactionNum()] = &theTransaction;
        theTransaction.SetParent(*this); // for convenience
        return true;
    }
    // Otherwise, if it was already there, log an error.
    else {
        otErr << "Attempt to add Transaction to ledger when already there for "
                 "that number: " << theTransaction.GetTransactionNum() << "\n";
    }

    return false;
}

OTTransaction* OTLedger::GetTransaction(OTTransaction::transactionType theType)
{
    // loop through the items that make up this transaction

    for (auto& it : m_mapTransactions) {
        OTTransaction* pTransaction = it.second;
        OT_ASSERT(nullptr != pTransaction);

        if (theType == pTransaction->GetType()) return pTransaction;
    }

    return nullptr;
}

// if not found, returns -1
int32_t OTLedger::GetTransactionIndex(int64_t lTransactionNum)
{
    // loop through the transactions inside this ledger
    // If a specific transaction is found, returns its index inside the ledger
    //
    int32_t nIndex = -1;

    for (auto& it : m_mapTransactions) {
        OTTransaction* pTransaction = it.second;
        OT_ASSERT(nullptr != pTransaction);

        ++nIndex; // 0 on first iteration.

        if (pTransaction->GetTransactionNum() == lTransactionNum) {
            return nIndex;
        }
    }
    return -1;
}

// Look up a transaction by transaction number and see if it is in the ledger.
// If it is, return a pointer to it, otherwise return nullptr.
OTTransaction* OTLedger::GetTransaction(int64_t lTransactionNum) const
{
    // loop through the transactions inside this ledger

    for (auto& it : m_mapTransactions) {
        OTTransaction* pTransaction = it.second;
        OT_ASSERT(nullptr != pTransaction);

        if (pTransaction->GetTransactionNum() == lTransactionNum) {
            return pTransaction;
        }
    }
    return nullptr;
}

// Return a count of all the transactions in this ledger that are IN REFERENCE
// TO a specific trans#.
//
// Might want to change this so that it only counts ACCEPTED receipts.
//
int32_t OTLedger::GetTransactionCountInRefTo(int64_t lReferenceNum) const
{
    int32_t nCount = 0;

    for (auto& it : m_mapTransactions) {
        OTTransaction* pTransaction = it.second;
        OT_ASSERT(nullptr != pTransaction);

        if (pTransaction->GetReferenceToNum() == lReferenceNum) nCount++;
    }

    return nCount;
}

// Look up a transaction by transaction number and see if it is in the ledger.
// If it is, return a pointer to it, otherwise return nullptr.
OTTransaction* OTLedger::GetTransactionByIndex(int32_t nIndex) const
{
    // Out of bounds.
    if ((nIndex < 0) || (nIndex >= GetTransactionCount())) return nullptr;

    int32_t nIndexCount = -1;

    for (auto& it : m_mapTransactions) {
        nIndexCount++; // On first iteration, this is now 0, same as nIndex.
        OTTransaction* pTransaction = it.second;
        OT_ASSERT((nullptr != pTransaction)); // Should always be good.

        // If this transaction is the one at the requested index
        if (nIndexCount == nIndex) return pTransaction;
    }

    return nullptr; // Should never reach this point, since bounds are checked
                    // at
                    // the top.
}

// Nymbox-only.
// Looks up replyNotice by REQUEST NUMBER.
//
OTTransaction* OTLedger::GetReplyNotice(const int64_t& lRequestNum)
{
    // loop through the transactions that make up this ledger.
    for (auto& it : m_mapTransactions) {
        OTTransaction* pTransaction = it.second;
        OT_ASSERT(nullptr != pTransaction);

        if (OTTransaction::replyNotice != pTransaction->GetType()) // <=======
            continue;

        if (pTransaction->GetRequestNum() == lRequestNum) return pTransaction;
    }

    return nullptr;
}

OTTransaction* OTLedger::GetTransferReceipt(int64_t lNumberOfOrigin)
{
    // loop through the transactions that make up this ledger.
    for (auto& it : m_mapTransactions) {
        OTTransaction* pTransaction = it.second;
        OT_ASSERT(nullptr != pTransaction);

        if (OTTransaction::transferReceipt == pTransaction->GetType()) {
            OTString strReference;
            pTransaction->GetReferenceString(strReference);

            std::unique_ptr<OTItem> pOriginalItem(OTItem::CreateItemFromString(
                strReference, pTransaction->GetPurportedServerID(),
                pTransaction->GetReferenceToNum()));
            OT_ASSERT(nullptr != pOriginalItem);

            if (pOriginalItem->GetType() != OTItem::acceptPending) {
                otErr << "OTLedger::" << __FUNCTION__
                      << ": Wrong item type attached to transferReceipt!\n";
                return nullptr;
            }
            else {
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
                    return pTransaction; // FOUND IT!
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
OTTransaction* OTLedger::GetChequeReceipt(int64_t lChequeNum,
                                          OTCheque** ppChequeOut) // CALLER
                                                                  // RESPONSIBLE
                                                                  // TO DELETE.
{
    for (auto& it : m_mapTransactions) {
        OTTransaction* pCurrentReceipt = it.second;
        OT_ASSERT(nullptr != pCurrentReceipt);

        if ((pCurrentReceipt->GetType() != OTTransaction::chequeReceipt) &&
            (pCurrentReceipt->GetType() != OTTransaction::voucherReceipt))
            continue;

        OTString strDepositChequeMsg;
        pCurrentReceipt->GetReferenceString(strDepositChequeMsg);

        std::unique_ptr<OTItem> pOriginalItem(OTItem::CreateItemFromString(
            strDepositChequeMsg, GetPurportedServerID(),
            pCurrentReceipt->GetReferenceToNum()));

        if (nullptr == pOriginalItem) {
            otErr << __FUNCTION__
                  << ": Expected original depositCheque request item to be "
                     "inside the chequeReceipt "
                     "(but failed to load it...)\n";
        }
        else if (OTItem::depositCheque != pOriginalItem->GetType()) {
            OTString strItemType;
            pOriginalItem->GetTypeString(strItemType);
            otErr << __FUNCTION__
                  << ": Expected original depositCheque request item to be "
                     "inside the chequeReceipt, "
                     "but somehow what we found instead was a " << strItemType
                  << "...\n";
        }
        else {
            // Get the cheque from the Item and load it up into a Cheque object.
            //
            OTString strCheque;
            pOriginalItem->GetAttachment(strCheque);

            OTCheque* pCheque = new OTCheque;
            OT_ASSERT(nullptr != pCheque);
            std::unique_ptr<OTCheque> theChequeAngel(pCheque);

            if (!((strCheque.GetLength() > 2) &&
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
                    // We found it! Let's return a pointer to pCurrentReceipt

                    // Also return pCheque, if the caller wants it (otherwise
                    // delete it.)
                    //
                    if (nullptr != ppChequeOut) // caller wants us to return the
                                                // cheque pointer as well.
                    {
                        (*ppChequeOut) =
                            pCheque; // now caller is responsible to delete.
                        theChequeAngel.release();
                    }

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
OTTransaction* OTLedger::GetFinalReceipt(int64_t lReferenceNum)
{
    // loop through the transactions that make up this ledger.
    for (auto& it : m_mapTransactions) {
        OTTransaction* pTransaction = it.second;
        OT_ASSERT(nullptr != pTransaction);

        if (OTTransaction::finalReceipt != pTransaction->GetType()) // <=======
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
OTItem* OTLedger::GenerateBalanceStatement(int64_t lAdjustment,
                                           const OTTransaction& theOwner,
                                           OTPseudonym& theNym,
                                           const OTAccount& theAccount,
                                           OTLedger& theOutbox)
{
    if (OTLedger::inbox != GetType()) {
        otErr << "OTLedger::GenerateBalanceStatement: Wrong ledger type.\n";
        return nullptr;
    }

    const OTIdentifier theNymID(theNym);

    if ((theAccount.GetPurportedAccountID() != GetPurportedAccountID()) ||
        (theAccount.GetPurportedServerID() != GetPurportedServerID()) ||
        (theAccount.GetUserID() != GetUserID())) {
        otErr << "Wrong Account passed in to "
                 "OTLedger::GenerateBalanceStatement.\n";
        return nullptr;
    }
    if ((theOutbox.GetPurportedAccountID() != GetPurportedAccountID()) ||
        (theOutbox.GetPurportedServerID() != GetPurportedServerID()) ||
        (theOutbox.GetUserID() != GetUserID())) {
        otErr << "Wrong Outbox passed in to "
                 "OTLedger::GenerateBalanceStatement.\n";
        return nullptr;
    }
    if ((theNymID != GetUserID())) {
        otErr << "Wrong Nym passed in to OTLedger::GenerateBalanceStatement.\n";
        return nullptr;
    }

    // theOwner is the withdrawal, or deposit, or whatever, that wants to change
    // the account balance, and thus that needs a new balance agreement signed.
    //
    OTItem* pBalanceItem = OTItem::CreateItemFromTransaction(
        theOwner, OTItem::balanceStatement); // <=== balanceStatement type, with
                                             // user ID, server ID, account ID,
                                             // transaction ID.

    // The above has an ASSERT, so this this will never actually happen.
    if (nullptr == pBalanceItem) return nullptr;

    // COPY THE ISSUED TRANSACTION NUMBERS FROM THE NYM to the MESSAGE NYM.

    OTPseudonym theMessageNym;

    theMessageNym.HarvestIssuedNumbers(
        GetPurportedServerID(),
        theNym /*unused in this case, not saving to disk*/, theNym,
        false); // bSave = false;

    switch (theOwner.GetType()) {
    // These six options will remove the transaction number from the issued
    // list, SUCCESS OR FAIL.
    // Server will expect the number to be missing from the list, in the case of
    // these.
    // Therefore I remove it here in order to generate a proper balance
    // agreement, acceptable to the server.
    case OTTransaction::processInbox:
    case OTTransaction::withdrawal:
    case OTTransaction::deposit:
    case OTTransaction::cancelCronItem:
    case OTTransaction::exchangeBasket:
    case OTTransaction::payDividend:

        theMessageNym.RemoveIssuedNum(
            theOwner.GetRealServerID(),
            theOwner.GetTransactionNum()); // a transaction number is being
                                           // used, and REMOVED from my list of
                                           // responsibility,
        theMessageNym.RemoveTransactionNum(
            theOwner.GetRealServerID(),
            theOwner.GetTransactionNum()); // a transaction number is being
                                           // used, and REMOVED from my list of
                                           // available numbers.
        break;

    case OTTransaction::transfer:
    case OTTransaction::marketOffer:
    case OTTransaction::paymentPlan:
    case OTTransaction::smartContract:
        // Nothing removed here since the transaction is still in play.
        // (Assuming success.)
        // If the server replies with rejection for any of these three, then I
        // can remove
        // the transaction number from my list of issued/signed for. But if
        // success, then I
        // am responsible for the transaction number until I sign off on closing
        // it.
        // Since the Balance Statement ANTICIPATES SUCCESS, NOT FAILURE, it
        // assumes the number
        // to be "in play" here, and thus DOES NOT remove it (vs the cases
        // above, which do.)
        break;
    default:
        // Error
        otErr << "OTLedger::" << __FUNCTION__
              << ": wrong owner transaction type: " << theOwner.GetTypeString()
              << "\n";
        break;
    }

    OTString strMessageNym(theMessageNym); // Okay now we have the transaction
                                           // numbers in this MessageNym string.

    pBalanceItem->SetAttachment(strMessageNym); // <======== This is where the
                                                // server will read the
                                                // transaction numbers from (A
                                                // nym in item.m_ascAttachment)

    int64_t lCurrentBalance = theAccount.GetBalance();

    pBalanceItem->SetAmount(lCurrentBalance + lAdjustment); // <==== Here's the
                                                            // new (predicted)
                                                            // balance for after
                                                            // the transaction
                                                            // is complete.
                                                            // (item.GetAmount)

    // loop through the INBOX transactions, and produce a sub-item onto
    // pBalanceItem for each, which will
    // be a report on each transaction in this inbox, therefore added to the
    // balance item.
    // (So the balance item contains a complete report on the receipts in this
    // inbox.)

    otInfo << "About to loop through the inbox items and produce a report for "
              "each one...\n";

    for (auto& it : m_mapTransactions) {
        OTTransaction* pTransaction = it.second;
        OT_ASSERT(nullptr != pTransaction);

        otInfo << "Producing a report...\n";

        // it only reports receipts where we don't yet have balance agreement.
        //      pTransaction->ProduceInboxReportItem(*pBalanceItem,
        // const_cast<OTTransaction&>(theOwner));
        pTransaction->ProduceInboxReportItem(
            *pBalanceItem); // <======= This function adds a receipt sub-item to
                            // pBalanceItem, where appropriate for INBOX items.
    }

    theOutbox.ProduceOutboxReport(*pBalanceItem); // <======= This function adds
                                                  // receipt sub-items to
                                                  // pBalanceItem, where
                                                  // appropriate for the OUTBOX
                                                  // items.

    pBalanceItem->SignContract(theNym); // <=== Sign, save, and return.
                                        // OTTransactionType needs to weasel in
                                        // a "date signed" variable.
    pBalanceItem->SaveContract();

    return pBalanceItem;
}

// for inbox only, allows you to lookup the total value of pending transfers
// within the inbox.
// (And it really loads the items to check the amount, but does all this ONLY
// for pending transfers.)
//
int64_t OTLedger::GetTotalPendingValue()
{
    int64_t lTotalPendingValue = 0;

    if (OTLedger::inbox != GetType()) {
        otErr << "OTLedger::GetTotalPendingValue: Wrong ledger type (expected "
                 "inbox).\n";
        return 0;
    }

    for (auto& it : m_mapTransactions) {
        OTTransaction* pTransaction = it.second;
        OT_ASSERT(nullptr != pTransaction);

        if (pTransaction->GetType() == OTTransaction::pending)
            lTotalPendingValue +=
                pTransaction->GetReceiptAmount(); // this actually loads up the
                                                  // original item and reads the
                                                  // amount.
    }

    return lTotalPendingValue;
}

// Called by the above function.
// This ledger is an outbox, and it is creating a report of itself,
// adding each report item to this balance item.
// DO NOT call this, it's meant to be used only by above function.
void OTLedger::ProduceOutboxReport(OTItem& theBalanceItem)
{
    if (OTLedger::outbox != GetType()) {
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
        OTTransaction* pTransaction = it.second;
        OT_ASSERT(nullptr != pTransaction);

        // it only reports receipts where we don't yet have balance agreement.
        pTransaction->ProduceOutboxReportItem(
            theBalanceItem); // <======= This function adds a pending transfer
                             // sub-item to theBalanceItem, where appropriate.
    }
}

// Auto-detects ledger type. (message/nymbox/inbox/outbox)
// Use this instead of LoadContractFromString (for ledgers,
// for when you don't know their type already.)
// Otherwise if you know the type, then use LoadNymboxFromString() etc.
//
bool OTLedger::LoadLedgerFromString(const OTString& theStr)
{
    bool bLoaded = false;

    // Todo security: Look how this is done...
    // Any vulnerabilities?
    //
    if (theStr.Contains("\"\n type=\"nymbox\""))
        bLoaded = LoadNymboxFromString(theStr);
    else if (theStr.Contains("\"\n type=\"inbox\""))
        bLoaded = LoadInboxFromString(theStr);
    else if (theStr.Contains("\"\n type=\"outbox\""))
        bLoaded = LoadOutboxFromString(theStr);
    else if (theStr.Contains("\"\n type=\"paymentInbox\""))
        bLoaded = LoadPaymentInboxFromString(theStr);
    else if (theStr.Contains("\"\n type=\"recordBox\""))
        bLoaded = LoadRecordBoxFromString(theStr);
    else if (theStr.Contains("\"\n type=\"expiredBox\""))
        bLoaded = LoadExpiredBoxFromString(theStr);
    else if (theStr.Contains("\"\n type=\"message\"")) {
        m_Type = OTLedger::message;
        bLoaded = LoadContractFromString(theStr);
    }
    return bLoaded;
}

// SignContract will call this function at the right time.
void OTLedger::UpdateContents() // Before transmission or serialization, this is
                                // where the ledger saves its contents
{
    bool bSavingAbbreviated =
        true; // The default. Only OTLedger::message changes this to false.
    int32_t nPartialRecordCount = 0; // We store this, so we know how many
                                     // abbreviated records to read back later.

    switch (GetType()) {
    // a message ledger stores the full receipts directly inside itself. (No
    // separate files.)
    case OTLedger::message:
        bSavingAbbreviated = false;
        nPartialRecordCount = 0; // In this case (OTLedger::message), they are
                                 // all FULL records, not abbreviated.
        break;

    // These store abbreviated versions of themselves, with the actual receipts
    // in separate files.
    // Those separate files are created on server side when first added to the
    // box, and on client
    // side when downloaded from the server. They must match the hash that
    // appears in the box.
    case OTLedger::nymbox:
    case OTLedger::inbox:
    case OTLedger::outbox:
    case OTLedger::paymentInbox:
    case OTLedger::recordBox:
    case OTLedger::expiredBox:
        bSavingAbbreviated = true;
        nPartialRecordCount = static_cast<int32_t>(
            m_mapTransactions.size()); // We store this, so we know how many
                                       // abbreviated records to read back
                                       // later.
        break;

    default:
        otErr << "OTLedger::UpdateContents: Error: unexpected box type (1st "
                 "block). (This should never happen.)\n";
        return;
    }

    //
    // Notice I use the PURPORTED Account ID and Server ID to create the output.
    // That's because
    // I don't want to inadvertantly substitute the real ID for a bad one and
    // then sign it.
    // So if there's a bad one in there when I read it, THAT's the one that I
    // write as well!
    //
    OTString strType(GetTypeString()), strLedgerAcctID(GetPurportedAccountID()),
        strLedgerAcctServerID(GetPurportedServerID()), strUserID(GetUserID());

    // I release this because I'm about to repopulate it.
    m_xmlUnsigned.Release();

    //    m_xmlUnsigned.Concatenate("<?xml version=\"%s\"?>\n\n", "1.0");

    OTString strLedgerContents = "";

    int32_t nPartialACTUALCount = 0;

    // loop through the transactions and print them out here.
    for (auto& it : m_mapTransactions) {
        OTTransaction* pTransaction = it.second;
        OT_ASSERT(nullptr != pTransaction);

        OTString strTransaction;

        if (false ==
            bSavingAbbreviated) // only OTLedger::message uses this block.
        { // Save the FULL version of the receipt inside the box, so no separate
            // files are necessary.
            //
            pTransaction->SaveContractRaw(strTransaction);
            OTASCIIArmor ascTransaction;
            ascTransaction.SetString(strTransaction, true); // linebreaks = true
            strLedgerContents.Concatenate("<transaction>\n%s</transaction>\n\n",
                                          ascTransaction.Get());
        }
        else // true == bSavingAbbreviated    // ALL OTHER ledger types are
               // saved here in abbreviated form.
        {
            switch (GetType()) {

            case OTLedger::nymbox:
                pTransaction->SaveAbbreviatedNymboxRecord(strTransaction);
                break;
            case OTLedger::inbox:
                pTransaction->SaveAbbreviatedInboxRecord(strTransaction);
                break;
            case OTLedger::outbox:
                pTransaction->SaveAbbreviatedOutboxRecord(strTransaction);
                break;
            case OTLedger::paymentInbox:
                pTransaction->SaveAbbrevPaymentInboxRecord(strTransaction);
                break;
            case OTLedger::recordBox:
                pTransaction->SaveAbbrevRecordBoxRecord(strTransaction);
                break;
            case OTLedger::expiredBox:
                pTransaction->SaveAbbrevExpiredBoxRecord(strTransaction);
                break;

            default: // todo: possibly change this to an OT_ASSERT. security.
                otErr << "OTLedger::UpdateContents: Error: unexpected box type "
                         "(2nd block). (This should never happen. Skipping.)\n";

                OT_FAIL_MSG("ASSERT: OTLedger::UpdateContents: Unexpected "
                            "ledger type.");

                continue;
            }

            nPartialACTUALCount++; // Just in case the number actually written
                                   // matches the count in the map.

            strLedgerContents.Concatenate("%s", strTransaction.Get());
        }
    }

    OT_ASSERT_MSG(nPartialACTUALCount == nPartialRecordCount,
                  "ASSERT: OTLedger::UpdateContents: "
                  "OT_ASSERT_MSG(nPartialACTUALCount == nPartialRecordCount)");

    // HERE IS WHERE WE ACTUALLY BUILD THE STRING:
    //
    m_xmlUnsigned.Concatenate("<accountLedger version=\"%s\"\n "
                              "type=\"%s\"\n "
                              "numPartialRecords=\"%d\"\n "
                              "accountID=\"%s\"\n "
                              "userID=\"%s\"\n "
                              "serverID=\"%s\" >\n\n",
                              m_strVersion.Get(), strType.Get(),
                              nPartialRecordCount, strLedgerAcctID.Get(),
                              strUserID.Get(), strLedgerAcctServerID.Get());

    m_xmlUnsigned.Concatenate("%s", strLedgerContents.Get());
    m_xmlUnsigned.Concatenate("</accountLedger>\n");
}

// LoadContract will call this function at the right time.
// return -1 if error, 0 if nothing, and 1 if the node was processed.
int32_t OTLedger::ProcessXMLNode(irr::io::IrrXMLReader*& xml)
{
    const char* szFunc = "OTLedger::ProcessXMLNode";

    const OTString strNodeName = xml->getNodeName();

    if (strNodeName.Compare("accountLedger")) {
        OTString strType,                    // ledger type
            strLedgerAcctID,                 // purported
            strLedgerAcctServerID,           // purported
            strUserID, strNumPartialRecords; // Ledger contains either full
                                             // receipts, or abbreviated
                                             // receipts with hashes and partial
                                             // data.

        strType = xml->getAttributeValue("type");
        m_strVersion = xml->getAttributeValue("version");

        if (strType.Compare("message")) // These are used for sending
                                        // transactions in messages. (Withdrawal
                                        // request, etc.)
            m_Type = OTLedger::message;
        else if (strType.Compare("nymbox")) // Used for receiving new
                                            // transaction numbers, and for
                                            // receiving notices.
            m_Type = OTLedger::nymbox;
        else if (strType.Compare("inbox")) // These are used for storing the
                                           // receipts in your inbox. (That
                                           // server must store until
                                           // signed-off.)
            m_Type = OTLedger::inbox;
        else if (strType.Compare("outbox")) // Outgoing, pending transfers.
            m_Type = OTLedger::outbox;
        else if (strType.Compare("paymentInbox")) // Receiving invoices, etc.
            m_Type = OTLedger::paymentInbox;
        else if (strType.Compare("recordBox")) // Where receipts go to die
                                               // (awaiting user deletion,
                                               // completed from other boxes
                                               // already.)
            m_Type = OTLedger::recordBox;
        else if (strType.Compare("expiredBox")) // Where expired payments go to
                                                // die (awaiting user deletion,
                                                // completed from other boxes
                                                // already.)
            m_Type = OTLedger::expiredBox;
        else
            m_Type = OTLedger::error_state; // Danger, Will Robinson.

        strLedgerAcctID = xml->getAttributeValue("accountID");
        strLedgerAcctServerID = xml->getAttributeValue("serverID");
        strUserID = xml->getAttributeValue("userID");

        if (!strLedgerAcctID.Exists() || !strLedgerAcctServerID.Exists() ||
            !strUserID.Exists()) {
            otOut << szFunc << ": Failure: missing strLedgerAcctID ("
                  << strLedgerAcctID << ") or "
                                        "strLedgerAcctServerID ("
                  << strLedgerAcctServerID << ") or strUserID (" << strUserID
                  << ") while loading transaction "
                     "from " << strType << " ledger. \n";
            return (-1);
        }

        OTIdentifier ACCOUNT_ID(strLedgerAcctID),
            SERVER_ID(strLedgerAcctServerID), USER_ID(strUserID);

        SetPurportedAccountID(ACCOUNT_ID);
        SetPurportedServerID(SERVER_ID);
        SetUserID(USER_ID);

        if (!m_bLoadSecurely) {
            SetRealAccountID(ACCOUNT_ID);
            SetRealServerID(SERVER_ID);
            //            OTString str1(GetRealAccountID()),
            // str2(GetRealServerID());
            //            otErr << "DEBUGGING:\nReal Acct ID: %s\nReal Server
            // ID: %s\n",
            //                          str1.Get(), str2.Get());
        }

        // Load up the partial records, based on the expected count...
        //
        strNumPartialRecords = xml->getAttributeValue("numPartialRecords");
        int32_t nPartialRecordCount =
            (strNumPartialRecords.Exists() ? atoi(strNumPartialRecords.Get())
                                           : 0);

        OTString strExpected; // The record type has a different name for each
                              // box.
        OTNumList theNumList;
        OTNumList* pNumList = nullptr;
        switch (m_Type) {
        case OTLedger::nymbox:
            strExpected.Set("nymboxRecord");
            pNumList = &theNumList;
            break;
        case OTLedger::inbox:
            strExpected.Set("inboxRecord");
            break;
        case OTLedger::outbox:
            strExpected.Set("outboxRecord");
            break;
        case OTLedger::paymentInbox:
            strExpected.Set("paymentInboxRecord");
            break;
        case OTLedger::recordBox:
            strExpected.Set("recordBoxRecord");
            break;
        case OTLedger::expiredBox:
            strExpected.Set("expiredBoxRecord");
            break;
        /* --- BREAK --- */
        case OTLedger::message:
            if (nPartialRecordCount > 0) {
                otErr << szFunc << ": Error: There are " << nPartialRecordCount
                      << " unexpected abbreviated records in an "
                         "OTLedger::message type ledger. (Failed loading "
                         "ledger with accountID: " << strLedgerAcctID << ")\n";
                return (-1);
            }

            break;
        default:
            otErr << "OTLedger::ProcessXMLNode: Unexpected ledger type ("
                  << strType << "). (Failed loading "
                                "ledger for account: " << strLedgerAcctID
                  << ")\n";
            return (-1);
        } // switch (to set strExpected to the abbreviated record type.)

        if (nPartialRecordCount > 0) // message ledger will never enter this
                                     // block due to switch block (above.)
        {

            // We iterate to read the expected number of partial records from
            // the xml.
            // (They had better be there...)
            //
            while (nPartialRecordCount-- > 0) {
                //                xml->read(); // <==================
                if (!SkipToElement(xml)) {
                    otOut << szFunc << ": Failure: Unable to find element when "
                                       "one was expected (" << strExpected
                          << ") "
                             "for abbreviated record of receipt in "
                          << GetTypeString() << " box:\n\n" << m_strRawFile
                          << "\n\n";
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
                const OTString strLoopNodeName = xml->getNodeName();

                if (strLoopNodeName.Exists() &&
                    (xml->getNodeType() == irr::io::EXN_ELEMENT) &&
                    (strExpected.Compare(strLoopNodeName))) {
                    int64_t lNumberOfOrigin = 0;
                    int64_t lTransactionNum = 0;
                    int64_t lInRefTo = 0;
                    int64_t lInRefDisplay = 0;

                    time64_t the_DATE_SIGNED = OT_TIME_ZERO;
                    int theType = OTTransaction::error_state; // default
                    OTString strHash;

                    int64_t lAdjustment = 0;
                    int64_t lDisplayValue = 0;
                    int64_t lClosingNum = 0;
                    int64_t lRequestNum = 0;
                    bool bReplyTransSuccess = false;

                    int32_t nAbbrevRetVal = LoadAbbreviatedRecord(
                        xml, lNumberOfOrigin, lTransactionNum, lInRefTo,
                        lInRefDisplay, the_DATE_SIGNED, theType, strHash,
                        lAdjustment, lDisplayValue, lClosingNum, lRequestNum,
                        bReplyTransSuccess,
                        pNumList); // This is for "OTTransaction::blank" and
                                   // "OTTransaction::successNotice",
                                   // otherwise nullptr.
                    if ((-1) == nAbbrevRetVal)
                        return (-1); // The function already logs appropriately.

                    //
                    // See if the same-ID transaction already exists in the
                    // ledger.
                    // (There can only be one.)
                    //
                    OTTransaction* pExistingTrans =
                        GetTransaction(lTransactionNum);
                    if (nullptr != pExistingTrans) // Uh-oh, it's already there!
                    {
                        otOut << szFunc << ": Error loading transaction "
                              << lTransactionNum << " (" << strExpected
                              << "), since one was already there, in box for "
                                 "account: " << strLedgerAcctID << ".\n";
                        return (-1);
                    }

                    // CONSTRUCT THE ABBREVIATED RECEIPT HERE...

                    // Set all the values we just loaded here during actual
                    // construction of transaction
                    // (as abbreviated transaction) i.e. make a special
                    // constructor for abbreviated transactions
                    // which is ONLY used here.
                    //
                    OTTransaction* pTransaction = new OTTransaction(
                        USER_ID, ACCOUNT_ID, SERVER_ID, lNumberOfOrigin,
                        lTransactionNum, lInRefTo, // lInRefTo
                        lInRefDisplay, the_DATE_SIGNED,
                        static_cast<OTTransaction::transactionType>(theType),
                        strHash, lAdjustment, lDisplayValue, lClosingNum,
                        lRequestNum, bReplyTransSuccess,
                        pNumList); // This is for "OTTransaction::blank" and
                                   // "OTTransaction::successNotice", otherwise
                                   // nullptr.
                    OT_ASSERT(nullptr != pTransaction);
                    //
                    // NOTE: For THIS CONSTRUCTOR ONLY, we DO set the purported
                    // AcctID and purported ServerID.
                    // WHY? Normally you set the "real" IDs at construction, and
                    // then set the "purported" IDs
                    // when loading from string. But this constructor (only this
                    // one) is actually used when
                    // loading abbreviated receipts as you load their
                    // inbox/outbox/nymbox.
                    // Abbreviated receipts are not like real transactions,
                    // which have serverID, AcctID, userID,
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
                    // This part normally happens in "GenerateTransaction".
                    // NOTE: Moved to OTTransaction constructor (for
                    // abbreviateds) for now.
                    //
                    //                    pTransaction->SetPurportedAccountID(
                    // GetPurportedAccountID());
                    //                    pTransaction->SetPurportedServerID(
                    // GetPurportedServerID());

                    // Add it to the ledger's list of transactions...
                    //

                    if (pTransaction->VerifyContractID()) {
                        // Add it to the ledger...
                        //
                        m_mapTransactions[pTransaction->GetTransactionNum()] =
                            pTransaction;
                        pTransaction->SetParent(*this);
                        //                      otLog5 << "Loaded abbreviated
                        // transaction and adding to m_mapTransactions in
                        // OTLedger\n");
                    }
                    else {
                        otErr << szFunc << ": ERROR: verifying contract ID on "
                                           "abbreviated transaction "
                              << pTransaction->GetTransactionNum() << "\n";
                        delete pTransaction;
                        pTransaction = nullptr;
                        return (-1);
                    }
                    //                    xml->read(); // <==================
                    // MIGHT need to add "skip after element" here.
                    //
                    // Update: Nope.
                }
                else {
                    otErr << szFunc
                          << ": Expected abbreviated record element.\n";
                    return (-1); // error condition
                }
            } // while
        }     // if (number of partial records > 0)

        otLog4 << szFunc << ": Loading account ledger of type \"" << strType
               << "\", version: " << m_strVersion << "\n";
        //                "accountID: %s\n userID: %s\n serverID:
        // %s\n----------\n",  szFunc,
        //                strLedgerAcctID.Get(), strUserID.Get(),
        // strLedgerAcctServerID.Get()

        // Since we just loaded this stuff, let's verify it.
        // We may have to remove this verification here and do it outside this
        // call.
        // But for now...

        if (VerifyContractID())
            return 1;
        else
            return (-1);
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
    else if (strNodeName.Compare("transaction")) {
        OTString strTransaction;
        OTASCIIArmor ascTransaction;

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
            // Each transaction is initially stored as an OTASCIIArmor string.
            const OTString strLoopNodeData = xml->getNodeData();

            if (strLoopNodeData.Exists())
                ascTransaction.Set(strLoopNodeData); // Put the ascii-armored
                                                     // node data into the
                                                     // ascii-armor object

            // Decode that into strTransaction, so we can load the transaction
            // object from that string.
            if (!ascTransaction.Exists() ||
                !ascTransaction.GetString(strTransaction)) {
                otErr << __FUNCTION__
                      << ": ERROR: Missing expected transaction contents. "
                         "Ledger contents:\n\n" << m_strRawFile << "\n\n";
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
            // OTTransaction(GetUserID(), GetRealAccountID(),
            // GetRealServerID());
            OTTransaction* pTransaction = new OTTransaction(
                GetUserID(), GetPurportedAccountID(), GetPurportedServerID());
            OT_ASSERT(nullptr != pTransaction);

            // Need this set before the LoadContractFromString().
            //
            if (!m_bLoadSecurely) pTransaction->SetLoadInsecure();

            // If we're able to successfully base64-decode the string and load
            // it up as
            // a transaction, then let's add it to the ledger's list of
            // transactions
            if (strTransaction.Exists() &&
                pTransaction->LoadContractFromString(strTransaction) &&
                pTransaction->VerifyContractID())
            // I responsible here to call pTransaction->VerifyContractID()
            // since
            // I am loading it here and adding it to the ledger. (So I do.)
            {

                OTTransaction* pExistingTrans =
                    GetTransaction(pTransaction->GetTransactionNum());
                if (nullptr != pExistingTrans) // Uh-oh, it's already there!
                {
                    const OTString strPurportedAcctID(GetPurportedAccountID());
                    otOut
                        << szFunc << ": Error loading full transaction "
                        << pTransaction->GetTransactionNum()
                        << ", since one was already there, in box for account: "
                        << strPurportedAcctID << ".\n";
                    delete pTransaction;
                    pTransaction = nullptr;
                    return (-1);
                }

                // It's not already there on this ledger -- so add it!
                // (Below this point, no need to delete pTransaction upon
                // returning.)
                //
                m_mapTransactions[pTransaction->GetTransactionNum()] =
                    pTransaction;
                pTransaction->SetParent(*this);
                //                otLog5 << "Loaded full transaction and adding
                // to m_mapTransactions in OTLedger\n");

                switch (GetType()) {
                case OTLedger::message:
                    break;
                case OTLedger::nymbox:
                case OTLedger::inbox:
                case OTLedger::outbox:
                case OTLedger::paymentInbox:
                case OTLedger::recordBox:
                case OTLedger::expiredBox: {
                    // For the sake of legacy data, check for existence of box
                    // receipt here,
                    // and re-save that box receipt if it doesn't exist.
                    //
                    otOut << "--- Apparently this is old data (the transaction "
                             "is still stored inside the ledger itself)... \n";
                    m_bLoadedLegacyData = true; // Only place this is set true.

                    const int32_t nBoxType = static_cast<int32_t>(GetType());

                    const bool bBoxReceiptAlreadyExists =
                        VerifyBoxReceiptExists(
                            pTransaction->GetRealServerID(),
                            pTransaction->GetUserID(),
                            pTransaction->GetRealAccountID(), // If Nymbox (vs
                                                              // inbox/outbox)
                                                              // the USER_ID
                                                              // will be in this
                                                              // field also.
                            nBoxType, // 0/nymbox, 1/inbox, 2/outbox
                            pTransaction->GetTransactionNum());
                    if (!bBoxReceiptAlreadyExists) // Doesn't already
                                                   // exist separately.
                    {
                        // Okay then, let's create it...
                        //
                        otOut << "--- The BoxReceipt doesn't exist separately "
                                 "(yet.) Creating it in local storage...\n";

                        const int64_t lBoxType = static_cast<int64_t>(nBoxType);

                        if (false ==
                            pTransaction->SaveBoxReceipt(
                                lBoxType)) //  <======== SAVE BOX RECEIPT
                            otErr << "--- FAILED trying to save BoxReceipt "
                                     "from legacy data to local storage!\n";
                    }
                } break;
                default:
                    otErr << szFunc
                          << ": Unknown ledger type while loading transaction!"
                             " (Should never happen.)\n"; // todo: assert here?
                                                          // "should never
                                                          // happen" ...
                    return (-1);
                } // switch (GetType())

            } // if transaction loads and verifies.
            else {
                otErr << "ERROR: loading or verifying transaction in "
                         "OTLedger::ProcessXMLNode\n";
                if (nullptr != pTransaction) {
                    delete pTransaction;
                    pTransaction = nullptr;
                }
                return (-1);
            }
        }
        else {
            otErr << "Error in OTLedger::ProcessXMLNode: transaction without "
                     "value.\n";
            return (-1); // error condition
        }
        return 1;
    }

    return 0;
}

OTLedger::~OTLedger()
{
    Release_Ledger();
}

void OTLedger::ReleaseTransactions()
{
    // If there were any dynamically allocated objects, clean them up here.

    while (!m_mapTransactions.empty()) {
        OTTransaction* pTransaction = m_mapTransactions.begin()->second;
        m_mapTransactions.erase(m_mapTransactions.begin());
        delete pTransaction;
        pTransaction = nullptr;
    }
}

void OTLedger::Release_Ledger()
{
    ReleaseTransactions();
}

void OTLedger::Release()
{
    Release_Ledger();

    ot_super::Release(); // since I've overridden the base class, I call it
                         // now...
}

} // namespace opentxs
