// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/otx/common/recurring/OTAgreement.hpp"  // IWYU pragma: associated

#include <chrono>
#include <compare>
#include <cstdint>
#include <cstring>
#include <memory>

#include "internal/api/session/FactoryAPI.hpp"
#include "internal/api/session/Wallet.hpp"
#include "internal/core/String.hpp"
#include "internal/otx/Types.hpp"
#include "internal/otx/common/Account.hpp"
#include "internal/otx/common/Item.hpp"
#include "internal/otx/common/Ledger.hpp"
#include "internal/otx/common/NumList.hpp"
#include "internal/otx/common/OTTransaction.hpp"
#include "internal/otx/common/XML.hpp"
#include "internal/otx/common/cron/OTCron.hpp"
#include "internal/otx/common/cron/OTCronItem.hpp"
#include "internal/otx/common/util/Common.hpp"
#include "internal/otx/consensus/Client.hpp"
#include "internal/otx/consensus/Consensus.hpp"
#include "internal/otx/consensus/ManagedNumber.hpp"
#include "internal/otx/consensus/Server.hpp"
#include "internal/util/Editor.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/identity/Nym.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

// OTAgreement is derived from OTCronItem.  It handles re-occuring billing.

namespace opentxs
{
OTAgreement::OTAgreement(const api::Session& api)
    : ot_super(api)
    , recipient_account_id_()
    , recipient_nym_id_()
    , consideration_(String::Factory())
    , merchant_signed_copy_(String::Factory())
    , recipient_closing_numbers_()
{
    InitAgreement();
}

OTAgreement::OTAgreement(
    const api::Session& api,
    const identifier::Notary& NOTARY_ID,
    const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID)
    : ot_super(api, NOTARY_ID, INSTRUMENT_DEFINITION_ID)
    , recipient_account_id_()
    , recipient_nym_id_()
    , consideration_(String::Factory())
    , merchant_signed_copy_(String::Factory())
    , recipient_closing_numbers_()
{
    InitAgreement();
}

OTAgreement::OTAgreement(
    const api::Session& api,
    const identifier::Notary& NOTARY_ID,
    const identifier::UnitDefinition& INSTRUMENT_DEFINITION_ID,
    const identifier::Account& SENDER_ACCT_ID,
    const identifier::Nym& SENDER_NYM_ID,
    const identifier::Account& RECIPIENT_ACCT_ID,
    const identifier::Nym& RECIPIENT_NYM_ID)
    : ot_super(
          api,
          NOTARY_ID,
          INSTRUMENT_DEFINITION_ID,
          SENDER_ACCT_ID,
          SENDER_NYM_ID)
    , recipient_account_id_()
    , recipient_nym_id_()
    , consideration_(String::Factory())
    , merchant_signed_copy_(String::Factory())
    , recipient_closing_numbers_()
{
    InitAgreement();
    SetRecipientAcctID(RECIPIENT_ACCT_ID);
    SetRecipientNymID(RECIPIENT_NYM_ID);
}

void OTAgreement::setCustomerNymId(const identifier::Nym& NYM_ID)
{
    ot_super::SetSenderNymID(NYM_ID);
}

auto OTAgreement::SendNoticeToAllParties(
    const api::Session& api,
    bool bSuccessMsg,
    const identity::Nym& theServerNym,
    const identifier::Notary& theNotaryID,
    const TransactionNumber& lNewTransactionNumber,
    // Each party has its own opening trans #.
    const String& strReference,
    const PasswordPrompt& reason,
    OTString pstrNote,
    OTString pstrAttachment,
    identity::Nym* pActualNym) const -> bool
{
    // Success is defined as ALL parties receiving a notice
    bool bSuccess = true;

    // Sender
    if (!OTAgreement::DropServerNoticeToNymbox(
            api,
            bSuccessMsg,  // "success" notice? or "failure" notice?
            theServerNym,
            theNotaryID,
            GetSenderNymID(),
            lNewTransactionNumber,
            GetTransactionNum(),  // in reference to
            strReference,
            originType::origin_payment_plan,
            pstrNote,
            pstrAttachment,
            GetSenderNymID(),
            reason)) {
        bSuccess = false;
    }
    // Notice I don't break here -- I still allow it to try to notice ALL
    // parties, even if one fails.

    // Recipient
    if (!OTAgreement::DropServerNoticeToNymbox(
            api,
            bSuccessMsg,  // "success" notice? or "failure" notice?
            theServerNym,
            theNotaryID,
            GetRecipientNymID(),
            lNewTransactionNumber,
            GetRecipientOpeningNum(),  // in reference to
            strReference,
            originType::origin_payment_plan,
            pstrNote,
            pstrAttachment,
            GetRecipientNymID(),
            reason)) {
        bSuccess = false;
    }

    return bSuccess;
}

// static
// Used by payment plans and smart contracts. Nym receives an
// Item::acknowledgment or Item::rejection.
auto OTAgreement::DropServerNoticeToNymbox(
    const api::Session& api,
    bool bSuccessMsg,
    const identity::Nym& theServerNym,
    const identifier::Notary& NOTARY_ID,
    const identifier::Nym& NYM_ID,
    const TransactionNumber& lNewTransactionNumber,
    const TransactionNumber& lInReferenceTo,
    const String& strReference,
    originType theOriginType,
    OTString pstrNote,
    OTString pstrAttachment,
    const identifier::Nym& actualNymID,
    const PasswordPrompt& reason) -> bool
{
    auto theLedger{
        api.Factory().InternalSession().Ledger(NYM_ID, NYM_ID, NOTARY_ID)};

    OT_ASSERT(false != bool(theLedger));

    // Inbox will receive notification of something ALREADY DONE.
    bool bSuccessLoading = theLedger->LoadNymbox();

    if (true == bSuccessLoading) {
        bSuccessLoading = theLedger->VerifyAccount(theServerNym);
    } else {
        bSuccessLoading = theLedger->GenerateLedger(
            NYM_ID,
            NOTARY_ID,
            ledgerType::nymbox,
            true);  // bGenerateFile=true
    }

    if (!bSuccessLoading) {
        LogError()(OT_PRETTY_STATIC(OTAgreement))(
            "Failed loading or generating a nymbox. "
            "(FAILED WRITING RECEIPT!!).")
            .Flush();

        return false;
    }

    auto pTransaction{api.Factory().InternalSession().Transaction(
        *theLedger,
        transactionType::notice,
        theOriginType,
        lNewTransactionNumber)};

    if (false != bool(pTransaction)) {
        // The nymbox will get a receipt with the new transaction ID.
        // That receipt has an "in reference to" field containing the original
        // OTScriptable

        // Set up the transaction items (each transaction may have multiple
        // items... but not in this case.)
        auto pItem1{api.Factory().InternalSession().Item(
            *pTransaction, itemType::notice, identifier::Account{})};
        OT_ASSERT(false != bool(pItem1));  // This may be unnecessary, I'll have
                                           // to check
                                           // CreateItemFromTransaction. I'll
                                           // leave it for now.

        pItem1->SetStatus(
            bSuccessMsg ? Item::acknowledgement
                        : Item::rejection);  // ACKNOWLEDGMENT or REJECTION ?

        //
        // Here I make sure that the receipt (the nymbox notice) references the
        // transaction number that the trader originally used to issue the cron
        // item...
        // This number is used to match up offers to trades, and used to track
        // all cron items.
        // (All Cron items require a transaction from the user to add them to
        // Cron in the
        // first place.)
        //
        pTransaction->SetReferenceToNum(lInReferenceTo);

        // The reference on the transaction probably contains a the original
        // cron item or entity contract.
        // Versus the updated item (which, if it exists, is stored on the pItem1
        // just below.)
        //
        pTransaction->SetReferenceString(strReference);

        // The notice ITEM's NOTE probably contains the UPDATED SCRIPTABLE
        // (usually a CRON ITEM. But maybe soon: Entity.)
        if (pstrNote->Exists()) {
            pItem1->SetNote(pstrNote);  // in markets, this is updated trade.
        }

        // Nothing is special stored here so far for transactionType::notice,
        // but the option is always there.
        //
        if (pstrAttachment->Exists()) { pItem1->SetAttachment(pstrAttachment); }

        // sign the item
        //
        pItem1->SignContract(theServerNym, reason);
        pItem1->SaveContract();

        // the Transaction "owns" the item now and will handle cleaning it up.
        std::shared_ptr<Item> item{pItem1.release()};
        pTransaction->AddItem(item);

        pTransaction->SignContract(theServerNym, reason);
        pTransaction->SaveContract();

        // Here the transaction we just created is actually added to the ledger.
        std::shared_ptr<OTTransaction> transaction{pTransaction.release()};
        theLedger->AddTransaction(transaction);

        // Release any signatures that were there before (They won't
        // verify anymore anyway, since the content has changed.)
        theLedger->ReleaseSignatures();

        // Sign and save.
        theLedger->SignContract(theServerNym, reason);
        theLedger->SaveContract();

        // TODO: Better rollback capabilities in case of failures here:

        auto theNymboxHash = identifier::Generic{};

        // Save nymbox to storage. (File, DB, wherever it goes.)
        (theLedger->SaveNymbox(theNymboxHash));

        // Corresponds to the AddTransaction() call just above. These
        // are stored in a separate file now.
        //
        transaction->SaveBoxReceipt(*theLedger);

        auto context = api.Wallet().Internal().mutable_ClientContext(

            actualNymID, reason);
        context.get().SetLocalNymboxHash(theNymboxHash);

        // Really this true should be predicated on ALL the above functions
        // returning true. Right?

        return true;
    } else {
        LogError()(OT_PRETTY_STATIC(OTAgreement))(
            "Failed trying to create Nymbox.")
            .Flush();
    }

    return false;  // unreachable.
}

// Overrides from OTTrackable.
auto OTAgreement::HasTransactionNum(const std::int64_t& lInput) const -> bool
{
    if (lInput == GetTransactionNum()) { return true; }

    const std::size_t nSizeClosing = closing_numbers_.size();

    for (auto nIndex = 0_uz; nIndex < nSizeClosing; ++nIndex) {
        if (lInput == closing_numbers_.at(nIndex)) { return true; }
    }

    const std::size_t nSizeRecipient = recipient_closing_numbers_.size();

    for (auto nIndex = 0_uz; nIndex < nSizeRecipient; ++nIndex) {
        if (lInput == recipient_closing_numbers_.at(nIndex)) { return true; }
    }

    return false;
}

void OTAgreement::GetAllTransactionNumbers(NumList& numlistOutput) const
{

    if (GetTransactionNum() > 0) { numlistOutput.Add(GetTransactionNum()); }

    const std::size_t nSizeClosing = closing_numbers_.size();

    for (auto nIndex = 0_uz; nIndex < nSizeClosing; ++nIndex) {
        const std::int64_t lTemp = closing_numbers_.at(nIndex);
        if (lTemp > 0) { numlistOutput.Add(lTemp); }
    }

    const std::size_t nSizeRecipient = recipient_closing_numbers_.size();

    for (auto nIndex = 0_uz; nIndex < nSizeRecipient; ++nIndex) {
        const std::int64_t lTemp = recipient_closing_numbers_.at(nIndex);
        if (lTemp > 0) { numlistOutput.Add(lTemp); }
    }
}

// Used to be I could just call pAgreement->VerifySignature(theNym), which is
// what I still call here, inside this function. But that's a special case -- an
// override from the OTScriptable / OTSmartContract version, which verifies
// parties and agents, etc.
auto OTAgreement::VerifyNymAsAgent(
    const identity::Nym& theNym,
    const identity::Nym&) const -> bool
{
    return VerifySignature(theNym);
}

// This is an override. See note above.
//
auto OTAgreement::VerifyNymAsAgentForAccount(
    const identity::Nym& theNym,
    const Account& theAccount) const -> bool
{
    return theAccount.VerifyOwner(theNym);
}

// This is called by OTCronItem::HookRemovalFromCron
// (After calling this method, HookRemovalFromCron then calls
// onRemovalFromCron.)
void OTAgreement::onFinalReceipt(
    OTCronItem& theOrigCronItem,
    const std::int64_t& lNewTransactionNumber,
    Nym_p theOriginator,
    Nym_p pRemover,
    const PasswordPrompt& reason)
{
    OTCron* pCron = GetCron();

    OT_ASSERT(nullptr != pCron);

    auto pServerNym = pCron->GetServerNym();

    OT_ASSERT(nullptr != pServerNym);

    // The finalReceipt Item's ATTACHMENT contains the UPDATED Cron Item.
    // (With the SERVER's signature on it!)
    auto strUpdatedCronItem = String::Factory(*this);
    OTString pstrAttachment = strUpdatedCronItem;
    const auto strOrigCronItem = String::Factory(theOrigCronItem);
    const identifier::Nym NYM_ID = GetRecipientNymID();

    // First, we are closing the transaction number ITSELF, of this cron item,
    // as an active issued number on the originating nym. (Changing it to
    // CLOSED.)
    //
    // Second, we're verifying the CLOSING number, and using it as the closing
    // number on the FINAL RECEIPT (with that receipt being "InReferenceTo"
    // GetTransactionNum())
    const TransactionNumber lRecipientOpeningNumber = GetRecipientOpeningNum();
    const TransactionNumber lRecipientClosingNumber = GetRecipientClosingNum();
    const TransactionNumber lSenderOpeningNumber =
        theOrigCronItem.GetTransactionNum();
    const TransactionNumber lSenderClosingNumber =
        (theOrigCronItem.GetCountClosingNumbers() > 0)
            ? theOrigCronItem.GetClosingTransactionNoAt(0)
            : 0;  // index 0 is closing number for sender, since
                  // GetTransactionNum() is his opening #.
    const auto strNotaryID = String::Factory(GetNotaryID(), api_.Crypto());
    auto oContext = api_.Wallet().Internal().mutable_ClientContext(

        theOriginator->ID(), reason);

    if ((lSenderOpeningNumber > 0) &&
        oContext.get().VerifyIssuedNumber(lSenderOpeningNumber)) {

        // The Nym (server side) stores a list of all opening and closing cron
        // #s. So when the number is released from the Nym, we also take it off
        // that list.
        oContext.get().CloseCronItem(lSenderOpeningNumber);

        // the RemoveIssued call means the original transaction# (to find this
        // cron item on cron) is now CLOSED. But the Transaction itself is still
        // OPEN. How? Because the CLOSING number is still signed out. The
        // closing number is also USED, since the NotarizePaymentPlan or
        // NotarizeMarketOffer call, but it remains ISSUED, until the final
        // receipt itself is accepted during a process inbox.
        oContext.get().ConsumeIssued(lSenderOpeningNumber);

        if (!DropFinalReceiptToNymbox(
                GetSenderNymID(),
                lNewTransactionNumber,
                strOrigCronItem,
                GetOriginType(),
                reason,
                String::Factory(),
                pstrAttachment)) {
            LogError()(OT_PRETTY_CLASS())(
                "Failure dropping sender final receipt into nymbox.")
                .Flush();
        }
    } else {
        LogError()(OT_PRETTY_CLASS())(
            "Failure verifying sender's opening number.")
            .Flush();
    }

    if ((lSenderClosingNumber > 0) &&
        oContext.get().VerifyIssuedNumber(lSenderClosingNumber)) {
        // In this case, I'm passing nullptr for pstrNote, since there is no
        // note. (Additional information would normally be stored in the note.)
        if (!DropFinalReceiptToInbox(
                GetSenderNymID(),
                GetSenderAcctID(),
                lNewTransactionNumber,
                lSenderClosingNumber,  // The closing transaction number to put
                                       // on the receipt.
                strOrigCronItem,
                GetOriginType(),
                reason,
                String::Factory(),
                pstrAttachment))  // pActualAcct=nullptr by default. (This
                                  // call will load it up and update its
                                  // inbox hash.)
        {
            LogError()(OT_PRETTY_CLASS())(
                "Failure dropping receipt into sender's inbox.")
                .Flush();
        }
        // This part below doesn't happen until theOriginator ACCEPTS the final
        // receipt (when processing his inbox.)
        //
        //      theOriginator.RemoveIssuedNum(strNotaryID, lSenderClosingNumber,
        // true); //bSave=false
    } else {
        LogError()(OT_PRETTY_CLASS())(
            "Failed verifying "
            "lSenderClosingNumber=theOrigCronItem. "
            "GetClosingTransactionNoAt(0)>0 && "
            "theOriginator.VerifyTransactionNum(lSenderClosingNumber).")
            .Flush();
    }

    auto rContext = api_.Wallet().Internal().mutable_ClientContext(

        GetRecipientNymID(), reason);

    if ((lRecipientOpeningNumber > 0) &&
        rContext.get().VerifyIssuedNumber(lRecipientOpeningNumber)) {
        // The Nym (server side) stores a list of all opening and closing cron
        // #s. So when the number is released from the Nym, we also take it off
        // thatlist.
        rContext.get().CloseCronItem(lRecipientOpeningNumber);

        // the RemoveIssued call means the original transaction# (to find this
        // cron item on cron) is now CLOSED. But the Transaction itself is still
        // OPEN. How? Because the CLOSING number is still signed out. The
        // closing number is also USED, since the NotarizePaymentPlan or
        // NotarizeMarketOffer call, but it remains ISSUED, until the final
        // receipt itself is accepted during a process inbox.
        rContext.get().ConsumeIssued(lRecipientOpeningNumber);

        // NymboxHash is updated here in recipient.
        const bool dropped = DropFinalReceiptToNymbox(
            GetRecipientNymID(),
            lNewTransactionNumber,
            strOrigCronItem,
            GetOriginType(),
            reason,
            String::Factory(),
            pstrAttachment);

        if (!dropped) {
            LogError()(OT_PRETTY_CLASS())(
                "Failure dropping recipient final receipt into nymbox.")
                .Flush();
        }
    } else {
        LogError()(OT_PRETTY_CLASS())(
            "Failed verifying "
            "lRecipientClosingNumber="
            "GetRecipientClosingTransactionNoAt(1)>0 && "
            "pRecipient->VerifyTransactionNum(lRecipientClosingNumber) && "
            "VerifyIssuedNum(lRecipientOpeningNumber).")
            .Flush();
    }

    if ((lRecipientClosingNumber > 0) &&
        rContext.get().VerifyIssuedNumber(lRecipientClosingNumber)) {
        if (!DropFinalReceiptToInbox(
                GetRecipientNymID(),
                GetRecipientAcctID(),
                lNewTransactionNumber,
                lRecipientClosingNumber,  // The closing transaction number to
                                          // put on the receipt.
                strOrigCronItem,
                GetOriginType(),
                reason,
                String::Factory(),
                pstrAttachment)) {
            LogError()(OT_PRETTY_CLASS())(
                "Failure dropping receipt into recipient's inbox.")
                .Flush();
        }
    } else {
        LogError()(OT_PRETTY_CLASS())(
            "Failed verifying "
            "lRecipientClosingNumber="
            "GetRecipientClosingTransactionNoAt(1)>0 && "
            "pRecipient->VerifyTransactionNum(lRecipientClosingNumber) && "
            "VerifyIssuedNum(lRecipientOpeningNumber).")
            .Flush();
    }

    // QUESTION: Won't there be Cron Items that have no asset account at all?
    // In which case, there'd be no need to drop a final receipt, but I don't
    // think that's the case, since you have to use a transaction number to get
    // onto cron in the first place.
}

auto OTAgreement::IsValidOpeningNumber(const std::int64_t& lOpeningNum) const
    -> bool
{
    if (GetRecipientOpeningNum() == lOpeningNum) { return true; }

    return ot_super::IsValidOpeningNumber(lOpeningNum);
}

void OTAgreement::onRemovalFromCron(const PasswordPrompt& reason)
{
    // Not much needed here.
    // Actually: Todo:  (unless it goes in payment plan code) need to set
    // receipts
    // in inboxes, and close out the closing transaction numbers.
    //
}

// You usually wouldn't want to use this, since if the transaction failed, the
// opening number
// is already burned and gone. But there might be cases where it's not, and you
// want to retrieve it.
// So I added this function.
//
void OTAgreement::HarvestOpeningNumber(otx::context::Server& context)
{
    // Since we overrode the parent, we give it a chance to harvest also.
    // IF theNym is the original sender, the opening number will be harvested
    // inside this call.
    ot_super::HarvestOpeningNumber(context);

    // The Nym is the original recipient. (If Compares true). IN CASES where
    // GetTransactionNum() isn't already burned, we can harvest
    // it here.
    if (context.Nym()->CompareID(GetRecipientNymID())) {
        // This function will only "add it back" if it was really there in the
        // first place. (Verifies it is on issued list first, before adding to
        // available list.)
        context.RecoverAvailableNumber(GetRecipientOpeningNum());
    }

    // NOTE: if the message failed (transaction never actually ran) then the
    // sender AND recipient can both reclaim their opening numbers. But if the
    // message SUCCEEDED and the transaction FAILED, then only the recipient can
    // claim his opening number -- the sender's is already burned. So then,
    // what if you mistakenly call this function and pass the sender, when that
    // number is already burned? There's nothing this function can do, because
    // we have no way of telling, from inside here, whether the message
    // succeeded or not, and whether the transaction succeeded or not.
    // Therefore, ==> we MUST rely on the CALLER to know this, and to avoid
    // calling this function in the first place, if he's sitting on a sender
    // with a failed transaction.
}

// Used for adding transaction numbers back to a Nym, after deciding not to use
// this agreement or failing in trying to use it. Client side.
void OTAgreement::HarvestClosingNumbers(otx::context::Server& context)
{
    // Since we overrode the parent, we give it a chance to harvest also.
    // If theNym is the sender, then his closing numbers will be harvested
    // inside here. But what if the transaction was a success? The numbers
    // will still be harvested, since they are still on the sender's issued
    // list, but they should not have been harvested, regardless, since the
    // transaction was a success and the server therefore has them marked as
    // "used." So clearly you cannot just blindly call this function unless
    // you know beforehand whether the message and transaction were a success.
    ot_super::HarvestClosingNumbers(context);

    // The Nym is the original recipient. (If Compares true). FYI, if Nym is the
    // original sender, then the above call will handle him.
    //
    // GetTransactionNum() is burned, but we can harvest the closing numbers
    // from the "Closing" list, which is only for the sender's numbers.
    // Subclasses will have to override this function for recipients, etc.
    if (context.Nym()->CompareID(GetRecipientNymID())) {
        // This function will only "add it back" if it was really there in the
        // first place. (Verifies it is on issued list first, before adding to
        // available list.)
        context.RecoverAvailableNumber(GetRecipientClosingNum());
    }
}

auto OTAgreement::GetOpeningNumber(const identifier::Nym& theNymID) const
    -> std::int64_t
{
    const auto& theRecipientNymID = GetRecipientNymID();

    if (theNymID == theRecipientNymID) { return GetRecipientOpeningNum(); }

    return ot_super::GetOpeningNumber(theNymID);
}

auto OTAgreement::GetClosingNumber(const identifier::Account& theAcctID) const
    -> std::int64_t
{
    const auto& theRecipientAcctID = GetRecipientAcctID();

    if (theAcctID == theRecipientAcctID) { return GetRecipientClosingNum(); }
    // else...
    return ot_super::GetClosingNumber(theAcctID);
}

auto OTAgreement::GetRecipientOpeningNum() const -> TransactionNumber
{
    return (GetRecipientCountClosingNumbers() > 0)
               ? GetRecipientClosingTransactionNoAt(0)
               : 0;  // todo stop hardcoding.
}

auto OTAgreement::GetRecipientClosingNum() const -> TransactionNumber
{
    return (GetRecipientCountClosingNumbers() > 1)
               ? GetRecipientClosingTransactionNoAt(1)
               : 0;  // todo stop hardcoding.
}

// These are for finalReceipt
// The Cron Item stores a list of these closing transaction numbers,
// used for closing a transaction.
//

auto OTAgreement::GetRecipientClosingTransactionNoAt(std::uint32_t nIndex) const
    -> std::int64_t
{
    OT_ASSERT_MSG(
        (nIndex < recipient_closing_numbers_.size()),
        "OTAgreement::GetClosingTransactionNoAt: index out of bounds.");

    return recipient_closing_numbers_.at(nIndex);
}

auto OTAgreement::GetRecipientCountClosingNumbers() const -> std::int32_t
{
    return static_cast<std::int32_t>(recipient_closing_numbers_.size());
}

void OTAgreement::AddRecipientClosingTransactionNo(
    const std::int64_t& closingNumber)
{
    recipient_closing_numbers_.push_back(closingNumber);
}

// OTCron calls this regularly, which is my chance to expire, etc.
// Child classes will override this, AND call it (to verify valid date range.)
auto OTAgreement::ProcessCron(const PasswordPrompt& reason) -> bool
{
    // END DATE --------------------------------
    // First call the parent's version (which this overrides) so it has
    // a chance to check its stuff. Currently it checks IsExpired().
    if (!ot_super::ProcessCron(reason)) {
        return false;  // It's expired or flagged--removed it from Cron.
    }

    // START DATE --------------------------------
    // Okay, so it's NOT expired. But might not have reached START DATE yet...
    // (If not expired, yet current date is not verified, that means it hasn't
    // ENTERED the date range YET.)
    //
    if (!VerifyCurrentDate()) {
        return true;  // The Trade is not yet valid, so we return. BUT, we
    }
    // return
    //  true, so it will stay on Cron until it BECOMES valid.

    // Process my Agreement-specific stuff
    // below.--------------------------------

    return true;
}

/// See if theNym has rights to remove this item from Cron.
///
auto OTAgreement::CanRemoveItemFromCron(const otx::context::Client& context)
    -> bool
{
    // You don't just go willy-nilly and remove a cron item from a market unless
    // you check first and make sure the Nym who requested it actually has said
    // number (or a related closing number) signed out to him on his last
    // receipt...
    if (true == ot_super::CanRemoveItemFromCron(context)) { return true; }

    const auto strNotaryID = String::Factory(GetNotaryID(), api_.Crypto());

    // Usually the Nym is the originator. (Meaning GetTransactionNum() on this
    // agreement is still verifiable as an issued number on theNum, and belongs
    // to him.) In that case, the above call will discover this, and return
    // true. In other cases, theNym has the right to Remove the item even though
    // theNym didn't originate it. (Like if he is the recipient -- not the
    // sender -- in a payment plan.) We check such things HERE in this function
    // (see below.)
    if (!context.RemoteNym().CompareID(GetRecipientNymID())) {
        LogConsole()(OT_PRETTY_CLASS())("Context Remote Nym ID: ")(
            context.RemoteNym().ID(), api_.Crypto())(". Sender Nym ID: ")(
            GetSenderNymID(), api_.Crypto())(". Recipient Nym ID: ")(
            GetRecipientNymID(), api_.Crypto())(
            ". Weird: Nym tried to remove agreement (payment plan), even "
            "though he apparently wasn't the sender OR recipient.")
            .Flush();

        return false;
    } else if (GetRecipientCountClosingNumbers() < 2) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Weird: Recipient tried to remove agreement (or payment plan); "
            "expected 2 closing numbers to be available--that weren't. "
            "(Found ")(GetRecipientCountClosingNumbers())(").")
            .Flush();

        return false;
    }

    if (!context.VerifyIssuedNumber(GetRecipientClosingNum())) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Recipient Closing number didn't verify (for removal from cron).")
            .Flush();

        return false;
    }

    // By this point, we KNOW theNym is the sender, and we KNOW there are the
    // proper number of transaction numbers available to close. We also know
    // that this cron item really was on the cron object, since that is where it
    // was looked up from, when this function got called! So I'm pretty sure, at
    // this point, to authorize removal, as long as the transaction num is still
    // issued to theNym (this check here.)

    return context.VerifyIssuedNumber(GetRecipientOpeningNum());

    // Normally this will be all we need to check. The originator will have the
    // transaction number signed-out to him still, if he is trying to close it.
    // BUT--in some cases, someone who is NOT the originator can cancel. Like in
    // a payment plan, the sender is also the depositor, who would normally be
    // the person cancelling the plan. But technically, the RECIPIENT should
    // also have the ability to cancel that payment plan.  BUT: the transaction
    // number isn't signed out to the RECIPIENT... In THAT case, the below
    // VerifyIssuedNum() won't work! In those cases, expect that the special
    // code will be in the subclasses override of this function.
    // (OTPaymentPlan::CanRemoveItem() etc)

    // P.S. If you override this function, MAKE SURE to call the parent
    // (OTCronItem::CanRemoveItem) first, for the VerifyIssuedNum call above.
    // Only if that fails, do you need to dig deeper...
}

auto OTAgreement::CompareAgreement(const OTAgreement& rhs) const -> bool
{
    // Compare OTAgreement specific info here.
    if ((consideration_->Compare(rhs.consideration_)) &&
        (GetRecipientAcctID() == rhs.GetRecipientAcctID()) &&
        (GetRecipientNymID() == rhs.GetRecipientNymID()) &&
        //        (   closing_numbers_ == rhs.closing_numbers_ ) && //
        // The merchant wouldn't know the customer's trans#s.
        // (Thus wouldn't expect them to be set in BOTH versions...)
        (recipient_closing_numbers_ == rhs.recipient_closing_numbers_) &&
        //      (   GetTransactionNum()  == rhs.GetTransactionNum()   ) && //
        // (commented out for same reason as above.)
        //      (   GetSenderAcctID()    == rhs.GetSenderAcctID()     ) && //
        // Same here -- we should let the merchant leave these blank,
        //      (   GetSenderNymID()    == rhs.GetSenderNymID()     ) && //
        // and then allow the customer to add them in his version,
        (GetInstrumentDefinitionID() ==
         rhs.GetInstrumentDefinitionID()) &&  // (and this Compare
                                              // function still still
                                              // verify it.)
        (GetNotaryID() == rhs.GetNotaryID()) &&
        (GetValidFrom() == rhs.GetValidFrom()) &&
        (GetValidTo() == rhs.GetValidTo())) {
        return true;
    }

    return false;
}

// THIS FUNCTION IS CALLED BY THE MERCHANT
//
// (lMerchantTransactionNumber, lMerchantClosingNumber are set internally in
// this call, from MERCHANT_NYM.)
auto OTAgreement::SetProposal(
    otx::context::Server& context,
    const Account& MERCHANT_ACCT,
    const String& strConsideration,
    const Time VALID_FROM,
    const Time VALID_TO) -> bool  // VALID_TO is a length here. (i.e. it's ADDED
                                  // to valid_from)
{
    const auto& nym = *context.Nym();
    const auto& id_MERCHANT_NYM = nym.ID();
    const auto& id_MERCHANT_ACCT = MERCHANT_ACCT.GetPurportedAccountID();

    if (GetRecipientNymID() != id_MERCHANT_NYM) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Merchant has wrong NymID (should be same "
            "as RecipientNymID).")
            .Flush();
        return false;
    } else if (GetRecipientAcctID() != id_MERCHANT_ACCT) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Merchant has wrong AcctID (should be same "
            "as RecipientAcctID).")
            .Flush();
        return false;
    } else if (!MERCHANT_ACCT.VerifyOwner(nym)) {
        LogConsole()(OT_PRETTY_CLASS())("Failure: Merchant account is not "
                                        "owned by Merchant Nym.")
            .Flush();
        return false;
    } else if (GetRecipientNymID() == GetSenderNymID()) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failure: Sender and recipient have the same "
            "Nym ID (not allowed).")
            .Flush();
        return false;
    } else if (context.AvailableNumbers() < 2) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failure. You need at least 2 transaction "
            "numbers available to do this.")
            .Flush();
        return false;
    }
    // --------------------------------------
    // Set the CREATION DATE
    //
    const auto CURRENT_TIME = Clock::now();

    // Set the Creation Date.
    SetCreationDate(CURRENT_TIME);

    // Putting this above here so I don't have to put the transaction numbers
    // back if this fails:

    // VALID_FROM
    //
    // The default "valid from" time is NOW.
    if (Time{} >= VALID_FROM) {
        SetValidFrom(CURRENT_TIME);
    } else {
        SetValidFrom(VALID_FROM);
    }

    // VALID_TO
    //
    // The default "valid to" time is 0 (which means no expiration date / cancel
    // anytime.)
    if (Time{} == VALID_TO)  // VALID_TO is 0
    {
        SetValidTo(VALID_TO);      // Keep it at zero then, so it
                                   // won't expire.
    } else if (Time{} < VALID_TO)  // VALID_TO is ABOVE zero...
    {
        SetValidTo(VALID_TO);
    } else  // VALID_TO is a NEGATIVE number... Error.
    {
        LogError()(OT_PRETTY_CLASS())("Invalid value for valid_to: ")(VALID_TO)
            .Flush();

        return false;
    }

    // Since we'll be needing 2 transaction numbers to do this, let's grab
    // 'em...
    auto strNotaryID = String::Factory(GetNotaryID(), api_.Crypto());
    const auto openingNumber = context.InternalServer().NextTransactionNumber(
        MessageType::notarizeTransaction);
    const auto closingNumber = context.InternalServer().NextTransactionNumber(
        MessageType::notarizeTransaction);

    if (0 == openingNumber.Value()) {
        LogError()(OT_PRETTY_CLASS())(
            "Error: Unable to get a transaction number.")
            .Flush();

        return false;
    }

    if (0 == closingNumber.Value()) {
        LogError()(OT_PRETTY_CLASS())("Error: Unable to get a closing "
                                      "transaction number.")
            .Flush();
        // (Since the first one was successful, we just put it back before
        // returning.)

        return false;
    }

    // Above this line, the transaction numbers will be recovered automatically
    openingNumber.SetSuccess(true);
    closingNumber.SetSuccess(true);
    LogError()(OT_PRETTY_CLASS())("Allocated opening transaction number ")(
        openingNumber.Value())(".")
        .Flush();

    LogError()(OT_PRETTY_CLASS())("Allocated closing transaction number ")(
        closingNumber.Value())(".")
        .Flush();

    // At this point we now have 2 transaction numbers...
    // We can't return without either USING THEM, or PUTTING THEM BACK.
    //

    // Set the Transaction Number and the Closing transaction number... (for
    // merchant / recipient.)
    //
    AddRecipientClosingTransactionNo(openingNumber.Value());
    AddRecipientClosingTransactionNo(closingNumber.Value());
    // (They just both go onto this same list.)

    // Set the Consideration memo...
    consideration_->Set(strConsideration);
    LogTrace()(OT_PRETTY_CLASS())("Successfully performed SetProposal.")
        .Flush();

    return true;
}

// THIS FUNCTION IS CALLED BY THE CUSTOMER
//
// (Transaction number and closing number are retrieved from Nym at this time.)
auto OTAgreement::Confirm(
    otx::context::Server& context,
    const Account& PAYER_ACCT,
    const identifier::Nym& p_id_MERCHANT_NYM,
    const identity::Nym* pMERCHANT_NYM) -> bool
{
    auto nym = context.Nym();

    if (nullptr == nym) { return false; }

    const auto& id_PAYER_NYM = nym->ID();
    const auto& id_PAYER_ACCT = PAYER_ACCT.GetPurportedAccountID();

    if (GetRecipientNymID() == GetSenderNymID()) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Error: Sender and recipient have the same "
            "Nym ID (not allowed).")
            .Flush();
        return false;
    } else if (
        (!p_id_MERCHANT_NYM.empty()) &&
        (GetRecipientNymID() != p_id_MERCHANT_NYM)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Merchant has wrong NymID (should be same "
            "as RecipientNymID).")
            .Flush();
        return false;
    } else if (
        (nullptr != pMERCHANT_NYM) &&
        (GetRecipientNymID() != pMERCHANT_NYM->ID())) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Merchant has wrong NymID (should be same "
            "as RecipientNymID).")
            .Flush();
        return false;
    } else if (GetSenderNymID() != id_PAYER_NYM) {
        LogConsole()(OT_PRETTY_CLASS())("Payer has wrong NymID (should be same"
                                        " as SenderNymID).")
            .Flush();
        return false;
    } else if (
        !GetSenderAcctID().empty() && (GetSenderAcctID() != id_PAYER_ACCT)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Payer has wrong AcctID (should be same "
            "as SenderAcctID).")
            .Flush();
        return false;
    } else if (!PAYER_ACCT.VerifyOwner(*nym)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failure: Payer (customer) account is not "
            "owned by Payer Nym.")
            .Flush();
        return false;
    } else if (context.AvailableNumbers() < 2) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failure. You need at least 2 transaction "
            "numbers available to do this.")
            .Flush();
        return false;
    } else if (GetRecipientCountClosingNumbers() < 2) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failure. (The merchant was supposed to "
            "attach 2 transaction numbers).")
            .Flush();
        return false;
    }

    // This is the single reason why MERCHANT_NYM was even passed in here!
    // Supposedly merchant has already signed.  Let's verify this!!
    //
    if ((nullptr != pMERCHANT_NYM) &&
        (false == VerifySignature(*pMERCHANT_NYM))) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Merchant's signature failed to verify.")
            .Flush();
        return false;
    }

    // Now that we KNOW the merchant signed it... SAVE MERCHANT's COPY.
    // Let's save a copy of the one the merchant signed, before changing it and
    // re-signing it,
    // (to add my own transaction numbers...)
    //
    auto strTemp = String::Factory();
    SaveContractRaw(strTemp);
    SetMerchantSignedCopy(strTemp);
    // --------------------------------------------------
    // NOTE: the payer account is either ALREADY set on the payment plan
    // beforehand,
    // in which case this function (above) verifies that the PayerAcct passed in
    // matches that -- OR the payer account was NOT set beforehand (which is
    // likely
    // how people will use it, since the account isn't even known until
    // confirmation,
    // since only the customer knows which account he will choose to pay it with
    // --
    // the merchant has no way of knowing that account ID when he does the
    // initial
    // proposal.)
    // EITHER WAY, we can go ahead and set it here, since we've either already
    // verified
    // it's the right one, or we know it's not set and needs to be set. Either
    // way, this
    // is a safe value to assign here.
    //
    SetSenderAcctID(id_PAYER_ACCT);
    // --------------------------------------------------
    // The payer has to submit TWO transaction numbers in order to activate this
    // agreement...
    //
    auto strNotaryIDstrTemp = String::Factory(GetNotaryID(), api_.Crypto());
    const auto openingNumber = context.InternalServer().NextTransactionNumber(
        MessageType::notarizeTransaction);
    const auto closingNumber = context.InternalServer().NextTransactionNumber(
        MessageType::notarizeTransaction);

    if (0 == openingNumber.Value()) {
        LogError()(OT_PRETTY_CLASS())(
            "Error: Strangely unable to get a transaction number.")
            .Flush();

        return false;
    }

    if (false == closingNumber.Value()) {
        LogError()(OT_PRETTY_CLASS())(
            "Error: Strangely unable to get a closing "
            "transaction number.")
            .Flush();

        return false;
    }

    // Above this line, the transaction numbers will be recovered automatically
    openingNumber.SetSuccess(true);
    closingNumber.SetSuccess(true);

    // At this point we now HAVE 2 transaction numbers (for payer / sender)...
    // We can't return without USING THEM or PUTTING THEM BACK.
    //

    SetTransactionNum(openingNumber.Value());  // Set the Transaction Number
    AddClosingTransactionNo(closingNumber.Value());  // and the Closing Number
                                                     // (both for sender)...

    // CREATION DATE was set in the Merchant's proposal, and it's RESET here in
    // the Confirm.
    // This way, (since we still have the original proposal) we can see BOTH
    // times.
    //
    // Set the Creation Date.
    SetCreationDate(Clock::now());
    LogTrace()(OT_PRETTY_CLASS())("Success!").Flush();

    return true;
}

void OTAgreement::InitAgreement()
{
    contract_type_ = String::Factory("AGREEMENT");
}

void OTAgreement::Release_Agreement()
{
    // If there were any dynamically allocated objects, clean them up here.
    //
    recipient_account_id_.clear();
    recipient_nym_id_.clear();

    consideration_->Release();
    merchant_signed_copy_->Release();

    recipient_closing_numbers_.clear();
}

// the framework will call this at the right time.
//
void OTAgreement::Release()
{
    Release_Agreement();

    ot_super::Release();  // since I've overridden the base class (OTCronItem),
                          // so I call it now...

    // Then I call this to re-initialize everything
    InitAgreement();
}

void OTAgreement::UpdateContents(const PasswordPrompt& reason)
{
    // See OTPaymentPlan::UpdateContents.
}

// return -1 if error, 0 if nothing, and 1 if the node was processed.
auto OTAgreement::ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t
{
    std::int32_t nReturnVal = 0;

    // Here we call the parent class first.
    // If the node is found there, or there is some error,
    // then we just return either way.  But if it comes back
    // as '0', then nothing happened, and we'll continue executing.
    //
    // -- Note you can choose not to call the parent if
    // you don't want to use any of those xml tags.
    // As I do below, in the case of OTAccount.
    if (0 != (nReturnVal = ot_super::ProcessXMLNode(xml))) {
        return nReturnVal;
    }

    if (!strcmp("agreement", xml->getNodeName())) {
        version_ = String::Factory(xml->getAttributeValue("version"));
        SetTransactionNum(
            String::StringToLong(xml->getAttributeValue("transactionNum")));

        const auto tCreation =
            parseTimestamp(xml->getAttributeValue("creationDate"));
        const auto tValidFrom =
            parseTimestamp(xml->getAttributeValue("validFrom"));
        const auto tValidTo = parseTimestamp(xml->getAttributeValue("validTo"));

        SetCreationDate(tCreation);
        SetValidFrom(tValidFrom);
        SetValidTo(tValidTo);

        const auto strNotaryID =
                       String::Factory(xml->getAttributeValue("notaryID")),
                   strInstrumentDefinitionID = String::Factory(
                       xml->getAttributeValue("instrumentDefinitionID")),
                   strSenderAcctID =
                       String::Factory(xml->getAttributeValue("senderAcctID")),
                   strSenderNymID =
                       String::Factory(xml->getAttributeValue("senderNymID")),
                   strRecipientAcctID = String::Factory(
                       xml->getAttributeValue("recipientAcctID")),
                   strRecipientNymID = String::Factory(
                       xml->getAttributeValue("recipientNymID")),
                   strCanceled =
                       String::Factory(xml->getAttributeValue("canceled")),
                   strCancelerNymID =
                       String::Factory(xml->getAttributeValue("cancelerNymID"));

        if (strCanceled->Exists() && strCanceled->Compare("true")) {
            canceled_ = true;

            if (strCancelerNymID->Exists()) {
                canceler_nym_id_ =
                    api_.Factory().NymIDFromBase58(strCancelerNymID->Bytes());
            }
            // else log
        } else {
            canceled_ = false;
            canceler_nym_id_.clear();
        }

        const auto NOTARY_ID =
            api_.Factory().NotaryIDFromBase58(strNotaryID->Bytes());
        const auto INSTRUMENT_DEFINITION_ID =
            api_.Factory().UnitIDFromBase58(strInstrumentDefinitionID->Bytes());
        const auto SENDER_ACCT_ID =
            api_.Factory().AccountIDFromBase58(strSenderAcctID->Bytes());
        const auto RECIPIENT_ACCT_ID =
            api_.Factory().AccountIDFromBase58(strRecipientAcctID->Bytes());
        const auto SENDER_NYM_ID =
            api_.Factory().NymIDFromBase58(strSenderNymID->Bytes());
        const auto RECIPIENT_NYM_ID =
            api_.Factory().NymIDFromBase58(strRecipientNymID->Bytes());

        SetNotaryID(NOTARY_ID);
        SetInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID);
        SetSenderAcctID(SENDER_ACCT_ID);
        SetSenderNymID(SENDER_NYM_ID);
        SetRecipientAcctID(RECIPIENT_ACCT_ID);
        SetRecipientNymID(RECIPIENT_NYM_ID);

        LogDetail()(OT_PRETTY_CLASS())(canceled_ ? "Canceled a" : "A")(
            "greement. Transaction Number: ")(transaction_num_)
            .Flush();

        LogVerbose()(OT_PRETTY_CLASS())("Creation Date: ")(
            tCreation)(" Valid From: ")(tValidFrom)(" Valid To: ")(
            tValidTo)(" InstrumentDefinitionID: ")(
            strInstrumentDefinitionID.get())(" NotaryID: ")(strNotaryID.get())(
            " senderAcctID: ")(strSenderAcctID.get())(" senderNymID: ")(
            strSenderNymID.get())(" recipientAcctID: ")(
            strRecipientAcctID.get())(" recipientNymID: ")(
            strRecipientNymID.get())
            .Flush();

        nReturnVal = 1;
    } else if (!strcmp("consideration", xml->getNodeName())) {
        if (false == LoadEncodedTextField(api_.Crypto(), xml, consideration_)) {
            LogError()(OT_PRETTY_CLASS())(
                "Error in OTPaymentPlan::ProcessXMLNode: Consideration "
                "field without value.")
                .Flush();
            return (-1);  // error condition
        }

        nReturnVal = 1;
    } else if (!strcmp("merchantSignedCopy", xml->getNodeName())) {
        if (false ==
            LoadEncodedTextField(api_.Crypto(), xml, merchant_signed_copy_)) {
            LogError()(OT_PRETTY_CLASS())(
                "Error in OTPaymentPlan::ProcessXMLNode: "
                "merchant_signed_copy field without value.")
                .Flush();
            return (-1);  // error condition
        }

        nReturnVal = 1;
    }

    //  UnallocatedDeque<std::int64_t>   recipient_closing_numbers_; //
    //  Numbers used
    // for CLOSING a transaction. (finalReceipt.)
    else if (!strcmp("closingRecipientNumber", xml->getNodeName())) {
        auto strClosingNumber =
            String::Factory(xml->getAttributeValue("value"));

        if (strClosingNumber->Exists()) {
            const TransactionNumber lClosingNumber = strClosingNumber->ToLong();

            AddRecipientClosingTransactionNo(lClosingNumber);
        } else {
            LogError()(OT_PRETTY_CLASS())(
                "closingRecipientNumber field without value.")
                .Flush();
            return (-1);  // error condition
        }

        nReturnVal = 1;
    }

    return nReturnVal;
}

OTAgreement::~OTAgreement() { Release_Agreement(); }
}  // namespace opentxs
