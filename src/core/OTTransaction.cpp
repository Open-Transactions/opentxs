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

#include "opentxs/core/OTTransaction.hpp"

#include "opentxs/consensus/ServerContext.hpp"
#include "opentxs/consensus/TransactionStatement.hpp"
#include "opentxs/core/cron/OTCronItem.hpp"
#include "opentxs/core/crypto/OTASCIIArmor.hpp"
#include "opentxs/core/recurring/OTPaymentPlan.hpp"
#include "opentxs/core/script/OTSmartContract.hpp"
#include "opentxs/core/trade/OTTrade.hpp"
#include "opentxs/core/transaction/Helpers.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/OTFolders.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Item.hpp"
#include "opentxs/core/Ledger.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Message.hpp"
#include "opentxs/core/NumList.hpp"
#include "opentxs/core/Nym.hpp"
#include "opentxs/core/OTStorage.hpp"
#include "opentxs/core/OTStringXML.hpp"
#include "opentxs/core/OTTransactionType.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/Types.hpp"

#include <irrxml/irrXML.hpp>
#include <stdint.h>
#include <cstdlib>
#include <cstring>
#include <memory>
#include <ostream>
#include <string>
#include <vector>

namespace opentxs
{

// Todo: eliminate this function since there is already a list of strings at
// the top of Helpers.hpp, and a list of enums at the top of this header file.
//
// static
OTTransaction::transactionType OTTransaction::GetTypeFromString(
    const String& strType)
{
    OTTransaction::transactionType theType = OTTransaction::error_state;

    if (strType.Compare("blank"))
        theType = OTTransaction::blank;

    else if (strType.Compare("message"))
        theType = OTTransaction::message;
    else if (strType.Compare("notice"))
        theType = OTTransaction::notice;
    else if (strType.Compare("replyNotice"))
        theType = OTTransaction::replyNotice;
    else if (strType.Compare("successNotice"))
        theType = OTTransaction::successNotice;

    else if (strType.Compare("pending"))
        theType = OTTransaction::pending;

    else if (strType.Compare("transferReceipt"))
        theType = OTTransaction::transferReceipt;
    else if (strType.Compare("voucherReceipt"))
        theType = OTTransaction::voucherReceipt;
    else if (strType.Compare("chequeReceipt"))
        theType = OTTransaction::chequeReceipt;
    else if (strType.Compare("marketReceipt"))
        theType = OTTransaction::marketReceipt;
    else if (strType.Compare("paymentReceipt"))
        theType = OTTransaction::paymentReceipt;
    else if (strType.Compare("finalReceipt"))
        theType = OTTransaction::finalReceipt;
    else if (strType.Compare("basketReceipt"))
        theType = OTTransaction::basketReceipt;

    else if (strType.Compare("instrumentNotice"))
        theType = OTTransaction::instrumentNotice;
    else if (strType.Compare("instrumentRejection"))
        theType = OTTransaction::instrumentRejection;

    else if (strType.Compare("processNymbox"))
        theType = OTTransaction::processNymbox;
    else if (strType.Compare("atProcessNymbox"))
        theType = OTTransaction::atProcessNymbox;
    else if (strType.Compare("processInbox"))
        theType = OTTransaction::processInbox;
    else if (strType.Compare("atProcessInbox"))
        theType = OTTransaction::atProcessInbox;
    else if (strType.Compare("transfer"))
        theType = OTTransaction::transfer;
    else if (strType.Compare("atTransfer"))
        theType = OTTransaction::atTransfer;
    else if (strType.Compare("deposit"))
        theType = OTTransaction::deposit;
    else if (strType.Compare("atDeposit"))
        theType = OTTransaction::atDeposit;
    else if (strType.Compare("withdrawal"))
        theType = OTTransaction::withdrawal;
    else if (strType.Compare("atWithdrawal"))
        theType = OTTransaction::atWithdrawal;
    else if (strType.Compare("marketOffer"))
        theType = OTTransaction::marketOffer;
    else if (strType.Compare("atMarketOffer"))
        theType = OTTransaction::atMarketOffer;
    else if (strType.Compare("paymentPlan"))
        theType = OTTransaction::paymentPlan;
    else if (strType.Compare("atPaymentPlan"))
        theType = OTTransaction::atPaymentPlan;
    else if (strType.Compare("smartContract"))
        theType = OTTransaction::smartContract;
    else if (strType.Compare("atSmartContract"))
        theType = OTTransaction::atSmartContract;
    else if (strType.Compare("cancelCronItem"))
        theType = OTTransaction::cancelCronItem;
    else if (strType.Compare("atCancelCronItem"))
        theType = OTTransaction::atCancelCronItem;
    else if (strType.Compare("exchangeBasket"))
        theType = OTTransaction::exchangeBasket;
    else if (strType.Compare("atExchangeBasket"))
        theType = OTTransaction::atExchangeBasket;
    else if (strType.Compare("payDividend"))
        theType = OTTransaction::payDividend;
    else if (strType.Compare("atPayDividend"))
        theType = OTTransaction::atPayDividend;
    else
        theType = OTTransaction::error_state;

    return theType;
}

// Used in balance agreement, part of the inbox report.
int64_t OTTransaction::GetClosingNum() const { return m_lClosingTransactionNo; }

void OTTransaction::SetClosingNum(int64_t lClosingNum)
{
    m_lClosingTransactionNo = lClosingNum;
}

// Make sure this contract checks out. Very high level.
// Verifies ID and signature.
// I do NOT call VerifyOwner() here, because the server may
// wish to verify its signature on this account, even though
// the server may not be the actual owner.
// So if you wish to VerifyOwner(), then call it.
//
// This overrides from OTTransactionType::VerifyAccount()
//
bool OTTransaction::VerifyAccount(const Nym& theNym)
{
    Ledger* pParent = const_cast<Ledger*>(m_pParent);

    // Make sure that the supposed AcctID matches the one read from the file.
    //
    if (!VerifyContractID()) {
        otErr << __FUNCTION__ << ": Error verifying account ID.\n";
        return false;
    }
    // todo security audit:
    else if (
        IsAbbreviated() && (pParent != nullptr) &&
        !pParent->VerifySignature(theNym)) {
        otErr << __FUNCTION__
              << ": Error verifying signature on parent ledger "
                 "for abbreviated transaction receipt.\n";
        return false;
    } else if (!IsAbbreviated() && (false == VerifySignature(theNym))) {
        otErr << __FUNCTION__ << ": Error verifying signature.\n";
        return false;
    }

    otLog4 << "\nWe now know that...\n"
              "1) The expected Account ID matches the ID that was found on the "
              "object.\n"
              "2) The SIGNATURE VERIFIED on the object.\n\n";
    return true;
}

/*
//                      **** MESSAGE TRANSACTIONS ****
//
--------------------------------------------------------------------------------------
        processNymbox,    // process nymbox transaction     // comes from client
        processInbox,    // process inbox transaction     // comes from client
        transfer,        // or "spend". This transaction is a request to
transfer from one account to another
        deposit,        // this transaction is a deposit (cash or cheque)
        withdrawal,        // this transaction is a withdrawal (cash or voucher)
        marketOffer,    // this transaction is a market offer
        paymentPlan,    // this transaction is a payment plan
        smartContract,    // this transaction is a smart contract
        cancelCronItem,    // this transaction is intended to cancel a market
offer or payment plan.
        exchangeBasket,    // this transaction is an exchange in/out of a basket
currency.
        payDividend,    // this transaction is a dividend payment (to
shareholders.)


 HarvestOpeningNumber:

// processNymbox,     // process nymbox transaction     // comes from client  //
HUH?? This message doesn't use a transaction number. That's the whole point of
processNymbox is that it doesn't require such a number.
 processInbox,      // process inbox transaction     // comes from client
 transfer,          // or "spend". This transaction is a request to transfer
from one account to another deposit,           // this transaction is a deposit
(cash or cheque) withdrawal,        // this transaction is a withdrawal (cash or
voucher) marketOffer,       // this transaction is a market offer paymentPlan,
// this transaction is a payment plan smartContract,     // this transaction is
a smart contract cancelCronItem,    // this transaction is intended to cancel a
market offer or payment plan. exchangeBasket,    // this transaction is an
exchange in/out of a basket currency. payDividend,        // this transaction is
dividend payment (to shareholders.)



 HarvestClosingNumbers:    (The X's means "not needed for closing numbers)

// X processNymbox,     // process nymbox transaction     // comes from client
// HUH?? The whole point of processNymbox is that it uses no transaction
numbers. X processInbox,      // process inbox transaction     // comes from
client X transfer,          // or "spend". This transaction is a request to
transfer from one account to another X deposit,           // this transaction is
a deposit (cash or cheque) X withdrawal,        // this transaction is a
withdrawal (cash or voucher) X cancelCronItem,      // this transaction is
intended to cancel a market offer or payment plan. X payDividend,          //
this transaction is a dividend payment (to shareholders.)

 // ONLY THESE:
 marketOffer,       // This contains one opening number, and two closing
numbers. paymentPlan,       // This contains one primary opening number (from
the payer) and his closing number,
                    // as well as the opening and closing numbers for the payee.
NOTE: Unless the paymentPlan SUCCEEDED in
                    // activating, then the SENDER's numbers are both still
good. (Normally even attempting a transaction
                    // will burn the opening number, which IS the case here, for
the payer. But the PAYEE only burns his
                    // opening number IF SUCCESS. Thus, even if the message
succeeded but the transaction failed, where
                    // normally the opening number is burned, it's still good
for the PAYEE (not the payer.) Therefore we
                    // need to make sure, in the case of paymentPlan, that we
still claw back the opening number (FOR THE
                    // PAYEE) in the place where we normally would only claw
back the closing number. smartContract,     // This contains an opening number
for each party, and a closing number for each asset account.



 exchangeBasket,    // this transaction is an exchange in/out of a
basketcurrency.



 */

// Only do this if the message itself failed, meaning this transaction never
// even
// attempted, and thus this transaction NEVER EVEN HAD A *CHANCE* TO FAIL, and
// thus
// the opening number never got burned (Normally no point in harvesting a burned
// number, now is there?)
//
// Client-side.
//
// Returns true/false whether it actually harvested a number.
//
bool OTTransaction::HarvestOpeningNumber(
    ServerContext& context,
    Nym& theNym,
    bool bHarvestingForRetry,     // The message was sent, failed somehow, and
                                  // is now being re-tried.
    bool bReplyWasSuccess,        // false until positively asserted.
    bool bReplyWasFailure,        // false until positively asserted.
    bool bTransactionWasSuccess,  // false until positively asserted.
    bool bTransactionWasFailure)  // false until positively asserted.
{
    bool bSuccess = false;

    switch (m_Type) {
        // Note: the below remarks about "success or fail" are specific to
        // TRANSACTION success, not message success.
        //      case OTTransaction::processNymbox:  // NOTE: why was this here?
        //      You
        // don't need trans#s to process a Nymbox--that's the whole point of a
        // Nymbox.
        case OTTransaction::processInbox:  // Uses 1 transaction #, the opening
                                           // number, and burns it whether
                                           // transaction is success-or-fail.
        case OTTransaction::withdrawal:    // Uses 1 transaction #, the opening
                                           // number, and burns it whether
                                           // transaction is success-or-fail.
        case OTTransaction::deposit:       // Uses 1 transaction #, the opening
                                           // number, and burns it whether
                                           // transaction is success-or-fail.
        case OTTransaction::cancelCronItem:  // Uses 1 transaction #, the
                                             // opening number, and burns it
                                             // whether transaction is
                                             // success-or-fail.
        case OTTransaction::payDividend:  // Uses 1 transaction #, the opening
            // number, and burns it whether transaction
            // is success-or-fail.

            // If the server reply message was unambiguously a FAIL, that means
            // the opening number is STILL GOOD. (Because the transaction
            // therefore never even had a chance to run.)
            //
            // Note: what if, instead, I don't know whether the transaction
            // itself failed, because I don't have a reply message? In that
            // case, I cannot claw back the numbers because I don't know for
            // sure. But my future transactions WILL fail if
            // my nymbox hash goes out of sync, so if that transaction DID
            // process, then I'll find out right away, and I'll be forced to
            // download the nymbox and box receipts in order to get back into
            // sync again. And if the transaction did NOT process,
            // then I'll know it when I don't find it among the receipts. In
            // which case I can pull the original message from the outbuffer,
            // using the request number from when it was sent, and then harvest
            // it from there.
            //
            if (bReplyWasFailure)  // NOTE: If I'm harvesting for a re-try,
            {
                bSuccess = context.RecoverAvailableNumber(GetTransactionNum());
            }
            // Else if the server reply message was unambiguously a SUCCESS,
            // that means the opening number is DEFINITELY BURNED. (Why? Because
            // that means the transaction definitely ran--and the opener is
            // burned success-or-fail, if the transaction runs.)
            //
            else if (bReplyWasSuccess) {
                // The opener is DEFINITELY BAD, so therefore, we're definitely
                // not going to claw it back!
                //
                //              bSuccess =
                // theNym.ClawbackTransactionNumber(GetPurportedNotaryID(),
                //                                                          GetTransactionNum());
                // //bSave=false, pSignerNym=nullptr
            }
            break;

        case OTTransaction::transfer:  // Uses 1 transaction #, the opening
                                       // number, and burns it if failure. But
                                       // if success, merely marks it as "used."

            // If the server reply message was unambiguously a FAIL, that means
            // the opening number is STILL GOOD. (Because the transaction
            // therefore never even had a chance to run.)
            //
            if (bReplyWasFailure) {
                bSuccess = context.RecoverAvailableNumber(GetTransactionNum());
            }
            // Else if the server reply message was unambiguously a SUCCESS,
            // that means the opening number is DEFINITELY NOT HARVESTABLE. Why?
            // Because that means the transaction definitely ran--and the opener
            // is marked as "used" on success, and "burned" on failure--either
            // way, that's bad for harvesting (no point.)
            //
            else if (bReplyWasSuccess) {
                if (bTransactionWasSuccess) {
                    // This means the "transfer" transaction# is STILL MARKED AS
                    // "USED", and will someday be marked as CLOSED.
                    // EITHER WAY, you certainly can't claw that number back
                    // now! (It is still outstanding, though. It's not gone,
                    // yet...)
                    //                  bSuccess =
                    // theNym.ClawbackTransactionNumber(GetPurportedNotaryID(),
                    //                                                              GetTransactionNum());
                    // //bSave=false, pSignerNym=nullptr
                } else if (bTransactionWasFailure) {
                    // Whereas if the transaction was a failure, that means the
                    // transaction number was DEFINITELY burned.
                    // (No point clawing it back now--it's gone already.)
                    //                  bSuccess =
                    // theNym.ClawbackTransactionNumber(GetPurportedNotaryID(),
                    //                                                              GetTransactionNum());
                    // //bSave=false, pSignerNym=nullptr
                }
            }
            break;

        case OTTransaction::marketOffer:  // Uses 3 transaction #s, the opening
                                          // number and 2 closers. If failure,
                                          // opener is burned.
            // But if success, merely marks it as "used." Closers are also
            // marked "used" if success, but if message succeeds while
            // transaction fails, then closers can be harvested. If the server
            // reply message was unambiguously a FAIL, that means the opening
            // number is STILL GOOD. (Because the transaction therefore never
            // even had a chance to run.)
            //
            if (bReplyWasFailure) {
                bSuccess = context.RecoverAvailableNumber(GetTransactionNum());
            }
            // Else if the server reply message was unambiguously a SUCCESS,
            // that means the opening number is DEFINITELY NOT HARVESTABLE. Why?
            // Because that means the transaction definitely ran--and the opener
            // is marked as "used" on success, and "burned" on failure--either
            // way, that's bad for harvesting (no point.)
            //
            else if (bReplyWasSuccess) {
                if (bTransactionWasSuccess) {
                    // This means the "marketOffer" transaction# is STILL MARKED
                    // AS "USED", and will someday be marked as CLOSED. EITHER
                    // WAY, you certainly can't claw that number back now! (It
                    // is still outstanding, though. It's not gone, yet...)
                    //                  bSuccess =
                    // theNym.ClawbackTransactionNumber(GetPurportedNotaryID(),
                    //                                                              GetTransactionNum());
                    // //bSave=false, pSignerNym=nullptr
                } else if (bTransactionWasFailure) {
                    // Whereas if the transaction was a failure, that means the
                    // transaction number was DEFINITELY burned.
                    // (No point clawing it back now--it's gone already.)
                    //                  bSuccess =
                    // theNym.ClawbackTransactionNumber(GetPurportedNotaryID(),
                    //                                                              GetTransactionNum());
                    // //bSave=false, pSignerNym=nullptr
                }
            }

            break;

        case OTTransaction::exchangeBasket:  // Uses X transaction #s: the
                                             // opener, which is burned
                                             // success-or-fail, and Y closers
                                             // (one for
            // each account.) Closers are marked "used" if success transaction,
            // but if message succeeds while transaction fails, then closers can
            // be harvested. If the server reply message was unambiguously a
            // FAIL, that means the opening number is STILL GOOD. (Because the
            // transaction therefore never even had a chance to run.)
            //
            if (bReplyWasFailure) {
                bSuccess = context.RecoverAvailableNumber(GetTransactionNum());
            }
            // Else if the server reply message was unambiguously a SUCCESS,
            // that means the opening number is DEFINITELY BURNED. (Why? Because
            // that means the transaction definitely ran--and the opener is
            // burned "success-or-fail", if this transaction runs.)
            //
            else if (bReplyWasSuccess) {
                // The opener is DEFINITELY BURNED, so therefore, we're
                // definitely not going to claw it back!
                //
                //              bSuccess =
                // theNym.ClawbackTransactionNumber(GetPurportedNotaryID(),
                //                                                          GetTransactionNum());
                // //bSave=false, pSignerNym=nullptr
            }
            break;

        // These aren't AS simple.
        case OTTransaction::paymentPlan:  // Uses 4 transaction #s: the opener
                                          // (sender's #), which burned on
                                          // failure but kept alive on success,
            // the sender's closer, which is only marked as "used" upon success,
            // and the recipient's opening and closing numbers, which are both
            // only marked as "used" upon success.
            {
                // The PAYER's (sender) opening number is burned just from
                // TRYING a transaction. It's only left open if the transaction
                // succeeds (but in that case, it's still marked as "used.") But
                // the PAYEE's (recipient) opening number isn't marked as "used"
                // UNLESS the transaction succeeds.
                //
                // Basically a failed transaction means the sender's opening
                // number is burned and gone, but the recipient's must be clawed
                // back!! Whereas if the message fails (before transaction even
                // has a chance to run) then BOTH sender and recipient can claw
                // back their numbers. The only way to tell the difference is to
                // look at the message itself (the info isn't stored here in the
                // transaction.)
                //
                // 1.
                // Therefore we must assume that the CALLER OF THIS FUNCTION
                // knows. If the message failed, he knows this, and he
                // SPECIFICALLY called HarvestOpeningNumber() ANYWAY, to get the
                // opening number back (when normally he would only recoup the
                // closed numbers--therefore he MUST know that the message
                // failed and that the number is thus still good!)
                //
                // 2.
                // WHEREAS if the message SUCCEEDED (followed by transaction
                // FAIL), then the payer/sender already used his opening number,
                // whereas the recipient DID NOT! Again, the caller MUST KNOW
                // THIS ALREADY. The caller wouldn't call "HarvestOpeningNumber"
                // for a burned number (of the sender.) Therefore he must be
                // calling it to recoup the (still issued) opening number of the
                // RECIPIENT.
                //
                // Problems:
                // 1. What if caller is stupid, and message hasn't actually
                // failed? What if caller is mistakenly
                //    trying to recoup numbers that are actually burned already?
                // Well, the opening number is already
                //    marked as "used but still issued" so when I try to claw it
                // back, that will work (because it
                //    only adds a number BACK after it can confirm that the
                //    number
                // WAS issued to me in the first place,
                //    and in this case, that verification will succeed.)
                //    THEREFORE: need to explicitly pass the message's
                // success/failure status into the current
                //    function. IF the msg was a failure, the transaction never
                //    had
                // a chance to run and thus the
                //    opening number is still good, and we can claw it back. But
                //    if
                // the message was a SUCCESS, then
                //    the transaction definitely TRIED to run, which means the
                // opening number is now burned. (IF
                //    the transaction itself failed, that is. Otherwise if it
                // succeeded, then it's possible, in the
                //    cases of transfer and marketOffer, that the opening number
                //    is
                // STILL "used but issued", until
                //    you finally close out your transferReceipt or the
                //    finalReceipt
                // for your market offer.)
                //
                // 2. What if caller is stupid, and he called
                // HarvestOpeningNumber for the sender, even though the
                //    number was already burned in the original attempt? (Which
                //    we
                // know it was, since the message
                //    itself succeeded.) The sender, of course, has that number
                //    on
                // his "issued" list, so his clawback
                //    will succeed, putting him out of sync.
                //
                // 3. What if the recipient is passed into this function? His
                // "opening number" is not the primary
                //    one, but rather, there are three "closing numbers" on a
                // payment plan. One for the sender, to
                //    match his normal opening number, and 2 more for the
                //    recipient
                // (an opening and closing number).
                //    Therefore in the case of the recipient, need to grab HIS
                // opening number, not the sender's.
                //    (Therefore need to know whether Nym is sender or
                //    recipient.)
                // Is that actually true? Or won't
                //    the harvest process be smart enough to figure that out
                // already? And will it know that the
                //    recipient still needs to harvest HIS opening number, even
                //    if
                // the transaction was attempted,
                //    since the recipient's number wasn't marked as "used"
                //    unless
                // the transaction itself succeeded.
                //    NOTE: CronItem/Agreement/PaymentPlan is definitely smart
                // enough already to know if the Nym is
                //    the sender or recipient. It will only grab the appropriate
                // number for the right Nym. But here
                //    in THIS function we still have to be smart enough not to
                //    call
                // it for the SENDER if the transaction
                //    was attempted (because it must be burned already), but TO
                //    call
                // it for the sender if the transaction
                //    was not even attempted (meaning it wasn't burned yet.)
                // Similarly, this function has to be smart
                //    enough TO call it for the recipient if transaction was
                // attempted but didn't succeed, since the
                //    recipient's opening number is still good in that case.
                //

                const Identifier theNymID(theNym);

                // Assumption: if theNymID matches GetNymID(), then theNym
                // must be the SENDER / PAYER!
                // Else, he must be RECIPIENT / PAYEE, instead!
                // This assumption is not for proving, since the harvest
                // functions will verify the Nym's identity anyway. Instead,
                // this assumption is merely for deciding which logic to use
                // about which harvest functions to call.
                //
                if (theNymID == GetNymID())  // theNym is SENDER / PAYER
                {
                    // If the server reply message was unambiguously a FAIL,
                    // that means the opening number is STILL GOOD. (Because the
                    // transaction therefore never even had a chance to run.)
                    //
                    if (bReplyWasFailure && !bHarvestingForRetry) {
                        bSuccess =
                            context.RecoverAvailableNumber(GetTransactionNum());
                    }
                    // Else if the server reply message was unambiguously a
                    // SUCCESS, that means the opening number is DEFINITELY NOT
                    // HARVESTABLE. (For the sender, anyway.) Why not? Because
                    // that means the transaction definitely ran--and
                    // the opener is marked as "used" on success, and "burned"
                    // on failure--either way, that's bad for harvesting (no
                    // point.)
                    //
                    else if (bReplyWasSuccess) {
                        if (bTransactionWasSuccess) {
                            // This means the "paymentPlan" transaction# is
                            // MARKED AS "USED", and will someday be marked as
                            // CLOSED. EITHER WAY, you certainly can't claw that
                            // number back now! (It is still outstanding,
                            // though. It's not gone, yet...)
                            //                      bSuccess =
                            // theNym.ClawbackTransactionNumber(GetPurportedNotaryID(),
                            //                                                                  GetTransactionNum());
                            // //bSave=false, pSignerNym=nullptr
                        } else if (bTransactionWasFailure) {
                            // Whereas if the transaction was a failure, that
                            // means the transaction number was DEFINITELY
                            // burned. (No point clawing it back now--it's gone
                            // already.)
                            //                      bSuccess =
                            // theNym.ClawbackTransactionNumber(GetPurportedNotaryID(),
                            //                                                                  GetTransactionNum());
                            // //bSave=false, pSignerNym=nullptr
                        }
                    }
                }

                // theNym is RECIPIENT / PAYEE
                //
                // This case is slightly different because above, a successful
                // message with a failed transaction will burn the
                // opening number, whereas here, if the message is successful
                // but the transaction is failed, the recipient's opening
                // transaction number is STILL GOOD and can be harvested!
                // TODO: Make sure payment plans drop a NOTICE
                // to the recipient, so he can harvest his numbers when this
                // happens (similar to todos I have for smart contracts.)
                //
                // The other big difference with the recipient is that he has a
                // different opening and closing number than the sender
                // does, so I need to see if I can get those from the
                // transaction, or if I have to load up the attached cron item
                // to get that data.
                //
                else  // theNym is RECIPIENT / PAYEE
                {
                    // What is this class doing here?
                    // Answer: it's the C++ equivalent of local functions.
                    //
                    class _getRecipientOpeningNum
                    {
                    public:
                        int64_t Run(OTTransaction& theTransaction)
                        {
                            Item* pItem =
                                theTransaction.GetItem(Item::paymentPlan);
                            if (nullptr != pItem) {
                                // Also load up the Payment Plan from inside the
                                // transaction item.
                                //
                                String strPaymentPlan;
                                OTPaymentPlan thePlan;
                                pItem->GetAttachment(strPaymentPlan);

                                if (strPaymentPlan.Exists() &&
                                    thePlan.LoadContractFromString(
                                        strPaymentPlan))
                                    return thePlan.GetRecipientOpeningNum();
                                else
                                    otErr << "OTTransaction::"
                                             "HarvestOpeningNumber: "
                                             "Error: Unable to load "
                                             "paymentPlan object from "
                                             "paymentPlan "
                                             "transaction item.\n";
                            } else
                                otErr << "OTTransaction::HarvestOpeningNumber: "
                                         "Error: Unable to find "
                                         "paymentPlan item in paymentPlan "
                                         "transaction.\n";
                            return 0;
                        }
                    };  // class _getRecipientOpeningNum

                    // If the server reply message was unambiguously a FAIL,
                    // that means the opening number is STILL GOOD. (Because the
                    // transaction therefore never even had a chance to run.)
                    //
                    if (bReplyWasFailure && !bHarvestingForRetry) {
                        _getRecipientOpeningNum getRecipientOpeningNum;
                        const int64_t lRecipientOpeningNum =
                            getRecipientOpeningNum.Run(*this);

                        if (lRecipientOpeningNum > 0) {
                            bSuccess = context.RecoverAvailableNumber(
                                lRecipientOpeningNum);
                        }
                    }
                    // Else if the server reply message was unambiguously a
                    // SUCCESS, then the next question is whether the
                    // TRANSACTION INSIDE IT was also a success, or if there's a
                    // "success message / failed transaction" situation
                    // going on here. For the recipient, that's important: in
                    // the first case, his opener is definitely marked as "used
                    // but still outstanding" and CANNOT be harvested. But in
                    // the second case, unlike with the sender, his opener IS
                    // harvestable!
                    // This is because of a peculiarity with payment plans: the
                    // recipient's opening number is not marked as used until
                    // the transaction itself is a success!
                    //
                    else if (bReplyWasSuccess) {
                        if (bTransactionWasSuccess)  // The opener is DEFINITELY
                                                     // marked as "used but
                                                     // still outstanding" and
                                                     // CANNOT be harvested.
                        {
                            // This means the "paymentPlan" transaction# is
                            // MARKED AS "USED", and will someday be marked as
                            // CLOSED. EITHER WAY, you certainly can't claw that
                            // number back now! (It is still outstanding,
                            // though. It's not gone, yet...)
                            //                      bSuccess =
                            // theNym.ClawbackTransactionNumber(GetPurportedNotaryID(),
                            //                                                                  RECIPIENTS--OPENING--NUMBER--GOES--HERE);
                            // //bSave=false, pSignerNym=nullptr
                        } else if (
                            bTransactionWasFailure && !bHarvestingForRetry) {
                            // In this case, unlike with the sender, the
                            // recipient's opener IS still harvestable! This is
                            // because of a peculiarity with payment plans: the
                            // recipient's opening number is not marked as used
                            // until the transaction itself is a success!
                            // Therefore, if the transaction was a failure, that
                            // means the recipient's opening number is
                            // DEFINITELY STILL GOOD.
                            //
                            _getRecipientOpeningNum getRecipientOpeningNum;
                            const int64_t lRecipientOpeningNum =
                                getRecipientOpeningNum.Run(*this);

                            if (lRecipientOpeningNum > 0) {
                                bSuccess = context.RecoverAvailableNumber(
                                    lRecipientOpeningNum);
                            }
                        }
                    }
                }
            }
            break;

        // TODO: Make sure that when a user receives a success notice that a
        // smart contract has been started up, that he marks his opener as
        // "burned" instead of as "used." It's gone! Also: if the user receives
        // a message failure notice (not done yet) then he can mark his opening
        // # as "new" again! But if he instead receives a "message success but
        // transaction failure", (todo: notice)
        // then he must burn his opening #, as that is what the server has
        // already done.
        //
        // In the case where message and transaction were BOTH success, then the
        // user's existing setup is already
        // correct. (The openers AND closers are already marked as "used but
        // still issued" on the client side, and the server-side sees things
        // that way already as well.)
        //

        case OTTransaction::smartContract:  // Uses X transaction #s, with an
                                            // opener for each party and a
                                            // closer for each asset account.
            // If the message is rejected by the server, then ALL openers can be
            // harvested. But if the
            // message was successful (REGARDLESS of whether the transaction was
            // successful) then all of
            // the openers for all of the parties have been burned. The closers,
            // meanwhile, can be recovered
            // if the message is a failure, as well as in cases where message
            // succeeds but transaction failed.
            // But if transaction succeeded, then the closers CANNOT be
            // recovered. (Only removed, once you sign off on the receipt.)
            {

                Item* pItem = GetItem(Item::smartContract);

                if (nullptr == pItem) {
                    otErr
                        << "OTTransaction::HarvestOpeningNumber: Error: Unable "
                           "to find "
                           "smartContract item in smartContract transaction.\n";
                } else  // Load up the smart contract...
                {
                    String strSmartContract;
                    OTSmartContract theSmartContract(GetPurportedNotaryID());
                    pItem->GetAttachment(strSmartContract);

                    // If we failed to load the smart contract...
                    if (!strSmartContract.Exists() ||
                        (false == theSmartContract.LoadContractFromString(
                                      strSmartContract))) {
                        otErr << "OTTransaction::HarvestOpeningNumber: Error: "
                                 "Unable to load "
                                 "smartContract object from smartContract "
                                 "transaction item.\n";
                    } else  // theSmartContract is ready to go....
                    {

                        // The message reply itself was a failure. This means
                        // the transaction itself never got a chance to run...
                        // which means ALL the opening numbers on that
                        // transaction are STILL GOOD.
                        //
                        if (bReplyWasFailure && !bHarvestingForRetry) {
                            // If I WAS harvesting for a re-try, I'd want to
                            // leave the opening number on this smart contract
                            theSmartContract.HarvestOpeningNumber(context);
                            bSuccess = true;
                        }
                        // Else if the server reply message was unambiguously a
                        // SUCCESS, that means the opening number is DEFINITELY
                        // NOT HARVESTABLE. Why? Because that means the
                        // transaction definitely ran--and the opener is marked
                        // as "used" on SUCCESS, or "burned" on FAILURE--either
                        // way, that's bad for harvesting (no point.)
                        //
                        else if (bReplyWasSuccess) {
                            if (bTransactionWasSuccess) {
                                // This means the "smartContract" opening trans#
                                // is MARKED AS "USED", and will someday be
                                // marked as CLOSED. EITHER WAY, you certainly
                                // can't claw that number back now! (It is still
                                // outstanding, though. It's not gone, yet...)
                                //
                                //                          theSmartContract.HarvestOpeningNumber(theNym);
                                //                          bSuccess = true;
                            } else if (bTransactionWasFailure) {
                                // Whereas if the transaction was a failure,
                                // that means the opening trans number was
                                // DEFINITELY burned. (No point clawing it back
                                // now--it's gone already.)
                                //
                                //                          theSmartContract.HarvestOpeningNumber(theNym);
                                //                          bSuccess = true;
                            }
                        }  // else if (bReplyWasSuccess)

                    }  // else (smart contract loaded successfully)
                }      // pItem was found.
            }
            break;

        default:
            break;
    }

    return bSuccess;
}

// Normally do this if your transaction ran--and failed--so you can get most of
// your transaction numbers back. (The opening number is usually already gone,
// but any others are still salvageable.)
//
bool OTTransaction::HarvestClosingNumbers(
    ServerContext& context,
    Nym& theNym,
    bool bHarvestingForRetry,     // false until positively asserted.
    bool bReplyWasSuccess,        // false until positively asserted.
    bool bReplyWasFailure,        // false until positively asserted.
    bool bTransactionWasSuccess,  // false until positively asserted.
    bool bTransactionWasFailure)  // false until positively asserted.
{
    bool bSuccess = false;

    switch (m_Type) {  // Note: the below remarks about "success or fail" are
                       // specific to TRANSACTION success, not message success.

        //      case OTTransaction::processNymbox:  // Why is this even here?
        // processNymbox uses NO trans#s--that's the purpose of processNymbox.
        case OTTransaction::processInbox:    // Has no closing numbers.
        case OTTransaction::deposit:         // Has no closing numbers.
        case OTTransaction::withdrawal:      // Has no closing numbers.
        case OTTransaction::cancelCronItem:  // Has no closing numbers.
        case OTTransaction::payDividend:     // Has no closing numbers.
        case OTTransaction::transfer:        // Has no closing numbers.

            break;

        case OTTransaction::marketOffer:  // Uses 3 transaction #s, the opening
                                          // number and 2 closers.
            // If message fails, all closing numbers are harvestable.
            // If message succeeds but transaction fails, closers can also be
            // harvested.
            // If message succeeds and transaction succeeds, closers are marked
            // as "used" (like opener.) In that last case, you can't claw them
            // back since they are used.
            {
                Item* pItem = GetItem(Item::marketOffer);

                if (nullptr == pItem) {
                    otErr << "OTTransaction::HarvestClosingNumbers: Error: "
                             "Unable "
                             "to find "
                             "marketOffer item in marketOffer transaction.\n";
                } else  // pItem is good. Let's load up the OTCronIteam
                        // object...
                {
                    OTTrade theTrade;
                    String strTrade;
                    pItem->GetAttachment(strTrade);

                    // First load the Trade up...
                    const bool bLoadContractFromString =
                        (strTrade.Exists() &&
                         theTrade.LoadContractFromString(strTrade));

                    // If failed to load the trade...
                    if (!bLoadContractFromString) {
                        otErr << "OTTransaction::HarvestClosingNumbers: ERROR: "
                                 "Failed loading trade from string:\n\n"
                              << strTrade << "\n\n";
                    } else  // theTrade is ready to go....
                    {

                        // The message reply itself was a failure. This means
                        // the transaction itself never got a chance to run...
                        // which means ALL the closing numbers on that
                        // transaction are STILL GOOD.
                        //
                        if (bReplyWasFailure)  // && !bHarvestingForRetry) // on
                                               // re-try, we need the closing #s
                                               // to stay put, so the re-try has
                                               // a chance to work.
                        {  // NOTE: We do NOT exclude harvesting of closing
                           // numbers,
                            // for a marketoffer, based on bHarvestingForRetry.
                            // Why not? Because with marketOffer, ALL
                            // transaction #s are re-set EACH re-try, not just
                            // the opening #. Therefore ALL must be clawed back.
                            theTrade.HarvestClosingNumbers(
                                context);  // (Contrast this with payment plan,
                                           // exchange basket, smart
                                           // contract...)
                            bSuccess = true;

                            //                      theTrade.GetAssetAcctClosingNum();
                            // // For reference.
                            //                      theTrade.GetCurrencyAcctClosingNum();
                            // // (The above harvest call grabs THESE numbers.)

                        }
                        // Else if the server reply message was unambiguously a
                        // SUCCESS, that means the opening number is DEFINITELY
                        // NOT HARVESTABLE. Why? Because that means the
                        // transaction definitely ran--and the opener is marked
                        // as "used" on SUCCESS, or "burned" on FAILURE--either
                        // way, that's bad for harvesting (no point.)
                        //
                        // ===> But the CLOSING numbers are harvestable on
                        // transaction *failure.*
                        // (They are not harvestable on transaction *success*
                        // though.)
                        //
                        else if (bReplyWasSuccess) {
                            if (bTransactionWasSuccess) {
                                // (They are not harvestable on transaction
                                // success though.) This means the "marketOffer"
                                // closing trans#s (one for asset account, and
                                // one for currency account) are both MARKED AS
                                // "USED", and will someday be marked as CLOSED.
                                // EITHER WAY, you certainly can't claw those
                                // numbers back now! (They are still
                                // outstanding, though. They're not gone,
                                // yet...)
                                //
                                //                          theTrade.HarvestClosingNumbers(theNym);
                                //                          bSuccess = true;
                            } else if (bTransactionWasFailure) {
                                // But the CLOSING numbers ARE harvestable on
                                // transaction failure.
                                // (Closing numbers for marketOffers are only
                                // marked "used" if the marketOffer transaction
                                // was a success.)
                                //
                                theTrade.HarvestClosingNumbers(context);
                                bSuccess = true;
                            }
                        }  // else if (bReplyWasSuccess)

                    }  // else (the trade loaded successfully)
                }      // pItem was found.
            }
            break;

        // These aren't AS simple.
        case OTTransaction::paymentPlan:  // Uses 4 transaction #s: the opener
                                          // (sender's #), which is burned on
            // transaction failure, but kept alive on success,
            // ===> the sender's closing #, which is only marked as "used" upon
            // success (harvestable up until that point.)
            // ===> and the recipient's opening/closing numbers, which are also
            // both only marked as "used" upon success, and are harvestable up
            // until that point.

            {
                Item* pItem = GetItem(Item::paymentPlan);

                if (nullptr == pItem) {
                    otErr << "OTTransaction::HarvestClosingNumbers: Error: "
                             "Unable "
                             "to find "
                             "paymentPlan item in paymentPlan transaction.\n";
                } else  // pItem is good. Let's load up the OTPaymentPlan
                        // object...
                {
                    String strPaymentPlan;
                    OTPaymentPlan thePlan;
                    pItem->GetAttachment(strPaymentPlan);

                    // First load the payment plan up...
                    const bool bLoadContractFromString =
                        (strPaymentPlan.Exists() &&
                         thePlan.LoadContractFromString(strPaymentPlan));

                    // If failed to load the payment plan from string...
                    if (!bLoadContractFromString) {
                        otErr << "OTTransaction::HarvestClosingNumbers: ERROR: "
                                 "Failed loading payment plan from string:\n\n"
                              << strPaymentPlan << "\n\n";
                    } else  // thePlan is ready to go....
                    {
                        // If the server reply message was unambiguously a FAIL,
                        // that means the closing numbers are STILL GOOD.
                        // (Because the transaction therefore never even had a
                        // chance to run.)
                        //
                        if (bReplyWasFailure &&
                            !bHarvestingForRetry)  // on re-try, we need the
                                                   // closing #s to stay put, so
                                                   // the re-try has a chance to
                                                   // work.
                        {
                            thePlan.HarvestClosingNumbers(context);
                            bSuccess = true;
                        }
                        // Else if the server reply message was unambiguously a
                        // SUCCESS, that means the opening number is DEFINITELY
                        // NOT HARVESTABLE. (For the sender, anyway.) Why not?
                        // Because that means the transaction definitely
                        // ran--and the opener is marked as "used" on success,
                        // and "burned" on failure--either way, that's bad for
                        // harvesting (no point.) The recipient, by contrast,
                        // actually retains harvestability on his opening number
                        // up until the very point of transaction success.
                        //
                        // ====> I know you are wondering:
                        // ====>    HOW ABOUT THE CLOSING NUMBERS?  (When
                        // message is success)
                        //  1. Transaction success: Sender and Recipient CANNOT
                        // harvest closing numbers, which are now marked as
                        // "used."
                        //  2. Transaction failed:  Sender and Recipient **CAN**
                        // both harvest their closing numbers.
                        //
                        else if (bReplyWasSuccess) {
                            if (bTransactionWasSuccess) {
                                // This means the "paymentPlan" closing trans#s
                                // are MARKED AS "USED", and will someday be
                                // marked as CLOSED. EITHER WAY, you certainly
                                // can't claw that number back now! (It is still
                                // outstanding, though. It's not gone, yet...)
                                //                          thePlan.HarvestClosingNumbers(theNym);
                                //                          bSuccess = true;
                            } else if (
                                bTransactionWasFailure &&
                                !bHarvestingForRetry)  // on re-try, we need
                                                       // the closing #s to
                                                       // stay put, so the
                                                       // re-try has a chance
                                                       // to work.
                            {
                                // Whereas if the payment plan was a failure,
                                // that means the closing numbers are
                                // harvestable!
                                thePlan.HarvestClosingNumbers(context);
                                bSuccess = true;
                            }
                        }

                    }  // else (the payment plan loaded successfully)
                }      // pItem was found.
            }
            break;

        case OTTransaction::smartContract:  // Uses X transaction #s, with an
                                            // opener for each party and a
                                            // closer for each asset account.
            // If the message is rejected by the server, then ALL openers can be
            // harvested. But if the
            // message was successful (REGARDLESS of whether the transaction was
            // successful) then all of
            // the openers for all of the parties have been burned. The closers,
            // meanwhile, can be recovered
            // if the message is a failure, as well as in cases where message
            // succeeds but transaction failed.
            // But if transaction succeeded, then the closers CANNOT be
            // recovered. (Only removed, once you sign off on the receipt.)
            {

                Item* pItem = GetItem(Item::smartContract);

                if (nullptr == pItem) {
                    otErr
                        << "OTTransaction::HarvestClosingNumbers: Error: "
                           "Unable "
                           "to find "
                           "smartContract item in smartContract transaction.\n";
                } else  // Load up the smart contract...
                {
                    String strSmartContract;
                    OTSmartContract theSmartContract(GetPurportedNotaryID());
                    pItem->GetAttachment(strSmartContract);

                    // If we failed to load the smart contract...
                    if (!strSmartContract.Exists() ||
                        (false == theSmartContract.LoadContractFromString(
                                      strSmartContract))) {
                        otErr << "OTTransaction::HarvestClosingNumbers: Error: "
                                 "Unable to load "
                                 "smartContract object from smartContract "
                                 "transaction item.\n";
                    } else  // theSmartContract is ready to go....
                    {

                        // The message reply itself was a failure. This means
                        // the transaction itself never got a chance to run...
                        // which means ALL the closing numbers on that
                        // transaction are STILL GOOD.
                        //
                        if (bReplyWasFailure &&
                            !bHarvestingForRetry)  // on re-try, we need the
                                                   // closing #s to stay put, so
                                                   // the re-try has a chance to
                                                   // work.
                        {
                            theSmartContract.HarvestClosingNumbers(context);
                            bSuccess = true;
                        }
                        // Else if the server reply message was unambiguously a
                        // SUCCESS, that means the opening number is DEFINITELY
                        // NOT HARVESTABLE. Why? Because that means the
                        // transaction definitely ran--and the opener is marked
                        // as "used" on SUCCESS, or "burned" on FAILURE--either
                        // way, that's bad for harvesting (no point.)
                        //
                        // ===> HOW ABOUT THE CLOSING NUMBERS?
                        // In cases where the message succeeds but the
                        // transaction failed, the closing numbers are
                        // recoverable. (TODO send notice to the parties when
                        // this happens...) But if transaction succeeded, then
                        // the closers CANNOT be recovered. They are now "used"
                        // on the server, so you might as well keep them in that
                        // format on the client side, since that's how the
                        // client has them already.
                        else if (bReplyWasSuccess) {
                            if (bTransactionWasSuccess) {
                                // This means the "smartContract" opening trans#
                                // is MARKED AS "USED", and will someday be
                                // marked as CLOSED. EITHER WAY, you certainly
                                // can't claw that number back now! (It is still
                                // outstanding, though. It's not gone, yet...)
                                //
                                //                          theSmartContract.HarvestClosingNumbers(theNym);
                                //                          bSuccess = true;
                            } else if (
                                bTransactionWasFailure &&
                                !bHarvestingForRetry)  // on re-try, we need
                                                       // the closing #s to
                                                       // stay put, so the
                                                       // re-try has a chance
                                                       // to work.
                            {
                                // If the transaction was a failure, the opening
                                // trans number was burned,
                                // but the CLOSING numbers are still
                                // harvestable...
                                //
                                theSmartContract.HarvestClosingNumbers(context);
                                bSuccess = true;
                            }
                        }  // else if (bReplyWasSuccess)

                    }  // else (smart contract loaded successfully)
                }      // pItem was found.
            }
            break;

        default:
            break;
    }

    return bSuccess;
}

// Client-side
//
// This transaction actually was saved as a balance receipt (filename:
// accountID.success) and now, for whatever reason, I want to verify the receipt
// against the local data (the Nym, the inbox, the outbox, and the account
// balance).
//
// Let's say the Nym has the numbers 9 and 10. He signs a receipt to that
// effect. Until a new receipt is signed, they should STILL be 9 and 10!
// Therefore I should be able to load up the last receipt, pass it the Nym, and
// verify this.
//
// But what if the last receipt is a transaction receipt, instead of a balance
// receipt? Let's say I grab the Nym and he has 9, 10, and 15! And though this
// balance receipt shows 9 and 10, there is a newer one that shows 9, 10, and
// 15? That means even when verifying a balance receipt, I need to also load the
// last transaction receipt and for transaction numbers, use whichever one is
// newer.
//
// When downloading the inbox, the outbox, the account, or the nym, if there is
// a receipt, I should compare what I've downloaded with the last receipt.
// Because if there's a discrepancy, then I don't want to USE that
// inbox/outbox/account/nym to sign a NEW receipt, causing me to sign agreement
// to invalid data!  Instead, I want a red flag to go up, and the receipt
// automatically saved to a disputes folder, etc.
bool OTTransaction::VerifyBalanceReceipt(const ServerContext& context)
{
    // Compare the inbox I just downloaded with what my last signed receipt SAYS
    // it should say. Let's say the inbox has transaction 9 in it -- well, my
    // last signed receipt better show that 9 was in my inbox then, too. But
    // what if 9 was on a cheque, and it only recently hit? Well it won't be in
    // my old inbox, but it WILL still be signed for as anopen transaction.

    // Since this involves verifying the outbox, inbox, AND account, this
    // function should only be called after all three have been downloaded, not
    // after each one. Basically the outbox should RARELY change, the inbox is
    // EXPECTED to change, and the account is EXPECTED to change, BUT ONLY in
    // cases where the inbox justifies it!
    //
    // -- Verify the transaction numbers on the nym match those exactly on the
    //    newest transaction or balance receipt. (this)
    // -- Make sure outbox is the same.
    // -- Loop through all items in the inbox, AND in the inbox according to the
    //    receipt, and total the values of both. I might have 9 and 10 issued in
    //    the last receipt, with #9 in the inbox, showing 50 clams and a balance
    //    of 93. But now I downloaded an inbox showing #9 AND #10, with values
    //    of 50 and 4, and a balance of 89. The new inbox is still valid, and
    //    the new account balance is still valid, because the new number that
    //    appeared was issued to me and signed out, and because the last
    //    receipt's balance of 93 with 50 clams worth of receipts, matches up to
    //    the new account/inbox balance of 89 with 54 clams worth of receipts.
    //    The two totals still match!  That's what we're checking for.
    //
    // Not JUST that actually, but that, if #10 is NOT found in the old one,
    // THEN the amount (4) must bOTTransaction::VerifyBalanceReceipte the
    // DIFFERENCE between the balances (counting all new transactions like #10.)
    // Meaning, the difference between the two balances MUST be made up EXACTLY
    // by the transactions that are found now, that were not previously found,
    // minus the total of the transactions from before that are no longer there,
    // but are also no longer on my issued list and thus don't matter.
    //
    // Wow! OTItem::VerifyBalanceStatement will have useful code but it doesn't
    // encapsulate all this new functionality, since this version must assume
    // differences are there, and STILL verify things by comparing details about
    // those differences, whereas that version only serves to make sure
    // everything still matches.
    //
    // -- Verify nym transactions match. (issued.)
    // -- Verify outbox matches.
    // -- Loop through all items on receipt. If outbox item, should match
    //    exactly.
    // -- But for inbox items, total up: the amount of the total of the items
    //    from the receipt, for all those that would actually change the balance
    //    (chequeReceipt, marketReceipt, paymentReceipt, basketReceipt.) These
    //    should ALL be found in the current version of the inbox. (They can
    //    only be removed by balance agreement, which would update THIS RECEIPT
    //    to remove them...)
    // -- That was the receipt. Now loop through the above inbox items and do
    //    the reverse: for each item in the NEW inbox, add up the total of those
    //    that would change the balance, for receipts found on the new but not
    //    the old, and account for that exactly as a difference in balance.
    /*
     Example.

     -- Oldest signed receipt shows a balance of 115 clams. But then, cheque #78
        hits my inbox and though I haven't yet accepted the receipt, I still
        need to do a transaction, like a 5 clam withdrawal, or whatever, and
        somehow I end up doing a balance agreement.  That results in the below
        signed receipt:
     --- Old receipt shows inbox/account/nym as:
         Currently signed out: 8, 9, 10, and 15
         Balance of 100 clams (Last signed balance before this was for 115 clams
         above)
         Inbox items:
         #78 cheque receipt (#8) for 15 clams. (The missing money is already
         reflected in the above balance. BUT!! #8 must still be signed out for
         this to verify. Here I must sign to acknowledge the receipt is in my
         inbox, but I still have option to accept or dispute the receipt. Until
         then, server keeps it around since it has my signature on it and proves
         the current balance.)
         #82 incoming transfer for 50 clams (A new balance  agreement occurs
         during acceptance of this. And the number doesn't belong to me. So,
         irrelevant here.)
         #10 transfer receipt for some old transfer (does NOT change balance,
         which already happened in the past, BUT!! #10 must still be signed out
         for thisto verify.)

     My nym ISSUED list should not change unless I have a new transaction
     agreement, therefore I expect the list to match every time. My outbox
     should also match. Thus, only my account balance and inbox might change.
     (On the server side, which I'll see when I dl new versions of them and
     compare against my last receipt i.e. this function.) How? NOT via transfer
     receipt, since I would sign a new balance agreement whenever that would
     actually impact my balance. But it could happen with a ***chequeReceipt, a
     paymentReceipt, marketReceipt, or basketReceipt.*** Those mean, my balance
     has changed. In those cases, my account balance WOULD be different, but
     there had better be matching receipts in the inbox!

     --- New inbox/account/nym shows:
     Currently signed out: 8, 9, 10, and 15
     Balance of 89 clams
     Inbox items:
     #78 cheque receipt (#8) for 15 clams.
     #82 incoming transfer for 50 clams (A new balance agreement occurs during
     acceptance. So this type has no affect on balance here.)
     #10 transfer receipt for some old transfer (does NOT change balance, which
     already happened in the past)
     #96 cheque receipt for 7 clams (cheque #9)
     #97 marketReceipt for 4 clams (marketOffer #15)
     #99 incoming transfer for 2000 clams (Accepting it will require a new
     balance agreement.)

     ---------------------------------

     How do I interpret all this data?
     -- Transaction numbers signed out had better match. (If #s issued had
     changed, I would have signed for it already.)

     Next loop through the inbox from the old receipt:
     -- #78, cheque receipt, had better be there in the new inbox, since
     removing it requires a balance agreement, meaning it would already be off
     the receipt that I'm verifying... Since it's here in inbox, should
     therefore also be in the last receipt.
     -- #82, incoming transfer from old receipt, had better be also on the new
     inbox, since I could only accept or reject it with a balance agreement,
     which I'm comparing the inbox to now.
     -- #10 had also better be there in the new inbox for the same reason: if I
     had accepted this transfer receipt, then my last balance receipt would
     reflect that.
     -- THEREFORE: ALL items from old receipt must be found inside new inbox!

     Next, loop through the new version of the inbox:
     -- #78, though found in the new inbox, wouldn't necessarily be expected to
     be found in the last receipt, since someone may have cashed my cheque since
     the receipt was made.
     -- #82, though found in the new inbox, wouldn't necessarily be expected to
     be found in the last receipt, since someone may have sent me the transfer
     since receipt was made.
     -- #10 in new inbox, same thing: Someone may have recently accepted my
     transfer, and thus #10 only would have popped in since the last agreement.
     (It was there before, but I couldn't EXPECT that in every case.)
     -- #96 and #97 represent balance CHANGES totalling -11 clams. They must
     correspond to a change in balance.
     -- #96 is a cheque receipt.. it has changed the balance and I must account
     for that. But #78 is ALSO a cheque receipt, so why am I not accounting for
     ITs total (instead just assuming it's accounted for already in the prior
     balance, like 78?) Because it's NEW and wasn't on the old receipt like 78
     is!
     -- Due to the reasoning explained on the above line, ANY chequeReceipt,
     paymentReceipt, marketReceipt, or basketReceipt
        found on the new version of the inbox but NOT on the old one from the
     receipt, must be accounted for against the balance.
     -- #99 is an incoming transfer, but it will not change the balance until
     that transfer is accepted with a new balance agreement sometime in the
     future.
     */

    if (IsAbbreviated()) {
        otErr << "OTTransaction::VerifyBalanceReceipt: Error: This is an "
                 "abbreviated receipt. (Load the box receipt first.)\n";

        return false;
    }

    const auto& THE_NYM = *context.Nym();
    const auto& SERVER_NYM = context.RemoteNym();
    Identifier NYM_ID(THE_NYM), NOTARY_NYM_ID(SERVER_NYM);
    const String strNotaryID(GetRealNotaryID()), strReceiptID(NYM_ID);

    // Load the last TRANSACTION STATEMENT as well...
    String strFilename;
    strFilename.Format("%s.success", strReceiptID.Get());
    const char* szFolder1name = OTFolders::Receipt().Get();
    const char* szFolder2name = strNotaryID.Get();
    const char* szFilename = strFilename.Get();

    if (!OTDB::Exists(szFolder1name, szFolder2name, szFilename)) {
        otWarn << "Receipt file doesn't exist in "
                  "OTTransaction::VerifyBalanceReceipt:\n "
               << szFilename << "\n";

        return false;
    }

    const std::string strFileContents(
        OTDB::QueryPlainString(szFolder1name, szFolder2name, szFilename));

    if (strFileContents.length() < 2) {
        otErr << "OTTransaction::VerifyBalanceReceipt: Error reading "
                 "transaction statement:\n "
              << szFolder1name << Log::PathSeparator() << szFolder2name
              << Log::PathSeparator() << szFilename << "\n";

        return false;
    }

    String strTransaction(strFileContents.c_str());
    std::unique_ptr<OTTransactionType> pContents(
        OTTransactionType::TransactionFactory(strTransaction));

    if (nullptr == pContents) {
        otErr << "OTTransaction::VerifyBalanceReceipt: Unable to load "
                 "transaction statement:\n "
              << szFolder1name << Log::PathSeparator() << szFolder2name
              << Log::PathSeparator() << szFilename << "\n";

        return false;
    } else if (!pContents->VerifySignature(SERVER_NYM)) {
        otErr << "OTTransaction::VerifyBalanceReceipt: Unable to verify "
                 "signature on transaction statement:\n "
              << szFolder1name << Log::PathSeparator() << szFolder2name
              << Log::PathSeparator() << szFilename << "\n";

        return false;
    }

    // At this point, pContents is successfully loaded and verified, containing
    // the last transaction receipt.
    OTTransaction* pTrans = dynamic_cast<OTTransaction*>(pContents.get());

    if (nullptr == pTrans) {
        otErr << "OTTransaction::VerifyBalanceReceipt: Was expecting an "
                 "OTTransaction to be stored in the transaction statement "
                 "at:\n "
              << szFolder1name << Log::PathSeparator() << szFolder2name
              << Log::PathSeparator() << szFilename << "\n";

        return false;
    }

    OTTransaction& tranOut = *pTrans;

    // I ONLY need this transaction statement if it's newer than the balance
    // statement. Otherwise, I don't use it at all.  But if it's newer, then I
    // use it instead of the current balance statement (only for verifying the
    // list of issued numbers, not for anything else.)
    //
    // And in the case where that happens, I ONLY expect to see new numbers
    // added, NOT removed. But again, ONLY if the transaction statement is MORE
    // RECENT. Otherwise it may have extra numbers alright: ones that were
    // already removed and I don't want to re-sign responsibility for!

    // CHECK IF IT'S NEWER AND SET A POINTER BASED ON THIS.
    // the item from that transaction that actually has the issued list we'll be
    // using.
    Item* pItemWithIssuedList = nullptr;
    Item* pTransactionItem = nullptr;
    const bool newer = tranOut.GetDateSigned() > GetDateSigned();

    if (newer) {
        Item* pResponseTransactionItem =
            tranOut.GetItem(Item::atTransactionStatement);

        if (nullptr == pResponseTransactionItem) {
            otOut << "No atTransactionStatement item found on receipt "
                     "(strange.)\n";

            return false;
        } else if (
            Item::acknowledgement != pResponseTransactionItem->GetStatus()) {
            otOut << "Error: atTransactionStatement found on receipt, but not "
                     "a successful one.\n";

            return false;
        } else if (!pResponseTransactionItem->VerifySignature(SERVER_NYM)) {
            otOut << "Unable to verify signature on atTransactionStatement "
                     "item in OTTransaction::VerifyBalanceReceipt.\n";

            return false;
        }

        String strBalanceItem;
        pResponseTransactionItem->GetReferenceString(strBalanceItem);

        if (!strBalanceItem.Exists()) {
            otOut << "No transactionStatement item found as 'in ref to' string "
                     "on a receipt containing atTransactionStatement item.\n";

            return false;
        }

        pTransactionItem = Item::CreateItemFromString(
            strBalanceItem,
            GetRealNotaryID(),
            pResponseTransactionItem->GetReferenceToNum());

        if (nullptr == pTransactionItem) {
            otOut << "Unable to load transactionStatement item from string "
                     "(from a receipt containing an atTransactionStatement "
                     "item.)\n";

            return false;
        } else if (pTransactionItem->GetType() != Item::transactionStatement) {
            otOut << "Wrong type on pTransactionItem (expected "
                     "OTItem::transactionStatement)\n";

            return false;
        } else if (!pTransactionItem->VerifySignature(THE_NYM)) {
            otOut << "Unable to verify signature on transactionStatement item "
                     "in OTTransaction::VerifyBalanceReceipt.\n";
            return false;
        }

        pItemWithIssuedList = pTransactionItem;
    }

    Account THE_ACCOUNT(NYM_ID, GetRealAccountID(), GetRealNotaryID());

    if (!THE_ACCOUNT.LoadContract() || !THE_ACCOUNT.VerifyAccount(THE_NYM)) {
        otOut << "Failed loading or verifying account for THE_NYM in "
                 "OTTransaction::VerifyBalanceReceipt.\n";

        return false;
    } else if (THE_ACCOUNT.GetPurportedNotaryID() != GetPurportedNotaryID()) {
        otOut << "Account, inbox or outbox server ID fails to match receipt "
                 "server ID.\n";

        return false;
    } else if (THE_ACCOUNT.GetPurportedAccountID() != GetPurportedAccountID()) {
        otOut << "Account ID fails to match receipt account ID.\n";

        return false;
    }

    std::unique_ptr<Ledger> pInbox(THE_ACCOUNT.LoadInbox(THE_NYM));
    std::unique_ptr<Ledger> pOutbox(THE_ACCOUNT.LoadOutbox(THE_NYM));

    if ((!pInbox) || (!pOutbox)) {
        otOut << "Inbox or outbox was nullptr after THE_ACCOUNT.Load in "
                 "OTTransaction::VerifyBalanceReceipt.\n";

        return false;
    }

    Item* pResponseBalanceItem = GetItem(Item::atBalanceStatement);

    if (nullptr == pResponseBalanceItem) {
        otOut << "No atBalanceStatement item found on receipt (strange.)\n";

        return false;
    } else if (Item::acknowledgement != pResponseBalanceItem->GetStatus()) {
        otOut << "Error: atBalanceStatement found on receipt, but not a "
                 "successful one.\n";

        return false;
    } else if (!pResponseBalanceItem->VerifySignature(SERVER_NYM)) {
        otOut << "Unable to verify signature on atBalanceStatement item in "
                 "OTTransaction::VerifyBalanceReceipt.\n";

        return false;
    }

    Item* pBalanceItem = nullptr;
    String strBalanceItem;
    pResponseBalanceItem->GetReferenceString(strBalanceItem);

    if (!strBalanceItem.Exists()) {
        otOut << "No balanceStatement item found as 'in ref to' string on a "
                 "receipt containing atBalanceStatement item.\n";

        return false;
    }

    pBalanceItem = Item::CreateItemFromString(
        strBalanceItem,
        GetRealNotaryID(),
        pResponseBalanceItem->GetReferenceToNum());

    if (nullptr == pBalanceItem) {
        otOut << "Unable to load balanceStatement item from string (from a "
                 "receipt containing an atBalanceStatement item.)\n";

        return false;
    } else if (pBalanceItem->GetType() != Item::balanceStatement) {
        otOut << "Wrong type on pBalanceItem (expected "
                 "OTItem::balanceStatement)\n";

        return false;
    } else if (!pBalanceItem->VerifySignature(THE_NYM)) {
        otOut << "Unable to verify signature on balanceStatement item in "
                 "OTTransaction::VerifyBalanceReceipt.\n";

        return false;
    }

    String serialized;
    const bool useTransactionItem = (nullptr != pTransactionItem) &&
                                    (tranOut.GetDateSigned() > GetDateSigned());

    if (useTransactionItem) {
        pItemWithIssuedList = pTransactionItem;
    } else {
        pItemWithIssuedList = pBalanceItem;
    }

    pItemWithIssuedList->GetAttachment(serialized);

    if (!serialized.Exists()) {
        otOut << "Unable to load message nym in "
                 "OTTransaction::VerifyBalanceReceipt.\n";

        return false;
    }

    TransactionStatement statement(serialized);

    // Finally everything is loaded and verified!
    // I have the Nym and Server Nym
    // I have the account, inbox, and outbox
    // I have the original balance statement, AND the server's reply to it (a
    // successful one)
    //
    // Repeating a note from above:
    // -- Verify nym transactions match. (The issued / signed-for ones.)
    // -- Verify outbox matches.
    // -- Loop through all items on receipt. If outbox item, should match
    //    exactly.
    // -- But for inbox items, total up: the amount of the total of the items
    //    from the receipt, for all those that would actually change the balance
    //    (chequeReceipt, marketReceipt, paymentReceipt.) These should ALL be
    //    found in the current version of the inbox. (They can only be removed
    //    by balance agreement which would update THIS RECEIPT to remove
    //    them...)
    // -- That was the receipt. Now loop through the latest inbox items and do
    //    the reverse: for each item in the NEW inbox, add up the total of those
    //    that would change the balance, for receipts found on the new but not
    //    the old, and account for that exactly as a difference in balance. Also
    //    make sure each receipt in the inbox (new or old) is an issued
    //    transaction number, signed out to THE_NYM.

    // VERIFY THE LIST OF ISSUED (SIGNED FOR) TRANSACTION NUMBERS ON THE NYM
    // AGAINST THE RECEIPT.
    // The Nym should match whatever is on the newest receipt (determined just
    // above.)
    //
    // NOTE: I used to VerifyIssuedNumbersOnNym -- but that won't work. Why?
    // Because let's say I signed a balance agreement with #s 9, 10, and 11.
    // That's my last receipt. Now let's say, using a DIFFERENT ASSET ACCOUNT, I
    // do a withdrawal, burning #9. Now my balance agreement says 10, 11 for
    // that other account, which correctly matches the server.  Now when the
    // FIRST ACCOUNT verifies his (formerly valid) receipt, 9 is missing from
    // his nym, which doesn't match the receipt!  Of course that's because
    // there's a newer balance receipt -- BUT ON A DIFFERENT ASSET ACCOUNT.
    //
    // VerifyTransactionStatement (vs VerifyBalanceStatement, where we are now)
    // gets around this whole problem with
    // VerifyTransactionStatementNumbersOnNym, which only verifies that every
    // issued number found on THE_NYM (client-side) is definitely also found in
    // the receipt (statement). It does NOT do the reverse. In other words,
    // it does NOT make sure that every Trans# on statement (last receipt)
    // is also found on THE_NYM (current client-side nym.) Numbers may have been
    // cleared since that receipt was signed, due to a balance agreement FROM A
    // DIFFERENT ASSET ACCOUNT. This is okay since numbers have not been ADDED
    // to your list of responsibility (which is the danger.) In order to ADD a
    // number to your list, a transaction statement would have to be signed,
    // since new transaction numbers can only be received through the Nymbox.
    // Since this function (VerifyBalanceReceipt) uses the transactionStatement
    // for verifying issued numbers in cases where it is newer than the
    // balanceStatement, then if a new number was added, it will be on the
    // receipt already.

    if (!context.Verify(statement)) {
        otOut << "Unable to verify issued numbers on last signed receipt with "
                 "numbers on THE_NYM in OTTransaction::VerifyBalanceReceipt.\n";

        return false;
    }

    // LOOP THROUGH THE BALANCE STATEMENT ITEMS (INBOX AND OUTBOX) TO GATHER
    // SOME DATA...

    int32_t nInboxItemCount = 0, nOutboxItemCount = 0;
    const char* szInbox = "Inbox";
    const char* szOutbox = "Outbox";
    const char* pszLedgerType = nullptr;
    // For measuring the amount of the total of items in the inbox that have
    // changed the balance (like cheque receipts)
    int64_t lReceiptBalanceChange = 0;

    // Notice here, I'm back to using pBalanceItem instead of
    // pItemWithIssuedList, since this is the inbox/outbox section...
    otWarn << "Number of inbox/outbox items on the balance statement: "
           << pBalanceItem->GetItemCount() << "\n";

    // TODO:  If the balance item shows a FINAL RECEIPT present, then ALL the
    // co-related cron receipts in the ACTUAL INBOX must ALSO be present on the
    // balance item, just as the final receipt is present. IT cannot be there
    // unless THEY are also there!  (The WHOLE PURPOSE of the final receipt is
    // to MAKE SURE that all its related paymentReceipts/marketReceipts have
    // been CLOSED OUT.)

    for (int32_t i = 0; i < pBalanceItem->GetItemCount(); i++) {
        // for outbox calculations. (It's the only case where GetReceiptAmount()
        // is wrong and needs -1 multiplication.)
        int64_t lReceiptAmountMultiplier = 1;
        Item* pSubItem = pBalanceItem->GetItem(i);

        OT_ASSERT(nullptr != pSubItem);

        Ledger* pLedger = nullptr;

        switch (pSubItem->GetType()) {
            // These types of receipts can actually change your balance.
            case Item::chequeReceipt:
            case Item::marketReceipt:
            case Item::paymentReceipt:
            case Item::basketReceipt: {
                lReceiptBalanceChange += pSubItem->GetAmount();
            }  // Intentional fall through
            // These types of receipts do NOT change your balance.
            case Item::transferReceipt:
            case Item::voucherReceipt:
            case Item::finalReceipt: {
                nInboxItemCount++;
                pLedger = pInbox.get();
                pszLedgerType = szInbox;
            }  // Intentional fall through
            case Item::transfer: {
                break;
            }
            default: {
                String strItemType;
                pSubItem->GetTypeString(strItemType);
                otLog3 << "OTTransaction::VerifyBalanceReceipt: Ignoring "
                       << strItemType << " item in balance statement while "
                       << "verifying it against inbox." << std::endl;
            }
                continue;
        }

        switch (pSubItem->GetType()) {
            case Item::transfer: {
                const bool isOutboxItem = pSubItem->GetAmount() < 0;

                if (isOutboxItem) {
                    // transfers out always reduce your balance.
                    lReceiptAmountMultiplier = -1;
                    nOutboxItemCount++;
                    pLedger = pOutbox.get();
                    pszLedgerType = szOutbox;
                } else {
                    // transfers in always increase your balance.
                    lReceiptAmountMultiplier = 1;
                    nInboxItemCount++;
                    pLedger = pInbox.get();
                    pszLedgerType = szInbox;
                }
            } break;
            // These types will have a 0 receipt amount.
            case Item::finalReceipt:
            case Item::transferReceipt:
            case Item::voucherReceipt:
            case Item::chequeReceipt:
            // These types will already be negative or positive based on
            // whichever is appropriate.
            case Item::marketReceipt:
            case Item::paymentReceipt:
            case Item::basketReceipt: {
                lReceiptAmountMultiplier = 1;
            } break;
            default: {
                continue;
            }
        }

        OTTransaction* pTransaction = nullptr;
        int64_t lTempTransactionNum = 0;
        int64_t lTempReferenceToNum = 0;
        int64_t lTempNumberOfOrigin = 0;

        // What's going on here? In the original balance statement, ONLY IN
        // CASES OF OUTOING TRANSFER, the user has put transaction # "1" in his
        // outbox, in anticipation that the server, upon success, will actually
        // put a real pending transfer into his outbox, and issue a number for
        // it (like "34"). Thus it's understood that whenever the
        // balanceStatement has a "1" in the outbox, I should find a
        // corresponding "34" (or whatever # the server chose) as the
        // GetNewOutboxTransNum member on the atBalanceStatement. Now here, when
        // verifying the receipt, this allows me to verify theoutbox request '1'
        // against the actual '34' that resulted.
        if ((pOutbox.get() == pLedger) &&
            (pSubItem->GetTransactionNum() == 1) &&
            (pResponseBalanceItem->GetNewOutboxTransNum() > 0)) {
            lTempTransactionNum = pResponseBalanceItem->GetNewOutboxTransNum();
            pTransaction = pLedger->GetTransaction(lTempTransactionNum);
            otLog3 << "OTTransaction::VerifyBalanceReceipt: (This iteration, "
                      "I'm handling an item listed as '1' in the outbox.)\n";
        } else {
            // THE ABOVE IS THE *UNUSUAL* CASE, WHEREAS THIS IS THE NORMAL CASE:
            //
            // Make sure that the transaction number of each sub-item is found
            // on the appropriate ledger (inbox or outbox).
            lTempTransactionNum = pSubItem->GetTransactionNum();
            pTransaction = pLedger->GetTransaction(lTempTransactionNum);
        }

        if (nullptr != pTransaction) {
            lTempReferenceToNum = pTransaction->GetReferenceToNum();
            lTempNumberOfOrigin = pTransaction->GetRawNumberOfOrigin();
        }

        // In the event that an outbox pending transforms into an inbox
        // transferReceipt, I set this true.
        bool bSwitchedBoxes = false;

        // Let's say I sign a balance receipt showing a 100 clam pending
        // transfer, sitting in my outbox. That means someday when I VERIFY that
        // receipt, the 100 clam pending better still be in that outbox, or
        // verification will fail. BUT WAIT -- if the receipient accepts the
        // transfer, then it will disappear out of my outbox, and show up in my
        // inbox as a transferReceipt. So when I go to verify my balance
        // receipt, I have to expect that any outbox item might be missing, but
        // if that is the case, there had better be a matching transferReceipt
        // in the inbox. (That wouldn't disappear unless I processed my inbox,
        // and signed a new balance agreement to get rid of it, so I know it
        // has to be there in the inbox if the pending wasn't in the outbox. (If
        // the receipt is any good.)
        // Therefore the code has to specifically allow for this case, for
        // outbox items...
        if ((nullptr == pTransaction) && (pOutbox.get() == pLedger)) {
            otLog4 << "OTTransaction::" << __FUNCTION__
                   << ": Outbox pending found as inbox transferReceipt. "
                      "(Normal.)\n";

            // We didn't find the transaction that was expected to be in the
            // outbox. (A pending.) Therefore maybe it is now a transfer receipt
            // in the Inbox. We allow for this case.
            pTransaction =
                pInbox->GetTransferReceipt(pSubItem->GetNumberOfOrigin());

            if (nullptr != pTransaction) {
                lTempTransactionNum = pTransaction->GetTransactionNum();
                lTempNumberOfOrigin = pTransaction->GetRawNumberOfOrigin();

                // re: the code below:
                // lTempReferenceToNum = pSubItem->GetReferenceToNum();
                //
                // If it had been in the outbox, the pending receipt would be
                // "in reference to" my original transfer. But if it's since
                // been processed into a transferReceipt in my inbox, that
                // transferReceipt is now "in reference to" the recipient's
                // acceptPending, and thus is NOT in reference to my original
                // transfer. Thus, when the ref num is compared (a little
                // farther below) to pSubItem->RefNum, it's GUARANTEED to fail.
                // That is why we are using pSubItem->GetReferenceToNum here
                // instead of pTransaction->GetReferenceToNum.
                //
                // Now you might ask, "Then why, below, are we comparing
                // lTempReferenceToNum (containing pSubItem->GetReferenceToNum)
                // to pSubItem->GetReferenceToNum?? Aren't we just comparing
                // it to itself?"
                //
                // The answer is yes, in this specific case, we are. But
                // remember, in most cases, lTempReferenceToNum contains
                // pTransaction->GetReferenceToNum, and so in most cases we are
                // NOT comparing it to itself. ONLY in this specific case where
                // the receipt has changed from an outbox-based pending to an
                // inbox-based transferReceipt. And so here, lTempReferenceToNum
                // is set to pSubItem's version, so that it will not fail the
                // below comparison, which would otherwise succeed properly in
                // all other cases.
                //
                // Next you might ask, "But if we are comparing it to itself in
                // this specific case, sure it will pass the comparison, but
                // then what happens to the security, in this specific case?"
                //
                // The answer is, the very next comparison after that is for the
                // NumberOfOrigin, which is unique and which still must match.
                // (Therefore we are still protected.)
                lTempReferenceToNum = pSubItem->GetReferenceToNum();
                lReceiptAmountMultiplier = 1;
                nInboxItemCount++;
                nOutboxItemCount--;
                pLedger = pInbox.get();
                pszLedgerType = szInbox;
                // We need to know this in one place below.
                bSwitchedBoxes = true;
            }

            /*
             Pending:
             Outbox: 1901, referencing 1884
             Inbox:  1901, referencing 1884

             transfer receipt:
             Trans 1902, referencing 1884 (That's just the display, however.
             Really 1902 refs 781, which refs 1884.)

             The pending in the outbox REFERENCES the right number.

             The transfer receipt includes (ref string) an acceptPending that
             references the right number.

             So for pending in outbox, when failure, get ReferenceNum(), and use
             that to find item in Inbox using GetTransferReceipt().
             */
        }

        // STILL not found??
        if (nullptr == pTransaction) {
            otOut << "OTTransaction::" << __FUNCTION__ << ": Expected "
                  << pszLedgerType << " transaction (" << lTempTransactionNum
                  << ") not found. (Amount " << pSubItem->GetAmount() << ".)\n";

            return false;
        }

        // subItem is from the balance statement, and pTransaction is from the
        // inbox/outbox
        if (pSubItem->GetReferenceToNum() != lTempReferenceToNum) {
            otOut << "OTTransaction::" << __FUNCTION__ << ": " << pszLedgerType
                  << " transaction (" << lTempTransactionNum
                  << ") mismatch Reference Num: "
                  << pSubItem->GetReferenceToNum() << ", expected "
                  << lTempReferenceToNum << "\n";

            return false;
        }

        if (pSubItem->GetRawNumberOfOrigin() != lTempNumberOfOrigin) {
            otOut << "OTTransaction::" << __FUNCTION__ << ": " << pszLedgerType
                  << " transaction (" << lTempTransactionNum
                  << ") mismatch Number of Origin: "
                  << pSubItem->GetRawNumberOfOrigin() << ", expected "
                  << lTempNumberOfOrigin << "\n";

            return false;
        }

        int64_t lTransactionAmount = pTransaction->GetReceiptAmount();
        lTransactionAmount *= lReceiptAmountMultiplier;

        if (pSubItem->GetAmount() != lTransactionAmount) {
            otOut << "OTTransaction::" << __FUNCTION__ << ": " << pszLedgerType
                  << " transaction (" << lTempTransactionNum
                  << ") amounts don't match: Report says "
                  << pSubItem->GetAmount() << ", but expected "
                  << lTransactionAmount
                  << ". Trans recpt amt: " << pTransaction->GetReceiptAmount()
                  << ", (pBalanceItem->GetAmount() == "
                  << pBalanceItem->GetAmount() << ".)\n";

            return false;
        }

        if ((pSubItem->GetType() == Item::transfer) &&
            (((bSwitchedBoxes == true) &&
              (pTransaction->GetType() != OTTransaction::transferReceipt)) ||
             ((pLedger == pOutbox.get()) &&
              (pTransaction->GetType() != OTTransaction::pending)) ||
             ((pLedger == pInbox.get()) &&
              (pTransaction->GetType() != OTTransaction::pending) &&
              (pTransaction->GetType() != OTTransaction::transferReceipt)))) {
            otOut << "OTTransaction::" << __FUNCTION__ << ": " << pszLedgerType
                  << " transaction (" << lTempTransactionNum
                  << ") wrong type. (pending block)\n";

            return false;
        }

        if ((pSubItem->GetType() == Item::chequeReceipt) &&
            (pTransaction->GetType() != OTTransaction::chequeReceipt)) {
            otOut << "OTTransaction::" << __FUNCTION__ << ": " << pszLedgerType
                  << " transaction (" << lTempTransactionNum
                  << ") wrong type. (chequeReceipt block)\n";

            return false;
        }

        if ((pSubItem->GetType() == Item::voucherReceipt) &&
            ((pTransaction->GetType() != OTTransaction::voucherReceipt) ||
             (pSubItem->GetOriginType() != pTransaction->GetOriginType()))) {
            otOut << "OTTransaction::" << __FUNCTION__ << ": " << pszLedgerType
                  << " transaction (" << lTempTransactionNum
                  << ") wrong type or origin type. (voucherReceipt block)\n";

            return false;
        }

        if ((pSubItem->GetType() == Item::marketReceipt) &&
            (pTransaction->GetType() != OTTransaction::marketReceipt)) {
            otOut << "OTTransaction::" << __FUNCTION__ << ": " << pszLedgerType
                  << " transaction (" << lTempTransactionNum
                  << ") wrong type. (marketReceipt block)\n";

            return false;
        }

        if ((pSubItem->GetType() == Item::paymentReceipt) &&
            ((pTransaction->GetType() != OTTransaction::paymentReceipt) ||
             (pSubItem->GetOriginType() != pTransaction->GetOriginType()))) {
            otOut << "OTTransaction::" << __FUNCTION__ << ": " << pszLedgerType
                  << " transaction (" << lTempTransactionNum
                  << ") wrong type or origin type. (paymentReceipt block)\n";

            return false;
        }

        if ((pSubItem->GetType() == Item::transferReceipt) &&
            (pTransaction->GetType() != OTTransaction::transferReceipt)) {
            otOut << "OTTransaction::" << __FUNCTION__ << ": " << pszLedgerType
                  << " transaction (" << lTempTransactionNum
                  << ") wrong type. (transferReceipt block)\n";

            return false;
        }

        if ((pSubItem->GetType() == Item::basketReceipt) &&
            ((pTransaction->GetType() != OTTransaction::basketReceipt) ||
             (pSubItem->GetClosingNum() != pTransaction->GetClosingNum()))) {
            otOut << "OTTransaction::" << __FUNCTION__ << ": " << pszLedgerType
                  << " transaction (" << lTempTransactionNum
                  << ") wrong type or closing num ("
                  << pSubItem->GetClosingNum() << "). (basketReceipt block)\n";

            return false;
        }

        if ((pSubItem->GetType() == Item::finalReceipt) &&
            ((pTransaction->GetType() != OTTransaction::finalReceipt) ||
             (pSubItem->GetClosingNum() != pTransaction->GetClosingNum()) ||
             (pSubItem->GetOriginType() != pTransaction->GetOriginType()))) {
            otOut << "OTTransaction::" << __FUNCTION__ << ": " << pszLedgerType
                  << " transaction (" << lTempTransactionNum
                  << ") wrong type or closing num or origin type ("
                  << pSubItem->GetClosingNum() << "). (finalReceipt block)\n";

            return false;
        }
    }

    // By this point, I have an accurate count of the inbox items, and outbox
    // items, represented by *this receipt. I also know that I found each item
    // from the receipt on the new inbox or outbox (as I should have) But do I
    // have to verify that the items are all signed for. I'll do that below
    // since this list is a subset of that one (supposedly.)

    if (nOutboxItemCount != pOutbox->GetTransactionCount()) {
        otOut << "OTTransaction::" << __FUNCTION__
              << ": Outbox mismatch in expected transaction count.\n"
                 " --- THE_INBOX count: "
              << pInbox->GetTransactionCount()
              << " --- THE_OUTBOX count: " << pOutbox->GetTransactionCount()
              << "\n--- nInboxItemCount: " << nInboxItemCount
              << " --- nOutboxItemCount: " << nOutboxItemCount << "\n\n";

        return false;
    }

    // (Notice I don't check inbox item count here, since that actually CAN
    // change.)

    // LOOP THROUGH LATEST INBOX AND GATHER DATA / VALIDATE AGAINST LAST
    // RECEIPT.

    // Change in the account balance we'd expect, based on TOTAL receipts in the
    // inbox.
    int64_t lInboxBalanceChange = 0;
    // Change in the account balance we'd expect, based on the NEW receipts in
    // the inbox.
    int64_t lInboxSupposedDifference = 0;

    for (int32_t i = 0; i < pInbox->GetTransactionCount(); i++) {
        OTTransaction* pTransaction = pInbox->GetTransactionByIndex(i);

        OT_ASSERT(nullptr != pTransaction);

        switch (pTransaction->GetType()) {
            case OTTransaction::chequeReceipt:
            case OTTransaction::marketReceipt:
            case OTTransaction::paymentReceipt:
            case OTTransaction::basketReceipt:

                lInboxBalanceChange +=
                    pTransaction->GetReceiptAmount();  // Here I total ALL
                                                       // relevant receipts.

            case OTTransaction::finalReceipt:  // finalReceipt has no amount.
            case OTTransaction::pending:       // pending has an amount, but it
                                          // already came out of the account and
                                          // thus isn't figured here.
            case OTTransaction::transferReceipt:  // transferReceipt has an
                                                  // amount, but it already came
                                                  // out of account and thus
                                                  // isn't figured in here.
            case OTTransaction::voucherReceipt:   // voucherReceipt has an
                                                  // amount, but it already came
                                                  // out of account and thus
                                                  // isn't figured in here.
                break;
            default: {
                otLog4 << "OTTransaction::" << __FUNCTION__ << ": Ignoring "
                       << pTransaction->GetTypeString()
                       << " item in inbox while verifying it against balance "
                          "receipt.\n";
            }
                continue;
        }

        // This "for" loop is in the process of iterating the LATEST INBOX...
        // ...For each receipt in that inbox, we try and look up a record of the
        // exact same receipt in
        // the INBOX REPORT (present in the balance agreement from the LAST
        // SIGNED TRANSACTION RECEIPT.)
        //
        // It may or may not be found...

        Item* pSubItem = pBalanceItem->GetItemByTransactionNum(
            pTransaction->GetTransactionNum());

        // The above loop already verified that all items in the receipt's inbox
        // were found in the new inbox.
        //
        // But THIS item, though found in the new inbox, WAS NOT FOUND in the
        // OLD inbox (on the receipt.)
        // That means it needs to be accounted for against the account balance!
        //
        if (nullptr == pSubItem) {
            Item* pFinalReceiptItem = nullptr;
            switch (pTransaction->GetType()) {
                case OTTransaction::marketReceipt:
                case OTTransaction::paymentReceipt:
                    //
                    // New thought: if this transaction is from cron
                    // (paymentReceipt or marketReceipt), AND THIS BEING A NEW
                    // ITEM that IS in the latest inbox (but was NOT there
                    // before, in the receipt), THEN the finalReceipt for THIS
                    // transaction had BETTER NOT BE in the old inbox from my
                    // last receipt!!
                    //
                    // Logic: Because the whole point of the finalReceipt is to
                    // prevent any NEW marketReceipts from popping in,
                    // once it is present! It's like a "red flag" or a "filing
                    // date"
                    // -- once it is triggered, IT IS THE FINAL RECEIPT.
                    // No other receipts can appear that reference the same
                    // transaction number!
                    //
                    // THEREFORE: If the FINAL RECEIPT is ALREADY in my last
                    // signed receipt, then WHY IN HELL are NEW marketReceipts
                    // or paymentReceipts going into the latest inbox ??
                    //
                    // That is why I  verify here that, IF THIS IS A CRON
                    // TRANSACTION (payment, market), then the finalReceipt
                    // should NOT be present in the inbox report from the last
                    // receipt!
                    //

                    pFinalReceiptItem =
                        pBalanceItem->GetFinalReceiptItemByReferenceNum(
                            pTransaction->GetReferenceToNum());

                    // If it was FOUND... (bad)
                    //
                    if (nullptr != pFinalReceiptItem) {
                        otOut << "OTTransaction::" << __FUNCTION__
                              << ": Malicious server? A new cronReceipt has "
                                 "appeared, "
                                 "even though its corresponding \nfinalReceipt "
                                 "was "
                                 "already present in the LAST SIGNED RECEIPT. "
                                 "In reference to: "
                              << pTransaction->GetReferenceToNum() << "\n";
                        return false;
                    }
                // else drop-through, since marketReceipts and paymentReceipts
                // DO affect the balance...

                case OTTransaction::chequeReceipt:  // Every one of these, we
                                                    // have to add up the total
                                                    // and reconcile against the
                                                    // latest balance.
                case OTTransaction::basketReceipt:

                    lInboxSupposedDifference +=
                        pTransaction->GetReceiptAmount();  // Here I only total
                                                           // the NEW receipts
                                                           // (not found in old
                                                           // receipt inbox but
                                                           // found in current
                                                           // inbox.)

                case OTTransaction::finalReceipt:  // This has no value. 0
                                                   // amount.
                case OTTransaction::pending:  // pending has value, why aren't
                                              // we adding it? Because it hasn't
                                              // changed the balance yet.
                case OTTransaction::transferReceipt:  // transferReceipt has an
                                                      // amount, but it already
                                                      // came out of
                // the account and thus isn't figured in here.
                case OTTransaction::voucherReceipt:  // voucherReceipt has an
                                                     // amount, but it already
                                                     // came out of
                    // the account and thus isn't figured in here.
                    break;

                default:
                    break;  // this should never happen due to above switch.
            }
        } else  // If the transaction from the inbox WAS found as an item on the
                // old receipt, let's verify the two against each other...
        {
            // subItem is from the balance statement, and pTransaction is from
            // the inbox
            if (pSubItem->GetReferenceToNum() !=
                pTransaction->GetReferenceToNum()) {
                otOut << "OTTransaction::" << __FUNCTION__
                      << ": Inbox transaction ("
                      << pSubItem->GetTransactionNum()
                      << ") mismatch Reference Num: "
                      << pSubItem->GetReferenceToNum() << ", expected "
                      << pTransaction->GetReferenceToNum() << "\n";
                return false;
            }

            if (pSubItem->GetNumberOfOrigin() !=
                pTransaction->GetNumberOfOrigin()) {
                otOut << "OTTransaction::" << __FUNCTION__
                      << ": Inbox transaction ("
                      << pSubItem->GetTransactionNum()
                      << ") mismatch Reference Num: "
                      << pSubItem->GetNumberOfOrigin() << ", expected "
                      << pTransaction->GetNumberOfOrigin() << "\n";
                return false;
            }

            // We're looping through the inbox here, so no multiplier is needed
            // for the amount
            // (that was only for outbox items.)
            if (pSubItem->GetAmount() != (pTransaction->GetReceiptAmount())) {
                otOut << "OTTransaction::" << __FUNCTION__
                      << ": Inbox transaction ("
                      << pSubItem->GetTransactionNum()
                      << ") "
                         "amounts don't match: "
                      << pSubItem->GetAmount() << ", expected "
                      << pTransaction->GetReceiptAmount()
                      << ". (pBalanceItem->GetAmount() == "
                      << pBalanceItem->GetAmount() << ".)\n";
                return false;
            }

            if ((pSubItem->GetType() == Item::transfer) &&
                (pTransaction->GetType() != OTTransaction::pending)) {
                otOut << "OTTransaction::" << __FUNCTION__
                      << ": Inbox transaction ("
                      << pSubItem->GetTransactionNum()
                      << ") wrong type. (pending block)\n";
                return false;
            }

            if ((pSubItem->GetType() == Item::chequeReceipt) &&
                (pTransaction->GetType() != OTTransaction::chequeReceipt)) {
                otOut << "OTTransaction::" << __FUNCTION__
                      << ": Inbox transaction ("
                      << pSubItem->GetTransactionNum()
                      << ") wrong type. "
                         "(chequeReceipt block)\n";
                return false;
            }

            if ((pSubItem->GetType() == Item::voucherReceipt) &&
                ((pTransaction->GetType() != OTTransaction::voucherReceipt) ||
                 (pSubItem->GetOriginType() !=
                  pTransaction->GetOriginType()))) {
                otOut << "OTTransaction:" << __FUNCTION__
                      << ": Inbox transaction ("
                      << pSubItem->GetTransactionNum()
                      << ") wrong type or origin type. "
                         "(voucherReceipt block)\n";
                return false;
            }

            if ((pSubItem->GetType() == Item::marketReceipt) &&
                (pTransaction->GetType() != OTTransaction::marketReceipt)) {
                otOut << "OTTransaction::" << __FUNCTION__
                      << ": Inbox transaction ("
                      << pSubItem->GetTransactionNum()
                      << ") wrong type. "
                         "(marketReceipt block)\n";
                return false;
            }

            if ((pSubItem->GetType() == Item::paymentReceipt) &&
                ((pTransaction->GetType() != OTTransaction::paymentReceipt) ||
                 (pSubItem->GetOriginType() !=
                  pTransaction->GetOriginType()))) {
                otOut << "OTTransaction::" << __FUNCTION__
                      << ": Inbox transaction ("
                      << pSubItem->GetTransactionNum()
                      << ") wrong type. "
                         "(paymentReceipt block)\n";
                return false;
            }

            if ((pSubItem->GetType() == Item::transferReceipt) &&
                (pTransaction->GetType() != OTTransaction::transferReceipt)) {
                otOut << "OTTransaction::" << __FUNCTION__
                      << ": Inbox transaction ("
                      << pSubItem->GetTransactionNum()
                      << ") wrong type. "
                         "(transferReceipt block)\n";
                return false;
            }

            if ((pSubItem->GetType() == Item::basketReceipt) &&
                ((pTransaction->GetType() != OTTransaction::basketReceipt) ||
                 (pSubItem->GetClosingNum() !=
                  pTransaction->GetClosingNum()))) {
                otOut << "OTTransaction::" << __FUNCTION__
                      << ": Inbox transaction ("
                      << pSubItem->GetTransactionNum()
                      << ") wrong type, "
                         "or mismatched closing num. (basketReceipt block)\n";
                return false;
            }

            if ((pSubItem->GetType() == Item::finalReceipt) &&
                ((pTransaction->GetType() != OTTransaction::finalReceipt) ||
                 (pSubItem->GetClosingNum() != pTransaction->GetClosingNum()) ||
                 (pSubItem->GetOriginType() !=
                  pTransaction->GetOriginType()))) {
                otOut << "OTTransaction::" << __FUNCTION__
                      << ": Inbox transaction ("
                      << pSubItem->GetTransactionNum()
                      << ") wrong type or origin type, "
                         "or mismatched closing num. (finalReceipt block)\n";
                return false;
            }

        }  // else pSubItem WAS found on the old receipt

        // Next I need to find out the transaction number that I ORIGINALLY
        // used, that's somehow associated with the receipt I found in my inbox,
        // by looking up the number from within the receipt...
        String strRespTo;
        // The number that must STILL be signed out to me, in order for this
        // receipt not be warrant disputing.
        TransactionNumber lIssuedNum = 0;
        OTTransaction* pFinalReceiptTransaction = nullptr;

        switch (pTransaction->GetType()) {
            // a transfer receipt is in reference to some guy's acceptPending
            case OTTransaction::transferReceipt:
            case OTTransaction::chequeReceipt:
            case OTTransaction::voucherReceipt: {
                lIssuedNum = pTransaction->GetNumberOfOrigin();
            } break;
            // ANY cron-related receipts should go here...
            case OTTransaction::marketReceipt:
            case OTTransaction::paymentReceipt: {
                // a payment receipt #92 is IN REFERENCE TO my payment plan #13,
                // which I am still signed out for... UNTIL the final receipt
                // appears. Once a final receipt appears that is "in reference
                // to" the same number as a marketReceipt (or paymentReceipt)
                // then the paymentReceipt #92 is now IN REFERENCE TO my payment
                // plan #13, WHICH IS CLOSED FOR NEW PAYMENTS, BUT THE PAYMENT
                // RECEIPT ITSELF IS STILL VALID UNTIL THE "closing transaction
                // num" ON THAT FINAL RECEIPT IS CLOSED.
                //
                // Therefore I first need to see if the final receipt is PRESENT
                // in the inbox, so I can then determine which number should be
                // expected to be found on my ISSUED list of transaction
                // numbers.
                pFinalReceiptTransaction =
                    pInbox->GetFinalReceipt(pTransaction->GetReferenceToNum());
                const bool finalReceiptWasFound =
                    (nullptr != pFinalReceiptTransaction);

                if (finalReceiptWasFound) {
                    lIssuedNum = pFinalReceiptTransaction->GetClosingNum();
                } else {
                    lIssuedNum = pTransaction->GetReferenceToNum();
                }

                // If marketReceipt #15 is IN REFERENCE TO original market offer
                // #10, then the "ISSUED NUM" that is still open on my "signed
                // out" list is #10.
                //
                // UNLESS!! Unless a final receipt is present in reference to
                // this same number, in which case the CLOSING TRANSACTION
                // NUMBER stored on that final receipt will become my ISSUED NUM
                // for the purposes of this code.  (Because the original number
                // IS closed, but the marketReceipt is also still valid until
                // the FINAL RECEIPT is closed.)
            } break;
            // basketReceipt always expects the issued num to be its "closing
            // num". The "reference to" is instead expected to contain the
            // basketExchange ID (Trans# of the original request to exchange,
            // which is already closed.)
            //
            // Final Receipt expects that its "in reference to" is already
            // closed (that's why the final receipt even exists...) Its own
            // GetClosingNum() now contains the only valid possible transaction
            // number for this receipt (and for any related to it in this same
            // inbox, which share the same "in reference to" number...
            case OTTransaction::finalReceipt:
            case OTTransaction::basketReceipt: {
                lIssuedNum = pTransaction->GetClosingNum();
            } break;
            default: {
                continue;
            }
                // Below this point (inside the loop) is ONLY for receipts that
                // somehow represent a transaction number that's still issued /
                // signed out to me.
        }

        // Whether pSubItem is nullptr or not, pTransaction DEFINITELY exists
        // either way, in the newest inbox. Therefore, let's verify whether I'm
        // even responsible for that transaction number... (Just because I
        // signed the instrument at some point in the past does NOT mean that
        // I'm still responsible for the transaction number that's listed on the
        // instrument. Maybe I already used it up a long time ago...)
        const bool missing = (1 != statement.Issued().count(lIssuedNum));

        if (missing) {
            otErr << "OTTransaction::" << __FUNCTION__
                  << ": Error verifying if transaction num in inbox ("
                  << pTransaction->GetTransactionNum()
                  << ") was actually signed out (" << lIssuedNum << ")\n";

            return false;
        }

        // NOTE: the above check to VerifyIssuedNum... in the case of
        // basketReceipts and finalReceipts, lIssuedNum is the CLOSING num (this
        // is already done.) With marketReceipts and paymentReceipts, they check
        // for the existence of a FINAL receipt, and if it's there, they use its
        // CLOSING NUM.  Otherwise they use the "in reference to" num. With
        // final receipts it uses its CLOSING NUM, since the original is
        // presumed closed.
    }

    // BY THIS POINT, I have lReceiptBalanceChange with the total change in the
    // receipt, and lInboxBalanceChange with the total change in the new inbox.
    // The difference between the two is the difference I should expect also in
    // the account balances! That amount should also be equal to
    // lInboxSupposedDifference, which is the total of JUST the inbox receipts
    // that I DIDN'T find in the old receipt (they were ONLY in the new inbox.)
    //
    // I have looped through all inbox items, and I know they were either found
    // in the receipt's inbox record (and verified), or their amounts were added
    // to lInboxSupposedDifference as appropriate.
    //
    // I also verified, for each inbox item, IF IT TAKES MONEY, THEN IT MUST
    // HAVE A TRANSACTION NUMBER SIGNED OUT TO ME... Otherwise I could dispute
    // it. The last code above verifies this.
    //
    // All that's left is to make sure the balance is right...

    // VERIFY ACCOUNT BALANCE (RECONCILING WITH NEW INBOX RECEIPTS)
    // lReceiptBalanceChange    -- The balance of all the inbox items on the
    //                             receipt (at least, the items that change the
    //                             balance.)
    // lInboxBalanceChange      -- The balance of all the inbox items in the
    //                             inbox (at least, the items that change the
    //                             balance.)
    // lInboxSupposedDifference -- The balance of all the inbox items in the
    //                             inbox that were NOT found in the receipt
    //                             (that change balance.)
    // lAbsoluteDifference      -- The absolute difference between the inbox
    //                             balance and the receipt balance. (Always
    //                             positive.)
    // lAbsoluteDifference      -- The balance of all the inbox items
    //                             (including new items) minus the old ones that
    //                             were on the receipt.

    // (Helping me to visualize lInboxBalanceChange and lReceiptBalanceChange)
    //                ACTUAL         SIMPLE ADD/SUBTRACT     SUBTRACT/ABS
    // -5 -100  difference == 95    (-5 - -100 ==   95)         95 *
    // 5   100  difference == 95    ( 5 -  100 ==  -95)         95 *
    // -5  100  difference == 105    (-5 -  100 == -105)        105 *
    // 5  -100  difference == 105   ( 5 - -100 ==  105)        105 *
    // -100 -5  difference == 95    (-100 - -5 ==  -95)         95 *
    // 100  -5  difference == 105    ( 100 - -5 ==  105)        105 *
    // -100  5  difference == 105    (-100 -  5 == -105)        105 *
    // 100   5  difference == 95    ( 100 -  5 ==   95)         95 *
    // Based on the above table, the solution is to subtract one value from the
    // other, and then take the absolute of that to get the actual difference.
    // Then use an 'if' statement to see which was larger, and based on that,
    // calculate whether the balance is what would be expected.
    //
    // -1099                    // -99 (example of 1000 absolute difference)

    // How much money came out? (Or went in, if the chequeReceipt was for
    // an invoice...) 901 -99 (example of 1000 absolute difference)
    const int64_t lAbsoluteDifference =
        std::abs(lInboxBalanceChange - lReceiptBalanceChange);
    const int64_t lNegativeDifference = (lAbsoluteDifference * (-1));

    // The new (current) inbox has a larger overall value than the balance in
    // the old (receipt) inbox. (As shown by subitem.)
    const bool bNewInboxWasBigger =
        ((lInboxBalanceChange > lReceiptBalanceChange) ? true : false);
    const bool bNewInboxWasSmaller =
        ((lInboxBalanceChange < lReceiptBalanceChange) ? true : false);
    int64_t lActualDifference;

    if (bNewInboxWasBigger) {
        lActualDifference = lAbsoluteDifference;
    } else if (bNewInboxWasSmaller) {
        lActualDifference = lNegativeDifference;
    } else {
        lActualDifference = 0;
    }

    // If the actual difference between the two totals is not equal to the
    // supposed difference from adding up just the new receipts, (Which is
    // probably impossible anyway) then return false.
    if (lActualDifference != lInboxSupposedDifference) {
        otErr << "OTTransaction::" << __FUNCTION__ << ": lActualDifference ("
              << lActualDifference
              << ") is not equal to lInboxSupposedDifference ("
              << lInboxSupposedDifference
              << ")\n"
                 "FYI, Inbox balance on old receipt: "
              << lReceiptBalanceChange
              << "  Inbox balance on current inbox: " << lInboxBalanceChange
              << "\n";

        return false;
    }

    // if, according to the two inboxes, they are different (in terms of how
    // they would impact balance), then therefore, they must have impacted my
    // balance. THEREFORE, my old balance MUST be equivalent to the current
    // (apparently new) balance, PLUS OR MINUS THE DIFFERENCE, ACCORDING TO THE
    // DIFFERENCE BETWEEN THE INBOXES. If the actual difference (according to
    // inbox receipts) + actual account balance (according to newest copy of
    // account) is not equal to the last signed balance agreement, then return
    // false.
    //
    // Let's say ActualDifference == 10-3 (prev balance minus current balance)
    // == 7. If that's the case, then 7 + THE_ACCT.Balance should equal 10 again
    // from the last balance statement!
    const bool wrongBalance =
        (pBalanceItem->GetAmount() !=
         (THE_ACCOUNT.GetBalance() + (lActualDifference * (-1))));

    if (wrongBalance) {
        otErr << "OTTransaction::" << __FUNCTION__
              << ": lActualDifference in receipts (" << lActualDifference
              << ") plus current acct balance (" << THE_ACCOUNT.GetBalance()
              << ") is NOT equal to last signed balance ("
              << pBalanceItem->GetAmount() << ")" << std::endl;

        return false;
    }

    return true;
}

// This doesn't actually delete the box receipt, per se.
// Instead, it adds the string "MARKED_FOR_DELETION" to the bottom
// of the file, so the sysadmin can delete later, at his leisure.
//
bool OTTransaction::DeleteBoxReceipt(Ledger& theLedger)
{
    String strFolder1name, strFolder2name, strFolder3name, strFilename;

    if (!SetupBoxReceiptFilename(
            theLedger,
            *this,
            "OTTransaction::DeleteBoxReceipt",
            strFolder1name,
            strFolder2name,
            strFolder3name,
            strFilename))
        return false;  // This already logs -- no need to log twice, here.

    // See if the box receipt exists before trying to save over it...
    //
    if (!OTDB::Exists(
            strFolder1name.Get(),
            strFolder2name.Get(),
            strFolder3name.Get(),
            strFilename.Get())) {
        otInfo
            << __FUNCTION__
            << ": Box receipt already doesn't exist, thus no need to delete: "
               "At location: "
            << strFolder1name << Log::PathSeparator() << strFolder2name
            << Log::PathSeparator() << strFolder3name << Log::PathSeparator()
            << strFilename << "\n";
        return false;
    }

    String strFinal;
    OTASCIIArmor ascTemp;

    if (m_strRawFile.Exists()) {
        ascTemp.SetString(m_strRawFile);

        if (false ==
            ascTemp.WriteArmoredString(strFinal, m_strContractType.Get())) {
            otErr << __FUNCTION__
                  << ": Error deleting (writing over) box receipt (failed "
                     "writing armored string):\n"
                  << strFolder1name << Log::PathSeparator() << strFolder2name
                  << Log::PathSeparator() << strFolder3name
                  << Log::PathSeparator() << strFilename << "\n";
            return false;
        }
    }

    // NOTE: I do the armored string FIRST, THEN add the "marked for deletion"
    // part as I save it. (This way, it's still searchable with grep.)

    //
    // Try to save the deleted box receipt to local storage.
    //
    String strOutput;

    if (m_strRawFile.Exists())
        strOutput.Format(
            "%s\n\n%s\n",
            strFinal.Get(),
            "MARKED_FOR_DELETION");  // todo hardcoded.
    else
        strOutput.Format(
            "%s\n\n%s\n",
            "(Transaction was already empty -- strange.)",
            "MARKED_FOR_DELETION");  // todo hardcoded.

    bool bDeleted = OTDB::StorePlainString(
        strOutput.Get(),
        strFolder1name.Get(),
        strFolder2name.Get(),
        strFolder3name.Get(),
        strFilename.Get());
    if (!bDeleted)
        otErr << __FUNCTION__
              << ": Error deleting (writing over) file: " << strFolder1name
              << Log::PathSeparator() << strFolder2name << Log::PathSeparator()
              << strFolder3name << Log::PathSeparator() << strFilename
              << "\nContents:\n\n"
              << m_strRawFile << "\n\n";

    return bDeleted;
}

bool OTTransaction::SaveBoxReceipt(int64_t lLedgerType)
{

    if (IsAbbreviated()) {
        otOut << __FUNCTION__ << ": Unable to save box receipt "
              << GetTransactionNum()
              << ": "
                 "This transaction is the abbreviated version (box receipt is "
                 "supposed to "
                 "consist of the full version, so we can't save THIS as the "
                 "box receipt.)\n";
        return false;
    }

    String strFolder1name, strFolder2name, strFolder3name, strFilename;

    if (!SetupBoxReceiptFilename(
            lLedgerType,
            *this,
            "OTTransaction::SaveBoxReceipt",
            strFolder1name,
            strFolder2name,
            strFolder3name,
            strFilename))
        return false;  // This already logs -- no need to log twice, here.

    // See if the box receipt exists before trying to save over it...
    //
    if (OTDB::Exists(
            strFolder1name.Get(),
            strFolder2name.Get(),
            strFolder3name.Get(),
            strFilename.Get())) {
        otOut << __FUNCTION__
              << ": Warning -- Box receipt already exists! (Overwriting)"
                 "At location: "
              << strFolder1name << Log::PathSeparator() << strFolder2name
              << Log::PathSeparator() << strFolder3name << Log::PathSeparator()
              << strFilename << "\n";
        //        return false;
    }

    // Try to save the box receipt to local storage.
    //
    String strFinal;
    OTASCIIArmor ascTemp(m_strRawFile);

    if (false ==
        ascTemp.WriteArmoredString(strFinal, m_strContractType.Get())) {
        otErr << __FUNCTION__
              << ": Error saving box receipt (failed writing armored string):\n"
              << strFolder1name << Log::PathSeparator() << strFolder2name
              << Log::PathSeparator() << strFolder3name << Log::PathSeparator()
              << strFilename << "\n";
        return false;
    }

    bool bSaved = OTDB::StorePlainString(
        strFinal.Get(),
        strFolder1name.Get(),
        strFolder2name.Get(),
        strFolder3name.Get(),
        strFilename.Get());

    if (!bSaved)
        otErr << __FUNCTION__ << ": Error writing file: " << strFolder1name
              << Log::PathSeparator() << strFolder2name << Log::PathSeparator()
              << strFolder3name << Log::PathSeparator() << strFilename
              << "\nContents:\n\n"
              << m_strRawFile << "\n\n";

    return bSaved;
}

// This function assumes that theLedger is the owner of this transaction.
// We pass the ledger in so we can determine the proper directory we're
// reading from.
bool OTTransaction::SaveBoxReceipt(Ledger& theLedger)
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
        //  case OTLedger::message:         lLedgerType = 3;    break;
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
            otErr << "OTTransaction::SaveBoxReceipt: Error: unknown box type. "
                     "(This should never happen.)\n";
            return false;
    }
    return SaveBoxReceipt(lLedgerType);
}

bool OTTransaction::VerifyBoxReceipt(OTTransaction& theFullVersion)
{
    if (!m_bIsAbbreviated || theFullVersion.IsAbbreviated()) {
        otErr << "OTTransaction::" << __FUNCTION__
              << ": Failure: This transaction "
                 "isn't abbreviated (val: "
              << (m_bIsAbbreviated ? "IS" : "IS NOT")
              << "), or the purported full version erroneously is (val: "
              << (theFullVersion.IsAbbreviated() ? "IS" : "IS NOT")
              << "). "
                 "Either way, you can't use it in this way, for trans num: "
              << GetTransactionNum() << "\n";
        return false;
    }

    // VERIFY THE HASH
    //
    Identifier idFullVersion;  // Generate a message digest of that string.
    theFullVersion.CalculateContractID(idFullVersion);

    // Abbreviated version (*this) stores a hash of the original full version.
    // Sooo... let's hash the purported "full version" that was passed in, and
    // compare it to the stored one.
    //
    if (m_Hash != idFullVersion) {
        otErr << "OTTransaction::" << __FUNCTION__
              << ": Failure: The purported 'full version' of the transaction, "
                 "passed in for verification fails to match the stored hash "
                 "value for trans num: "
              << GetTransactionNum() << "\n";
        return false;
    }

    // BY THIS POINT, we already know it's a definite match.
    // But we check a few more things, just to be safe.
    // Such as the TRANSACTION NUMBER...
    if (GetTransactionNum() != theFullVersion.GetTransactionNum()) {
        otErr << "OTTransaction::" << __FUNCTION__
              << ": Failure: The purported 'full version' of the transaction "
                 "passed in (number "
              << theFullVersion.GetTransactionNum()
              << ") fails to match the actual transaction number: "
              << GetTransactionNum() << "\n";
        return false;
    }

    // THE "IN REFERENCE TO" NUMBER (DISPLAY VERSION)
    if (GetAbbrevInRefDisplay() != theFullVersion.GetReferenceNumForDisplay()) {
        otErr << "OTTransaction::" << __FUNCTION__
              << ": Failure: The purported 'full version' of the transaction "
                 "passed, GetReferenceNumForDisplay() ("
              << theFullVersion.GetReferenceNumForDisplay()
              << ") fails to match the GetAbbrevInRefDisplay ("
              << GetAbbrevInRefDisplay() << ") on this.\n";
        return false;
    }

    return true;
}

// When the items are first loaded up, VerifyContractID() is called on them.
// Therefore, the notaryID and account ID have already been verified.
// Now I want to go deeper, before actually processing a transaction, and
// make sure that the items on it also have the right owner, as well as that
// owner's signature, and a matching transaction number to boot.
//
bool OTTransaction::VerifyItems(Nym& theNym)
{
    const Identifier NYM_ID(theNym);

    if (NYM_ID != GetNymID()) {
        otErr << "Wrong owner passed to OTTransaction::VerifyItems\n";
        return false;
    }

    // I'm not checking signature on transaction itself since that is already
    // checked before this function is called. But I AM calling Verify Owner,
    // so that when Verify Owner is called in the loop below, it proves the
    // items
    // and the transaction both have the same owner: Nym.

    // if pointer not null, and it's a withdrawal, and it's an acknowledgement
    // (not a rejection or error)
    //
    for (auto& it : GetItemList()) {
        // loop through the ALL items that make up this transaction and check
        // to see if a response to deposit.
        Item* pItem = it;
        OT_ASSERT(nullptr != pItem);

        if (GetTransactionNum() != pItem->GetTransactionNum()) return false;

        if (NYM_ID != pItem->GetNymID()) return false;

        if (!pItem->VerifySignature(theNym))  // NO need to call
                                              // VerifyAccount since
                                              // VerifyContractID is
                                              // ALREADY called and now
                                              // here's
                                              // VerifySignature().
            return false;
    }

    return true;
}

/*
bool                m_bIsAbbreviated;
int64_t                m_lAbbrevAmount;
int64_t                m_lDisplayAmount;
OTIdentifier        m_Hash;            // todo: make this const and force it to
be set during construction.
time64_t                m_DATE_SIGNED;    // The date, in seconds, when the
instrument was last signed.
transactionType        m_Type;            // blank, pending, processInbox,
transfer, deposit, withdrawal, trade, etc.
*/

// private and hopefully not needed
//
OTTransaction::OTTransaction()
    : OTTransactionType()
    , m_DATE_SIGNED(OT_TIME_ZERO)
{
    InitTransaction();
}

// Let's say you never knew their NymID, you just loaded the inbox based on
// AccountID.
// Now you want to add a transaction to that inbox. Just pass the inbox into the
// transaction constructor (below) and it will get the rest of the info it needs
// off of
// the inbox itself (which you presumably just read from a file or socket.)
//
OTTransaction::OTTransaction(const Ledger& theOwner)
    : OTTransactionType(
          theOwner.GetNymID(),
          theOwner.GetPurportedAccountID(),
          theOwner.GetPurportedNotaryID())
    , m_pParent(&theOwner)
    , m_DATE_SIGNED(OT_TIME_ZERO)
{
    InitTransaction();
}

// By calling this function, I'm saying "I know the real account ID and Server
// ID, and here
// they are, and feel free to compare them with whatever YOU load up, which
// we'll leave
// blank for now unless you generate a transaction, or load one up,

// ==> or maybe I might need to add a constructor where another transaction or a
// ledger is passed in.
//      Then it can grab whatever it needs from those. I'm doing something
// similar in OTItem
OTTransaction::OTTransaction(
    const Identifier& theNymID,
    const Identifier& theAccountID,
    const Identifier& theNotaryID,
    originType theOriginType /*=originType::not_applicable*/)
    : OTTransactionType(theNymID, theAccountID, theNotaryID, theOriginType)
    , m_DATE_SIGNED(OT_TIME_ZERO)
{
    InitTransaction();

    //  m_AcctID    = theID; // these must be loaded or generated. NOT set in
    //  constructor, for security reasons. m_NotaryID    = theNotaryID; // There
    //  are only here in ghostly form as a WARNING to you!
}

OTTransaction::OTTransaction(
    const Identifier& theNymID,
    const Identifier& theAccountID,
    const Identifier& theNotaryID,
    int64_t lTransactionNum,
    originType theOriginType /*=originType::not_applicable*/)
    : OTTransactionType(
          theNymID,
          theAccountID,
          theNotaryID,
          lTransactionNum,
          theOriginType)
    , m_DATE_SIGNED(OT_TIME_ZERO)
{
    InitTransaction();

    //  m_lTransactionNum = lTransactionNum; // This is set in the constructor,
    //  as are m_ID and m_NotaryID m_AcctID    = theID; // these must be loaded
    //  or generated. NOT set in constructor, for security reasons. m_NotaryID
    //  = theNotaryID; // There are only here in ghostly form as a WARNING to
    //  you!
}

// all common OTTransaction stuff goes here.
// (I don't like constructor loops, prefer to use a separate function they all
// call.)
void OTTransaction::InitTransaction()
{
    m_strContractType = "TRANSACTION";  // CONTRACT, MESSAGE, TRANSACTION,
                                        // LEDGER, TRANSACTION ITEM
    m_DATE_SIGNED = OT_TIME_ZERO;  // Make sure to set this to the current time
                                   // whenever contract is signed.
    m_Type = OTTransaction::error_state;
    m_lClosingTransactionNo = 0;
    m_lRequestNumber = 0;
    m_bReplyTransSuccess = false;
}

// This CONSTRUCTOR is used for instantiating "abbreviated" transactions,
// each of which separately load their full contents from a separate datafile
// not during loading but during the subsequent verification process.
// See: bool OTTransaction::VerifyItems(OTPseudonym& theNym)
//
OTTransaction::OTTransaction(
    const Identifier& theNymID,
    const Identifier& theAccountID,
    const Identifier& theNotaryID,
    const int64_t& lNumberOfOrigin,
    originType theOriginType,
    const int64_t& lTransactionNum,
    const int64_t& lInRefTo,
    const int64_t& lInRefDisplay,
    time64_t the_DATE_SIGNED,
    OTTransaction::transactionType theType,
    const String& strHash,
    const int64_t& lAdjustment,
    const int64_t& lDisplayValue,
    const int64_t& lClosingNum,
    const int64_t& lRequestNum,
    bool bReplyTransSuccess,
    NumList* pNumList)
    : OTTransactionType(
          theNymID,
          theAccountID,
          theNotaryID,
          lTransactionNum,
          theOriginType)
    , m_bIsAbbreviated(true)
    , m_lAbbrevAmount(lAdjustment)
    , m_lDisplayAmount(lDisplayValue)
    , m_lInRefDisplay(lInRefDisplay)
    , m_Hash(strHash)
    , m_DATE_SIGNED(the_DATE_SIGNED)
    , m_Type(theType)
    , m_lClosingTransactionNo(lClosingNum)
    , m_lRequestNumber(lRequestNum)
    , m_bReplyTransSuccess(bReplyTransSuccess)
{
    InitTransaction();

    // This gets zeroed out in InitTransaction() above. But since we set it in
    // this
    // constructor, I'm setting it back again.
    // Then why call it? I don't know, convention? For the sake of future
    // subclasses?
    //
    m_bIsAbbreviated = true;
    m_DATE_SIGNED = the_DATE_SIGNED;
    m_Type =
        theType;  // This one is same story as date signed. Setting it back.
    m_lClosingTransactionNo = lClosingNum;
    m_lAbbrevAmount = lAdjustment;
    m_lDisplayAmount = lDisplayValue;
    m_lInRefDisplay = lInRefDisplay;

    m_lRequestNumber = lRequestNum;             // for replyNotice
    m_bReplyTransSuccess = bReplyTransSuccess;  // for replyNotice

    m_Hash.SetString(strHash);
    m_lTransactionNum =
        lTransactionNum;  // This is set in OTTransactionType's
                          // constructor, as are m_ID and m_NotaryID

    SetReferenceToNum(lInRefTo);
    SetNumberOfOrigin(lNumberOfOrigin);

    // NOTE: For THIS CONSTRUCTOR ONLY, we DO set the purported AcctID and
    // purported NotaryID.
    // (AFTER the constructor has executed, in OTLedger::ProcessXMLNode();
    //
    // WHY? Normally you set the "real" IDs at construction, and then set the
    // "purported" IDs
    // when loading from string. But this constructor (only this one) is
    // actually used when
    // loading abbreviated receipts as you load their inbox/outbox/nymbox.
    // Abbreviated receipts are not like real transactions, which have notaryID,
    // AcctID, nymID,
    // and signature attached, and the whole thing is base64-encoded and then
    // added to the ledger
    // as part of a list of contained objects. Rather, with abbreviated
    // receipts, there are a series
    // of XML records loaded up as PART OF the ledger itself. None of these
    // individual XML records
    // has its own signature, or its own record of the main IDs -- those are
    // assumed to be on the parent
    // ledger.
    // That's the whole point: abbreviated records don't store redundant info,
    // and don't each have their
    // own signature, because we want them to be as small as possible inside
    // their parent ledger.
    // Therefore I will pass in the parent ledger's "real" IDs at construction,
    // and immediately thereafter
    // set the parent ledger's "purported" IDs onto the abbreviated transaction.
    // That way, VerifyContractID()
    // will still work and do its job properly with these abbreviated records.

    // Note: I'm going to go ahead and set it in here for now. This is a special
    // constructor (abbreviated receipt constructor)
    // todo: come back to this during security sweep.
    //
    SetRealAccountID(theAccountID);
    SetPurportedAccountID(theAccountID);

    SetRealNotaryID(theNotaryID);
    SetPurportedNotaryID(theNotaryID);

    SetNymID(theNymID);

    if (nullptr != pNumList) m_Numlist = *pNumList;
}

// bool GenerateTransaction(const OTIdentifier& theAccountID, const
// OTIdentifier& theNotaryID, int64_t lTransactionNum);
//
// static
// OTTransaction * GenerateTransaction(const OTIdentifier& theNymID, const
// OTIdentifier& theAccountID,
//                                    const OTIdentifier& theNotaryID,
// transactionType theType,
//                                    int64_t lTransactionNum=0);
// static
// OTTransaction * GenerateTransaction(const OTLedger& theOwner,
// transactionType theType, int64_t lTransactionNum=0);

// static
OTTransaction* OTTransaction::GenerateTransaction(
    const Ledger& theOwner,
    OTTransaction::transactionType theType,
    originType theOriginType /*=originType::not_applicable*/,
    int64_t lTransactionNum /*=0*/)
{
    OTTransaction* pTransaction = GenerateTransaction(
        theOwner.GetNymID(),
        theOwner.GetPurportedAccountID(),
        theOwner.GetPurportedNotaryID(),
        theType,
        theOriginType,
        lTransactionNum);
    if (nullptr != pTransaction) pTransaction->SetParent(theOwner);

    return pTransaction;
}

// static
OTTransaction* OTTransaction::GenerateTransaction(
    const Identifier& theNymID,
    const Identifier& theAccountID,
    const Identifier& theNotaryID,
    OTTransaction::transactionType theType,
    originType theOriginType /*=originType::not_applicable*/,
    int64_t lTransactionNum /*=0*/)
{
    OTTransaction* pTransaction = new OTTransaction(
        theNymID, theAccountID, theNotaryID, lTransactionNum, theOriginType);
    OT_ASSERT(nullptr != pTransaction);

    pTransaction->m_Type = theType;

    // Since we're actually generating this transaction, then we can go ahead
    // and set the purported account and server IDs (we have already set the
    // real ones in the constructor). Now both sets are fill with matching data.
    // No need to security check the IDs since we are creating this transaction
    // versus loading and inspecting it.
    pTransaction->SetPurportedAccountID(theAccountID);
    pTransaction->SetPurportedNotaryID(theNotaryID);

    return pTransaction;
}

OTTransaction::~OTTransaction()
{
    while (!m_listItems.empty()) {
        Item* pItem = m_listItems.front();
        m_listItems.pop_front();
        delete pItem;
    }
}

void OTTransaction::Release()
{
    while (!m_listItems.empty()) {
        Item* pItem = m_listItems.front();
        m_listItems.pop_front();
        delete pItem;
    }

    OTTransactionType::Release();
}

// You have to allocate the item on the heap and then pass it in as a reference.
// OTTransaction will take care of it from there and will delete it in
// destructor.
void OTTransaction::AddItem(Item& theItem) { m_listItems.push_back(&theItem); }

// While processing a transaction, you may wish to query it for items of a
// certain type.
Item* OTTransaction::GetItem(Item::itemType theType)
{
    for (auto& it : m_listItems) {
        Item* pItem = it;
        OT_ASSERT(nullptr != pItem);

        if (pItem->GetType() == theType) return pItem;
    }

    return nullptr;
}

// While processing a transaction, you may wish to query it for items in
// reference to a particular transaction number.
//
Item* OTTransaction::GetItemInRefTo(int64_t lReference)
{
    if (GetItemCountInRefTo(lReference) > 1) {
        OT_FAIL_MSG("CAN'T USE GetItemInRefTo! (There are multiple items in "
                    "reference to the same number...) SWITCH to using "
                    "NumberOfOrigin?");
    }

    for (auto& it : m_listItems) {
        Item* pItem = it;
        OT_ASSERT(nullptr != pItem);

        if (pItem->GetReferenceToNum() == lReference) return pItem;
    }

    return nullptr;
}

// Count the number of items that are IN REFERENCE TO some transaction#.
//
// Might want to change this so that it only counts ACCEPTED receipts.
//
int32_t OTTransaction::GetItemCountInRefTo(int64_t lReference)
{
    int32_t nCount = 0;

    for (auto& it : m_listItems) {
        Item* pItem = it;
        OT_ASSERT(nullptr != pItem);

        if (pItem->GetReferenceToNum() == lReference) nCount++;
    }

    return nCount;
}

/*
 a processNymbox transaction has a list of items--each one accepting a nymbox
 receipt (ottransaction) so as to remove it from the nymbox. It also has a
 transaction statement item, which must verify in order for the others to run.

 Here are the types of items:
case OTItem::acceptFinalReceipt:
 theReplyItemType = OTItem::atAcceptFinalReceipt;
 break;
case OTItem::acceptTransaction:
 theReplyItemType = OTItem::atAcceptTransaction;
 break;
case OTItem::acceptMessage:
 theReplyItemType = OTItem::atAcceptMessage;
 break;
case OTItem::acceptNotice:
 theReplyItemType = OTItem::atAcceptNotice;
 break;

 */

//  OTTransaction::GetSuccess()
//
// Tries to determine, based on items within, whether it was a success or fail.
//
// DONE:
//
// What about a processNymbox message? ALL of its items must be
// successful, for any of them to be (for the transaction statement to
// make any sense....)
//
// What I NEED to do , in case of atProcessNymbox and maybe others, is
// loop through them ALL and check them ALL for success.
//
// What it does now is, if it sees an item of a certain type, then it
// IMMEDIATELY returns success or fail based on its status. Imagine this
// problem: My transaction failed (say, due to empty account) but the
// balance statement itself had succeeded before it got to that point.
// The below loop as it exists now will see that the atBalanceStatement
// was successful, and IMMEDAITELY RETURNS TRUE! (Even if the item for
// the transaction itself had failed.)
//
// In the case of processNymbox it's worse, since the ENTIRE TRANSACTION
// must fail, if ANY of its items do. You have to loop them ALL and make
// sure they are ALL success. (regardless of their type.) You can only
// do this if you know *this is a processNymbox transaction, yet we can
// clearly see, that the below code is simply looping the items
// REGARDLESS of what type of transaction *this actually is.
//
// Note: (Below is now fixed.) What if, as above, the processNymbox
// failed, but has a successful transaction statement as one of its
// items? The below loop would return true!
//
// This is actually a pretty good argument for using polymorphism for
// the various transaction and item types, so these sorts of SWITCH
// STATEMENTS aren't necessary all over the transaction and ledger code.
// Although IMO a default implementation should still cover most cases.
//
//
// Tries to determine, based on items within, whether it was a success or fail.
//
bool OTTransaction::GetSuccess(
    bool* pbHasSuccess /*=nullptr*/,  // Just for those who need more detail
    bool* pbIsSuccess /*=nullptr*/)   // and granularity.
{
    // Only used here and there, when more granularity is necessary.
    // (The return value of false doesn't necessarily mean a transaction
    // has failed -- only that one hasn't succeeded. So these provide that
    // detail.)
    //
    bool bHasSuccess = false;
    bool bIsSuccess = false;
    // --------------------------------------
    if (nullptr == pbHasSuccess) pbHasSuccess = &bHasSuccess;
    if (nullptr == pbIsSuccess) pbIsSuccess = &bIsSuccess;
    // --------------------------------------
    // Below this point, the above pointers are always good.
    // (AKA they're not nullptr.)
    //
    *pbHasSuccess = false;
    *pbIsSuccess = false;
    // ---------------------------------------
    bool bFoundAnActionItem = false, bFoundABalanceAgreement = false;

    if ((OTTransaction::atProcessInbox == GetType()) ||
        (OTTransaction::atProcessNymbox == GetType())) {
        for (auto& it : m_listItems) {
            Item* pItem = it;
            OT_ASSERT(nullptr != pItem);

            switch (pItem->GetType()) {

                // BALANCE AGREEMENT  /  TRANSACTION STATEMENT

                // Even if one of these is a success, it is only the balance
                // agreement for the transaction itself, which must also be a
                // success. For example, if there is a transaction for a cash
                // withdrawal, then it will contain 2 items: one item is the
                // withdrawal itself, and the other item is the balance
                // agreement for that withdrawal. Therefore, even if the
                // balanace agreement item is successful, the withdrawal item
                // itself must ALSO be successful, for the transaction AS A
                // WHOLE to be "successful." However, if this balance agreement
                // failed, then the rest of the transaction has definitely
                // failed as well. Therefore, here we either return false, or
                // CONTINUE and let the decision be made from the other items in
                // this transaction otherwise.
                //
                case Item::atBalanceStatement:  // processInbox and
                                                // notarizeTransaction. server's
                                                // reply to a
                // balance statement. One of these items appears inside any
                // transaction reply.
                case Item::atTransactionStatement:  // processNymbox and also
                                                    // for starting/stopping any
                                                    // cron
                    // items. (notarizeTransaction: payment plan, market offer,
                    // smart contract, trigger clause, cancel cron item, etc.)
                    // The server's reply to a transaction statement. Like a
                    // balance statement, except no asset acct is involved.
                    //
                    if (Item::acknowledgement == pItem->GetStatus()) {
                        bFoundABalanceAgreement = true;
                    } else if (Item::rejection == pItem->GetStatus()) {
                        *pbHasSuccess = true;  // We DEFINITELY have a success
                                               // value (of false.) So
                                               // true/false here.
                                               //                  *pbIsSuccess
                                               //                  = false; //
                                               //                  Already set
                                               //                  above.

                        return false;
                    }
                    // else continue...
                    //
                    continue;

                // PROCESS NYMBOX

                // These are only a success if ALL of them (all of the items
                // in this processNymbox transaction) are a success.

                // NOTE: these cases only matter if *this is an atProcessNymbox,
                // and in THAT case, it requires ALL items to verify, not just
                // the first one of a specific type.
                //
                //
                case Item::atAcceptTransaction:  // processNymbox. server's
                                                 // reply to the Nym's attempt
                                                 // to accept (sign for)
                                                 // transaction
                // numbers that are sitting in his nymbox (since he requested
                // more numbers....)
                case Item::atAcceptMessage:  // processNymbox. server's reply to
                                             // nym's accepting a message (from
                                             // another nym) that's in his
                                             // nymbox.
                case Item::atAcceptNotice:   // processNymbox. server's reply to
                                            // nym's accepting a notice from the
                                            // server, such as a successNotice
                                            // (success signing out new
                                            // transaction numbers) or a
                                            // replyNotice, or an
                                            // instrumentNotice. For example,
                                            // some server replies are dropped
                                            // into your Nymbox, to make sure
                                            // you receive them. Then you accept
                                            // them, to remove from your Nymbox.

                // PROCESS NYMBOX *and* PROCESS INBOX

                // These are only a success if ALL of them (all of the items
                // in this processInbox or processNymbox transaction) are a
                // success.

                case Item::
                    atAcceptFinalReceipt:  // Part of a processInbox or
                                           // processNymbox transaction
                                           // reply from the server.
                                           //          case
                                           //          Item::atDisputeFinalReceipt:
                                           //          // Would be in
                                           //          processNymbox AND
                                           // processInbox. Can these be
                // disputed? Think through the process. Todo.

                // PROCESS INBOX

                // These are only a success if ALL of them (all of the items
                // in this processInbox transaction) are a success.

                case Item::atAcceptPending:  // processInbox. server's reply to
                                             // nym's request to accept an
                                             // incoming pending transfer that's
                                             // sitting in his asset acct's
                                             // inbox.
                //          case Item::atRejectPending: // processInbox. Same
                //          thing, except
                // rejecting that pending transfer
                // instead of accepting it.

                case Item::
                    atAcceptCronReceipt:  // processInbox. Accepting a
                                          // payment receipt or market
                                          // receipt. (Smart contracts
                                          // also drop payment receipts,
                                          // currently.)
                                          //          case
                                          //          Item::atDisputeCronReceipt:
                                          //          // processInbox. Dispute.
                                          //          (Todo.)

                case Item::
                    atAcceptItemReceipt:  // processInbox. Accepting a
                                          // transferReceipt, or
                                          // chequeReceipt, etc.
                                          //          case
                                          //          Item::atDisputeItemReceipt:
                                          //          // processInbox. Dispute.
                                          //          (Todo.)

                case Item::atAcceptBasketReceipt:  // processInbox. Basket
                                                   // Receipt, from a basket
                                                   // currency exchange (in or
                                                   // out.)
                    //          case Item::atDisputeBasketReceipt: //
                    //          processInbox. dispute basket receipt.

                    // If we found at least one of these, and nothing fails by
                    // the end of the loop, then for processNymbox and
                    // processInbox, it's a success. (If balance agreement
                    // also...)

                    if (Item::acknowledgement == pItem->GetStatus()) {
                        bFoundAnActionItem = true;
                    } else if (Item::rejection == pItem->GetStatus()) {
                        *pbHasSuccess = true;  // We DEFINITELY have a success
                                               // value (of false.) So
                                               // true/false here.
                                               //                  *pbIsSuccess
                                               //                  = false; //
                                               //                  Already set
                                               //                  above.

                        return false;
                    }
                    // else continue...
                    //
                    continue;

                default:
                    otErr << "Wrong transaction type passed to "
                             "OTTransaction::GetSuccess() "
                             "for processNymbox or processInbox transaction.\n";
                    return false;
            }  // switch
        }

        const bool bReturnValue =
            (bFoundABalanceAgreement && bFoundAnActionItem);

        // If we didn't even have a balance agreement or action item, then we
        // couldn't say for sure whether or not it was a "success". (We just
        // don't know.)
        *pbHasSuccess = bReturnValue;

        // In the above switch, if an Item::rejection was found, then we KNOW we
        // found the success status, and we KNOW it failed. (And we returned
        // already.) Whereas here, we only know whether we have the success
        // status if *pbHasSuccess is true. And IF it is, then we KNOW the item
        // is acknowledged, since we would already have returned above if it had
        // been rejected.
        if (*pbHasSuccess)
            *pbIsSuccess =
                true;  // If it were false we would already have returned above.

        return bReturnValue;

    }  // if processNymbox or processInbox.
    // --------------------------------------------------------------
    // Okay, it's not a processNymbox or processInbox.
    //
    // Maybe it's one of these other transaction types...
    for (auto& it : m_listItems) {
        Item* pItem = it;
        OT_ASSERT(nullptr != pItem);

        switch (pItem->GetType()) {
            //      case OTItem::atServerfee:  // Fees currently aren't coded.
            //      Todo. case OTItem::atIssuerfee:  // Same as above. Todo.

            // BALANCE AGREEMENT  /  TRANSACTION STATEMENT

            // Even if one of these is a success, it is only the balance
            // agreement for the transaction itself, which must also be a
            // success. For example, if there is a transaction for a cash
            // withdrawal, then it will contain 2 items: one item is the
            // withdrawal itself, and the other item is the balance agreement
            // for that withdrawal. Therefore, even if the balanace agreement
            // item is successful, the withdrawal item itself must ALSO be
            // successful, for the transaction AS A WHOLE to be "successful."
            // However, if this balance agreement failed, then the rest of the
            // transaction has definitely failed as well. Therefore, here we
            // either return false, or CONTINUE and let the decision be made
            // from the other items in this transaction otherwise.
            //
            case Item::atBalanceStatement:  // processInbox and
                                            // notarizeTransaction. server's
                                            // reply to a balance statement. One
                                            // of these items appears inside any
                                            // transaction reply.
            case Item::atTransactionStatement:  // processNymbox and also for
                                                // starting/stopping any cron
                                                // items. (notarizeTransaction:
                                                // payment plan, market offer,
                                                // smart contract, trigger
                                                // clause, cancel cron item,
                                                // etc.) The server's reply to a
                                                // transaction statement. Like a
                                                // balance statement, except no
                                                // asset acct is involved.

                if (Item::acknowledgement == pItem->GetStatus()) {
                    bFoundABalanceAgreement = true;
                }
                if (Item::rejection == pItem->GetStatus()) {
                    *pbHasSuccess = true;  // We DEFINITELY have a success value
                                           // (of false.) So true/false here.
                    //              *pbIsSuccess  = false; // Already set above.

                    return false;
                }

                if (bFoundAnActionItem)
                    break;
                else
                    continue;
            // -------------------------------------------------
            /*
             atProcessNymbox,   // process nymbox reply   // comes from server
             atProcessInbox,    // process inbox reply    // comes from server

             // Note: the above two transaction types are handled in the switch
             block above this one.
             // Whereas the below transaction types are handled right here in
             this switch block.

             atTransfer,        // reply from the server regarding a transfer
             request atDeposit,         // reply from the server regarding a
             deposit request atWithdrawal,      // reply from the server
             regarding a withdrawal request atMarketOffer,     // reply from the
             server regarding a market offer atPaymentPlan,     // reply from
             the server regarding a payment plan atSmartContract,   // reply
             from the server regarding a smart contract atCancelCronItem,  //
             reply from the server regarding said cancellation.
             atExchangeBasket,  // reply from the server regarding said
             exchange.
             */

            // NOTARIZE TRANSACTION
            // If any of these are a success, then the transaction as a whole is
            // a success also. (But that's still predicated on a successful
            // balance agreement also being present.)

            case Item::atTransfer:  // notarizeTransaction. server's reply to
                                    // nym's request to initiate a transfer

            case Item::atWithdrawal:  // notarizeTransaction. server's reply to
                                      // withdrawal (cash) request.
            case Item::atDeposit:     // notarizeTransaction. server's reply to
                                      // deposit (cash) request.
            case Item::atWithdrawVoucher:  // notarizeTransaction. server's
                                           // reply to "withdraw voucher"
                                           // request.
            case Item::atDepositCheque:  // notarizeTransaction. server's reply
                                         // to "deposit cheque" request.
            case Item::atPayDividend:  // notarizeTransaction. server's reply to
                                       // "pay dividend" request.
            case Item::atMarketOffer:  // notarizeTransaction. server's reply to
                                       // request to place a market offer.
            case Item::atPaymentPlan:  // notarizeTransaction. server's reply to
                                       // request to activate a payment plan.
            case Item::atSmartContract:  // notarizeTransaction. server's reply
                                         // to request to activate a smart
                                         // contract.

            case Item::atCancelCronItem:  // notarizeTransaction. server's reply
                                          // to request to cancel a [ market
                                          // offer | payment plan | smart
                                          // contract ]
            case Item::atExchangeBasket:  // notarizeTransaction. server's reply
                                          // to request to exchange in or out of
                                          // a basket currency.
                if (Item::acknowledgement == pItem->GetStatus()) {
                    bFoundAnActionItem = true;
                } else if (Item::rejection == pItem->GetStatus()) {
                    *pbHasSuccess = true;  // We DEFINITELY have a success value
                                           // (of false.) So true/false here.
                    //              *pbIsSuccess  = false; // Already set above.

                    return false;
                }

                if (bFoundABalanceAgreement)
                    break;
                else
                    continue;
            // -------------------------------------------------
            // RECEIPTS

            // 1. ACTUAL RECEIPTS (item attached to similar transaction), and
            // also 2. INBOX REPORT ITEMS (sub-item to ANOTHER item, which is
            // used on
            //    Balance Agreements and Transaction Statements.)
            //
            // In case of (1), GetSuccess() is relevant.
            // But in case of (2) GetSuccess() is NOT relevant. FYI.
            //
            // Anyway, if a marketReceipt item is attached to a marketReceipt
            // transaction, then we can return success or failure right away,
            // since such status is set on the item, not the transaction, and
            // since there are no other items that matter if this IS a success.

            //      case OTItem::chequeReceipt:   // not needed in OTItem.
            // Meaning, actual OTTransaction cheque receipts do NOT need a
            // chequeReceipt Item attached....
            case Item::chequeReceipt:  // ...but it's here anyway as a type, for
                                       // dual use reasons (balance agreement
                                       // sub-items. Like for an inbox report.)
            case Item::voucherReceipt:
            case Item::marketReceipt:    // Used for actual market receipts, as
                                         // well as for balance agreement
                                         // sub-items.
            case Item::paymentReceipt:   // Used for actual payment receipts, as
                                         // well as for balance agreement
                                         // sub-items.
            case Item::transferReceipt:  // Used for actual transfer receipts,
                                         // as well as for balance agreement
                                         // sub-items. (Hmm does balance
                                         // agreement need this?)
            case Item::finalReceipt:  // Used for actual final receipt (I think)
                                      // as well as for balance agreement sub
                                      // item (I think.)
            case Item::basketReceipt:  // Used for basket receipt (I think) as
                                       // well as for balance agreement sub-item
                                       // (I think.)

                if (Item::acknowledgement == pItem->GetStatus()) {
                    *pbHasSuccess = true;  // We DEFINITELY have a success
                                           // value. (HasSuccess = true)
                    *pbIsSuccess = true;  // ...and that value is acknowledgment
                                          // (IsSuccess  = true as well)
                } else if (Item::rejection == pItem->GetStatus()) {
                    *pbHasSuccess = true;  // We DEFINITELY have a success
                                           // value. (Of false.) So true/false.
                    //              *pbIsSuccess  = false; // Already set above.
                }

                // False regardless, because this isn't a real transaction, only
                // an inbox receipt. (That's why you have to check the two
                // output parameters if you want this info.)
                return false;
            // -------------------------------------------------
            // Used to inform Nyms about success/failure status of
            // the activation of payment plans and smart contracts.
            //
            case Item::notice:

                if (Item::acknowledgement == pItem->GetStatus()) {
                    *pbHasSuccess = true;
                    *pbIsSuccess = true;
                } else if (Item::rejection == pItem->GetStatus()) {
                    *pbHasSuccess = true;
                    //              *pbIsSuccess  = false; // Already set above.
                }

                // False regardless, because this isn't a real transaction, only
                // a notice. (That's why you have to check the two output
                // parameters if you want this info.)
                return false;
            // -----------------------------------------------
            default:
                otErr << "Wrong transaction type passed to "
                         "OTTransaction::GetSuccess()\n";
                return false;
        }
    }

    const bool bReturnValue = (bFoundABalanceAgreement && bFoundAnActionItem);

    // If we didn't even have a balance agreement or action item, then we
    // couldn't say for sure whether or not it was a "success". (We just don't
    // know.)
    *pbHasSuccess = bReturnValue;

    // In the above switch, if an Item::rejection was found, then we KNOW we
    // found the success status, and we KNOW it failed. (And we returned
    // already.) Whereas here, we only know whether we have the success status
    // if *pbHasSuccess is true. And IF it is, then we KNOW the item is
    // acknowledged, since we would already have returned above if it had been
    // rejected.
    if (*pbHasSuccess)
        *pbIsSuccess =
            true;  // If it were false we would already have returned above.

    return bReturnValue;
}

OTTransaction::transactionType OTTransaction::GetType() const { return m_Type; }

void OTTransaction::SetType(OTTransaction::transactionType theType)
{
    m_Type = theType;
}

const char* OTTransaction::GetTypeString() const
{
    return GetTransactionTypeString(static_cast<int>(m_Type));
}

// return -1 if error, 0 if nothing, and 1 if the node was processed.
int32_t OTTransaction::ProcessXMLNode(irr::io::IrrXMLReader*& xml)
{
    const String strNodeName = xml->getNodeName();

    NumList* pNumList = nullptr;
    if (strNodeName.Compare("nymboxRecord")) {
        pNumList = &m_Numlist;
    }

    if (strNodeName.Compare("nymboxRecord") ||
        strNodeName.Compare("inboxRecord") ||
        strNodeName.Compare("outboxRecord") ||
        strNodeName.Compare("paymentInboxRecord") ||
        strNodeName.Compare("recordBoxRecord") ||
        strNodeName.Compare("expiredBoxRecord")) {
        int64_t lNumberOfOrigin = 0;
        int theOriginType =
            static_cast<int>(originType::not_applicable);  // default
        int64_t lTransactionNum = 0;
        int64_t lInRefTo = 0;
        int64_t lInRefDisplay = 0;

        time64_t the_DATE_SIGNED = OT_TIME_ZERO;
        int theType = OTTransaction::error_state;  // default
        String strHash;

        int64_t lAdjustment = 0;
        int64_t lDisplayValue = 0;
        int64_t lClosingNum = 0;
        int64_t lRequestNumber = 0;
        bool bReplyTransSuccess = false;

        int32_t nAbbrevRetVal = LoadAbbreviatedRecord(
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
            lRequestNumber,
            bReplyTransSuccess,
            pNumList);

        if ((-1) == nAbbrevRetVal)
            return (-1);  // The function already logs appropriately.

        m_bIsAbbreviated = true;

        SetNumberOfOrigin(lNumberOfOrigin);
        SetOriginType(static_cast<originType>(theOriginType));
        SetTransactionNum(lTransactionNum);
        SetReferenceToNum(lInRefTo);
        SetClosingNum(lClosingNum);
        SetRequestNum(lRequestNumber);

        SetReplyTransSuccess(bReplyTransSuccess);

        m_lInRefDisplay = lInRefDisplay;
        m_lAbbrevAmount = lAdjustment;
        m_lDisplayAmount = lDisplayValue;
        m_DATE_SIGNED = the_DATE_SIGNED;
        m_Type = static_cast<OTTransaction::transactionType>(theType);

        if (strHash.Exists())
            m_Hash.SetString(strHash);
        else
            otErr << "OTTransaction::ProcessXMLNode: Missing receiptHash on "
                     "abbreviated record.\n";

        return 1;
    }

    // THIS PART is probably what you're looking for.
    else if (strNodeName.Compare("transaction"))  // Todo:  notice how this
                                                  // "else if" uses
                                                  // OTString::Compare, where
                                                  // most other ProcessXMLNode
                                                  // functions in OT use
                                                  // !strcmp()? (That's right:
                                                  // Buffer overflow. Need to
                                                  // fix elsewhere as it is
                                                  // fixed here.)
    {

        const String strType = xml->getAttributeValue("type");

        if (strType.Exists())
            m_Type = OTTransaction::GetTypeFromString(strType);
        else {
            otOut << "OTTransaction::ProcessXMLNode: Failure: unknown "
                     "transaction type: "
                  << strType << " \n";
            return (-1);
        }

        String strCancelled = xml->getAttributeValue("cancelled");
        if (strCancelled.Exists() && strCancelled.Compare("true"))
            m_bCancelled = true;
        else
            m_bCancelled = false;

        String strDateSigned = xml->getAttributeValue("dateSigned");
        const int64_t lDateSigned =
            strDateSigned.Exists() ? parseTimestamp(strDateSigned.Get()) : 0;
        m_DATE_SIGNED =
            OTTimeGetTimeFromSeconds(lDateSigned);  // Todo casting ?

        const String strAcctID = xml->getAttributeValue("accountID");
        const String strNotaryID = xml->getAttributeValue("notaryID");
        const String strNymID = xml->getAttributeValue("nymID");

        if (!strAcctID.Exists() || !strNotaryID.Exists() ||
            !strNymID.Exists()) {
            otOut
                << "OTTransaction::ProcessXMLNode: Failure: missing strAcctID ("
                << strAcctID << ") or strNotaryID (" << strNotaryID
                << ") or strNymID (" << strNymID << "). \n";
            return (-1);
        }

        const String strOrigin = xml->getAttributeValue("numberOfOrigin");
        const String strOriginType = xml->getAttributeValue("originType");
        const String strTransNum = xml->getAttributeValue("transactionNum");
        const String strInRefTo = xml->getAttributeValue("inReferenceTo");

        if (!strTransNum.Exists() || !strInRefTo.Exists()) {
            otOut << "OTTransaction::ProcessXMLNode: Failure: missing "
                     "strTransNum ("
                  << strTransNum << ") or strInRefTo (" << strInRefTo
                  << "). \n";
            return (-1);
        }

        // a replyNotice (a copy of the server's reply to one of my messages)
        // is often dropped into my Nymbox, to make sure I see it. Usually these
        // have a REQUEST NUMBER on them, so I can quickly tell WHICH MESSAGE
        // it is in reply to.
        //
        if (OTTransaction::replyNotice == m_Type) {
            const String strRequestNum =
                xml->getAttributeValue("requestNumber");

            if (strRequestNum.Exists())
                m_lRequestNumber = strRequestNum.ToLong();

            const String strTransSuccess =
                xml->getAttributeValue("transSuccess");

            m_bReplyTransSuccess = strTransSuccess.Compare("true");
        }

        if ((OTTransaction::blank == m_Type) ||
            (OTTransaction::successNotice == m_Type)) {
            const String strTotalList =
                xml->getAttributeValue("totalListOfNumbers");
            m_Numlist.Release();

            if (strTotalList.Exists())
                m_Numlist.Add(
                    strTotalList);  // (Comma-separated list of numbers
                                    // now becomes std::set<int64_t>.)
        }

        Identifier ACCOUNT_ID(strAcctID), NOTARY_ID(strNotaryID),
            NYM_ID(strNymID);

        SetPurportedAccountID(
            ACCOUNT_ID);  // GetPurportedAccountID() const { return m_AcctID; }
        SetPurportedNotaryID(NOTARY_ID);  // GetPurportedNotaryID() const {
                                          // return m_AcctNotaryID; }
        SetNymID(NYM_ID);

        //  m_bLoadSecurely defaults to true.
        // Normally the RealAccountID and RealNotaryID are set from above,
        // before
        // loading. That way, I can compare them to whatever is actually loaded.
        // (So people don't swap files on us!!)
        // But if the coder SPECIALLY sets  m_bLoadSecurely to FALSE, that means
        // he
        // honestly doesn't know those IDs, and he is loading the file, and he
        // wants it
        // to load up properly AS IF THE IDs IN THE FILE WERE CORRECT. He only
        // does this
        // because it's the only way to get the file loaded without knowing
        // those IDs in
        // advance, and because he takes care, when doing this, to check them
        // after the fact
        // and see if they are, indeed, the ones he was expecting.
        //
        // This mechanism was ONLY FINALLY ADDED to get the class factory
        // working properly.
        // And even in this case, it is still INTERNALLY CONSISTENT. (The
        // sub-items will still
        // be expected to be correct with their parent items.)
        //
        if (!m_bLoadSecurely) {
            SetRealAccountID(ACCOUNT_ID);
            SetRealNotaryID(NOTARY_ID);
        }

        if (strOrigin.Exists()) SetNumberOfOrigin(strOrigin.ToLong());
        if (strOriginType.Exists())
            SetOriginType(
                OTTransactionType::GetOriginTypeFromString(strOriginType));

        SetTransactionNum(strTransNum.ToLong());
        SetReferenceToNum(strInRefTo.ToLong());

        otLog4 << "Loaded transaction " << GetTransactionNum()
               << ", in reference to: " << GetReferenceToNum()
               << " type: " << strType << "\n";

        return 1;
    } else if (!strcmp("closingTransactionNumber", xml->getNodeName())) {
        String strClosingNumber = xml->getAttributeValue("value");

        if (strClosingNumber.Exists() &&
            ((OTTransaction::finalReceipt == m_Type) ||
             (OTTransaction::basketReceipt == m_Type))) {
            m_lClosingTransactionNo = strClosingNumber.ToLong();
        } else {
            otErr << "Error in OTTransaction::ProcessXMLNode: "
                     "closingTransactionNumber field without value, or in "
                     "wrong transaction type.\n";
            return (-1);  // error condition
        }

        return 1;
    } else if (!strcmp("cancelRequest", xml->getNodeName())) {
        if (false ==
            Contract::LoadEncodedTextField(xml, m_ascCancellationRequest)) {
            otErr << "Error in OTTransaction::ProcessXMLNode: cancelRequest "
                     "field without value.\n";
            return (-1);  // error condition
        }

        return 1;
    } else if (!strcmp("inReferenceTo", xml->getNodeName())) {
        if (false == Contract::LoadEncodedTextField(xml, m_ascInReferenceTo)) {
            otErr << "Error in OTTransaction::ProcessXMLNode: inReferenceTo "
                     "field without value.\n";
            return (-1);  // error condition
        }

        return 1;
    } else if (!strcmp("item", xml->getNodeName())) {
        String strData;

        if (!Contract::LoadEncodedTextField(xml, strData) ||
            !strData.Exists()) {
            otErr << "Error in OTTransaction::ProcessXMLNode: transaction item "
                     "field without value.\n";
            return (-1);  // error condition
        } else {
            Item* pItem = new Item(GetNymID(), *this);
            OT_ASSERT(nullptr != pItem);

            if (!m_bLoadSecurely) pItem->SetLoadInsecure();

            // If we're able to successfully base64-decode the string and load
            // it up as
            // a transaction, then add it to the ledger's list of transactions
            if (!pItem->LoadContractFromString(strData)) {
                otErr << "ERROR: OTTransaction failed loading item from "
                         "string: \n\n"
                      << (strData.Exists() ? strData.Get() : "") << "\n\n";
                delete pItem;
                pItem = nullptr;
                return (-1);
            } else if (!pItem->VerifyContractID()) {
                otErr << "ERROR: Failed verifying transaction Item in "
                         "OTTransaction::ProcessXMLNode: \n\n"
                      << strData << "\n\n";
                delete pItem;
                pItem = nullptr;
                return (-1);
            } else {
                m_listItems.push_back(pItem);
                //                otLog5 << "Loaded transaction Item and adding
                // to m_listItems in OTTransaction\n");
            }
        }

        return 1;
    }

    return 0;
}

// For "OTTransaction::blank" and "OTTransaction::successNotice"
// which periodically have more numbers added to them.
//
bool OTTransaction::AddNumbersToTransaction(const NumList& theAddition)
{
    return m_Numlist.Add(theAddition);
}

// This is called automatically by SignContract to make sure what's being signed
// is the most up-to-date
// Before transmission or serialization, this is where the ledger saves its
// contents
// So let's make sure this transaction has the right contents.
//
void OTTransaction::UpdateContents()
{
    const char* pTypeStr = GetTypeString();  // TYPE
    const String strType((nullptr != pTypeStr) ? pTypeStr : "error_state"),
        strAcctID(GetPurportedAccountID()), strNotaryID(GetPurportedNotaryID()),
        strNymID(GetNymID());

    // I release this because I'm about to repopulate it.
    m_xmlUnsigned.Release();

    Tag tag("transaction");

    tag.add_attribute("type", strType.Get());
    tag.add_attribute("dateSigned", getTimestamp());
    tag.add_attribute("accountID", strAcctID.Get());
    tag.add_attribute("nymID", strNymID.Get());
    tag.add_attribute("notaryID", strNotaryID.Get());
    tag.add_attribute("numberOfOrigin", formatLong(GetRawNumberOfOrigin()));

    if (GetOriginType() != originType::not_applicable) {
        String strOriginType(GetOriginTypeString());
        tag.add_attribute("originType", strOriginType.Get());
    }

    tag.add_attribute("transactionNum", formatLong(GetTransactionNum()));
    tag.add_attribute("inReferenceTo", formatLong(GetReferenceToNum()));

    if (m_bCancelled) tag.add_attribute("cancelled", formatBool(m_bCancelled));

    if (OTTransaction::replyNotice == m_Type) {
        tag.add_attribute("requestNumber", formatLong(m_lRequestNumber));
        tag.add_attribute("transSuccess", formatBool(m_bReplyTransSuccess));
    }

    // IF this transaction is "blank" or
    // "successNotice" this will serialize the list of
    // transaction numbers for it. (They now support
    // multiple numbers.)
    //
    // Blank is a freshly issued transaction number,
    // not accepted by the user (yet.)
    // Whereas successNotice means a transaction #
    // has successfully been signed out.
    if ((OTTransaction::blank == m_Type) ||
        (OTTransaction::successNotice == m_Type)) {
        // Count is always 0, except for blanks
        // and successNotices.
        if (m_Numlist.Count() > 0) {
            String strNumbers;
            if (m_Numlist.Output(strNumbers))
                tag.add_attribute("totalListOfNumbers", strNumbers.Get());
        }
    }

    if (IsAbbreviated()) {
        if (nullptr != m_pParent) {

            switch (m_pParent->GetType()) {
                case Ledger::nymbox:
                    SaveAbbreviatedNymboxRecord(tag);
                    break;
                case Ledger::inbox:
                    SaveAbbreviatedInboxRecord(tag);
                    break;
                case Ledger::outbox:
                    SaveAbbreviatedOutboxRecord(tag);
                    break;
                case Ledger::paymentInbox:
                    SaveAbbrevPaymentInboxRecord(tag);
                    break;
                case Ledger::recordBox:
                    SaveAbbrevRecordBoxRecord(tag);
                    break;
                case Ledger::expiredBox:
                    SaveAbbrevExpiredBoxRecord(tag);
                    break;
                /* --- BREAK --- */
                case Ledger::message:
                    otErr
                        << "OTTransaction::" << __FUNCTION__
                        << ": Unexpected message ledger type in 'abbreviated' "
                           "block. (Error.) \n";
                    break;
                default:
                    otErr << "OTTransaction::" << __FUNCTION__
                          << ": Unexpected ledger type in 'abbreviated' block. "
                             "(Error.) \n";
                    break;
            } /*switch*/
        } else
            otErr << "OTTransaction::" << __FUNCTION__
                  << ": Error: Unable to save abbreviated receipt here, since "
                     "m_pParent is nullptr.\n";

    } else  // not abbreviated (full details.)
    {
        if ((OTTransaction::finalReceipt == m_Type) ||
            (OTTransaction::basketReceipt == m_Type)) {
            TagPtr tagClosingNo(new Tag("closingTransactionNumber"));
            tagClosingNo->add_attribute(
                "value", formatLong(m_lClosingTransactionNo));
            tag.add_tag(tagClosingNo);
        }

        // a transaction contains a list of items, but it is also in reference
        // to some item, from someone else
        // We include a full copy of that item here.
        if (m_ascInReferenceTo.GetLength()) {
            tag.add_tag("inReferenceTo", m_ascInReferenceTo.Get());
        }

        if (m_ascCancellationRequest.GetLength()) {
            tag.add_tag("cancelRequest", m_ascCancellationRequest.Get());
        }

        // loop through the items that make up this transaction and print them
        // out here, base64-encoded, of course.
        for (auto& it : m_listItems) {
            Item* pItem = it;
            OT_ASSERT(nullptr != pItem);

            String strItem;
            pItem->SaveContractRaw(strItem);

            OTASCIIArmor ascItem;
            ascItem.SetString(strItem, true);  // linebreaks = true

            tag.add_tag("item", ascItem.Get());
        }
    }  // not abbreviated (full details.)

    std::string str_result;
    tag.output(str_result);

    m_xmlUnsigned.Concatenate("%s", str_result.c_str());
}

/*
 Question note to self:  Which of the above transaction types can be found
inside:
 paymentInbox ledger, paymentOutbox ledger, and recordBox ledger?

 void SaveAbbrevPaymentInboxRecord(OTString& strOutput);
 void SaveAbbrevPaymentOutboxRecord(OTString& strOutput);
 void SaveAbbrevRecordBoxRecord(OTString& strOutput);


 --- paymentInbox ledger:

    "instrumentNotice",        // Receive these in paymentInbox, and send in
paymentOutbox. (When done, they go to recordBox to await deletion.)
    "instrumentRejection",    // When someone rejects your invoice from his
paymentInbox, you get one of these in YOUR paymentInbox.


 --- paymentOutbox ledger:

    "instrumentNotice",        // Receive these in paymentInbox, and send in
paymentOutbox. (When done, they go to recordBox to await deletion.)


 --- recordBox ledger:

 // These all come from the asset account inbox (where they are transferred from
before they end up in the record box.)
    "pending",            // Pending transfer, in the inbox/outbox. (This can
                          // end up in your recordBox if you cancel your pending
outgoing transfer.) "transferReceipt",    // the server drops this into your
inbox, when someone
                          // accepts your transfer.
    "chequeReceipt",    // the server drops this into your inbox, when someone
                        // cashes your cheque.
    "voucherReceipt",    // the server drops this into your inbox, when someone
                         // cashes your voucher.
    "marketReceipt",    // server drops this into inbox periodically, if you
                        // have an offer on the market.
    "paymentReceipt",    // the server drops this into people's inboxes,
                         // periodically, if they have payment plans.
    "finalReceipt",     // the server drops this into your inbox(es), when a
                        // CronItem expires or is canceled.
    "basketReceipt",    // the server drops this into your inboxes, when a
                        // basket exchange is processed.

// Record box may also store things that came from a Nymbox, and then had to go
// somewhere client-side for storage, until user decides to delete them.
// For example:
    "notice",  // in nymbox, notice from the server. Probably contains an
updated smart contract.

// Whereas for a recordBox storing paymentInbox/paymentOutbox receipts, once
// they are completed, they go here to die.
    "instrumentNotice",       // Receive these in paymentInbox, and send in
                              // paymentOutbox. (When done, they go to recordBox
to await deletion.) "instrumentRejection",    // When someone rejects your
invoice from his
                              // paymentInbox, you get one of these in YOUR
paymentInbox.
 */

/*
  --- paymentInbox ledger:
    "instrumentNotice",       // Receive these in paymentInbox, and send in
                              // paymentOutbox. (When done, they go to recordBox
  to await deletion.) "instrumentRejection",    // When someone rejects your
  invoice from his
                              // paymentInbox, you get one of these in YOUR
  paymentInbox.
 */
void OTTransaction::SaveAbbrevPaymentInboxRecord(Tag& parent)
{
    int64_t lDisplayValue = 0;

    switch (m_Type) {
        case OTTransaction::instrumentNotice:
            if (IsAbbreviated())
                lDisplayValue = GetAbbrevDisplayAmount();
            else
                lDisplayValue = GetReceiptAmount();
            break;
        case OTTransaction::instrumentRejection:
            if (IsAbbreviated())  // not the actual value of 0.
                lDisplayValue = GetAbbrevDisplayAmount();
            break;
        default:  // All other types are irrelevant for payment inbox reports
            otErr << "OTTransaction::" << __FUNCTION__ << ": Unexpected "
                  << GetTypeString()
                  << " transaction "
                     "in payment inbox while making abbreviated "
                     "payment inbox record.\n";

            OT_FAIL_MSG("ASSERT: OTTransaction::SaveAbbrevPaymentInboxRecord: "
                        "Unexpected transaction type.");

            return;
    }

    // By this point, we know only the right types of receipts
    // are being saved, and the adjustment and display value are
    // both set correctly.

    // TYPE
    String strType;
    const char* pTypeStr = GetTypeString();
    strType.Set((nullptr != pTypeStr) ? pTypeStr : "error_state");

    // HASH OF THE COMPLETE "BOX RECEIPT"
    // Save abbreviated is only used for receipts in boxes such as inbox,
    // outbox, and nymbox.
    // (Thus the moniker "Box Receipt", as contrasted with cron receipts or
    // normal transaction receipts with balance agreements.)
    //
    String strHash;

    // If this is already an abbreviated record, then save the existing hash.
    if (IsAbbreviated()) m_Hash.GetString(strHash);
    // Otherwise if it's a full record, then calculate the hash and save it.
    else {
        Identifier idReceiptHash;  // a hash of the actual transaction is
                                   // stored with its
        CalculateContractID(idReceiptHash);  // abbreviated short-form
                                             // record (in the payment
                                             // inbox, for example.)
        idReceiptHash.GetString(strHash);
    }

    TagPtr pTag(new Tag("paymentInboxRecord"));

    pTag->add_attribute("type", strType.Get());
    pTag->add_attribute("dateSigned", formatTimestamp(m_DATE_SIGNED));
    pTag->add_attribute("receiptHash", strHash.Get());
    pTag->add_attribute("displayValue", formatLong(lDisplayValue));
    pTag->add_attribute("transactionNum", formatLong(GetTransactionNum()));
    pTag->add_attribute(
        "inRefDisplay", formatLong(GetReferenceNumForDisplay()));
    pTag->add_attribute("inReferenceTo", formatLong(GetReferenceToNum()));

    if (GetOriginType() != originType::not_applicable) {
        String strOriginType(GetOriginTypeString());
        pTag->add_attribute("originType", strOriginType.Get());
    }

    parent.add_tag(pTag);
}

void OTTransaction::SaveAbbrevExpiredBoxRecord(Tag& parent)
{
    int64_t lDisplayValue = 0;

    switch (m_Type) {
        // PAYMENT INBOX / PAYMENT OUTBOX
        case OTTransaction::instrumentNotice:
            if (IsAbbreviated())  // not the actual value of 0.
                lDisplayValue = GetAbbrevDisplayAmount();
            else
                lDisplayValue = GetReceiptAmount();
            break;
        case OTTransaction::instrumentRejection:
            if (IsAbbreviated())  // not the actual value of 0.
                lDisplayValue = GetAbbrevDisplayAmount();
            else
                lDisplayValue = 0;
            break;
        case OTTransaction::notice:  // A notice from the server. Used in
                                     // Nymbox. Probably contains an updated
                                     // smart contract.
            if (IsAbbreviated())     // not the actual value of 0.
                lDisplayValue = GetAbbrevDisplayAmount();
            else
                lDisplayValue = 0;
            break;
        default:  // All other types are irrelevant for inbox reports
        {
            otErr << "OTTransaction::" << __FUNCTION__ << ": Unexpected "
                  << GetTypeString()
                  << " transaction "
                     "in expired box while making abbreviated expired-box "
                     "record.\n";

            OT_FAIL_MSG("ASSERT: OTTransaction::SaveAbbrevExpiredBoxRecord: "
                        "Unexpected transaction type.");
        }
            return;
    }

    // By this point, we know only the right types of receipts are being saved,
    // and
    // the adjustment and display value are both set correctly.

    // TYPE
    String strType;
    const char* pTypeStr = GetTypeString();
    strType.Set((nullptr != pTypeStr) ? pTypeStr : "error_state");

    // HASH OF THE COMPLETE "BOX RECEIPT"
    // Save abbreviated is only used for receipts in boxes such as inbox,
    // outbox, and nymbox.
    // (Thus the moniker "Box Receipt", as contrasted with cron receipts or
    // normal transaction receipts with balance agreements.)
    //
    String strHash;

    // If this is already an abbreviated record, then save the existing hash.
    if (IsAbbreviated()) m_Hash.GetString(strHash);
    // Otherwise if it's a full record, then calculate the hash and save it.
    else {
        Identifier idReceiptHash;  // a hash of the actual transaction is
                                   // stored with its
        CalculateContractID(idReceiptHash);  // abbreviated short-form
                                             // record (in the expired box,
                                             // for example.)
        idReceiptHash.GetString(strHash);
    }

    TagPtr pTag(new Tag("expiredBoxRecord"));

    pTag->add_attribute("type", strType.Get());
    pTag->add_attribute("dateSigned", formatTimestamp(m_DATE_SIGNED));
    pTag->add_attribute("receiptHash", strHash.Get());
    pTag->add_attribute("displayValue", formatLong(lDisplayValue));
    pTag->add_attribute("transactionNum", formatLong(GetTransactionNum()));
    pTag->add_attribute(
        "inRefDisplay", formatLong(GetReferenceNumForDisplay()));
    pTag->add_attribute("inReferenceTo", formatLong(GetReferenceToNum()));

    if (GetOriginType() != originType::not_applicable) {
        String strOriginType(GetOriginTypeString());
        pTag->add_attribute("originType", strOriginType.Get());
    }

    parent.add_tag(pTag);
}

/*
 --- recordBox ledger:

 // These all come from the ASSET ACCT INBOX (where they are transferred from
before they end up in the record box.)
    "pending",            // Pending transfer, in the inbox/outbox. (This can
end up in your recordBox if you cancel your pending outgoing transfer.)
    "transferReceipt",    // the server drops this into your inbox, when someone
accepts your transfer.
    "chequeReceipt",    // the server drops this into your inbox, when someone
cashes your cheque.
    "voucherReceipt",    // the server drops this into your inbox, when someone
cashes your voucher.
    "marketReceipt",    // server drops this into inbox periodically, if you
have an offer on the market.
    "paymentReceipt",    // the server drops this into people's inboxes,
periodically, if they have payment plans.
    "finalReceipt",     // the server drops this into your inbox(es), when a
CronItem expires or is canceled.
    "basketReceipt",    // the server drops this into your inboxes, when a
basket exchange is processed.

 // Record box may also store things that came from a NYMBOX, and then had to go
somewhere client-side for storage,
 // until user decides to delete them. For example:
    "notice",            // in nymbox, notice from the server. Probably contains
an updated smart contract.

// Whereas for a recordBox storing PAYMENT-INBOX and PAYMENT-OUTBOX receipts,
once they are completed, they go here to die.
    "instrumentNotice",        // Receive these in paymentInbox, and send in
paymentOutbox. (When done, they go to recordBox to await deletion.)
    "instrumentRejection",    // When someone rejects your invoice from his
paymentInbox, you get one of these in YOUR paymentInbox.


 NOTE: The expiredBox is identical to recordBox (for things that came from
payments inbox or outpayments box.)
 Except it's used for expired payments, instead of completed / canceled
payments.
 */
void OTTransaction::SaveAbbrevRecordBoxRecord(Tag& parent)
{
    // Have some kind of check in here, whether the AcctID and NymID match.
    // Some recordBoxes DO, and some DON'T (the different kinds store different
    // kinds of receipts. See above comment.)

    int64_t lAdjustment = 0, lDisplayValue = 0;

    switch (m_Type) {
        // ASSET ACCOUNT INBOX
        // -- In inbox, pending hasn't been accepted yet. In outbox, it's
        // already gone. Either way, it will have a 0 adjustment amount, even
        // though perhaps 500 clams display amount. Here I use the 500 for
        // display, but in SaveAbbrevToOutbox, I multiply it by -1 so it appears
        // as -500 (leaving my account.)
        // -- In my inbox, the transferReceipt is notice of money that is
        // already gone. It thus has adjustment value of 0. But the DISPLAY
        // amount is the amount I originally sent. (Already multiplied by -1 by
        // GetReceiptAmount())
        //
        case OTTransaction::pending:  // (The pending amount is stored on the
                                      // transfer item in my list of transaction
                                      // items.)
        case OTTransaction::transferReceipt:  // The transferReceipt and
                                              // voucherReceipt amounts are the
                                              // display value (according to
        case OTTransaction::voucherReceipt:  // GetReceiptAmount()), and not the
                                             // actual value of 0.
            if (IsAbbreviated()) {
                lAdjustment = GetAbbrevAdjustment();
                lDisplayValue = GetAbbrevDisplayAmount();
            } else {
                lAdjustment = 0;
                lDisplayValue = GetReceiptAmount();
            }
            break;
        // If chequeReceipt for 100 clams hit my inbox, then my balance is -100
        // from where it was. (Same value should be displayed.) Luckily,
        // GetReceiptAmount() already multiplies by (-1) for chequeReceipt. For
        // these (marketReceipt, paymentReceipt, basketReceipt), the actual
        // adjustment is positive OR negative
        // already, and the display value should match.
        case OTTransaction::chequeReceipt:  // the amount is stored on cheque
                                            // (attached to depositCheque item,
                                            // attached.)
        case OTTransaction::marketReceipt:  // amount is stored on marketReceipt
                                            // item.  |
        case OTTransaction::paymentReceipt:  // amount is stored on
                                             // paymentReceipt item. | and the
                                             // display value should match.
        case OTTransaction::basketReceipt:  // amount is stored on basketReceipt
                                            // item.  |
            if (IsAbbreviated())            // not the actual value of 0.
            {
                lAdjustment = GetAbbrevAdjustment();
                lDisplayValue = GetAbbrevDisplayAmount();
            } else {
                lAdjustment = GetReceiptAmount();
                lDisplayValue = lAdjustment;
            }
            break;
        case OTTransaction::finalReceipt:  // amount is 0 according to
                                           // GetReceiptAmount()
            if (IsAbbreviated())           // not the actual value of 0.
            {
                lAdjustment = GetAbbrevAdjustment();
                lDisplayValue = GetAbbrevDisplayAmount();
            } else {
                lAdjustment = 0;
                lDisplayValue = 0;
            }
            break;
        // NYMBOX
        case OTTransaction::notice:  // A notice from the server. Used in
                                     // Nymbox. Probably contains an updated
                                     // smart contract.
            if (IsAbbreviated())     // not the actual value of 0.
            {
                lAdjustment = GetAbbrevAdjustment();
                lDisplayValue = GetAbbrevDisplayAmount();
            } else {
                lAdjustment = 0;
                lDisplayValue = 0;
            }
            break;
        // PAYMENT INBOX / PAYMENT OUTBOX
        case OTTransaction::instrumentNotice:
            if (IsAbbreviated())  // not the actual value of 0.
            {
                lAdjustment = GetAbbrevAdjustment();
                lDisplayValue = GetAbbrevDisplayAmount();
            } else {
                lAdjustment = 0;
                lDisplayValue = GetReceiptAmount();
            }
            break;
        case OTTransaction::instrumentRejection:
            if (IsAbbreviated())  // not the actual value of 0.
            {
                lAdjustment = GetAbbrevAdjustment();
                lDisplayValue = GetAbbrevDisplayAmount();
            } else {
                lAdjustment = 0;
                lDisplayValue = 0;
            }
            break;
        default:  // All other types are irrelevant for inbox reports
        {
            otErr << "OTTransaction::SaveAbbrevRecordBoxRecord: Unexpected "
                  << GetTypeString()
                  << " transaction "
                     "in record box while making abbreviated record-box "
                     "record.\n";
        }
            return;
    }  // why not transfer receipt? Because the amount was already removed from
       // your account when you transferred it,

    // By this point, we know only the right types of receipts are being saved,
    // and
    // the adjustment and display value are both set correctly.

    // TYPE
    String strType;
    const char* pTypeStr = GetTypeString();
    strType.Set((nullptr != pTypeStr) ? pTypeStr : "error_state");

    // HASH OF THE COMPLETE "BOX RECEIPT"
    // Save abbreviated is only used for receipts in boxes such as inbox,
    // outbox, and nymbox.
    // (Thus the moniker "Box Receipt", as contrasted with cron receipts or
    // normal transaction receipts with balance agreements.)
    //
    String strHash;

    // If this is already an abbreviated record, then save the existing hash.
    if (IsAbbreviated()) m_Hash.GetString(strHash);
    // Otherwise if it's a full record, then calculate the hash and save it.
    else {
        Identifier idReceiptHash;  // a hash of the actual transaction is
                                   // stored with its
        CalculateContractID(idReceiptHash);  // abbreviated short-form
                                             // record (in the record box,
                                             // for example.)
        idReceiptHash.GetString(strHash);
    }

    TagPtr pTag(new Tag("recordBoxRecord"));

    pTag->add_attribute("type", strType.Get());
    pTag->add_attribute("dateSigned", formatTimestamp(m_DATE_SIGNED));
    pTag->add_attribute("receiptHash", strHash.Get());
    pTag->add_attribute("adjustment", formatLong(lAdjustment));
    pTag->add_attribute("displayValue", formatLong(lDisplayValue));
    pTag->add_attribute("numberOfOrigin", formatLong(GetRawNumberOfOrigin()));

    if (GetOriginType() != originType::not_applicable) {
        String strOriginType(GetOriginTypeString());
        pTag->add_attribute("originType", strOriginType.Get());
    }

    pTag->add_attribute("transactionNum", formatLong(GetTransactionNum()));
    pTag->add_attribute(
        "inRefDisplay", formatLong(GetReferenceNumForDisplay()));
    pTag->add_attribute("inReferenceTo", formatLong(GetReferenceToNum()));

    if ((OTTransaction::finalReceipt == m_Type) ||
        (OTTransaction::basketReceipt == m_Type))
        pTag->add_attribute("closingNum", formatLong(GetClosingNum()));

    parent.add_tag(pTag);
}

// All of the actual receipts cannot fit inside the inbox file,
// which can get huge, and bog down network ability to transmit.
// Instead, we save receipts in abbreviated form in the inbox,
// then let the users download those receipts individually. That
// way, each message cannot be too large to download, such as
// a giant inbox can be with 400000 receipts inside of it.
//
void OTTransaction::SaveAbbreviatedNymboxRecord(Tag& parent)
{
    int64_t lDisplayValue = 0;
    bool bAddRequestNumber = false;

    String strListOfBlanks;  // IF this transaction is "blank" or
                             // "successNotice" this will serialize the list of
                             // transaction numbers for it. (They now support
                             // multiple numbers.)
    switch (m_Type) {
        case OTTransaction::blank:  // freshly issued transaction number, not
                                    // accepted by the user (yet).
        case OTTransaction::successNotice:  // A transaction # has successfully
                                            // been signed out.
        {
            // This is always 0, except for blanks and successNotices.
            if (m_Numlist.Count() > 0) m_Numlist.Output(strListOfBlanks);
        }

        /* ! CONTINUES FALLING THROUGH HERE!!... */

        case OTTransaction::replyNotice:  // A copy of a server reply to a
                                          // previous request you sent. (To make
                                          // SURE you get the reply.)

            // NOTE: a comment says "ONLY replyNotice
            // transactions carry a request num" but the
            // fall-thru above seems to disagree. Bug?
            // Or just an old comment?
            bAddRequestNumber = true;

            break;

        case OTTransaction::message:  // A message from one user to another,
                                      // also in the nymbox.
        case OTTransaction::notice:   // A notice from the server. Used in
                                      // Nymbox. Probably contains an updated
                                      // smart contract.
        case OTTransaction::finalReceipt:  // Any finalReceipt in an inbox will
                                           // also drop a copy into the Nymbox.
            break;

        // paymentInbox items are transported through the Nymbox.
        // Therefore, this switch statement from SaveAbbrevPaymentInbox
        // is also found here, to handle those receipts as they pass through.
        case OTTransaction::instrumentNotice:  // A financial instrument sent
                                               // from/to another nym.
            if (IsAbbreviated())
                lDisplayValue = GetAbbrevDisplayAmount();
            else
                lDisplayValue = GetReceiptAmount();

            break;  // (These last two are just passing through, on their way to
                    // the paymentInbox.)
        case OTTransaction::instrumentRejection:  // A rejection notice from the
                                                  // intended recipient of an
                                                  // instrumentNotice.
            lDisplayValue = 0;
            break;

        default:  // All other types are irrelevant for nymbox reports.
            otErr << __FUNCTION__ << ": Unexpected " << GetTypeString()
                  << " transaction in nymbox while making abbreviated nymbox "
                     "record.\n";
            OT_FAIL_MSG("ASSERT: OTTransaction::SaveAbbreviatedNymboxRecord: "
                        "Unexpected transaction in this Nymbox.");

            return;
    }

    // By this point, we know only the right types of receipts are being saved,
    // and
    // the adjustment and display value are both set correctly.

    // TYPE
    String strType;
    const char* pTypeStr = GetTypeString();
    strType.Set((nullptr != pTypeStr) ? pTypeStr : "error_state");

    // HASH OF THE COMPLETE "BOX RECEIPT"
    // Save abbreviated is only used for receipts in boxes such as inbox,
    // outbox, and nymbox.
    // (Thus the moniker "Box Receipt", as contrasted with cron receipts or
    // normal transaction receipts with balance agreements.)
    //
    String strHash;

    // If this is already an abbreviated record, then save the existing hash.
    if (IsAbbreviated()) m_Hash.GetString(strHash);
    // Otherwise if it's a full record, then calculate the hash and save it.
    else {
        Identifier idReceiptHash;  // a hash of the actual transaction is
                                   // stored with its
        CalculateContractID(idReceiptHash);  // abbreviated short-form
                                             // record (in the inbox, for
                                             // example.)
        idReceiptHash.GetString(strHash);
    }

    TagPtr pTag(new Tag("nymboxRecord"));

    pTag->add_attribute("type", strType.Get());
    pTag->add_attribute("dateSigned", formatTimestamp(m_DATE_SIGNED));
    pTag->add_attribute("receiptHash", strHash.Get());
    pTag->add_attribute("transactionNum", formatLong(GetTransactionNum()));
    pTag->add_attribute(
        "inRefDisplay", formatLong(GetReferenceNumForDisplay()));
    pTag->add_attribute("inReferenceTo", formatLong(GetReferenceToNum()));

    if (GetOriginType() != originType::not_applicable) {
        String strOriginType(GetOriginTypeString());
        pTag->add_attribute("originType", strOriginType.Get());
    }

    // I actually don't think you can put a basket receipt
    // notice in a nymbox, the way you can with a final
    // receipt notice. Probably can remove that line.
    if ((OTTransaction::finalReceipt == m_Type) ||
        (OTTransaction::basketReceipt == m_Type))
        pTag->add_attribute("closingNum", formatLong(GetClosingNum()));
    else {
        if (strListOfBlanks.Exists())
            pTag->add_attribute("totalListOfNumbers", strListOfBlanks.Get());
        if (bAddRequestNumber) {
            pTag->add_attribute("requestNumber", formatLong(m_lRequestNumber));
            pTag->add_attribute(
                "transSuccess", formatBool(m_bReplyTransSuccess));
        }
        if (lDisplayValue > 0) {
            // IF this transaction is passing through on its
            // way to the paymentInbox, it will have a
            // displayValue.
            pTag->add_attribute("displayValue", formatLong(lDisplayValue));
        }
    }

    parent.add_tag(pTag);
}

void OTTransaction::SaveAbbreviatedOutboxRecord(Tag& parent)
{
    int64_t lAdjustment = 0, lDisplayValue = 0;

    switch (m_Type) {
        case OTTransaction::pending:
            if (IsAbbreviated()) {
                lAdjustment = GetAbbrevAdjustment();
                lDisplayValue = GetAbbrevDisplayAmount();
            } else {
                lAdjustment =
                    0;  // In the inbox, a pending hasn't been accepted yet.
                lDisplayValue =  //    In the outbox, it's already gone.
                    (GetReceiptAmount() *
                     (-1));  // Either way, it will have a 0
                             // adjustment amount, even though
                             // perhaps 500 clams display
                             // amount.
            }
            break;  // In this case, since it's the outbox, then it's a MINUS
                    // (-500) Display amount (since I'm sending, not receiving
                    // it.)
        default:    // All other types are irrelevant for outbox reports.
            otErr << "OTTransaction::SaveAbbreviatedOutboxRecord: Unexpected "
                  << GetTypeString()
                  << " transaction "
                     "in outbox while making abbreviated outbox record.\n";

            OT_FAIL_MSG("ASSERT: OTTransaction::SaveAbbreviatedOutboxRecord: "
                        "unexpected transaction type.");

            return;
    }

    // By this point, we know only the right types of receipts are being saved,
    // and
    // the adjustment and display value are both set correctly.

    // TYPE
    String strType;
    const char* pTypeStr = GetTypeString();
    strType.Set((nullptr != pTypeStr) ? pTypeStr : "error_state");

    // HASH OF THE COMPLETE "BOX RECEIPT"
    // Save abbreviated is only used for receipts in boxes such as inbox,
    // outbox, and nymbox.
    // (Thus the moniker "Box Receipt", as contrasted with cron receipts or
    // normal transaction receipts with balance agreements.)
    //
    String strHash;

    // If this is already an abbreviated record, then save the existing hash.
    if (IsAbbreviated()) m_Hash.GetString(strHash);
    // Otherwise if it's a full record, then calculate the hash and save it.
    else {
        Identifier idReceiptHash;  // a hash of the actual transaction is
                                   // stored with its
        CalculateContractID(idReceiptHash);  // abbreviated short-form
                                             // record (in the inbox, for
                                             // example.)
        idReceiptHash.GetString(strHash);
    }

    TagPtr pTag(new Tag("outboxRecord"));

    pTag->add_attribute("type", strType.Get());
    pTag->add_attribute("dateSigned", formatTimestamp(m_DATE_SIGNED));
    pTag->add_attribute("receiptHash", strHash.Get());
    pTag->add_attribute("adjustment", formatLong(lAdjustment));
    pTag->add_attribute("displayValue", formatLong(lDisplayValue));
    pTag->add_attribute("numberOfOrigin", formatLong(GetRawNumberOfOrigin()));

    if (GetOriginType() != originType::not_applicable) {
        String strOriginType(GetOriginTypeString());
        pTag->add_attribute("originType", strOriginType.Get());
    }

    pTag->add_attribute("transactionNum", formatLong(GetTransactionNum()));
    pTag->add_attribute(
        "inRefDisplay", formatLong(GetReferenceNumForDisplay()));
    pTag->add_attribute("inReferenceTo", formatLong(GetReferenceToNum()));

    parent.add_tag(pTag);
}

void OTTransaction::SaveAbbreviatedInboxRecord(Tag& parent)
{
    // This is the actual amount that your account is changed BY this receipt.
    // Versus the useful amount the user will want to see (lDisplayValue.) For
    // example, if you perform
    // a transfer of 500 clams, then the money leaves your account at that time,
    // and you receive a transaction receipt
    // to that effect. LATER ON, when the recipient ACCEPTS the transfer, a
    // "transferReceipt" will pop into your inbox,
    // which you must accept in order to close out the transaction number. But
    // this transferReceipt "adjusts" your account
    // by ZERO, since the amount has ALREADY left your account before the
    // transferReceipt arrived. In that example, the
    // lAdjustment would be 0, while the lDisplayValue would be 500. The first
    // value is the actual impact on your balance
    // from that specific receipt, whereas the second value is the one that the
    // user probably wants to see.

    // NOTE: A similar logic envelops the GetReferenceNumForDisplay() field,
    // which, instead of returning the ACTUAL
    // ref# that OT needs to use, it will return the one that the user probably
    // wants to see.
    //
    int64_t lAdjustment = 0, lDisplayValue = 0;

    switch (m_Type) {
        // -- In inbox, pending hasn't been accepted yet. In outbox, it's
        // already gone. Either way, it will have a 0 adjustment amount, even
        // though perhaps 500 clams display amount. Here I use the 500 for
        // display, but in SaveAbbrevToOutbox, I multiply it by -1 so it appears
        // as -500 (leaving my account.)
        // -- In my inbox, the transferReceipt is notice of money that is
        // already gone. It thus has adjustment value of 0. But the DISPLAY
        // amount is the amount I originally sent. (Already multiplied by -1 by
        // GetReceiptAmount())
        //
        case OTTransaction::pending:  // (The pending amount is stored on the
                                      // transfer item in my list of transaction
                                      // items.)
        case OTTransaction::transferReceipt:  // The transferReceipt and
                                              // voucherReceipt amounts are the
                                              // display value (according to
        case OTTransaction::voucherReceipt:  // GetReceiptAmount()), and not the
                                             // actual value of 0.
            if (IsAbbreviated()) {
                lAdjustment = GetAbbrevAdjustment();
                lDisplayValue = GetAbbrevDisplayAmount();
            } else {
                lAdjustment = 0;
                lDisplayValue = GetReceiptAmount();
            }
            break;
        // If chequeReceipt for 100 clams hit my inbox, then my balance is -100
        // from where it was. (Same value should be displayed.) Luckily,
        // GetReceiptAmount() already multiplies by (-1) for chequeReceipt. For
        // these (marketReceipt, paymentReceipt, basketReceipt), the actual
        // adjustment is positive OR negative
        // already, and the display value should match.
        case OTTransaction::chequeReceipt:  // the amount is stored on cheque
                                            // (attached to depositCheque item,
                                            // attached.)
        case OTTransaction::marketReceipt:  // amount is stored on marketReceipt
                                            // item.  |
        case OTTransaction::paymentReceipt:  // amount is stored on
                                             // paymentReceipt item. | and the
                                             // display value should match.
        case OTTransaction::basketReceipt:  // amount is stored on basketReceipt
                                            // item.  |
            if (IsAbbreviated())            // not the actual value of 0.
            {
                lAdjustment = GetAbbrevAdjustment();
                lDisplayValue = GetAbbrevDisplayAmount();
            } else {
                lAdjustment = GetReceiptAmount();
                lDisplayValue = lAdjustment;
            }
            break;
        case OTTransaction::finalReceipt:  // amount is 0 according to
                                           // GetReceiptAmount()
            if (IsAbbreviated())           // not the actual value of 0.
            {
                lAdjustment = GetAbbrevAdjustment();
                lDisplayValue = GetAbbrevDisplayAmount();
            } else {
                lAdjustment = 0;
                lDisplayValue = 0;
            }
            break;
        default:  // All other types are irrelevant for inbox reports
        {
            otErr << "OTTransaction::" << __FUNCTION__ << ": Unexpected "
                  << GetTypeString()
                  << " transaction "
                     "in inbox while making abbreviated inbox record.\n";

            OT_FAIL_MSG("ASSERT: OTTransaction::SaveAbbreviatedInboxRecord: "
                        "unexpected transaction type.");
        }
            return;
    }  // why not transfer receipt? Because the amount was already removed from
       // your account when you transferred it,

    // By this point, we know only the right types of receipts are being saved,
    // and
    // the adjustment and display value are both set correctly.

    // TYPE
    String strType;
    const char* pTypeStr = GetTypeString();
    strType.Set((nullptr != pTypeStr) ? pTypeStr : "error_state");

    // HASH OF THE COMPLETE "BOX RECEIPT"
    // Save abbreviated is only used for receipts in boxes such as inbox,
    // outbox, and nymbox.
    // (Thus the moniker "Box Receipt", as contrasted with cron receipts or
    // normal transaction receipts with balance agreements.)
    //
    String strHash;

    // If this is already an abbreviated record, then save the existing hash.
    if (IsAbbreviated()) m_Hash.GetString(strHash);
    // Otherwise if it's a full record, then calculate the hash and save it.
    else {
        Identifier idReceiptHash;  // a hash of the actual transaction is
                                   // stored with its
        CalculateContractID(idReceiptHash);  // abbreviated short-form
                                             // record (in the inbox, for
                                             // example.)
        idReceiptHash.GetString(strHash);
    }

    TagPtr pTag(new Tag("inboxRecord"));

    pTag->add_attribute("type", strType.Get());
    pTag->add_attribute("dateSigned", formatTimestamp(m_DATE_SIGNED));
    pTag->add_attribute("receiptHash", strHash.Get());
    pTag->add_attribute("adjustment", formatLong(lAdjustment));
    pTag->add_attribute("displayValue", formatLong(lDisplayValue));
    pTag->add_attribute("numberOfOrigin", formatLong(GetRawNumberOfOrigin()));

    if (GetOriginType() != originType::not_applicable) {
        String strOriginType(GetOriginTypeString());
        pTag->add_attribute("originType", strOriginType.Get());
    }

    pTag->add_attribute("transactionNum", formatLong(GetTransactionNum()));
    pTag->add_attribute(
        "inRefDisplay", formatLong(GetReferenceNumForDisplay()));
    pTag->add_attribute("inReferenceTo", formatLong(GetReferenceToNum()));

    if ((OTTransaction::finalReceipt == m_Type) ||
        (OTTransaction::basketReceipt == m_Type))
        pTag->add_attribute("closingNum", formatLong(GetClosingNum()));

    parent.add_tag(pTag);
}

// The ONE case where an Item has SUB-ITEMS is in the case of Balance Agreement.
// For example, you might have a Withdrawal Transaction (request) that contains
// 2 items: the withdrawal item itself, and the balance agreement item for that
// withdrawal.  The balance agreement item contains a LIST OF SUB ITEMS, each of
// which represents a chequeReceipt, marketReceipt, or paymentReceipt from my
// inbox. The Balance Agreement item needs to be able to report on the inbox
// status, so I give it a list of sub-items.
//
void OTTransaction::ProduceInboxReportItem(Item& theBalanceItem)
{
    Item::itemType theItemType = Item::error_state;

    otLog3 << "Producing statement report item for inbox item type: "
           << GetTypeString() << ".\n";  // temp remove.

    switch (m_Type) {  // These are the types that have an amount (somehow)
        case OTTransaction::pending:  // the amount is stored on the transfer
                                      // item in my list.
            theItemType = Item::transfer;
            break;
        case OTTransaction::chequeReceipt:  // the amount is stored on cheque
                                            // (attached to depositCheque item,
                                            // attached.)
            theItemType = Item::chequeReceipt;
            break;
        case OTTransaction::voucherReceipt:  // the amount is stored on voucher
                                             // (attached to depositCheque item,
                                             // attached.)
            theItemType = Item::voucherReceipt;
            break;
        case OTTransaction::marketReceipt:  // the amount is stored on
                                            // marketReceipt item
            theItemType = Item::marketReceipt;
            break;
        case OTTransaction::paymentReceipt:  // amount is stored on
                                             // paymentReceipt item
            theItemType = Item::paymentReceipt;
            break;
        case OTTransaction::transferReceipt:  // amount is 0 according to
                                              // GetReceiptAmount()
            theItemType = Item::transferReceipt;
            break;
        case OTTransaction::basketReceipt:  // amount is stored on basketReceipt
                                            // item.
            theItemType = Item::basketReceipt;
            break;
        case OTTransaction::finalReceipt:  // amount is 0 according to
                                           // GetReceiptAmount()
            theItemType = Item::finalReceipt;
            break;
        default:  // All other types are irrelevant for inbox reports
        {
            otLog3 << "OTTransaction::ProduceInboxReportItem: Ignoring "
                   << GetTypeString()
                   << " transaction "
                      "in inbox while making balance statement.\n";
        }
            return;
    }  // why not transfer receipt? Because the amount was already removed from
       // your account when you transferred it,
    // and you already signed a balance agreement at that time. Thus, nothing in
    // your inbox is necessary to prove
    // the change in balance -- you already signed off on it. UPDATE: that's
    // okay since the below GetReceiptAmount()
    // will return 0 for a transfer receipt anyway.

    // In the case of a cron receipt which is in the inbox, but is being
    // accepted
    // by a notarizeProcessInbox, (if theOwner is a processInbox transaction)
    // then
    // we don't want to report that item. Why not? Because if the processInbox
    // is a
    // success, the item would be presumed removed. (That's what the process
    // aims to do,
    // after all: accept and remove the market receipt.) Therefore, I don't want
    // to add
    // it to the report, since the server will then think it's supposed to be
    // there, when
    // in fact it's supposed to be gone. I'm supposed to be showing a picture of
    // what would
    // be left in the event of a success. And if I successfully processed the
    // receipt out of my
    // inbox, then I would expect not to see it there anymore, so since that is
    // what I would
    // expect in that case, that is the picture I need to construct now.
    //
    // Thus, here we loop through theOwner (IF he's a process inbox transaction)
    // and we see
    // if he's actually trying to process a receipt off the inbox FOR ME (THIS
    // transaction.) If he is, then
    // we don't need to add this transaction to the report.
    //

    // the item will represent THIS TRANSACTION, and will be added to
    // theBalanceItem.

    Item* pReportItem = Item::CreateItemFromTransaction(*this, theItemType);

    if (nullptr !=
        pReportItem)  // above line will assert if mem allocation fails.
    {
        int64_t lAmount = GetReceiptAmount();
        pReportItem->SetAmount(lAmount);

        pReportItem->SetTransactionNum(
            GetTransactionNum());  // Just making sure these both get set.
        pReportItem->SetReferenceToNum(
            GetReferenceToNum());  // Especially this one.
        pReportItem->SetNumberOfOrigin(GetNumberOfOrigin());

        // The "closing transaction number" is only used on finalReceipts and
        // basketReceipts.
        // FYI, Any cron receipts need to see if there is a corresponding final
        // receipt before checking
        // their transaction number for validity (since it changes that
        // number)... and also, if the final
        // receipt itself is present, then ALL of the cron receipts that it
        // corresponds to must be closed!
        //
        if ((OTTransaction::finalReceipt == m_Type) ||
            (OTTransaction::basketReceipt == m_Type))
            pReportItem->SetClosingNum(GetClosingNum());

        theBalanceItem.AddItem(
            *pReportItem);  // Now theBalanceItem will handle cleaning it up.

        // No need to sign/save pReportItem, since it is just used for in-memory
        // storage, and is
        // otherwise saved as part of its owner's data, as part of its owner.
        // (As long as theBalanceItem
        // is signed and saved, which the caller does, then we're fine.)
    }
}

// No longer using outbox hash :(
// Since I would have to add the pending items to the outbox and calculate
// it myself, and there's no way every single byte would be the same as the
// server
// (Well with this implementation there is, actually, but what one of the items
// in the outbox is SIGNED by me on one side, and by the server on the other?
// the
// hashes won't match!)  Therefore I'm sending a real outbox report, the same as
// I do for the inbox. In fact, it's the same report! Just more items being
// added.
//
void OTTransaction::ProduceOutboxReportItem(Item& theBalanceItem)
{
    Item::itemType theItemType = Item::error_state;

    switch (m_Type) {
        case OTTransaction::pending:
            theItemType = Item::transfer;
            break;
        default:  // All other types are irrelevant for outbox reports.
            otErr << "ProduceOutboxReportItem: Error, wrong item type. "
                     "Returning.\n";
            return;
    }

    // the item will represent THIS TRANSACTION, and will be added to
    // theBalanceItem.

    Item* pReportItem = Item::CreateItemFromTransaction(*this, theItemType);

    if (nullptr !=
        pReportItem)  // above line will assert if mem allocation fails.
    {
        // I get away with "carte blanche" multiplying it by -1 here, because
        // I've
        // already verified that this is ONLY an OTTransaction::transfer before
        // even
        // getting this far. There is no other transaction type that I even have
        // to
        // worry about.
        const int64_t lAmount =
            GetReceiptAmount() * (-1);  // in outbox, a transfer is leaving my
                                        // account. Balance gets smaller.
        pReportItem->SetAmount(lAmount);

        pReportItem->SetTransactionNum(
            GetTransactionNum());  // Just making sure these both get set.
        pReportItem->SetReferenceToNum(
            GetReferenceToNum());  // Especially this one.
        pReportItem->SetNumberOfOrigin(GetNumberOfOrigin());

        theBalanceItem.AddItem(
            *pReportItem);  // Now theBalanceItem will handle cleaning it up.

        // No need to sign/save pReportItem, since it is just used for in-memory
        // storage, and is
        // otherwise saved as part of its owner's data, as part of its owner.
        // (As long as theBalanceItem
        // is signed and saved, which the caller does, then we're fine.)
    }
}

// A Transaction normally doesn't have an amount. (Only a transaction item
// does.)
// But this function will look up the item, when appropriate, and find out the
// amount.
//
// That way we can record it during a balance agreement.
// NOTE: Not ALL transaction types with an amount are listed here,
// just the ones necessary for balance agreement.
//
int64_t OTTransaction::GetReceiptAmount()
{
    if (IsAbbreviated()) return GetAbbrevAdjustment();

    int64_t lAdjustment = 0;

    Item* pOriginalItem = nullptr;
    std::unique_ptr<Item> theItemAngel;

    switch (GetType()) {  // These are the types that have an amount (somehow)
        case OTTransaction::marketReceipt:  // amount is stored on **
                                            // marketReceipt item **, on MY LIST
                                            // of items.
            pOriginalItem =
                GetItem(Item::marketReceipt);  // (The Reference string
                                               // contains an
                                               // OTCronItem with the
                                               // Original Trade.)
            break;                             // The "reference to" ID is
        case OTTransaction::paymentReceipt:    // amount is stored on **
                                               // paymentReceipt
                                               // ** item, on MY LIST of items.
            pOriginalItem = GetItem(Item::paymentReceipt);
            break;
        case OTTransaction::basketReceipt:  // amount is stored on **
                                            // basketReceipt
                                            // ** item, on MY LIST of items.
            pOriginalItem = GetItem(Item::basketReceipt);
            break;
        case OTTransaction::pending:  // amount is stored on the ** transfer
                                      // item **, here as reference string.
        case OTTransaction::chequeReceipt:   // amount is stored on *cheque*
                                             // (attached to ** depositCheque
                                             // ITEM **, which is here as
                                             // reference string.)
        case OTTransaction::voucherReceipt:  // amount is stored on *voucher*
                                             // (attached to ** depositCheque
                                             // ITEM
        // **, which is here as reference string.)
        case OTTransaction::transferReceipt:  // amount is stored on **
                                              // acceptPending ITEM **, (here as
                                              // reference string.)
        {
            String strReference;
            GetReferenceString(strReference);

            pOriginalItem = Item::CreateItemFromString(
                strReference, GetPurportedNotaryID(), GetReferenceToNum());

            if (nullptr != pOriginalItem) theItemAngel.reset(pOriginalItem);

            break;
        }

        default:  // All other types have no amount -- return 0.
            return 0;
    }

    if (nullptr == pOriginalItem) {
        otErr << "OTTransaction::" << __FUNCTION__
              << ": Unable to find original item. Should never happen.\n";
        return 0;  // Should never happen, since we always expect one based on
                   // the transaction type.
    }

    switch (GetType()) {  // These are the types that have an amount (somehow)
        case OTTransaction::chequeReceipt:   // amount is stored on cheque
                                             // (attached to depositCheque item,
                                             // attached.)
        case OTTransaction::voucherReceipt:  // amount is stored on voucher
                                             // (attached to depositCheque item,
                                             // attached.)
        {
            if (pOriginalItem->GetType() != Item::depositCheque) {
                otErr << __FUNCTION__ << ": Wrong item type attached to "
                      << ((OTTransaction::chequeReceipt == GetType())
                              ? "chequeReceipt"
                              : "voucherReceipt")
                      << ". (expected depositCheque)\n";
                return 0;
            }

            String strAttachment;
            Cheque theCheque;

            // Get the cheque from the Item and load it up into a Cheque object.
            pOriginalItem->GetAttachment(strAttachment);
            bool bLoadContractFromString =
                theCheque.LoadContractFromString(strAttachment);

            if (!bLoadContractFromString) {
                String strCheque(theCheque);

                otErr << "ERROR loading cheque from string in OTTransaction::"
                      << __FUNCTION__ << ":\n"
                      << strCheque << "\n";
            } else {
                lAdjustment =
                    (theCheque.GetAmount() * (-1));  // a cheque reduces my
                                                     // balance, unless
                                                     // it's negative.
            }  // So if I wrote a 100 clam cheque, that  means -100 hit my
               // account
               // when I got the
            // chequeReceipt, and writing a -100c cheque means 100 went in when
            // I
            // got the chequeReceipt.
        } break;

        // NOTE: a voucherReceipt (above) doesn't actually change your balance,
        // and neither does a transferReceipt. (below) But they both have a
        // "receipt amount" for display purposes.

        case OTTransaction::transferReceipt:  // amount is stored on
                                              // acceptPending item. (Server
                                              // refuses acceptPendings with
                                              // wrong amount on them.)

            if (pOriginalItem->GetType() != Item::acceptPending) {
                otErr << "Wrong item type attached to transferReceipt\n";
            } else {  // If I transfer 100 clams to someone, then my account is
                      // smaller by 100 clams. -100 has hit my account.
                // So the pending will show as -100 in my outbox, not 100,
                // because that is the adjustment actually made to my account.
                // This positive/negative aspect of pending transactions is not
                // stored in the data itself, since it switches based
                // on whether the pending appears in the inbox or the outbox.
                // It's based on context. Whereas the transferReceipt is IN
                // REFERENCE TO that same transaction--it appears in my inbox
                // when the recipient accepts the pending transfer
                // I sent him.) Therefore the transferReceipt is "worth" -100
                // (just as the pending in my outbox was "worth" -100), even
                // though its actual value is 0.
                // (Since the transferReceipt itself doesn't change my balance,
                // but merely is a notice that such has happened.) You could
                // say, for example in the SaveAbbreviatedToInbox() function,
                // that the transferReceipt has an "actual value" of 0 and a
                // "display value" of -100 clams, when it is in reference to an
                // original transfer of 100 clams.
                // This function is clearly returning the display value, since
                // the actual value (of 0) is useless, since balance agreements
                // already discount transferReceipts as having any impact on the
                // balance.
                //
                lAdjustment = (pOriginalItem->GetAmount() * (-1));
            }
            break;
        case OTTransaction::pending:  // amount is stored on transfer item

            if (pOriginalItem->GetType() != Item::transfer) {
                otErr << "Wrong item type attached to pending transfer\n";
            } else {
                // Pending transfer adds to my account if this is inbox, and
                // removes if outbox. I'll let the caller multiply by (-1) or
                // not. His choice. Note: Indeed, if you look in
                // ProduceOutboxReportItem(), it is multiplying by (-1).
                lAdjustment = pOriginalItem->GetAmount();
            }
            break;
        case OTTransaction::marketReceipt:  // amount is stored on marketReceipt
                                            // item

            if (pOriginalItem->GetType() != Item::marketReceipt) {
                otErr << "Wrong item type attached to marketReceipt\n";
            } else {
                lAdjustment =
                    pOriginalItem->GetAmount();  // THIS WILL ALSO USE THE
                                                 // POSITIVE / NEGATIVE
                                                 // THING. (Already.)
            }
            break;
        case OTTransaction::paymentReceipt:  // amount is stored on
                                             // paymentReceipt item

            if (pOriginalItem->GetType() != Item::paymentReceipt) {
                otErr << "Wrong item type attached to paymentReceipt\n";
            } else {
                lAdjustment =
                    pOriginalItem->GetAmount();  // THIS WILL ALSO USE THE
                                                 // POSITIVE / NEGATIVE
                                                 // THING. (Already.)
            }
            break;
        case OTTransaction::basketReceipt:  // amount is stored on basketReceipt
                                            // item

            if (pOriginalItem->GetType() != Item::basketReceipt) {
                otErr << "Wrong item type attached to basketReceipt\n";
            } else {
                lAdjustment =
                    pOriginalItem->GetAmount();  // THIS WILL ALSO USE THE
                                                 // POSITIVE / NEGATIVE
                                                 // THING. (Already.)
            }

            break;
        default:  // All other types have no amount -- return 0.
            return 0;
    }

    return lAdjustment;
}

// Need to know the transaction number of the ORIGINAL transaction? Call this.
// virtual
int64_t OTTransaction::GetNumberOfOrigin()
{

    if (0 == m_lNumberOfOrigin) {
        switch (GetType()) {
            case transferReceipt:  // the server drops this into your inbox,
                                   // when someone accepts your transfer.
            case deposit:    // this transaction is a deposit (cash or cheque)
            case atDeposit:  // reply from the server regarding a deposit
                             // request
            case instrumentNotice:  // Receive these in paymentInbox (by way of
                                    // Nymbox), and send in Outpayments.
            case instrumentRejection:  // When someone rejects your invoice, you
                                       // get one of these in YOUR paymentInbox.

                otErr << __FUNCTION__
                      << ": In this case, you can't calculate the "
                         "origin number, you must set it "
                         "explicitly.\n";
                SetNumberOfOrigin(0);  // Not applicable.
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

void OTTransaction::CalculateNumberOfOrigin()
{
    OT_ASSERT(!IsAbbreviated());

    switch (GetType()) {
        case blank:        // freshly issued transaction number, not used yet
        case message:      // A message from one user to another, also in the
                           // nymbox.
        case notice:       // A notice from the server. Used in Nymbox.
        case replyNotice:  // A copy of a server reply to a previous request you
                           // sent. (To make SURE you get the reply.)
        case successNotice:    // A transaction # has successfully been signed
                               // out.
        case processNymbox:    // process nymbox transaction    // comes from
                               // client
        case atProcessNymbox:  // process nymbox reply          // comes from
                               // server
            SetNumberOfOrigin(0);  // Not applicable.
            break;

        case pending:  // Server puts this in your outbox (when sending) and
                       // recipient's inbox.
        case marketReceipt:   // server periodically drops this into your inbox
                              // if an offer is live.
        case paymentReceipt:  // the server drops this into people's inboxes,
                              // every time a payment processes.
        case finalReceipt:    // the server drops this into your in/nym box(es),
                              // when a CronItem expires or is canceled.
        case basketReceipt:   // the server drops this into your inboxes, when a
                              // basket exchange is processed.
            SetNumberOfOrigin(GetReferenceToNum());  // pending is in
                                                     // reference to the
                                                     // original
                                                     // transfer.
            break;

        case transferReceipt:  // the server drops this into your inbox, when
                               // someone accepts your transfer.
        case deposit:          // this transaction is a deposit (cash or cheque)
        case atDeposit:  // reply from the server regarding a deposit request
        case instrumentNotice:     // Receive these in paymentInbox (by way of
                                   // Nymbox), and send in Outpayments.
        case instrumentRejection:  // When someone rejects your invoice, you get
                                   // one of these in YOUR paymentInbox.
            otErr << __FUNCTION__
                  << ": In this case, you can't calculate the "
                     "origin number, you must set it explicitly.\n";
            SetNumberOfOrigin(0);  // Not applicable.
            // Comment this out later so people can't use it to crash the
            // server:
            OT_FAIL_MSG(
                "In this case, you can't calculate the origin number, you "
                "must set it explicitly.");
            break;

        case chequeReceipt:   // the server drops this into your inbox, when
                              // someone deposits your cheque.
        case voucherReceipt:  // the server drops this into your inbox, when
                              // someone deposits your voucher.
        {
            String strReference;
            GetReferenceString(strReference);

            // "In reference to" is the depositor's trans#, which I use here to
            // load
            // the depositor's
            // depositCheque item, which I use to get the cheque, which contains
            // the
            // number of origin
            // as its transaction number.
            //
            std::unique_ptr<Item> pOriginalItem(Item::CreateItemFromString(
                strReference, GetPurportedNotaryID(), GetReferenceToNum()));
            OT_ASSERT(nullptr != pOriginalItem);

            if (Item::depositCheque != pOriginalItem->GetType()) {
                otErr << __FUNCTION__ << ": ERROR: Wrong item type attached to "
                      << ((chequeReceipt == GetType()) ? "chequeReceipt"
                                                       : "voucherReceipt")
                      << " "
                         "(expected OTItem::depositCheque)\n";
                SetNumberOfOrigin(0);
                return;
            }

            SetNumberOfOrigin(pOriginalItem->GetNumberOfOrigin());
        } break;

        case processInbox:  // process inbox transaction    // comes from client
        case atProcessInbox:  // process inbox reply          // comes from
                              // server

        case transfer:  // or "spend". This transaction is a request to transfer
                        // from one account to another
        case atTransfer:  // reply from the server regarding a transfer request

        case withdrawal:  // this transaction is a withdrawal (cash or voucher)
        case atWithdrawal:  // reply from the server regarding a withdrawal
                            // request

        case marketOffer:    // this transaction is a market offer
        case atMarketOffer:  // reply from the server regarding a market offer

        case paymentPlan:    // this transaction is a payment plan
        case atPaymentPlan:  // reply from the server regarding a payment plan

        case smartContract:    // this transaction is a smart contract
        case atSmartContract:  // reply from the server regarding a smart
                               // contract

        case cancelCronItem:  // this transaction is intended to cancel a market
                              // offer or payment plan.
        case atCancelCronItem:  // reply from the server regarding said
                                // cancellation.

        case exchangeBasket:    // this transaction is an exchange in/out of a
                                // basket currency.
        case atExchangeBasket:  // reply from the server regarding said
                                // exchange.

        case payDividend:    // this transaction is dividend payment (to all
                             // shareholders...)
        case atPayDividend:  // reply from the server regarding said dividend
                             // payment.

        default:
            SetNumberOfOrigin(GetTransactionNum());
            break;
    }  // switch
}

/// for display purposes. The "reference #" we show the user is not the same one
/// we used internally.
///
/// The "display reference #" that we want to display for the User might be
/// different depending on the type.
///
/// For example, if pending, then it's in ref to the original transfer request
/// (sender's transaction #)
/// But if chequeReceipt, then it's in reference to the original cheque (also
/// sender's transaction #)
/// But if marketReceipt, then it's in reference to the original market offer
/// (which is my own trans#)
/// But if paymentReceipt, then it's in reference to the original "activate
/// payment plan" request, which may or may not be mine.
///
/// Internally of course, a chequeReceipt is "in reference to" the depositor's
/// deposit request.
/// But the user doesn't care about that number -- he wants to see the original
/// cheque # from when he first
/// wrote it. Thus we have this function for resolving the "display reference #"
/// in cases like that.
///
/// Another example: with market trades, you want the "in reference to" to show
/// the trans# of the original market offer request. Of course, if you load up
/// the item within, you can get the "in reference to" showing a different
/// trans# for EACH TRADE THAT HAS OCCURRED. We use that internally, we need to
/// be able to reference each of those trades. But the user merely wants to see
/// that his receipt is in reference to the original market offer, so he can
/// line up his receipts with his offers. What else does he care?
///
int64_t OTTransaction::GetReferenceNumForDisplay()
{
    if (IsAbbreviated()) return GetAbbrevInRefDisplay();

    int64_t lReferenceNum = 0;

    switch (GetType()) {
        // "in ref to #" is stored on me: GetReferenceToNum()
        case OTTransaction::pending:
        case OTTransaction::notice:
        case OTTransaction::replyNotice:
        case OTTransaction::successNotice:
        case OTTransaction::marketReceipt:
        case OTTransaction::basketReceipt:
        case OTTransaction::instrumentNotice:
        /*
         NOTE: Right about here you might be wondering to yourself, Hmm,
         I wonder why the instrumentNotice returns the GetReferenceToNum.
         I guess I'd think that an instrumentNotice containing an incoming
         cheque should have the cheque# as its "in reference to" number.
         Makes sense, right?

         ...EXCEPT THAT CHEQUE IS ENCRYPTED. The payload on an instrumentNotice
         is encrypted. So unless we had the recipient's private key inside this
         function, which we don't, we have no way of decrypting that cheque and
         returning the "Display" number that the user actually wants to see.

         TODO long term: Add a Nym* parameter so we have the OPTION here to
         decrypt the payload and return the correct data.

         In the meantime, I don't need to change anything here, since
         OTRecordList decrypts the payloads already, and has a pPayment* now
         where I can get the actual instrument's transaction number. So I will
         harvest it in OTRecordList and from there the GUI will have the right
         one.
         */
        case OTTransaction::instrumentRejection:
            lReferenceNum = GetReferenceToNum();
            break;

        case OTTransaction::paymentReceipt:
        case OTTransaction::finalReceipt: {
            lReferenceNum = GetReferenceToNum();

            String strRef;
            GetReferenceString(strRef);
            if (strRef.Exists()) {
                std::unique_ptr<OTCronItem> pCronItem(
                    OTCronItem::NewCronItem(strRef));

                if (pCronItem) {
                    lReferenceNum = pCronItem->GetTransactionNum();
                    // -------------------------------------------
                    OTPaymentPlan* pPlan =
                        dynamic_cast<OTPaymentPlan*>(pCronItem.get());
                    OTSmartContract* pSmartContract =
                        dynamic_cast<OTSmartContract*>(pCronItem.get());

                    if (nullptr != pPlan) {
                        lReferenceNum = pPlan->GetRecipientOpeningNum();
                    } else if (nullptr != pSmartContract) {
                        const std::vector<int64_t>&
                            openingNumsInOrderOfSigning =
                                pSmartContract->openingNumsInOrderOfSigning();

                        if (openingNumsInOrderOfSigning.size() > 0)
                            lReferenceNum = openingNumsInOrderOfSigning[0];
                    }
                    // -------------------------------------------
                }
            }
        } break;

        // A transferReceipt ACTUALLY references the acceptPending (recipient's
        // trans#) that accepted it.
        // But I don't care about the recipient's transaction #s! This function
        // is for DISPLAY. I am the sender, and I want to see a reference to my
        // original transfer that I sent.  This receipt, as far as I care, is
        // for THAT TRANSFER.
        case OTTransaction::transferReceipt:
        case OTTransaction::chequeReceipt:
        case OTTransaction::voucherReceipt:
            lReferenceNum = GetNumberOfOrigin();
            break;

        default
            :  // All other types have no display reference number -- return 0.
            return 0;
    }

    return lReferenceNum;
}

// Decoding and understanding the various subtleties of the marketReceipt
// transaction!!!
//
// For a marketReceipt transaction, the transaction itself carries a NEW
// TRANSACTION ID for EACH RECEIPT.
// I might have many trades process against a single offer. Each time, the
// marketReceipt will be a fresh one,
// with its own fresh transaction number that's owned by the server.
//
// The marketReceipt's "reference to" is for the original Trade, placed by the
// trader, owned by the trader.
//
// 1. pTrans1->SetReferenceToNum(theTrade.GetTransactionNum());
// 2. pTrans1->SetReferenceString(strOrigTrade);
//
// In 2, the Reference String contains the ORIGINAL TRADE, signed by the TRADER.
//
// The marketReceipt transaction is SIGNED by the SERVER, AS IS the
// marketReceipt Item on its list.
// but the original trade was signed by the TRADER. The marketReceipt is in
// REFERENCE to that
// original trade, and so references its number and contains its complete string
// as the reference.
//
// The Item is a marketReceipt item, which is on the "my list of items" for the
// marketReceipt transaction.
// It is signed by the server, and it bears a transaction number that's owned by
// the server.
// The ITEM also contains the AMOUNT for the CURRENT RECEIPT.  If THIS trade
// deducted 50 clams from your
// account, then THIS ITEM will have an AMOUNT of -50 on THIS RECEIPT!

// The item has two attachments... The NOTE, which contains the updated
// (server-signed) TRADE, and
// the ATTACHMENT, which contains the updated (server-signed) OFFER. Both should
// have the same transaction number as pTrans->ReferenceTo().
//
// 3. pItem1->SetNote(strTrade);
// 4. pItem1->SetAttachment(strOffer);
//

bool OTTransaction::GetSenderNymIDForDisplay(Identifier& theReturnID)
{
    if (IsAbbreviated()) return false;

    bool bSuccess = false;

    Item* pOriginalItem = nullptr;
    std::unique_ptr<Item> theItemAngel;

    String strReference;
    GetReferenceString(strReference);

    if (strReference.GetLength() < 2) return false;

    switch (GetType()) {
        case OTTransaction::notice:  // for paymentPlans AND smartcontracts.
        {
            String strUpdatedCronItem;
            Item* pItem = GetItem(Item::notice);

            if (nullptr != pItem)
                pItem->GetNote(strUpdatedCronItem);
            else if (strReference.Exists())
                strUpdatedCronItem = strReference;  // Here we make-do with the
                                                    // original version of the
                                                    // cron item, instead of the
                                                    // updated version.
            else {
                otErr << "OTTransaction::" << __FUNCTION__
                      << ": Failed trying to get notice item from notice "
                         "transaction. "
                         "Or couldn't find cron item within that we were "
                         "seeking.\n";
                return false;
            }

            std::unique_ptr<OTCronItem> pCronItem(
                OTCronItem::NewCronItem(strUpdatedCronItem));

            OTSmartContract* pSmart =
                dynamic_cast<OTSmartContract*>(pCronItem.get());

            if (nullptr != pSmart)  // if it's a smart contract...
            {
                if (!pSmart->GetLastSenderNymID().Exists()) return false;

                // WARNING: This may not be correct. I believe
                // GetLastSenderNymID refers to the most recent Nym who has PAID
                // the smart contract, versus the most recent Nym who has SENT
                // the smart contract. So later on, if you have trouble and find
                // yourself reading this comment, that's why!
                //
                // You might ask yourself, then why is it coded this way?
                // Because this code was just copied from the paymentReceipt
                // case block just below here.

                theReturnID.SetString(pSmart->GetLastSenderNymID());
                return !theReturnID.IsEmpty();
            } else if (nullptr != pCronItem)  // else if it is any other kind of
                                              // cron item...
            {
                theReturnID = pCronItem->GetSenderNymID();
                return !theReturnID.IsEmpty();
            } else {
                otErr << "OTTransaction::" << __FUNCTION__
                      << ": Unable to load Cron Item. Should never happen. "
                         "Receipt: "
                      << GetTransactionNum()
                      << "  Origin: " << GetNumberOfOrigin() << "\n";
                return false;
            }
            break;
        }

        case OTTransaction::paymentReceipt:  // for paymentPlans AND
                                             // smartcontracts. (If the smart
                                             // contract does a payment, it
                                             // leaves a paymentReceipt...)
        {
            String strUpdatedCronItem;
            Item* pItem = GetItem(Item::paymentReceipt);

            if (nullptr != pItem)
                pItem->GetAttachment(strUpdatedCronItem);
            else {
                otErr << "OTTransaction::" << __FUNCTION__
                      << ": Failed trying to get paymentReceipt item from "
                         "paymentReceipt transaction.\n";
                return false;
            }

            std::unique_ptr<OTCronItem> pCronItem(
                OTCronItem::NewCronItem(strUpdatedCronItem));

            OTSmartContract* pSmart =
                dynamic_cast<OTSmartContract*>(pCronItem.get());

            if (nullptr != pSmart)  // if it's a smart contract...
            {
                if (!pSmart->GetLastSenderNymID().Exists()) return false;

                theReturnID.SetString(pSmart->GetLastSenderNymID());
                return !theReturnID.IsEmpty();
            } else if (nullptr != pCronItem)  // else if it is any other kind of
                                              // cron item...
            {
                theReturnID = pCronItem->GetSenderNymID();
                return !theReturnID.IsEmpty();
            } else {
                otErr << "OTTransaction::" << __FUNCTION__
                      << ": Unable to load Cron Item. Should never happen. "
                         "Receipt: "
                      << GetTransactionNum()
                      << "  Origin: " << GetNumberOfOrigin() << "\n";
                return false;
            }
            break;
        }

        case OTTransaction::instrumentNotice: {
            /*
             Therefore, if I am looping through my Nymbox, iterating through
             transactions, and one of them
             is an *** instrumentNotice *** then I should expect
             GetReferenceString(strOutput) to:

             1. load up from string as an OTMessage of type "sendNymInstrument",
             -------------------------------------------------------------------
             2. and I should expect the PAYLOAD of that message to contain an
             encrypted OTEnvelope,
             3. which can be decrypted by Msg.m_strNymID2's private key,
             4. And the resulting plaintext can be loaded into memory as an
             OTPayment object,
             5. ...which contains an instrument of ambiguous type.
             -------------------------------------------------------------------
             FYI:
             OTPayment provides a consistent interface, and consolidation of
             handling, for
             the several financial instruments that appear on the PAYMENTS page,
             in the PaymentsInbox. For example: [ Cheques, Invoices, Vouchers ],
             Payment Plans, Smart Contracts, ...and Purses.
             -------------------------------------------------------------------
             (In this block we don't need to go any farther than step 1 above.)
             -------------------------------------------------------------------
             */
            //          OTString strReference;              // (Already done
            //          above.) GetReferenceString(strReference);   // (Already
            //          done above.)

            Message theSentMsg;

            if (strReference.Exists() &&
                theSentMsg.LoadContractFromString(strReference)) {
                // All we need is this message itself. We aren't going to
                // decrypt the payload, we're just going to grab the
                // sender/receiver data from the msg.
                //
                // We can't decrypt the payload (the OTPayment object) but we
                // still have sender / recipient. todo security need to consider
                // security implications of that and maybe improve it a bit.
                // (But I do NOT want to get into the messaging business.)

                if (theSentMsg.m_strNymID.Exists()) {
                    theReturnID.SetString(theSentMsg.m_strNymID);
                    return true;
                }
            }
            return false;
        }

        case OTTransaction::pending:
        case OTTransaction::chequeReceipt:
        case OTTransaction::voucherReceipt: {
            pOriginalItem = Item::CreateItemFromString(
                strReference, GetPurportedNotaryID(), GetReferenceToNum());

            if (nullptr != pOriginalItem) theItemAngel.reset(pOriginalItem);

            break;
        }

        default:  // All other types are irrelevant here.
            return false;
    }

    if (nullptr == pOriginalItem) {
        otErr << "OTTransaction::GetSenderNymIDForDisplay: original item not "
                 "found. Should never happen.\n";
        return false;  // Should never happen, since we always expect one based
                       // on the transaction type.
    }

    switch (GetType()) {  // These are the types that have an amount (somehow)
        case OTTransaction::chequeReceipt:   // amount is stored on cheque
                                             // (attached to depositCheque item,
                                             // attached.)
        case OTTransaction::voucherReceipt:  // amount is stored on voucher
                                             // (attached to depositCheque item,
                                             // attached.)
        {
            if (pOriginalItem->GetType() != Item::depositCheque) {
                otErr << __FUNCTION__ << ": Wrong item type attached to "
                      << ((OTTransaction::chequeReceipt == GetType())
                              ? "chequeReceipt"
                              : "voucherReceipt")
                      << " (expected depositCheque)\n";
                return false;
            }

            Cheque theCheque;
            String strAttachment;

            // Get the cheque from the Item and load it up into a Cheque object.
            pOriginalItem->GetAttachment(strAttachment);
            bool bLoadContractFromString =
                theCheque.LoadContractFromString(strAttachment);

            if (!bLoadContractFromString) {
                String strCheque(theCheque);

                otErr << "ERROR loading cheque or voucher from string in "
                         "OTTransaction::"
                      << __FUNCTION__ << ":\n"
                      << strCheque << "\n";
            } else {
                if (OTTransaction::chequeReceipt == GetType())
                    theReturnID = theCheque.GetSenderNymID();
                else
                    theReturnID = theCheque.GetRemitterNymID();

                bSuccess = true;
            }
        } break;

        case OTTransaction::pending:  // amount is stored on transfer item

            if (pOriginalItem->GetType() != Item::transfer) {
                otErr << "Wrong item type attached to pending transfer\n";
            } else {
                theReturnID = pOriginalItem->GetNymID();
                bSuccess = true;
            }
            break;
        default:  // All other types have no sender user ID -- return false.
            return false;
    }

    return bSuccess;
}

bool OTTransaction::GetRecipientNymIDForDisplay(Identifier& theReturnID)
{
    if (IsAbbreviated()) return false;

    bool bSuccess = false;

    Item* pOriginalItem = nullptr;
    std::unique_ptr<Item> theItemAngel;

    String strReference;
    GetReferenceString(strReference);

    switch (GetType()) {
        case OTTransaction::notice:  // Used for paymentPlans AND for smart
                                     // contracts...
        {
            String strUpdatedCronItem;
            Item* pItem = GetItem(Item::notice);

            if (nullptr != pItem)
                pItem->GetNote(strUpdatedCronItem);
            else if (strReference.Exists())
                strUpdatedCronItem =
                    strReference;  // Better than  nothing. This is the original
                                   // version of the payment plan, instead of
                                   // the updated verison. (Or smart contract.)
            else {
                otErr << "OTTransaction::" << __FUNCTION__
                      << ": Failed trying to get notice item from notice "
                         "transaction, and couldn't find the instrument this "
                         "notice is about.\n";
                return false;
            }

            std::unique_ptr<OTCronItem> pCronItem(
                OTCronItem::NewCronItem(strUpdatedCronItem));

            OTSmartContract* pSmart =
                dynamic_cast<OTSmartContract*>(pCronItem.get());
            OTPaymentPlan* pPlan =
                dynamic_cast<OTPaymentPlan*>(pCronItem.get());

            if (nullptr != pSmart)  // if it's a smart contract...
            {
                if (!pSmart->GetLastRecipientNymID().Exists()) return false;

                // WARNING: This might not be appropriate for a ::notice.
                // GetLastRecipientNymID I believe refers to the last Nym to
                // receive FUNDS from the smart contract, which is not the same
                // thing as the last Nym to receive a COPY of the smart
                // contract. So if this causes problems later on, this comment
                // will be here to guide you  :-) So why is this code like this
                // in the first place? Simple: I just copied it from the
                // paymentReceipt case below.

                theReturnID.SetString(pSmart->GetLastRecipientNymID());
                return !theReturnID.IsEmpty();
            } else if (nullptr != pPlan)  // else if it is a payment plan...
            {
                theReturnID = pPlan->GetRecipientNymID();
                return !theReturnID.IsEmpty();
            } else {
                otErr << "OTTransaction::" << __FUNCTION__
                      << ": Unable to load Cron Item. Should never happen. "
                         "Receipt: "
                      << GetTransactionNum()
                      << "  Origin: " << GetNumberOfOrigin() << "\n";
                return false;
            }
        } break;  // this break never actually happens. Above always returns, if
                  // triggered.

        case OTTransaction::paymentReceipt:  // Used for paymentPlans AND for
                                             // smart contracts...
        {
            String strUpdatedCronItem;
            Item* pItem = GetItem(Item::paymentReceipt);

            if (nullptr != pItem)
                pItem->GetAttachment(strUpdatedCronItem);
            else {
                otErr << "OTTransaction::" << __FUNCTION__
                      << ": Failed trying to get paymentReceipt item from "
                         "paymentReceipt transaction.\n";
                return false;
            }

            std::unique_ptr<OTCronItem> pCronItem(
                OTCronItem::NewCronItem(strUpdatedCronItem));

            OTSmartContract* pSmart =
                dynamic_cast<OTSmartContract*>(pCronItem.get());
            OTPaymentPlan* pPlan =
                dynamic_cast<OTPaymentPlan*>(pCronItem.get());

            if (nullptr != pSmart)  // if it's a smart contract...
            {
                if (!pSmart->GetLastRecipientNymID().Exists()) return false;

                theReturnID.SetString(pSmart->GetLastRecipientNymID());
                return !theReturnID.IsEmpty();
            } else if (nullptr != pPlan)  // else if it is a payment plan...
            {
                theReturnID = pPlan->GetRecipientNymID();
                return !theReturnID.IsEmpty();
            } else {
                otErr << "OTTransaction::" << __FUNCTION__
                      << ": Unable to load Cron Item. Should never happen. "
                         "Receipt: "
                      << GetTransactionNum()
                      << "  Origin: " << GetNumberOfOrigin() << "\n";
                return false;
            }
        } break;  // this break never actually happens. Above always returns, if
                  // triggered.

        case OTTransaction::instrumentNotice: {
            /*
             Therefore, if I am looping through my Nymbox, iterating through
             transactions, and one of them is an *** instrumentNotice *** then I
             should expect GetReferenceString(strOutput) to:

             1. load up from string as an OTMessage of type "sendNymInstrument",
             -------------------------------------------------------------------
             2. and I should expect the PAYLOAD of that message to contain an
                encrypted OTEnvelope,
             3. which can be decrypted by Msg.m_strNymID2's private key,
             4. And the resulting plaintext can be loaded into memory as an
                OTPayment object,
             5. ...which contains an instrument of ambiguous type.
             -------------------------------------------------------------------
             FYI:
             OTPayment provides a consistent interface, and consolidation of
             handling, for the several financial instruments that appear on the
             PAYMENTS page, in the PaymentsInbox. For example: [ Cheques,
             Invoices, Vouchers ], Payment Plans, Smart Contracts, ...and
             Purses.
             -------------------------------------------------------------------
             (In this block we don't need to go any farther than step 1 above.)
             -------------------------------------------------------------------
             */

            Message theSentMsg;

            if (strReference.Exists() &&
                theSentMsg.LoadContractFromString(strReference)) {
                // All we need is this message itself. We aren't going to
                // decrypt the payload, we're just going to grab the
                // sender/receiver data from the msg.
                //
                // We can't decrypt the payload (the OTPayment object) but we
                // still have sender / recipient. todo security need to consider
                // security implications of that and maybe improve it a bit.
                // (But I do NOT want to get into the messaging business.)

                if (theSentMsg.m_strNymID2.Exists()) {
                    theReturnID.SetString(theSentMsg.m_strNymID2);
                    return true;
                }
            }
            return false;
        } break;

        case OTTransaction::transferReceipt:
        case OTTransaction::chequeReceipt:
        case OTTransaction::voucherReceipt: {
            pOriginalItem = Item::CreateItemFromString(
                strReference, GetPurportedNotaryID(), GetReferenceToNum());

            if (nullptr != pOriginalItem) theItemAngel.reset(pOriginalItem);

            break;
        }
        default:  // All other types have no amount -- return false.
            return false;
    }
    // -------------------------------------------------------
    if (nullptr == pOriginalItem)
        return false;  // Should never happen, since we always expect one based
                       // on the transaction type.
    // -------------------------------------------------------
    switch (GetType()) {
        case OTTransaction::transferReceipt: {
            if (pOriginalItem->GetType() != Item::acceptPending) {
                otErr << "Wrong item type attached to transferReceipt\n";
                return false;
            } else {
                theReturnID =
                    pOriginalItem->GetNymID();  // Even though a transfer
                                                // has no recipient user
                                                // (just a recipient acct)
                                                // I still get the Nym ID
                                                // when he accepts it!
                bSuccess = true;
            }
        } break;

        case OTTransaction::chequeReceipt:   // amount is stored on cheque
                                             // (attached to depositCheque item,
                                             // attached.)
        case OTTransaction::voucherReceipt:  // amount is stored on voucher
                                             // (attached to depositCheque item,
                                             // attached.)
        {
            if (pOriginalItem->GetType() != Item::depositCheque) {
                otErr << __FUNCTION__ << ": Wrong item type attached to "
                      << ((OTTransaction::chequeReceipt == GetType())
                              ? "chequeReceipt"
                              : "voucherReceipt")
                      << " (expected depositCheque)\n";
                return false;
            }

            Cheque theCheque;
            String strAttachment;

            // Get the cheque from the Item and load it up into a Cheque object.
            pOriginalItem->GetAttachment(strAttachment);
            bool bLoadContractFromString =
                theCheque.LoadContractFromString(strAttachment);

            if (!bLoadContractFromString) {
                String strCheque(theCheque);

                otErr << "ERROR loading cheque or voucher from string in "
                         "OTTransaction::"
                      << __FUNCTION__ << ":\n"
                      << strCheque << "\n";
            } else if (theCheque.HasRecipient()) {
                theReturnID = theCheque.GetRecipientNymID();
                bSuccess = true;
            } else {
                theReturnID =
                    pOriginalItem->GetNymID();  // Even though the cheque has no
                                                // recipient, I still get the
                                                // Nym ID when he deposits it!
                bSuccess = true;
            }
        } break;

        default:  // All other types have no recipient user ID -- return false.
            return false;
    }

    return bSuccess;
}

bool OTTransaction::GetSenderAcctIDForDisplay(Identifier& theReturnID)
{
    if (IsAbbreviated()) return false;

    bool bSuccess = false;

    Item* pOriginalItem = nullptr;
    std::unique_ptr<Item> theItemAngel;

    String strReference;
    GetReferenceString(strReference);

    if (strReference.GetLength() < 2) return false;

    switch (GetType()) {
        case OTTransaction::paymentReceipt: {
            String strUpdatedCronItem;
            Item* pItem = GetItem(Item::paymentReceipt);

            if (nullptr != pItem)
                pItem->GetAttachment(strUpdatedCronItem);
            else
                otErr << "OTTransaction::" << __FUNCTION__
                      << ": Failed trying to get paymentReceipt item from "
                         "paymentReceipt transaction.\n";

            std::unique_ptr<OTCronItem> pCronItem(
                OTCronItem::NewCronItem(strUpdatedCronItem));

            OTSmartContract* pSmart =
                dynamic_cast<OTSmartContract*>(pCronItem.get());

            if (nullptr != pSmart)  // if it's a smart contract...
            {
                if (!pSmart->GetLastSenderAcctID().Exists()) return false;

                theReturnID.SetString(pSmart->GetLastSenderAcctID());
                return true;
            } else if (nullptr != pCronItem)  // else if it is any other kind of
                                              // cron item...
            {
                theReturnID = pCronItem->GetSenderAcctID();
                return true;
            } else {
                otErr << "OTTransaction::" << __FUNCTION__
                      << ": Unable to load Cron Item. Should never happen. "
                         "Receipt: "
                      << GetTransactionNum()
                      << "  Origin: " << GetNumberOfOrigin() << "\n";
                return false;
            }
        } break;
        case OTTransaction::pending:  // amount is stored on the transfer item,
                                      // on my list of items.
        case OTTransaction::chequeReceipt:   // amount is stored on cheque
                                             // (attached to depositCheque item,
                                             // attached.)
        case OTTransaction::voucherReceipt:  // amount is stored on voucher
                                             // (attached to depositCheque item,
                                             // attached.)
        {
            pOriginalItem = Item::CreateItemFromString(
                strReference, GetPurportedNotaryID(), GetReferenceToNum());

            if (nullptr != pOriginalItem) theItemAngel.reset(pOriginalItem);

            break;
        }
        default:  // All other types have no sender acct ID -- return false.
            return false;
    }

    if (nullptr == pOriginalItem) {
        otErr << "OTTransaction::" << __FUNCTION__
              << ": couldn't load original item, should never happen. \n";
        return false;  // Should never happen, since we always expect one based
                       // on the transaction type.
    }

    switch (GetType()) {  // These are the types that have an amount (somehow)
        case OTTransaction::chequeReceipt:   // amount is stored on cheque
                                             // (attached to depositCheque item,
                                             // attached.)
        case OTTransaction::voucherReceipt:  // amount is stored on voucher
                                             // (attached to depositCheque item,
                                             // attached.)
        {
            if (pOriginalItem->GetType() != Item::depositCheque) {
                otErr << __FUNCTION__ << ": Wrong item type attached to "
                      << ((OTTransaction::chequeReceipt == GetType())
                              ? "chequeReceipt"
                              : "voucherReceipt")
                      << " (expected depositCheque)\n";
                return false;
            }

            Cheque theCheque;
            String strAttachment;

            // Get the cheque from the Item and load it up into a Cheque object.
            pOriginalItem->GetAttachment(strAttachment);
            bool bLoadContractFromString =
                theCheque.LoadContractFromString(strAttachment);

            if (!bLoadContractFromString) {
                String strCheque(theCheque);

                otErr << "ERROR loading cheque from string in OTTransaction::"
                      << __FUNCTION__ << ":\n"
                      << strCheque << "\n";
            } else {
                if (OTTransaction::chequeReceipt == GetType())
                    theReturnID = theCheque.GetSenderAcctID();
                else
                    theReturnID = theCheque.GetRemitterAcctID();

                bSuccess = true;
            }
        } break;

        case OTTransaction::pending:  // amount is stored on transfer item

            if (pOriginalItem->GetType() != Item::transfer) {
                otErr << "Wrong item type attached to pending transfer\n";
            } else {
                theReturnID = pOriginalItem->GetPurportedAccountID();
                bSuccess = true;
            }
            break;

        default:  // All other types have no amount -- return 0.
            return false;
    }

    return bSuccess;
}

bool OTTransaction::GetRecipientAcctIDForDisplay(Identifier& theReturnID)
{
    if (IsAbbreviated()) return false;

    bool bSuccess = false;

    Item* pOriginalItem = nullptr;
    std::unique_ptr<Item> theItemAngel;

    String strReference;
    GetReferenceString(strReference);

    switch (GetType()) {
        case OTTransaction::paymentReceipt: {
            String strUpdatedCronItem;
            Item* pItem = GetItem(Item::paymentReceipt);

            if (nullptr != pItem)
                pItem->GetAttachment(strUpdatedCronItem);
            else
                otErr << "OTTransaction::" << __FUNCTION__
                      << ": Failed trying to get paymentReceipt item from "
                         "paymentReceipt transaction.\n";

            std::unique_ptr<OTCronItem> pCronItem(
                OTCronItem::NewCronItem(strUpdatedCronItem));

            OTSmartContract* pSmart =
                dynamic_cast<OTSmartContract*>(pCronItem.get());
            OTPaymentPlan* pPlan =
                dynamic_cast<OTPaymentPlan*>(pCronItem.get());

            if (nullptr != pSmart)  // if it's a smart contract...
            {
                if (!pSmart->GetLastRecipientAcctID().Exists()) return false;

                theReturnID.SetString(pSmart->GetLastRecipientAcctID());
                return true;
            } else if (nullptr != pPlan)  // else if it's a payment plan.
            {
                theReturnID = pPlan->GetRecipientAcctID();
                return true;
            } else  // else if it is any other kind of cron item...
            {
                otErr << "OTTransaction::" << __FUNCTION__
                      << ": Unable to load Cron Item. Should never happen. "
                         "Receipt: "
                      << GetTransactionNum()
                      << "  Origin: " << GetNumberOfOrigin() << "\n";
                return false;
            }
        } break;  // this break never actually happens. Above always returns, if
                  // triggered.

        case OTTransaction::pending:
        case OTTransaction::transferReceipt:
        case OTTransaction::chequeReceipt:
        case OTTransaction::voucherReceipt: {
            pOriginalItem = Item::CreateItemFromString(
                strReference, GetPurportedNotaryID(), GetReferenceToNum());

            if (nullptr != pOriginalItem) theItemAngel.reset(pOriginalItem);

            break;
        }
        default:  // All other types have no amount -- return 0.
            return false;
    }

    if (nullptr == pOriginalItem)
        return false;  // Should never happen, since we always expect one based
                       // on the transaction type.

    Cheque theCheque;  // allocated on the stack :-)
    String strAttachment;

    switch (GetType()) {
        case OTTransaction::transferReceipt: {
            if (pOriginalItem->GetType() != Item::acceptPending) {
                otErr << "Wrong item type attached to transferReceipt\n";
                return false;
            } else {
                theReturnID = pOriginalItem->GetPurportedAccountID();
                bSuccess = true;
            }
        } break;

        case OTTransaction::chequeReceipt:
        case OTTransaction::voucherReceipt: {
            if (pOriginalItem->GetType() != Item::depositCheque) {
                otErr << __FUNCTION__ << ": Wrong item type attached to "
                      << ((OTTransaction::chequeReceipt == GetType())
                              ? "chequeReceipt"
                              : "voucherReceipt")
                      << " (expected depositCheque)\n";
                return false;
            } else {
                theReturnID =
                    pOriginalItem->GetPurportedAccountID();  // Here's the
                // depositor's account
                // ID (even though the
                // cheque was made out
                // to a user, not an
                // account, it still
                // eventually had to be
                // DEPOSITED into an
                // account... right?)
                bSuccess = true;
            }
        } break;

        case OTTransaction::pending:  // amount is stored on transfer item

            if (pOriginalItem->GetType() != Item::transfer) {
                otErr << "Wrong item type attached to pending transfer\n";
            } else {
                theReturnID = pOriginalItem->GetDestinationAcctID();
                bSuccess = true;
            }
            break;

        default:  // All other types have no amount -- return 0.
            return false;
    }

    return bSuccess;
}

bool OTTransaction::GetMemo(String& strMemo)
{
    if (IsAbbreviated()) return false;

    bool bSuccess = false;

    Item* pOriginalItem = nullptr;
    std::unique_ptr<Item> theItemAngel;

    String strReference;
    GetReferenceString(strReference);

    switch (GetType()) {
        case OTTransaction::paymentReceipt: {
            String strUpdatedCronItem;
            Item* pItem = GetItem(Item::paymentReceipt);

            if (nullptr != pItem)
                pItem->GetAttachment(strUpdatedCronItem);
            else
                otErr << "OTTransaction::" << __FUNCTION__
                      << ": Failed trying to get paymentReceipt item from "
                         "paymentReceipt transaction.\n";

            std::unique_ptr<OTCronItem> pCronItem(
                OTCronItem::NewCronItem(strUpdatedCronItem));

            OTSmartContract* pSmart =
                dynamic_cast<OTSmartContract*>(pCronItem.get());
            OTPaymentPlan* pPlan =
                dynamic_cast<OTPaymentPlan*>(pCronItem.get());

            if (nullptr != pSmart)  // if it's a smart contract...
            {
                // NOTE: smart contracts currently do not have a "memo" field.

                return false;
            } else if (nullptr != pPlan)  // else if it is a payment plan.
            {
                if (pPlan->GetConsideration().Exists())
                    strMemo.Set(pPlan->GetConsideration());

                return true;
            } else  // else if it's any other kind of cron item.
            {
                otErr << "OTTransaction::" << __FUNCTION__
                      << ": Unable to load Cron Item. Should never happen. "
                         "Receipt: "
                      << GetTransactionNum()
                      << "  Origin: " << GetNumberOfOrigin() << "\n";
                return false;
            }
        } break;  // this break never actually happens. Above always returns, if
                  // triggered.

        case OTTransaction::pending:
        case OTTransaction::transferReceipt:
        case OTTransaction::chequeReceipt:
        case OTTransaction::voucherReceipt: {
            pOriginalItem = Item::CreateItemFromString(
                strReference, GetPurportedNotaryID(), GetReferenceToNum());

            if (nullptr != pOriginalItem) theItemAngel.reset(pOriginalItem);

            break;
        }
        default:
            return false;
    }

    if (nullptr == pOriginalItem)
        return false;  // Should never happen, since we always expect one based
                       // on the transaction type.

    switch (GetType()) {
        case OTTransaction::transferReceipt: {
            if (pOriginalItem->GetType() != Item::acceptPending) {
                otErr << __FUNCTION__
                      << ": Wrong item type attached to transferReceipt\n";
                return false;
            } else {
                pOriginalItem->GetNote(strMemo);
                bSuccess = strMemo.Exists();
            }
        } break;

        case OTTransaction::chequeReceipt:
        case OTTransaction::voucherReceipt: {
            if (pOriginalItem->GetType() != Item::depositCheque) {
                otErr << __FUNCTION__ << ": Wrong item type attached to "
                      << ((OTTransaction::chequeReceipt == GetType())
                              ? "chequeReceipt"
                              : "voucherReceipt")
                      << " (expected depositCheque)\n";
                return false;
            } else {
                Cheque theCheque;
                String strCheque;
                pOriginalItem->GetAttachment(strCheque);

                if (!((strCheque.GetLength() > 2) &&
                      theCheque.LoadContractFromString(strCheque))) {
                    otErr << __FUNCTION__
                          << ": Error loading cheque or voucher from string:\n"
                          << strCheque << "\n";
                    return false;
                }

                // Success loading the cheque.
                strMemo = theCheque.GetMemo();
                bSuccess = strMemo.Exists();
            }
        } break;

        case OTTransaction::pending:

            if (pOriginalItem->GetType() != Item::transfer) {
                otErr << __FUNCTION__
                      << ": Wrong item type attached to pending transfer\n";
            } else {
                pOriginalItem->GetNote(strMemo);
                bSuccess = strMemo.Exists();
            }
            break;

        default:
            return false;
    }

    return bSuccess;
}

}  // namespace opentxs
