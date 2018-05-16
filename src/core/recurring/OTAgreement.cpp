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

#include "opentxs/stdafx.hpp"

#include "opentxs/core/recurring/OTAgreement.hpp"

#include "opentxs/api/client/Wallet.hpp"
#include "opentxs/api/Native.hpp"
#include "opentxs/consensus/ClientContext.hpp"
#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/core/cron/OTCron.hpp"
#include "opentxs/core/cron/OTCronItem.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Item.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/NumList.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/OT.hpp"
#include "opentxs/Types.hpp"

#include <irrxml/irrXML.hpp>
#include <string.h>
#include <cstdint>
#include <deque>
#include <ostream>
#include <set>

#define OT_METHOD "opentxs::OTAgreement::"

// OTAgreement is derived from OTCronItem.  It handles re-occuring billing.

namespace opentxs
{

void OTAgreement::setCustomerNymId(const Identifier& NYM_ID)
{
    ot_super::SetSenderNymID(NYM_ID);
}

bool OTAgreement::SendNoticeToAllParties(
    bool bSuccessMsg,
    Nym& theServerNym,
    const Identifier& theNotaryID,
    const TransactionNumber& lNewTransactionNumber,
    // Each party has its own opening trans #.
    const String& strReference,
    String* pstrNote,
    String* pstrAttachment,
    Nym* pActualNym) const
{
    bool bSuccess =
        true;  // Success is defined as ALL parties receiving a notice

    Nym theRecipientNym;  // Don't use this... use the pointer just below.
    Nym* pRecipient = nullptr;

    if (theServerNym.CompareID(GetRecipientNymID())) {
        pRecipient = &theServerNym;  // Just in case the recipient Nym is also
                                     // the server Nym.
    } else if (
        (nullptr != pActualNym) && pActualNym->CompareID(GetRecipientNymID())) {
        pRecipient = pActualNym;
    }

    if (nullptr == pRecipient) {
        const auto NYM_ID = Identifier::Factory(GetRecipientNymID());
        theRecipientNym.SetIdentifier(NYM_ID);

        if (!theRecipientNym.LoadPublicKey()) {
            const String strNymID(NYM_ID);
            otErr << __FUNCTION__
                  << ": Failure loading Recipient's public key: " << strNymID
                  << "\n";
            return false;
        } else if (
            theRecipientNym.VerifyPseudonym() &&
            theRecipientNym.LoadSignedNymfile(
                theServerNym))  // ServerNym here is merely the signer on
                                // this file.
        {
            pRecipient = &theRecipientNym;  //  <=====
        } else {
            const String strNymID(NYM_ID);
            otErr << __FUNCTION__
                  << ": Failure verifying Recipient's public key or loading "
                     "signed nymfile: "
                  << strNymID << "\n";
            return false;
        }
    }
    // BY THIS POINT, the Recipient Nym is definitely loaded up and we have
    // a pointer to him (pRecipient.)

    Nym theSenderNym;  // Don't use this... use the pointer just below.
    Nym* pSender = nullptr;

    if (theServerNym.CompareID(GetSenderNymID())) {
        pSender = &theServerNym;  // Just in case the Sender Nym is also the
                                  // server Nym.
    } else if (
        (nullptr != pActualNym) && pActualNym->CompareID(GetSenderNymID())) {
        pSender = pActualNym;
    }

    if (nullptr == pSender) {
        const auto NYM_ID = Identifier::Factory(GetSenderNymID());
        theSenderNym.SetIdentifier(NYM_ID);

        if (!theSenderNym.LoadPublicKey()) {
            const String strNymID(NYM_ID);
            otErr << __FUNCTION__
                  << ": Failure loading Sender's public key: " << strNymID
                  << "\n";
            return false;
        } else if (
            theSenderNym.VerifyPseudonym() &&
            theSenderNym.LoadSignedNymfile(theServerNym))  // ServerNym
                                                           // here is
                                                           // merely the
                                                           // signer on
                                                           // this file.
        {
            pSender = &theSenderNym;  //  <=====
        } else {
            const String strNymID(NYM_ID);
            otErr << __FUNCTION__
                  << ": Failure verifying Sender's public key "
                     "or loading signed nymfile: "
                  << strNymID << "\n";
            return false;
        }
    }
    // BY THIS POINT, the Sender Nym is definitely loaded up and we have
    // a pointer to him (pSender.)

    // (pRecipient and pSender are both good pointers by this point.)

    // Sender
    if (!OTAgreement::DropServerNoticeToNymbox(
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
            pSender))
        bSuccess = false;
    // Notice I don't break here -- I still allow it to try to notice ALL
    // parties, even if one fails.

    // Recipient
    if (!OTAgreement::DropServerNoticeToNymbox(
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
            pRecipient))
        bSuccess = false;

    return bSuccess;
}

// static
// Used by payment plans and smart contracts. Nym receives an
// OTItem::acknowledgment or OTItem::rejection.
bool OTAgreement::DropServerNoticeToNymbox(
    bool bSuccessMsg,
    const Nym& theServerNym,
    const Identifier& NOTARY_ID,
    const Identifier& NYM_ID,
    const TransactionNumber& lNewTransactionNumber,
    const TransactionNumber& lInReferenceTo,
    const String& strReference,
    originType theOriginType,
    String* pstrNote,
    String* pstrAttachment,
    const Nym* pActualNym)
{
    Ledger theLedger(NYM_ID, NYM_ID, NOTARY_ID);
    // Inbox will receive notification of something ALREADY DONE.
    bool bSuccessLoading = theLedger.LoadNymbox();

    if (true == bSuccessLoading) {
        bSuccessLoading = theLedger.VerifyAccount(theServerNym);
    } else {
        bSuccessLoading = theLedger.GenerateLedger(
            NYM_ID, NOTARY_ID, Ledger::nymbox, true);  // bGenerateFile=true
    }

    if (!bSuccessLoading) {
        otErr << __FUNCTION__
              << ": Failed loading or generating a nymbox. "
                 "(FAILED WRITING RECEIPT!!) \n";

        return false;
    }

    OTTransaction* pTransaction = OTTransaction::GenerateTransaction(
        theLedger, OTTransaction::notice, theOriginType, lNewTransactionNumber);

    if (nullptr != pTransaction) {
        // The nymbox will get a receipt with the new transaction ID.
        // That receipt has an "in reference to" field containing the original
        // OTScriptable

        // Set up the transaction items (each transaction may have multiple
        // items... but not in this case.)
        Item* pItem1 =
            Item::CreateItemFromTransaction(*pTransaction, Item::notice);
        OT_ASSERT(nullptr != pItem1);  // This may be unnecessary, I'll have to
                                       // check CreateItemFromTransaction. I'll
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
        if (nullptr != pstrNote) {
            pItem1->SetNote(*pstrNote);  // in markets, this is updated trade.
        }

        // Nothing is special stored here so far for OTTransaction::notice, but
        // the option is always there.
        //
        if (nullptr != pstrAttachment) {
            pItem1->SetAttachment(*pstrAttachment);
        }

        // sign the item
        //
        pItem1->SignContract(theServerNym);
        pItem1->SaveContract();

        // the Transaction "owns" the item now and will handle cleaning it up.
        pTransaction->AddItem(*pItem1);

        pTransaction->SignContract(theServerNym);
        pTransaction->SaveContract();

        // Here the transaction we just created is actually added to the ledger.
        theLedger.AddTransaction(*pTransaction);

        // Release any signatures that were there before (They won't
        // verify anymore anyway, since the content has changed.)
        theLedger.ReleaseSignatures();

        // Sign and save.
        theLedger.SignContract(theServerNym);
        theLedger.SaveContract();

        // TODO: Better rollback capabilities in case of failures here:

        auto theNymboxHash = Identifier::Factory();

        // Save nymbox to storage. (File, DB, wherever it goes.)
        (theLedger.SaveNymbox(&theNymboxHash.get()));

        // Corresponds to the AddTransaction() call just above. These
        // are stored in a separate file now.
        //
        pTransaction->SaveBoxReceipt(theLedger);

        // Update the NymboxHash (in the nymfile.)
        //
        const Identifier& ACTUAL_NYM_ID = NYM_ID;
        Nym theActualNym;  // unused unless it's really not already
                           // loaded. (use pActualNym.)

        // We couldn't find the Nym among those already loaded--so we have to
        // load
        // it ourselves (so we can update its NymboxHash value.)

        if (nullptr == pActualNym) {
            if (theServerNym.CompareID(ACTUAL_NYM_ID))
                pActualNym = &theServerNym;

            else {
                theActualNym.SetIdentifier(ACTUAL_NYM_ID);

                if (!theActualNym.LoadPublicKey())  // Note: this step
                                                    // may be unnecessary
                                                    // since we are only
                                                    // updating his
                                                    // Nymfile, not his
                                                    // key.
                {
                    String strNymID(ACTUAL_NYM_ID);
                    otErr << __FUNCTION__
                          << ": Failure loading public key for Nym: "
                          << strNymID
                          << ". "
                             "(To update his NymboxHash.) \n";
                } else if (
                    theActualNym.VerifyPseudonym() &&  // this line may be
                                                       // unnecessary.
                    theActualNym.LoadSignedNymfile(
                        theServerNym))  // ServerNym here is not
                                        // theActualNym's identity, but
                                        // merely the signer on this file.
                {
                    otLog3 << __FUNCTION__
                           << ": Loading actual Nym, since he "
                              "wasn't already loaded. (To "
                              "update his NymboxHash.)\n";
                    pActualNym = &theActualNym;  //  <=====
                } else {
                    String strNymID(ACTUAL_NYM_ID);
                    otErr << __FUNCTION__
                          << ": Failure loading or verifying Actual Nym public "
                             "key: "
                          << strNymID
                          << ". "
                             "(To update his NymboxHash.)\n";
                }
            }
        }

        // By this point we've made every possible effort to get the proper Nym
        // loaded, so that we can update his NymboxHash appropriately.
        if (nullptr != pActualNym) {
            auto context = OT::App().Wallet().mutable_ClientContext(
                theServerNym.ID(), pActualNym->ID());
            context.It().SetLocalNymboxHash(theNymboxHash);
        }

        // Really this true should be predicated on ALL the above functions
        // returning true. Right?

        return true;
    } else {
        otErr << __FUNCTION__ << ": Failed trying to create Nymbox.\n";
    }

    return false;  // unreachable.
}

// Overrides from OTTrackable.
bool OTAgreement::HasTransactionNum(const std::int64_t& lInput) const
{
    if (lInput == GetTransactionNum()) return true;

    const size_t nSizeClosing = m_dequeClosingNumbers.size();

    for (size_t nIndex = 0; nIndex < nSizeClosing; ++nIndex) {
        if (lInput == m_dequeClosingNumbers.at(nIndex)) return true;
    }

    const size_t nSizeRecipient = m_dequeRecipientClosingNumbers.size();

    for (size_t nIndex = 0; nIndex < nSizeRecipient; ++nIndex) {
        if (lInput == m_dequeRecipientClosingNumbers.at(nIndex)) return true;
    }

    return false;
}

void OTAgreement::GetAllTransactionNumbers(NumList& numlistOutput) const
{

    if (GetTransactionNum() > 0) numlistOutput.Add(GetTransactionNum());

    const size_t nSizeClosing = m_dequeClosingNumbers.size();

    for (size_t nIndex = 0; nIndex < nSizeClosing; ++nIndex) {
        const std::int64_t lTemp = m_dequeClosingNumbers.at(nIndex);
        if (lTemp > 0) numlistOutput.Add(lTemp);
    }

    const size_t nSizeRecipient = m_dequeRecipientClosingNumbers.size();

    for (size_t nIndex = 0; nIndex < nSizeRecipient; ++nIndex) {
        const std::int64_t lTemp = m_dequeRecipientClosingNumbers.at(nIndex);
        if (lTemp > 0) numlistOutput.Add(lTemp);
    }
}

// Used to be I could just call pAgreement->VerifySignature(theNym), which is
// what I still call here, inside this function. But that's a special case -- an
// override from the OTScriptable / OTSmartContract version, which verifies
// parties and agents, etc.
bool OTAgreement::VerifyNymAsAgent(
    const Nym& theNym,
    const Nym&,
    mapOfConstNyms*) const
{
    return VerifySignature(theNym);
}

// This is an override. See note above.
//
bool OTAgreement::VerifyNymAsAgentForAccount(
    const Nym& theNym,
    Account& theAccount) const
{
    return theAccount.VerifyOwner(theNym);
}

// This is called by OTCronItem::HookRemovalFromCron
// (After calling this method, HookRemovalFromCron then calls
// onRemovalFromCron.)
void OTAgreement::onFinalReceipt(
    OTCronItem& theOrigCronItem,
    const std::int64_t& lNewTransactionNumber,
    Nym& theOriginator,
    Nym* pRemover)
{
    OTCron* pCron = GetCron();

    OT_ASSERT(nullptr != pCron);

    Nym* pServerNym = pCron->GetServerNym();

    OT_ASSERT(nullptr != pServerNym);

    const char* szFunc = "OTAgreement::onFinalReceipt";

    // The finalReceipt Item's ATTACHMENT contains the UPDATED Cron Item.
    // (With the SERVER's signature on it!)
    String strUpdatedCronItem(*this);
    String* pstrAttachment = &strUpdatedCronItem;
    const String strOrigCronItem(theOrigCronItem);
    // Don't use this... use the pointer just below.
    Nym theRecipientNym;
    // The Nym who is actively requesting to remove a cron item will be passed
    // in as pRemover. However, sometimes there is no Nym... perhaps it just
    // expired and pRemover is nullptr. The originating Nym (if different than
    // remover) is loaded up. Otherwise the originator pointer just pointers to
    // *pRemover.
    const Nym* pRecipient = nullptr;

    if (pServerNym->CompareID(GetRecipientNymID())) {
        // Just in case the recipient Nym is also the server Nym.
        pRecipient = pServerNym;
    }
    // If pRemover is NOT nullptr, and he has the Recipient's ID...
    // then set the pointer accordingly.
    else if (
        (nullptr != pRemover) &&
        (true == pRemover->CompareID(GetRecipientNymID()))) {
        // now both pointers are set (to same Nym).
        pRecipient = pRemover;
    }

    if (nullptr == pRecipient) {
        // GetSenderNymID() should be the same on THIS (updated version of the
        // same cron item) but for whatever reason, I'm checking the nymID on
        // the original version. Sue me.
        const auto NYM_ID = Identifier::Factory(GetRecipientNymID());
        theRecipientNym.SetIdentifier(NYM_ID);

        if (!theRecipientNym.LoadPublicKey()) {
            const String strNymID(NYM_ID);
            otErr << szFunc << ": Failure loading Recipient's public key:\n"
                  << strNymID << "\n";
        } else if (
            theRecipientNym.VerifyPseudonym() &&
            theRecipientNym.LoadSignedNymfile(*pServerNym)) {
            pRecipient = &theRecipientNym;
        } else {
            const String strNymID(NYM_ID);
            otErr << szFunc
                  << ": Failure verifying Recipient's public key or "
                     "loading signed nymfile: "
                  << strNymID << "\n";
        }
    }

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
    const String strNotaryID(GetNotaryID());
    // unused unless it's really not already loaded. (use pActualNym.)
    Nym theActualNym;
    auto oContext = OT::App().Wallet().mutable_ClientContext(
        pServerNym->ID(), theOriginator.ID());

    if ((lSenderOpeningNumber > 0) &&
        oContext.It().VerifyIssuedNumber(lSenderOpeningNumber)) {
        Nym* pActualNym = nullptr;  // use this. DON'T use theActualNym.

        // The Nym (server side) stores a list of all opening and closing cron
        // #s. So when the number is released from the Nym, we also take it off
        // that list.
        oContext.It().CloseCronItem(lSenderOpeningNumber);

        // the RemoveIssued call means the original transaction# (to find this
        // cron item on cron) is now CLOSED. But the Transaction itself is still
        // OPEN. How? Because the CLOSING number is still signed out. The
        // closing number is also USED, since the NotarizePaymentPlan or
        // NotarizeMarketOffer call, but it remains ISSUED, until the final
        // receipt itself is accepted during a process inbox.
        oContext.It().ConsumeIssued(lSenderOpeningNumber);
        theOriginator.SaveSignedNymfile(*pServerNym);

        const Identifier& ACTUAL_NYM_ID = GetSenderNymID();

        if ((nullptr != pServerNym) && pServerNym->CompareID(ACTUAL_NYM_ID)) {
            pActualNym = pServerNym;
        } else if (theOriginator.CompareID(ACTUAL_NYM_ID)) {
            pActualNym = &theOriginator;
        } else if (
            (nullptr != pRemover) && pRemover->CompareID(ACTUAL_NYM_ID)) {
            pActualNym = pRemover;
        } else {
            // We couldn't find the Nym among those already loaded--so we have
            // to load it ourselves (so we can update its NymboxHash value.)
            theActualNym.SetIdentifier(ACTUAL_NYM_ID);

            // Note: this step may be unnecessary since we are only updating his
            // Nymfile, not his key.
            if (!theActualNym.LoadPublicKey()) {
                String strNymID(ACTUAL_NYM_ID);
                otErr << szFunc
                      << ": Failure loading public key for Nym : " << strNymID
                      << ". "
                         "(To update his NymboxHash.) \n";
            } else if (
                theActualNym.VerifyPseudonym() &&
                theActualNym.LoadSignedNymfile(*pServerNym)) {
                otLog3
                    << szFunc
                    << ": Loading actual Nym, since he wasn't already loaded. "
                       "(To update his NymboxHash.)\n";
                pActualNym = &theActualNym;  //  <=====
            } else {
                String strNymID(ACTUAL_NYM_ID);
                otErr
                    << szFunc
                    << ": Failure loading or verifying Actual Nym public key: "
                    << strNymID << ". (To update his NymboxHash.)\n";
            }
        }

        if (!DropFinalReceiptToNymbox(
                GetSenderNymID(),
                lNewTransactionNumber,
                strOrigCronItem,
                GetOriginType(),
                nullptr,
                pstrAttachment,
                pActualNym)) {
            otErr << szFunc
                  << ": Failure dropping sender final receipt into nymbox.\n";
        }
    } else {
        otErr << szFunc << ": Failure verifying sender's opening number.\n";
    }

    if ((lSenderClosingNumber > 0) &&
        oContext.It().VerifyIssuedNumber(lSenderClosingNumber)) {
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
                nullptr,
                pstrAttachment))  // pActualAcct=nullptr by default. (This call
                                  // will load it up and update its inbox hash.)
        {
            otErr << szFunc
                  << ": Failure dropping receipt into sender's inbox.\n";
        }
        // This part below doesn't happen until theOriginator ACCEPTS the final
        // receipt (when processing his inbox.)
        //
        //      theOriginator.RemoveIssuedNum(strNotaryID, lSenderClosingNumber,
        // true); //bSave=false
    } else {
        otErr << szFunc
              << ": Failed verifying "
                 "lSenderClosingNumber=theOrigCronItem."
                 "GetClosingTransactionNoAt(0)>0 &&  "
                 "theOriginator.VerifyTransactionNum(lSenderClosingNumber)\n";
    }

    if (nullptr == pRecipient) {
        return;
    }

    auto rContext = OT::App().Wallet().mutable_ClientContext(
        pServerNym->ID(), pRecipient->ID());

    if ((lRecipientOpeningNumber > 0) &&
        rContext.It().VerifyIssuedNumber(lRecipientOpeningNumber)) {
        // The Nym (server side) stores a list of all opening and closing cron
        // #s. So when the number is released from the Nym, we also take it off
        // thatlist.
        rContext.It().CloseCronItem(lRecipientOpeningNumber);

        // the RemoveIssued call means the original transaction# (to find this
        // cron item on cron) is now CLOSED. But the Transaction itself is still
        // OPEN. How? Because the CLOSING number is still signed out. The
        // closing number is also USED, since the NotarizePaymentPlan or
        // NotarizeMarketOffer call, but it remains ISSUED, until the final
        // receipt itself is accepted during a process inbox.
        rContext.It().ConsumeIssued(lRecipientOpeningNumber);

        // NymboxHash is updated here in pRecipient.
        const bool dropped = DropFinalReceiptToNymbox(
            GetRecipientNymID(),
            lNewTransactionNumber,
            strOrigCronItem,
            GetOriginType(),
            nullptr,
            pstrAttachment,
            pRecipient);

        if (!dropped) {
            otErr
                << szFunc
                << ": Failure dropping recipient final receipt into nymbox.\n";
        }
    } else {
        otErr << szFunc
              << ": Failed verifying "
                 "lRecipientClosingNumber="
                 "GetRecipientClosingTransactionNoAt(1)>0 &&  "
                 "pRecipient->VerifyTransactionNum(lRecipientClosingNumber) && "
                 "VerifyIssuedNum(lRecipientOpeningNumber)\n";
    }

    if ((lRecipientClosingNumber > 0) &&
        rContext.It().VerifyIssuedNumber(lRecipientClosingNumber)) {
        if (!DropFinalReceiptToInbox(
                GetRecipientNymID(),
                GetRecipientAcctID(),
                lNewTransactionNumber,
                lRecipientClosingNumber,  // The closing transaction number to
                                          // put on the receipt.
                strOrigCronItem,
                GetOriginType(),
                nullptr,
                pstrAttachment)) {
            otErr << szFunc
                  << ": Failure dropping receipt into recipient's inbox.\n";
        }
    } else {
        otErr << szFunc
              << ": Failed verifying "
                 "lRecipientClosingNumber="
                 "GetRecipientClosingTransactionNoAt(1)>0 &&  "
                 "pRecipient->VerifyTransactionNum(lRecipientClosingNumber) && "
                 "VerifyIssuedNum(lRecipientOpeningNumber)\n";
    }

    // QUESTION: Won't there be Cron Items that have no asset account at all?
    // In which case, there'd be no need to drop a final receipt, but I don't
    // think that's the case, since you have to use a transaction number to get
    // onto cron in the first place.
}

bool OTAgreement::IsValidOpeningNumber(const std::int64_t& lOpeningNum) const
{
    if (GetRecipientOpeningNum() == lOpeningNum) return true;

    return ot_super::IsValidOpeningNumber(lOpeningNum);
}

void OTAgreement::onRemovalFromCron()
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
void OTAgreement::HarvestOpeningNumber(ServerContext& context)
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
void OTAgreement::HarvestClosingNumbers(ServerContext& context)
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

std::int64_t OTAgreement::GetOpeningNumber(const Identifier& theNymID) const
{
    const Identifier& theRecipientNymID = GetRecipientNymID();

    if (theNymID == theRecipientNymID) return GetRecipientOpeningNum();
    // else...
    return ot_super::GetOpeningNumber(theNymID);
}

std::int64_t OTAgreement::GetClosingNumber(const Identifier& theAcctID) const
{
    const Identifier& theRecipientAcctID = GetRecipientAcctID();

    if (theAcctID == theRecipientAcctID) return GetRecipientClosingNum();
    // else...
    return ot_super::GetClosingNumber(theAcctID);
}

TransactionNumber OTAgreement::GetRecipientOpeningNum() const
{
    return (GetRecipientCountClosingNumbers() > 0)
               ? GetRecipientClosingTransactionNoAt(0)
               : 0;  // todo stop hardcoding.
}

TransactionNumber OTAgreement::GetRecipientClosingNum() const
{
    return (GetRecipientCountClosingNumbers() > 1)
               ? GetRecipientClosingTransactionNoAt(1)
               : 0;  // todo stop hardcoding.
}

// These are for finalReceipt
// The Cron Item stores a list of these closing transaction numbers,
// used for closing a transaction.
//

std::int64_t OTAgreement::GetRecipientClosingTransactionNoAt(
    std::uint32_t nIndex) const
{
    OT_ASSERT_MSG(
        (nIndex < m_dequeRecipientClosingNumbers.size()),
        "OTAgreement::GetClosingTransactionNoAt: index out of bounds.");

    return m_dequeRecipientClosingNumbers.at(nIndex);
}

std::int32_t OTAgreement::GetRecipientCountClosingNumbers() const
{
    return static_cast<std::int32_t>(m_dequeRecipientClosingNumbers.size());
}

void OTAgreement::AddRecipientClosingTransactionNo(
    const std::int64_t& closingNumber)
{
    m_dequeRecipientClosingNumbers.push_back(closingNumber);
}

// OTCron calls this regularly, which is my chance to expire, etc.
// Child classes will override this, AND call it (to verify valid date range.)
bool OTAgreement::ProcessCron()
{
    // END DATE --------------------------------
    // First call the parent's version (which this overrides) so it has
    // a chance to check its stuff. Currently it checks IsExpired().
    if (!ot_super::ProcessCron())
        return false;  // It's expired or flagged--removed it from Cron.

    // START DATE --------------------------------
    // Okay, so it's NOT expired. But might not have reached START DATE yet...
    // (If not expired, yet current date is not verified, that means it hasn't
    // ENTERED the date range YET.)
    //
    if (!VerifyCurrentDate())
        return true;  // The Trade is not yet valid, so we return. BUT, we
                      // return
                      //  true, so it will stay on Cron until it BECOMES valid.

    // Process my Agreement-specific stuff
    // below.--------------------------------

    return true;
}

/// See if theNym has rights to remove this item from Cron.
///
bool OTAgreement::CanRemoveItemFromCron(const ClientContext& context)
{
    // You don't just go willy-nilly and remove a cron item from a market unless
    // you check first and make sure the Nym who requested it actually has said
    // number (or a related closing number) signed out to him on his last
    // receipt...
    if (true == ot_super::CanRemoveItemFromCron(context)) {
        return true;
    }

    const String strNotaryID(GetNotaryID());

    // Usually the Nym is the originator. (Meaning GetTransactionNum() on this
    // agreement is still verifiable as an issued number on theNum, and belongs
    // to him.) In that case, the above call will discover this, and return
    // true. In other cases, theNym has the right to Remove the item even though
    // theNym didn't originate it. (Like if he is the recipient -- not the
    // sender -- in a payment plan.) We check such things HERE in this function
    // (see below.)
    if (!context.RemoteNym().CompareID(GetRecipientNymID())) {
        otOut << "OTAgreement::" << __FUNCTION__ << "\n Context Remote Nym ID: "
              << String(context.RemoteNym().ID()) << "\n"
              << "\n Sender Nym ID: " << String(GetSenderNymID()) << "\n"
              << "\n Recipient Nym ID: " << String(GetRecipientNymID()) << "\n"
              << " Weird: Nym tried to remove agreement (payment plan), even "
                 "though he apparently wasn't the sender OR recipient.\n";

        return false;
    } else if (GetRecipientCountClosingNumbers() < 2) {
        otOut << "OTAgreement::" << __FUNCTION__
              << ": Weird: Recipient tried to remove agreement "
                 "(or payment plan); expected 2 closing numbers to be "
                 "available--that weren't."
                 " (Found "
              << GetRecipientCountClosingNumbers() << ").\n";

        return false;
    }

    if (!context.VerifyIssuedNumber(GetRecipientClosingNum())) {
        otOut << "OTAgreement::" << __FUNCTION__
              << ": Recipient Closing "
                 "number didn't verify (for "
                 "removal from cron).\n";

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

bool OTAgreement::CompareAgreement(const OTAgreement& rhs) const
{
    // Compare OTAgreement specific info here.

    if ((m_strConsideration.Compare(rhs.m_strConsideration)) &&
        (GetRecipientAcctID() == rhs.GetRecipientAcctID()) &&
        (GetRecipientNymID() == rhs.GetRecipientNymID()) &&
        //        (   m_dequeClosingNumbers == rhs.m_dequeClosingNumbers ) && //
        // The merchant wouldn't know the customer's trans#s.
        // (Thus wouldn't expect them to be set in BOTH versions...)
        (m_dequeRecipientClosingNumbers ==
         rhs.m_dequeRecipientClosingNumbers) &&
        //      (   GetTransactionNum()  == rhs.GetTransactionNum()   ) && //
        // (commented out for same reason as above.)
        //      (   GetSenderAcctID()    == rhs.GetSenderAcctID()     ) && //
        // Same here -- we should let the merchant leave these blank,
        //      (   GetSenderNymID()    == rhs.GetSenderNymID()     ) && //
        // and then allow the customer to add them in his version,
        (GetInstrumentDefinitionID() ==
         rhs.GetInstrumentDefinitionID()) &&  // (and this Compare function
                                              // still still verify it.)
        (GetNotaryID() == rhs.GetNotaryID()) &&
        (GetValidFrom() == rhs.GetValidFrom()) &&
        (GetValidTo() == rhs.GetValidTo()))
        return true;

    return false;
}

// THIS FUNCTION IS CALLED BY THE MERCHANT
//
// (lMerchantTransactionNumber, lMerchantClosingNumber are set internally in
// this call, from MERCHANT_NYM.)
bool OTAgreement::SetProposal(
    ServerContext& context,
    Account& MERCHANT_ACCT,
    const String& strConsideration,
    time64_t VALID_FROM,
    time64_t VALID_TO)  // VALID_TO is a length here. (i.e. it's ADDED to
                        // valid_from)
{
    auto& nym = *context.Nym();

    auto id_MERCHANT_NYM = Identifier::Factory(nym),
         id_MERCHANT_ACCT =
             Identifier::Factory(MERCHANT_ACCT.GetPurportedAccountID());

    if (GetRecipientNymID() != id_MERCHANT_NYM) {
        otOut << __FUNCTION__
              << ": Merchant has wrong NymID (should be same "
                 "as RecipientNymID.)\n";
        return false;
    } else if (GetRecipientAcctID() != id_MERCHANT_ACCT) {
        otOut << __FUNCTION__
              << ": Merchant has wrong AcctID (should be same "
                 "as RecipientAcctID.)\n";
        return false;
    } else if (!MERCHANT_ACCT.VerifyOwner(nym)) {
        otOut << __FUNCTION__
              << ": Failure: Merchant account is not owned by Merchant Nym.\n";
        return false;
    } else if (GetRecipientNymID() == GetSenderNymID()) {
        otOut << __FUNCTION__
              << ": Failure: Sender and recipient have the same "
                 "Nym ID (not allowed.)\n";
        return false;
    } else if (context.AvailableNumbers() < 2) {
        otOut << __FUNCTION__
              << ": Failure. You need at least 2 transaction "
                 "numbers available to do this.\n";
        return false;
    }
    // --------------------------------------
    // Set the CREATION DATE
    //
    const time64_t CURRENT_TIME = OTTimeGetCurrentTime();

    // Set the Creation Date.
    SetCreationDate(CURRENT_TIME);

    // Putting this above here so I don't have to put the transaction numbers
    // back if this fails:

    // VALID_FROM
    //
    // The default "valid from" time is NOW.
    if (OT_TIME_ZERO >= VALID_FROM)  // if it's 0 or less, set to current time.
        SetValidFrom(CURRENT_TIME);
    else  // Otherwise use whatever was passed in.
        SetValidFrom(VALID_FROM);

    // VALID_TO
    //
    // The default "valid to" time is 0 (which means no expiration date / cancel
    // anytime.)
    if (OT_TIME_ZERO == VALID_TO)  // VALID_TO is 0
    {
        SetValidTo(VALID_TO);  // Keep it at zero then, so it won't expire.
    } else if (OT_TIME_ZERO < VALID_TO)  // VALID_TO is ABOVE zero...
    {
        SetValidTo(OTTimeAddTimeInterval(
            GetValidFrom(), OTTimeGetSecondsFromTime(VALID_TO)));  // Set it to
                                                                   // itself +
        // valid_from.
    } else  // VALID_TO is a NEGATIVE number... Error.
    {
        std::int64_t lValidTo = OTTimeGetSecondsFromTime(VALID_TO);
        otErr << __FUNCTION__ << ": Negative value for valid_to: " << lValidTo
              << "\n";

        return false;
    }

    // Since we'll be needing 2 transaction numbers to do this, let's grab
    // 'em...
    String strNotaryID(GetNotaryID());
    const auto openingNumber =
        context.NextTransactionNumber(MessageType::notarizeTransaction);
    const auto closingNumber =
        context.NextTransactionNumber(MessageType::notarizeTransaction);

    if (0 == TransactionNumber(openingNumber)) {
        otErr << __FUNCTION__
              << ": Error: Unable to get a transaction number.\n";

        return false;
    }

    if (0 == TransactionNumber(closingNumber)) {
        otErr << __FUNCTION__
              << ": Error: Unable to get a closing "
                 "transaction number.\n";
        // (Since the first one was successful, we just put it back before
        // returning.)

        return false;
    }

    // Above this line, the transaction numbers will be recovered automatically
    openingNumber.SetSuccess(true);
    closingNumber.SetSuccess(true);
    otErr << OT_METHOD << __FUNCTION__
          << ": Allocated opening transaction number "
          << TransactionNumber(openingNumber) << std::endl;

    otErr << OT_METHOD << __FUNCTION__
          << ": Allocated closing transaction number "
          << TransactionNumber(closingNumber) << std::endl;

    // At this point we now have 2 transaction numbers...
    // We can't return without either USING THEM, or PUTTING THEM BACK.
    //

    // Set the Transaction Number and the Closing transaction number... (for
    // merchant / recipient.)
    //
    AddRecipientClosingTransactionNo(openingNumber);
    AddRecipientClosingTransactionNo(closingNumber);
    // (They just both go onto this same list.)

    // Set the Consideration memo...
    m_strConsideration.Set(strConsideration);

    otLog4 << "Successfully performed SetProposal.\n";

    return true;
}

// THIS FUNCTION IS CALLED BY THE CUSTOMER
//
// (Transaction number and closing number are retrieved from Nym at this time.)
bool OTAgreement::Confirm(
    ServerContext& context,
    Account& PAYER_ACCT,
    const Nym* pMERCHANT_NYM,
    const Identifier* p_id_MERCHANT_NYM)
{
    auto nym = context.Nym();
    if (nullptr == nym) {
        return false;
    }

    auto id_PAYER_NYM = Identifier::Factory(*nym),
         id_PAYER_ACCT =
             Identifier::Factory(PAYER_ACCT.GetPurportedAccountID());

    if (GetRecipientNymID() == GetSenderNymID()) {
        otOut << __FUNCTION__
              << ": Error: Sender and recipient have the same "
                 "Nym ID (not allowed.)\n";
        return false;
    } else if (
        (nullptr != p_id_MERCHANT_NYM) &&
        (GetRecipientNymID() != *p_id_MERCHANT_NYM)) {
        otOut << __FUNCTION__
              << ": Merchant has wrong NymID (should be same "
                 "as RecipientNymID.)\n";
        return false;
    } else if (
        (nullptr != pMERCHANT_NYM) &&
        (GetRecipientNymID() != pMERCHANT_NYM->GetConstID())) {
        otOut << __FUNCTION__
              << ": Merchant has wrong NymID (should be same "
                 "as RecipientNymID.)\n";
        return false;
    } else if (GetSenderNymID() != id_PAYER_NYM) {
        otOut << __FUNCTION__
              << ": Payer has wrong NymID (should be same as SenderNymID.)\n";
        return false;
    } else if (
        !GetSenderAcctID().empty() && (GetSenderAcctID() != id_PAYER_ACCT)) {
        otOut << __FUNCTION__
              << ": Payer has wrong AcctID (should be same as SenderAcctID.)\n";
        return false;
    } else if (!PAYER_ACCT.VerifyOwner(*nym)) {
        otOut << __FUNCTION__
              << ": Failure: Payer (customer) account is not owned by Payer Nym"
              << std::endl;
        return false;
    } else if (context.AvailableNumbers() < 2) {
        otOut << __FUNCTION__
              << ": Failure. You need at least 2 transaction "
                 "numbers available to do this.\n";
        return false;
    } else if (GetRecipientCountClosingNumbers() < 2) {
        otOut << __FUNCTION__
              << ": Failure. (The merchant was supposed to "
                 "attach 2 transaction numbers.)\n";
        return false;
    }

    // This is the single reason why MERCHANT_NYM was even passed in here!
    // Supposedly merchant has already signed.  Let's verify this!!
    //
    if ((nullptr != pMERCHANT_NYM) &&
        (false == VerifySignature(*pMERCHANT_NYM))) {
        otOut << __FUNCTION__ << ": Merchant's signature failed to verify.\n";
        return false;
    }

    // Now that we KNOW the merchant signed it... SAVE MERCHANT's COPY.
    // Let's save a copy of the one the merchant signed, before changing it and
    // re-signing it,
    // (to add my own transaction numbers...)
    //
    String strTemp;
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
    String strNotaryID(GetNotaryID());
    const auto openingNumber =
        context.NextTransactionNumber(MessageType::notarizeTransaction);
    const auto closingNumber =
        context.NextTransactionNumber(MessageType::notarizeTransaction);

    if (0 == TransactionNumber(openingNumber)) {
        otErr << __FUNCTION__
              << ": Error: Strangely unable to get a transaction number.\n";

        return false;
    }

    if (false == TransactionNumber(closingNumber)) {
        otErr << __FUNCTION__
              << ": Error: Strangely unable to get a closing "
                 "transaction number.\n";

        return false;
    }

    // Above this line, the transaction numbers will be recovered automatically
    openingNumber.SetSuccess(true);
    closingNumber.SetSuccess(true);

    // At this point we now HAVE 2 transaction numbers (for payer / sender)...
    // We can't return without USING THEM or PUTTING THEM BACK.
    //

    SetTransactionNum(openingNumber);  // Set the Transaction Number
    AddClosingTransactionNo(
        closingNumber);  // and the Closing Number (both for sender)...

    // CREATION DATE was set in the Merchant's proposal, and it's RESET here in
    // the Confirm.
    // This way, (since we still have the original proposal) we can see BOTH
    // times.
    //
    time64_t CURRENT_TIME = OTTimeGetCurrentTime();
    // Set the Creation Date.
    SetCreationDate(CURRENT_TIME);

    otLog4 << __FUNCTION__ << "(): Success!\n";

    return true;
}

OTAgreement::OTAgreement()
    : ot_super()
    , m_RECIPIENT_ACCT_ID(Identifier::Factory())
    , m_RECIPIENT_NYM_ID(Identifier::Factory())
    , m_strConsideration()
    , m_strMerchantSignedCopy()
    , m_dequeRecipientClosingNumbers()
{
    InitAgreement();
}

OTAgreement::OTAgreement(
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID)
    : ot_super(NOTARY_ID, INSTRUMENT_DEFINITION_ID)
    , m_RECIPIENT_ACCT_ID(Identifier::Factory())
    , m_RECIPIENT_NYM_ID(Identifier::Factory())
    , m_strConsideration()
    , m_strMerchantSignedCopy()
    , m_dequeRecipientClosingNumbers()
{
    InitAgreement();
}

OTAgreement::OTAgreement(
    const Identifier& NOTARY_ID,
    const Identifier& INSTRUMENT_DEFINITION_ID,
    const Identifier& SENDER_ACCT_ID,
    const Identifier& SENDER_NYM_ID,
    const Identifier& RECIPIENT_ACCT_ID,
    const Identifier& RECIPIENT_NYM_ID)
    : ot_super(
          NOTARY_ID,
          INSTRUMENT_DEFINITION_ID,
          SENDER_ACCT_ID,
          SENDER_NYM_ID)
    , m_RECIPIENT_ACCT_ID(Identifier::Factory())
    , m_RECIPIENT_NYM_ID(Identifier::Factory())
    , m_strConsideration()
    , m_strMerchantSignedCopy()
    , m_dequeRecipientClosingNumbers()
{
    InitAgreement();
    SetRecipientAcctID(RECIPIENT_ACCT_ID);
    SetRecipientNymID(RECIPIENT_NYM_ID);
}

OTAgreement::~OTAgreement() { Release_Agreement(); }

void OTAgreement::InitAgreement() { m_strContractType = "AGREEMENT"; }

void OTAgreement::Release_Agreement()
{
    // If there were any dynamically allocated objects, clean them up here.
    //
    m_RECIPIENT_ACCT_ID->Release();
    m_RECIPIENT_NYM_ID->Release();

    m_strConsideration.Release();
    m_strMerchantSignedCopy.Release();

    m_dequeRecipientClosingNumbers.clear();
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

void OTAgreement::UpdateContents()
{
    // See OTPaymentPlan::UpdateContents.
}

// return -1 if error, 0 if nothing, and 1 if the node was processed.
std::int32_t OTAgreement::ProcessXMLNode(irr::io::IrrXMLReader*& xml)
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
    if (0 != (nReturnVal = ot_super::ProcessXMLNode(xml))) return nReturnVal;

    if (!strcmp("agreement", xml->getNodeName())) {
        m_strVersion = xml->getAttributeValue("version");
        SetTransactionNum(
            String::StringToLong(xml->getAttributeValue("transactionNum")));

        std::int64_t tCreation =
            parseTimestamp(xml->getAttributeValue("creationDate"));

        SetCreationDate(OTTimeGetTimeFromSeconds(tCreation));

        std::int64_t tValidFrom =
            parseTimestamp(xml->getAttributeValue("validFrom"));
        std::int64_t tValidTo =
            parseTimestamp(xml->getAttributeValue("validTo"));

        SetValidFrom(OTTimeGetTimeFromSeconds(tValidFrom));
        SetValidTo(OTTimeGetTimeFromSeconds(tValidTo));

        const String strNotaryID(xml->getAttributeValue("notaryID")),
            strInstrumentDefinitionID(
                xml->getAttributeValue("instrumentDefinitionID")),
            strSenderAcctID(xml->getAttributeValue("senderAcctID")),
            strSenderNymID(xml->getAttributeValue("senderNymID")),
            strRecipientAcctID(xml->getAttributeValue("recipientAcctID")),
            strRecipientNymID(xml->getAttributeValue("recipientNymID")),
            strCanceled(xml->getAttributeValue("canceled")),
            strCancelerNymID(xml->getAttributeValue("cancelerNymID"));

        if (strCanceled.Exists() && strCanceled.Compare("true")) {
            m_bCanceled = true;

            if (strCancelerNymID.Exists())
                m_pCancelerNymID->SetString(strCancelerNymID);
            // else log
        } else {
            m_bCanceled = false;
            m_pCancelerNymID->Release();
        }

        const auto NOTARY_ID = Identifier::Factory(strNotaryID),
                   INSTRUMENT_DEFINITION_ID =
                       Identifier::Factory(strInstrumentDefinitionID),
                   SENDER_ACCT_ID = Identifier::Factory(strSenderAcctID),
                   SENDER_NYM_ID = Identifier::Factory(strSenderNymID),
                   RECIPIENT_ACCT_ID = Identifier::Factory(strRecipientAcctID),
                   RECIPIENT_NYM_ID = Identifier::Factory(strRecipientNymID);

        SetNotaryID(NOTARY_ID);
        SetInstrumentDefinitionID(INSTRUMENT_DEFINITION_ID);
        SetSenderAcctID(SENDER_ACCT_ID);
        SetSenderNymID(SENDER_NYM_ID);
        SetRecipientAcctID(RECIPIENT_ACCT_ID);
        SetRecipientNymID(RECIPIENT_NYM_ID);

        otWarn << "\n\n"
               << (m_bCanceled ? "Canceled a" : "A")
               << "greement. Transaction Number: " << m_lTransactionNum << "\n";

        otInfo << " Creation Date: " << tCreation
               << "   Valid From: " << tValidFrom << "\n Valid To: " << tValidTo
               << "\n"
                  " InstrumentDefinitionID: "
               << strInstrumentDefinitionID << "\n NotaryID: " << strNotaryID
               << "\n"
                  " senderAcctID: "
               << strSenderAcctID << "\n senderNymID: " << strSenderNymID
               << "\n "
                  " recipientAcctID: "
               << strRecipientAcctID
               << "\n recipientNymID: " << strRecipientNymID << "\n ";

        nReturnVal = 1;
    } else if (!strcmp("consideration", xml->getNodeName())) {
        if (false == Contract::LoadEncodedTextField(xml, m_strConsideration)) {
            otErr << "Error in OTPaymentPlan::ProcessXMLNode: consideration "
                     "field without value.\n";
            return (-1);  // error condition
        }

        nReturnVal = 1;
    } else if (!strcmp("merchantSignedCopy", xml->getNodeName())) {
        if (false ==
            Contract::LoadEncodedTextField(xml, m_strMerchantSignedCopy)) {
            otErr << "Error in OTPaymentPlan::ProcessXMLNode: "
                     "merchant_signed_copy field without value.\n";
            return (-1);  // error condition
        }

        nReturnVal = 1;
    }

    //  std::deque<std::int64_t>   m_dequeRecipientClosingNumbers; // Numbers
    //  used
    // for CLOSING a transaction. (finalReceipt.)
    else if (!strcmp("closingRecipientNumber", xml->getNodeName())) {
        String strClosingNumber = xml->getAttributeValue("value");

        if (strClosingNumber.Exists()) {
            const TransactionNumber lClosingNumber = strClosingNumber.ToLong();

            AddRecipientClosingTransactionNo(lClosingNumber);
        } else {
            otErr << "Error in OTAgreement::ProcessXMLNode: "
                     "closingRecipientNumber field without value.\n";
            return (-1);  // error condition
        }

        nReturnVal = 1;
    }

    return nReturnVal;
}

}  // namespace opentxs
