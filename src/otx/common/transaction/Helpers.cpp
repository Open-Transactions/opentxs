// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "internal/otx/common/transaction/Helpers.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <filesystem>

#include "internal/api/Legacy.hpp"
#include "internal/api/session/FactoryAPI.hpp"
#include "internal/api/session/Session.hpp"
#include "internal/core/Factory.hpp"
#include "internal/core/String.hpp"
#include "internal/otx/Types.hpp"
#include "internal/otx/common/Ledger.hpp"
#include "internal/otx/common/NumList.hpp"
#include "internal/otx/common/OTTransaction.hpp"
#include "internal/otx/common/OTTransactionType.hpp"
#include "internal/otx/common/util/Common.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "otx/common/OTStorage.hpp"

namespace
{
// NOTE: The below strings correspond to the transaction types
// listed near the top of OTTransaction.hpp as enum transactionType.
// NOLINTNEXTLINE(modernize-avoid-c-arrays)
char const* const TypeStrings[] = {
    "blank",    // freshly issued, not used yet  // comes from server, stored on
                // Nym. (Nymbox.)
    "message",  // in nymbox, message from one user to another.
    "notice",   // in nymbox, notice from the server. Probably contains an
                // updated
                // smart contract.
    "replyNotice",  // When you send a request to the server, sometimes its
                    // reply
                    // is so important,
    // that it drops a copy into your Nymbox to make you receive and process it.
    "successNotice",    // A transaction # has successfully been signed out.
                        // (Nymbox.)
    "pending",          // Pending transfer, in the inbox/outbox.
    "transferReceipt",  // the server drops this into your inbox, when someone
                        // accepts your transfer.
    "chequeReceipt",    // the server drops this into your inbox, when someone
                        // deposits your cheque.
    "voucherReceipt",   // the server drops this into your inbox, when someone
                        // deposits your voucher.
    "marketReceipt",   // server drops this into inbox periodically, if you have
                       // an offer on the market.
    "paymentReceipt",  // the server drops this into people's inboxes,
                       // periodically, if they have payment plans.
    "finalReceipt",    // the server drops this into your inbox(es), when a
                       // CronItem expires or is canceled.
    "basketReceipt",   // the server drops this into your inboxes, when a basket
                       // exchange is processed.
    "instrumentNotice",  // Receive these in paymentInbox (by way of Nymbox),
                         // and
                         // send in Outpayments (like outMail.) (When done, they
                         // go to recordBox or expiredBox to await deletion.)
    "instrumentRejection",  // When someone rejects your invoice from his
                            // paymentInbox, you get one of these in YOUR
                            // paymentInbox.
    "processNymbox",    // process nymbox transaction     // comes from client
    "atProcessNymbox",  // process nymbox reply             // comes from server
    "processInbox",     // process inbox transaction     // comes from client
    "atProcessInbox",   // process inbox reply             // comes from server
    "transfer",  // or "spend". This transaction is a transfer from one account
                 // to another
    "atTransfer",  // reply from the server regarding a transfer request
    "deposit",  // this transaction is a deposit of bearer tokens (from client)
    "atDeposit",         // reply from the server regarding a deposit request
    "withdrawal",        // this transaction is a withdrawal of bearer tokens
    "atWithdrawal",      // reply from the server regarding a withdrawal request
    "marketOffer",       // this transaction is a market offer
    "atMarketOffer",     // reply from the server regarding a market offer
    "paymentPlan",       // this transaction is a payment plan
    "atPaymentPlan",     // reply from the server regarding a payment plan
    "smartContract",     // this transaction is a smart contract
    "atSmartContract",   // reply from the server regarding a smart contract
    "cancelCronItem",    // this transaction is a cancellation of a cron item
                         // (payment plan etc)
    "atCancelCronItem",  // reply from the server regarding said cancellation.
    "exchangeBasket",    // this transaction is an exchange in/out of a basket
                         // currency.
    "atExchangeBasket",  // reply from the server regarding said exchange.
    "payDividend",       // this transaction is a dividend payment (to the
                         // shareholders.)
    "atPayDividend",  // reply from the server regarding said dividend payment.
    "incomingCash",
    "error_state"};

// NOLINTNEXTLINE(modernize-avoid-c-arrays)
char const* const OriginTypeStrings[] = {
    "not_applicable",
    "origin_market_offer",    // finalReceipt
    "origin_payment_plan",    // finalReceipt, paymentReceipt
    "origin_smart_contract",  // finalReceipt, paymentReceipt
    "origin_pay_dividend",    // SOME voucher receipts are from a payDividend.
    "origin_error_state"};

}  // namespace

namespace opentxs
{

auto GetTransactionTypeString(int transactionTypeIndex) -> const
    char*  // enum transactionType
{
    return TypeStrings[transactionTypeIndex];
}

auto GetOriginTypeToString(int originTypeIndex) -> const
    char*  // enum originType
{
    return OriginTypeStrings[originTypeIndex];
}

// Returns 1 if success, -1 if error.
auto LoadAbbreviatedRecord(
    irr::io::IrrXMLReader*& xml,
    std::int64_t& lNumberOfOrigin,
    originType& theOriginType,
    std::int64_t& lTransactionNum,
    std::int64_t& lInRefTo,
    std::int64_t& lInRefDisplay,
    Time& the_DATE_SIGNED,
    transactionType& theType,
    String& strHash,
    Amount& lAdjustment,
    Amount& lDisplayValue,
    std::int64_t& lClosingNum,
    std::int64_t& lRequestNum,
    bool& bReplyTransSuccess,
    NumList* pNumList) -> std::int32_t
{

    const auto strOriginNum =
        String::Factory(xml->getAttributeValue("numberOfOrigin"));
    const auto strOriginType =
        String::Factory(xml->getAttributeValue("originType"));
    const auto strTransNum =
        String::Factory(xml->getAttributeValue("transactionNum"));
    const auto strInRefTo =
        String::Factory(xml->getAttributeValue("inReferenceTo"));
    const auto strInRefDisplay =
        String::Factory(xml->getAttributeValue("inRefDisplay"));
    const auto strDateSigned =
        String::Factory(xml->getAttributeValue("dateSigned"));

    if (!strTransNum->Exists() || !strInRefTo->Exists() ||
        !strInRefDisplay->Exists() || !strDateSigned->Exists()) {
        LogConsole()(__func__)("Failure: missing "
                               "strTransNum (")(strTransNum.get())(
            ") or strInRefTo (")(strInRefTo.get())(") or strInRefDisplay (")(
            strInRefDisplay.get())(") or strDateSigned(")(strDateSigned.get())(
            ") while loading abbreviated receipt.")
            .Flush();
        return (-1);
    }
    lTransactionNum = strTransNum->ToLong();
    lInRefTo = strInRefTo->ToLong();
    lInRefDisplay = strInRefDisplay->ToLong();

    if (strOriginNum->Exists()) { lNumberOfOrigin = strOriginNum->ToLong(); }
    if (strOriginType->Exists()) {
        theOriginType =
            OTTransactionType::GetOriginTypeFromString(strOriginType);
    }

    the_DATE_SIGNED = parseTimestamp(strDateSigned->Get());

    // Transaction TYPE for the abbreviated record...
    theType = transactionType::error_state;  // default
    const auto strAbbrevType = String::Factory(
        xml->getAttributeValue("type"));  // the type of inbox receipt, or
                                          // outbox receipt, or nymbox receipt.
                                          // (Transaction type.)
    if (strAbbrevType->Exists()) {
        theType = OTTransaction::GetTypeFromString(strAbbrevType);

        if (transactionType::error_state == theType) {
            LogError()(__func__)(
                "Failure: Error_state was the found type (based on "
                "string ")(strAbbrevType.get())(
                "), when loading abbreviated receipt "
                "for trans num: ")(lTransactionNum)(" (In Reference To: ")(
                lInRefTo)(").")
                .Flush();
            return (-1);
        }
    } else {
        LogConsole()(__func__)("Failure: unknown "
                               "transaction type (")(strAbbrevType.get())(
            ") when "
            "loading abbreviated receipt for trans num: ")(
            lTransactionNum)(" (In Reference To: ")(lInRefTo)(").")
            .Flush();
        return (-1);
    }

    // RECEIPT HASH
    //
    strHash.Set(xml->getAttributeValue("receiptHash"));
    if (!strHash.Exists()) {
        LogConsole()(__func__)("Failure: Expected "
                               "receiptHash while loading "
                               "abbreviated receipt for trans num: ")(
            lTransactionNum)(" (In Reference To: ")(lInRefTo)(").")
            .Flush();
        return (-1);
    }

    lAdjustment = 0;
    lDisplayValue = 0;
    lClosingNum = 0;

    const auto strAbbrevAdjustment =
        String::Factory(xml->getAttributeValue("adjustment"));
    if (strAbbrevAdjustment->Exists()) {
        lAdjustment = factory::Amount(strAbbrevAdjustment->Get());
    }
    // -------------------------------------
    const auto strAbbrevDisplayValue =
        String::Factory(xml->getAttributeValue("displayValue"));
    if (strAbbrevDisplayValue->Exists()) {
        lDisplayValue = factory::Amount(strAbbrevDisplayValue->Get());
    }

    if (transactionType::replyNotice == theType) {
        const auto strRequestNum =
            String::Factory(xml->getAttributeValue("requestNumber"));

        if (!strRequestNum->Exists()) {
            LogConsole()(__func__)(
                "Failed loading "
                "abbreviated receipt: "
                "expected requestNumber on replyNotice trans num: ")(
                lTransactionNum)(" (In Reference To: ")(lInRefTo)(").")
                .Flush();
            return (-1);
        }
        lRequestNum = strRequestNum->ToLong();

        const auto strTransSuccess =
            String::Factory(xml->getAttributeValue("transSuccess"));

        bReplyTransSuccess = strTransSuccess->Compare("true");
    }  // if replyNotice (expecting request Number)

    // If the transaction is a certain type, then it will also have a CLOSING
    // number.
    // (Grab that too.)
    //
    if ((transactionType::finalReceipt == theType) ||
        (transactionType::basketReceipt == theType)) {
        const auto strAbbrevClosingNum =
            String::Factory(xml->getAttributeValue("closingNum"));

        if (!strAbbrevClosingNum->Exists()) {
            LogConsole()(__func__)("Failed loading "
                                   "abbreviated receipt: "
                                   "expected closingNum on trans num: ")(
                lTransactionNum)(" (In Reference To: ")(lInRefTo)(").")
                .Flush();
            return (-1);
        }
        lClosingNum = strAbbrevClosingNum->ToLong();
    }  // if finalReceipt or basketReceipt (expecting closing num)

    // These types carry their own internal list of numbers.
    //
    if ((nullptr != pNumList) &&
        ((transactionType::blank == theType) ||
         (transactionType::successNotice == theType))) {
        const auto strNumbers =
            String::Factory(xml->getAttributeValue("totalListOfNumbers"));
        pNumList->Release();

        if (strNumbers->Exists()) { pNumList->Add(strNumbers); }
    }  // if blank or successNotice (expecting totalListOfNumbers.. no more
       // multiple blanks in the same ledger! They all go in a single
       // transaction.)

    return 1;
}

auto VerifyBoxReceiptExists(
    const api::Session& api,
    const UnallocatedCString& dataFolder,
    const identifier::Notary& NOTARY_ID,
    const identifier::Nym& NYM_ID,          // Unused here for now, but still
                                            // convention.
    const identifier::Generic& ACCOUNT_ID,  // If for Nymbox (vs inbox/outbox)
                                            // then pass NYM_ID in this field
                                            // also.
    const std::int32_t nBoxType,            // 0/nymbox, 1/inbox, 2/outbox
    const std::int64_t& lTransactionNum) -> bool
{
    const auto lLedgerType = static_cast<std::int64_t>(nBoxType);

    const auto strNotaryID = String::Factory(NOTARY_ID),
               strUserOrAcctID = String::Factory(
                   0 == lLedgerType ? NYM_ID : ACCOUNT_ID);  // (For Nymbox
                                                             // aka type 0,
                                                             // the NymID
                                                             // will be
                                                             // here.)
    // --------------------------------------------------------------------
    auto strFolder1name = String::Factory(), strFolder2name = String::Factory(),
         strFolder3name = String::Factory(), strFilename = String::Factory();

    if (!SetupBoxReceiptFilename(
            api,
            lLedgerType,  // nBoxType is lLedgerType
            strUserOrAcctID,
            strNotaryID,
            lTransactionNum,
            "OTTransaction::VerifyBoxReceiptExists",
            strFolder1name,
            strFolder2name,
            strFolder3name,
            strFilename)) {
        return false;  // This already logs -- no need to log twice, here.
    }
    // --------------------------------------------------------------------
    // See if the box receipt exists before trying to save over it...
    //
    const bool bExists = OTDB::Exists(
        api,
        dataFolder,
        strFolder1name->Get(),
        strFolder2name->Get(),
        strFolder3name->Get(),
        strFilename->Get());

    LogDetail()(__func__)(
        bExists ? "(Already have this one)"
                : "(Need to download this one) : ")(strFolder1name.get())('/')(
        strFolder2name.get())('/')(strFolder3name.get())('/')(strFilename.get())
        .Flush();

    return bExists;
}

auto LoadBoxReceipt(
    const api::Session& api,
    OTTransaction& theAbbrev,
    Ledger& theLedger) -> std::unique_ptr<OTTransaction>
{
    const auto lLedgerType = static_cast<std::int64_t>(theLedger.GetType());
    return LoadBoxReceipt(api, theAbbrev, lLedgerType);
}

auto LoadBoxReceipt(
    const api::Session& api,
    OTTransaction& theAbbrev,
    std::int64_t lLedgerType) -> std::unique_ptr<OTTransaction>
{
    // See if the appropriate file exists, and load it up from
    // local storage, into a string.
    // Then, try to load the transaction from that string and see if successful.
    // If it verifies, then return it. Otherwise return nullptr.

    // Can only load abbreviated transactions (so they'll become their full
    // form.)
    //
    if (!theAbbrev.IsAbbreviated()) {
        LogConsole()(__func__)("Unable to load box receipt ")(
            theAbbrev.GetTransactionNum())(
            ": (Because argument 'theAbbrev' wasn't abbreviated).")
            .Flush();
        return nullptr;
    }

    // Next, see if the appropriate file exists, and load it up from
    // local storage, into a string.

    auto strFolder1name = String::Factory(), strFolder2name = String::Factory(),
         strFolder3name = String::Factory(), strFilename = String::Factory();

    if (!SetupBoxReceiptFilename(
            api,
            lLedgerType,
            theAbbrev,
            __func__,  // "OTTransaction::LoadBoxReceipt",
            strFolder1name,
            strFolder2name,
            strFolder3name,
            strFilename)) {
        return nullptr;  // This already logs -- no need to log twice, here.
    }

    // See if the box receipt exists before trying to load it...
    //
    if (!OTDB::Exists(
            api,
            api.DataFolder().string(),
            strFolder1name->Get(),
            strFolder2name->Get(),
            strFolder3name->Get(),
            strFilename->Get())) {
        LogDetail()(__func__)("Box receipt does not exist: ")(
            strFolder1name.get())('/')(strFolder2name.get())('/')(
            strFolder3name.get())('/')(strFilename.get())
            .Flush();
        return nullptr;
    }

    // Try to load the box receipt from local storage.
    //
    UnallocatedCString strFileContents(OTDB::QueryPlainString(
        api,
        api.DataFolder().string(),
        strFolder1name->Get(),  // <=== LOADING FROM DATA STORE.
        strFolder2name->Get(),
        strFolder3name->Get(),
        strFilename->Get()));
    if (strFileContents.length() < 2) {
        LogError()(__func__)("Error reading file: ")(strFolder1name.get())('/')(
            strFolder2name.get())('/')(strFolder3name.get())('/')(
            strFilename.get())
            .Flush();
        return nullptr;
    }

    auto strRawFile = String::Factory(strFileContents.c_str());

    if (!strRawFile->Exists()) {
        LogError()(__func__)(
            "Error reading file (resulting output string is empty): ")(
            strFolder1name.get())('/')(strFolder2name.get())('/')(
            strFolder3name.get())('/')(strFilename.get())
            .Flush();
        return nullptr;
    }

    // Finally, try to load the transaction from that string and see if
    // successful.
    //
    auto pTransType = api.Factory().InternalSession().Transaction(strRawFile);

    if (false == bool(pTransType)) {
        LogError()(__func__)(
            "Error instantiating transaction type based on strRawFile: ")(
            strFolder1name.get())('/')(strFolder2name.get())('/')(
            strFolder3name.get())('/')(strFilename.get())
            .Flush();
        return nullptr;
    }

    std::unique_ptr<OTTransaction> pBoxReceipt{
        dynamic_cast<OTTransaction*>(pTransType.release())};

    if (false == bool(pBoxReceipt)) {
        LogError()(__func__)("Error dynamic_cast from transaction type to "
                             "transaction, based on strRawFile: ")(
            strFolder1name.get())('/')(strFolder2name.get())('/')(
            strFolder3name.get())('/')(strFilename.get())
            .Flush();
        return nullptr;
    }

    // BELOW THIS POINT, pBoxReceipt exists, and is an OTTransaction pointer,
    // and is loaded,
    // and basically is ready to be compared to theAbbrev, which is its
    // abbreviated version.
    // It MUST either be returned or deleted.

    bool bSuccess = theAbbrev.VerifyBoxReceipt(*pBoxReceipt);

    if (!bSuccess) {
        LogError()(__func__)("Failed verifying Box Receipt: ")(
            strFolder1name.get())('/')(strFolder2name.get())('/')(
            strFolder3name.get())('/')(strFilename.get())
            .Flush();

        return nullptr;
    } else {
        LogVerbose()(__func__)("Successfully loaded Box Receipt in: ")(
            strFolder1name.get())('/')(strFolder2name.get())('/')(
            strFolder3name.get())('/')(strFilename.get())
            .Flush();
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
    //  pBoxReceipt->SetAbbrevAdjustment(       theAbbrev.GetAbbrevAdjustment()
    // );
    //  pBoxReceipt->SetAbbrevDisplayAmount(
    // theAbbrev.GetAbbrevDisplayAmount() );

    return pBoxReceipt;
}

auto SetupBoxReceiptFilename(
    const api::Session& api,
    std::int64_t lLedgerType,
    const String& strUserOrAcctID,
    const String& strNotaryID,
    const std::int64_t& lTransactionNum,
    const char* szCaller,
    String& strFolder1name,
    String& strFolder2name,
    String& strFolder3name,
    String& strFilename) -> bool
{
    OT_ASSERT(nullptr != szCaller);

    const char* pszFolder = nullptr;  // "nymbox" (or "inbox" or "outbox")
    switch (lLedgerType) {
        case 0:
            pszFolder = api.Internal().Legacy().Nymbox();
            break;
        case 1:
            pszFolder = api.Internal().Legacy().Inbox();
            break;
        case 2:
            pszFolder = api.Internal().Legacy().Outbox();
            break;
        //      case 3: (message ledger.)
        case 4:
            pszFolder = api.Internal().Legacy().PaymentInbox();
            break;
        case 5:
            pszFolder = api.Internal().Legacy().RecordBox();
            break;
        case 6:
            pszFolder = api.Internal().Legacy().ExpiredBox();
            break;
        default:
            LogError()(__func__)("Error: Unknown box type: ")(
                lLedgerType)(". (This should never happen).")
                .Flush();
            return false;
    }

    strFolder1name.Set(pszFolder);    // "nymbox" (or "inbox" or "outbox")
    strFolder2name.Set(strNotaryID);  // "NOTARY_ID"
    auto folder_3_name = api::Legacy::GetFilenameR(strUserOrAcctID.Get());
    strFolder3name.Set(folder_3_name.c_str());

    // "TRANSACTION_ID.rct"
    auto filename = api::Legacy::GetFilenameRct(lTransactionNum);
    strFilename.Set(filename.c_str());

    // Finished product: "nymbox/NOTARY_ID/NYM_ID.r/TRANSACTION_ID.rct"

    return true;
}

auto SetupBoxReceiptFilename(
    const api::Session& api,
    std::int64_t lLedgerType,
    OTTransaction& theTransaction,
    const char* szCaller,
    String& strFolder1name,
    String& strFolder2name,
    String& strFolder3name,
    String& strFilename) -> bool
{
    auto strUserOrAcctID = String::Factory();
    theTransaction.GetIdentifier(strUserOrAcctID);

    const auto strNotaryID = String::Factory(theTransaction.GetRealNotaryID());

    return SetupBoxReceiptFilename(
        api,
        lLedgerType,
        strUserOrAcctID,
        strNotaryID,
        theTransaction.GetTransactionNum(),
        szCaller,
        strFolder1name,
        strFolder2name,
        strFolder3name,
        strFilename);
}

auto SetupBoxReceiptFilename(
    const api::Session& api,
    Ledger& theLedger,
    OTTransaction& theTransaction,
    const char* szCaller,
    String& strFolder1name,
    String& strFolder2name,
    String& strFolder3name,
    String& strFilename) -> bool
{
    std::int64_t lLedgerType = 0;

    switch (theLedger.GetType()) {
        case ledgerType::nymbox: {
            lLedgerType = 0;
        } break;
        case ledgerType::inbox: {
            lLedgerType = 1;
        } break;
        case ledgerType::outbox: {
            lLedgerType = 2;
        } break;
        case ledgerType::paymentInbox: {
            lLedgerType = 4;
        } break;
        case ledgerType::recordBox: {
            lLedgerType = 5;
        } break;
        case ledgerType::expiredBox: {
            lLedgerType = 6;
        } break;
        case ledgerType::message:
        case ledgerType::error_state:
        default: {
            LogError()(__func__)("Error: unknown box type. "
                                 "(This should never happen).")
                .Flush();
            return false;
        }
    }

    return SetupBoxReceiptFilename(
        api,
        lLedgerType,
        theTransaction,
        szCaller,
        strFolder1name,
        strFolder2name,
        strFolder3name,
        strFilename);
}
}  // namespace opentxs
