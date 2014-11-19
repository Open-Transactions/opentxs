/************************************************************
 *
 *  OTItem.cpp
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

#include <opentxs/core/OTItem.hpp>
#include <opentxs/core/Account.hpp>
#include <opentxs/core/Cheque.hpp>
#include <opentxs/core/OTLedger.hpp>
#include <opentxs/core/OTLog.hpp>
#include <opentxs/core/OTPseudonym.hpp>
#include <opentxs/core/OTStorage.hpp>

#include <irrxml/irrXML.hpp>
#include <cstring>

#include <memory>

// Server-side.
//
// By the time this is called, I know that the item, AND this balance item
// (this)
// both have the correct user id, server id, account id, and transaction id, and
// they have been signed properly by the owner.
//
// So what do I need to verify in this function?
//
// -- That the transactions on THE_NYM (server-side), minus the current
// transaction number being processed,
//    are all still there.
// -- If theMessageNym is missing certain numbers that I expected to find on
// him, that means he is trying to
//    trick the server into signing a new agreement where he is no longer
// responsible for those numbers. They must
//    all be there.
// -- If theMessageNym has ADDED certain numbers that I DIDN'T expect to find on
// him, then he's trying to trick
//    me into allowing him to add those numbers to his receipt -- OR it could
// mean that certain numbers were already
//    removed on my side (such as the opening # for a cron item like a market
// offer that has already been closed),
//    but the client-side isn't aware of this yet, and so he is trying to sign
// off on formerly-good numbers that
//    have since expired.  This shouldn't happen IF the client has been properly
// notified about these numbers before
//    sending his request.  Such notifications are dropped into the Nymbox AND
// related asset account inboxes.
//

namespace opentxs
{

bool OTItem::VerifyTransactionStatement(OTPseudonym& THE_NYM,
                                        OTTransaction& TARGET_TRANSACTION,
                                        bool bIsRealTransaction) // Sometimes
                                                                 // the trans#
                                                                 // is 0 (like
// when processing Nymbox)
{
    if (GetType() != OTItem::transactionStatement) {
        otOut << __FUNCTION__
              << ": wrong item type. Expected OTItem::transactionStatement.\n";
        return false;
    }

    //
    // So if the caller was planning to remove a number, or clear a receipt from
    // the inbox, he'll have to do
    // so first before calling this function, and then ADD IT AGAIN if this
    // function fails.  (Because the new
    // Balance Agreement is always the user signing WHAT THE NEW VERSION WILL BE
    // AFTER THE TRANSACTION IS PROCESSED.)
    //
    const String SERVER_ID(GetPurportedServerID());

    OTPseudonym theRemovedNym;

    if (bIsRealTransaction) // Sometimes my "transaction number" is 0 since
                            // we're accepting numbers from the Nymbox (which is
                            // done by message, not transaction.)
    { //  In such cases, there's no point in checking the server-side to "make
        // sure it has number 0!" (because it won't.)
        bool bIWasFound =
            THE_NYM.VerifyIssuedNum(SERVER_ID, GetTransactionNum());

        if (!bIWasFound) {
            otOut << __FUNCTION__ << ": Transaction# (" << GetTransactionNum()
                  << ") doesn't appear on Nym's issued list.\n";
            return false;
        }

        // In the case that this is a real transaction, it must be a
        // cancelCronItem, payment plan or
        // market offer (since the other transaction types require a balance
        // statement, not a transaction
        // statement.) Also this might not be a transaction at all, but in that
        // case we won't enter this
        // block anyway.
        //
        switch (TARGET_TRANSACTION.GetType()) {
        // In the case of cancelCronItem(), we'd expect, if success, the number
        // would be removed, so we have
        // to remove it now, to simulate success for the verification. Then we
        // add it again afterwards, before
        // returning.
        //
        case OTTransaction::cancelCronItem:
            // Only adding it to theRemovedNym here since VerifyIssuedNum() is
            // called just above.
            // (Don't want to "add it back" if it wasn't there in the first
            // place!)
            //
            if (THE_NYM.RemoveIssuedNum(SERVER_ID,
                                        GetTransactionNum())) // doesn't save.
                theRemovedNym.AddIssuedNum(SERVER_ID, GetTransactionNum());
            else
                otOut << "OTItem::VerifyTransactionStatemen: Expected THE_NYM "
                         "to have trans# " << GetTransactionNum()
                      << " but didn't find it.\n";
            break;

        // IN the case of the offer/plan, we do NOT want to remove from issued
        // list. That only happens when
        // the plan or offer is removed from Cron and closed. As the plan or
        // offer continues processing,
        // the user is responsible for its main transaction number until he
        // signs off on final closing,
        // after many receipts have potentially been received.
        //
        case OTTransaction::marketOffer:
        case OTTransaction::paymentPlan:
        case OTTransaction::smartContract:
            //              THE_NYM.RemoveIssuedNum(SERVER_ID,
            // GetTransactionNum()); // commented out, explained just
            // above.
            break;
        default:
            otErr << "OTItem::VerifyTransactionStatement: Unexpected "
                     "transaction type.\n";
            break;
        }

        // Client side will NOT remove from issued list in this case (market
        // offer, payment plan, which are
        // the only transactions that use a transactionStatement, which is
        // otherwise used for Nymbox.)
    }

    // VERIFY that the Nyms have a matching list of transaction numbers...

    bool bSuccess = false;

    String strMessageNym;

    GetAttachment(strMessageNym);
    OTPseudonym theMessageNym;

    if ((strMessageNym.GetLength() > 2) &&
        theMessageNym.LoadFromString(strMessageNym)) {
        // If success, I know the server-side copy of the user's Nym (THE_NYM)
        // has the same number
        // of transactions as the message nym, and that EVERY ONE OF THEM was
        // found individually.
        //
        bSuccess = THE_NYM.VerifyIssuedNumbersOnNym(
            theMessageNym); // <==== ************************************
    }

    // NOW let's set things back how they were before, so we can RETURN.
    //
    if (bIsRealTransaction) {
        switch (TARGET_TRANSACTION.GetType()) {
        case OTTransaction::cancelCronItem:
            // Should only actually iterate once, in this case.
            for (int32_t i = 0;
                 i < theRemovedNym.GetIssuedNumCount(GetPurportedServerID());
                 i++) {
                int64_t lTemp =
                    theRemovedNym.GetIssuedNum(GetPurportedServerID(), i);

                if (i > 0)
                    otErr << "OTItem::VerifyTransactionStatement: THIS SHOULD "
                             "NOT HAPPEN.\n";
                else if (false ==
                         THE_NYM.AddIssuedNum(SERVER_ID,
                                              lTemp)) // doesn't save.
                    otErr << "Failed adding issued number back to THE_NYM in "
                             "OTItem::VerifyTransactionStatement.\n";
            }
            break;

        case OTTransaction::marketOffer:
        case OTTransaction::paymentPlan:
        case OTTransaction::smartContract:
        default:
            //              THE_NYM.RemoveIssuedNum(SERVER_ID,
            // GetTransactionNum()); // commented out, explained just
            // above.
            break;
        }
    }

    // Might want to consider saving the Nym here.
    // Also want to save the latest signed receipt, since it VERIFIES.
    // Or maybe let caller decide?

    return bSuccess;
}

// Server-side.
//
// By the time this is called, I know that the item, AND this balance item
// (this)
// both have the correct user id, server id, account id, and transaction id, and
// they have been signed properly by the owner.
//
// So what do I need to verify in this function?
//
// 1) That THE_ACCOUNT.GetBalance() + lActualAdjustment equals the amount in
// GetAmount().
//
// 2) That the inbox transactions and outbox transactions match up to the list
// of sub-items
//    on THIS balance item.
//
// 3) That the transactions on the Nym, minus the current transaction number
// being processed,
//    are all still there.
//
bool OTItem::VerifyBalanceStatement(
    int64_t lActualAdjustment, OTPseudonym& THE_NYM, OTLedger& THE_INBOX,
    OTLedger& THE_OUTBOX, const Account& THE_ACCOUNT,
    OTTransaction& TARGET_TRANSACTION,
    int64_t lOutboxTrnsNum) // Only used in the case of transfer,
                            // where the user
{ // doesn't know the outbox trans# in advance, so he sends
    if (GetType() != OTItem::balanceStatement) // a dummy number (currently '1')
                                               // which we verify against
    { // the actual outbox trans# successfully, only in that special case.
        otOut << "OTItem::" << __FUNCTION__ << ": wrong item type.\n";
        return false;
    }

    // We need to verify:
    //
    // 1) That THE_ACCOUNT.GetBalance() + lActualAdjustment equals the amount in
    // GetAmount().

    if ((THE_ACCOUNT.GetBalance() + lActualAdjustment) !=
        GetAmount()) // GetAmount() contains what the balance WOULD
                     // be AFTER successful transaction.
    {
        otOut << "OTItem::" << __FUNCTION__
              << ": This balance statement has a value of " << GetAmount()
              << ", but expected "
              << (THE_ACCOUNT.GetBalance() + lActualAdjustment)
              << ". "
                 "(Acct balance of " << THE_ACCOUNT.GetBalance()
              << " plus actualAdjustment of " << lActualAdjustment << ".)\n";
        return false;
    }

    // 2) That the inbox transactions and outbox transactions match up to the
    // list of sub-items
    //    on THIS balance item.

    int32_t nInboxItemCount = 0, nOutboxItemCount = 0;

    const char* szInbox = "Inbox";
    const char* szOutbox = "Outbox";

    const char* pszLedgerType = nullptr;

    //    otWarn << "OTItem::VerifyBalanceStatement: (ENTERING LOOP)... INBOX
    // COUNT: %d\n"
    //                   "# of inbox/outbox items on this balance statement:
    // %d\n",
    //                   THE_INBOX.GetTransactionCount(), GetItemCount());

    for (int32_t i = 0; i < GetItemCount(); i++) {
        OTItem* pSubItem = GetItem(i);
        OT_ASSERT(nullptr != pSubItem);
        //      otWarn << "OTItem::VerifyBalanceStatement: TOP OF LOOP (through
        // sub-items).......\n");

        int64_t lReceiptAmountMultiplier = 1; // needed for outbox items.

        OTLedger* pLedger = nullptr;

        switch (pSubItem->GetType()) {
        case OTItem::voucherReceipt:
        case OTItem::chequeReceipt:
        case OTItem::marketReceipt:
        case OTItem::paymentReceipt:
        case OTItem::transferReceipt:
        case OTItem::basketReceipt:
        case OTItem::finalReceipt:
            nInboxItemCount++;
            pLedger = &THE_INBOX;
            pszLedgerType = szInbox;

        //              otWarn << "OTItem::VerifyBalanceStatement: Subitem is
        // Inbox receipt item (NOT pending transfer)....\n");

        case OTItem::transfer:
            break;
        default: {
            String strItemType;
            GetTypeString(strItemType);
            otWarn << "OTItem::" << __FUNCTION__ << ": Ignoring " << strItemType
                   << " item in balance statement while verifying it against "
                      "inbox.\n";
        }
            continue;
        }

        switch (pSubItem->GetType()) {
        case OTItem::transfer:
            if (pSubItem->GetAmount() < 0) // it's an outbox item
            {
                //                  otWarn << "OTItem::VerifyBalanceStatement:
                // Subitem is pending transfer (in outbox)....\n");

                lReceiptAmountMultiplier =
                    -1; // transfers out always reduce your balance.
                nOutboxItemCount++;
                pLedger = &THE_OUTBOX;
                pszLedgerType = szOutbox;
            }
            else {
                //                  otWarn << "OTItem::VerifyBalanceStatement:
                // Subitem is pending transfer (in inbox)....\n");

                lReceiptAmountMultiplier =
                    1; // transfers in always increase your balance.
                nInboxItemCount++;
                pLedger = &THE_INBOX;
                pszLedgerType = szInbox;
            }
            break;
        case OTItem::finalReceipt:
        // Here: If there is a finalReceipt on this balance statement, then ALL
        // the other
        // related receipts in the inbox (with same "reference to" value) had
        // better ALSO
        // be on the same balance statement!

        // HMM that is true, but NOT HERE... That's only true when PROCESSING
        // the final Receipt
        // from the inbox (in that case, all the marketReceipts must also be
        // processed with it.)
        // But here, I am looping through the inbox report, and there happens to
        // be a finalReceipt
        // on it. (Which doesn't mean necessarily that it's being processed
        // out...)
        case OTItem::basketReceipt:

        case OTItem::transferReceipt:
        case OTItem::voucherReceipt:
        case OTItem::chequeReceipt:
        case OTItem::marketReceipt:
        case OTItem::paymentReceipt:
            lReceiptAmountMultiplier = 1;
            break;
        default:
            otErr << "OTItem::" << __FUNCTION__
                  << ": Bad Subitem type (SHOULD NEVER HAPPEN)....\n";

            continue; // This will never happen, due to the first continue above
                      // in the first switch.
        }

        OTTransaction* pTransaction = nullptr;

        // In the special case of account transfer, the user has put an outbox
        // transaction
        // into his balance agreement with the special number '1', since he has
        // no idea what
        // actual number will be generated on the server side (for the outbox)
        // when his
        // message is received by the server.
        //
        // When that happens (ONLY in account transfer) then lOutboxTrnsNum will
        // be passed
        // in with the new transaction number chosen by the server (a real
        // number, like 18736
        // or whatever, instead of the default of 0 that will otherwise be
        // passed in here.)
        //
        // Therefore, if lOutboxTrnsNum is larger than 0, AND if we're on an
        // outbox item,
        // then we can expect lOutboxTrnsNum to contain an actual transaction
        // number, and
        // we can expect there is a CHANCE that the sub-item will be trans# 1.
        // (It might
        // NOT be number 1, since there may be other outbox items-we're looping
        // through them
        // right now in this block.) So we'll check to see if this is the '1'
        // and if so,
        // we'll look up pTransaction from the outbox using the real transaction
        // number,
        // instead of '1' which of course would not find it (since the version
        // in the ledger
        // contains the ACTUAL number now, since the server just issued it.)
        //
        if ((lOutboxTrnsNum > 0) && (&THE_OUTBOX == pLedger) &&
            (pSubItem->GetTransactionNum() == 1)) // TODO use a constant for
                                                  // this 1.
        {
            otLog3 << "OTItem::" << __FUNCTION__
                   << ": Subitem is new Outbox Transaction... retrieving by "
                      "special ID: " << lOutboxTrnsNum << "\n";

            pTransaction = pLedger->GetTransaction(lOutboxTrnsNum);
        }
        else {
            otLog4 << "OTItem::" << __FUNCTION__
                   << ": Subitem is normal Transaction... retrieving by ID: "
                   << pSubItem->GetTransactionNum() << "\n";

            pTransaction =
                pLedger->GetTransaction(pSubItem->GetTransactionNum());
        }

        // Make sure that the transaction number of each sub-item is found
        // on the appropriate ledger (inbox or outbox).
        if (nullptr == pTransaction) {
            otOut << "OTItem::" << __FUNCTION__ << ": Expected "
                  << pszLedgerType << " transaction (serv " << lOutboxTrnsNum
                  << ", client " << pSubItem->GetTransactionNum()
                  << ") "
                     "not found. (Amount " << pSubItem->GetAmount() << ".)\n";
            return false;
        }
        // pTransaction is set below this point.

        if (pSubItem->GetReferenceToNum() !=
            pTransaction->GetReferenceToNum()) {
            otOut << "OTItem::" << __FUNCTION__ << ": " << pszLedgerType
                  << " transaction (" << pSubItem->GetTransactionNum()
                  << ") mismatch Reference Num: "
                  << pSubItem->GetReferenceToNum() << ", expected "
                  << pTransaction->GetReferenceToNum() << "\n";
            return false;
        }

        if (pSubItem->GetRawNumberOfOrigin() !=
            pTransaction->GetRawNumberOfOrigin()) {
            otOut << "OTItem::" << __FUNCTION__ << ": " << pszLedgerType
                  << " transaction (" << pSubItem->GetTransactionNum()
                  << ") mismatch Origin Num: "
                  << pSubItem->GetRawNumberOfOrigin() << ", expected "
                  << pTransaction->GetRawNumberOfOrigin() << "\n";

            // THE BELOW STUFF IS JUST FOR DEBUGGING PURPOSES.
            // ERASE IT.

            /*
            OTString strTempType;
            pSubItem->GetTypeString(strTempType);

            int64_t lTempAmount = pSubItem->GetAmount();

            OTIdentifier ACCOUNT_ID, SERVER_ID, USER_ID;

            ACCOUNT_ID = pSubItem->GetPurportedAccountID();
            SERVER_ID  = pSubItem->GetPurportedServerID();
            USER_ID    = pSubItem->GetUserID();

            const OTString strAccountID(ACCOUNT_ID), strServerID(SERVER_ID),
            strUserID(USER_ID);


            int64_t lTempNumOfOrigin = pSubItem->GetNumberOfOrigin();
            int64_t lTempTransNum    = pSubItem->GetTransactionNum();
            int64_t lTempRefNum      = pSubItem->GetReferenceToNum();
            int64_t lTempClosingNum  = pSubItem->GetClosingNum();


            const OTString strTrans(*pTransaction);
            otOut << "OTItem::%s: %s transaction (%" PRId64 ") mismatch Origin
            Num:
            %" PRId64 ", expected %" PRId64 "\n\nTRANSACTION:\n%s\n\n"
               "SubItem Type: %s  Amount: %" PRId64 "\nAccount: %s\nServer:
            %s\nUser: %s\n"
            " Number of Origin: %" PRId64 "\n Transaction Num: %" PRId64 "\n In
            Reference To: %" PRId64 "\n Closing Num: %d\n",
                           __FUNCTION__, pszLedgerType,
            pSubItem->GetTransactionNum(),
                           pSubItem->GetRawNumberOfOrigin(),
            pTransaction->GetRawNumberOfOrigin(),
                           strTrans.Get(),
                           strTempType.Get(), lTempAmount, strAccountID.Get(),
            strServerID.Get(), strUserID.Get(),
                           lTempNumOfOrigin, lTempTransNum, lTempRefNum,
            lTempClosingNum
                           );
             */
            return false;
        }

        int64_t lTransactionAmount = pTransaction->GetReceiptAmount();
        lTransactionAmount *= lReceiptAmountMultiplier;

        if (pSubItem->GetAmount() != lTransactionAmount) {
            otOut << "OTItem::" << __FUNCTION__ << ": " << pszLedgerType
                  << " transaction (" << pSubItem->GetTransactionNum()
                  << ") "
                     "amounts don't match: report amount is "
                  << pSubItem->GetAmount() << ", but expected "
                  << lTransactionAmount
                  << ". Trans Receipt Amt: " << pTransaction->GetReceiptAmount()
                  << " (GetAmount() == " << GetAmount() << ".)\n";
            return false;
        }

        if ((pSubItem->GetType() == OTItem::transfer) &&
            (pTransaction->GetType() != OTTransaction::pending)) {
            otOut << "OTItem::" << __FUNCTION__ << ": " << pszLedgerType
                  << " transaction (" << pSubItem->GetTransactionNum()
                  << ") wrong type. (transfer block)\n";
            return false;
        }

        if ((pSubItem->GetType() == OTItem::chequeReceipt) &&
            (pTransaction->GetType() != OTTransaction::chequeReceipt)) {
            otOut << "OTItem::" << __FUNCTION__ << ": " << pszLedgerType
                  << " transaction (" << pSubItem->GetTransactionNum()
                  << ") wrong type. (chequeReceipt block)\n";
            return false;
        }

        if ((pSubItem->GetType() == OTItem::voucherReceipt) &&
            (pTransaction->GetType() != OTTransaction::voucherReceipt)) {
            otOut << "OTItem::" << __FUNCTION__ << ": " << pszLedgerType
                  << " transaction (" << pSubItem->GetTransactionNum()
                  << ") wrong type. (voucherReceipt block)\n";
            return false;
        }

        if ((pSubItem->GetType() == OTItem::marketReceipt) &&
            (pTransaction->GetType() != OTTransaction::marketReceipt)) {
            otOut << "OTItem::" << __FUNCTION__ << ": " << pszLedgerType
                  << " transaction (" << pSubItem->GetTransactionNum()
                  << ") wrong type. (marketReceipt block)\n";
            return false;
        }

        if ((pSubItem->GetType() == OTItem::paymentReceipt) &&
            (pTransaction->GetType() != OTTransaction::paymentReceipt)) {
            otOut << "OTItem::" << __FUNCTION__ << ": " << pszLedgerType
                  << " transaction (" << pSubItem->GetTransactionNum()
                  << ") wrong type. (paymentReceipt block)\n";
            return false;
        }

        if ((pSubItem->GetType() == OTItem::transferReceipt) &&
            (pTransaction->GetType() != OTTransaction::transferReceipt)) {
            otOut << "OTItem::" << __FUNCTION__ << ": " << pszLedgerType
                  << " transaction (" << pSubItem->GetTransactionNum()
                  << ") wrong type. (transferReceipt block)\n";
            return false;
        }

        if ((pSubItem->GetType() == OTItem::basketReceipt) &&
            ((pTransaction->GetType() != OTTransaction::basketReceipt) ||
             (pSubItem->GetClosingNum() != pTransaction->GetClosingNum()))) {
            otOut << "OTItem::" << __FUNCTION__ << ": " << pszLedgerType
                  << " transaction (" << pSubItem->GetTransactionNum()
                  << ") wrong type or closing num ("
                  << pSubItem->GetClosingNum() << "). "
                                                  "(basketReceipt block)\n";
            return false;
        }

        if ((pSubItem->GetType() == OTItem::finalReceipt) &&
            ((pTransaction->GetType() != OTTransaction::finalReceipt) ||
             (pSubItem->GetClosingNum() != pTransaction->GetClosingNum()))) {
            otOut << "OTItem::" << __FUNCTION__ << ": " << pszLedgerType
                  << " transaction (" << pSubItem->GetTransactionNum()
                  << ") wrong type or closing num ("
                  << pSubItem->GetClosingNum() << "). "
                                                  "(finalReceipt block)\n";
            return false;
        }
    }

    // By this point, I have an accurate count of the inbox items, and outbox
    // items, represented
    // by this. let's compare those counts to the actual inbox and outbox on my
    // side:

    if ((nInboxItemCount != THE_INBOX.GetTransactionCount()) ||
        (nOutboxItemCount != THE_OUTBOX.GetTransactionCount())) {
        otOut << "OTItem::" << __FUNCTION__
              << ": Inbox or Outbox mismatch in expected transaction count.\n"
                 " --- THE_INBOX count: " << THE_INBOX.GetTransactionCount()
              << " --- THE_OUTBOX count: " << THE_OUTBOX.GetTransactionCount()
              << "\n"
                 "--- nInboxItemCount count: " << nInboxItemCount
              << " --- nOutboxItemCount count: " << nOutboxItemCount << "\n\n";

        return false;
    }

    // Now I KNOW that the inbox and outbox counts are the same, AND I know that
    // EVERY transaction number
    // on the balance item (this) was also found in the inbox or outbox,
    // wherever it was expected to be found.
    // I also know:
    // the amount was correct,
    // the "in reference to" number was correct,
    // and the type was correct.
    //
    // So if the caller was planning to remove a number, or clear a receipt from
    // the inbox, he'll have to do
    // so first before calling this function, and then ADD IT AGAIN if this
    // function fails.  (Because the new
    // Balance Agreement is always the user signing WHAT THE NEW VERSION WILL BE
    // AFTER THE TRANSACTION IS PROCESSED.
    // Thus, if the transaction fails to process, the action hasn't really
    // happened, so need to add it back again.)

    // 3) Also need to verify the transactions on the Nym, against the
    // transactions stored on this
    //    (in a message Nym attached to this.)  Check for presence of each, then
    // compare count, like above.

    OTPseudonym theRemovedNym;

    String SERVER_ID(GetPurportedServerID());

    // GetTransactionNum() is the ID for this balance agreement, THUS it's also
    // the ID
    // for whatever actual transaction is being attempted. If that ID is not
    // verified as
    // on my issued list, then the whole transaction is invalid (not
    // authorized.)
    //
    bool bIWasFound = THE_NYM.VerifyIssuedNum(SERVER_ID, GetTransactionNum());

    if (!bIWasFound) {
        otOut << "OTItem::" << __FUNCTION__ << ": Transaction has # that "
                                               "doesn't appear on Nym's issued "
                                               "list.\n";
        return false;
    }

    // BELOW THIS POINT, WE *KNOW* THE ISSUED NUM IS CURRENTLY ON THE LIST...
    //
    // (SO I CAN remove it and add it again, KNOWING that I'm never re-adding a
    // num that wasn't there in the first place.

    // For process inbox, deposit, and withdrawal, the client will remove from
    // issued list as soon as he
    // receives my acknowledgment OR rejection. He expects server (me) to
    // remove, so he signs a balance
    // agreement to that effect. (With the number removed from issued list.)
    //
    // Therefore, to verify the balance agreement, we remove it on our side as
    // well, so that they will match.
    // The picture thus formed is what would be correct assuming a successful
    // transaction. That way if
    // the transaction goes through, we have our signed receipt showing the new
    // state of things (without
    // which we would not permit the transaction to go through :)
    //
    // This allows the client side to then ACTUALLY remove the number when they
    // receive our response,
    // as well as permits me (server) to actually remove from issued list.
    //
    // If ANYTHING ELSE fails during this verify process (other than
    // processInbox, deposit, and withdraw)
    // then we have to ADD THE # AGAIN since we still don't have a valid
    // signature on that number. So
    // you'll see this code repeated a few times in reverse, down inside this
    // function. For example,
    //
    switch (TARGET_TRANSACTION.GetType()) {
    case OTTransaction::processInbox:
    case OTTransaction::withdrawal:
    case OTTransaction::deposit:
    case OTTransaction::payDividend:
    case OTTransaction::cancelCronItem:
    case OTTransaction::exchangeBasket:
        // We DID verify the issued num (above) but I'm still just being safe
        // here...
        // ... since theRemovedNym contains numbers being re-added, just wanna
        // make sure
        // they were there in the first place.
        //
        if (THE_NYM.RemoveIssuedNum(SERVER_ID,
                                    GetTransactionNum())) // doesn't save.
            theRemovedNym.AddIssuedNum(SERVER_ID, GetTransactionNum());
        break;

    case OTTransaction::transfer:
    case OTTransaction::marketOffer:
    case OTTransaction::paymentPlan:
    case OTTransaction::smartContract:
        // These, assuming success, do NOT remove an issued number. So no need
        // to anticipate setting up the list that way, to get a match.
        break;
    default:
        // Error
        otErr << "OTItem::" << __FUNCTION__
              << ": wrong target transaction type: "
              << TARGET_TRANSACTION.GetTypeString() << "\n";
        break;
    }
    int32_t nNumberOfTransactionNumbers1 = 0; // The Nym on this side
    int32_t nNumberOfTransactionNumbers2 = 0; // The Message Nym.

    String strMessageNym;

    // First, loop through the Nym on my side, and count how many numbers total
    // he has...
    //
    for (auto& it : THE_NYM.GetMapIssuedNum()) {
        std::string strServerID = it.first;
        dequeOfTransNums* pDeque = it.second;
        OT_ASSERT(nullptr != pDeque);

        const Identifier theServerID(strServerID.c_str());

        if (!(pDeque->empty()) && (theServerID == GetPurportedServerID())) {
            nNumberOfTransactionNumbers1 +=
                static_cast<int32_t>(pDeque->size());
            break; // There's only one, in this loop, that would/could/should
                   // match. (Therefore, break after finding it.)
        }
    } // for

    // Next, loop through theMessageNym, and count his numbers as well...
    // But ALSO verify that each one exists on THE_NYM, so that each individual
    // number is checked.
    GetAttachment(strMessageNym);
    OTPseudonym theMessageNym;

    if ((strMessageNym.GetLength() > 2) &&
        theMessageNym.LoadFromString(strMessageNym)) {
        for (auto& it : theMessageNym.GetMapIssuedNum()) {
            std::string strServerID = it.first;
            dequeOfTransNums* pDeque = it.second;
            OT_ASSERT(nullptr != pDeque);

            const Identifier theServerID(strServerID.c_str());
            const String OTstrServerID(theServerID);

            if (!(pDeque->empty()) && (theServerID == GetPurportedServerID())) {
                nNumberOfTransactionNumbers2 +=
                    static_cast<int32_t>(pDeque->size());

                for (uint32_t i = 0; i < pDeque->size(); i++) {
                    int64_t lTransactionNumber = pDeque->at(i);
                    if (false ==
                        THE_NYM.VerifyIssuedNum(OTstrServerID,
                                                lTransactionNumber)) // FAILURE
                    {
                        otOut << "OTItem::" << __FUNCTION__
                              << ": Issued transaction # " << lTransactionNumber
                              << " from Message Nym not found on this side.\n";

                        // I have to do this whenever I RETURN :-(
                        switch (TARGET_TRANSACTION.GetType()) {
                        case OTTransaction::processInbox:
                        case OTTransaction::withdrawal:
                        case OTTransaction::deposit:
                        case OTTransaction::payDividend:
                        case OTTransaction::cancelCronItem:
                        case OTTransaction::exchangeBasket:
                            // Should only actually iterate once, in this case.
                            for (int32_t j = 0;
                                 j < theRemovedNym.GetIssuedNumCount(
                                         GetPurportedServerID());
                                 j++) {
                                int64_t lTemp = theRemovedNym.GetIssuedNum(
                                    GetPurportedServerID(), j);

                                if (j > 0)
                                    otErr << "OTItem::" << __FUNCTION__
                                          << ": THIS SHOULD NOT HAPPEN.\n";
                                else if (false ==
                                         THE_NYM.AddIssuedNum(
                                             SERVER_ID, lTemp)) // doesn't save.
                                    otErr << "OTItem::" << __FUNCTION__
                                          << ": Failed adding issued number "
                                             "back to THE_NYM.\n";
                            }
                            break;

                        case OTTransaction::transfer:
                        case OTTransaction::marketOffer:
                        case OTTransaction::paymentPlan:
                        case OTTransaction::smartContract:
                            break;
                        default:
                            // Error
                            otErr << "OTItem::" << __FUNCTION__
                                  << ": wrong target transaction type: "
                                  << TARGET_TRANSACTION.GetTypeString() << "\n";
                            break;
                        }

                        return false;
                    }
                }      // for (numbers for a specific server.)
                break; // Only one server ID should match, so we can break after
                       // finding it.
            }          // If the server ID matches
        }              // for (deques of numbers for each server)
    }

    // Finally, verify that the counts match...
    if (nNumberOfTransactionNumbers1 != nNumberOfTransactionNumbers2) {
        otOut << "OTItem::" << __FUNCTION__
              << ": Transaction # Count mismatch: "
              << nNumberOfTransactionNumbers1 << " and "
              << nNumberOfTransactionNumbers2 << "\n";

        // I have to do this whenever I RETURN :-(
        switch (TARGET_TRANSACTION.GetType()) {
        case OTTransaction::processInbox:
        case OTTransaction::withdrawal:
        case OTTransaction::deposit:
        case OTTransaction::payDividend:
        case OTTransaction::cancelCronItem:
        case OTTransaction::exchangeBasket:
            // Should only actually iterate once, in this case.
            for (int32_t i = 0;
                 i < theRemovedNym.GetIssuedNumCount(GetPurportedServerID());
                 i++) {
                int64_t lTemp =
                    theRemovedNym.GetIssuedNum(GetPurportedServerID(), i);

                if (i > 0)
                    otErr << "OTItem::" << __FUNCTION__
                          << ": THIS SHOULD NOT HAPPEN.\n";
                else if (false ==
                         THE_NYM.AddIssuedNum(SERVER_ID,
                                              lTemp)) // doesn't save.
                    otErr << "OTItem::" << __FUNCTION__
                          << ": Failed adding issued number back to THE_NYM.\n";
            }
            break;

        case OTTransaction::transfer:
        case OTTransaction::marketOffer:
        case OTTransaction::paymentPlan:
        case OTTransaction::smartContract:
            break;
        default:
            // Error
            otErr << "OTItem::" << __FUNCTION__
                  << ": wrong target transaction type: "
                  << TARGET_TRANSACTION.GetTypeString() << "\n";
            break;
        }

        return false;
    }

    // By this point, I know the local Nym has the same number of transactions
    // as the message nym, and that
    // EVERY ONE OF THEM was found individually.

    // Might want to consider saving the Nym here.
    // Also want to save the latest signed receipt, since it VERIFIES.
    // Or maybe let caller decide?

    // I have to do this whenever I RETURN :-(
    // EVEN IF SUCCESS, we have only succeeded to verify the balance statement.
    // We must still go on to verify the transaction itself, and ONLY THEN will
    // we (possibly) remove the issued number from the list. And the decision
    // will
    // change from situation to situation, depending on the instrument.
    // Therefore I add it back here as well. We only fiddled with it in the
    // first place
    // in order to verify the balance statement. Done. So now let the other
    // pieces decide
    // their own logic from there.
    //
    switch (TARGET_TRANSACTION.GetType()) {
    case OTTransaction::processInbox:
    case OTTransaction::withdrawal:
    case OTTransaction::deposit:
    case OTTransaction::payDividend:
    case OTTransaction::cancelCronItem:
    case OTTransaction::exchangeBasket:
        // Should only actually iterate once, in this case.
        for (int32_t i = 0;
             i < theRemovedNym.GetIssuedNumCount(GetPurportedServerID()); i++) {
            int64_t lTemp =
                theRemovedNym.GetIssuedNum(GetPurportedServerID(), i);

            if (i > 0)
                otErr << "OTItem::" << __FUNCTION__
                      << ": THIS SHOULD NOT HAPPEN.\n";
            else if (false ==
                     THE_NYM.AddIssuedNum(SERVER_ID, lTemp)) // doesn't save.
                otErr << "OTItem::" << __FUNCTION__
                      << ": Failed adding issued number back to THE_NYM.\n";
        }
        break;

    case OTTransaction::transfer:
    case OTTransaction::marketOffer:
    case OTTransaction::paymentPlan:
    case OTTransaction::smartContract:
        break;
    default:
        // Error
        otErr << "OTItem::" << __FUNCTION__
              << ": wrong target transaction type: "
              << TARGET_TRANSACTION.GetTypeString() << "\n";
        break;
    }

    return true;
}

// You have to allocate the item on the heap and then pass it in as a reference.
// OTTransaction will take care of it from there and will delete it in
// destructor.
void OTItem::AddItem(OTItem& theItem)
{
    m_listItems.push_back(&theItem);
}

// While processing a transaction, you may wish to query it for items of a
// certain type.
OTItem* OTItem::GetItem(int32_t nIndex)
{
    int32_t nTempIndex = (-1);

    for (auto& it : m_listItems) {
        OTItem* pItem = it;
        OT_ASSERT(nullptr != pItem);

        nTempIndex++; // first iteration this becomes 0 here.

        if (nTempIndex == nIndex) return pItem;
    }

    return nullptr;
}

// While processing an item, you may wish to query it for sub-items
OTItem* OTItem::GetItemByTransactionNum(int64_t lTransactionNumber)
{
    for (auto& it : m_listItems) {
        OTItem* pItem = it;
        OT_ASSERT(nullptr != pItem);

        if (pItem->GetTransactionNum() == lTransactionNumber) return pItem;
    }

    return nullptr;
}

// Count the number of items that are IN REFERENCE TO some transaction#.
//
// Might want to change this so that it only counts ACCEPTED receipts.
//
int32_t OTItem::GetItemCountInRefTo(int64_t lReference)
{
    int32_t nCount = 0;

    for (auto& it : m_listItems) {
        OTItem* pItem = it;
        OT_ASSERT(nullptr != pItem);

        if (pItem->GetReferenceToNum() == lReference) nCount++;
    }

    return nCount;
}

// The final receipt item MAY be present, and co-relates to others that share
// its "in reference to" value. (Others such as marketReceipts and
// paymentReceipts.)
//
OTItem* OTItem::GetFinalReceiptItemByReferenceNum(int64_t lReferenceNumber)
{
    for (auto& it : m_listItems) {
        OTItem* pItem = it;
        OT_ASSERT(nullptr != pItem);

        if (OTItem::finalReceipt != pItem->GetType()) continue;
        if (pItem->GetReferenceToNum() == lReferenceNumber) return pItem;
    }

    return nullptr;
}

// For "OTItem::acceptTransaction"
//
bool OTItem::AddBlankNumbersToItem(const OTNumList& theAddition)
{
    return m_Numlist.Add(theAddition);
}

// Need to know the transaction number of the ORIGINAL transaction? Call this.
// virtual
int64_t OTItem::GetNumberOfOrigin()
{

    if (0 == m_lNumberOfOrigin) {
        switch (GetType()) {
        case acceptPending: // this item is a client-side acceptance of a
                            // pending transfer
        case rejectPending: // this item is a client-side rejection of a pending
                            // transfer
        case acceptCronReceipt:  // this item is a client-side acceptance of a
                                 // cron receipt in his inbox.
        case acceptItemReceipt:  // this item is a client-side acceptance of an
                                 // item receipt in his inbox.
        case disputeCronReceipt: // this item is a client dispute of a cron
                                 // receipt in his inbox.
        case disputeItemReceipt: // this item is a client dispute of an item
                                 // receipt in his inbox.

        case acceptFinalReceipt:   // this item is a client-side acceptance of a
                                   // final receipt in his inbox. (All related
                                   // receipts must also be closed!)
        case acceptBasketReceipt:  // this item is a client-side acceptance of a
                                   // basket receipt in his inbox.
        case disputeFinalReceipt:  // this item is a client-side rejection of a
                                   // final receipt in his inbox. (All related
                                   // receipts must also be closed!)
        case disputeBasketReceipt: // this item is a client-side rejection of a
                                   // basket receipt in his inbox.

            otErr << __FUNCTION__ << ": In this case, you can't calculate the "
                                     "origin number, you must set it "
                                     "explicitly.\n";
            // Comment this out later so people can't use it to crash the
            // server:
            OT_FAIL_MSG("In this case, you can't calculate the origin number, "
                        "you must set it explicitly.");
            break;
        default:
            break;
        }

        CalculateNumberOfOrigin();
    }

    return m_lNumberOfOrigin;
}

// virtual
void OTItem::CalculateNumberOfOrigin()
{
    switch (GetType()) {
    case acceptTransaction:   // this item is a client-side acceptance of a
                              // transaction number (a blank) in my Nymbox
    case atAcceptTransaction: // server reply
    case acceptMessage: // this item is a client-side acceptance of a message in
                        // my Nymbox
    case atAcceptMessage: // server reply
    case acceptNotice:    // this item is a client-side acceptance of a server
                          // notification in my Nymbox
    case atAcceptNotice:  // server reply
    case replyNotice:   // server notice of a reply that nym should have already
                        // received as a response to a request. (Copy dropped in
                        // nymbox.)
    case successNotice: // server notice dropped into nymbox as result of a
                        // transaction# being successfully signed out.
    case notice: // server notice dropped into nymbox as result of a smart
                 // contract processing.
    case transferReceipt: // Currently don't create an OTItem for transfer
                          // receipt in inbox. Used only for inbox report.
    case chequeReceipt:   // Currently don't create an OTItem for cheque receipt
                          // in inbox. Used only for inbox report.
    case voucherReceipt: // Currently don't create an OTItem for voucher receipt
                         // in inbox. Used only for inbox report.

        SetNumberOfOrigin(0); // Not applicable.
        break;

    case acceptPending: // this item is a client-side acceptance of a pending
                        // transfer
    case rejectPending: // this item is a client-side rejection of a pending
                        // transfer
    case acceptCronReceipt:  // this item is a client-side acceptance of a cron
                             // receipt in his inbox.
    case acceptItemReceipt:  // this item is a client-side acceptance of an item
                             // receipt in his inbox.
    case disputeCronReceipt: // this item is a client dispute of a cron receipt
                             // in his inbox.
    case disputeItemReceipt: // this item is a client dispute of an item receipt
                             // in his inbox.

    case acceptFinalReceipt: // this item is a client-side acceptance of a final
                             // receipt in his inbox. (All related receipts must
                             // also be closed!)
    case acceptBasketReceipt: // this item is a client-side acceptance of a
                              // basket receipt in his inbox.
    case disputeFinalReceipt: // this item is a client-side rejection of a final
                              // receipt in his inbox. (All related receipts
                              // must also be closed!)
    case disputeBasketReceipt: // this item is a client-side rejection of a
                               // basket receipt in his inbox.

        otErr << __FUNCTION__ << ": In this case, you can't calculate the "
                                 "origin number, you must set it explicitly.\n";
        SetNumberOfOrigin(0); // Not applicable.
        // Comment this out later so people can't use it to crash the server:
        OT_FAIL_MSG("In this case, you can't calculate the origin number, you "
                    "must set it explicitly.");
        break;

    case marketReceipt: // server receipt dropped into inbox as result of market
                        // trading. Also used in inbox report.
    case paymentReceipt: // server receipt dropped into an inbox as result of
                         // payment occuring. Also used in inbox report.
    case finalReceipt:   // server receipt dropped into inbox / nymbox as result
                         // of cron item expiring or being canceled.
    case basketReceipt:  // server receipt dropped into inbox as result of a
                         // basket exchange.

        SetNumberOfOrigin(GetReferenceToNum()); // pending is in
                                                // reference to the
                                                // original
                                                // transfer.
        break;

    case depositCheque: // this item is a request to deposit a cheque.
    {
        Cheque theCheque;
        String strAttachment;
        GetAttachment(strAttachment);

        if (!theCheque.LoadContractFromString(strAttachment))
            otErr << __FUNCTION__ << ": ERROR loading cheque from string:\n"
                  << strAttachment << "\n";
        else
            SetNumberOfOrigin(theCheque.GetTransactionNum());
    } break;

    case atDepositCheque:     // this item is a server response to that request.
    case atAcceptPending:     // server reply to acceptPending.
    case atRejectPending:     // server reply to rejectPending.
    case atAcceptCronReceipt: // this item is a server reply to that acceptance.
    case atAcceptItemReceipt: // this item is a server reply to that acceptance.
    case atDisputeCronReceipt:   // Server reply to dispute message.
    case atDisputeItemReceipt:   // Server reply to dispute message.
    case atAcceptFinalReceipt:   // server reply
    case atAcceptBasketReceipt:  // server reply
    case atDisputeFinalReceipt:  // server reply
    case atDisputeBasketReceipt: // server reply
    {
        String strReference;
        GetReferenceString(strReference);

        // "In reference to" number is my original deposit trans#, which I use
        // here to load my
        // original depositCheque item, which I use to get the cheque, which
        // contains the number
        // of origin as its transaction number.
        //
        std::unique_ptr<OTItem> pOriginalItem(OTItem::CreateItemFromString(
            strReference, GetPurportedServerID(), GetReferenceToNum()));
        OT_ASSERT(nullptr != pOriginalItem);

        if (((m_Type == atDepositCheque) &&
             (OTItem::depositCheque != pOriginalItem->GetType())) ||
            ((m_Type == atAcceptPending) &&
             (OTItem::acceptPending != pOriginalItem->GetType())) ||
            ((m_Type == atRejectPending) &&
             (OTItem::rejectPending != pOriginalItem->GetType())) ||
            ((m_Type == atAcceptCronReceipt) &&
             (OTItem::acceptCronReceipt != pOriginalItem->GetType())) ||
            ((m_Type == atAcceptItemReceipt) &&
             (OTItem::acceptItemReceipt != pOriginalItem->GetType())) ||
            ((m_Type == atDisputeCronReceipt) &&
             (OTItem::disputeCronReceipt != pOriginalItem->GetType())) ||
            ((m_Type == atDisputeItemReceipt) &&
             (OTItem::disputeItemReceipt != pOriginalItem->GetType())) ||
            ((m_Type == atAcceptFinalReceipt) &&
             (OTItem::acceptFinalReceipt != pOriginalItem->GetType())) ||
            ((m_Type == atAcceptBasketReceipt) &&
             (OTItem::acceptBasketReceipt != pOriginalItem->GetType())) ||
            ((m_Type == atDisputeFinalReceipt) &&
             (OTItem::disputeFinalReceipt != pOriginalItem->GetType())) ||
            ((m_Type == atDisputeBasketReceipt) &&
             (OTItem::disputeBasketReceipt != pOriginalItem->GetType()))) {
            String strType;
            pOriginalItem->GetTypeString(strType);
            otErr << __FUNCTION__
                  << ": ERROR: Wrong item type as 'in reference to' string on "
                  << strType << " item.\n";
            SetNumberOfOrigin(0);
            return;
        }

        // Else:
        SetNumberOfOrigin(pOriginalItem->GetNumberOfOrigin());
    } break;

    // FEEs
    case serverfee: // this item is a fee from the transaction server (per
                    // contract)
    case atServerfee:
    case issuerfee: // this item is a fee from the issuer (per contract)
    case atIssuerfee:

    // INFO (BALANCE, HASH, etc) these are still all messages with replies.
    case balanceStatement: // this item is a statement of balance. (For asset
                           // account.)
    case atBalanceStatement:
    case transactionStatement: // this item is a transaction statement. (For Nym
                               // -- which numbers are assigned to him.)
    case atTransactionStatement:

    // TRANSFER
    case transfer:   // This item is an outgoing transfer, probably part of an
                     // outoing transaction.
    case atTransfer: // Server reply.

    // CASH WITHDRAWAL / DEPOSIT
    case withdrawal: // this item is a cash withdrawal (of chaumian blinded
                     // tokens)
    case atWithdrawal:
    case deposit: // this item is a cash deposit (of a purse containing blinded
                  // tokens.)
    case atDeposit:

    // CHEQUES AND VOUCHERS
    case withdrawVoucher: // this item is a request to purchase a voucher (a
                          // cashier's cheque)
    case atWithdrawVoucher:

    // PAYING DIVIDEND ON SHARES OF STOCK
    case payDividend:   // this item is a request to pay a dividend.
    case atPayDividend: // the server reply to that request.

    // TRADING ON MARKETS
    case marketOffer:   // this item is an offer to be put on a market.
    case atMarketOffer: // server reply or updated notification regarding a
                        // market offer.

    // PAYMENT PLANS
    case paymentPlan:   // this item is a new payment plan
    case atPaymentPlan: // server reply or updated notification regarding a
                        // payment plan.

    // SMART CONTRACTS
    case smartContract:   // this item is a new smart contract
    case atSmartContract: // server reply or updated notification regarding a
                          // smart contract.

    // CANCELLING: Market Offers and Payment Plans.
    case cancelCronItem:   // this item is intended to cancel a market offer or
                           // payment plan.
    case atCancelCronItem: // reply from the server regarding said cancellation.

    // EXCHANGE IN/OUT OF A BASKET CURRENCY
    case exchangeBasket:   // this item is an exchange in/out of a basket
                           // currency.
    case atExchangeBasket: // reply from the server regarding said exchange.

    default:
        SetNumberOfOrigin(GetTransactionNum());
        break;
    } // switch
}

void OTItem::GetAttachment(String& theStr) const
{
    m_ascAttachment.GetString(theStr);
}

void OTItem::SetAttachment(const String& theStr)
{
    m_ascAttachment.SetString(theStr);
}

#define MINIMUM_CLEARTEXT_SIZE_OTASCIIARMOR 100

#define OTASSCIIARMOR_PADDING_TEXT                                             \
    " IGNORE -- NOTE PADDING -- IGNORE -- NOTE PADDING \nIGNORE -- NOTE "      \
    "PADDING -- IGNORE -- NOTE PADDING \n"

void OTItem::SetNote(const String& theStr)
{
    if (theStr.Exists() && theStr.GetLength() > 2) {
        String theString(theStr);
        if (theStr.GetLength() < MINIMUM_CLEARTEXT_SIZE_OTASCIIARMOR) {
            String strPadding(OTASSCIIARMOR_PADDING_TEXT);

            theString.Concatenate(strPadding);
        }

        m_ascNote.SetString(theString);
    }
    else {
        m_ascNote.Release();
    }
}

void OTItem::GetNote(String& theStr) const
{
    if (m_ascNote.GetLength() > 2) {
        m_ascNote.GetString(theStr);

        if (theStr.Contains(OTASSCIIARMOR_PADDING_TEXT))
            theStr.Truncate(theStr.GetLength() -
                            MINIMUM_CLEARTEXT_SIZE_OTASCIIARMOR);
    }
    else {
        theStr.Release();
    }
}

// Let's say you have created a transaction, and you are creating an item to put
// into it.
// Well in that case, you don't care to verify that the real IDs match the
// purported IDs, since
// you are creating this item yourself, not verifying it from someone else.
// Use this function to create the new Item before you add it to your new
// Transaction.
OTItem* OTItem::CreateItemFromTransaction(const OTTransaction& theOwner,
                                          OTItem::itemType theType,
                                          const Identifier* pDestinationAcctID)
{
    OTItem* pItem =
        new OTItem(theOwner.GetUserID(), theOwner, theType, pDestinationAcctID);

    if (pItem) {
        pItem->SetPurportedAccountID(theOwner.GetPurportedAccountID());
        pItem->SetPurportedServerID(theOwner.GetPurportedServerID());
        return pItem;
    }
    return nullptr;
}

// Sometimes I don't know user ID of the originator, or the account ID of the
// originator,
// until after I have loaded the item. It's simply impossible to set those
// values ahead
// of time, sometimes. In those cases, we set the values appropriately but then
// we need
// to verify that the user ID is actually the owner of the AccountID. TOdo that.
OTItem* OTItem::CreateItemFromString(const String& strItem,
                                     const Identifier& theServerID,
                                     int64_t lTransactionNumber)
{
    if (!strItem.Exists()) {
        otErr << "OTItem::CreateItemFromString: strItem is empty. (Expected an "
                 "item.)\n";
        return nullptr;
    }

    OTItem* pItem = new OTItem();

    // So when it loads its own server ID, we can compare to this one.
    pItem->SetRealServerID(theServerID);

    // This loads up the purported account ID and the user ID.
    if (pItem->LoadContractFromString(strItem)) {
        const Identifier& ACCOUNT_ID = pItem->GetPurportedAccountID();
        pItem->SetRealAccountID(ACCOUNT_ID); // I do this because it's all we've
                                             // got in this case. It's what's in
                                             // the
        // xml, so it must be right. If it's a lie, the signature will fail or
        // the
        // user will not show as the owner of that account. But remember, the
        // server
        // sent the message in the first place.

        pItem->SetTransactionNum(lTransactionNumber);

        if (pItem->VerifyContractID()) // this compares purported and real
                                       // account IDs, as well as server IDs.
        {
            return pItem;
        }
        else {
            delete pItem;
            pItem = nullptr;
        }
    }

    return nullptr;
}

void OTItem::InitItem()
{
    m_lAmount = 0; // Accounts default to ZERO.  They can only change that
                   // amount by receiving from another account.
    m_Type = OTItem::error_state;
    m_Status =
        OTItem::request; // (Unless an issuer account, which can create currency
    m_lNewOutboxTransNum = 0; // When the user puts a "1" in his outbox for a
                              // balance agreement (since he doesn't know what
                              // trans# the actual outbox item
    // will have if the transaction is successful, since the server hasn't
    // issued it yet) then the balance receipt will have 1 in
    // the user's portion for that outbox transaction, and the balance receipt
    // will also have, say, #34 (the actual number) here
    // in this variable, in the server's reply portion of that same receipt.

    m_lClosingTransactionNo = 0;

    m_strContractType = "TRANSACTION ITEM"; // CONTRACT, MESSAGE, TRANSACTION,
                                            // LEDGER, TRANSACTION ITEM
}

// this one is private (I hope to keep it that way.)
// probvably not actually. If I end up back here, it's because
// sometimes I dont' WANT to assign the stuff, but leave it blank
// because I'm about to load it.
OTItem::OTItem()
    : OTTransactionType()
    , m_lAmount(0)
    , m_Type(OTItem::error_state)
    , m_Status(OTItem::request)
    , m_lNewOutboxTransNum(0)
    , m_lClosingTransactionNo(0)
{
    InitItem();
}

// From owner we can get acct ID, server ID, and transaction Num
OTItem::OTItem(const Identifier& theUserID, const OTTransaction& theOwner)
    : OTTransactionType(theUserID, theOwner.GetRealAccountID(),
                        theOwner.GetRealServerID(),
                        theOwner.GetTransactionNum())
    , m_lAmount(0)
    , m_Type(OTItem::error_state)
    , m_Status(OTItem::request)
    , m_lNewOutboxTransNum(0)
    , m_lClosingTransactionNo(0)
{
    InitItem();
}

// From owner we can get acct ID, server ID, and transaction Num
OTItem::OTItem(const Identifier& theUserID, const OTItem& theOwner)
    : OTTransactionType(theUserID, theOwner.GetRealAccountID(),
                        theOwner.GetRealServerID(),
                        theOwner.GetTransactionNum())
    , m_lAmount(0)
    , m_Type(OTItem::error_state)
    , m_Status(OTItem::request)
    , m_lNewOutboxTransNum(0)
    , m_lClosingTransactionNo(0)
{
    InitItem();
}

OTItem::OTItem(const Identifier& theUserID, const OTTransaction& theOwner,
               OTItem::itemType theType, const Identifier* pDestinationAcctID)
    : OTTransactionType(theUserID, theOwner.GetRealAccountID(),
                        theOwner.GetRealServerID(),
                        theOwner.GetTransactionNum())
    , m_lAmount(0)
    , m_Type(OTItem::error_state)
    , m_Status(OTItem::request)
    , m_lNewOutboxTransNum(0)
    , m_lClosingTransactionNo(0)
{
    InitItem();

    m_Type = theType; // This has to be below the InitItem() call that appears
                      // just above

    // Most transactions items don't HAVE a "to" account, just a primary
    // account.
    // (If you deposit, or withdraw, you don't need a "to" account.)
    // But for the ones that do, you can pass the "to" account's ID in
    // as a pointer, and we'll set that too....
    if (nullptr != pDestinationAcctID) {
        m_AcctToID = *pDestinationAcctID;
    }
}

OTItem::~OTItem()
{
    Release_Item();
}

void OTItem::Release()
{
    Release_Item();

    ot_super::Release();
}

void OTItem::Release_Item()
{
    ReleaseItems();

    m_AcctToID.clear();
    m_lAmount = 0;
    m_lNewOutboxTransNum = 0;
    m_lClosingTransactionNo = 0;
}

void OTItem::ReleaseItems()
{

    while (!m_listItems.empty()) {
        OTItem* pItem = m_listItems.front();
        m_listItems.pop_front();
        delete pItem;
    }
}

OTItem::itemType OTItem::GetItemTypeFromString(const String& strType)
{
    OTItem::itemType theType = OTItem::error_state;

    if (strType.Compare("transfer"))
        theType = OTItem::transfer;
    else if (strType.Compare("atTransfer"))
        theType = OTItem::atTransfer;

    else if (strType.Compare("acceptTransaction"))
        theType = OTItem::acceptTransaction;
    else if (strType.Compare("atAcceptTransaction"))
        theType = OTItem::atAcceptTransaction;

    else if (strType.Compare("acceptMessage"))
        theType = OTItem::acceptMessage;
    else if (strType.Compare("atAcceptMessage"))
        theType = OTItem::atAcceptMessage;

    else if (strType.Compare("acceptNotice"))
        theType = OTItem::acceptNotice;
    else if (strType.Compare("atAcceptNotice"))
        theType = OTItem::atAcceptNotice;

    else if (strType.Compare("acceptPending"))
        theType = OTItem::acceptPending;
    else if (strType.Compare("atAcceptPending"))
        theType = OTItem::atAcceptPending;
    else if (strType.Compare("rejectPending"))
        theType = OTItem::rejectPending;
    else if (strType.Compare("atRejectPending"))
        theType = OTItem::atRejectPending;

    else if (strType.Compare("acceptCronReceipt"))
        theType = OTItem::acceptCronReceipt;
    else if (strType.Compare("atAcceptCronReceipt"))
        theType = OTItem::atAcceptCronReceipt;
    else if (strType.Compare("disputeCronReceipt"))
        theType = OTItem::disputeCronReceipt;
    else if (strType.Compare("atDisputeCronReceipt"))
        theType = OTItem::atDisputeCronReceipt;
    else if (strType.Compare("acceptItemReceipt"))
        theType = OTItem::acceptItemReceipt;
    else if (strType.Compare("atAcceptItemReceipt"))
        theType = OTItem::atAcceptItemReceipt;
    else if (strType.Compare("disputeItemReceipt"))
        theType = OTItem::disputeItemReceipt;
    else if (strType.Compare("atDisputeItemReceipt"))
        theType = OTItem::atDisputeItemReceipt;

    else if (strType.Compare("acceptFinalReceipt"))
        theType = OTItem::acceptFinalReceipt;
    else if (strType.Compare("atAcceptFinalReceipt"))
        theType = OTItem::atAcceptFinalReceipt;
    else if (strType.Compare("disputeFinalReceipt"))
        theType = OTItem::disputeFinalReceipt;
    else if (strType.Compare("atDisputeFinalReceipt"))
        theType = OTItem::atDisputeFinalReceipt;

    else if (strType.Compare("acceptBasketReceipt"))
        theType = OTItem::acceptBasketReceipt;
    else if (strType.Compare("atAcceptBasketReceipt"))
        theType = OTItem::atAcceptBasketReceipt;
    else if (strType.Compare("disputeBasketReceipt"))
        theType = OTItem::disputeBasketReceipt;
    else if (strType.Compare("atDisputeBasketReceipt"))
        theType = OTItem::atDisputeBasketReceipt;

    else if (strType.Compare("serverfee"))
        theType = OTItem::serverfee;
    else if (strType.Compare("atServerfee"))
        theType = OTItem::atServerfee;
    else if (strType.Compare("issuerfee"))
        theType = OTItem::issuerfee;
    else if (strType.Compare("atIssuerfee"))
        theType = OTItem::atIssuerfee;

    else if (strType.Compare("balanceStatement"))
        theType = OTItem::balanceStatement;
    else if (strType.Compare("atBalanceStatement"))
        theType = OTItem::atBalanceStatement;
    else if (strType.Compare("transactionStatement"))
        theType = OTItem::transactionStatement;
    else if (strType.Compare("atTransactionStatement"))
        theType = OTItem::atTransactionStatement;

    else if (strType.Compare("withdrawal"))
        theType = OTItem::withdrawal;
    else if (strType.Compare("atWithdrawal"))
        theType = OTItem::atWithdrawal;
    else if (strType.Compare("deposit"))
        theType = OTItem::deposit;
    else if (strType.Compare("atDeposit"))
        theType = OTItem::atDeposit;

    else if (strType.Compare("withdrawVoucher"))
        theType = OTItem::withdrawVoucher;
    else if (strType.Compare("atWithdrawVoucher"))
        theType = OTItem::atWithdrawVoucher;
    else if (strType.Compare("depositCheque"))
        theType = OTItem::depositCheque;
    else if (strType.Compare("atDepositCheque"))
        theType = OTItem::atDepositCheque;

    else if (strType.Compare("payDividend"))
        theType = OTItem::payDividend;
    else if (strType.Compare("atPayDividend"))
        theType = OTItem::atPayDividend;

    else if (strType.Compare("marketOffer"))
        theType = OTItem::marketOffer;
    else if (strType.Compare("atMarketOffer"))
        theType = OTItem::atMarketOffer;

    else if (strType.Compare("paymentPlan"))
        theType = OTItem::paymentPlan;
    else if (strType.Compare("atPaymentPlan"))
        theType = OTItem::atPaymentPlan;

    else if (strType.Compare("smartContract"))
        theType = OTItem::smartContract;
    else if (strType.Compare("atSmartContract"))
        theType = OTItem::atSmartContract;

    else if (strType.Compare("cancelCronItem"))
        theType = OTItem::cancelCronItem;
    else if (strType.Compare("atCancelCronItem"))
        theType = OTItem::atCancelCronItem;

    else if (strType.Compare("exchangeBasket"))
        theType = OTItem::exchangeBasket;
    else if (strType.Compare("atExchangeBasket"))
        theType = OTItem::atExchangeBasket;

    else if (strType.Compare("chequeReceipt"))
        theType = OTItem::chequeReceipt;
    else if (strType.Compare("voucherReceipt"))
        theType = OTItem::voucherReceipt;
    else if (strType.Compare("marketReceipt"))
        theType = OTItem::marketReceipt;
    else if (strType.Compare("paymentReceipt"))
        theType = OTItem::paymentReceipt;
    else if (strType.Compare("transferReceipt"))
        theType = OTItem::transferReceipt;

    else if (strType.Compare("finalReceipt"))
        theType = OTItem::finalReceipt;
    else if (strType.Compare("basketReceipt"))
        theType = OTItem::basketReceipt;

    else if (strType.Compare("replyNotice"))
        theType = OTItem::replyNotice;
    else if (strType.Compare("successNotice"))
        theType = OTItem::successNotice;
    else if (strType.Compare("notice"))
        theType = OTItem::notice;

    else
        theType = OTItem::error_state;

    return theType;
}

// return -1 if error, 0 if nothing, and 1 if the node was processed.
int32_t OTItem::ProcessXMLNode(irr::io::IrrXMLReader*& xml)
{
    if (!strcmp("item", xml->getNodeName())) {
        String strType, strStatus;

        strType = xml->getAttributeValue("type");
        strStatus = xml->getAttributeValue("status");

        // Type
        m_Type = GetItemTypeFromString(strType); // just above.

        // Status
        if (strStatus.Compare("request"))
            m_Status = OTItem::request;
        else if (strStatus.Compare("acknowledgement"))
            m_Status = OTItem::acknowledgement;
        else if (strStatus.Compare("rejection"))
            m_Status = OTItem::rejection;
        else
            m_Status = OTItem::error_status;

        String strAcctFromID, strAcctToID, strServerID, strUserID,
            strOutboxNewTransNum;

        strAcctFromID = xml->getAttributeValue("fromAccountID");
        strAcctToID = xml->getAttributeValue("toAccountID");
        strServerID = xml->getAttributeValue("serverID");
        strUserID = xml->getAttributeValue("userID");

        strOutboxNewTransNum = xml->getAttributeValue("outboxNewTransNum");

        if (strOutboxNewTransNum.Exists())
            m_lNewOutboxTransNum = strOutboxNewTransNum.ToLong();

        // an OTTransaction::blank may now contain 20 or 100 new numbers.
        // Therefore, the OTItem::acceptTransaction must contain the same list,
        // otherwise you haven't actually SIGNED for the list, have you!
        //
        if (OTItem::acceptTransaction == m_Type) {
            const String strTotalList =
                xml->getAttributeValue("totalListOfNumbers");
            m_Numlist.Release();

            if (strTotalList.Exists())
                m_Numlist.Add(strTotalList); // (Comma-separated list of numbers
                                             // now becomes std::set<int64_t>.)
        }

        Identifier ACCOUNT_ID(strAcctFromID), SERVER_ID(strServerID),
            DESTINATION_ACCOUNT(strAcctToID), USER_ID(strUserID);

        SetPurportedAccountID(ACCOUNT_ID); // OTTransactionType::m_AcctID  the
                                           // PURPORTED Account ID
        SetPurportedServerID(SERVER_ID);   // OTTransactionType::m_AcctServerID
                                           // the PURPORTED Server ID
        SetDestinationAcctID(DESTINATION_ACCOUNT);
        SetUserID(USER_ID);

        if (!m_bLoadSecurely) {
            SetRealAccountID(ACCOUNT_ID);
            SetRealServerID(SERVER_ID);
        }

        String strTemp;

        strTemp = xml->getAttributeValue("numberOfOrigin");
        if (strTemp.Exists()) SetNumberOfOrigin(strTemp.ToLong());

        strTemp = xml->getAttributeValue("transactionNum");
        if (strTemp.Exists()) SetTransactionNum(strTemp.ToLong());

        strTemp = xml->getAttributeValue("inReferenceTo");
        if (strTemp.Exists()) SetReferenceToNum(strTemp.ToLong());

        m_lAmount = String::StringToLong(xml->getAttributeValue("amount"));

        otLog3 << "Loaded transaction Item, transaction num "
               << GetTransactionNum()
               << ", In Reference To: " << GetReferenceToNum()
               << ", type: " << strType << ", status: " << strStatus << "\n";
        //                "fromAccountID:\n%s\n UserID:\n%s\n toAccountID:\n%s\n
        // serverID:\n%s\n----------\n",
        //                strAcctFromID.Get(), strUserID.Get(),
        // strAcctToID.Get(), strServerID.Get()

        return 1;
    }
    else if (!strcmp("note", xml->getNodeName())) {
        if (!Contract::LoadEncodedTextField(xml, m_ascNote)) {
            otErr << "Error in OTItem::ProcessXMLNode: note field without "
                     "value.\n";
            return (-1); // error condition
        }

        return 1;
    }
    else if (!strcmp("inReferenceTo", xml->getNodeName())) {
        if (false == Contract::LoadEncodedTextField(xml, m_ascInReferenceTo)) {
            otErr << "Error in OTItem::ProcessXMLNode: inReferenceTo field "
                     "without value.\n";
            return (-1); // error condition
        }

        return 1;
    }
    else if (!strcmp("attachment", xml->getNodeName())) {
        if (!Contract::LoadEncodedTextField(xml, m_ascAttachment)) {
            otErr << "Error in OTItem::ProcessXMLNode: attachment field "
                     "without value.\n";
            return (-1); // error condition
        }

        return 1;
    }
    else if (!strcmp("transactionReport", xml->getNodeName())) {
        if ((OTItem::balanceStatement == m_Type) ||
            (OTItem::atBalanceStatement == m_Type)) {
            // Notice it initializes with the wrong transaction number, in this
            // case.
            // That's okay, because I'm setting it below with
            // pItem->SetTransactionNum...
            OTItem* pItem = new OTItem(GetUserID(), *this); // But I've also got
                                                            // ITEM types with
                                                            // the same names...
            // That way, it will translate the string and set the type
            // correctly.
            OT_ASSERT(nullptr != pItem); // That way I can use each item to
                                         // REPRESENT an inbox transaction

            // Type
            String strType;
            strType = xml->getAttributeValue(
                "type"); // it's reading a TRANSACTION type: chequeReceipt,
                         // voucherReceipt, marketReceipt, or paymentReceipt.
                         // But I also have the same names for item types.

            pItem->SetType(GetItemTypeFromString(
                strType)); // It's actually translating a transaction type to an
                           // item type. (Same names in the case of the 3
                           // receipts that matter for inbox reports for balance
                           // agreements.)

            pItem->SetAmount(
                String::StringToLong(xml->getAttributeValue("adjustment")));

            // Status
            pItem->SetStatus(OTItem::acknowledgement); // I don't need this, but
                                                       // I'd rather it not say
                                                       // error state. This way
                                                       // if it changes to
                                                       // error_state later, I
                                                       // know I had a problem.

            String strAccountID, strServerID, strUserID;

            strAccountID = xml->getAttributeValue("accountID");
            strServerID = xml->getAttributeValue("serverID");
            strUserID = xml->getAttributeValue("userID");

            Identifier ACCOUNT_ID(strAccountID), SERVER_ID(strServerID),
                USER_ID(strUserID);

            pItem->SetPurportedAccountID(
                ACCOUNT_ID); // OTTransactionType::m_AcctID  the PURPORTED
                             // Account ID
            pItem->SetPurportedServerID(
                SERVER_ID); // OTTransactionType::m_AcctServerID the PURPORTED
                            // Server ID
            pItem->SetUserID(USER_ID);

            String strTemp;

            strTemp = xml->getAttributeValue("numberOfOrigin");
            if (strTemp.Exists()) pItem->SetNumberOfOrigin(strTemp.ToLong());

            strTemp = xml->getAttributeValue("transactionNum");
            if (strTemp.Exists()) pItem->SetTransactionNum(strTemp.ToLong());

            strTemp = xml->getAttributeValue("inReferenceTo");
            if (strTemp.Exists()) pItem->SetReferenceToNum(strTemp.ToLong());

            strTemp = xml->getAttributeValue(
                "closingTransactionNum"); // only used in the inbox report for
                                          // balance agreement.
            if (strTemp.Exists()) pItem->SetClosingNum(strTemp.ToLong());

            AddItem(*pItem); // <======= adding to list.

            otLog3 << "Loaded transactionReport Item, transaction num "
                   << pItem->GetTransactionNum()
                   << ", In Reference To: " << pItem->GetReferenceToNum()
                   << ", type: " << strType << "\n";
            //                         "fromAccountID:\n%s\n UserID:\n%s\n
            // toAccountID:\n%s\n serverID:\n%s\n----------\n",
            //                         strAcctFromID.Get(), strUserID.Get(),
            // strAcctToID.Get(), strServerID.Get()
        }
        else {
            otErr << "Outbox hash in item wrong type (expected "
                     "balanceStatement or atBalanceStatement.\n";
        }

        return 1;
    }

    return 0;
}

// Used in balance agreement, part of the inbox report.
int64_t OTItem::GetClosingNum() const
{
    return m_lClosingTransactionNo;
}

void OTItem::SetClosingNum(int64_t lClosingNum)
{
    m_lClosingTransactionNo = lClosingNum;
}

void OTItem::GetStringFromType(OTItem::itemType theType, String& strType)
{
    switch (theType) {
    case OTItem::transfer:
        strType.Set("transfer");
        break;
    case OTItem::acceptTransaction:
        strType.Set("acceptTransaction");
        break;
    case OTItem::acceptMessage:
        strType.Set("acceptMessage");
        break;
    case OTItem::acceptNotice:
        strType.Set("acceptNotice");
        break;
    case OTItem::acceptPending:
        strType.Set("acceptPending");
        break;
    case OTItem::rejectPending:
        strType.Set("rejectPending");
        break;
    case OTItem::acceptCronReceipt:
        strType.Set("acceptCronReceipt");
        break;
    case OTItem::disputeCronReceipt:
        strType.Set("disputeCronReceipt");
        break;
    case OTItem::acceptItemReceipt:
        strType.Set("acceptItemReceipt");
        break;
    case OTItem::disputeItemReceipt:
        strType.Set("disputeItemReceipt");
        break;
    case OTItem::acceptFinalReceipt:
        strType.Set("acceptFinalReceipt");
        break;
    case OTItem::acceptBasketReceipt:
        strType.Set("acceptBasketReceipt");
        break;
    case OTItem::disputeFinalReceipt:
        strType.Set("disputeFinalReceipt");
        break;
    case OTItem::disputeBasketReceipt:
        strType.Set("disputeBasketReceipt");
        break;
    case OTItem::serverfee:
        strType.Set("serverfee");
        break;
    case OTItem::issuerfee:
        strType.Set("issuerfee");
        break;
    case OTItem::withdrawal:
        strType.Set("withdrawal");
        break;
    case OTItem::deposit:
        strType.Set("deposit");
        break;
    case OTItem::withdrawVoucher:
        strType.Set("withdrawVoucher");
        break;
    case OTItem::depositCheque:
        strType.Set("depositCheque");
        break;
    case OTItem::payDividend:
        strType.Set("payDividend");
        break;
    case OTItem::marketOffer:
        strType.Set("marketOffer");
        break;
    case OTItem::paymentPlan:
        strType.Set("paymentPlan");
        break;
    case OTItem::smartContract:
        strType.Set("smartContract");
        break;
    case OTItem::balanceStatement:
        strType.Set("balanceStatement");
        break;
    case OTItem::transactionStatement:
        strType.Set("transactionStatement");
        break;

    case OTItem::cancelCronItem:
        strType.Set("cancelCronItem");
        break;
    case OTItem::exchangeBasket:
        strType.Set("exchangeBasket");
        break;

    case OTItem::atCancelCronItem:
        strType.Set("atCancelCronItem");
        break;
    case OTItem::atExchangeBasket:
        strType.Set("atExchangeBasket");
        break;

    case OTItem::chequeReceipt: // used for inbox statements in balance
                                // agreement.
        strType.Set("chequeReceipt");
        break;
    case OTItem::voucherReceipt: // used for inbox statements in balance
                                 // agreement.
        strType.Set("voucherReceipt");
        break;
    case OTItem::marketReceipt: // used as market receipt, and also for inbox
                                // statement containing market receipt will use
                                // this as well.
        strType.Set("marketReceipt");
        break;
    case OTItem::paymentReceipt: // used as payment receipt, also used in inbox
                                 // statement as payment receipt.
        strType.Set("paymentReceipt");
        break;
    case OTItem::transferReceipt: // used in inbox statement as transfer
                                  // receipt.
        strType.Set("transferReceipt");
        break;

    case OTItem::finalReceipt: // used for final receipt. Also used in inbox
                               // statement as final receipt. (For expiring or
                               // cancelled Cron Item.)
        strType.Set("finalReceipt");
        break;
    case OTItem::basketReceipt: // used in inbox statement as basket receipt.
                                // (For exchange.)
        strType.Set("basketReceipt");
        break;

    case OTItem::notice: // used in Nymbox statement as notification from
                         // server.
        strType.Set("notice");
        break;
    case OTItem::replyNotice: // some server replies (to your request) have a
                              // copy dropped into your nymbox, to make sure you
                              // received it.
        strType.Set("replyNotice");
        break;
    case OTItem::successNotice: // used in Nymbox statement as notification from
                                // server of successful sign-out of a trans#.
        strType.Set("successNotice");
        break;

    case OTItem::atTransfer:
        strType.Set("atTransfer");
        break;
    case OTItem::atAcceptTransaction:
        strType.Set("atAcceptTransaction");
        break;
    case OTItem::atAcceptMessage:
        strType.Set("atAcceptMessage");
        break;
    case OTItem::atAcceptNotice:
        strType.Set("atAcceptNotice");
        break;
    case OTItem::atAcceptPending:
        strType.Set("atAcceptPending");
        break;
    case OTItem::atRejectPending:
        strType.Set("atRejectPending");
        break;
    case OTItem::atAcceptCronReceipt:
        strType.Set("atAcceptCronReceipt");
        break;
    case OTItem::atDisputeCronReceipt:
        strType.Set("atDisputeCronReceipt");
        break;
    case OTItem::atAcceptItemReceipt:
        strType.Set("atAcceptItemReceipt");
        break;
    case OTItem::atDisputeItemReceipt:
        strType.Set("atDisputeItemReceipt");
        break;

    case OTItem::atAcceptFinalReceipt:
        strType.Set("atAcceptFinalReceipt");
        break;
    case OTItem::atAcceptBasketReceipt:
        strType.Set("atAcceptBasketReceipt");
        break;
    case OTItem::atDisputeFinalReceipt:
        strType.Set("atDisputeFinalReceipt");
        break;
    case OTItem::atDisputeBasketReceipt:
        strType.Set("atDisputeBasketReceipt");
        break;

    case OTItem::atServerfee:
        strType.Set("atServerfee");
        break;
    case OTItem::atIssuerfee:
        strType.Set("atIssuerfee");
        break;
    case OTItem::atWithdrawal:
        strType.Set("atWithdrawal");
        break;
    case OTItem::atDeposit:
        strType.Set("atDeposit");
        break;
    case OTItem::atWithdrawVoucher:
        strType.Set("atWithdrawVoucher");
        break;
    case OTItem::atDepositCheque:
        strType.Set("atDepositCheque");
        break;
    case OTItem::atPayDividend:
        strType.Set("atPayDividend");
        break;
    case OTItem::atMarketOffer:
        strType.Set("atMarketOffer");
        break;
    case OTItem::atPaymentPlan:
        strType.Set("atPaymentPlan");
        break;
    case OTItem::atSmartContract:
        strType.Set("atSmartContract");
        break;
    case OTItem::atBalanceStatement:
        strType.Set("atBalanceStatement");
        break;
    case OTItem::atTransactionStatement:
        strType.Set("atTransactionStatement");
        break;

    default:
        strType.Set("error-unknown");
        break;
    }
}

void OTItem::UpdateContents() // Before transmission or serialization, this is
                              // where the ledger saves its contents
{
    String strListOfBlanks; // IF this item is "acceptTransaction" then this
                            // will serialize the list of transaction numbers
                            // being accepted. (They now support multiple
                            // numbers.)

    switch (m_Type) {
    case OTItem::acceptTransaction: {
        if (m_Numlist.Count() >
            0) // This is always 0, except for OTItem::acceptTransaction.
        {
            String strNumbers;
            if (true == m_Numlist.Output(strNumbers))
                strListOfBlanks.Format(" totalListOfNumbers=\"%s\"\n",
                                       strNumbers.Get());
            else // (False just means m_Numlist was empty.)
                strListOfBlanks.Set("");
        }
    }
    default:
        break;
    }

    String strFromAcctID(GetPurportedAccountID()),
        strToAcctID(GetDestinationAcctID()),
        strServerID(GetPurportedServerID()), strType, strStatus,
        strUserID(GetUserID());

    GetStringFromType(m_Type, strType);

    switch (m_Status) {
    case OTItem::request:
        strStatus.Set("request");
        break;
    case OTItem::acknowledgement:
        strStatus.Set("acknowledgement");
        break;
    case OTItem::rejection:
        strStatus.Set("rejection");
        break;
    default:
        strStatus.Set("error-unknown");
        break;
    }

    // I release this because I'm about to repopulate it.
    m_xmlUnsigned.Release();

    if (m_lNewOutboxTransNum > 0)
        m_xmlUnsigned.Concatenate(
            "<item type=\"%s\"\n status=\"%s\"\n"
            " outboxNewTransNum=\"%" PRId64
            "\"\n" // only used in server reply item:
                   // atBalanceStatement. In cases
                   // where the statement includes a
                   // new outbox item, this variable is
                   // used to transport the new
                   // transaction number (generated on
                   // server side for that new outbox
                   // item) back to the client, so the
                   // client knows the transaction
                   // number to verify when he is
                   // verifying the outbox against the
                   // last signed receipt.
            " numberOfOrigin=\"%" PRId64 "\"\n"
            " transactionNum=\"%" PRId64 "\"\n"
            " serverID=\"%s\"\n"
            " userID=\"%s\"\n"
            " fromAccountID=\"%s\"\n"
            " toAccountID=\"%s\"\n"
            " inReferenceTo=\"%" PRId64 "\"\n"
            " amount=\"%" PRId64 "\" >\n\n",
            strType.Get(), strStatus.Get(), m_lNewOutboxTransNum,
            GetRawNumberOfOrigin(), // GetRaw so it doesn't calculate.
            GetTransactionNum(), strServerID.Get(), strUserID.Get(),
            strFromAcctID.Get(), strToAcctID.Get(), GetReferenceToNum(),
            m_lAmount);
    else
        m_xmlUnsigned.Concatenate(
            "<item type=\"%s\"\n status=\"%s\"\n"
            " numberOfOrigin=\"%" PRId64 "\"\n"
            " transactionNum=\"%" PRId64 "\"\n%s"
            " serverID=\"%s\"\n"
            " userID=\"%s\"\n"
            " fromAccountID=\"%s\"\n"
            " toAccountID=\"%s\"\n"
            " inReferenceTo=\"%" PRId64 "\"\n"
            " amount=\"%" PRId64 "\" >\n\n",
            strType.Get(), strStatus.Get(),
            GetRawNumberOfOrigin(), // GetRaw so it doesn't calculate.
            GetTransactionNum(), strListOfBlanks.Get(), strServerID.Get(),
            strUserID.Get(), strFromAcctID.Get(), strToAcctID.Get(),
            GetReferenceToNum(), m_lAmount);

    if (m_ascNote.GetLength() > 2) {
        m_xmlUnsigned.Concatenate("<note>\n%s</note>\n\n", m_ascNote.Get());
    }

    if (m_ascInReferenceTo.GetLength() > 2) {
        m_xmlUnsigned.Concatenate("<inReferenceTo>\n%s</inReferenceTo>\n\n",
                                  m_ascInReferenceTo.Get());
    }

    if (m_ascAttachment.GetLength() > 2) {
        m_xmlUnsigned.Concatenate("<attachment>\n%s</attachment>\n\n",
                                  m_ascAttachment.Get());
    }

    if ((OTItem::balanceStatement == m_Type) ||
        (OTItem::atBalanceStatement == m_Type)) {

        // loop through the sub-items (only used for balance agreement.)
        //
        for (auto& it : m_listItems) {
            OTItem* pItem = it;
            OT_ASSERT(nullptr != pItem);

            String acctID(pItem->GetPurportedAccountID()),
                serverID(pItem->GetPurportedServerID()),
                userID(pItem->GetUserID());

            String receiptType;
            GetStringFromType(pItem->GetType(), receiptType);

            m_xmlUnsigned.Concatenate(
                "<transactionReport type=\"%s\"\n"
                " adjustment=\"%" PRId64 "\"\n"
                " accountID=\"%s\"\n"
                " userID=\"%s\"\n"
                " serverID=\"%s\"\n"
                " numberOfOrigin=\"%" PRId64 "\"\n"
                " transactionNum=\"%" PRId64 "\"\n"
                " closingTransactionNum=\"%" PRId64 "\"\n"
                " inReferenceTo=\"%" PRId64 "\" />\n\n",
                receiptType.Exists() ? receiptType.Get() : "error_state",
                pItem->GetAmount(), acctID.Get(), userID.Get(), serverID.Get(),
                pItem->GetRawNumberOfOrigin(), pItem->GetTransactionNum(),
                pItem->GetClosingNum(), pItem->GetReferenceToNum());
        }
    }

    m_xmlUnsigned.Concatenate("</item>\n");
}

} // namespace opentxs
