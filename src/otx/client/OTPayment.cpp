// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                       // IWYU pragma: associated
#include "1_Internal.hpp"                     // IWYU pragma: associated
#include "internal/otx/client/OTPayment.hpp"  // IWYU pragma: associated

#include <chrono>
#include <compare>
#include <cstdint>
#include <memory>

#include "internal/api/session/FactoryAPI.hpp"
#include "internal/otx/Types.hpp"
#include "internal/otx/common/Cheque.hpp"
#include "internal/otx/common/Contract.hpp"
#include "internal/otx/common/Item.hpp"
#include "internal/otx/common/NumList.hpp"
#include "internal/otx/common/OTTrackable.hpp"
#include "internal/otx/common/OTTransaction.hpp"
#include "internal/otx/common/OTTransactionType.hpp"
#include "internal/otx/common/StringXML.hpp"
#include "internal/otx/common/XML.hpp"
#include "internal/otx/common/recurring/OTPaymentPlan.hpp"
#include "internal/otx/common/util/Tag.hpp"
#include "internal/otx/smartcontract/OTSmartContract.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace opentxs
{
// NOLINTNEXTLINE(modernize-avoid-c-arrays)
char const* const TypeStringsPayment[] = {

    // OTCheque is derived from OTTrackable, which is derived from OTInstrument,
    // which is
    // derived from OTScriptable, which is derived from Contract.
    "CHEQUE",   // A cheque drawn on a user's account.
    "VOUCHER",  // A cheque drawn on a server account (cashier's cheque aka
                // banker's cheque)
    "INVOICE",  // A cheque with a negative amount. (Depositing this causes a
                // payment out, instead of a deposit in.)
    "PAYMENT PLAN",   // An OTCronItem-derived OTPaymentPlan, related to a
                      // recurring payment plan.
    "SMARTCONTRACT",  // An OTCronItem-derived OTSmartContract, related to a
                      // smart contract.
    "NOTICE",  // An OTTransaction containing a notice that a cron item was
               // activated/canceled.
    // NOTE: Even though a notice isn't a "payment instrument" it can still be
    // found
    // in the Nym's record box, where all his received payments are moved once
    // they
    // are deposited. Interestingly though, I believe those are all RECEIVED,
    // except
    // for the notices, which are SENT. (Well, the notice was actually received
    // from
    // the server, BUT IN REFERENCE TO something that had been sent, and thus
    // the outgoing
    // payment is removed when the notice is received into the record box.
    "ERROR_STATE"};

OTPayment::OTPayment(const api::Session& api)
    : Contract(api)
    , payment_(String::Factory())
    , type_(OTPayment::ERROR_STATE)
    , are_temp_values_set_(false)
    , has_recipient_(false)
    , has_remitter_(false)
    , amount_(0)
    , transaction_num_(0)
    , trans_num_display_(0)
    , memo_(String::Factory())
    , instrument_definition_id_()
    , notary_id_()
    , sender_nym_id_()
    , sender_account_id_()
    , recipient_nym_id_()
    , recipient_account_id_()
    , remitter_nym_id_()
    , remitter_account_id_()
    , valid_from_()
    , valid_to_()
{
    InitPayment();
}

OTPayment::OTPayment(const api::Session& api, const String& strPayment)
    : Contract(api)
    , payment_(String::Factory())
    , type_(OTPayment::ERROR_STATE)
    , are_temp_values_set_(false)
    , has_recipient_(false)
    , has_remitter_(false)
    , amount_(0)
    , transaction_num_(0)
    , trans_num_display_(0)
    , memo_(String::Factory())
    , instrument_definition_id_()
    , notary_id_()
    , sender_nym_id_()
    , sender_account_id_()
    , recipient_nym_id_()
    , recipient_account_id_()
    , remitter_nym_id_()
    , remitter_account_id_()
    , valid_from_()
    , valid_to_()
{
    InitPayment();
    SetPayment(strPayment);
}

// static
auto OTPayment::GetTypeString(paymentType theType) -> const char*
{
    auto nType = static_cast<std::int32_t>(theType);
    return TypeStringsPayment[nType];
}

auto OTPayment::GetTypeFromString(const String& strType)
    -> OTPayment::paymentType
{
#define OT_NUM_ELEM(blah) (sizeof(blah) / sizeof(*(blah)))
    for (std::uint32_t i = 0; i < (OT_NUM_ELEM(TypeStringsPayment) - 1); i++) {
        if (strType.Compare(TypeStringsPayment[i])) {
            return static_cast<OTPayment::paymentType>(i);
        }
    }
#undef OT_NUM_ELEM
    return OTPayment::ERROR_STATE;
}

auto OTPayment::SetTempRecipientNymID(const identifier::Nym& id) -> bool
{
    recipient_nym_id_ = id;

    return true;
}

// Since the temp values are not available until at least ONE instantiating has
// occured,
// this function forces that very scenario (cleanly) so you don't have to
// instantiate-and-
// then-delete a payment instrument. Instead, just call this, and then the temp
// values will
// be available thereafter.
//
auto OTPayment::SetTempValues(const PasswordPrompt& reason)
    -> bool  // This version for
             // OTTrackable
{
    if (OTPayment::NOTICE == type_) {
        // Perform instantiation of a notice (OTTransaction), then use it to set
        // the temp values, then clean it up again before returning
        // success/fail.
        //
        std::unique_ptr<OTTransaction> pNotice(InstantiateNotice());

        if (!pNotice) {
            LogError()(OT_PRETTY_CLASS())(
                "Error: Failed instantiating "
                "OTPayment (purported notice) contents: ")(payment_.get())(".")
                .Flush();
            return false;
        }

        return SetTempValuesFromNotice(*pNotice, reason);
    } else {
        OTTrackable* pTrackable = Instantiate();

        if (nullptr == pTrackable) {
            LogError()(OT_PRETTY_CLASS())(
                "Error: Failed instantiating "
                "OTPayment contents: ")(payment_.get())(".")
                .Flush();
            return false;
        }
        // BELOW THIS POINT, MUST DELETE pTrackable!
        std::unique_ptr<OTTrackable> theTrackableAngel(pTrackable);

        Cheque* pCheque = nullptr;
        OTPaymentPlan* pPaymentPlan = nullptr;
        OTSmartContract* pSmartContract = nullptr;

        switch (type_) {
            case CHEQUE:
            case VOUCHER:
            case INVOICE: {
                pCheque = dynamic_cast<Cheque*>(pTrackable);
                if (nullptr == pCheque) {
                    LogError()(OT_PRETTY_CLASS())(
                        "Failure: "
                        "dynamic_cast<OTCheque *>(pTrackable). Contents: ")(
                        payment_.get())(".")
                        .Flush();
                    // Let's grab all the temp values from the cheque!!
                    //
                } else {  // success
                    return SetTempValuesFromCheque(*pCheque);
                }
            } break;
            case PAYMENT_PLAN: {
                pPaymentPlan = dynamic_cast<OTPaymentPlan*>(pTrackable);
                if (nullptr == pPaymentPlan) {
                    LogError()(OT_PRETTY_CLASS())(
                        "Failure: "
                        "dynamic_cast<OTPaymentPlan *>(pTrackable). "
                        "Contents: ")(payment_.get())(".")
                        .Flush();
                    // Let's grab all the temp values from the payment plan!!
                    //
                } else {  // success
                    return SetTempValuesFromPaymentPlan(*pPaymentPlan);
                }
            } break;
            case SMART_CONTRACT: {
                pSmartContract = dynamic_cast<OTSmartContract*>(pTrackable);
                if (nullptr == pSmartContract) {
                    LogError()(OT_PRETTY_CLASS())(
                        "Failure: "
                        "dynamic_cast<OTSmartContract *>(pTrackable). "
                        "Contents: ")(payment_.get())(".")
                        .Flush();
                    // Let's grab all the temp values from the smart contract!!
                    //
                } else {  // success
                    return SetTempValuesFromSmartContract(*pSmartContract);
                }
            } break;
            case NOTICE:
            case ERROR_STATE:
            default: {
                LogError()(OT_PRETTY_CLASS())("Failure: Wrong type_. "
                                              "Contents: ")(payment_.get())(".")
                    .Flush();
                return false;
            }
        }
    }

    return false;  // Should never actually reach this point.
}

auto OTPayment::SetTempValuesFromCheque(const Cheque& theInput) -> bool
{
    switch (type_) {
        case OTPayment::CHEQUE:
        case OTPayment::VOUCHER:
        case OTPayment::INVOICE: {
            are_temp_values_set_ = true;

            amount_ = theInput.GetAmount();
            transaction_num_ = theInput.GetTransactionNum();
            trans_num_display_ = transaction_num_;

            if (theInput.GetMemo().Exists()) {
                memo_->Set(theInput.GetMemo());
            } else {
                memo_->Release();
            }

            instrument_definition_id_ = theInput.GetInstrumentDefinitionID();
            notary_id_ = theInput.GetNotaryID();

            sender_nym_id_ = theInput.GetSenderNymID();
            sender_account_id_ = theInput.GetSenderAcctID();

            if (theInput.HasRecipient()) {
                has_recipient_ = true;
                recipient_nym_id_ = theInput.GetRecipientNymID();
            } else {
                has_recipient_ = false;
                recipient_nym_id_.clear();
            }

            if (theInput.HasRemitter()) {
                has_remitter_ = true;
                remitter_nym_id_ = theInput.GetRemitterNymID();
                remitter_account_id_ = theInput.GetRemitterAcctID();
            } else {
                has_remitter_ = false;
                remitter_nym_id_.clear();
                remitter_account_id_.clear();
            }

            // NOTE: the "Recipient Acct" is NOT KNOWN when cheque is written,
            // but only once the cheque gets deposited. Therefore if type is
            // CHEQUE, then Recipient Acct ID is not set, and attempts to read
            // it will result in failure.
            recipient_account_id_.clear();

            valid_from_ = theInput.GetValidFrom();
            valid_to_ = theInput.GetValidTo();

            return true;
        }
        case OTPayment::ERROR_STATE:
        case OTPayment::PAYMENT_PLAN:
        case OTPayment::SMART_CONTRACT:
        case OTPayment::NOTICE:
        default:
            LogError()(OT_PRETTY_CLASS())("Error: Wrong type. "
                                          "(Returning false).")
                .Flush();
            break;
    }

    return false;
}

auto OTPayment::SetTempValuesFromNotice(
    const OTTransaction& theInput,
    const PasswordPrompt& reason) -> bool
{
    if (OTPayment::NOTICE == type_) {
        are_temp_values_set_ = true;
        has_recipient_ = true;
        has_remitter_ = false;
        // -------------------------------------------
        auto strCronItem = String::Factory();

        auto pItem =
            (const_cast<OTTransaction&>(theInput)).GetItem(itemType::notice);

        if (false != bool(pItem)) {       // The item's NOTE, as opposed to the
                                          // transaction's reference string,
            pItem->GetNote(strCronItem);  // contains the updated version of the
        }
        // cron item, versus the original.
        //        else
        //        {
        //            otErr << "DEBUGGING: Failed to get the notice item! Thus
        //            forcing us to grab the old version of the payment plan
        //            instead of the current version.\n";
        //            String strBlah(theInput);
        //            otErr << "THE ACTUAL TRANSACTION:\n\n" << strBlah << "\n";
        //        }
        // -------------------------------------------
        if (!strCronItem->Exists()) {
            theInput.GetReferenceString(strCronItem);  // Didn't find the
        }
        // updated one? Okay
        // let's grab the
        // original then.
        // -------------------------------------------
        if (!strCronItem->Exists()) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed geting reference string (containing cron item) "
                "from instantiated OTPayment: ")(payment_.get())(".")
                .Flush();
            return false;
        }
        // -------------------------------------------
        std::unique_ptr<OTPayment> pCronItemPayment(
            new OTPayment(api_, strCronItem));

        if (!pCronItemPayment || !pCronItemPayment->IsValid() ||
            !pCronItemPayment->SetTempValues(reason)) {
            LogError()(OT_PRETTY_CLASS())(
                "1 Failed instantiating or verifying a "
                "(purported) cron item: ")(strCronItem.get())(".")
                .Flush();
            return false;
        }
        // -------------------------------------------
        OTTrackable* pTrackable = pCronItemPayment->Instantiate();

        if (nullptr == pTrackable) {
            LogError()(OT_PRETTY_CLASS())(
                "2 Failed instantiating or verifying a "
                "(purported) cron item: ")(strCronItem.get())(".")
                .Flush();
            return false;
        }
        std::unique_ptr<OTTrackable> theTrackableAngel(pTrackable);
        // -------------------------------------------
        auto* pPlan = dynamic_cast<OTPaymentPlan*>(pTrackable);
        auto* pSmartContract = dynamic_cast<OTSmartContract*>(pTrackable);

        if (nullptr != pPlan) {
            lowLevelSetTempValuesFromPaymentPlan(*pPlan);
            return true;
        } else if (nullptr != pSmartContract) {
            lowLevelSetTempValuesFromSmartContract(*pSmartContract);
            return true;
        }
        // -------------------------------------------
        LogError()(OT_PRETTY_CLASS())(
            "Error: Apparently it's not a payment plan "
            "or smart contract – but was supposed to be. "
            "(Returning false).")
            .Flush();
    } else {
        LogError()(OT_PRETTY_CLASS())("Error: Wrong type. (Returning false).")
            .Flush();
    }

    return false;
}

void OTPayment::lowLevelSetTempValuesFromPaymentPlan(
    const OTPaymentPlan& theInput)
{
    are_temp_values_set_ = true;
    has_recipient_ = true;
    has_remitter_ = false;

    // There're also regular payments of GetPaymentPlanAmount().
    // Can't fit 'em all.
    amount_ = theInput.GetInitialPaymentAmount();
    transaction_num_ = theInput.GetTransactionNum();
    trans_num_display_ = theInput.GetRecipientOpeningNum();

    if (theInput.GetConsideration().Exists()) {
        memo_->Set(theInput.GetConsideration());
    } else {
        memo_->Release();
    }

    instrument_definition_id_ = theInput.GetInstrumentDefinitionID();
    notary_id_ = theInput.GetNotaryID();

    sender_nym_id_ = theInput.GetSenderNymID();
    sender_account_id_ = theInput.GetSenderAcctID();

    recipient_nym_id_ = theInput.GetRecipientNymID();
    recipient_account_id_ = theInput.GetRecipientAcctID();

    remitter_nym_id_.clear();
    remitter_account_id_.clear();

    valid_from_ = theInput.GetValidFrom();
    valid_to_ = theInput.GetValidTo();
}

auto OTPayment::SetTempValuesFromPaymentPlan(const OTPaymentPlan& theInput)
    -> bool
{
    if (OTPayment::PAYMENT_PLAN == type_) {
        lowLevelSetTempValuesFromPaymentPlan(theInput);
        return true;
    } else {
        LogError()(OT_PRETTY_CLASS())("Error: Wrong type. (Returning false).")
            .Flush();
    }

    return false;
}

void OTPayment::lowLevelSetTempValuesFromSmartContract(
    const OTSmartContract& theInput)
{
    are_temp_values_set_ = true;
    has_recipient_ = false;
    has_remitter_ = false;

    amount_ = 0;  // not used here.
    transaction_num_ = theInput.GetTransactionNum();
    //  trans_num_display_ = theInput.GetTransactionNum();

    // NOTE: ON THE DISPLAY NUMBER!
    // For nearly all instruments, the display number is the transaction
    // number on the instrument.
    // Except for payment plans -- the display number is the recipient's
    // (merchant's) opening number. That's because the merchant has no
    // way of knowing what number the customer will use when the customer
    // activates the contract. Before then it's already in the merchant's
    // outpayments box. So we choose a number (for display) that we know the
    // merchant will know. This way customer and merchant can cross-reference
    // the payment plan in their respective GUIs.
    //
    // BUT WHAT ABOUT SMART CONTRACTS? Not so easy. There is no "sender" and
    // "recipient." Well there's a sender but he's the activator -- that is,
    // the LAST nym who sees the contract before it gets activated. Of course,
    // he actually activated the thing, so his transaction number is its
    // "official" transaction number. But none of the other parties could have
    // anticipated what that number would be when they originally sent their
    // smart contract proposal. So none of them will know how to match that
    // number back up to the original sent contract (that's still sitting in
    // each party's outpayments box!)
    //
    // This is a conundrum. What can we do? Really we have to calculate the
    // display number outside of this class. (Even though we had to do it INSIDE
    // for the payment plan.)
    //
    // When the first party sends a smart contract, his transaction numbers are
    // on it, but once 3 or 4 parties have sent it along, there's no way of
    // telling
    // which party was the first signer. Sure, you could check the signing date,
    // but it's not authoritative.
    //
    // IF the signed copies were stored on the smart contract IN ORDER then we'd
    // know for sure which one signed first.
    //
    // UPDATE: I am now storing a new member variable,
    // openingNumsInOrderOfSigning_,
    // inside OTScriptable! This way I can see exactly which opening number came
    // first, and I can use that for the display transaction num.

    const UnallocatedVector<std::int64_t>& openingNumsInOrderOfSigning =
        theInput.openingNumsInOrderOfSigning();

    trans_num_display_ = openingNumsInOrderOfSigning.size() > 0
                             ? openingNumsInOrderOfSigning[0]
                             : transaction_num_;

    // Note: Maybe later, store the Smart Contract's temporary name, or ID,
    // in the memo field.
    // Or something.
    //
    memo_->Release();  // not used here.

    notary_id_ = theInput.GetNotaryID();
    instrument_definition_id_.clear();  // not used here.

    sender_nym_id_ = theInput.GetSenderNymID();
    sender_account_id_.clear();

    recipient_nym_id_.clear();      // not used here.
    recipient_account_id_.clear();  // not used here.

    remitter_nym_id_.clear();
    remitter_account_id_.clear();

    valid_from_ = theInput.GetValidFrom();
    valid_to_ = theInput.GetValidTo();
}

auto OTPayment::SetTempValuesFromSmartContract(const OTSmartContract& theInput)
    -> bool
{
    if (OTPayment::SMART_CONTRACT == type_) {
        lowLevelSetTempValuesFromSmartContract(theInput);
        return true;
    } else {
        LogError()(OT_PRETTY_CLASS())("Error: Wrong type. (Returning false).")
            .Flush();
    }

    return false;
}

auto OTPayment::GetMemo(String& strOutput) const -> bool
{
    strOutput.Release();

    if (!are_temp_values_set_) { return false; }

    bool bSuccess = false;

    switch (type_) {
        case OTPayment::CHEQUE:
        case OTPayment::VOUCHER:
        case OTPayment::INVOICE:
        case OTPayment::PAYMENT_PLAN:
        case OTPayment::NOTICE: {
            if (memo_->Exists()) {
                strOutput.Set(memo_);
                bSuccess = true;
            } else {
                bSuccess = false;
            }
        } break;
        case OTPayment::SMART_CONTRACT: {
            bSuccess = false;
        } break;
        case OTPayment::ERROR_STATE:
        default: {
            LogError()(OT_PRETTY_CLASS())("Bad payment type!").Flush();
        }
    }

    return bSuccess;
}

auto OTPayment::GetAmount(Amount& lOutput) const -> bool
{
    lOutput = 0;

    if (!are_temp_values_set_) { return false; }

    bool bSuccess = false;

    switch (type_) {
        case OTPayment::CHEQUE:
        case OTPayment::VOUCHER:
        case OTPayment::INVOICE:
        case OTPayment::PAYMENT_PLAN: {
            lOutput = amount_;
            bSuccess = true;
        } break;
        case OTPayment::NOTICE:
        case OTPayment::SMART_CONTRACT: {
            lOutput = 0;
            bSuccess = false;
        } break;
        case OTPayment::ERROR_STATE:
        default: {
            LogError()(OT_PRETTY_CLASS())("Bad payment type!").Flush();
        }
    }

    return bSuccess;
}

auto OTPayment::GetAllTransactionNumbers(
    NumList& numlistOutput,
    const PasswordPrompt& reason) const -> bool
{
    OT_ASSERT_MSG(
        are_temp_values_set_,
        "Temp values weren't even set! Should "
        "NOT have called this function at all.");

    // SMART CONTRACTS and PAYMENT PLANS get a little special treatment
    // here at the top. Notice, BTW, that you MUST call SetTempValues
    // before doing this, otherwise the type_ isn't even set!
    //
    if (  // (false == are_temp_values_set_)    || // Why is this here? Because
          // if temp values haven't been set yet,
        (OTPayment::SMART_CONTRACT == type_) ||  // then type_ isn't set
                                                 // either. We only want
                                                 // smartcontracts and
        (OTPayment::PAYMENT_PLAN == type_))  // payment plans here, but without
                                             // type_ we can't know the
                                             // type...
    {  // ===> UPDATE: type_ IS set!! This comment is wrong!
        OTTrackable* pTrackable = Instantiate();
        if (nullptr == pTrackable) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed instantiating OTPayment containing cron item: ")(
                payment_.get())(".")
                .Flush();
            return false;
        }  // Below THIS POINT, MUST DELETE pTrackable!
        std::unique_ptr<OTTrackable> theTrackableAngel(pTrackable);

        auto* pPlan = dynamic_cast<OTPaymentPlan*>(pTrackable);
        auto* pSmartContract = dynamic_cast<OTSmartContract*>(pTrackable);

        if (nullptr != pPlan) {
            pPlan->GetAllTransactionNumbers(numlistOutput);
            return true;
        } else if (nullptr != pSmartContract) {
            pSmartContract->GetAllTransactionNumbers(numlistOutput);
            return true;
        }

        return false;
    }
    // ------------------------------------------------------
    // Notice from the server (in our Nym's record box probably)
    // which is in reference to a sent payment plan or smart contract.
    //
    else if (OTPayment::NOTICE == type_) {
        std::unique_ptr<OTTransaction> pNotice(InstantiateNotice());

        if (!pNotice) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed instantiating OTPayment containing a notice: ")(
                payment_.get())(".")
                .Flush();
            return false;
        }
        auto strCronItem = String::Factory();
        // -------------------------------------------
        auto pItem = pNotice->GetItem(itemType::notice);

        if (false != bool(pItem)) {       // The item's NOTE, as opposed to the
                                          // transaction's reference string,
            pItem->GetNote(strCronItem);  // contains the updated version of the
        }
        // cron item, versus the original.
        // -------------------------------------------
        if (!strCronItem->Exists()) {
            pNotice->GetReferenceString(strCronItem);  // Didn't find the
        }
        // updated one? Okay
        // let's grab the
        // original then.
        // -------------------------------------------
        if (!strCronItem->Exists()) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed geting reference string (containing cron item) "
                "from instantiated OTPayment: ")(payment_.get())(".")
                .Flush();
            return false;
        }
        // -------------------------------------------
        std::unique_ptr<OTPayment> pCronItemPayment(
            new OTPayment(api_, strCronItem));

        if (!pCronItemPayment || !pCronItemPayment->IsValid() ||
            !pCronItemPayment->SetTempValues(reason)) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed instantiating or verifying a "
                "(purported) cron item: ")(strCronItem.get())(".")
                .Flush();
            return false;
        }
        // -------------------------------------------
        // NOTE: We may wish to additionally add the transaction numbers from
        // pNotice
        // (and not just from its attached payments.) It depends on what those
        // numbers
        // are being used for. May end up revisiting this, since pNotice itself
        // has
        // a different transaction number than the numbers on the instrument it
        // has attached.
        //
        return pCronItemPayment->GetAllTransactionNumbers(
            numlistOutput, reason);
    }
    // ------------------------------------------------------
    // Next: ALL OTHER payment types...
    //
    bool bSuccess = false;

    switch (type_) {
        case OTPayment::CHEQUE:
        case OTPayment::VOUCHER:
        case OTPayment::INVOICE: {
            if (transaction_num_ > 0) { numlistOutput.Add(transaction_num_); }
            bSuccess = true;
        } break;
        case OTPayment::PAYMENT_PLAN:  // Should never happen. (Handled already
                                       // above.)
        case OTPayment::SMART_CONTRACT:  // Should never happen. (Handled
                                         // already above.)
        case OTPayment::NOTICE:  // Should never happen. (Handled already
                                 // above.)
        case OTPayment::ERROR_STATE:
        default: {
            LogError()(OT_PRETTY_CLASS())("Bad payment type!").Flush();
        }
    }

    return bSuccess;
}

// This works with a cheque who has a transaction number.
// It also works with a payment plan or smart contract, for opening AND closing
// numbers.
auto OTPayment::HasTransactionNum(
    const std::int64_t& lInput,
    const PasswordPrompt& reason) const -> bool
{
    OT_ASSERT_MSG(
        are_temp_values_set_,
        "Should never call this method unless "
        "you have first set the temp values.");

    // SMART CONTRACTS and PAYMENT PLANS get a little special
    // treatment here at the top.
    //
    if (  // (false == are_temp_values_set_)     ||
        (OTPayment::SMART_CONTRACT == type_) ||
        (OTPayment::PAYMENT_PLAN == type_)) {
        OTTrackable* pTrackable = Instantiate();
        if (nullptr == pTrackable) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed instantiating OTPayment containing: ")(payment_.get())(
                ".")
                .Flush();
            return false;
        }  // BELOW THIS POINT, MUST DELETE pTrackable!
        std::unique_ptr<OTTrackable> theTrackableAngel(pTrackable);

        OTPaymentPlan* pPlan = nullptr;
        OTSmartContract* pSmartContract = nullptr;

        pPlan = dynamic_cast<OTPaymentPlan*>(pTrackable);
        pSmartContract = dynamic_cast<OTSmartContract*>(pTrackable);

        if (nullptr != pPlan) {
            return pPlan->HasTransactionNum(lInput);
        } else if (nullptr != pSmartContract) {
            return pSmartContract->HasTransactionNum(lInput);
        }

        return false;
    }
    // ------------------------------------------------------
    // Notice from the server (in our Nym's record box probably)
    // which is in reference to a sent payment plan or smart contract.
    //
    else if (OTPayment::NOTICE == type_) {
        std::unique_ptr<OTTransaction> pNotice(InstantiateNotice());

        if (!pNotice) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed instantiating OTPayment containing a notice: ")(
                payment_.get())(".")
                .Flush();
            return false;
        }
        auto strCronItem = String::Factory();
        // -------------------------------------------
        auto pItem = pNotice->GetItem(itemType::notice);

        if (false != bool(pItem)) {       // The item's NOTE, as opposed to the
                                          // transaction's reference string,
            pItem->GetNote(strCronItem);  // contains the updated version of the
        }
        // cron item, versus the original.
        // -------------------------------------------
        if (!strCronItem->Exists()) {
            pNotice->GetReferenceString(strCronItem);  // Didn't find the
        }
        // updated one? Okay
        // let's grab the
        // original then.
        // -------------------------------------------
        if (!strCronItem->Exists()) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed geting reference string (containing cron item) "
                "from instantiated OTPayment: ")(payment_.get())(".")
                .Flush();
            return false;
        }
        // -------------------------------------------
        std::unique_ptr<OTPayment> pCronItemPayment(
            new OTPayment(api_, strCronItem));

        if (!pCronItemPayment || !pCronItemPayment->IsValid() ||
            !pCronItemPayment->SetTempValues(reason)) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed instantiating or verifying a "
                "(purported) cron item: ")(strCronItem.get())(".")
                .Flush();
            return false;
        }
        // -------------------------------------------
        return pCronItemPayment->HasTransactionNum(lInput, reason);
    }
    // ------------------------------------------------------
    // Next: ALL OTHER payment types...
    //
    bool bSuccess = false;

    switch (type_) {
        case OTPayment::CHEQUE:
        case OTPayment::VOUCHER:
        case OTPayment::INVOICE: {
            if (lInput == transaction_num_) { bSuccess = true; }
        } break;
        case OTPayment::PAYMENT_PLAN:  // Should never happen. (Handled already
                                       // above.)
        case OTPayment::SMART_CONTRACT:  // Should never happen. (Handled
                                         // already above.)
        case OTPayment::NOTICE:
        case OTPayment::ERROR_STATE:
        default: {  // Should never happen. (Handled already above.)
            LogError()(OT_PRETTY_CLASS())("Bad payment type!").Flush();
        }
    }

    return bSuccess;
}

auto OTPayment::GetClosingNum(
    std::int64_t& lOutput,
    const identifier::Generic& theAcctID,
    const PasswordPrompt& reason) const -> bool
{
    lOutput = 0;

    // SMART CONTRACTS and PAYMENT PLANS get a little special
    // treatment here at the top.
    //
    if ((false == are_temp_values_set_) ||  // type_ isn't set if this is
                                            // false.
        (OTPayment::SMART_CONTRACT == type_) ||
        (OTPayment::PAYMENT_PLAN == type_)) {
        OTTrackable* pTrackable = Instantiate();
        if (nullptr == pTrackable) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed instantiating OTPayment containing: ")(payment_.get())(
                ".")
                .Flush();
            return false;
        }  // BELOW THIS POINT, MUST DELETE pTrackable!
        std::unique_ptr<OTTrackable> theTrackableAngel(pTrackable);

        OTSmartContract* pSmartContract = nullptr;
        pSmartContract = dynamic_cast<OTSmartContract*>(pTrackable);

        OTPaymentPlan* pPlan = nullptr;
        pPlan = dynamic_cast<OTPaymentPlan*>(pTrackable);

        if (nullptr != pSmartContract) {
            lOutput = pSmartContract->GetClosingNumber(theAcctID);
            if (lOutput > 0) { return true; }
            return false;
        } else if (nullptr != pPlan) {
            lOutput = pPlan->GetClosingNumber(theAcctID);
            if (lOutput > 0) { return true; }
            return false;
        }

        // There's no "return false" here because of the "if
        // !are_temp_values_set_"
        // In other words, it still very well could be a cheque or invoice, or
        // whatever.
    }

    if (!are_temp_values_set_) { return false; }
    // --------------------------------------
    if (OTPayment::NOTICE == type_) {
        std::unique_ptr<OTTransaction> pNotice(InstantiateNotice());

        if (!pNotice) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed instantiating OTPayment containing a notice: ")(
                payment_.get())(".")
                .Flush();
            return false;
        }
        auto strCronItem = String::Factory();
        // -------------------------------------------
        auto pItem = pNotice->GetItem(itemType::notice);

        if (false != bool(pItem)) {       // The item's NOTE, as opposed to the
                                          // transaction's reference string,
            pItem->GetNote(strCronItem);  // contains the updated version of the
        }
        // cron item, versus the original.
        // -------------------------------------------
        if (!strCronItem->Exists()) {
            pNotice->GetReferenceString(strCronItem);  // Didn't find the
        }
        // updated one? Okay
        // let's grab the
        // original then.
        // -------------------------------------------
        if (!strCronItem->Exists()) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed geting reference string (containing cron item) "
                "from instantiated OTPayment: ")(payment_.get())(".")
                .Flush();
            return false;
        }
        // -------------------------------------------
        std::unique_ptr<OTPayment> pCronItemPayment(
            new OTPayment(api_, strCronItem));

        if (!pCronItemPayment || !pCronItemPayment->IsValid() ||
            !pCronItemPayment->SetTempValues(reason)) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed instantiating or verifying a "
                "(purported) cron item: ")(strCronItem.get())(".")
                .Flush();
            return false;
        }
        // -------------------------------------------
        return pCronItemPayment->GetClosingNum(lOutput, theAcctID, reason);
    }
    // --------------------------------------
    // Next: ALL OTHER payment types...
    //
    bool bSuccess = false;

    switch (type_) {
        case OTPayment::CHEQUE:
        case OTPayment::VOUCHER:
        case OTPayment::INVOICE: {
            lOutput = 0;  // Redundant, but just making sure.
            bSuccess = false;
        } break;
        case OTPayment::PAYMENT_PLAN:
        case OTPayment::SMART_CONTRACT:
        case OTPayment::NOTICE:
        case OTPayment::ERROR_STATE:
        default: {
            LogError()(OT_PRETTY_CLASS())("Bad payment type!").Flush();
        }
    }

    return bSuccess;
}

auto OTPayment::GetOpeningNum(
    std::int64_t& lOutput,
    const identifier::Nym& theNymID,
    const PasswordPrompt& reason) const -> bool
{
    lOutput = 0;

    // SMART CONTRACTS and PAYMENT PLANS get a little special
    // treatment here at the top.
    //
    if ((false == are_temp_values_set_) ||  // type_ isn't available if this is
                                            // false.
        (OTPayment::SMART_CONTRACT == type_) ||
        (OTPayment::PAYMENT_PLAN == type_)) {
        OTTrackable* pTrackable = Instantiate();
        if (nullptr == pTrackable) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed instantiating OTPayment containing: ")(payment_.get())(
                ".")
                .Flush();
            return false;
        }  // BELOW THIS POINT, MUST DELETE pTrackable!
        std::unique_ptr<OTTrackable> theTrackableAngel(pTrackable);

        OTSmartContract* pSmartContract = nullptr;
        pSmartContract = dynamic_cast<OTSmartContract*>(pTrackable);

        OTPaymentPlan* pPlan = nullptr;
        pPlan = dynamic_cast<OTPaymentPlan*>(pTrackable);

        if (nullptr != pSmartContract) {
            lOutput = pSmartContract->GetOpeningNumber(theNymID);
            if (lOutput > 0) { return true; }
            return false;
        } else if (nullptr != pPlan) {
            lOutput = pPlan->GetOpeningNumber(theNymID);
            if (lOutput > 0) { return true; }
            return false;
        }

        // There's no "return false" here because of the "if
        // !are_temp_values_set_"
        // In other words, it still very well could be a cheque or invoice, or
        // whatever.
    }

    if (!are_temp_values_set_) { return false; }
    // --------------------------------------
    if (OTPayment::NOTICE == type_) {
        std::unique_ptr<OTTransaction> pNotice(InstantiateNotice());

        if (!pNotice) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed instantiating OTPayment containing a notice: ")(
                payment_.get())(".")
                .Flush();
            return false;
        }
        auto strCronItem = String::Factory();
        // -------------------------------------------
        auto pItem = pNotice->GetItem(itemType::notice);

        if (false != bool(pItem)) {       // The item's NOTE, as opposed to the
                                          // transaction's reference string,
            pItem->GetNote(strCronItem);  // contains the updated version of the
        }
        // cron item, versus the original.
        // -------------------------------------------
        if (!strCronItem->Exists()) {
            pNotice->GetReferenceString(strCronItem);  // Didn't find the
        }
        // updated one? Okay
        // let's grab the
        // original then.
        // -------------------------------------------
        if (!strCronItem->Exists()) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed geting reference string (containing cron item) "
                "from instantiated OTPayment: ")(payment_.get())(".")
                .Flush();
            return false;
        }
        // -------------------------------------------
        std::unique_ptr<OTPayment> pCronItemPayment(
            new OTPayment(api_, strCronItem));

        if (!pCronItemPayment || !pCronItemPayment->IsValid() ||
            !pCronItemPayment->SetTempValues(reason)) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed instantiating or verifying a "
                "(purported) cron item: ")(strCronItem.get())(".")
                .Flush();
            return false;
        }
        // -------------------------------------------
        return pCronItemPayment->GetOpeningNum(lOutput, theNymID, reason);
    }
    // --------------------------------------
    // Next: ALL OTHER payment types...
    //
    bool bSuccess = false;

    switch (type_) {
        case OTPayment::CHEQUE:
        case OTPayment::INVOICE: {
            if (sender_nym_id_ == theNymID) {
                lOutput =
                    transaction_num_;  // The "opening" number for a cheque is
                                       // the ONLY number it has.
                bSuccess = true;
            } else {
                lOutput = 0;
                bSuccess = false;
            }
        } break;
        case OTPayment::VOUCHER: {
            if (remitter_nym_id_ == theNymID) {
                lOutput =
                    transaction_num_;  // The "opening" number for a cheque is
                                       // the ONLY number it has.
                bSuccess = true;
            } else {
                lOutput = 0;
                bSuccess = false;
            }
        } break;
        case OTPayment::PAYMENT_PLAN:
        case OTPayment::SMART_CONTRACT:
        case OTPayment::NOTICE:
        case OTPayment::ERROR_STATE:
        default: {
            LogError()(OT_PRETTY_CLASS())("Bad payment type!").Flush();
        }
    }

    return bSuccess;
}

auto OTPayment::GetTransNumDisplay(std::int64_t& lOutput) const -> bool
{
    lOutput = 0;

    if (!are_temp_values_set_) { return false; }

    bool bSuccess = false;

    switch (type_) {
        case OTPayment::CHEQUE:
        case OTPayment::VOUCHER:
        case OTPayment::INVOICE: {
            lOutput = transaction_num_;
            bSuccess = true;
        } break;
        case OTPayment::PAYMENT_PLAN: {  // For payment plans, this is the
                                         // opening
            // transaction FOR THE RECIPIENT NYM (The merchant.)
            lOutput = trans_num_display_;
            bSuccess = true;
        } break;
        case OTPayment::SMART_CONTRACT: {  // For smart contracts, this is the
                                           // opening
                                           // transaction number FOR THE NYM who
                                           // first proposed the contract.
            // NOTE: We need a consistent number we can use for display
            // purposes, so all
            // the parties can cross-reference the smart contract in their GUIs.
            // THEREFORE
            // need to get ALL transaction numbers from a contract, and then use
            // the first
            // one. That's most likely the opening number for the first party.
            // That's the ONLY number that we know ALL parties have access to.
            // (The first
            // party has no idea what transaction numbers the SECOND party
            // used...so the only
            // way to have a number they can ALL cross-reference, is to use a #
            // from the first
            // party.)
            // NOTE: the above logic is performed where trans_num_display_ is
            // set.
            lOutput = trans_num_display_;
            bSuccess = true;
        } break;
        case OTPayment::NOTICE: {
            lOutput = trans_num_display_;
            bSuccess = true;
        } break;
        case OTPayment::ERROR_STATE:
        default: {
            LogError()(OT_PRETTY_CLASS())("Bad payment type!").Flush();
        }
    }

    return bSuccess;
}

auto OTPayment::GetTransactionNum(std::int64_t& lOutput) const -> bool
{
    lOutput = 0;

    if (!are_temp_values_set_) { return false; }

    bool bSuccess = false;

    switch (type_) {
        case OTPayment::NOTICE:
        case OTPayment::CHEQUE:
        case OTPayment::VOUCHER:
        case OTPayment::INVOICE:
        // For payment plans, this is the opening transaction FOR THE NYM who
        // activated the contract (probably the customer.)
        case OTPayment::PAYMENT_PLAN:
        // For smart contracts, this is the opening transaction number FOR THE
        // NYM who activated the contract.
        case OTPayment::SMART_CONTRACT: {
            lOutput = transaction_num_;
            bSuccess = true;
        } break;
        case OTPayment::ERROR_STATE:
        default: {
            LogError()(OT_PRETTY_CLASS())("Bad payment type!").Flush();
        }
    }

    return bSuccess;
}

auto OTPayment::GetValidFrom(Time& tOutput) const -> bool
{
    tOutput = Time{};

    if (!are_temp_values_set_) { return false; }

    bool bSuccess = false;

    switch (type_) {
        case OTPayment::NOTICE:
        case OTPayment::CHEQUE:
        case OTPayment::VOUCHER:
        case OTPayment::INVOICE:
        case OTPayment::PAYMENT_PLAN:
        case OTPayment::SMART_CONTRACT: {
            tOutput = valid_from_;
            bSuccess = true;
        } break;
        case OTPayment::ERROR_STATE:
        default: {
            LogError()(OT_PRETTY_CLASS())("Bad payment type!").Flush();
        }
    }

    return bSuccess;
}

auto OTPayment::GetValidTo(Time& tOutput) const -> bool
{
    tOutput = Time{};

    if (!are_temp_values_set_) { return false; }

    bool bSuccess = false;

    switch (type_) {
        case OTPayment::NOTICE:
        case OTPayment::CHEQUE:
        case OTPayment::VOUCHER:
        case OTPayment::INVOICE:
        case OTPayment::PAYMENT_PLAN:
        case OTPayment::SMART_CONTRACT: {
            tOutput = valid_to_;
            bSuccess = true;
        } break;
        case OTPayment::ERROR_STATE:
        default: {
            LogError()(OT_PRETTY_CLASS())("Bad payment type!").Flush();
        }
    }

    return bSuccess;
}

// Verify whether the CURRENT date is AFTER the the VALID TO date.
// Notice, this will return false, if the instrument is NOT YET VALID.
// You have to use VerifyCurrentDate() to make sure you're within the
// valid date range to use this instrument. But sometimes you only want
// to know if it's expired, regardless of whether it's valid yet. So this
// function answers that for you.
//
auto OTPayment::IsExpired(bool& bExpired) -> bool
{
    if (!are_temp_values_set_) { return false; }

    const auto CURRENT_TIME = Clock::now();

    // If the current time is AFTER the valid-TO date,
    // AND the valid_to is a nonzero number (0 means "doesn't expire")
    // THEN return true (it's expired.)
    //
    if ((CURRENT_TIME >= valid_to_) && (valid_to_ > Time{})) {
        bExpired = true;
    } else {
        bExpired = false;
    }

    return true;
}

// Verify whether the CURRENT date is WITHIN the VALID FROM / TO dates.
//
auto OTPayment::VerifyCurrentDate(bool& bVerified) -> bool
{
    if (!are_temp_values_set_) { return false; }

    const auto CURRENT_TIME = Clock::now();

    if ((CURRENT_TIME >= valid_from_) &&
        ((CURRENT_TIME <= valid_to_) || (Time{} == valid_to_))) {
        bVerified = true;
    } else {
        bVerified = false;
    }

    return true;
}

auto OTPayment::GetInstrumentDefinitionID(identifier::Generic& theOutput) const
    -> bool
{
    theOutput.clear();

    if (!are_temp_values_set_) { return false; }

    bool bSuccess = false;

    switch (type_) {
        case OTPayment::CHEQUE:
        case OTPayment::VOUCHER:
        case OTPayment::INVOICE:
        case OTPayment::PAYMENT_PLAN:
        case OTPayment::NOTICE: {
            theOutput.Assign(instrument_definition_id_);
            bSuccess = !instrument_definition_id_.empty();
        } break;
        case OTPayment::SMART_CONTRACT: {
            bSuccess = false;
        } break;
        case OTPayment::ERROR_STATE:
        default: {
            LogError()(OT_PRETTY_CLASS())("Bad payment type!").Flush();
        }
    }

    return bSuccess;
}

auto OTPayment::GetNotaryID(identifier::Generic& theOutput) const -> bool
{
    theOutput.clear();

    if (!are_temp_values_set_) {
        LogError()(OT_PRETTY_CLASS())("Object not yet instantiated.").Flush();

        return false;
    }

    bool bSuccess = false;

    switch (type_) {
        case OTPayment::CHEQUE:
        case OTPayment::VOUCHER:
        case OTPayment::INVOICE:
        case OTPayment::PAYMENT_PLAN:
        case OTPayment::SMART_CONTRACT:
        case OTPayment::NOTICE: {
            theOutput.Assign(notary_id_);
            bSuccess = !notary_id_.empty();
        } break;
        case ERROR_STATE:
        default:
            LogError()(OT_PRETTY_CLASS())("Bad payment type!").Flush();
            break;
    }

    return bSuccess;
}

// With a voucher (cashier's cheque) the "bank" is the "sender",
// whereas the actual Nym who purchased it is the "remitter."
//
auto OTPayment::GetRemitterNymID(identifier::Nym& theOutput) const -> bool
{
    theOutput.clear();

    if (!are_temp_values_set_) { return false; }

    bool bSuccess = false;

    switch (type_) {
        case OTPayment::VOUCHER:
            theOutput.Assign(remitter_nym_id_);
            bSuccess = !remitter_nym_id_.empty();
            break;

        default:
            LogError()(OT_PRETTY_CLASS())("Bad payment type! Expected a "
                                          "voucher cheque.")
                .Flush();
            break;
    }

    return bSuccess;
}

// With a voucher (cashier's cheque) the "bank"s account is the "sender" acct,
// whereas the actual account originally used to purchase it is the "remitter"
// acct.
//
auto OTPayment::GetRemitterAcctID(identifier::Generic& theOutput) const -> bool
{
    theOutput.clear();

    if (!are_temp_values_set_) { return false; }

    bool bSuccess = false;

    switch (type_) {
        case OTPayment::VOUCHER:
            theOutput.Assign(remitter_account_id_);
            bSuccess = !remitter_account_id_.empty();
            break;

        default:
            LogError()(OT_PRETTY_CLASS())("Bad payment type! Expected a "
                                          "voucher cheque.")
                .Flush();
            break;
    }

    return bSuccess;
}

auto OTPayment::GetSenderNymIDForDisplay(identifier::Nym& theOutput) const
    -> bool
{
    if (IsVoucher()) { return GetRemitterNymID(theOutput); }

    return GetSenderNymID(theOutput);
}

auto OTPayment::GetSenderAcctIDForDisplay(identifier::Generic& theOutput) const
    -> bool
{
    if (IsVoucher()) { return GetRemitterAcctID(theOutput); }

    return GetSenderAcctID(theOutput);
}

auto OTPayment::GetSenderNymID(identifier::Nym& theOutput) const -> bool
{
    theOutput.clear();

    if (!are_temp_values_set_) { return false; }

    bool bSuccess = false;

    switch (type_) {
        case OTPayment::CHEQUE:
        case OTPayment::VOUCHER:
        case OTPayment::INVOICE:
        case OTPayment::PAYMENT_PLAN:
        case OTPayment::SMART_CONTRACT:
        case OTPayment::NOTICE: {
            theOutput.Assign(sender_nym_id_);
            bSuccess = !sender_nym_id_.empty();
        } break;
        case ERROR_STATE:
        default:
            LogError()(OT_PRETTY_CLASS())("Bad payment type!").Flush();
            break;
    }

    return bSuccess;
}

auto OTPayment::GetSenderAcctID(identifier::Generic& theOutput) const -> bool
{
    theOutput.clear();

    if (!are_temp_values_set_) { return false; }

    bool bSuccess = false;

    switch (type_) {
        case OTPayment::CHEQUE:
        case OTPayment::VOUCHER:
        case OTPayment::INVOICE:
        case OTPayment::PAYMENT_PLAN:
        case OTPayment::NOTICE: {
            theOutput.Assign(sender_account_id_);
            bSuccess = !sender_account_id_.empty();
        } break;
        case OTPayment::SMART_CONTRACT: {
            bSuccess = false;
        } break;
        case ERROR_STATE:
        default: {
            LogError()(OT_PRETTY_CLASS())("Bad payment type!").Flush();
        }
    }

    return bSuccess;
}

auto OTPayment::GetRecipientNymID(identifier::Nym& theOutput) const -> bool
{
    theOutput.clear();

    if (!are_temp_values_set_) { return false; }

    bool bSuccess = false;

    switch (type_) {
        case OTPayment::CHEQUE:
        case OTPayment::VOUCHER:
        case OTPayment::INVOICE:
        case OTPayment::PAYMENT_PLAN:
        case OTPayment::NOTICE: {
            if (has_recipient_) {
                theOutput.Assign(recipient_nym_id_);
                bSuccess = !recipient_nym_id_.empty();
            } else {
                bSuccess = false;
            }

        } break;
        case OTPayment::SMART_CONTRACT: {
            bSuccess = false;
        } break;
        case ERROR_STATE:
        default:
            LogError()(OT_PRETTY_CLASS())("Bad payment type!").Flush();
            break;
    }

    return bSuccess;
}

auto OTPayment::GetRecipientAcctID(identifier::Generic& theOutput) const -> bool
{
    // NOTE:
    // A cheque HAS NO "Recipient Asset Acct ID", since the recipient's account
    // (where he deposits
    // the cheque) is not known UNTIL the time of the deposit. It's certainly
    // not
    // known at the time
    // that the cheque is written...

    theOutput.clear();

    if (!are_temp_values_set_) { return false; }

    bool bSuccess = false;

    switch (type_) {
        case OTPayment::PAYMENT_PLAN:
        case OTPayment::NOTICE: {
            if (has_recipient_) {
                theOutput.Assign(recipient_account_id_);
                bSuccess = !recipient_account_id_.empty();
            } else {
                bSuccess = false;
            }
        } break;
        case OTPayment::CHEQUE:
        case OTPayment::VOUCHER:
        case OTPayment::INVOICE:
        case OTPayment::SMART_CONTRACT: {
            bSuccess = false;
        } break;
        case ERROR_STATE:
        default:
            LogError()(OT_PRETTY_CLASS())("Bad payment type!").Flush();
            break;
    }

    return bSuccess;
}

void OTPayment::InitPayment()
{
    type_ = OTPayment::ERROR_STATE;
    amount_ = 0;
    transaction_num_ = 0;
    trans_num_display_ = 0;
    valid_from_ = Time{};
    valid_to_ = Time{};
    are_temp_values_set_ = false;
    has_recipient_ = false;
    has_remitter_ = false;
    contract_type_->Set("PAYMENT");
}

// CALLER is responsible to delete.
//
auto OTPayment::Instantiate() const -> OTTrackable*
{
    std::unique_ptr<Contract> pContract;
    OTTrackable* pTrackable = nullptr;
    Cheque* pCheque = nullptr;
    OTPaymentPlan* pPaymentPlan = nullptr;
    OTSmartContract* pSmartContract = nullptr;

    switch (type_) {
        case CHEQUE:
        case VOUCHER:
        case INVOICE: {
            pContract = api_.Factory().InternalSession().Contract(payment_);

            if (false != bool(pContract)) {
                pCheque = dynamic_cast<Cheque*>(pContract.release());

                if (nullptr == pCheque) {
                    LogError()(OT_PRETTY_CLASS())(
                        "Tried to instantiate cheque, "
                        "but factory returned non-cheque: ")(payment_.get())(
                        ".")
                        .Flush();
                } else {
                    pTrackable = pCheque;
                }
            } else {
                LogError()(OT_PRETTY_CLASS())(
                    "Tried to instantiate cheque, but "
                    "factory returned nullptr: ")(payment_.get())(".")
                    .Flush();
            }
        } break;
        case PAYMENT_PLAN: {
            pContract = api_.Factory().InternalSession().Contract(payment_);

            if (false != bool(pContract)) {
                pPaymentPlan =
                    dynamic_cast<OTPaymentPlan*>(pContract.release());

                if (nullptr == pPaymentPlan) {
                    LogError()(OT_PRETTY_CLASS())(
                        "Tried to instantiate payment "
                        "plan, but factory returned non-payment-plan: ")(
                        payment_.get())(".")
                        .Flush();
                } else {
                    pTrackable = pPaymentPlan;
                }
            } else {
                LogError()(OT_PRETTY_CLASS())(
                    "Tried to instantiate payment "
                    "plan, but factory returned nullptr: ")(payment_.get())(".")
                    .Flush();
            }
        } break;
        case SMART_CONTRACT: {
            pContract = api_.Factory().InternalSession().Contract(payment_);

            if (false != bool(pContract)) {
                pSmartContract =
                    dynamic_cast<OTSmartContract*>(pContract.release());

                if (nullptr == pSmartContract) {
                    LogError()(OT_PRETTY_CLASS())(
                        "Tried to instantiate smart contract, but factory "
                        "returned non-smart-contract: ")(payment_.get())(".")
                        .Flush();
                } else {
                    pTrackable = pSmartContract;
                }
            } else {
                LogError()(OT_PRETTY_CLASS())(
                    "Tried to instantiate smart "
                    "contract, but factory returned nullptr: ")(payment_.get())(
                    ".")
                    .Flush();
            }
        } break;
        case NOTICE: {
            LogError()(OT_PRETTY_CLASS())(
                "ERROR: Tried to instantiate a notice, "
                "but should have called OTPayment::InstantiateNotice.")
                .Flush();
        }
            return nullptr;
        case ERROR_STATE:
        default: {
            LogError()(OT_PRETTY_CLASS())(
                "ERROR: Tried to instantiate payment "
                "object, but had a bad type. Contents: ")(payment_.get())(".")
                .Flush();
            return nullptr;
        }
    }

    return pTrackable;
}

auto OTPayment::Instantiate(const String& strPayment) -> OTTrackable*
{
    if (SetPayment(strPayment)) { return Instantiate(); }

    return nullptr;
}

auto OTPayment::InstantiateNotice(const String& strNotice) -> OTTransaction*
{
    if (!SetPayment(strNotice)) {
        LogError()(OT_PRETTY_CLASS())("WARNING: Failed setting the "
                                      "notice string based on "
                                      "what was passed in: ")(strNotice)(".")
            .Flush();
    } else if (OTPayment::NOTICE != type_) {
        LogError()(OT_PRETTY_CLASS())("WARNING: No notice was found in "
                                      "provided string: ")(strNotice)(".")
            .Flush();
    } else {
        return InstantiateNotice();
    }

    return nullptr;
}

auto OTPayment::InstantiateNotice() const -> OTTransaction*
{
    if (payment_->Exists() && (OTPayment::NOTICE == GetType())) {
        auto pType = api_.Factory().InternalSession().Transaction(payment_);

        if (false == bool(pType)) {
            LogError()(OT_PRETTY_CLASS())("Failure 1: This payment object does "
                                          "NOT contain a notice. "
                                          "Contents: ")(payment_.get())(".")
                .Flush();
            return nullptr;
        }

        auto* pNotice = dynamic_cast<OTTransaction*>(pType.release());

        if (nullptr == pNotice) {
            LogError()(OT_PRETTY_CLASS())("Failure 2: This payment object does "
                                          "NOT contain a notice. "
                                          "Contents: ")(payment_.get())(".")
                .Flush();
            return nullptr;
        }

        return pNotice;
    } else {
        LogError()(OT_PRETTY_CLASS())(
            "Failure 3: This payment object does NOT contain a notice. "
            "Contents: ")(payment_.get())(".")
            .Flush();
    }

    return nullptr;
}

auto OTPayment::IsCancelledCheque(const PasswordPrompt& reason) -> bool
{
    if (false == are_temp_values_set_) {
        if (false == SetTempValues(reason)) {
            LogError()(OT_PRETTY_CLASS())("Failed to set temp values.").Flush();

            return false;
        }
    }

    OT_ASSERT(are_temp_values_set_);

    if (false == IsCheque()) { return false; }

    auto sender = identifier::Nym{};
    auto recipient = identifier::Nym{};
    Amount amount{0};

    if (false == GetSenderNymID(sender)) {
        LogError()(OT_PRETTY_CLASS())("Failed to get sender nym id.").Flush();

        return false;
    }

    if (false == GetRecipientNymID(recipient)) {
        LogError()(OT_PRETTY_CLASS())("Failed to get recipient nym id.")
            .Flush();

        return false;
    }

    if (sender != recipient) { return false; }

    if (false == GetAmount(amount)) {
        LogError()(OT_PRETTY_CLASS())("Failed to amount.").Flush();

        return false;
    }

    if (0 != amount) { return false; }

    return true;
}

auto OTPayment::ProcessXMLNode(irr::io::IrrXMLReader*& xml) -> std::int32_t
{
    const auto strNodeName = String::Factory(xml->getNodeName());

    if (strNodeName->Compare("payment")) {
        version_ = String::Factory(xml->getAttributeValue("version"));

        const auto strPaymentType =
            String::Factory(xml->getAttributeValue("type"));

        if (strPaymentType->Exists()) {
            type_ = OTPayment::GetTypeFromString(strPaymentType);
        } else {
            type_ = OTPayment::ERROR_STATE;
        }

        LogTrace()(OT_PRETTY_CLASS())("Loaded payment... Type: ")(
            GetTypeString())
            .Flush();

        return (OTPayment::ERROR_STATE == type_) ? (-1) : 1;
    } else if (strNodeName->Compare("contents")) {
        auto strContents = String::Factory();

        if (!LoadEncodedTextField(xml, strContents) || !strContents->Exists() ||
            !SetPayment(strContents)) {
            LogError()(OT_PRETTY_CLASS())(
                "ERROR: Contents field "
                "without a value, OR error setting that "
                "value onto this object. Raw: ")(strContents.get())(".")
                .Flush();

            return (-1);  // error condition
        }
        // else success -- the value is now set on this object.
        // todo security: Make sure the type of the payment that's ACTUALLY
        // there
        // matches the type expected (based on the type_ that we already read,
        // above.)

        return 1;
    }

    return 0;
}

void OTPayment::Release()
{
    Release_Payment();

    // Finally, we call the method we overrode:
    //
    Contract::Release();
}

void OTPayment::Release_Payment()
{
    type_ = OTPayment::ERROR_STATE;
    amount_ = 0;
    transaction_num_ = 0;
    trans_num_display_ = 0;
    valid_from_ = Time{};
    valid_to_ = Time{};
    payment_->Release();
    are_temp_values_set_ = false;
    has_recipient_ = false;
    has_remitter_ = false;
    memo_->Release();
    instrument_definition_id_.clear();
    notary_id_.clear();
    sender_nym_id_.clear();
    sender_account_id_.clear();
    recipient_nym_id_.clear();
    recipient_account_id_.clear();
    remitter_nym_id_.clear();
    remitter_account_id_.clear();
}

auto OTPayment::SetPayment(const String& strPayment) -> bool
{
    if (!strPayment.Exists()) {
        LogError()(OT_PRETTY_CLASS())("Empty input string.").Flush();

        return false;
    }

    auto strContract = String::Factory(strPayment.Get());

    if (!strContract->DecodeIfArmored(false))  // bEscapedIsAllowed=true
                                               // by default.
    {
        LogError()(OT_PRETTY_CLASS())(
            "Input string apparently was encoded and "
            "then failed decoding. Contents: ")(strPayment)(".")
            .Flush();
        return false;
    }

    payment_->Release();

    // todo: should be "starts with" and perhaps with a trim first
    //
    if (strContract->Contains("-----BEGIN SIGNED CHEQUE-----")) {
        type_ = OTPayment::CHEQUE;
    } else if (strContract->Contains("-----BEGIN SIGNED VOUCHER-----")) {
        type_ = OTPayment::VOUCHER;
    } else if (strContract->Contains("-----BEGIN SIGNED INVOICE-----")) {
        type_ = OTPayment::INVOICE;

    } else if (strContract->Contains("-----BEGIN SIGNED PAYMENT PLAN-----")) {
        type_ = OTPayment::PAYMENT_PLAN;
    } else if (strContract->Contains("-----BEGIN SIGNED SMARTCONTRACT-----")) {
        type_ = OTPayment::SMART_CONTRACT;

    } else if (strContract->Contains("-----BEGIN SIGNED TRANSACTION-----")) {
        type_ = OTPayment::NOTICE;
    } else {
        type_ = OTPayment::ERROR_STATE;

        LogError()(OT_PRETTY_CLASS())(
            "Failure: Unable to determine payment type, from input: ")(
            strContract.get())(".")
            .Flush();
    }

    if (OTPayment::ERROR_STATE == type_) { return false; }

    payment_->Set(strContract);

    return true;
}

void OTPayment::UpdateContents(const PasswordPrompt& reason)
{
    // I release this because I'm about to repopulate it.
    xml_unsigned_->Release();

    Tag tag("payment");

    tag.add_attribute("version", version_->Get());
    tag.add_attribute("type", GetTypeString());

    if (payment_->Exists()) {
        const auto ascContents = Armored::Factory(payment_);

        if (ascContents->Exists()) {
            tag.add_tag("contents", ascContents->Get());
        }
    }

    UnallocatedCString str_result;
    tag.output(str_result);

    xml_unsigned_->Concatenate(String::Factory(str_result));
}

OTPayment::~OTPayment() { Release_Payment(); }
}  // namespace opentxs
