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

#include "opentxs/core/stdafx.hpp"

#include "opentxs/client/Helpers.hpp"

#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/crypto/OTEnvelope.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/ext/OTPayment.hpp"

#include <stdint.h>
#include <memory>
#include <ostream>

#define OT_METHOD "Helpers.cpp::"

namespace opentxs
{

// ------------------------------------------------------------

// returns financial instrument (Cheque, Purse, etc.) by
// receipt ID in ledger. So if ledger contains 5 receipts,
// the transaction ID of one of those receipts might contain
// a purse or something, that the caller wants to retrieve.
//
// Caller is responsible to delete.
//
OTPayment* GetInstrumentByReceiptID(
    const Nym& theNym,
    const int64_t& lReceiptId,
    Ledger& ledger)
{
    OT_VERIFY_MIN_BOUND(lReceiptId, 1);

    OTTransaction* pTransaction = ledger.GetTransaction(lReceiptId);
    if (nullptr == pTransaction) {
        otErr << OT_METHOD << __FUNCTION__
              << ": supposedly good receipt ID, but uncovered nullptr "
                 "transaction: "
              << lReceiptId << "\n";
        return nullptr;  // Weird.
    }
    return GetInstrument(theNym, ledger, pTransaction);
}
// ------------------------------------------------------------
OTPayment* GetInstrumentByIndex(
    const Nym& theNym,
    const int32_t& nIndex,
    Ledger& ledger)
{
    OT_VERIFY_BOUNDS(nIndex, 0, ledger.GetTransactionCount());

    OTTransaction* pTransaction = ledger.GetTransactionByIndex(nIndex);
    if (nullptr == pTransaction) {
        otErr << OT_METHOD << __FUNCTION__
              << ": supposedly good index, but uncovered nullptr transaction: "
              << nIndex << "\n";
        return nullptr;  // Weird.
    }
    return GetInstrument(theNym, ledger, pTransaction);
}
// ------------------------------------------------------------
// For paymentsInbox and possibly the Nym's recordbox / expired box.
// (Starting to write it now...)
// Returns financial instrument contained in receipt.
// Caller responsible to delete.
//
OTPayment* GetInstrument(
    const Nym& theNym,
    Ledger& ledger,
    OTTransaction*& pTransaction)
{
    OT_ASSERT(nullptr != pTransaction);

    const int64_t lTransactionNum = pTransaction->GetTransactionNum();

    // Update: for transactions in ABBREVIATED form, the string is empty,
    // since it has never actually been signed (in fact the whole point32_t
    // with abbreviated transactions in a ledger is that they take up very
    // little room, and have no signature of their own, but exist merely as
    // XML tags on their parent ledger.)
    //
    // THEREFORE I must check to see if this transaction is abbreviated and
    // if so, sign it in order to force the UpdateContents() call, so the
    // programmatic user of this API will be able to load it up.
    //
    if (pTransaction->IsAbbreviated()) {
        ledger.LoadBoxReceipt(static_cast<int64_t>(
            lTransactionNum));  // I don't check return val here because I still
                                // want it to send the abbreviated form, if this
                                // fails.
        pTransaction =
            ledger.GetTransaction(static_cast<int64_t>(lTransactionNum));

        if (nullptr == pTransaction) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": good index but uncovered nullptr "
                     "after trying to load full version of abbreviated receipt "
                     "with transaction number: "
                  << lTransactionNum << "\n";
            return nullptr;  // Weird. Clearly I need the full box receipt, if
                             // I'm to get the instrument out of it.
        }
    }
    // ------------------------------------------------------------
    /*
    TO EXTRACT INSTRUMENT FROM PAYMENTS INBOX:
    -- Iterate through the transactions in the payments inbox.
    -- (They should all be "instrumentNotice" transactions.)
    -- Each transaction contains an
       OTMessage in the "in ref to" field, which in turn contains
           an encrypted OTPayment in the payload field, which contains
           the actual financial instrument.
    -- Therefore, this function, based purely on ledger index (as we iterate):
     1. extracts the OTMessage from the Transaction at each index,
        from its "in ref to" field.
     2. then decrypts the payload on that message, producing an OTPayment
    object, 3. ...which contains the actual instrument.
    */

    if ((OTTransaction::instrumentNotice != pTransaction->GetType()) &&
        (OTTransaction::payDividend != pTransaction->GetType()) &&
        (OTTransaction::notice != pTransaction->GetType())) {
        otOut << OT_METHOD << __FUNCTION__
              << ": Failure: Expected OTTransaction::instrumentNotice, "
                 "::payDividend or ::notice, "
                 "but found: OTTransaction::"
              << pTransaction->GetTypeString() << "\n";

        return nullptr;
    }
    // ------------------------------------------------------------
    // By this point, we know the transaction is loaded up, it's
    // not abbreviated, and is one of the accepted receipt types
    // that would contain the sort of instrument we're looking for.
    //
    OTPayment* pPayment =
        extract_payment_instrument_from_notice(theNym, pTransaction);

    return pPayment;
}

// Low-level.
OTPayment* extract_payment_instrument_from_notice(
    const Nym& theNym,
    OTTransaction*& pTransaction)
{
    const bool bValidNotice =
        (OTTransaction::instrumentNotice == pTransaction->GetType()) ||
        (OTTransaction::payDividend == pTransaction->GetType()) ||
        (OTTransaction::notice == pTransaction->GetType());
    OT_NEW_ASSERT_MSG(
        bValidNotice, "Invalid receipt type passed to this function.");
    // ----------------------------------------------------------------
    if ((OTTransaction::instrumentNotice ==
         pTransaction->GetType()) ||  // It's encrypted.
        (OTTransaction::payDividend == pTransaction->GetType())) {
        String strMsg;
        pTransaction->GetReferenceString(strMsg);

        if (!strMsg.Exists()) {
            otOut << OT_METHOD << __FUNCTION__
                  << ": Failure: Expected OTTransaction::instrumentNotice to "
                     "contain an 'in reference to' string, but it was empty. "
                     "(Returning \"\".)\n";
            return nullptr;
        }
        // --------------------
        std::unique_ptr<Message> pMsg(new Message);
        if (!pMsg) {
            otErr << OT_METHOD << __FUNCTION__
                  << ": Null:  Assert while allocating memory "
                     "for an OTMessage!\n";
            OT_FAIL;
        }
        if (!pMsg->LoadContractFromString(strMsg)) {
            otOut << OT_METHOD << __FUNCTION__
                  << ": Failed trying to load OTMessage from string:\n\n"
                  << strMsg << "\n\n";
            return nullptr;
        }
        // --------------------
        // By this point, the original OTMessage has been loaded from string
        // successfully.
        // Now we need to decrypt the payment on that message (which contains
        // the instrument
        // itself that we need to return.) We decrypt it the same way as we do
        // in OTAPI_Wrap::GetNym_MailContentsByIndex():
        //

        // SENDER:     pMsg->m_strNymID
        // RECIPIENT:  pMsg->m_strNymID2
        // INSTRUMENT: pMsg->m_ascPayload (in an OTEnvelope)
        //
        OTEnvelope theEnvelope;
        String strEnvelopeContents;

        // Decrypt the Envelope.
        if (!theEnvelope.SetCiphertext(pMsg->m_ascPayload))
            otOut << OT_METHOD << __FUNCTION__
                  << ": Failed trying to set ASCII-armored data for envelope:\n"
                  << strMsg << "\n\n";
        else if (!theEnvelope.Open(theNym, strEnvelopeContents))
            otOut << OT_METHOD << __FUNCTION__
                  << ": Failed trying to decrypt the financial instrument "
                     "that was supposedly attached as a payload to this "
                     "payment message:\n"
                  << strMsg << "\n\n";
        else if (!strEnvelopeContents.Exists())
            otOut << OT_METHOD << __FUNCTION__
                  << ": Failed: after decryption, cleartext is empty. From:\n"
                  << strMsg << "\n\n";
        else {
            // strEnvelopeContents contains a PURSE or CHEQUE
            // (etc) and not specifically a generic "PAYMENT".
            //
            std::unique_ptr<OTPayment> pPayment(
                new OTPayment(strEnvelopeContents));
            if (!pPayment || !pPayment->IsValid())
                otOut << OT_METHOD << __FUNCTION__
                      << ": Failed: after decryption, payment is invalid. "
                         "Contents:\n\n"
                      << strEnvelopeContents << "\n\n";
            else  // success.
            {
                return pPayment.release();  // Caller responsible to delete.
            }
        }
    } else if (OTTransaction::notice == pTransaction->GetType()) {
        String strNotice(*pTransaction);
        std::unique_ptr<OTPayment> pPayment(new OTPayment(strNotice));

        if (!pPayment || !pPayment->IsValid())
            otOut << OT_METHOD << __FUNCTION__
                  << ": Failed: the notice is invalid. Contents:\n\n"
                  << strNotice << "\n\n";
        else  // success.
        {
            return pPayment.release();  // Caller responsible to delete.
        }
    }

    return nullptr;
}

int32_t GetOutpaymentsIndexByTransNum(const Nym& nym, int64_t lTransNum)
{
    const int32_t lOutpaymentsCount = nym.GetOutpaymentsCount();

    for (int32_t lOutpaymentsIndex = 0; lOutpaymentsIndex < lOutpaymentsCount;
         ++lOutpaymentsIndex) {
        Message* pOutpaymentMsg = nym.GetOutpaymentsByIndex(lOutpaymentsIndex);
        if (nullptr != pOutpaymentMsg) {
            String strPayment;

            // There isn't any encrypted envelope this time, since it's my
            // outPayments box.
            //
            if (pOutpaymentMsg->m_ascPayload.Exists() &&
                pOutpaymentMsg->m_ascPayload.GetString(strPayment) &&
                strPayment.Exists()) {
                OTPayment thePayment(strPayment);

                // Let's see if it's the cheque we're looking for...
                //
                if (thePayment.IsValid()) {
                    if (thePayment.SetTempValues()) {
                        if (thePayment.HasTransactionNum(lTransNum)) {
                            return lOutpaymentsIndex;
                        }
                    }
                }
            }
        }
    }

    return -1;
}

}  // namespace opentxs
