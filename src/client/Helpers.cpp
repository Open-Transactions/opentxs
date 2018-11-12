// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/client/Helpers.hpp"

#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/crypto/OTEnvelope.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/ext/OTPayment.hpp"

#include <cstdint>
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
std::shared_ptr<OTPayment> GetInstrumentByReceiptID(
    const Nym& theNym,
    const std::int64_t& lReceiptId,
    Ledger& ledger)
{
    OT_VERIFY_MIN_BOUND(lReceiptId, 1);

    auto pTransaction = ledger.GetTransaction(lReceiptId);
    if (false == bool(pTransaction)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Supposedly good receipt ID, but uncovered nullptr "
            "transaction: ")(lReceiptId)(".")
            .Flush();
        return nullptr;  // Weird.
    }
    return GetInstrument(theNym, ledger, pTransaction);
}
// ------------------------------------------------------------
std::shared_ptr<OTPayment> GetInstrumentByIndex(
    const Nym& theNym,
    const std::int32_t& nIndex,
    Ledger& ledger)
{
    OT_VERIFY_BOUNDS(nIndex, 0, ledger.GetTransactionCount());

    auto pTransaction = ledger.GetTransactionByIndex(nIndex);
    if (false == bool(pTransaction)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Supposedly good index, but uncovered nullptr transaction: ")(
            nIndex)(".")
            .Flush();
        return nullptr;  // Weird.
    }
    return GetInstrument(theNym, ledger, pTransaction);
}
// ------------------------------------------------------------
// For paymentsInbox and possibly the Nym's recordbox / expired box.
// (Starting to write it now...)
// Returns financial instrument contained in receipt.
//
std::shared_ptr<OTPayment> GetInstrument(
    const Nym& theNym,
    Ledger& ledger,
    std::shared_ptr<OTTransaction> pTransaction)
{
    OT_ASSERT(false != bool(pTransaction));

    const std::int64_t lTransactionNum = pTransaction->GetTransactionNum();

    // Update: for transactions in ABBREVIATED form, the string is empty,
    // since it has never actually been signed (in fact the whole postd::int32_t
    // with abbreviated transactions in a ledger is that they take up very
    // little room, and have no signature of their own, but exist merely as
    // XML tags on their parent ledger.)
    //
    // THEREFORE I must check to see if this transaction is abbreviated and
    // if so, sign it in order to force the UpdateContents() call, so the
    // programmatic user of this API will be able to load it up.
    //
    if (pTransaction->IsAbbreviated()) {
        ledger.LoadBoxReceipt(static_cast<std::int64_t>(
            lTransactionNum));  // I don't check return val here because I still
                                // want it to send the abbreviated form, if this
                                // fails.
        pTransaction =
            ledger.GetTransaction(static_cast<std::int64_t>(lTransactionNum));

        if (false == bool(pTransaction)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Good index but uncovered nullptr "
                "after trying to load full version of abbreviated receipt "
                "with transaction number: ")(lTransactionNum)(".")
                .Flush();
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

    if ((transactionType::instrumentNotice != pTransaction->GetType()) &&
        (transactionType::payDividend != pTransaction->GetType()) &&
        (transactionType::notice != pTransaction->GetType())) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Failure: Expected OTTransaction::instrumentNotice, "
            "::payDividend or ::notice, "
            "but found: OTTransaction::")(pTransaction->GetTypeString())(".")
            .Flush();

        return nullptr;
    }
    // ------------------------------------------------------------
    // By this point, we know the transaction is loaded up, it's
    // not abbreviated, and is one of the accepted receipt types
    // that would contain the sort of instrument we're looking for.
    //
    auto pPayment =
        extract_payment_instrument_from_notice(theNym, pTransaction);

    return pPayment;
}

// Low-level.
std::shared_ptr<OTPayment> extract_payment_instrument_from_notice(
    const Nym& theNym,
    std::shared_ptr<OTTransaction> pTransaction)
{
    const bool bValidNotice =
        (transactionType::instrumentNotice == pTransaction->GetType()) ||
        (transactionType::payDividend == pTransaction->GetType()) ||
        (transactionType::notice == pTransaction->GetType());
    OT_NEW_ASSERT_MSG(
        bValidNotice, "Invalid receipt type passed to this function.");
    // ----------------------------------------------------------------
    if ((transactionType::instrumentNotice ==
         pTransaction->GetType()) ||  // It's encrypted.
        (transactionType::payDividend == pTransaction->GetType())) {
        auto strMsg = String::Factory();
        pTransaction->GetReferenceString(strMsg);

        if (!strMsg->Exists()) {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Failure: Expected OTTransaction::instrumentNotice to "
                "contain an 'in reference to' string, but it was empty. "
                "(Returning).")
                .Flush();
            return nullptr;
        }
        // --------------------
        auto pMsg{pTransaction->API().Factory().Message()};
        if (false == bool(pMsg)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Null: Assert while allocating memory "
                "for an OTMessage!")
                .Flush();
            OT_FAIL;
        }
        if (!pMsg->LoadContractFromString(strMsg)) {
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Failed trying to load OTMessage from string: ")(strMsg)(".")
                .Flush();
            return nullptr;
        }
        // --------------------
        // By this point, the original OTMessage has been loaded from string
        // successfully.
        // Now we need to decrypt the payment on that message (which contains
        // the instrument
        // itself that we need to return.) We decrypt it the same way as we do
        // in SwigWrap::GetNym_MailContentsByIndex():
        //

        // SENDER:     pMsg->m_strNymID
        // RECIPIENT:  pMsg->m_strNymID2
        // INSTRUMENT: pMsg->m_ascPayload (in an OTEnvelope)
        //
        OTEnvelope theEnvelope;
        auto strEnvelopeContents = String::Factory();

        // Decrypt the Envelope.
        if (!theEnvelope.SetCiphertext(pMsg->m_ascPayload))
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Failed trying to set ASCII-armored data for envelope: ")(
                strMsg)(".")
                .Flush();
        else if (!theEnvelope.Open(theNym, strEnvelopeContents))
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Failed trying to decrypt the financial instrument "
                "that was supposedly attached as a payload to this "
                "payment message: ")(strMsg)(".")
                .Flush();
        else if (!strEnvelopeContents->Exists())
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Failed: after decryption, cleartext is empty. From: ")(
                strMsg)(".")
                .Flush();
        else {
            // strEnvelopeContents contains a PURSE or CHEQUE
            // (etc) and not specifically a generic "PAYMENT".
            //
            auto pPayment{
                pTransaction->API().Factory().Payment(strEnvelopeContents)};
            if (false == bool(pPayment) || !pPayment->IsValid())
                LogNormal(OT_METHOD)(__FUNCTION__)(
                    ": Failed: after decryption, payment is invalid. "
                    "Contents: ")(strEnvelopeContents)(".")
                    .Flush();
            else  // success.
            {
                std::shared_ptr<OTPayment> payment{pPayment.release()};
                return payment;
            }
        }
    } else if (transactionType::notice == pTransaction->GetType()) {
        auto strNotice = String::Factory(*pTransaction);
        auto pPayment{pTransaction->API().Factory().Payment(strNotice)};

        if (false == bool(pPayment) || !pPayment->IsValid())
            LogNormal(OT_METHOD)(__FUNCTION__)(
                ": Failed: the notice is invalid. Contents: ")(strNotice)(".")
                .Flush();
        else  // success.
        {
            std::shared_ptr<OTPayment> payment{pPayment.release()};
            return payment;
        }
    }

    return nullptr;
}
}  // namespace opentxs
