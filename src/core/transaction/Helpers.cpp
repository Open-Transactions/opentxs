/************************************************************
 *
 *  Helpers.cpp
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

#include <opentxs/core/OTTransaction.hpp>
#include <opentxs/core/Ledger.hpp>
#include <opentxs/core/String.hpp>
#include <opentxs/core/util/OTFolders.hpp>
#include <opentxs/core/Log.hpp>
#include <opentxs/core/OTStorage.hpp>
#include <opentxs/core/Identifier.hpp>
#include <opentxs/core/NumList.hpp>
#include <irrxml/irrXML.hpp>
#include <string>

namespace
{

char const* const TypeStrings[] = {
    "blank",   // freshly issued, not used yet  // comes from server, stored on
               // Nym. (Nymbox.)
    "message", // in nymbox, message from one user to another.
    "notice", // in nymbox, notice from the server. Probably contains an updated
              // smart contract.
    "replyNotice", // When you send a request to the server, sometimes its reply
                   // is so important,
    // that it drops a copy into your Nymbox to make you receive and process it.
    "successNotice",   // A transaction # has successfully been signed out.
                       // (Nymbox.)
    "pending",         // Pending transfer, in the inbox/outbox.
    "transferReceipt", // the server drops this into your inbox, when someone
                       // accepts your transfer.
    "chequeReceipt",   // the server drops this into your inbox, when someone
                       // deposits your cheque.
    "voucherReceipt",  // the server drops this into your inbox, when someone
                       // deposits your voucher.
    "marketReceipt",   // server drops this into inbox periodically, if you have
                       // an offer on the market.
    "paymentReceipt",  // the server drops this into people's inboxes,
                       // periodically, if they have payment plans.
    "finalReceipt",    // the server drops this into your inbox(es), when a
                       // CronItem expires or is canceled.
    "basketReceipt",   // the server drops this into your inboxes, when a basket
                       // exchange is processed.
    "instrumentNotice", // Receive these in paymentInbox (by way of Nymbox), and
                        // send in Outpayments (like outMail.) (When done, they
                        // go to recordBox or expiredBox to await deletion.)
    "instrumentRejection", // When someone rejects your invoice from his
                           // paymentInbox, you get one of these in YOUR
                           // paymentInbox.
    "processNymbox",   // process nymbox transaction     // comes from client
    "atProcessNymbox", // process nymbox reply             // comes from server
    "processInbox",    // process inbox transaction     // comes from client
    "atProcessInbox",  // process inbox reply             // comes from server
    "transfer",   // or "spend". This transaction is a transfer from one account
                  // to another
    "atTransfer", // reply from the server regarding a transfer request
    "deposit",   // this transaction is a deposit of bearer tokens (from client)
    "atDeposit", // reply from the server regarding a deposit request
    "withdrawal",       // this transaction is a withdrawal of bearer tokens
    "atWithdrawal",     // reply from the server regarding a withdrawal request
    "marketOffer",      // this transaction is a market offer
    "atMarketOffer",    // reply from the server regarding a market offer
    "paymentPlan",      // this transaction is a payment plan
    "atPaymentPlan",    // reply from the server regarding a payment plan
    "smartContract",    // this transaction is a smart contract
    "atSmartContract",  // reply from the server regarding a smart contract
    "cancelCronItem",   // this transaction is a cancellation of a cron item
                        // (payment plan etc)
    "atCancelCronItem", // reply from the server regarding said cancellation.
    "exchangeBasket",   // this transaction is an exchange in/out of a basket
                        // currency.
    "atExchangeBasket", // reply from the server regarding said exchange.
    "payDividend",      // this transaction is a dividend payment (to the
                        // shareholders.)
    "atPayDividend", // reply from the server regarding said dividend payment.
    "error_state"};

} // namespace

namespace opentxs
{

const char* GetTransactionTypeString(int transactionNumber)
{
    return TypeStrings[transactionNumber];
}

// Returns 1 if success, -1 if error.
int32_t LoadAbbreviatedRecord(irr::io::IrrXMLReader*& xml,
                              int64_t& lNumberOfOrigin,
                              int64_t& lTransactionNum, int64_t& lInRefTo,
                              int64_t& lInRefDisplay, time64_t& the_DATE_SIGNED,
                              int& theType, String& strHash,
                              int64_t& lAdjustment, int64_t& lDisplayValue,
                              int64_t& lClosingNum, int64_t& lRequestNum,
                              bool& bReplyTransSuccess, NumList* pNumList)
{

    const String strOrigin = xml->getAttributeValue("numberOfOrigin");
    const String strTransNum = xml->getAttributeValue("transactionNum");
    const String strInRefTo = xml->getAttributeValue("inReferenceTo");
    const String strInRefDisplay = xml->getAttributeValue("inRefDisplay");
    const String strDateSigned = xml->getAttributeValue("dateSigned");

    if (!strTransNum.Exists() || !strInRefTo.Exists() ||
        !strInRefDisplay.Exists() || !strDateSigned.Exists()) {
        otOut << "OTTransaction::LoadAbbreviatedRecord: Failure: missing "
                 "strTransNum (" << strTransNum << ") or strInRefTo ("
              << strInRefTo << ") or strInRefDisplay (" << strInRefDisplay
              << ") or strDateSigned(" << strDateSigned
              << ") while loading abbreviated receipt. \n";
        return (-1);
    }
    lTransactionNum = strTransNum.ToLong();
    lInRefTo = strInRefTo.ToLong();
    lInRefDisplay = strInRefDisplay.ToLong();

    if (strOrigin.Exists()) lNumberOfOrigin = strOrigin.ToLong();

    the_DATE_SIGNED = toTime_t(std::string(strDateSigned.Get()));

    // Transaction TYPE for the abbreviated record...
    theType = OTTransaction::error_state; // default
    const String strAbbrevType =
        xml->getAttributeValue("type"); // the type of inbox receipt, or outbox
                                        // receipt, or nymbox receipt.
                                        // (Transaction type.)
    if (strAbbrevType.Exists()) {
        theType = OTTransaction::GetTypeFromString(strAbbrevType);

        if (OTTransaction::error_state == theType) {
            otErr << "OTTransaction::LoadAbbreviatedRecord: Failure: "
                     "OTTransaction::error_state was the found type (based on "
                     "string " << strAbbrevType
                  << "), when loading abbreviated receipt for trans num: "
                  << lTransactionNum << " (In Reference To: " << lInRefTo
                  << ") \n";
            return (-1);
        }
    }
    else {
        otOut << "OTTransaction::LoadAbbreviatedRecord: Failure: unknown "
                 "transaction type (" << strAbbrevType
              << ") when "
                 "loading abbreviated receipt for trans num: "
              << lTransactionNum << " (In Reference To: " << lInRefTo << ") \n";
        return (-1);
    }

    // RECEIPT HASH
    //
    strHash = xml->getAttributeValue("receiptHash");
    if (!strHash.Exists()) {
        otOut << "OTTransaction::LoadAbbreviatedRecord: Failure: Expected "
                 "receiptHash while loading "
                 "abbreviated receipt for trans num: " << lTransactionNum
              << " (In Reference To: " << lInRefTo << ")\n";
        return (-1);
    }

    lAdjustment = 0;
    lDisplayValue = 0;
    lClosingNum = 0;

    const String strAbbrevAdjustment = xml->getAttributeValue("adjustment");
    if (strAbbrevAdjustment.Exists())
        lAdjustment = strAbbrevAdjustment.ToLong();
    // -------------------------------------
    const String strAbbrevDisplayValue = xml->getAttributeValue("displayValue");
    if (strAbbrevDisplayValue.Exists())
        lDisplayValue = strAbbrevDisplayValue.ToLong();

    if (OTTransaction::replyNotice == theType) {
        const String strRequestNum = xml->getAttributeValue("requestNumber");

        if (!strRequestNum.Exists()) {
            otOut << "OTTransaction::LoadAbbreviatedRecord: Failed loading "
                     "abbreviated receipt: "
                     "expected requestNumber on replyNotice trans num: "
                  << lTransactionNum << " (In Reference To: " << lInRefTo
                  << ")\n";
            return (-1);
        }
        lRequestNum = strRequestNum.ToLong();

        const String strTransSuccess = xml->getAttributeValue("transSuccess");

        bReplyTransSuccess = strTransSuccess.Compare("true");
    } // if replyNotice (expecting request Number)

    // If the transaction is a certain type, then it will also have a CLOSING
    // number.
    // (Grab that too.)
    //
    if ((OTTransaction::finalReceipt == theType) ||
        (OTTransaction::basketReceipt == theType)) {
        const String strAbbrevClosingNum = xml->getAttributeValue("closingNum");

        if (!strAbbrevClosingNum.Exists()) {
            otOut << "OTTransaction::LoadAbbreviatedRecord: Failed loading "
                     "abbreviated receipt: "
                     "expected closingNum on trans num: " << lTransactionNum
                  << " (In Reference To: " << lInRefTo << ")\n";
            return (-1);
        }
        lClosingNum = strAbbrevClosingNum.ToLong();
    } // if finalReceipt or basketReceipt (expecting closing num)

    // These types carry their own internal list of numbers.
    //
    if ((nullptr != pNumList) && ((OTTransaction::blank == theType) ||
                                  (OTTransaction::successNotice == theType))) {
        const String strNumbers = xml->getAttributeValue("totalListOfNumbers");
        pNumList->Release();

        if (strNumbers.Exists()) pNumList->Add(strNumbers);
    } // if blank or successNotice (expecting totalListOfNumbers.. no more
      // multiple blanks in the same ledger! They all go in a single
      // transaction.)

    return 1;
}

bool VerifyBoxReceiptExists(
    const Identifier& NOTARY_ID,
    const Identifier& NYM_ID,     // Unused here for now, but still convention.
    const Identifier& ACCOUNT_ID, // If for Nymbox (vs inbox/outbox) then pass
                                  // NYM_ID in this field also.
    const int32_t nBoxType,       // 0/nymbox, 1/inbox, 2/outbox
    const int64_t& lTransactionNum)
{
    const int64_t lLedgerType = static_cast<int64_t>(nBoxType);

    const String strNotaryID(NOTARY_ID),
        strUserOrAcctID(0 == lLedgerType ? NYM_ID : ACCOUNT_ID); // (For Nymbox
                                                                 // aka type 0,
                                                                 // the NymID
                                                                 // will be
                                                                 // here.)
    // --------------------------------------------------------------------
    String strFolder1name, strFolder2name, strFolder3name, strFilename;

    if (!SetupBoxReceiptFilename(lLedgerType, // nBoxType is lLedgerType
                                 strUserOrAcctID, strNotaryID, lTransactionNum,
                                 "OTTransaction::VerifyBoxReceiptExists",
                                 strFolder1name, strFolder2name, strFolder3name,
                                 strFilename))
        return false; // This already logs -- no need to log twice, here.
    // --------------------------------------------------------------------
    // See if the box receipt exists before trying to save over it...
    //
    const bool bExists =
        OTDB::Exists(strFolder1name.Get(), strFolder2name.Get(),
                     strFolder3name.Get(), strFilename.Get());

    otWarn << "OTTransaction::" << (bExists ? "(Already have this one)"
                                            : "(Need to download this one)")
           << ": " << __FUNCTION__ << ": " << strFolder1name
           << Log::PathSeparator() << strFolder2name << Log::PathSeparator()
           << strFolder3name << Log::PathSeparator() << strFilename << "\n";

    return bExists;
}

OTTransaction* LoadBoxReceipt(OTTransaction& theAbbrev, Ledger& theLedger)
{
    const int64_t lLedgerType = static_cast<int64_t>(theLedger.GetType());
    return LoadBoxReceipt(theAbbrev, lLedgerType);
}

OTTransaction* LoadBoxReceipt(OTTransaction& theAbbrev, int64_t lLedgerType)
{
    // See if the appropriate file exists, and load it up from
    // local storage, into a string.
    // Then, try to load the transaction from that string and see if successful.
    // If it verifies, then return it. Otherwise return nullptr.

    // Can only load abbreviated transactions (so they'll become their full
    // form.)
    //
    if (!theAbbrev.IsAbbreviated()) {
        otOut << __FUNCTION__ << ": Unable to load box receipt "
              << theAbbrev.GetTransactionNum()
              << ": "
                 "(Because argument 'theAbbrev' wasn't abbreviated.)\n";
        return nullptr;
    }

    // Next, see if the appropriate file exists, and load it up from
    // local storage, into a string.

    String strFolder1name, strFolder2name, strFolder3name, strFilename;

    if (!SetupBoxReceiptFilename(
            lLedgerType, theAbbrev,
            __FUNCTION__, // "OTTransaction::LoadBoxReceipt",
            strFolder1name, strFolder2name, strFolder3name, strFilename))
        return nullptr; // This already logs -- no need to log twice, here.

    // See if the box receipt exists before trying to load it...
    //
    if (!OTDB::Exists(strFolder1name.Get(), strFolder2name.Get(),
                      strFolder3name.Get(), strFilename.Get())) {
        otWarn << __FUNCTION__
               << ": Box receipt does not exist: " << strFolder1name
               << Log::PathSeparator() << strFolder2name << Log::PathSeparator()
               << strFolder3name << Log::PathSeparator() << strFilename << "\n";
        return nullptr;
    }

    // Try to load the box receipt from local storage.
    //
    std::string strFileContents(OTDB::QueryPlainString(
        strFolder1name.Get(), // <=== LOADING FROM DATA STORE.
        strFolder2name.Get(), strFolder3name.Get(), strFilename.Get()));
    if (strFileContents.length() < 2) {
        otErr << __FUNCTION__ << ": Error reading file: " << strFolder1name
              << Log::PathSeparator() << strFolder2name << Log::PathSeparator()
              << strFolder3name << Log::PathSeparator() << strFilename << "\n";
        return nullptr;
    }

    String strRawFile(strFileContents.c_str());

    if (!strRawFile.Exists()) {
        otErr << __FUNCTION__ << ": Error reading file (resulting output "
                                 "string is empty): " << strFolder1name
              << Log::PathSeparator() << strFolder2name << Log::PathSeparator()
              << strFolder3name << Log::PathSeparator() << strFilename << "\n";
        return nullptr;
    }

    // Finally, try to load the transaction from that string and see if
    // successful.
    //
    OTTransactionType* pTransType =
        OTTransactionType::TransactionFactory(strRawFile);

    if (nullptr == pTransType) {
        otErr << __FUNCTION__ << ": Error instantiating transaction "
                                 "type based on strRawFile: " << strFolder1name
              << Log::PathSeparator() << strFolder2name << Log::PathSeparator()
              << strFolder3name << Log::PathSeparator() << strFilename << "\n";
        return nullptr;
    }

    OTTransaction* pBoxReceipt = dynamic_cast<OTTransaction*>(pTransType);

    if (nullptr == pBoxReceipt) {
        otErr << __FUNCTION__
              << ": Error dynamic_cast from transaction "
                 "type to transaction, based on strRawFile: " << strFolder1name
              << Log::PathSeparator() << strFolder2name << Log::PathSeparator()
              << strFolder3name << Log::PathSeparator() << strFilename << "\n";
        delete pTransType;
        pTransType = nullptr; // cleanup!
        return nullptr;
    }

    // BELOW THIS POINT, pBoxReceipt exists, and is an OTTransaction pointer,
    // and is loaded,
    // and basically is ready to be compared to theAbbrev, which is its
    // abbreviated version.
    // It MUST either be returned or deleted.

    bool bSuccess = theAbbrev.VerifyBoxReceipt(*pBoxReceipt);

    if (!bSuccess) {
        otErr << __FUNCTION__ << ": Failed verifying Box Receipt:\n"
              << strFolder1name << Log::PathSeparator() << strFolder2name
              << Log::PathSeparator() << strFolder3name << Log::PathSeparator()
              << strFilename << "\n";

        delete pBoxReceipt;
        pBoxReceipt = nullptr;
        return nullptr;
    }
    else
        otInfo << __FUNCTION__ << ": Successfully loaded Box Receipt in:\n"
               << strFolder1name << Log::PathSeparator() << strFolder2name
               << Log::PathSeparator() << strFolder3name << Log::PathSeparator()
               << strFilename << "\n";

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
    //  pBoxReceipt->SetAbbrevAdjustment(       theAbbrev.GetAbbrevAdjustment()
    // );
    //  pBoxReceipt->SetAbbrevDisplayAmount(
    // theAbbrev.GetAbbrevDisplayAmount() );

    return pBoxReceipt;
}

bool SetupBoxReceiptFilename(int64_t lLedgerType, const String& strUserOrAcctID,
                             const String& strNotaryID,
                             const int64_t& lTransactionNum,
                             const char* szCaller, String& strFolder1name,
                             String& strFolder2name, String& strFolder3name,
                             String& strFilename)
{
    OT_ASSERT(nullptr != szCaller);

    const char* pszFolder = nullptr; // "nymbox" (or "inbox" or "outbox")
    switch (lLedgerType) {
    case 0:
        pszFolder = OTFolders::Nymbox().Get();
        break;
    case 1:
        pszFolder = OTFolders::Inbox().Get();
        break;
    case 2:
        pszFolder = OTFolders::Outbox().Get();
        break;
    //      case 3: (message ledger.)
    case 4:
        pszFolder = OTFolders::PaymentInbox().Get();
        break;
    case 5:
        pszFolder = OTFolders::RecordBox().Get();
        break;
    case 6:
        pszFolder = OTFolders::ExpiredBox().Get();
        break;
    default:
        otErr << "OTTransaction::" << __FUNCTION__ << " " << szCaller
              << ": Error: unknown box type: " << lLedgerType
              << ". (This should never happen.)\n";
        return false;
    }

    strFolder1name.Set(pszFolder);   // "nymbox" (or "inbox" or "outbox")
    strFolder2name.Set(strNotaryID); // "NOTARY_ID"
    strFolder3name.Format("%s.r", strUserOrAcctID.Get()); // "NYM_ID.r"

    // "TRANSACTION_ID.rct"
    strFilename.Format("%" PRId64 ".rct", lTransactionNum);
    // todo hardcoding of file extension. Need to standardize extensions.

    // Finished product: "nymbox/NOTARY_ID/NYM_ID.r/TRANSACTION_ID.rct"

    return true;
}

bool SetupBoxReceiptFilename(int64_t lLedgerType, OTTransaction& theTransaction,
                             const char* szCaller, String& strFolder1name,
                             String& strFolder2name, String& strFolder3name,
                             String& strFilename)
{
    String strUserOrAcctID;
    theTransaction.GetIdentifier(strUserOrAcctID);

    const String strNotaryID(theTransaction.GetRealNotaryID());

    return SetupBoxReceiptFilename(lLedgerType, strUserOrAcctID, strNotaryID,
                                   theTransaction.GetTransactionNum(), szCaller,
                                   strFolder1name, strFolder2name,
                                   strFolder3name, strFilename);
}

bool SetupBoxReceiptFilename(Ledger& theLedger, OTTransaction& theTransaction,
                             const char* szCaller, String& strFolder1name,
                             String& strFolder2name, String& strFolder3name,
                             String& strFilename)
{
    int64_t lLedgerType = 0;

    switch (theLedger.GetType()) {
    case Ledger::nymbox:
        lLedgerType = 0;
        break;
    case Ledger::inbox:
        lLedgerType = 1;
        break;
    case Ledger::outbox:
        lLedgerType = 2;
        break;
    //        case OTLedger::message:         lLedgerType = 3;    break;
    case Ledger::paymentInbox:
        lLedgerType = 4;
        break;
    case Ledger::recordBox:
        lLedgerType = 5;
        break;
    case Ledger::expiredBox:
        lLedgerType = 6;
        break;
    default:
        otErr << "OTTransaction::" << __FUNCTION__ << " " << szCaller
              << ": Error: unknown box type. "
                 "(This should never happen.)\n";
        return false;
    }

    return SetupBoxReceiptFilename(lLedgerType, theTransaction, szCaller,
                                   strFolder1name, strFolder2name,
                                   strFolder3name, strFilename);
}

} // namespace opentxs
