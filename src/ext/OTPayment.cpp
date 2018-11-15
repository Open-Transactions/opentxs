// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/ext/OTPayment.hpp"

#if OT_CASH
#include "opentxs/cash/Purse.hpp"
#endif  // OT_CASH
#include "opentxs/api/Core.hpp"
#include "opentxs/api/Factory.hpp"
#include "opentxs/core/recurring/OTPaymentPlan.hpp"
#include "opentxs/core/script/OTSmartContract.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/core/Account.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Cheque.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Item.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/NumList.hpp"
#include "opentxs/core/StringXML.hpp"
#include "opentxs/core/OTTrackable.hpp"
#include "opentxs/core/OTTransaction.hpp"
#include "opentxs/core/OTTransactionType.hpp"
#include "opentxs/core/String.hpp"

#include <irrxml/irrXML.hpp>

#include <cstdint>
#include <ostream>
#include <string>
#include <vector>

template class std::shared_ptr<const opentxs::OTPayment>;

#define OT_METHOD "opentxs::OTPayment::"

namespace opentxs
{
char const* const __TypeStringsPayment[] = {

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
    "PURSE",          // An Contract-derived OTPurse containing a list of cash
                      // OTTokens.
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

OTPayment::OTPayment(const api::Core& core)
    : Contract(core)
    , m_strPayment(String::Factory())
    , m_Type(OTPayment::ERROR_STATE)
    , m_bAreTempValuesSet(false)
    , m_bHasRecipient(false)
    , m_bHasRemitter(false)
    , m_lAmount(0)
    , m_lTransactionNum(0)
    , m_lTransNumDisplay(0)
    , m_strMemo(String::Factory())
    , m_InstrumentDefinitionID(Identifier::Factory())
    , m_NotaryID(Identifier::Factory())
    , m_SenderNymID(Identifier::Factory())
    , m_SenderAcctID(Identifier::Factory())
    , m_RecipientNymID(Identifier::Factory())
    , m_RecipientAcctID(Identifier::Factory())
    , m_RemitterNymID(Identifier::Factory())
    , m_RemitterAcctID(Identifier::Factory())
    , m_VALID_FROM(OT_TIME_ZERO)
    , m_VALID_TO(OT_TIME_ZERO)
{
    InitPayment();
}

OTPayment::OTPayment(const api::Core& core, const String& strPayment)
    : Contract(core)
    , m_strPayment(String::Factory())
    , m_Type(OTPayment::ERROR_STATE)
    , m_bAreTempValuesSet(false)
    , m_bHasRecipient(false)
    , m_bHasRemitter(false)
    , m_lAmount(0)
    , m_lTransactionNum(0)
    , m_lTransNumDisplay(0)
    , m_strMemo(String::Factory())
    , m_InstrumentDefinitionID(Identifier::Factory())
    , m_NotaryID(Identifier::Factory())
    , m_SenderNymID(Identifier::Factory())
    , m_SenderAcctID(Identifier::Factory())
    , m_RecipientNymID(Identifier::Factory())
    , m_RecipientAcctID(Identifier::Factory())
    , m_RemitterNymID(Identifier::Factory())
    , m_RemitterAcctID(Identifier::Factory())
    , m_VALID_FROM(OT_TIME_ZERO)
    , m_VALID_TO(OT_TIME_ZERO)
{
    InitPayment();
    SetPayment(strPayment);
}

// static
const char* OTPayment::_GetTypeString(paymentType theType)
{
    std::int32_t nType = static_cast<std::int32_t>(theType);
    return __TypeStringsPayment[nType];
}

OTPayment::paymentType OTPayment::GetTypeFromString(const String& strType)
{
#define OT_NUM_ELEM(blah) (sizeof(blah) / sizeof(*(blah)))
    for (std::uint32_t i = 0; i < (OT_NUM_ELEM(__TypeStringsPayment) - 1);
         i++) {
        if (strType.Compare(__TypeStringsPayment[i]))
            return static_cast<OTPayment::paymentType>(i);
    }
#undef OT_NUM_ELEM
    return OTPayment::ERROR_STATE;
}

bool OTPayment::SetTempRecipientNymID(const Identifier& id)
{
    m_RecipientNymID = Identifier::Factory(id);

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
bool OTPayment::SetTempValues()  // This version for OTTrackable (all types
                                 // EXCEPT purses.)
{
    if (OTPayment::PURSE == m_Type) {
#if OT_CASH
        // Perform instantiation of a purse, then use it to set the temp values,
        // then cleans it up again before returning success/fail.
        //
        std::unique_ptr<Purse> pPurse(InstantiatePurse(api_.Wallet()));

        if (!pPurse) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error: Failed instantiating "
                "OTPayment (purported purse) contents: ")(m_strPayment)(".")
                .Flush();
            return false;
        }

        return SetTempValuesFromPurse(*pPurse);
#else
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Support for cash was not compiled in.")
            .Flush();

        return false;
#endif  // OT_CASH
    } else if (OTPayment::NOTICE == m_Type) {
        // Perform instantiation of a notice (OTTransaction), then use it to set
        // the temp values, then clean it up again before returning
        // success/fail.
        //
        std::unique_ptr<OTTransaction> pNotice(InstantiateNotice());

        if (!pNotice) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error: Failed instantiating "
                "OTPayment (purported notice) contents: ")(m_strPayment)(".")
                .Flush();
            return false;
        }

        return SetTempValuesFromNotice(*pNotice);
    } else {
        OTTrackable* pTrackable = Instantiate();

        if (nullptr == pTrackable) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error: Failed instantiating "
                "OTPayment contents: ")(m_strPayment)(".")
                .Flush();
            return false;
        }
        // BELOW THIS POINT, MUST DELETE pTrackable!
        std::unique_ptr<OTTrackable> theTrackableAngel(pTrackable);

        Cheque* pCheque = nullptr;
        OTPaymentPlan* pPaymentPlan = nullptr;
        OTSmartContract* pSmartContract = nullptr;

        switch (m_Type) {
            case CHEQUE:
            case VOUCHER:
            case INVOICE:
                pCheque = dynamic_cast<Cheque*>(pTrackable);
                if (nullptr == pCheque)
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Failure: "
                        "dynamic_cast<OTCheque *>(pTrackable). Contents: ")(
                        m_strPayment)(".")
                        .Flush();
                // Let's grab all the temp values from the cheque!!
                //
                else  // success
                    return SetTempValuesFromCheque(*pCheque);
                break;

            case PAYMENT_PLAN:
                pPaymentPlan = dynamic_cast<OTPaymentPlan*>(pTrackable);
                if (nullptr == pPaymentPlan)
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Failure: "
                        "dynamic_cast<OTPaymentPlan *>(pTrackable). "
                        "Contents: ")(m_strPayment)(".")
                        .Flush();
                // Let's grab all the temp values from the payment plan!!
                //
                else  // success
                    return SetTempValuesFromPaymentPlan(*pPaymentPlan);
                break;

            case SMART_CONTRACT:
                pSmartContract = dynamic_cast<OTSmartContract*>(pTrackable);
                if (nullptr == pSmartContract)
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Failure: "
                        "dynamic_cast<OTSmartContract *>(pTrackable). "
                        "Contents: ")(m_strPayment)(".")
                        .Flush();
                // Let's grab all the temp values from the smart contract!!
                //
                else  // success
                    return SetTempValuesFromSmartContract(*pSmartContract);
                break;

            default:
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Failure: Wrong m_Type. "
                    "Contents: ")(m_strPayment)(".")
                    .Flush();
                return false;
        }
    }

    return false;  // Should never actually reach this point.
}

bool OTPayment::SetTempValuesFromCheque(const Cheque& theInput)
{
    switch (m_Type) {
        case OTPayment::CHEQUE:
        case OTPayment::VOUCHER:
        case OTPayment::INVOICE:

            m_bAreTempValuesSet = true;

            m_lAmount = theInput.GetAmount();
            m_lTransactionNum = theInput.GetTransactionNum();
            m_lTransNumDisplay = m_lTransactionNum;

            if (theInput.GetMemo().Exists())
                m_strMemo->Set(theInput.GetMemo());
            else
                m_strMemo->Release();

            m_InstrumentDefinitionID = theInput.GetInstrumentDefinitionID();
            m_NotaryID = theInput.GetNotaryID();

            m_SenderNymID = theInput.GetSenderNymID();
            m_SenderAcctID = theInput.GetSenderAcctID();

            if (theInput.HasRecipient()) {
                m_bHasRecipient = true;
                m_RecipientNymID = theInput.GetRecipientNymID();
            } else {
                m_bHasRecipient = false;
                m_RecipientNymID->Release();
            }

            if (theInput.HasRemitter()) {
                m_bHasRemitter = true;
                m_RemitterNymID = theInput.GetRemitterNymID();
                m_RemitterAcctID = theInput.GetRemitterAcctID();
            } else {
                m_bHasRemitter = false;
                m_RemitterNymID->Release();
                m_RemitterAcctID->Release();
            }

            // NOTE: the "Recipient Acct" is NOT KNOWN when cheque is written,
            // but
            // only
            // once the cheque gets deposited. Therefore if type is CHEQUE, then
            // Recipient
            // Acct ID is not set, and attempts to read it will result in
            // failure.
            //
            m_RecipientAcctID->Release();

            m_VALID_FROM = theInput.GetValidFrom();
            m_VALID_TO = theInput.GetValidTo();

            return true;

        default:
            LogOutput(OT_METHOD)(__FUNCTION__)(": Error: Wrong type. "
                                               "(Returning false).")
                .Flush();
            break;
    }

    return false;
}

#if OT_CASH
bool OTPayment::SetTempValuesFromPurse(const Purse& theInput)
{
    if (OTPayment::PURSE == m_Type) {
        m_bAreTempValuesSet = true;
        m_bHasRecipient = theInput.IsNymIDIncluded();
        m_bHasRemitter = false;

        m_lAmount = theInput.GetTotalValue();
        m_lTransactionNum = 0;   // (A purse has no transaction number.)
        m_lTransNumDisplay = 0;  // (A purse has no transaction number.)

        m_strMemo->Release();  // So far there's no purse memo (could add it,
                               // though.)

        m_InstrumentDefinitionID = theInput.GetInstrumentDefinitionID();
        m_NotaryID = theInput.GetNotaryID();

        m_SenderNymID->Release();
        m_SenderAcctID->Release();

        if (!m_bHasRecipient || !theInput.GetNymID(m_RecipientNymID)) {
            m_bHasRecipient = false;
            m_RecipientNymID->Release();
        }

        m_RecipientAcctID->Release();

        m_RemitterNymID->Release();
        m_RemitterAcctID->Release();

        m_VALID_FROM = theInput.GetLatestValidFrom();
        m_VALID_TO = theInput.GetEarliestValidTo();

        return true;
    } else
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error: Wrong type. (Returning false).")
            .Flush();

    return false;
}
#endif  // OT_CASH

bool OTPayment::SetTempValuesFromNotice(const OTTransaction& theInput)
{
    if (OTPayment::NOTICE == m_Type) {
        m_bAreTempValuesSet = true;
        m_bHasRecipient = true;
        m_bHasRemitter = false;
        // -------------------------------------------
        auto strCronItem = String::Factory();

        auto pItem =
            (const_cast<OTTransaction&>(theInput)).GetItem(itemType::notice);

        if (false != bool(pItem))         // The item's NOTE, as opposed to the
                                          // transaction's reference string,
            pItem->GetNote(strCronItem);  // contains the updated version of the
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
        if (!strCronItem->Exists())
            theInput.GetReferenceString(strCronItem);  // Didn't find the
                                                       // updated one? Okay
                                                       // let's grab the
                                                       // original then.
        // -------------------------------------------
        if (!strCronItem->Exists()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed geting reference string (containing cron item) "
                "from instantiated OTPayment: ")(m_strPayment)(".")
                .Flush();
            return false;
        }
        // -------------------------------------------
        std::unique_ptr<OTPayment> pCronItemPayment(
            new OTPayment(api_, strCronItem));

        if (!pCronItemPayment || !pCronItemPayment->IsValid() ||
            !pCronItemPayment->SetTempValues()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": 1 Failed instantiating or verifying a "
                "(purported) cron item: ")(strCronItem)(".")
                .Flush();
            return false;
        }
        // -------------------------------------------
        OTTrackable* pTrackable = pCronItemPayment->Instantiate();

        if (nullptr == pTrackable) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": 2 Failed instantiating or verifying a "
                "(purported) cron item: ")(strCronItem)(".")
                .Flush();
            return false;
        }
        std::unique_ptr<OTTrackable> theTrackableAngel(pTrackable);
        // -------------------------------------------
        OTPaymentPlan* pPlan = dynamic_cast<OTPaymentPlan*>(pTrackable);
        OTSmartContract* pSmartContract =
            dynamic_cast<OTSmartContract*>(pTrackable);

        if (nullptr != pPlan) {
            lowLevelSetTempValuesFromPaymentPlan(*pPlan);
            return true;
        } else if (nullptr != pSmartContract) {
            lowLevelSetTempValuesFromSmartContract(*pSmartContract);
            return true;
        }
        // -------------------------------------------
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error: Apparently it's not a payment plan "
            "or smart contract – but was supposed to be. "
            "(Returning false).")
            .Flush();
    } else
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error: Wrong type. (Returning false).")
            .Flush();

    return false;
}

void OTPayment::lowLevelSetTempValuesFromPaymentPlan(
    const OTPaymentPlan& theInput)
{
    m_bAreTempValuesSet = true;
    m_bHasRecipient = true;
    m_bHasRemitter = false;

    // There're also regular payments of GetPaymentPlanAmount().
    // Can't fit 'em all.
    m_lAmount = theInput.GetInitialPaymentAmount();
    m_lTransactionNum = theInput.GetTransactionNum();
    m_lTransNumDisplay = theInput.GetRecipientOpeningNum();

    if (theInput.GetConsideration().Exists())
        m_strMemo->Set(theInput.GetConsideration());
    else
        m_strMemo->Release();

    m_InstrumentDefinitionID = theInput.GetInstrumentDefinitionID();
    m_NotaryID = theInput.GetNotaryID();

    m_SenderNymID = theInput.GetSenderNymID();
    m_SenderAcctID = theInput.GetSenderAcctID();

    m_RecipientNymID = theInput.GetRecipientNymID();
    m_RecipientAcctID = theInput.GetRecipientAcctID();

    m_RemitterNymID->Release();
    m_RemitterAcctID->Release();

    m_VALID_FROM = theInput.GetValidFrom();
    m_VALID_TO = theInput.GetValidTo();
}

bool OTPayment::SetTempValuesFromPaymentPlan(const OTPaymentPlan& theInput)
{
    if (OTPayment::PAYMENT_PLAN == m_Type) {
        lowLevelSetTempValuesFromPaymentPlan(theInput);
        return true;
    } else
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error: Wrong type. (Returning false).")
            .Flush();

    return false;
}

void OTPayment::lowLevelSetTempValuesFromSmartContract(
    const OTSmartContract& theInput)
{
    m_bAreTempValuesSet = true;
    m_bHasRecipient = false;
    m_bHasRemitter = false;

    m_lAmount = 0;  // not used here.
    m_lTransactionNum = theInput.GetTransactionNum();
    //  m_lTransNumDisplay = theInput.GetTransactionNum();

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

    const std::vector<std::int64_t>& openingNumsInOrderOfSigning =
        theInput.openingNumsInOrderOfSigning();

    m_lTransNumDisplay = openingNumsInOrderOfSigning.size() > 0
                             ? openingNumsInOrderOfSigning[0]
                             : m_lTransactionNum;

    // Note: Maybe later, store the Smart Contract's temporary name, or ID,
    // in the memo field.
    // Or something.
    //
    m_strMemo->Release();  // not used here.

    m_NotaryID = theInput.GetNotaryID();
    m_InstrumentDefinitionID->Release();  // not used here.

    m_SenderNymID = theInput.GetSenderNymID();
    m_SenderAcctID->Release();

    m_RecipientNymID->Release();   // not used here.
    m_RecipientAcctID->Release();  // not used here.

    m_RemitterNymID->Release();
    m_RemitterAcctID->Release();

    m_VALID_FROM = theInput.GetValidFrom();
    m_VALID_TO = theInput.GetValidTo();
}

bool OTPayment::SetTempValuesFromSmartContract(const OTSmartContract& theInput)
{
    if (OTPayment::SMART_CONTRACT == m_Type) {
        lowLevelSetTempValuesFromSmartContract(theInput);
        return true;
    } else
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Error: Wrong type. (Returning false).")
            .Flush();

    return false;
}

bool OTPayment::GetMemo(String& strOutput) const
{
    strOutput.Release();

    if (!m_bAreTempValuesSet) return false;

    bool bSuccess = false;

    switch (m_Type) {
        case OTPayment::CHEQUE:
        case OTPayment::VOUCHER:
        case OTPayment::INVOICE:
        case OTPayment::PAYMENT_PLAN:
        case OTPayment::NOTICE:
            if (m_strMemo->Exists()) {
                strOutput.Set(m_strMemo);
                bSuccess = true;
            } else
                bSuccess = false;
            break;

        case OTPayment::SMART_CONTRACT:
        case OTPayment::PURSE:
            bSuccess = false;
            break;

        default:
            LogOutput(OT_METHOD)(__FUNCTION__)(": Bad payment type!").Flush();
            break;
    }

    return bSuccess;
}

bool OTPayment::GetAmount(std::int64_t& lOutput) const
{
    lOutput = 0;

    if (!m_bAreTempValuesSet) return false;

    bool bSuccess = false;

    switch (m_Type) {
        case OTPayment::CHEQUE:
        case OTPayment::VOUCHER:
        case OTPayment::INVOICE:
        case OTPayment::PAYMENT_PLAN:
        case OTPayment::PURSE:
            lOutput = m_lAmount;
            bSuccess = true;
            break;

        case OTPayment::NOTICE:
        case OTPayment::SMART_CONTRACT:
            lOutput = 0;
            bSuccess = false;
            break;

        default:
            LogOutput(OT_METHOD)(__FUNCTION__)(": Bad payment type!").Flush();
            break;
    }

    return bSuccess;
}

bool OTPayment::GetAllTransactionNumbers(NumList& numlistOutput) const
{
    OT_ASSERT_MSG(
        m_bAreTempValuesSet,
        "Temp values weren't even set! Should "
        "NOT have called this function at all.");

    // SMART CONTRACTS and PAYMENT PLANS get a little special treatment
    // here at the top. Notice, BTW, that you MUST call SetTempValues
    // before doing this, otherwise the m_Type isn't even set!
    //
    if (  // (false == m_bAreTempValuesSet)    || // Why is this here? Because
          // if temp values haven't been set yet,
        (OTPayment::SMART_CONTRACT == m_Type) ||  // then m_Type isn't set
                                                  // either. We only want
                                                  // smartcontracts and
        (OTPayment::PAYMENT_PLAN == m_Type))  // payment plans here, but without
                                              // m_Type we can't know the
                                              // type...
    {  // ===> UPDATE: m_Type IS set!! This comment is wrong!
        OTTrackable* pTrackable = Instantiate();
        if (nullptr == pTrackable) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed instantiating OTPayment containing cron item: ")(
                m_strPayment)(".")
                .Flush();
            return false;
        }  // Below THIS POINT, MUST DELETE pTrackable!
        std::unique_ptr<OTTrackable> theTrackableAngel(pTrackable);

        OTPaymentPlan* pPlan = dynamic_cast<OTPaymentPlan*>(pTrackable);
        OTSmartContract* pSmartContract =
            dynamic_cast<OTSmartContract*>(pTrackable);

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
    else if (OTPayment::NOTICE == m_Type) {
        std::unique_ptr<OTTransaction> pNotice(InstantiateNotice());

        if (!pNotice) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed instantiating OTPayment containing a notice: ")(
                m_strPayment)(".")
                .Flush();
            return false;
        }
        auto strCronItem = String::Factory();
        // -------------------------------------------
        auto pItem = pNotice->GetItem(itemType::notice);

        if (false != bool(pItem))         // The item's NOTE, as opposed to the
                                          // transaction's reference string,
            pItem->GetNote(strCronItem);  // contains the updated version of the
                                          // cron item, versus the original.
        // -------------------------------------------
        if (!strCronItem->Exists())
            pNotice->GetReferenceString(strCronItem);  // Didn't find the
                                                       // updated one? Okay
                                                       // let's grab the
                                                       // original then.
        // -------------------------------------------
        if (!strCronItem->Exists()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed geting reference string (containing cron item) "
                "from instantiated OTPayment: ")(m_strPayment)(".")
                .Flush();
            return false;
        }
        // -------------------------------------------
        std::unique_ptr<OTPayment> pCronItemPayment(
            new OTPayment(api_, strCronItem));

        if (!pCronItemPayment || !pCronItemPayment->IsValid() ||
            !pCronItemPayment->SetTempValues()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed instantiating or verifying a "
                "(purported) cron item: ")(strCronItem)(".")
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
        return pCronItemPayment->GetAllTransactionNumbers(numlistOutput);
    }
    // ------------------------------------------------------
    // Next: ALL OTHER payment types...
    //
    bool bSuccess = false;

    switch (m_Type) {
        case OTPayment::CHEQUE:
        case OTPayment::VOUCHER:
        case OTPayment::INVOICE:
            if (m_lTransactionNum > 0) numlistOutput.Add(m_lTransactionNum);
            bSuccess = true;
            break;

        case OTPayment::PURSE:
            bSuccess = false;
            break;

        default:
        case OTPayment::PAYMENT_PLAN:  // Should never happen. (Handled already
                                       // above.)
        case OTPayment::SMART_CONTRACT:  // Should never happen. (Handled
                                         // already above.)
        case OTPayment::NOTICE:  // Should never happen. (Handled already
                                 // above.)
            LogOutput(OT_METHOD)(__FUNCTION__)(": Bad payment type!").Flush();
            break;
    }

    return bSuccess;
}

// This works with a cheque who has a transaction number.
// It also works with a payment plan or smart contract, for opening AND closing
// numbers.
bool OTPayment::HasTransactionNum(const std::int64_t& lInput) const
{
    OT_ASSERT_MSG(
        m_bAreTempValuesSet,
        "Should never call this method unless "
        "you have first set the temp values.");

    // SMART CONTRACTS and PAYMENT PLANS get a little special
    // treatment here at the top.
    //
    if (  // (false == m_bAreTempValuesSet)     ||
        (OTPayment::SMART_CONTRACT == m_Type) ||
        (OTPayment::PAYMENT_PLAN == m_Type)) {
        OTTrackable* pTrackable = Instantiate();
        if (nullptr == pTrackable) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed instantiating OTPayment containing: ")(m_strPayment)(
                ".")
                .Flush();
            return false;
        }  // BELOW THIS POINT, MUST DELETE pTrackable!
        std::unique_ptr<OTTrackable> theTrackableAngel(pTrackable);

        OTPaymentPlan* pPlan = nullptr;
        OTSmartContract* pSmartContract = nullptr;

        pPlan = dynamic_cast<OTPaymentPlan*>(pTrackable);
        pSmartContract = dynamic_cast<OTSmartContract*>(pTrackable);

        if (nullptr != pPlan)
            return pPlan->HasTransactionNum(lInput);
        else if (nullptr != pSmartContract)
            return pSmartContract->HasTransactionNum(lInput);

        return false;
    }
    // ------------------------------------------------------
    // Notice from the server (in our Nym's record box probably)
    // which is in reference to a sent payment plan or smart contract.
    //
    else if (OTPayment::NOTICE == m_Type) {
        std::unique_ptr<OTTransaction> pNotice(InstantiateNotice());

        if (!pNotice) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed instantiating OTPayment containing a notice: ")(
                m_strPayment)(".")
                .Flush();
            return false;
        }
        auto strCronItem = String::Factory();
        // -------------------------------------------
        auto pItem = pNotice->GetItem(itemType::notice);

        if (false != bool(pItem))         // The item's NOTE, as opposed to the
                                          // transaction's reference string,
            pItem->GetNote(strCronItem);  // contains the updated version of the
                                          // cron item, versus the original.
        // -------------------------------------------
        if (!strCronItem->Exists())
            pNotice->GetReferenceString(strCronItem);  // Didn't find the
                                                       // updated one? Okay
                                                       // let's grab the
                                                       // original then.
        // -------------------------------------------
        if (!strCronItem->Exists()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed geting reference string (containing cron item) "
                "from instantiated OTPayment: ")(m_strPayment)(".")
                .Flush();
            return false;
        }
        // -------------------------------------------
        std::unique_ptr<OTPayment> pCronItemPayment(
            new OTPayment(api_, strCronItem));

        if (!pCronItemPayment || !pCronItemPayment->IsValid() ||
            !pCronItemPayment->SetTempValues()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed instantiating or verifying a "
                "(purported) cron item: ")(strCronItem)(".")
                .Flush();
            return false;
        }
        // -------------------------------------------
        return pCronItemPayment->HasTransactionNum(lInput);
    }
    // ------------------------------------------------------
    // Next: ALL OTHER payment types...
    //
    bool bSuccess = false;

    switch (m_Type) {
        case OTPayment::CHEQUE:
        case OTPayment::VOUCHER:
        case OTPayment::INVOICE:
            if (lInput == m_lTransactionNum) bSuccess = true;
            break;

        case OTPayment::PURSE:
            bSuccess = false;
            break;

        default:
        case OTPayment::PAYMENT_PLAN:  // Should never happen. (Handled already
                                       // above.)
        case OTPayment::SMART_CONTRACT:  // Should never happen. (Handled
                                         // already above.)
        case OTPayment::NOTICE:  // Should never happen. (Handled already
                                 // above.)
            LogOutput(OT_METHOD)(__FUNCTION__)(": Bad payment type!").Flush();
            break;
    }

    return bSuccess;
}

bool OTPayment::GetClosingNum(
    std::int64_t& lOutput,
    const Identifier& theAcctID) const
{
    lOutput = 0;

    // SMART CONTRACTS and PAYMENT PLANS get a little special
    // treatment here at the top.
    //
    if ((false == m_bAreTempValuesSet) ||  // m_Type isn't set if this is false.
        (OTPayment::SMART_CONTRACT == m_Type) ||
        (OTPayment::PAYMENT_PLAN == m_Type)) {
        OTTrackable* pTrackable = Instantiate();
        if (nullptr == pTrackable) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed instantiating OTPayment containing: ")(m_strPayment)(
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
            if (lOutput > 0) return true;
            return false;
        } else if (nullptr != pPlan) {
            lOutput = pPlan->GetClosingNumber(theAcctID);
            if (lOutput > 0) return true;
            return false;
        }

        // There's no "return false" here because of the "if
        // !m_bAreTempValuesSet"
        // In other words, it still very well could be a cheque or invoice, or
        // whatever.
    }

    if (!m_bAreTempValuesSet) return false;
    // --------------------------------------
    if (OTPayment::NOTICE == m_Type) {
        std::unique_ptr<OTTransaction> pNotice(InstantiateNotice());

        if (!pNotice) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed instantiating OTPayment containing a notice: ")(
                m_strPayment)(".")
                .Flush();
            return false;
        }
        auto strCronItem = String::Factory();
        // -------------------------------------------
        auto pItem = pNotice->GetItem(itemType::notice);

        if (false != bool(pItem))         // The item's NOTE, as opposed to the
                                          // transaction's reference string,
            pItem->GetNote(strCronItem);  // contains the updated version of the
                                          // cron item, versus the original.
        // -------------------------------------------
        if (!strCronItem->Exists())
            pNotice->GetReferenceString(strCronItem);  // Didn't find the
                                                       // updated one? Okay
                                                       // let's grab the
                                                       // original then.
        // -------------------------------------------
        if (!strCronItem->Exists()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed geting reference string (containing cron item) "
                "from instantiated OTPayment: ")(m_strPayment)(".")
                .Flush();
            return false;
        }
        // -------------------------------------------
        std::unique_ptr<OTPayment> pCronItemPayment(
            new OTPayment(api_, strCronItem));

        if (!pCronItemPayment || !pCronItemPayment->IsValid() ||
            !pCronItemPayment->SetTempValues()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed instantiating or verifying a "
                "(purported) cron item: ")(strCronItem)(".")
                .Flush();
            return false;
        }
        // -------------------------------------------
        return pCronItemPayment->GetClosingNum(lOutput, theAcctID);
    }
    // --------------------------------------
    // Next: ALL OTHER payment types...
    //
    bool bSuccess = false;

    switch (m_Type) {

        case OTPayment::CHEQUE:
        case OTPayment::VOUCHER:
        case OTPayment::INVOICE:
        case OTPayment::PURSE:
            lOutput = 0;  // Redundant, but just making sure.
            bSuccess = false;
            break;

        default:
        case OTPayment::PAYMENT_PLAN:
        case OTPayment::SMART_CONTRACT:
        case OTPayment::NOTICE:
            LogOutput(OT_METHOD)(__FUNCTION__)(": Bad payment type!").Flush();
            break;
    }

    return bSuccess;
}

bool OTPayment::GetOpeningNum(std::int64_t& lOutput, const Identifier& theNymID)
    const
{
    lOutput = 0;

    // SMART CONTRACTS and PAYMENT PLANS get a little special
    // treatment here at the top.
    //
    if ((false == m_bAreTempValuesSet) ||  // m_Type isn't available if this is
                                           // false.
        (OTPayment::SMART_CONTRACT == m_Type) ||
        (OTPayment::PAYMENT_PLAN == m_Type)) {
        OTTrackable* pTrackable = Instantiate();
        if (nullptr == pTrackable) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed instantiating OTPayment containing: ")(m_strPayment)(
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
            if (lOutput > 0) return true;
            return false;
        } else if (nullptr != pPlan) {
            lOutput = pPlan->GetOpeningNumber(theNymID);
            if (lOutput > 0) return true;
            return false;
        }

        // There's no "return false" here because of the "if
        // !m_bAreTempValuesSet"
        // In other words, it still very well could be a cheque or invoice, or
        // whatever.
    }

    if (!m_bAreTempValuesSet) return false;
    // --------------------------------------
    if (OTPayment::NOTICE == m_Type) {
        std::unique_ptr<OTTransaction> pNotice(InstantiateNotice());

        if (!pNotice) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed instantiating OTPayment containing a notice: ")(
                m_strPayment)(".")
                .Flush();
            return false;
        }
        auto strCronItem = String::Factory();
        // -------------------------------------------
        auto pItem = pNotice->GetItem(itemType::notice);

        if (false != bool(pItem))         // The item's NOTE, as opposed to the
                                          // transaction's reference string,
            pItem->GetNote(strCronItem);  // contains the updated version of the
                                          // cron item, versus the original.
        // -------------------------------------------
        if (!strCronItem->Exists())
            pNotice->GetReferenceString(strCronItem);  // Didn't find the
                                                       // updated one? Okay
                                                       // let's grab the
                                                       // original then.
        // -------------------------------------------
        if (!strCronItem->Exists()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed geting reference string (containing cron item) "
                "from instantiated OTPayment: ")(m_strPayment)(".")
                .Flush();
            return false;
        }
        // -------------------------------------------
        std::unique_ptr<OTPayment> pCronItemPayment(
            new OTPayment(api_, strCronItem));

        if (!pCronItemPayment || !pCronItemPayment->IsValid() ||
            !pCronItemPayment->SetTempValues()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failed instantiating or verifying a "
                "(purported) cron item: ")(strCronItem)(".")
                .Flush();
            return false;
        }
        // -------------------------------------------
        return pCronItemPayment->GetOpeningNum(lOutput, theNymID);
    }
    // --------------------------------------
    // Next: ALL OTHER payment types...
    //
    bool bSuccess = false;

    switch (m_Type) {

        case OTPayment::CHEQUE:
        case OTPayment::INVOICE:
            if (theNymID == m_SenderNymID) {
                lOutput =
                    m_lTransactionNum;  // The "opening" number for a cheque is
                                        // the ONLY number it has.
                bSuccess = true;
            } else {
                lOutput = 0;
                bSuccess = false;
            }
            break;

        case OTPayment::VOUCHER:
            if (theNymID == m_RemitterNymID) {
                lOutput =
                    m_lTransactionNum;  // The "opening" number for a cheque is
                                        // the ONLY number it has.
                bSuccess = true;
            } else {
                lOutput = 0;
                bSuccess = false;
            }
            break;

        case OTPayment::PURSE:
            lOutput = 0;  // Redundant, but just making sure.
            bSuccess = false;
            break;

        default:
        case OTPayment::PAYMENT_PLAN:
        case OTPayment::SMART_CONTRACT:
        case OTPayment::NOTICE:
            LogOutput(OT_METHOD)(__FUNCTION__)(": Bad payment type!").Flush();
            break;
    }

    return bSuccess;
}

bool OTPayment::GetTransNumDisplay(std::int64_t& lOutput) const
{
    lOutput = 0;

    if (!m_bAreTempValuesSet) return false;

    bool bSuccess = false;

    switch (m_Type) {
        case OTPayment::CHEQUE:
        case OTPayment::VOUCHER:
        case OTPayment::INVOICE:
            lOutput = m_lTransactionNum;
            bSuccess = true;
            break;

        case OTPayment::PAYMENT_PLAN:  // For payment plans, this is the opening
            // transaction FOR THE RECIPIENT NYM (The merchant.)
            lOutput = m_lTransNumDisplay;
            bSuccess = true;
            break;

        case OTPayment::SMART_CONTRACT:  // For smart contracts, this is the
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
            // NOTE: the above logic is performed where m_lTransNumDisplay is
            // set.
            lOutput = m_lTransNumDisplay;
            bSuccess = true;
            break;

        case OTPayment::NOTICE:
            lOutput = m_lTransNumDisplay;
            bSuccess = true;
            break;

        case OTPayment::PURSE:
            lOutput = 0;
            bSuccess = false;
            break;

        default:
            LogOutput(OT_METHOD)(__FUNCTION__)(": Bad payment type!").Flush();
            break;
    }

    return bSuccess;
}

bool OTPayment::GetTransactionNum(std::int64_t& lOutput) const
{
    lOutput = 0;

    if (!m_bAreTempValuesSet) return false;

    bool bSuccess = false;

    switch (m_Type) {
        case OTPayment::NOTICE:
        case OTPayment::CHEQUE:
        case OTPayment::VOUCHER:
        case OTPayment::INVOICE:
        case OTPayment::PAYMENT_PLAN:  // For payment plans, this is the opening
        // transaction FOR THE NYM who activated the
        // contract (probably the customer.)
        case OTPayment::SMART_CONTRACT:  // For smart contracts, this is the
                                         // opening
                                         // transaction number FOR THE NYM who
                                         // activated the contract.
            lOutput = m_lTransactionNum;
            bSuccess = true;
            break;

        case OTPayment::PURSE:
            lOutput = 0;
            bSuccess = false;
            break;

        default:
            LogOutput(OT_METHOD)(__FUNCTION__)(": Bad payment type!").Flush();
            break;
    }

    return bSuccess;
}

bool OTPayment::GetValidFrom(time64_t& tOutput) const
{
    tOutput = OT_TIME_ZERO;

    if (!m_bAreTempValuesSet) return false;

    bool bSuccess = false;

    switch (m_Type) {
        case OTPayment::PURSE:
        case OTPayment::NOTICE:
        case OTPayment::CHEQUE:
        case OTPayment::VOUCHER:
        case OTPayment::INVOICE:
        case OTPayment::PAYMENT_PLAN:
        case OTPayment::SMART_CONTRACT:
            tOutput = m_VALID_FROM;
            bSuccess = true;
            break;

        default:
            LogOutput(OT_METHOD)(__FUNCTION__)(": Bad payment type!").Flush();
            break;
    }

    return bSuccess;
}

bool OTPayment::GetValidTo(time64_t& tOutput) const
{
    tOutput = OT_TIME_ZERO;

    if (!m_bAreTempValuesSet) return false;

    bool bSuccess = false;

    switch (m_Type) {
        case OTPayment::PURSE:
        case OTPayment::NOTICE:
        case OTPayment::CHEQUE:
        case OTPayment::VOUCHER:
        case OTPayment::INVOICE:
        case OTPayment::PAYMENT_PLAN:
        case OTPayment::SMART_CONTRACT:
            tOutput = m_VALID_TO;
            bSuccess = true;
            break;

        default:
            LogOutput(OT_METHOD)(__FUNCTION__)(": Bad payment type!").Flush();
            break;
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
bool OTPayment::IsExpired(bool& bExpired)
{
    if (!m_bAreTempValuesSet) return false;

    const time64_t CURRENT_TIME = OTTimeGetCurrentTime();

    // If the current time is AFTER the valid-TO date,
    // AND the valid_to is a nonzero number (0 means "doesn't expire")
    // THEN return true (it's expired.)
    //
    if ((CURRENT_TIME >= m_VALID_TO) && (m_VALID_TO > OT_TIME_ZERO))
        bExpired = true;
    else
        bExpired = false;

    return true;
}

// Verify whether the CURRENT date is WITHIN the VALID FROM / TO dates.
//
bool OTPayment::VerifyCurrentDate(bool& bVerified)
{
    if (!m_bAreTempValuesSet) return false;

    const time64_t CURRENT_TIME = OTTimeGetCurrentTime();

    if ((CURRENT_TIME >= m_VALID_FROM) &&
        ((CURRENT_TIME <= m_VALID_TO) || (OT_TIME_ZERO == m_VALID_TO)))
        bVerified = true;
    else
        bVerified = false;

    return true;
}

bool OTPayment::GetInstrumentDefinitionID(Identifier& theOutput) const
{
    theOutput.Release();

    if (!m_bAreTempValuesSet) return false;

    bool bSuccess = false;

    switch (m_Type) {
        case OTPayment::CHEQUE:
        case OTPayment::VOUCHER:
        case OTPayment::INVOICE:
        case OTPayment::PAYMENT_PLAN:
        case OTPayment::PURSE:
        case OTPayment::NOTICE:
            theOutput.Assign(m_InstrumentDefinitionID);
            bSuccess = !m_InstrumentDefinitionID->empty();
            break;

        case OTPayment::SMART_CONTRACT:
            bSuccess = false;
            break;

        default:
            LogOutput(OT_METHOD)(__FUNCTION__)(": Bad payment type!").Flush();
            break;
    }

    return bSuccess;
}

bool OTPayment::GetNotaryID(Identifier& theOutput) const
{
    theOutput.Release();

    if (!m_bAreTempValuesSet) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Object not yet instantiated.")
            .Flush();

        return false;
    }

    bool bSuccess = false;

    switch (m_Type) {
        case OTPayment::CHEQUE:
        case OTPayment::VOUCHER:
        case OTPayment::INVOICE:
        case OTPayment::PAYMENT_PLAN:
        case OTPayment::SMART_CONTRACT:
        case OTPayment::PURSE:
        case OTPayment::NOTICE:
            theOutput.Assign(m_NotaryID);
            bSuccess = !m_NotaryID->empty();
            break;

        default:
            LogOutput(OT_METHOD)(__FUNCTION__)(": Bad payment type!").Flush();
            break;
    }

    return bSuccess;
}

// With a voucher (cashier's cheque) the "bank" is the "sender",
// whereas the actual Nym who purchased it is the "remitter."
//
bool OTPayment::GetRemitterNymID(Identifier& theOutput) const
{
    theOutput.Release();

    if (!m_bAreTempValuesSet) return false;

    bool bSuccess = false;

    switch (m_Type) {
        case OTPayment::VOUCHER:
            theOutput.Assign(m_RemitterNymID);
            bSuccess = !m_RemitterNymID->empty();
            break;

        default:
            LogOutput(OT_METHOD)(__FUNCTION__)(": Bad payment type! Expected a "
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
bool OTPayment::GetRemitterAcctID(Identifier& theOutput) const
{
    theOutput.Release();

    if (!m_bAreTempValuesSet) return false;

    bool bSuccess = false;

    switch (m_Type) {
        case OTPayment::VOUCHER:
            theOutput.Assign(m_RemitterAcctID);
            bSuccess = !m_RemitterAcctID->empty();
            break;

        default:
            LogOutput(OT_METHOD)(__FUNCTION__)(": Bad payment type! Expected a "
                                               "voucher cheque.")
                .Flush();
            break;
    }

    return bSuccess;
}

bool OTPayment::GetSenderNymIDForDisplay(Identifier& theOutput) const
{
    if (IsVoucher()) return GetRemitterNymID(theOutput);

    return GetSenderNymID(theOutput);
}

bool OTPayment::GetSenderAcctIDForDisplay(Identifier& theOutput) const
{
    if (IsVoucher()) return GetRemitterAcctID(theOutput);

    return GetSenderAcctID(theOutput);
}

bool OTPayment::GetSenderNymID(Identifier& theOutput) const
{
    theOutput.Release();

    if (!m_bAreTempValuesSet) return false;

    bool bSuccess = false;

    switch (m_Type) {
        case OTPayment::CHEQUE:
        case OTPayment::VOUCHER:
        case OTPayment::INVOICE:
        case OTPayment::PAYMENT_PLAN:
        case OTPayment::SMART_CONTRACT:
        case OTPayment::NOTICE:
            theOutput.Assign(m_SenderNymID);
            bSuccess = !m_SenderNymID->empty();
            break;

        case OTPayment::PURSE:
            bSuccess = false;
            break;

        default:
            LogOutput(OT_METHOD)(__FUNCTION__)(": Bad payment type!").Flush();
            break;
    }

    return bSuccess;
}

bool OTPayment::GetSenderAcctID(Identifier& theOutput) const
{
    theOutput.Release();

    if (!m_bAreTempValuesSet) return false;

    bool bSuccess = false;

    switch (m_Type) {
        case OTPayment::CHEQUE:
        case OTPayment::VOUCHER:
        case OTPayment::INVOICE:
        case OTPayment::PAYMENT_PLAN:
        case OTPayment::NOTICE:
            theOutput.Assign(m_SenderAcctID);
            bSuccess = !m_SenderAcctID->empty();
            break;

        case OTPayment::SMART_CONTRACT:
        case OTPayment::PURSE:
            bSuccess = false;
            break;

        default:
            LogOutput(OT_METHOD)(__FUNCTION__)(": Bad payment type!").Flush();
            break;
    }

    return bSuccess;
}

bool OTPayment::GetRecipientNymID(Identifier& theOutput) const
{
    theOutput.Release();

    if (!m_bAreTempValuesSet) return false;

    bool bSuccess = false;

    switch (m_Type) {
        case OTPayment::CHEQUE:
        case OTPayment::VOUCHER:
        case OTPayment::INVOICE:
        case OTPayment::PAYMENT_PLAN:
        case OTPayment::PURSE:
        case OTPayment::NOTICE:
            if (m_bHasRecipient) {
                theOutput.Assign(m_RecipientNymID);
                bSuccess = !m_RecipientNymID->empty();
            } else
                bSuccess = false;

            break;

        case OTPayment::SMART_CONTRACT:
            bSuccess = false;
            break;

        default:
            LogOutput(OT_METHOD)(__FUNCTION__)(": Bad payment type!").Flush();
            break;
    }

    return bSuccess;
}

bool OTPayment::GetRecipientAcctID(Identifier& theOutput) const
{
    // NOTE:
    // A cheque HAS NO "Recipient Asset Acct ID", since the recipient's account
    // (where he deposits
    // the cheque) is not known UNTIL the time of the deposit. It's certainly
    // not
    // known at the time
    // that the cheque is written...

    theOutput.Release();

    if (!m_bAreTempValuesSet) return false;

    bool bSuccess = false;

    switch (m_Type) {
        case OTPayment::PAYMENT_PLAN:
        case OTPayment::NOTICE:
            if (m_bHasRecipient) {
                theOutput.Assign(m_RecipientAcctID);
                bSuccess = !m_RecipientAcctID->empty();
            } else
                bSuccess = false;

            break;

        case OTPayment::CHEQUE:
        case OTPayment::VOUCHER:
        case OTPayment::INVOICE:
        case OTPayment::SMART_CONTRACT:
        case OTPayment::PURSE:  // A purse might have a recipient USER, but
                                // never a
                                // recipient ACCOUNT.
            bSuccess = false;
            break;

        default:
            LogOutput(OT_METHOD)(__FUNCTION__)(": Bad payment type!").Flush();
            break;
    }

    return bSuccess;
}

void OTPayment::InitPayment()
{
    m_Type = OTPayment::ERROR_STATE;
    m_lAmount = 0;
    m_lTransactionNum = 0;
    m_lTransNumDisplay = 0;
    m_VALID_FROM = OT_TIME_ZERO;
    m_VALID_TO = OT_TIME_ZERO;
    m_bAreTempValuesSet = false;
    m_bHasRecipient = false;
    m_bHasRemitter = false;
    m_strContractType->Set("PAYMENT");
}

// CALLER is responsible to delete.
//
OTTrackable* OTPayment::Instantiate() const
{
    std::unique_ptr<Contract> pContract;
    OTTrackable* pTrackable = nullptr;
    Cheque* pCheque = nullptr;
    OTPaymentPlan* pPaymentPlan = nullptr;
    OTSmartContract* pSmartContract = nullptr;

    switch (m_Type) {
        case CHEQUE:
        case VOUCHER:
        case INVOICE:
            pContract = api_.Factory().Contract(m_strPayment);

            if (false != bool(pContract)) {
                pCheque = dynamic_cast<Cheque*>(pContract.release());

                if (nullptr == pCheque) {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Tried to instantiate cheque, "
                        "but factory returned non-cheque: ")(m_strPayment)(".")
                        .Flush();
                } else
                    pTrackable = pCheque;
            } else
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Tried to instantiate cheque, but "
                    "factory returned nullptr: ")(m_strPayment)(".")
                    .Flush();
            break;

        case PAYMENT_PLAN:
            pContract = api_.Factory().Contract(m_strPayment);

            if (false != bool(pContract)) {
                pPaymentPlan =
                    dynamic_cast<OTPaymentPlan*>(pContract.release());

                if (nullptr == pPaymentPlan) {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Tried to instantiate payment "
                        "plan, but factory returned non-payment-plan: ")(
                        m_strPayment)(".")
                        .Flush();
                } else
                    pTrackable = pPaymentPlan;
            } else
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Tried to instantiate payment "
                    "plan, but factory returned nullptr: ")(m_strPayment)(".")
                    .Flush();
            break;

        case SMART_CONTRACT:
            pContract = api_.Factory().Contract(m_strPayment);

            if (false != bool(pContract)) {
                pSmartContract =
                    dynamic_cast<OTSmartContract*>(pContract.release());

                if (nullptr == pSmartContract) {
                    LogOutput(OT_METHOD)(__FUNCTION__)(
                        ": Tried to instantiate smart contract, but factory "
                        "returned non-smart-contract: ")(m_strPayment)(".")
                        .Flush();
                } else
                    pTrackable = pSmartContract;
            } else
                LogOutput(OT_METHOD)(__FUNCTION__)(
                    ": Tried to instantiate smart "
                    "contract, but factory returned nullptr: ")(m_strPayment)(
                    ".")
                    .Flush();
            break;

        case PURSE:
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": ERROR: Tried to instantiate purse, "
                "but should have called OTPayment::InstantiatePurse.")
                .Flush();
            return nullptr;

        case NOTICE:
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": ERROR: Tried to instantiate a notice, "
                "but should have called OTPayment::InstantiateNotice.")
                .Flush();
            return nullptr;

        default:
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": ERROR: Tried to instantiate payment "
                "object, but had a bad type. Contents: ")(m_strPayment)(".")
                .Flush();
            return nullptr;
    }

    return pTrackable;
}

OTTrackable* OTPayment::Instantiate(const String& strPayment)
{
    if (SetPayment(strPayment)) return Instantiate();

    return nullptr;
}

OTTransaction* OTPayment::InstantiateNotice(const String& strNotice)
{
    if (!SetPayment(strNotice))
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": WARNING: Failed setting the "
            "notice string based on "
            "what was passed in: ")(strNotice)(".")
            .Flush();
    else if (OTPayment::NOTICE != m_Type)
        LogOutput(OT_METHOD)(__FUNCTION__)(": WARNING: No notice was found in "
                                           "provided string: ")(strNotice)(".")
            .Flush();
    else
        return InstantiateNotice();

    return nullptr;
}

OTTransaction* OTPayment::InstantiateNotice() const
{
    if (m_strPayment->Exists() && (OTPayment::NOTICE == GetType())) {
        auto pType = api_.Factory().Transaction(m_strPayment);

        if (false == bool(pType)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failure 1: This payment object does "
                "NOT contain a notice. "
                "Contents: ")(m_strPayment)(".")
                .Flush();
            return nullptr;
        }

        OTTransaction* pNotice = dynamic_cast<OTTransaction*>(pType.release());

        if (nullptr == pNotice) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Failure 2: This payment object does "
                "NOT contain a notice. "
                "Contents: ")(m_strPayment)(".")
                .Flush();
            return nullptr;
        }

        return pNotice;
    } else
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failure 3: This payment object does NOT contain a notice. "
            "Contents: ")(m_strPayment)(".")
            .Flush();

    return nullptr;
}

#if OT_CASH
// You need the server ID to instantiate a purse, unlike all the
// other payment types. UPDATE: Not anymore.
//
// CALLER is responsible to delete!
//
Purse* OTPayment::InstantiatePurse(const api::Wallet& wallet) const
{
    if (OTPayment::PURSE == GetType()) {
        auto purse = api_.Factory().Purse(m_strPayment);
        OT_ASSERT(false != bool(purse));
        return purse.release();
    } else
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failure: This payment object "
                                           "does NOT contain a purse. "
                                           "Contents: ")(m_strPayment)(".")
            .Flush();

    return nullptr;
}

Purse* OTPayment::InstantiatePurse(
    const api::Wallet& wallet,
    const String& strPayment)
{
    if (!SetPayment(strPayment))
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": WARNING: Failed setting the "
            "payment string based on "
            "what was passed in: ")(strPayment)(".")
            .Flush();
    else if (OTPayment::PURSE != m_Type)
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": WARNING: No purse was found in "
            "the payment string: ")(strPayment)(".")
            .Flush();
    else
        return InstantiatePurse(wallet);

    return nullptr;
}
#endif  // OT_CASH

bool OTPayment::IsCancelledCheque()
{
    if (false == m_bAreTempValuesSet) {
        if (false == SetTempValues()) {
            LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to set temp values.")
                .Flush();

            return false;
        }
    }

    OT_ASSERT(m_bAreTempValuesSet)

    if (false == IsCheque()) { return false; }

    auto sender = Identifier::Factory();
    auto recipient = Identifier::Factory();
    Amount amount{0};

    if (false == GetSenderNymID(sender)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to get sender nym id.")
            .Flush();

        return false;
    }

    if (false == GetRecipientNymID(recipient)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to get recipient nym id.")
            .Flush();

        return false;
    }

    if (sender != recipient) { return false; }

    if (false == GetAmount(amount)) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Failed to amount.").Flush();

        return false;
    }

    if (0 != amount) { return false; }

    return true;
}

std::int32_t OTPayment::ProcessXMLNode(irr::io::IrrXMLReader*& xml)
{
    const auto strNodeName = String::Factory(xml->getNodeName());

    if (strNodeName->Compare("payment")) {
        m_strVersion = String::Factory(xml->getAttributeValue("version"));

        const auto strPaymentType =
            String::Factory(xml->getAttributeValue("type"));

        if (strPaymentType->Exists())
            m_Type = OTPayment::GetTypeFromString(strPaymentType);
        else
            m_Type = OTPayment::ERROR_STATE;

        LogTrace(OT_METHOD)(__FUNCTION__)(": Loaded payment... Type: ")(
            GetTypeString())
            .Flush();

        return (OTPayment::ERROR_STATE == m_Type) ? (-1) : 1;
    } else if (strNodeName->Compare("contents")) {
        auto strContents = String::Factory();

        if (!Contract::LoadEncodedTextField(xml, strContents) ||
            !strContents->Exists() || !SetPayment(strContents)) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": ERROR: Contents field "
                "without a value, OR error setting that "
                "value onto this object. Raw: ")(strContents)(".")
                .Flush();

            return (-1);  // error condition
        }
        // else success -- the value is now set on this object.
        // todo security: Make sure the type of the payment that's ACTUALLY
        // there
        // matches the type expected (based on the m_Type that we already read,
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
    m_Type = OTPayment::ERROR_STATE;
    m_lAmount = 0;
    m_lTransactionNum = 0;
    m_lTransNumDisplay = 0;
    m_VALID_FROM = OT_TIME_ZERO;
    m_VALID_TO = OT_TIME_ZERO;
    m_strPayment->Release();
    m_bAreTempValuesSet = false;
    m_bHasRecipient = false;
    m_bHasRemitter = false;
    m_strMemo->Release();
    m_InstrumentDefinitionID->Release();
    m_NotaryID->Release();
    m_SenderNymID->Release();
    m_SenderAcctID->Release();
    m_RecipientNymID->Release();
    m_RecipientAcctID->Release();
    m_RemitterNymID->Release();
    m_RemitterAcctID->Release();
}

bool OTPayment::SetPayment(const String& strPayment)
{
    if (!strPayment.Exists()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Empty input string.").Flush();

        return false;
    }

    auto strContract = String::Factory(strPayment.Get());

    if (!strContract->DecodeIfArmored(false))  // bEscapedIsAllowed=true
                                               // by default.
    {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Input string apparently was encoded and "
            "then failed decoding. Contents: ")(strPayment)(".")
            .Flush();
        return false;
    }

    m_strPayment->Release();

    // todo: should be "starts with" and perhaps with a trim first
    //
    if (strContract->Contains("-----BEGIN SIGNED CHEQUE-----"))
        m_Type = OTPayment::CHEQUE;
    else if (strContract->Contains("-----BEGIN SIGNED VOUCHER-----"))
        m_Type = OTPayment::VOUCHER;
    else if (strContract->Contains("-----BEGIN SIGNED INVOICE-----"))
        m_Type = OTPayment::INVOICE;

    else if (strContract->Contains("-----BEGIN SIGNED PAYMENT PLAN-----"))
        m_Type = OTPayment::PAYMENT_PLAN;
    else if (strContract->Contains("-----BEGIN SIGNED SMARTCONTRACT-----"))
        m_Type = OTPayment::SMART_CONTRACT;

    else if (strContract->Contains("-----BEGIN SIGNED PURSE-----"))
        m_Type = OTPayment::PURSE;
    else if (strContract->Contains("-----BEGIN SIGNED TRANSACTION-----"))
        m_Type = OTPayment::NOTICE;
    else {
        m_Type = OTPayment::ERROR_STATE;

        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Failure: Unable to determine payment type, from input: ")(
            strContract)(".")
            .Flush();
    }

    if (OTPayment::ERROR_STATE == m_Type) return false;

    m_strPayment->Set(strContract);

    return true;
}

void OTPayment::UpdateContents()
{
    // I release this because I'm about to repopulate it.
    m_xmlUnsigned->Release();

    Tag tag("payment");

    tag.add_attribute("version", m_strVersion->Get());
    tag.add_attribute("type", GetTypeString());

    if (m_strPayment->Exists()) {
        const auto ascContents = Armored::Factory(m_strPayment);

        if (ascContents->Exists()) {
            tag.add_tag("contents", ascContents->Get());
        }
    }

    std::string str_result;
    tag.output(str_result);

    m_xmlUnsigned->Concatenate("%s", str_result.c_str());
}

OTPayment::~OTPayment() { Release_Payment(); }
}  // namespace opentxs
