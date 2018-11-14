// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/Item.hpp"

#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/api/Wallet.hpp"
#include "opentxs/consensus/ClientContext.hpp"
#include "opentxs/consensus/TransactionStatement.hpp"
#include "opentxs/core/transaction/Helpers.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/NumList.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/StringXML.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/OTTransactionType.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/Types.hpp"

#include <irrxml/irrXML.hpp>

#include <cstdint>
#include <cstring>
#include <memory>
#include <ostream>
#include <string>
#include <utility>

#define OT_METHOD "opentxs::Item::"

namespace opentxs
{
// this one is private (I hope to keep it that way.)
// probvably not actually. If I end up back here, it's because
// sometimes I dont' WANT to assign the stuff, but leave it blank
// because I'm about to load it.
Item::Item(const api::Core& core)
    : OTTransactionType(core)
    , m_ascNote(Armored::Factory())
    , m_ascAttachment(Armored::Factory())
    , m_AcctToID(Identifier::Factory())
    , m_lAmount(0)
    , m_listItems()
    , m_Type(itemType::error_state)
    , m_Status(Item::request)
    , m_lNewOutboxTransNum(0)
    , m_lClosingTransactionNo(0)
{
    InitItem();
}

// From owner we can get acct ID, server ID, and transaction Num
Item::Item(
    const api::Core& core,
    const Identifier& theNymID,
    const OTTransaction& theOwner)
    : OTTransactionType(
          core,
          theNymID,
          theOwner.GetRealAccountID(),
          theOwner.GetRealNotaryID(),
          theOwner.GetTransactionNum(),
          theOwner.GetOriginType())
    , m_ascNote(Armored::Factory())
    , m_ascAttachment(Armored::Factory())
    , m_AcctToID(Identifier::Factory())
    , m_lAmount(0)
    , m_listItems()
    , m_Type(itemType::error_state)
    , m_Status(Item::request)
    , m_lNewOutboxTransNum(0)
    , m_lClosingTransactionNo(0)
{
    InitItem();
}

// From owner we can get acct ID, server ID, and transaction Num
Item::Item(
    const api::Core& core,
    const Identifier& theNymID,
    const Item& theOwner)
    : OTTransactionType(
          core,
          theNymID,
          theOwner.GetRealAccountID(),
          theOwner.GetRealNotaryID(),
          theOwner.GetTransactionNum(),
          theOwner.GetOriginType())
    , m_ascNote(Armored::Factory())
    , m_ascAttachment(Armored::Factory())
    , m_AcctToID(Identifier::Factory())
    , m_lAmount(0)
    , m_listItems()
    , m_Type(itemType::error_state)
    , m_Status(Item::request)
    , m_lNewOutboxTransNum(0)
    , m_lClosingTransactionNo(0)
{
    InitItem();
}

Item::Item(
    const api::Core& core,
    const Identifier& theNymID,
    const OTTransaction& theOwner,
    itemType theType,
    const Identifier& pDestinationAcctID)
    : OTTransactionType(
          core,
          theNymID,
          theOwner.GetRealAccountID(),
          theOwner.GetRealNotaryID(),
          theOwner.GetTransactionNum(),
          theOwner.GetOriginType())
    , m_ascNote(Armored::Factory())
    , m_ascAttachment(Armored::Factory())
    , m_AcctToID(Identifier::Factory())
    , m_lAmount(0)
    , m_listItems()
    , m_Type(itemType::error_state)
    , m_Status(Item::request)
    , m_lNewOutboxTransNum(0)
    , m_lClosingTransactionNo(0)
{
    InitItem();

    m_Type = theType;  // This has to be below the InitItem() call that appears
                       // just above

    // Most transactions items don't HAVE a "to" account, just a primary
    // account.
    // (If you deposit, or withdraw, you don't need a "to" account.)
    // But for the ones that do, you can pass the "to" account's ID in
    // as a pointer, and we'll set that too....
    if (!pDestinationAcctID.empty()) {
        m_AcctToID = Identifier::Factory(pDestinationAcctID);
    }
}

// Server-side.
//
// By the time this is called, I know that the item, AND this balance item
// (this) both have the correct user id, server id, account id, and transaction
// id, and they have been signed properly by the owner.
//
// So what do I need to verify in this function?
//
// -- That the transactions on THE_NYM (server-side), minus the current
// transaction number being processed, are all still there.
// -- If theMessageNym is missing certain numbers that I expected to find on
// him, that means he is trying to trick the server into signing a new agreement
// where he is no longer responsible for those numbers. They must all be there.
// -- If theMessageNym has ADDED certain numbers that I DIDN'T expect to find on
// him, then he's trying to trick me into allowing him to add those numbers to
// his receipt -- OR it could mean that certain numbers were already removed on
// my side (such as the opening # for a cron item like a market  offer that has
// already been closed), but the client-side isn't aware of this yet, and so he
// is trying to sign off on formerly-good numbers that have since expired.  This
// shouldn't happen IF the client has been properly notified about these numbers
// before sending his request.  Such notifications are dropped into the Nymbox
// AND related asset account inboxes.
bool Item::VerifyTransactionStatement(
    const ClientContext& context,
    const OTTransaction& transaction,
    const bool real) const
{
    const std::set<TransactionNumber> empty;

    return VerifyTransactionStatement(context, transaction, empty, real);
}

bool Item::VerifyTransactionStatement(
    const ClientContext& context,
    const OTTransaction& TARGET_TRANSACTION,
    const std::set<TransactionNumber> newNumbers,
    const bool bIsRealTransaction) const
{
    if (GetType() != itemType::transactionStatement) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Wrong item type. Expected Item::transactionStatement.")
            .Flush();
        return false;
    }

    // So if the caller was planning to remove a number, or clear a receipt from
    // the inbox, he'll have to do so first before calling this function, and
    // then ADD IT AGAIN if this function fails.  (Because the new Balance
    // Agreement is always the user signing WHAT THE NEW VERSION WILL BE AFTER
    // THE TRANSACTION IS PROCESSED.)
    const auto NOTARY_ID = String::Factory(GetPurportedNotaryID());
    const TransactionNumber itemNumber = GetTransactionNum();
    std::set<TransactionNumber> excluded;

    // Sometimes my "transaction number" is 0 since we're accepting numbers from
    // the Nymbox (which is done by message, not transaction.) In such cases,
    // there's no point in checking the server-side to "make sure it has number
    // 0!" (because it won't.)
    if (bIsRealTransaction) {
        const bool foundExisting = context.VerifyIssuedNumber(itemNumber);
        const bool foundNew = (1 == newNumbers.count(itemNumber));
        const bool found = (foundExisting || foundNew);

        if (!found) {
            LogNormal(OT_METHOD)(__FUNCTION__)(": Transaction# (")(itemNumber)(
                ") doesn't appear on Nym's issued list.")
                .Flush();

            return false;
        }

        // In the case that this is a real transaction, it must be a
        // cancelCronItem, payment plan or market offer (since the other
        // transaction types require a balance statement, not a transaction
        // statement.) Also this might not be a transaction at all, but in that
        // case we won't enter this block anyway.
        switch (TARGET_TRANSACTION.GetType()) {
            // In the case of cancelCronItem(), we'd expect, if success, the
            // number would be excluded, so we have to remove it now, to
            // simulate success for the verification. Then we add it again
            // afterwards, before returning.
            case transactionType::cancelCronItem: {
                excluded.insert(itemNumber);
            } break;
            // IN the case of the offer/plan, we do NOT want to remove from
            // issued list. That only happens when the plan or offer is excluded
            // from Cron and closed. As the plan or offer continues processing,
            // the user is  responsible for its main transaction number until he
            // signs off on  final closing, after many receipts have potentially
            // been received.
            case transactionType::marketOffer:
            case transactionType::paymentPlan:
            case transactionType::smartContract: {
                break;
            }
            default: {
                LogOutput(OT_METHOD)(__FUNCTION__)(": Unexpected "
                                                   "transaction type.")
                    .Flush();
            } break;
        }
        // Client side will NOT remove from issued list in this case (market
        // offer, payment plan, which are
        // the only transactions that use a transactionStatement, which is
        // otherwise used for Nymbox.)
    }

    auto serialized = String::Factory();
    GetAttachment(serialized);

    if (3 > serialized->GetLength()) { return false; }

    const TransactionStatement statement(serialized);

    return context.Verify(statement, excluded, newNumbers);
}

// Server-side.
//
// By the time this is called, I know that the item, AND this balance item
// (this) both have the correct user id, server id, account id, and transaction
// id, and they have been signed properly by the owner.
//
// So what do I need to verify in this function?
//
// 1) That THE_ACCOUNT.GetBalance() + lActualAdjustment equals the amount in
//    GetAmount().
// 2) That the inbox transactions and outbox transactions match up to the list
//    of sub-items on THIS balance item.
// 3) That the transactions on the Nym, minus the current transaction number
//    being processed, are all still there.
bool Item::VerifyBalanceStatement(
    std::int64_t lActualAdjustment,
    const ClientContext& context,
    const Ledger& THE_INBOX,
    const Ledger& THE_OUTBOX,
    const Account& THE_ACCOUNT,
    const OTTransaction& TARGET_TRANSACTION,
    const std::set<TransactionNumber>& excluded,
    TransactionNumber outboxNum) const  // Only used in the case of transfer,
                                        // where the user doesn't know the
                                        // outbox trans# in advance, so he sends
                                        // a dummy number (currently '1') which
                                        // we verify against the actual outbox
                                        // trans# successfully, only in that
                                        // special case.
{
    std::set<TransactionNumber> removed(excluded);

    if (GetType() != itemType::balanceStatement) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Wrong item type.").Flush();

        return false;
    }

    // We need to verify:
    //
    // 1) That THE_ACCOUNT.GetBalance() + lActualAdjustment equals the amount in
    // GetAmount().

    // GetAmount() contains what the balance WOULD be AFTER successful
    // transaction.
    if ((THE_ACCOUNT.GetBalance() + lActualAdjustment) != GetAmount()) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": This balance statement has a value of ")(GetAmount())(
            ", but expected ")(THE_ACCOUNT.GetBalance() + lActualAdjustment)(
            ". (Acct balance of ")(THE_ACCOUNT.GetBalance())(
            " plus actualAdjustment of ")(lActualAdjustment)(").")
            .Flush();

        return false;
    }

    // 2) That the inbox transactions and outbox transactions match up to the
    // list of sub-items on THIS balance item.

    std::int32_t nInboxItemCount = 0, nOutboxItemCount = 0;
    const char* szInbox = "Inbox";
    const char* szOutbox = "Outbox";
    const char* pszLedgerType = nullptr;

    for (std::int32_t i = 0; i < GetItemCount(); i++) {
        const auto pSubItem = GetItem(i);

        OT_ASSERT(false != bool(pSubItem));

        std::int64_t lReceiptAmountMultiplier = 1;  // needed for outbox items.
        const Ledger* pLedger = nullptr;

        switch (pSubItem->GetType()) {
            case itemType::voucherReceipt:
            case itemType::chequeReceipt:
            case itemType::marketReceipt:
            case itemType::paymentReceipt:
            case itemType::transferReceipt:
            case itemType::basketReceipt:
            case itemType::finalReceipt: {
                nInboxItemCount++;
                pLedger = &THE_INBOX;
                pszLedgerType = szInbox;
            }  // intentional fall through
            case itemType::transfer: {
                break;
            }
            default: {
                auto strItemType = String::Factory();
                GetTypeString(strItemType);
                LogDetail(OT_METHOD)(__FUNCTION__)(": Ignoring ")(strItemType)(
                    " item in balance statement while "
                    "verifying it against inbox.")
                    .Flush();
            }
                continue;
        }

        switch (pSubItem->GetType()) {
            case itemType::transfer: {
                if (pSubItem->GetAmount() < 0) {  // it's an outbox item
                    // transfers out always reduce your balance.
                    lReceiptAmountMultiplier = -1;
                    nOutboxItemCount++;
                    pLedger = &THE_OUTBOX;
                    pszLedgerType = szOutbox;
                } else {
                    // transfers in always increase your balance.
                    lReceiptAmountMultiplier = 1;
                    nInboxItemCount++;
                    pLedger = &THE_INBOX;
                    pszLedgerType = szInbox;
                }
            } break;
            // Here: If there is a finalReceipt on this balance statement, then
            // ALL the other related receipts in the inbox (with same "reference
            // to" value) had better ALSO be on the same balance statement!
            // HMM that is true, but NOT HERE... That's only true when
            // PROCESSING the final Receipt from the inbox (in that case, all
            // the marketReceipts must also be processed with it.) But here, I
            // am looping through the inbox report, and there happens to be a
            // finalReceipt on it. (Which doesn't mean necessarily that it's
            // being processed out...)
            case itemType::finalReceipt:
            case itemType::basketReceipt:
            case itemType::transferReceipt:
            case itemType::voucherReceipt:
            case itemType::chequeReceipt:
            case itemType::marketReceipt:
            case itemType::paymentReceipt: {
                lReceiptAmountMultiplier = 1;
            } break;
            default: {
                LogOutput(OT_METHOD)(__FUNCTION__)(": Bad Subitem type "
                                                   "(SHOULD NEVER HAPPEN)....")
                    .Flush();
            }
                continue;  // This will never happen, due to the first continue
                           // above in the first switch.
        }

        std::shared_ptr<OTTransaction> pTransaction;

        // In the special case of account transfer, the user has put an outbox
        // transaction into his balance agreement with the special number '1',
        // since he has no idea what actual number will be generated on the
        // server side (for the outbox) when his message is received by the
        // server.
        //
        // When that happens (ONLY in account transfer) then outboxNum will be
        // passed in with the new transaction number chosen by the server (a
        // real number, like 18736 or whatever, instead of the default of 0 that
        // will otherwise be passed in here.)
        //
        // Therefore, if outboxNum is larger than 0, AND if we're on an outbox
        // item, then we can expect outboxNum to contain an actual transaction
        // number, and we can expect there is a CHANCE that the sub-item will be
        // trans# 1. (It might NOT be number 1, since there may be other outbox
        // items-we're looping through them right now in this block.) So we'll
        // check to see if this is the '1' and if so, we'll look up pTransaction
        // from the outbox using the real transaction number, instead of '1'
        // which of course would not find it (since the version in the ledger
        // contains the ACTUAL number now, since the server just issued it.)
        if ((outboxNum > 0) && (&THE_OUTBOX == pLedger) &&
            (pSubItem->GetTransactionNum() == 1))  // TODO use a constant for
                                                   // this 1.
        {
            LogDebug(OT_METHOD)(__FUNCTION__)(
                " : Subitem is new Outbox Transaction... "
                " retrieving by special ID: ")(outboxNum)
                .Flush();

            pTransaction = pLedger->GetTransaction(outboxNum);
        } else {
            LogTrace(OT_METHOD)(__FUNCTION__)(
                ": Subitem is normal Transaction... retrieving by ID: ")(
                pSubItem->GetTransactionNum())
                .Flush();
            pTransaction =
                pLedger->GetTransaction(pSubItem->GetTransactionNum());
        }

        // Make sure that the transaction number of each sub-item is found on
        // the appropriate ledger (inbox or outbox).
        if (false == bool(pTransaction)) {
            LogNormal(OT_METHOD)(__FUNCTION__)(": Expected ")(pszLedgerType)(
                " transaction (server ")(outboxNum)(", client ")(
                pSubItem->GetTransactionNum())(") not found. (Amount ")(
                pSubItem->GetAmount())(").")
                .Flush();

            return false;
        }

        // pTransaction is set below this point.

        if (pSubItem->GetReferenceToNum() !=
            pTransaction->GetReferenceToNum()) {
            LogNormal(OT_METHOD)(__FUNCTION__)(": Transaction (")(
                pSubItem->GetTransactionNum())(") mismatch Reference Num: ")(
                pSubItem->GetReferenceToNum())(", expected ")(
                pTransaction->GetReferenceToNum())(".")
                .Flush();

            return false;
        }

        if (pSubItem->GetRawNumberOfOrigin() !=
            pTransaction->GetRawNumberOfOrigin()) {
            LogNormal(OT_METHOD)(__FUNCTION__)(": Transaction (")(
                pSubItem->GetTransactionNum())(") mismatch Origin Num: ")(
                pSubItem->GetRawNumberOfOrigin())(", expected ")(
                pTransaction->GetRawNumberOfOrigin())(".")
                .Flush();

            return false;
        }

        std::int64_t lTransactionAmount = pTransaction->GetReceiptAmount();
        lTransactionAmount *= lReceiptAmountMultiplier;

        if (pSubItem->GetAmount() != lTransactionAmount) {
            LogNormal(OT_METHOD)(__FUNCTION__)(": Transaction (")(
                pSubItem->GetTransactionNum())(
                ") amounts don't match: report amount is ")(
                pSubItem->GetAmount())(", but expected ")(lTransactionAmount)(
                ". Trans Receipt Amt: ")(pTransaction->GetReceiptAmount())(
                " (GetAmount() == ")(GetAmount())(").")
                .Flush();

            return false;
        }

        if ((pSubItem->GetType() == itemType::transfer) &&
            (pTransaction->GetType() != transactionType::pending)) {
            LogNormal(OT_METHOD)(__FUNCTION__)(": Transaction (")(
                pSubItem->GetTransactionNum())(
                ") wrong type. (transfer block).")
                .Flush();

            return false;
        }

        if ((pSubItem->GetType() == itemType::chequeReceipt) &&
            (pTransaction->GetType() != transactionType::chequeReceipt)) {
            LogNormal(OT_METHOD)(__FUNCTION__)(": Transaction (")(
                pSubItem->GetTransactionNum())(
                ") wrong type. (chequeReceipt block).")
                .Flush();

            return false;
        }

        if ((pSubItem->GetType() == itemType::voucherReceipt) &&
            ((pTransaction->GetType() != transactionType::voucherReceipt) ||
             (pSubItem->GetOriginType() != pTransaction->GetOriginType()))) {
            LogNormal(OT_METHOD)(__FUNCTION__)(": Transaction (")(
                pSubItem->GetTransactionNum())(
                ") wrong type or origin type. (voucherReceipt block).")
                .Flush();

            return false;
        }

        if ((pSubItem->GetType() == itemType::marketReceipt) &&
            (pTransaction->GetType() != transactionType::marketReceipt)) {
            LogNormal(OT_METHOD)(__FUNCTION__)(": Transaction (")(
                pSubItem->GetTransactionNum())(
                ") wrong type. (marketReceipt block).")
                .Flush();

            return false;
        }

        if ((pSubItem->GetType() == itemType::paymentReceipt) &&
            ((pTransaction->GetType() != transactionType::paymentReceipt) ||
             (pSubItem->GetOriginType() != pTransaction->GetOriginType()))) {
            LogNormal(OT_METHOD)(__FUNCTION__)(": Transaction (")(
                pSubItem->GetTransactionNum())(
                ") wrong type or origin type. (paymentReceipt block).")
                .Flush();

            return false;
        }

        if ((pSubItem->GetType() == itemType::transferReceipt) &&
            (pTransaction->GetType() != transactionType::transferReceipt)) {
            LogNormal(OT_METHOD)(__FUNCTION__)(": Transaction (")(
                pSubItem->GetTransactionNum())(
                ") wrong type. (transferReceipt block).")
                .Flush();

            return false;
        }

        if ((pSubItem->GetType() == itemType::basketReceipt) &&
            ((pTransaction->GetType() != transactionType::basketReceipt) ||
             (pSubItem->GetClosingNum() != pTransaction->GetClosingNum()))) {
            LogNormal(OT_METHOD)(__FUNCTION__)(": Transaction (")(
                pSubItem->GetTransactionNum())(") wrong type or closing num (")(
                pSubItem->GetClosingNum())("). (basketReceipt block).")
                .Flush();

            return false;
        }

        if ((pSubItem->GetType() == itemType::finalReceipt) &&
            ((pTransaction->GetType() != transactionType::finalReceipt) ||
             (pSubItem->GetClosingNum() != pTransaction->GetClosingNum()) ||
             (pSubItem->GetOriginType() != pTransaction->GetOriginType()))) {
            LogNormal(OT_METHOD)(__FUNCTION__)(": Transaction (")(
                pSubItem->GetTransactionNum())(
                ") wrong type or origin type or closing num (")(
                pSubItem->GetClosingNum())("). (finalReceipt block).")
                .Flush();

            return false;
        }
    }

    // By this point, I have an accurate count of the inbox items, and outbox
    // items, represented by this. let's compare those counts to the actual
    // inbox and outbox on my side:
    if ((nInboxItemCount != THE_INBOX.GetTransactionCount()) ||
        (nOutboxItemCount != THE_OUTBOX.GetTransactionCount())) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Inbox or Outbox mismatch in expected transaction count."
            " --- THE_INBOX count: ")(THE_INBOX.GetTransactionCount())(
            " --- THE_OUTBOX count: ")(THE_OUTBOX.GetTransactionCount())(
            " --- nInboxItemCount count: ")(nInboxItemCount)(
            " --- nOutboxItemCount count: ")(nOutboxItemCount)(".")
            .Flush();

        return false;
    }

    // Now I KNOW that the inbox and outbox counts are the same, AND I know that
    // EVERY transaction number on the balance item (this) was also found in the
    // inbox or outbox, wherever it was expected to be found. I also know:
    // * the amount was correct,
    // * the "in reference to" number was correct,
    // * and the type was correct.
    //
    // So if the caller was planning to remove a number, or clear a receipt from
    // the inbox, he'll have to do so first before calling this function,
    // andGetTransactionNum
    // then ADD IT AGAIN if this function fails.  (Because the new Balance
    // Agreement is always the user signing WHAT THE NEW VERSION WILL BE AFTER
    // THE TRANSACTION IS PROCESSED. Thus, if the transaction fails to process,
    // the action hasn't really happened, so need to add it back again.)
    // 3) Also need to verify the transactions on the Nym, against the
    // transactions stored on this (in a message Nym attached to this.) Check
    // for presence of each, then compare count, like above.
    const auto notaryID = Identifier::Factory(GetPurportedNotaryID());
    const auto notary = String::Factory(notaryID);
    const auto targetNumber = GetTransactionNum();

    // GetTransactionNum() is the ID for this balance agreement, THUS it's also
    // the ID for whatever actual transaction is being attempted. If that ID is
    // not verified as on my issued list, then the whole transaction is invalid
    // (not authorized.)
    const bool bIWasFound = context.VerifyIssuedNumber(targetNumber, removed);

    if (!bIWasFound) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Transaction has a # that "
            "doesn't appear on Nym's issued list.")
            .Flush();

        return false;
    }

    // BELOW THIS POINT, WE *KNOW* THE ISSUED NUM IS CURRENTLY ON THE LIST...
    // (SO I CAN remove it and add it again, KNOWING that I'm never re-adding a
    // num that wasn't there in the first place. For process inbox, deposit, and
    // withdrawal, the client will remove from issued list as soon as he
    // receives my acknowledgment OR rejection. He expects server (me) to
    // remove, so he signs a balance agreement to that effect. (With the number
    // removed from issued list.)
    //
    // Therefore, to verify the balance agreement, we remove it on our side as
    // well, so that they will match. The picture thus formed is what would be
    // correct assuming a successful transaction. That way if the transaction
    // goes through, we have our signed receipt showing the new state of things
    // (without which we would not permit the transaction to go through :)
    //
    // This allows the client side to then ACTUALLY remove the number when they
    // receive our response, as well as permits me (server) to actually remove
    // from issued list.
    //
    // If ANYTHING ELSE fails during this verify process (other than
    // processInbox, deposit, and withdraw) then we have to ADD THE # AGAIN
    // since we still don't have a valid signature on that number. So you'll see
    // this code repeated a few times in reverse, down inside this function. For
    // example,
    switch (TARGET_TRANSACTION.GetType()) {
        case transactionType::processInbox:
        case transactionType::withdrawal:
        case transactionType::deposit:
        case transactionType::payDividend:
        case transactionType::cancelCronItem:
        case transactionType::exchangeBasket: {
            removed.insert(targetNumber);
            LogDetail(OT_METHOD)(__FUNCTION__)(": Transaction number: ")(
                targetNumber)(" from TARGET_TRANSACTION "
                              "is being closed.")
                .Flush();
        } break;
        case transactionType::transfer:
        case transactionType::marketOffer:
        case transactionType::paymentPlan:
        case transactionType::smartContract: {
            // These, assuming success, do NOT remove an issued number. So no
            // need to anticipate setting up the list that way, to get a match.
            LogDetail(OT_METHOD)(__FUNCTION__)(": Transaction number: ")(
                targetNumber)(" from TARGET_TRANSACTION "
                              "will remain open.")
                .Flush();
        } break;
        default: {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Wrong target transaction type: ")(
                TARGET_TRANSACTION.GetTypeString())(".")
                .Flush();
        } break;
    }

    auto serialized = String::Factory();
    GetAttachment(serialized);

    if (3 > serialized->GetLength()) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Unable to decode transaction statement...")
            .Flush();

        return false;
    }

    TransactionStatement statement(serialized);
    std::set<TransactionNumber> added;

    return context.Verify(statement, removed, added);
}

// You have to allocate the item on the heap and then pass it in as a reference.
// OTTransaction will take care of it from there and will delete it in
// destructor.
void Item::AddItem(std::shared_ptr<Item> theItem)
{
    m_listItems.push_back(theItem);
}

// While processing a transaction, you may wish to query it for items of a
// certain type.
std::shared_ptr<Item> Item::GetItem(std::int32_t nIndex)
{
    std::int32_t nTempIndex = (-1);

    for (auto& it : m_listItems) {
        const auto pItem = it;
        OT_ASSERT(false != bool(pItem));

        nTempIndex++;  // first iteration this becomes 0 here.

        if (nTempIndex == nIndex) return pItem;
    }

    return nullptr;
}

std::shared_ptr<const Item> Item::GetItem(std::int32_t nIndex) const
{
    std::int32_t nTempIndex = (-1);

    for (auto& it : m_listItems) {
        const auto pItem = it;
        OT_ASSERT(false != bool(pItem));

        nTempIndex++;  // first iteration this becomes 0 here.

        if (nTempIndex == nIndex) return pItem;
    }

    return nullptr;
}

// While processing an item, you may wish to query it for sub-items
std::shared_ptr<Item> Item::GetItemByTransactionNum(
    std::int64_t lTransactionNumber)
{
    for (auto& it : m_listItems) {
        const auto pItem = it;
        OT_ASSERT(false != bool(pItem));

        if (pItem->GetTransactionNum() == lTransactionNumber) return pItem;
    }

    return nullptr;
}

// Count the number of items that are IN REFERENCE TO some transaction#.
//
// Might want to change this so that it only counts ACCEPTED receipts.
//
std::int32_t Item::GetItemCountInRefTo(std::int64_t lReference)
{
    std::int32_t nCount = 0;

    for (auto& it : m_listItems) {
        const auto pItem = it;
        OT_ASSERT(false != bool(pItem));

        if (pItem->GetReferenceToNum() == lReference) nCount++;
    }

    return nCount;
}

// The final receipt item MAY be present, and co-relates to others that share
// its "in reference to" value. (Others such as marketReceipts and
// paymentReceipts.)
//
std::shared_ptr<Item> Item::GetFinalReceiptItemByReferenceNum(
    std::int64_t lReferenceNumber)
{
    for (auto& it : m_listItems) {
        const auto pItem = it;
        OT_ASSERT(false != bool(pItem));

        if (itemType::finalReceipt != pItem->GetType()) continue;
        if (pItem->GetReferenceToNum() == lReferenceNumber) return pItem;
    }

    return nullptr;
}

// For "Item::acceptTransaction"
//
bool Item::AddBlankNumbersToItem(const NumList& theAddition)
{
    return m_Numlist.Add(theAddition);
}

// Need to know the transaction number of the ORIGINAL transaction? Call this.
// virtual
std::int64_t Item::GetNumberOfOrigin()
{

    if (0 == m_lNumberOfOrigin) {
        switch (GetType()) {
            case itemType::acceptPending:  // this item is a client-side
                                           // acceptance of a pending transfer
            case itemType::rejectPending:  // this item is a client-side
                                           // rejection of a pending transfer
            case itemType::acceptCronReceipt:   // this item is a client-side
                                                // acceptance of
                                                // a
                                                // cron receipt in his inbox.
            case itemType::acceptItemReceipt:   // this item is a client-side
                                                // acceptance of an item receipt
                                                // in his inbox.
            case itemType::disputeCronReceipt:  // this item is a client dispute
                                                // of a cron receipt in his
                                                // inbox.
            case itemType::disputeItemReceipt:  // this item is a client dispute
                                                // of an item receipt in his
                                                // inbox.

            case itemType::acceptFinalReceipt:  // this item is a client-side
                                                // acceptance of a
            // final receipt in his inbox. (All related
            // receipts must also be closed!)
            case itemType::acceptBasketReceipt:  // this item is a client-side
                                                 // acceptance of a basket
                                                 // receipt in his inbox.
            case itemType::disputeFinalReceipt:  // this item is a client-side
                                                 // rejection of a
            // final receipt in his inbox. (All related
            // receipts must also be closed!)
            case itemType::disputeBasketReceipt:  // this item is a client-side
                                                  // rejection of a basket
                                                  // receipt in his inbox.

                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": In this case, you can't calculate the "
                    "origin number, you must set it "
                    "explicitly.")
                    .Flush();
                // Comment this out later so people can't use it to crash the
                // server:
                OT_FAIL_MSG(
                    "In this case, you can't calculate the origin number, "
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
void Item::CalculateNumberOfOrigin()
{
    switch (GetType()) {
        case itemType::acceptTransaction:  // this item is a client-side
                                           // acceptance of a transaction number
                                           // (a blank) in my Nymbox
        case itemType::atAcceptTransaction:  // server reply
        case itemType::acceptMessage:  // this item is a client-side acceptance
                                       // of a message in my Nymbox
        case itemType::atAcceptMessage:  // server reply
        case itemType::acceptNotice:    // this item is a client-side acceptance
                                        // of a server notification in my Nymbox
        case itemType::atAcceptNotice:  // server reply
        case itemType::replyNotice:  // server notice of a reply that nym should
                                     // have already
        // received as a response to a request. (Copy dropped in
        // nymbox.)
        case itemType::successNotice:  // server notice dropped into nymbox as
                                       // result of a transaction# being
                                       // successfully signed out.
        case itemType::notice:  // server notice dropped into nymbox as result
                                // of a smart contract processing.
        case itemType::transferReceipt:  // Currently don't create an Item for
                                         // transfer receipt in inbox. Used only
                                         // for inbox report.
        case itemType::chequeReceipt:    // Currently don't create an Item for
                                       // cheque receipt in inbox. Used only for
                                       // inbox report.
        case itemType::voucherReceipt:  // Currently don't create an Item for
                                        // voucher receipt in inbox. Used only
                                        // for inbox report.

            SetNumberOfOrigin(0);  // Not applicable.
            break;

        case itemType::acceptPending:  // this item is a client-side acceptance
                                       // of a pending transfer
        case itemType::rejectPending:  // this item is a client-side rejection
                                       // of a pending transfer
        case itemType::acceptCronReceipt:   // this item is a client-side
                                            // acceptance of a cron receipt in
                                            // his inbox.
        case itemType::acceptItemReceipt:   // this item is a client-side
                                            // acceptance of an item receipt in
                                            // his inbox.
        case itemType::disputeCronReceipt:  // this item is a client dispute of
                                            // a cron receipt in his inbox.
        case itemType::disputeItemReceipt:  // this item is a client dispute of
                                            // an item receipt in his inbox.

        case itemType::acceptFinalReceipt:  // this item is a client-side
                                            // acceptance of a final
        // receipt in his inbox. (All related receipts must
        // also be closed!)
        case itemType::acceptBasketReceipt:  // this item is a client-side
                                             // acceptance of a basket receipt
                                             // in his inbox.
        case itemType::disputeFinalReceipt:  // this item is a client-side
                                             // rejection of a final
        // receipt in his inbox. (All related receipts
        // must also be closed!)
        case itemType::disputeBasketReceipt:  // this item is a client-side
                                              // rejection of a basket receipt
                                              // in his inbox.

            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": In this case, you can't calculate the "
                "origin number, you must set it explicitly.")
                .Flush();
            SetNumberOfOrigin(0);  // Not applicable.
            // Comment this out later so people can't use it to crash the
            // server:
            OT_FAIL_MSG(
                "In this case, you can't calculate the origin number, you "
                "must set it explicitly.");
            break;

        case itemType::marketReceipt:  // server receipt dropped into inbox as
                                       // result of market trading. Also used in
                                       // inbox report.
        case itemType::paymentReceipt:  // server receipt dropped into an inbox
                                        // as result of payment occuring. Also
                                        // used in inbox report.
        case itemType::finalReceipt:    // server receipt dropped into inbox /
                                      // nymbox as result of cron item expiring
                                      // or being canceled.
        case itemType::basketReceipt:  // server receipt dropped into inbox as
                                       // result of a basket exchange.

            SetNumberOfOrigin(GetReferenceToNum());  // pending is in
                                                     // reference to the
                                                     // original
                                                     // transfer.
            break;

        case itemType::depositCheque:  // this item is a request to deposit a
                                       // cheque.
        {
            const auto theCheque{api_.Factory().Cheque()};
            auto strAttachment = String::Factory();
            GetAttachment(strAttachment);

            if (!theCheque->LoadContractFromString(strAttachment))
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": ERROR loading cheque from string: ")(strAttachment)(".")
                    .Flush();
            else
                SetNumberOfOrigin(theCheque->GetTransactionNum());
        } break;

        case itemType::atDepositCheque:  // this item is a server response to
                                         // that request.
        case itemType::atAcceptPending:  // server reply to acceptPending.
        case itemType::atRejectPending:  // server reply to rejectPending.
        case itemType::atAcceptCronReceipt:    // this item is a server reply to
                                               // that acceptance.
        case itemType::atAcceptItemReceipt:    // this item is a server reply to
                                               // that acceptance.
        case itemType::atDisputeCronReceipt:   // Server reply to dispute
                                               // message.
        case itemType::atDisputeItemReceipt:   // Server reply to dispute
                                               // message.
        case itemType::atAcceptFinalReceipt:   // server reply
        case itemType::atAcceptBasketReceipt:  // server reply
        case itemType::atDisputeFinalReceipt:  // server reply
        case itemType::atDisputeBasketReceipt:  // server reply
        {
            auto strReference = String::Factory();
            GetReferenceString(strReference);

            // "In reference to" number is my original deposit trans#, which I
            // use
            // here to load my
            // original depositCheque item, which I use to get the cheque, which
            // contains the number
            // of origin as its transaction number.
            //
            const auto pOriginalItem{api_.Factory().Item(
                strReference, GetPurportedNotaryID(), GetReferenceToNum())};
            OT_ASSERT(false != bool(pOriginalItem));

            if (((m_Type == itemType::atDepositCheque) &&
                 (itemType::depositCheque != pOriginalItem->GetType())) ||
                ((m_Type == itemType::atAcceptPending) &&
                 (itemType::acceptPending != pOriginalItem->GetType())) ||
                ((m_Type == itemType::atRejectPending) &&
                 (itemType::rejectPending != pOriginalItem->GetType())) ||
                ((m_Type == itemType::atAcceptCronReceipt) &&
                 (itemType::acceptCronReceipt != pOriginalItem->GetType())) ||
                ((m_Type == itemType::atAcceptItemReceipt) &&
                 (itemType::acceptItemReceipt != pOriginalItem->GetType())) ||
                ((m_Type == itemType::atDisputeCronReceipt) &&
                 (itemType::disputeCronReceipt != pOriginalItem->GetType())) ||
                ((m_Type == itemType::atDisputeItemReceipt) &&
                 (itemType::disputeItemReceipt != pOriginalItem->GetType())) ||
                ((m_Type == itemType::atAcceptFinalReceipt) &&
                 (itemType::acceptFinalReceipt != pOriginalItem->GetType())) ||
                ((m_Type == itemType::atAcceptBasketReceipt) &&
                 (itemType::acceptBasketReceipt != pOriginalItem->GetType())) ||
                ((m_Type == itemType::atDisputeFinalReceipt) &&
                 (itemType::disputeFinalReceipt != pOriginalItem->GetType())) ||
                ((m_Type == itemType::atDisputeBasketReceipt) &&
                 (itemType::disputeBasketReceipt !=
                  pOriginalItem->GetType()))) {
                auto strType = String::Factory();
                pOriginalItem->GetTypeString(strType);
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": ERROR: Wrong item type as 'in "
                    "reference to' string on ")(strType)(" item.")
                    .Flush();
                SetNumberOfOrigin(0);
                return;
            }

            // Else:
            SetNumberOfOrigin(pOriginalItem->GetNumberOfOrigin());
        } break;

        // FEEs
        case itemType::serverfee:  // this item is a fee from the transaction
                                   // server (per contract)
        case itemType::atServerfee:
        case itemType::issuerfee:  // this item is a fee from the issuer (per
                                   // contract)
        case itemType::atIssuerfee:

        // INFO (BALANCE, HASH, etc) these are still all messages with replies.
        case itemType::balanceStatement:  // this item is a statement of
                                          // balance. (For asset account.)
        case itemType::atBalanceStatement:
        case itemType::transactionStatement:  // this item is a transaction
                                              // statement. (For Nym
                                              // -- which numbers are assigned
                                              // to him.)
        case itemType::atTransactionStatement:

        // TRANSFER
        case itemType::transfer:  // This item is an outgoing transfer, probably
                                  // part of an outoing transaction.
        case itemType::atTransfer:  // Server reply.

        // CASH WITHDRAWAL / DEPOSIT
        case itemType::withdrawal:  // this item is a cash withdrawal (of
                                    // chaumian blinded tokens)
        case itemType::atWithdrawal:
        case itemType::deposit:  // this item is a cash deposit (of a purse
                                 // containing blinded tokens.)
        case itemType::atDeposit:

        // CHEQUES AND VOUCHERS
        case itemType::withdrawVoucher:  // this item is a request to purchase a
                                         // voucher (a cashier's cheque)
        case itemType::atWithdrawVoucher:

        // PAYING DIVIDEND ON SHARES OF STOCK
        case itemType::payDividend:    // this item is a request to pay a
                                       // dividend.
        case itemType::atPayDividend:  // the server reply to that request.

        // TRADING ON MARKETS
        case itemType::marketOffer:    // this item is an offer to be put on a
                                       // market.
        case itemType::atMarketOffer:  // server reply or updated notification
                                       // regarding a market offer.

        // PAYMENT PLANS
        case itemType::paymentPlan:    // this item is a new payment plan
        case itemType::atPaymentPlan:  // server reply or updated notification
                                       // regarding a payment plan.

        // SMART CONTRACTS
        case itemType::smartContract:    // this item is a new smart contract
        case itemType::atSmartContract:  // server reply or updated notification
                                         // regarding
                                         // a
                                         // smart contract.

        // CANCELLING: Market Offers and Payment Plans.
        case itemType::cancelCronItem:    // this item is intended to cancel a
                                          // market offer or payment plan.
        case itemType::atCancelCronItem:  // reply from the server regarding
                                          // said cancellation.

        // EXCHANGE IN/OUT OF A BASKET CURRENCY
        case itemType::exchangeBasket:  // this item is an exchange in/out of a
                                        // basket currency.
        case itemType::atExchangeBasket:  // reply from the server regarding
                                          // said exchange.

        default:
            SetNumberOfOrigin(GetTransactionNum());
            break;
    }  // switch
}  // namespace opentxs

void Item::GetAttachment(String& theStr) const
{
    m_ascAttachment->GetString(theStr);
}

void Item::SetAttachment(const String& theStr)
{
    m_ascAttachment->SetString(theStr);
}

void Item::SetNote(const String& theStr)
{
    if (theStr.Exists() && theStr.GetLength() > 2) {
        m_ascNote->SetString(theStr);
    } else {
        m_ascNote->Release();
    }
}

void Item::GetNote(String& theStr) const
{
    if (m_ascNote->GetLength() > 2) {
        m_ascNote->GetString(theStr);
    } else {
        theStr.Release();
    }
}

void Item::InitItem()
{
    m_lAmount = 0;  // Accounts default to ZERO.  They can only change that
                    // amount by receiving from another account.
    m_Type = itemType::error_state;
    m_Status = request;        // (Unless an issuer account, which can
                               // create currency
    m_lNewOutboxTransNum = 0;  // When the user puts a "1" in his outbox for a
                               // balance agreement (since he doesn't know what
                               // trans# the actual outbox item
    // will have if the transaction is successful, since the server hasn't
    // issued it yet) then the balance receipt will have 1 in
    // the user's portion for that outbox transaction, and the balance receipt
    // will also have, say, #34 (the actual number) here
    // in this variable, in the server's reply portion of that same receipt.

    m_lClosingTransactionNo = 0;

    m_strContractType =
        String::Factory("TRANSACTION ITEM");  // CONTRACT, MESSAGE, TRANSACTION,
                                              // LEDGER, TRANSACTION ITEM
}

void Item::Release()
{
    Release_Item();

    ot_super::Release();
}

void Item::Release_Item()
{
    ReleaseItems();

    m_AcctToID->Release();
    m_lAmount = 0;
    m_lNewOutboxTransNum = 0;
    m_lClosingTransactionNo = 0;
}

void Item::ReleaseItems() { m_listItems.clear(); }

itemType Item::GetItemTypeFromString(const String& strType)
{
    itemType theType = itemType::error_state;

    if (strType.Compare("transfer"))
        theType = itemType::transfer;
    else if (strType.Compare("atTransfer"))
        theType = itemType::atTransfer;

    else if (strType.Compare("acceptTransaction"))
        theType = itemType::acceptTransaction;
    else if (strType.Compare("atAcceptTransaction"))
        theType = itemType::atAcceptTransaction;

    else if (strType.Compare("acceptMessage"))
        theType = itemType::acceptMessage;
    else if (strType.Compare("atAcceptMessage"))
        theType = itemType::atAcceptMessage;

    else if (strType.Compare("acceptNotice"))
        theType = itemType::acceptNotice;
    else if (strType.Compare("atAcceptNotice"))
        theType = itemType::atAcceptNotice;

    else if (strType.Compare("acceptPending"))
        theType = itemType::acceptPending;
    else if (strType.Compare("atAcceptPending"))
        theType = itemType::atAcceptPending;
    else if (strType.Compare("rejectPending"))
        theType = itemType::rejectPending;
    else if (strType.Compare("atRejectPending"))
        theType = itemType::atRejectPending;

    else if (strType.Compare("acceptCronReceipt"))
        theType = itemType::acceptCronReceipt;
    else if (strType.Compare("atAcceptCronReceipt"))
        theType = itemType::atAcceptCronReceipt;
    else if (strType.Compare("disputeCronReceipt"))
        theType = itemType::disputeCronReceipt;
    else if (strType.Compare("atDisputeCronReceipt"))
        theType = itemType::atDisputeCronReceipt;
    else if (strType.Compare("acceptItemReceipt"))
        theType = itemType::acceptItemReceipt;
    else if (strType.Compare("atAcceptItemReceipt"))
        theType = itemType::atAcceptItemReceipt;
    else if (strType.Compare("disputeItemReceipt"))
        theType = itemType::disputeItemReceipt;
    else if (strType.Compare("atDisputeItemReceipt"))
        theType = itemType::atDisputeItemReceipt;

    else if (strType.Compare("acceptFinalReceipt"))
        theType = itemType::acceptFinalReceipt;
    else if (strType.Compare("atAcceptFinalReceipt"))
        theType = itemType::atAcceptFinalReceipt;
    else if (strType.Compare("disputeFinalReceipt"))
        theType = itemType::disputeFinalReceipt;
    else if (strType.Compare("atDisputeFinalReceipt"))
        theType = itemType::atDisputeFinalReceipt;

    else if (strType.Compare("acceptBasketReceipt"))
        theType = itemType::acceptBasketReceipt;
    else if (strType.Compare("atAcceptBasketReceipt"))
        theType = itemType::atAcceptBasketReceipt;
    else if (strType.Compare("disputeBasketReceipt"))
        theType = itemType::disputeBasketReceipt;
    else if (strType.Compare("atDisputeBasketReceipt"))
        theType = itemType::atDisputeBasketReceipt;

    else if (strType.Compare("serverfee"))
        theType = itemType::serverfee;
    else if (strType.Compare("atServerfee"))
        theType = itemType::atServerfee;
    else if (strType.Compare("issuerfee"))
        theType = itemType::issuerfee;
    else if (strType.Compare("atIssuerfee"))
        theType = itemType::atIssuerfee;

    else if (strType.Compare("balanceStatement"))
        theType = itemType::balanceStatement;
    else if (strType.Compare("atBalanceStatement"))
        theType = itemType::atBalanceStatement;
    else if (strType.Compare("transactionStatement"))
        theType = itemType::transactionStatement;
    else if (strType.Compare("atTransactionStatement"))
        theType = itemType::atTransactionStatement;

    else if (strType.Compare("withdrawal"))
        theType = itemType::withdrawal;
    else if (strType.Compare("atWithdrawal"))
        theType = itemType::atWithdrawal;
    else if (strType.Compare("deposit"))
        theType = itemType::deposit;
    else if (strType.Compare("atDeposit"))
        theType = itemType::atDeposit;

    else if (strType.Compare("withdrawVoucher"))
        theType = itemType::withdrawVoucher;
    else if (strType.Compare("atWithdrawVoucher"))
        theType = itemType::atWithdrawVoucher;
    else if (strType.Compare("depositCheque"))
        theType = itemType::depositCheque;
    else if (strType.Compare("atDepositCheque"))
        theType = itemType::atDepositCheque;

    else if (strType.Compare("payDividend"))
        theType = itemType::payDividend;
    else if (strType.Compare("atPayDividend"))
        theType = itemType::atPayDividend;

    else if (strType.Compare("marketOffer"))
        theType = itemType::marketOffer;
    else if (strType.Compare("atMarketOffer"))
        theType = itemType::atMarketOffer;

    else if (strType.Compare("paymentPlan"))
        theType = itemType::paymentPlan;
    else if (strType.Compare("atPaymentPlan"))
        theType = itemType::atPaymentPlan;

    else if (strType.Compare("smartContract"))
        theType = itemType::smartContract;
    else if (strType.Compare("atSmartContract"))
        theType = itemType::atSmartContract;

    else if (strType.Compare("cancelCronItem"))
        theType = itemType::cancelCronItem;
    else if (strType.Compare("atCancelCronItem"))
        theType = itemType::atCancelCronItem;

    else if (strType.Compare("exchangeBasket"))
        theType = itemType::exchangeBasket;
    else if (strType.Compare("atExchangeBasket"))
        theType = itemType::atExchangeBasket;

    else if (strType.Compare("chequeReceipt"))
        theType = itemType::chequeReceipt;
    else if (strType.Compare("voucherReceipt"))
        theType = itemType::voucherReceipt;
    else if (strType.Compare("marketReceipt"))
        theType = itemType::marketReceipt;
    else if (strType.Compare("paymentReceipt"))
        theType = itemType::paymentReceipt;
    else if (strType.Compare("transferReceipt"))
        theType = itemType::transferReceipt;

    else if (strType.Compare("finalReceipt"))
        theType = itemType::finalReceipt;
    else if (strType.Compare("basketReceipt"))
        theType = itemType::basketReceipt;

    else if (strType.Compare("replyNotice"))
        theType = itemType::replyNotice;
    else if (strType.Compare("successNotice"))
        theType = itemType::successNotice;
    else if (strType.Compare("notice"))
        theType = itemType::notice;

    else
        theType = itemType::error_state;

    return theType;
}

// return -1 if error, 0 if nothing, and 1 if the node was processed.
std::int32_t Item::ProcessXMLNode(irr::io::IrrXMLReader*& xml)
{
    if (!strcmp("item", xml->getNodeName())) {
        auto strType = String::Factory(), strStatus = String::Factory();

        strType = String::Factory(xml->getAttributeValue("type"));
        strStatus = String::Factory(xml->getAttributeValue("status"));

        // Type
        m_Type = GetItemTypeFromString(strType);  // just above.

        // Status
        if (strStatus->Compare("request"))
            m_Status = request;
        else if (strStatus->Compare("acknowledgement"))
            m_Status = acknowledgement;
        else if (strStatus->Compare("rejection"))
            m_Status = rejection;
        else
            m_Status = error_status;

        auto strAcctFromID = String::Factory(), strAcctToID = String::Factory(),
             strNotaryID = String::Factory(), strNymID = String::Factory(),
             strOutboxNewTransNum = String::Factory();

        strAcctFromID =
            String::Factory(xml->getAttributeValue("fromAccountID"));
        strAcctToID = String::Factory(xml->getAttributeValue("toAccountID"));
        strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
        strNymID = String::Factory(xml->getAttributeValue("nymID"));

        strOutboxNewTransNum =
            String::Factory(xml->getAttributeValue("outboxNewTransNum"));

        if (strOutboxNewTransNum->Exists())
            m_lNewOutboxTransNum = strOutboxNewTransNum->ToLong();

        // an OTTransaction::blank may now contain 20 or 100 new numbers.
        // Therefore, the Item::acceptTransaction must contain the same list,
        // otherwise you haven't actually SIGNED for the list, have you!
        //
        if (itemType::acceptTransaction == m_Type) {
            const auto strTotalList =
                String::Factory(xml->getAttributeValue("totalListOfNumbers"));
            m_Numlist.Release();

            if (strTotalList->Exists())
                m_Numlist.Add(strTotalList);  // (Comma-separated list of
                                              // numbers now becomes
                                              // std::set<std::int64_t>.)
        }

        auto ACCOUNT_ID = Identifier::Factory(strAcctFromID),
             NOTARY_ID = Identifier::Factory(strNotaryID),
             DESTINATION_ACCOUNT = Identifier::Factory(strAcctToID),
             NYM_ID = Identifier::Factory(strNymID);

        SetPurportedAccountID(ACCOUNT_ID);  // OTTransactionType::m_AcctID  the
                                            // PURPORTED Account ID
        SetPurportedNotaryID(NOTARY_ID);    // OTTransactionType::m_AcctNotaryID
                                            // the PURPORTED Notary ID
        SetDestinationAcctID(DESTINATION_ACCOUNT);
        SetNymID(NYM_ID);

        if (!m_bLoadSecurely) {
            SetRealAccountID(ACCOUNT_ID);
            SetRealNotaryID(NOTARY_ID);
        }

        auto strTemp = String::Factory();

        strTemp = String::Factory(xml->getAttributeValue("numberOfOrigin"));
        if (strTemp->Exists()) SetNumberOfOrigin(strTemp->ToLong());

        strTemp = String::Factory(xml->getAttributeValue("originType"));
        if (strTemp->Exists()) SetOriginType(GetOriginTypeFromString(strTemp));

        strTemp = String::Factory(xml->getAttributeValue("transactionNum"));
        if (strTemp->Exists()) SetTransactionNum(strTemp->ToLong());

        strTemp = String::Factory(xml->getAttributeValue("inReferenceTo"));
        if (strTemp->Exists()) SetReferenceToNum(strTemp->ToLong());

        m_lAmount = String::StringToLong(xml->getAttributeValue("amount"));

        LogDebug(OT_METHOD)(__FUNCTION__)(
            ": Loaded transaction Item, transaction num ")(GetTransactionNum())(
            ", In Reference To: ")(GetReferenceToNum())(", type: ")(strType)(
            ", status: ")(strStatus)
            .Flush();
        //                "fromAccountID:\n%s\n NymID:\n%s\n toAccountID:\n%s\n
        // notaryID:\n%s\n----------\n",
        //                strAcctFromID.Get(), strNymID.Get(),
        // strAcctToID.Get(), strNotaryID.Get()

        return 1;
    } else if (!strcmp("note", xml->getNodeName())) {
        if (!Contract::LoadEncodedTextField(xml, m_ascNote)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error in Item::ProcessXMLNode: note field without "
                "value.")
                .Flush();
            return (-1);  // error condition
        }

        return 1;
    } else if (!strcmp("inReferenceTo", xml->getNodeName())) {
        if (false == Contract::LoadEncodedTextField(xml, m_ascInReferenceTo)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error in Item::ProcessXMLNode: inReferenceTo field "
                "without value.")
                .Flush();
            return (-1);  // error condition
        }

        return 1;
    } else if (!strcmp("attachment", xml->getNodeName())) {
        if (!Contract::LoadEncodedTextField(xml, m_ascAttachment)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error in Item::ProcessXMLNode: attachment field "
                "without value.")
                .Flush();
            return (-1);  // error condition
        }

        return 1;
    } else if (!strcmp("transactionReport", xml->getNodeName())) {
        if ((itemType::balanceStatement == m_Type) ||
            (itemType::atBalanceStatement == m_Type)) {
            // Notice it initializes with the wrong transaction number, in this
            // case.
            // That's okay, because I'm setting it below with
            // pItem->SetTransactionNum...
            std::shared_ptr<Item> pItem{
                new Item(api_, GetNymID(), *this)};  // But I've also got
                                                     // ITEM types with
                                                     // the same names...
            // That way, it will translate the string and set the type
            // correctly.
            OT_ASSERT(false != bool(pItem));  // That way I can use each item to
                                              // REPRESENT an inbox transaction

            // Type
            auto strType = String::Factory();
            strType = String::Factory(xml->getAttributeValue(
                "type"));  // it's reading a TRANSACTION type: chequeReceipt,
                           // voucherReceipt, marketReceipt, or paymentReceipt.
                           // But I also have the same names for item types.

            pItem->SetType(GetItemTypeFromString(strType));  // It's actually
                                                             // translating a
                                                             // transaction type
                                                             // to an
            // item type. (Same names in the case of the 3
            // receipts that matter for inbox reports for balance
            // agreements.)

            pItem->SetAmount(
                String::StringToLong(xml->getAttributeValue("adjustment")));

            // Status
            pItem->SetStatus(acknowledgement);  // I don't need this, but
                                                // I'd rather it not say
                                                // error state. This way
                                                // if it changes to
                                                // error_state later, I
                                                // know I had a problem.

            auto strAccountID = String::Factory(),
                 strNotaryID = String::Factory(), strNymID = String::Factory();

            strAccountID = String::Factory(xml->getAttributeValue("accountID"));
            strNotaryID = String::Factory(xml->getAttributeValue("notaryID"));
            strNymID = String::Factory(xml->getAttributeValue("nymID"));

            auto ACCOUNT_ID = Identifier::Factory(strAccountID),
                 NOTARY_ID = Identifier::Factory(strNotaryID),
                 NYM_ID = Identifier::Factory(strNymID);

            pItem->SetPurportedAccountID(
                ACCOUNT_ID);  // OTTransactionType::m_AcctID
                              // the PURPORTED Account
                              // ID
            pItem->SetPurportedNotaryID(
                NOTARY_ID);  // OTTransactionType::m_AcctNotaryID
                             // the PURPORTED Notary ID
            pItem->SetNymID(NYM_ID);

            auto strTemp = String::Factory();

            strTemp = String::Factory(xml->getAttributeValue("numberOfOrigin"));
            if (strTemp->Exists()) pItem->SetNumberOfOrigin(strTemp->ToLong());

            strTemp = String::Factory(xml->getAttributeValue("originType"));
            if (strTemp->Exists())
                pItem->SetOriginType(GetOriginTypeFromString(strTemp));

            strTemp = String::Factory(xml->getAttributeValue("transactionNum"));
            if (strTemp->Exists()) pItem->SetTransactionNum(strTemp->ToLong());

            strTemp = String::Factory(xml->getAttributeValue("inReferenceTo"));
            if (strTemp->Exists()) pItem->SetReferenceToNum(strTemp->ToLong());

            strTemp = String::Factory(xml->getAttributeValue(
                "closingTransactionNum"));  // only used in the inbox report for
                                            // balance agreement.
            if (strTemp->Exists()) pItem->SetClosingNum(strTemp->ToLong());

            AddItem(pItem);  // <======= adding to list.

            LogDebug(OT_METHOD)(__FUNCTION__)(
                ": Loaded transactionReport Item, transaction num ")(
                pItem->GetTransactionNum())(", In Reference To: ")(
                pItem->GetReferenceToNum())(", type: ")(strType)
                .Flush();
            //                         "fromAccountID:\n%s\n NymID:\n%s\n
            // toAccountID:\n%s\n notaryID:\n%s\n----------\n",
            //                         strAcctFromID.Get(), strNymID.Get(),
            // strAcctToID.Get(), strNotaryID.Get()
        } else {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Outbox hash in item wrong type (expected "
                "balanceStatement or atBalanceStatement.")
                .Flush();
        }

        return 1;
    }

    return 0;
}

// Used in balance agreement, part of the inbox report.
std::int64_t Item::GetClosingNum() const { return m_lClosingTransactionNo; }

void Item::SetClosingNum(std::int64_t lClosingNum)
{
    m_lClosingTransactionNo = lClosingNum;
}

void Item::GetStringFromType(itemType theType, String& strType)
{
    switch (theType) {
        case itemType::transfer:
            strType.Set("transfer");
            break;
        case itemType::acceptTransaction:
            strType.Set("acceptTransaction");
            break;
        case itemType::acceptMessage:
            strType.Set("acceptMessage");
            break;
        case itemType::acceptNotice:
            strType.Set("acceptNotice");
            break;
        case itemType::acceptPending:
            strType.Set("acceptPending");
            break;
        case itemType::rejectPending:
            strType.Set("rejectPending");
            break;
        case itemType::acceptCronReceipt:
            strType.Set("acceptCronReceipt");
            break;
        case itemType::disputeCronReceipt:
            strType.Set("disputeCronReceipt");
            break;
        case itemType::acceptItemReceipt:
            strType.Set("acceptItemReceipt");
            break;
        case itemType::disputeItemReceipt:
            strType.Set("disputeItemReceipt");
            break;
        case itemType::acceptFinalReceipt:
            strType.Set("acceptFinalReceipt");
            break;
        case itemType::acceptBasketReceipt:
            strType.Set("acceptBasketReceipt");
            break;
        case itemType::disputeFinalReceipt:
            strType.Set("disputeFinalReceipt");
            break;
        case itemType::disputeBasketReceipt:
            strType.Set("disputeBasketReceipt");
            break;
        case itemType::serverfee:
            strType.Set("serverfee");
            break;
        case itemType::issuerfee:
            strType.Set("issuerfee");
            break;
        case itemType::withdrawal:
            strType.Set("withdrawal");
            break;
        case itemType::deposit:
            strType.Set("deposit");
            break;
        case itemType::withdrawVoucher:
            strType.Set("withdrawVoucher");
            break;
        case itemType::depositCheque:
            strType.Set("depositCheque");
            break;
        case itemType::payDividend:
            strType.Set("payDividend");
            break;
        case itemType::marketOffer:
            strType.Set("marketOffer");
            break;
        case itemType::paymentPlan:
            strType.Set("paymentPlan");
            break;
        case itemType::smartContract:
            strType.Set("smartContract");
            break;
        case itemType::balanceStatement:
            strType.Set("balanceStatement");
            break;
        case itemType::transactionStatement:
            strType.Set("transactionStatement");
            break;

        case itemType::cancelCronItem:
            strType.Set("cancelCronItem");
            break;
        case itemType::exchangeBasket:
            strType.Set("exchangeBasket");
            break;

        case itemType::atCancelCronItem:
            strType.Set("atCancelCronItem");
            break;
        case itemType::atExchangeBasket:
            strType.Set("atExchangeBasket");
            break;

        case itemType::chequeReceipt:  // used for inbox statements in balance
                                       // agreement.
            strType.Set("chequeReceipt");
            break;
        case itemType::voucherReceipt:  // used for inbox statements in balance
                                        // agreement.
            strType.Set("voucherReceipt");
            break;
        case itemType::marketReceipt:  // used as market receipt, and also for
                                       // inbox
            // statement containing market receipt will use
            // this as well.
            strType.Set("marketReceipt");
            break;
        case itemType::paymentReceipt:  // used as payment receipt, also used in
                                        // inbox
                                        // statement as payment receipt.
            strType.Set("paymentReceipt");
            break;
        case itemType::transferReceipt:  // used in inbox statement as transfer
                                         // receipt.
            strType.Set("transferReceipt");
            break;

        case itemType::finalReceipt:  // used for final receipt. Also used in
                                      // inbox
            // statement as final receipt. (For expiring or
            // cancelled Cron Item.)
            strType.Set("finalReceipt");
            break;
        case itemType::basketReceipt:  // used in inbox statement as basket
                                       // receipt. (For exchange.)
            strType.Set("basketReceipt");
            break;

        case itemType::notice:  // used in Nymbox statement as notification from
                                // server.
            strType.Set("notice");
            break;
        case itemType::replyNotice:  // some server replies (to your request)
                                     // have a
            // copy dropped into your nymbox, to make sure you
            // received it.
            strType.Set("replyNotice");
            break;
        case itemType::successNotice:  // used in Nymbox statement as
                                       // notification from server of successful
                                       // sign-out of a trans#.
            strType.Set("successNotice");
            break;

        case itemType::atTransfer:
            strType.Set("atTransfer");
            break;
        case itemType::atAcceptTransaction:
            strType.Set("atAcceptTransaction");
            break;
        case itemType::atAcceptMessage:
            strType.Set("atAcceptMessage");
            break;
        case itemType::atAcceptNotice:
            strType.Set("atAcceptNotice");
            break;
        case itemType::atAcceptPending:
            strType.Set("atAcceptPending");
            break;
        case itemType::atRejectPending:
            strType.Set("atRejectPending");
            break;
        case itemType::atAcceptCronReceipt:
            strType.Set("atAcceptCronReceipt");
            break;
        case itemType::atDisputeCronReceipt:
            strType.Set("atDisputeCronReceipt");
            break;
        case itemType::atAcceptItemReceipt:
            strType.Set("atAcceptItemReceipt");
            break;
        case itemType::atDisputeItemReceipt:
            strType.Set("atDisputeItemReceipt");
            break;

        case itemType::atAcceptFinalReceipt:
            strType.Set("atAcceptFinalReceipt");
            break;
        case itemType::atAcceptBasketReceipt:
            strType.Set("atAcceptBasketReceipt");
            break;
        case itemType::atDisputeFinalReceipt:
            strType.Set("atDisputeFinalReceipt");
            break;
        case itemType::atDisputeBasketReceipt:
            strType.Set("atDisputeBasketReceipt");
            break;

        case itemType::atServerfee:
            strType.Set("atServerfee");
            break;
        case itemType::atIssuerfee:
            strType.Set("atIssuerfee");
            break;
        case itemType::atWithdrawal:
            strType.Set("atWithdrawal");
            break;
        case itemType::atDeposit:
            strType.Set("atDeposit");
            break;
        case itemType::atWithdrawVoucher:
            strType.Set("atWithdrawVoucher");
            break;
        case itemType::atDepositCheque:
            strType.Set("atDepositCheque");
            break;
        case itemType::atPayDividend:
            strType.Set("atPayDividend");
            break;
        case itemType::atMarketOffer:
            strType.Set("atMarketOffer");
            break;
        case itemType::atPaymentPlan:
            strType.Set("atPaymentPlan");
            break;
        case itemType::atSmartContract:
            strType.Set("atSmartContract");
            break;
        case itemType::atBalanceStatement:
            strType.Set("atBalanceStatement");
            break;
        case itemType::atTransactionStatement:
            strType.Set("atTransactionStatement");
            break;

        default:
            strType.Set("error-unknown");
            break;
    }
}

void Item::UpdateContents()  // Before transmission or serialization, this is
                             // where the ledger saves its contents
{
    auto strFromAcctID = String::Factory(GetPurportedAccountID()),
         strToAcctID = String::Factory(GetDestinationAcctID()),
         strNotaryID = String::Factory(GetPurportedNotaryID()),
         strType = String::Factory(), strStatus = String::Factory(),
         strNymID = String::Factory(GetNymID());

    GetStringFromType(m_Type, strType);

    switch (m_Status) {
        case request:
            strStatus->Set("request");
            break;
        case acknowledgement:
            strStatus->Set("acknowledgement");
            break;
        case rejection:
            strStatus->Set("rejection");
            break;
        default:
            strStatus->Set("error-unknown");
            break;
    }

    // I release this because I'm about to repopulate it.
    m_xmlUnsigned->Release();

    Tag tag("item");

    tag.add_attribute("type", strType->Get());
    tag.add_attribute("status", strStatus->Get());
    tag.add_attribute(
        "numberOfOrigin",  // GetRaw so it doesn't calculate.
        formatLong(GetRawNumberOfOrigin()));

    if (GetOriginType() != originType::not_applicable) {
        auto strOriginType = String::Factory(GetOriginTypeString());
        tag.add_attribute("originType", strOriginType->Get());
    }

    tag.add_attribute("transactionNum", formatLong(GetTransactionNum()));
    tag.add_attribute("notaryID", strNotaryID->Get());
    tag.add_attribute("nymID", strNymID->Get());
    tag.add_attribute("fromAccountID", strFromAcctID->Get());
    tag.add_attribute("toAccountID", strToAcctID->Get());
    tag.add_attribute("inReferenceTo", formatLong(GetReferenceToNum()));
    tag.add_attribute("amount", formatLong(m_lAmount));

    // Only used in server reply item:
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
    if (m_lNewOutboxTransNum > 0)
        tag.add_attribute(
            "outboxNewTransNum", formatLong(m_lNewOutboxTransNum));
    else {
        // IF this item is "acceptTransaction" then this
        // will serialize the list of transaction numbers
        // being accepted. (They now support multiple
        // numbers.)
        if ((itemType::acceptTransaction == m_Type) &&
            (m_Numlist.Count() > 0)) {
            // m_Numlist.Count is always 0, except for
            // itemType::acceptTransaction.
            auto strListOfBlanks = String::Factory();

            if (true == m_Numlist.Output(strListOfBlanks))
                tag.add_attribute("totalListOfNumbers", strListOfBlanks->Get());
        }
    }

    if (m_ascNote->GetLength() > 2) { tag.add_tag("note", m_ascNote->Get()); }

    if (m_ascInReferenceTo->GetLength() > 2) {
        tag.add_tag("inReferenceTo", m_ascInReferenceTo->Get());
    }

    if (m_ascAttachment->GetLength() > 2) {
        tag.add_tag("attachment", m_ascAttachment->Get());
    }

    if ((itemType::balanceStatement == m_Type) ||
        (itemType::atBalanceStatement == m_Type)) {

        // loop through the sub-items (only used for balance agreement.)
        //
        for (auto& it : m_listItems) {
            const auto pItem = it;
            OT_ASSERT(false != bool(pItem));

            auto acctID = String::Factory(pItem->GetPurportedAccountID()),
                 notaryID = String::Factory(pItem->GetPurportedNotaryID()),
                 nymID = String::Factory();
            auto receiptType = String::Factory();
            GetStringFromType(pItem->GetType(), receiptType);

            TagPtr tagReport(new Tag("transactionReport"));

            tagReport->add_attribute(
                "type",
                receiptType->Exists() ? receiptType->Get() : "error_state");
            tagReport->add_attribute(
                "adjustment", formatLong(pItem->GetAmount()));
            tagReport->add_attribute("accountID", acctID->Get());
            tagReport->add_attribute("nymID", nymID->Get());
            tagReport->add_attribute("notaryID", notaryID->Get());
            tagReport->add_attribute(
                "numberOfOrigin", formatLong(pItem->GetRawNumberOfOrigin()));

            if (pItem->GetOriginType() != originType::not_applicable) {
                auto strOriginType =
                    String::Factory(pItem->GetOriginTypeString());
                tagReport->add_attribute("originType", strOriginType->Get());
            }

            tagReport->add_attribute(
                "transactionNum", formatLong(pItem->GetTransactionNum()));
            tagReport->add_attribute(
                "closingTransactionNum", formatLong(pItem->GetClosingNum()));
            tagReport->add_attribute(
                "inReferenceTo", formatLong(pItem->GetReferenceToNum()));

            tag.add_tag(tagReport);
        }
    }

    std::string str_result;
    tag.output(str_result);

    m_xmlUnsigned->Concatenate("%s", str_result.c_str());
}

Item::~Item() { Release_Item(); }
}  // namespace opentxs
