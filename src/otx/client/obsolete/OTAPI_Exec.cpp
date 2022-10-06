// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "internal/otx/client/obsolete/OTAPI_Exec.hpp"  // IWYU pragma: associated

#include <UnitDefinition.pb.h>
#include <compare>
#include <cstdint>
#include <memory>
#include <utility>

#include "internal/api/session/FactoryAPI.hpp"
#include "internal/otx/client/obsolete/OT_API.hpp"
#include "internal/otx/common/NumList.hpp"
#include "internal/otx/common/basket/Basket.hpp"
#include "internal/otx/common/recurring/OTPaymentPlan.hpp"
#include "internal/otx/common/script/OTScriptable.hpp"
#include "internal/otx/smartcontract/OTAgent.hpp"
#include "internal/otx/smartcontract/OTBylaw.hpp"
#include "internal/otx/smartcontract/OTClause.hpp"
#include "internal/otx/smartcontract/OTParty.hpp"
#include "internal/otx/smartcontract/OTPartyAccount.hpp"
#include "internal/otx/smartcontract/OTVariable.hpp"
#include "internal/serialization/protobuf/Proto.tpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Time.hpp"
#include "opentxs/api/crypto/Seed.hpp"
#include "opentxs/api/session/Crypto.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/api/session/Wallet.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/UnitType.hpp"
#include "opentxs/core/contract/BasketContract.hpp"
#include "opentxs/core/contract/ServerContract.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "opentxs/crypto/Language.hpp"
#include "opentxs/crypto/SeedStyle.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"
#include "opentxs/util/SharedPimpl.hpp"

#define OT_ERROR_AMOUNT INT64_MIN

namespace opentxs
{
#ifndef OT_ERROR
const std::int32_t OT_ERROR = (-1);
#endif

OTAPI_Exec::OTAPI_Exec(
    const api::Session& api,
    const api::session::Activity& activity,
    const api::session::Contacts& contacts,
    const api::network::ZMQ& zeromq,
    const OT_API& otapi,
    ContextLockCallback lockCallback)
    : api_(api)
    , ot_api_(otapi)
    , lock_callback_(std::move(lockCallback))
{
    // WARNING: do not access api_.Wallet() during construction
}

// PROPOSE PAYMENT PLAN
//
// (Return as STRING)
//
// PARAMETER NOTES:
// -- Payment Plan Delay, and Payment Plan Period, both default to 30 days (if
// you pass 0.)
//
// -- Payment Plan Length, and Payment Plan Max Payments, both default to 0,
// which means
//    no maximum length and no maximum number of payments.
//
// FYI, the payment plan creation process (finally) is:
//
// 1) Payment plan is written, and signed, by the recipient.  (This function:
// OTAPI_Exec::ProposePaymentPlan)
// 2) He sends it to the sender, who signs it and submits it.
// (OTAPI_Exec::ConfirmPaymentPlan and OTAPI_Exec::depositPaymentPlan)
// 3) The server loads the recipient nym to verify the transaction
//    number. The sender also had to burn a transaction number (to
//    submit it) so now, both have verified trns#s in this way.
//
auto OTAPI_Exec::ProposePaymentPlan(
    const UnallocatedCString& NOTARY_ID,
    const Time& VALID_FROM,  // Default (0 or nullptr) == current time
                             // measured
                             // in seconds since Jan 1970.
    const Time& VALID_TO,    // Default (0 or nullptr) == no expiry / cancel
    // anytime. Otherwise this is ADDED to VALID_FROM
    // (it's a length.)
    const UnallocatedCString& SENDER_ACCT_ID,  // Mandatory parameters. UPDATE:
                                               // Making sender Acct optional
                                               // here.
    const UnallocatedCString& SENDER_NYM_ID,   // Both sender and recipient must
                                               // sign before submitting.
    const UnallocatedCString& PLAN_CONSIDERATION,  // Like a memo.
    const UnallocatedCString& RECIPIENT_ACCT_ID,   // NOT optional.
    const UnallocatedCString& RECIPIENT_NYM_ID,    // Both sender and recipient
                                                 // must sign before submitting.
    const std::int64_t& INITIAL_PAYMENT_AMOUNT,  // zero or "" is no initial
                                                 // payment.
    const std::chrono::seconds& INITIAL_PAYMENT_DELAY,  // seconds from creation
                                                        // date. Default is zero
                                                        // or "".
    const std::int64_t& PAYMENT_PLAN_AMOUNT,         // Zero or "" is no regular
                                                     // payments.
    const std::chrono::seconds& PAYMENT_PLAN_DELAY,  // No. of seconds from
                                                     // creation date. Default
                                                     // is zero or "".
    const std::chrono::seconds& PAYMENT_PLAN_PERIOD,  // No. of seconds between
                                                      // payments. Default is
                                                      // zero or "".
    const std::chrono::seconds& PAYMENT_PLAN_LENGTH,  // In seconds. Defaults to
                                                      // 0 or "" (no maximum
                                                      // length.)
    const std::int32_t& PAYMENT_PLAN_MAX_PAYMENTS  // Integer. Defaults to 0 or
                                                   // ""
                                                   // (no
                                                   // maximum payments.)
) const -> UnallocatedCString
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(SENDER_NYM_ID);
    // NOTE: Making this optional for this step. (Since sender account may
    // not be known until the customer receives / confirms the payment plan.)
    //  OT_VERIFY_ID_STR(SENDER_ACCT_ID); // Optional parameter.
    OT_VERIFY_ID_STR(RECIPIENT_NYM_ID);
    OT_VERIFY_ID_STR(RECIPIENT_ACCT_ID);
    OT_VERIFY_STD_STR(PLAN_CONSIDERATION);
    OT_VERIFY_MIN_BOUND(INITIAL_PAYMENT_AMOUNT, 0);
    OT_VERIFY_MIN_BOUND(INITIAL_PAYMENT_DELAY, 0s);
    OT_VERIFY_MIN_BOUND(PAYMENT_PLAN_AMOUNT, 0);
    OT_VERIFY_MIN_BOUND(PAYMENT_PLAN_DELAY, 0s);
    OT_VERIFY_MIN_BOUND(PAYMENT_PLAN_PERIOD, 0s);
    OT_VERIFY_MIN_BOUND(PAYMENT_PLAN_LENGTH, 0s);
    OT_VERIFY_MIN_BOUND(PAYMENT_PLAN_MAX_PAYMENTS, 0);

    identifier::Generic angelSenderAcctId = identifier::Generic{};

    if (!SENDER_ACCT_ID.empty()) {
        angelSenderAcctId = api_.Factory().IdentifierFromBase58(SENDER_ACCT_ID);
    }

    std::unique_ptr<OTPaymentPlan> pPlan(ot_api_.ProposePaymentPlan(
        api_.Factory().NotaryIDFromBase58(NOTARY_ID),
        VALID_FROM,  // Default (0) == NOW
        VALID_TO,    // Default (0) == no expiry / cancel
                     // anytime We're making this acct optional
                     // here.
        // (Customer acct is unknown until confirmation by customer.)
        angelSenderAcctId,
        api_.Factory().NymIDFromBase58(SENDER_NYM_ID),
        PLAN_CONSIDERATION.empty()
            ? String::Factory("(Consideration for the agreement between "
                              "the parties is meant to be recorded "
                              "here.)")
            // Like a memo.
            : String::Factory(PLAN_CONSIDERATION),
        api_.Factory().IdentifierFromBase58(RECIPIENT_ACCT_ID),
        api_.Factory().NymIDFromBase58(RECIPIENT_NYM_ID),
        static_cast<std::int64_t>(INITIAL_PAYMENT_AMOUNT),
        INITIAL_PAYMENT_DELAY,
        static_cast<std::int64_t>(PAYMENT_PLAN_AMOUNT),
        PAYMENT_PLAN_DELAY,
        PAYMENT_PLAN_PERIOD,
        PAYMENT_PLAN_LENGTH,
        static_cast<std::int32_t>(PAYMENT_PLAN_MAX_PAYMENTS)));

    if (!pPlan) {
        LogError()(OT_PRETTY_CLASS())(
            "Failed in OTAPI_Exec::ProposePaymentPlan.")
            .Flush();
        return {};
    }

    return String::Factory(*pPlan)->Get();
}

// TODO RESUME:
// 1. Find out what DiscardCheque is used for.
// 2. Make sure we don't need a "Discard Payment Plan" and "Discard Smart
// Contract" as well.
// 3. Add "EasyProposePlan" to the rest of the API like OTAPI_Basic
// 4. Add 'propose' and 'confirm' (for payment plan) commands to opentxs client.
// 5. Finish opentxs basket commands, so opentxs is COMPLETE.

auto OTAPI_Exec::EasyProposePlan(
    const UnallocatedCString& NOTARY_ID,
    const UnallocatedCString& DATE_RANGE,  // "from,to"  Default 'from' (0 or
                                           // "") == NOW, and default 'to' (0 or
                                           // "") == no expiry / cancel anytime
    const UnallocatedCString& SENDER_ACCT_ID,  // Mandatory parameters. UPDATE:
                                               // Making sender acct optional
                                               // here since it may not be known
                                               // at this point.
    const UnallocatedCString& SENDER_NYM_ID,   // Both sender and recipient must
                                               // sign before submitting.
    const UnallocatedCString& PLAN_CONSIDERATION,  // Like a memo.
    const UnallocatedCString& RECIPIENT_ACCT_ID,   // NOT optional.
    const UnallocatedCString& RECIPIENT_NYM_ID,    // Both sender and recipient
                                                 // must sign before submitting.
    const UnallocatedCString& INITIAL_PAYMENT,  // "amount,delay"  Default
                                                // 'amount' (0
    // or "") == no initial payment. Default
    // 'delay' (0 or nullptr) is seconds from
    // creation date.
    const UnallocatedCString& PAYMENT_PLAN,  // "amount,delay,period" 'amount'
                                             // is a recurring payment. 'delay'
                                             // and 'period' cause 30 days if
                                             // you pass 0 or "".
    const UnallocatedCString& PLAN_EXPIRY    // "length,number" 'length' is
                                             // maximum
    // lifetime in seconds. 'number' is maximum
    // number of payments in seconds. 0 or "" is
    // unlimited.
) const -> UnallocatedCString
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(SENDER_NYM_ID);
    //  OT_VERIFY_ID_STR(SENDER_ACCT_ID); // Optional parameter.
    OT_VERIFY_ID_STR(RECIPIENT_NYM_ID);
    OT_VERIFY_ID_STR(RECIPIENT_ACCT_ID);
    OT_VERIFY_STD_STR(PLAN_CONSIDERATION);

    Time VALID_FROM = convert_stime(0);
    Time VALID_TO = convert_stime(0);
    std::int64_t INITIAL_PAYMENT_AMOUNT = 0;
    std::chrono::seconds INITIAL_PAYMENT_DELAY{0};
    std::int64_t PAYMENT_PLAN_AMOUNT = 0;
    std::chrono::seconds PAYMENT_PLAN_DELAY{0};
    std::chrono::seconds PAYMENT_PLAN_PERIOD{0};
    std::chrono::seconds PAYMENT_PLAN_LENGTH{0};
    std::int32_t PAYMENT_PLAN_MAX_PAYMENTS = 0;
    if (!DATE_RANGE.empty()) {
        NumList theList;
        const auto otstrNumList = String::Factory(DATE_RANGE);
        theList.Add(otstrNumList);
        // VALID_FROM
        if (theList.Count() > 0) {
            std::int64_t lVal = 0;
            if (theList.Peek(lVal)) { VALID_FROM = convert_stime(lVal); }
            theList.Pop();
        }
        // VALID_TO
        if (theList.Count() > 0) {
            std::int64_t lVal = 0;
            if (theList.Peek(lVal)) { VALID_TO = convert_stime(lVal); }
            theList.Pop();
        }
    }
    if (!INITIAL_PAYMENT.empty()) {
        NumList theList;
        const auto otstrNumList = String::Factory(INITIAL_PAYMENT);
        theList.Add(otstrNumList);
        // INITIAL_PAYMENT_AMOUNT
        if (theList.Count() > 0) {
            std::int64_t lVal = 0;
            if (theList.Peek(lVal)) { INITIAL_PAYMENT_AMOUNT = lVal; }
            theList.Pop();
        }
        // INITIAL_PAYMENT_DELAY
        if (theList.Count() > 0) {
            std::int64_t lVal = 0;
            if (theList.Peek(lVal)) {
                INITIAL_PAYMENT_DELAY = std::chrono::seconds{lVal};
            }
            theList.Pop();
        }
    }
    if (!PAYMENT_PLAN.empty()) {
        NumList theList;
        const auto otstrNumList = String::Factory(PAYMENT_PLAN);
        theList.Add(otstrNumList);
        // PAYMENT_PLAN_AMOUNT
        if (theList.Count() > 0) {
            std::int64_t lVal = 0;
            if (theList.Peek(lVal)) { PAYMENT_PLAN_AMOUNT = lVal; }
            theList.Pop();
        }
        // PAYMENT_PLAN_DELAY
        if (theList.Count() > 0) {
            std::int64_t lVal = 0;
            if (theList.Peek(lVal)) {
                PAYMENT_PLAN_DELAY = std::chrono::seconds{lVal};
            }
            theList.Pop();
        }
        // PAYMENT_PLAN_PERIOD
        if (theList.Count() > 0) {
            std::int64_t lVal = 0;
            if (theList.Peek(lVal)) {
                PAYMENT_PLAN_PERIOD = std::chrono::seconds{lVal};
            }
            theList.Pop();
        }
    }
    if (!PLAN_EXPIRY.empty()) {
        NumList theList;
        const auto otstrNumList = String::Factory(PLAN_EXPIRY);
        theList.Add(otstrNumList);
        // PAYMENT_PLAN_LENGTH
        if (theList.Count() > 0) {
            std::int64_t lVal = 0;
            if (theList.Peek(lVal)) {
                PAYMENT_PLAN_LENGTH = std::chrono::seconds{lVal};
            }
            theList.Pop();
        }
        // PAYMENT_PLAN_MAX_PAYMENTS
        if (theList.Count() > 0) {
            std::int64_t lVal = 0;
            if (theList.Peek(lVal)) {
                PAYMENT_PLAN_MAX_PAYMENTS = static_cast<std::int32_t>(lVal);
            }
            theList.Pop();
        }
    }
    return OTAPI_Exec::ProposePaymentPlan(
        NOTARY_ID,
        VALID_FROM,
        VALID_TO,
        SENDER_ACCT_ID,
        SENDER_NYM_ID,
        PLAN_CONSIDERATION,
        RECIPIENT_ACCT_ID,
        RECIPIENT_NYM_ID,
        INITIAL_PAYMENT_AMOUNT,
        INITIAL_PAYMENT_DELAY,
        PAYMENT_PLAN_AMOUNT,
        PAYMENT_PLAN_DELAY,
        PAYMENT_PLAN_PERIOD,
        PAYMENT_PLAN_LENGTH,
        PAYMENT_PLAN_MAX_PAYMENTS);
}

// Called by CUSTOMER.
// "PAYMENT_PLAN" is the output of the above function (ProposePaymentPlan)
// Customer should call OTAPI_Exec::depositPaymentPlan after this.
//
auto OTAPI_Exec::ConfirmPaymentPlan(
    const UnallocatedCString& NOTARY_ID,
    const UnallocatedCString& SENDER_NYM_ID,
    const UnallocatedCString& SENDER_ACCT_ID,
    const UnallocatedCString& RECIPIENT_NYM_ID,
    const UnallocatedCString& PAYMENT_PLAN) const -> UnallocatedCString
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(SENDER_NYM_ID);
    OT_VERIFY_ID_STR(SENDER_ACCT_ID);
    OT_VERIFY_ID_STR(RECIPIENT_NYM_ID);
    OT_VERIFY_STD_STR(PAYMENT_PLAN);

    const auto theNotaryID = api_.Factory().NotaryIDFromBase58(NOTARY_ID);
    const auto theSenderNymID = api_.Factory().NymIDFromBase58(SENDER_NYM_ID);
    const auto theSenderAcctID =
        api_.Factory().IdentifierFromBase58(SENDER_ACCT_ID);
    const auto theRecipientNymID =
        api_.Factory().NymIDFromBase58(RECIPIENT_NYM_ID);

    auto thePlan{api_.Factory().InternalSession().PaymentPlan()};

    OT_ASSERT(false != bool(thePlan));

    const auto strPlan = String::Factory(PAYMENT_PLAN);

    if (!strPlan->Exists() ||
        (false == thePlan->LoadContractFromString(strPlan))) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failure loading payment plan from string.")
            .Flush();
        return {};
    }
    bool bConfirmed = ot_api_.ConfirmPaymentPlan(
        theNotaryID,
        theSenderNymID,
        theSenderAcctID,
        theRecipientNymID,
        *thePlan);
    if (!bConfirmed) {
        LogConsole()(OT_PRETTY_CLASS())(
            "failed in OTAPI_Exec::ConfirmPaymentPlan().")
            .Flush();
        return {};
    }

    auto strOutput =
        String::Factory(*thePlan);  // Extract the payment plan to string form.

    return strOutput->Get();
}

// RETURNS:  the Smart Contract itself. (Or "".)
//
auto OTAPI_Exec::Create_SmartContract(
    const UnallocatedCString& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                              // signing
    // at this postd::int32_t is only to cause a
    // save.)
    const Time& VALID_FROM,  // Default (0 or "") == NOW
    const Time& VALID_TO,    // Default (0 or "") == no expiry / cancel
                             // anytime
    bool SPECIFY_ASSETS,     // This means asset type IDs must be provided for
                             // every named account.
    bool SPECIFY_PARTIES     // This means Nym IDs must be provided for every
                             // party.
) const -> UnallocatedCString
{
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);

    if (convert_stime(0) > VALID_FROM) {
        LogError()(OT_PRETTY_CLASS())("Negative: VALID_FROM passed in!")
            .Flush();
        return {};
    }
    if (convert_stime(0) > VALID_TO) {
        LogError()(OT_PRETTY_CLASS())("Negative: VALID_TO passed in!").Flush();
        return {};
    }
    const auto theSignerNymID = api_.Factory().NymIDFromBase58(SIGNER_NYM_ID);
    auto strOutput = String::Factory();

    const bool bCreated = ot_api_.Create_SmartContract(
        theSignerNymID,
        VALID_FROM,      // Default (0 or "") == NOW
        VALID_TO,        // Default (0 or "") == no expiry / cancel
                         // anytime
        SPECIFY_ASSETS,  // This means asset type IDs must be provided for every
                         // named account.
        SPECIFY_PARTIES,  // This means Nym IDs must be provided for every
                          // party.
        strOutput);
    if (!bCreated || !strOutput->Exists()) { return {}; }
    // Success!
    //
    return strOutput->Get();
}

auto OTAPI_Exec::Smart_ArePartiesSpecified(
    const UnallocatedCString& THE_CONTRACT) const -> bool
{
    OT_VERIFY_STD_STR(THE_CONTRACT);

    const auto strContract = String::Factory(THE_CONTRACT);

    return ot_api_.Smart_ArePartiesSpecified(strContract);
}

auto OTAPI_Exec::Smart_AreAssetTypesSpecified(
    const UnallocatedCString& THE_CONTRACT) const -> bool
{
    OT_VERIFY_STD_STR(THE_CONTRACT);

    const auto strContract = String::Factory(THE_CONTRACT);

    return ot_api_.Smart_AreAssetTypesSpecified(strContract);
}

// RETURNS:  the Smart Contract itself. (Or "".)
//
auto OTAPI_Exec::SmartContract_SetDates(
    const UnallocatedCString& THE_CONTRACT,   // The contract, about to have the
                                              // dates changed on it.
    const UnallocatedCString& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                              // signing at this point is only
                                              // to cause a save.)
    const Time& VALID_FROM,                   // Default (0 or nullptr) == NOW
    const Time& VALID_TO) const
    -> UnallocatedCString  // Default (0 or nullptr) == no
                           // expiry / cancel anytime.
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);

    if (convert_stime(0) > VALID_FROM) {
        LogError()(OT_PRETTY_CLASS())("Negative: VALID_FROM passed in!")
            .Flush();
        return {};
    }
    if (convert_stime(0) > VALID_TO) {
        LogError()(OT_PRETTY_CLASS())("Negative: VALID_TO passed in!").Flush();
        return {};
    }
    const auto strContract = String::Factory(THE_CONTRACT);
    const auto theSignerNymID = api_.Factory().NymIDFromBase58(SIGNER_NYM_ID);
    auto strOutput = String::Factory();

    const bool bAdded = ot_api_.SmartContract_SetDates(
        strContract,     // The contract, about to have the dates changed on it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // point is only to cause a save.)
        VALID_FROM,      // Default (0 or "") == NOW
        VALID_TO,        // Default (0 or "") == no expiry / cancel
                         // anytime
        strOutput);
    if (!bAdded || !strOutput->Exists()) { return {}; }
    // Success!
    //
    return strOutput->Get();
}

//
// todo: Someday add a parameter here BYLAW_LANGUAGE so that people can use
// custom languages in their scripts.  For now I have a default language, so
// I'll just make that the default. (There's only one language right now
// anyway.)
//
// returns: the updated smart contract (or "")
auto OTAPI_Exec::SmartContract_AddBylaw(
    const UnallocatedCString& THE_CONTRACT,   // The contract, about to have the
                                              // bylaw added to it.
    const UnallocatedCString& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                              // signing
                                              // at this point is only to cause
                                              // a save.)
    const UnallocatedCString& BYLAW_NAME) const
    -> UnallocatedCString  // The Bylaw's NAME as
                           // referenced in the
// smart contract. (And the scripts...)
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);
    OT_VERIFY_STD_STR(BYLAW_NAME);

    const auto strContract = String::Factory(THE_CONTRACT),
               strBylawName = String::Factory(BYLAW_NAME);
    const auto theSignerNymID = api_.Factory().NymIDFromBase58(SIGNER_NYM_ID);
    auto strOutput = String::Factory();

    const bool bAdded = ot_api_.SmartContract_AddBylaw(
        strContract,     // The contract, about to have the bylaw added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // postd::int32_t is only to cause a save.)
        strBylawName,  // The Bylaw's NAME as referenced in the smart contract.
                       // (And the scripts...)
        strOutput);
    if (!bAdded || !strOutput->Exists()) { return {}; }
    // Success!
    //
    return strOutput->Get();
}

// returns: the updated smart contract (or "")
auto OTAPI_Exec::SmartContract_RemoveBylaw(
    const UnallocatedCString& THE_CONTRACT,   // The contract, about to have the
                                              // bylaw removed from it.
    const UnallocatedCString& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                              // signing
    // at this postd::int32_t is only to cause a
    // save.)
    const UnallocatedCString& BYLAW_NAME) const
    -> UnallocatedCString  // The Bylaw's NAME as
                           // referenced in the
// smart contract. (And the scripts...)
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);
    OT_VERIFY_STD_STR(BYLAW_NAME);

    const auto strContract = String::Factory(THE_CONTRACT),
               strBylawName = String::Factory(BYLAW_NAME);
    const auto theSignerNymID = api_.Factory().NymIDFromBase58(SIGNER_NYM_ID);
    auto strOutput = String::Factory();

    const bool bAdded = ot_api_.SmartContract_RemoveBylaw(
        strContract,     // The contract, about to have the bylaw added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // postd::int32_t is only to cause a save.)
        strBylawName,  // The Bylaw's NAME as referenced in the smart contract.
                       // (And the scripts...)
        strOutput);
    if (!bAdded || !strOutput->Exists()) { return {}; }
    // Success!
    //
    return strOutput->Get();
}

// returns: the updated smart contract (or "")
auto OTAPI_Exec::SmartContract_AddClause(
    const UnallocatedCString& THE_CONTRACT,   // The contract, about to have the
                                              // clause added to it.
    const UnallocatedCString& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                              // signing
    // at this postd::int32_t is only to cause a
    // save.)
    const UnallocatedCString& BYLAW_NAME,  // Should already be on the contract.
                                           // (This way we can find it.)
    const UnallocatedCString& CLAUSE_NAME,  // The Clause's name as referenced
                                            // in the smart contract. (And the
                                            // scripts...)
    const UnallocatedCString& SOURCE_CODE) const
    -> UnallocatedCString  // The actual source code for the
                           // clause.
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);
    OT_VERIFY_STD_STR(BYLAW_NAME);
    OT_VERIFY_STD_STR(CLAUSE_NAME);

    const auto strContract = String::Factory(THE_CONTRACT),
               strBylawName = String::Factory(BYLAW_NAME),
               strClauseName = String::Factory(CLAUSE_NAME),
               strSourceCode = String::Factory(SOURCE_CODE);
    const auto theSignerNymID = api_.Factory().NymIDFromBase58(SIGNER_NYM_ID);
    auto strOutput = String::Factory();

    const bool bAdded = ot_api_.SmartContract_AddClause(
        strContract,     // The contract, about to have the clause added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // postd::int32_t is only to cause a save.)
        strBylawName,    // Should already be on the contract. (This way we can
                         // find it.)
        strClauseName,   // The Clause's name as referenced in the smart
                         // contract.
                         // (And the scripts...)
        strSourceCode,   // The actual source code for the clause.
        strOutput);
    if (!bAdded || !strOutput->Exists()) { return {}; }
    // Success!
    //
    return strOutput->Get();
}

// returns: the updated smart contract (or "")
auto OTAPI_Exec::SmartContract_UpdateClause(
    const UnallocatedCString& THE_CONTRACT,   // The contract, about to have the
                                              // clause updated on it.
    const UnallocatedCString& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                              // signing
    // at this postd::int32_t is only to cause a
    // save.)
    const UnallocatedCString& BYLAW_NAME,  // Should already be on the contract.
                                           // (This way we can find it.)
    const UnallocatedCString& CLAUSE_NAME,  // The Clause's name as referenced
                                            // in the smart contract. (And the
                                            // scripts...)
    const UnallocatedCString& SOURCE_CODE) const
    -> UnallocatedCString  // The actual source code for the
                           // clause.
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);
    OT_VERIFY_STD_STR(BYLAW_NAME);
    OT_VERIFY_STD_STR(CLAUSE_NAME);
    //  OT_VERIFY_STD_STR(SOURCE_CODE);

    const auto strContract = String::Factory(THE_CONTRACT),
               strBylawName = String::Factory(BYLAW_NAME),
               strClauseName = String::Factory(CLAUSE_NAME),
               strSourceCode = String::Factory(SOURCE_CODE);
    const auto theSignerNymID = api_.Factory().NymIDFromBase58(SIGNER_NYM_ID);
    auto strOutput = String::Factory();

    const bool bAdded = ot_api_.SmartContract_UpdateClause(
        strContract,     // The contract, about to have the clause added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // postd::int32_t is only to cause a save.)
        strBylawName,    // Should already be on the contract. (This way we can
                         // find it.)
        strClauseName,   // The Clause's name as referenced in the smart
                         // contract.
                         // (And the scripts...)
        strSourceCode,   // The actual source code for the clause.
        strOutput);
    if (!bAdded || !strOutput->Exists()) { return {}; }
    // Success!
    //
    return strOutput->Get();
}

// returns: the updated smart contract (or "")
auto OTAPI_Exec::SmartContract_RemoveClause(
    const UnallocatedCString& THE_CONTRACT,   // The contract, about to have the
                                              // clause added to it.
    const UnallocatedCString& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                              // signing
    // at this postd::int32_t is only to cause a
    // save.)
    const UnallocatedCString& BYLAW_NAME,  // Should already be on the contract.
                                           // (This way we can find it.)
    const UnallocatedCString& CLAUSE_NAME) const -> UnallocatedCString
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);
    OT_VERIFY_STD_STR(BYLAW_NAME);
    OT_VERIFY_STD_STR(CLAUSE_NAME);

    const auto strContract = String::Factory(THE_CONTRACT),
               strBylawName = String::Factory(BYLAW_NAME),
               strClauseName = String::Factory(CLAUSE_NAME);
    const auto theSignerNymID = api_.Factory().NymIDFromBase58(SIGNER_NYM_ID);
    auto strOutput = String::Factory();

    const bool bAdded = ot_api_.SmartContract_RemoveClause(
        strContract,     // The contract, about to have the clause added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // postd::int32_t is only to cause a save.)
        strBylawName,    // Should already be on the contract. (This way we can
                         // find it.)
        strClauseName,   // The Clause's name as referenced in the smart
                         // contract.
                         // (And the scripts...)
        strOutput);
    if (!bAdded || !strOutput->Exists()) { return {}; }
    // Success!
    //
    return strOutput->Get();
}

// returns: the updated smart contract (or "")
auto OTAPI_Exec::SmartContract_AddVariable(
    const UnallocatedCString& THE_CONTRACT,   // The contract, about to have the
                                              // variable
                                              // added to it.
    const UnallocatedCString& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                              // signing
    // at this postd::int32_t is only to cause a
    // save.)
    const UnallocatedCString& BYLAW_NAME,  // Should already be on the contract.
                                           // (This way we can find it.)
    const UnallocatedCString& VAR_NAME,  // The Variable's name as referenced in
                                         // the smart contract. (And the
                                         // scripts...)
    const UnallocatedCString& VAR_ACCESS,  // "constant", "persistent", or
                                           // "important".
    const UnallocatedCString& VAR_TYPE,  // "string", "std::int64_t", or "bool"
    const UnallocatedCString& VAR_VALUE) const
    -> UnallocatedCString  // Contains a string. If
                           // type is std::int64_t,
// StringToLong() will be used to convert
// value to a std::int64_t. If type is bool, the
// strings "true" or "false" are expected here
// in order to convert to a bool.
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);
    OT_VERIFY_STD_STR(BYLAW_NAME);
    OT_VERIFY_STD_STR(VAR_NAME);
    OT_VERIFY_STD_STR(VAR_ACCESS);
    OT_VERIFY_STD_STR(VAR_TYPE);

    const auto strContract = String::Factory(THE_CONTRACT),
               strBylawName = String::Factory(BYLAW_NAME),
               strVarName = String::Factory(VAR_NAME),
               strVarAccess = String::Factory(VAR_ACCESS),
               strVarType = String::Factory(VAR_TYPE),
               strVarValue = String::Factory(VAR_VALUE);
    const auto theSignerNymID = api_.Factory().NymIDFromBase58(SIGNER_NYM_ID);
    auto strOutput = String::Factory();

    const bool bAdded = ot_api_.SmartContract_AddVariable(
        strContract,     // The contract, about to have the clause added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // postd::int32_t is only to cause a save.)
        strBylawName,    // Should already be on the contract. (This way we can
                         // find it.)
        strVarName,  // The Variable's name as referenced in the smart contract.
                     // (And the scripts...)
        strVarAccess,  // "constant", "persistent", or "important".
        strVarType,    // "string", "std::int64_t", or "bool"
        strVarValue,   // Contains a string. If type is std::int64_t,
                       // StringToLong()
        // will be used to convert value to a std::int64_t. If type is
        // bool, the strings "true" or "false" are expected here in
        // order to convert to a bool.
        strOutput);
    if (!bAdded || !strOutput->Exists()) { return {}; }
    // Success!
    //
    return strOutput->Get();
}

// returns: the updated smart contract (or "")
auto OTAPI_Exec::SmartContract_RemoveVariable(
    const UnallocatedCString& THE_CONTRACT,   // The contract, about to have the
                                              // variable
                                              // added to it.
    const UnallocatedCString& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                              // signing
    // at this postd::int32_t is only to cause a
    // save.)
    const UnallocatedCString& BYLAW_NAME,  // Should already be on the contract.
                                           // (This way we can find it.)
    const UnallocatedCString& VAR_NAME  // The Variable's name as referenced in
                                        // the smart contract. (And the
                                        // scripts...)
) const -> UnallocatedCString
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);
    OT_VERIFY_STD_STR(BYLAW_NAME);
    OT_VERIFY_STD_STR(VAR_NAME);

    const auto strContract = String::Factory(THE_CONTRACT),
               strBylawName = String::Factory(BYLAW_NAME),
               strVarName = String::Factory(VAR_NAME);
    const auto theSignerNymID = api_.Factory().NymIDFromBase58(SIGNER_NYM_ID);
    auto strOutput = String::Factory();

    const bool bAdded = ot_api_.SmartContract_RemoveVariable(
        strContract,     // The contract, about to have the clause added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // postd::int32_t is only to cause a save.)
        strBylawName,    // Should already be on the contract. (This way we can
                         // find it.)
        strVarName,  // The Variable's name as referenced in the smart contract.
                     // (And the scripts...)
        strOutput);
    if (!bAdded || !strOutput->Exists()) { return {}; }
    // Success!
    //
    return strOutput->Get();
}

// returns: the updated smart contract (or "")
auto OTAPI_Exec::SmartContract_AddCallback(
    const UnallocatedCString& THE_CONTRACT,   // The contract, about to have the
                                              // callback
                                              // added to it.
    const UnallocatedCString& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                              // signing
    // at this postd::int32_t is only to cause a
    // save.)
    const UnallocatedCString& BYLAW_NAME,  // Should already be on the contract.
                                           // (This way we can find it.)
    const UnallocatedCString& CALLBACK_NAME,  // The Callback's name as
                                              // referenced in the smart
                                              // contract. (And the scripts...)
    const UnallocatedCString& CLAUSE_NAME) const
    -> UnallocatedCString  // The actual clause that will be
                           // triggered
                           // by the callback. (Must exist.)
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);
    OT_VERIFY_STD_STR(BYLAW_NAME);
    OT_VERIFY_STD_STR(CALLBACK_NAME);
    OT_VERIFY_STD_STR(CLAUSE_NAME);

    const auto strContract = String::Factory(THE_CONTRACT),
               strBylawName = String::Factory(BYLAW_NAME),
               strCallbackName = String::Factory(CALLBACK_NAME),
               strClauseName = String::Factory(CLAUSE_NAME);
    const auto theSignerNymID = api_.Factory().NymIDFromBase58(SIGNER_NYM_ID);
    auto strOutput = String::Factory();

    const bool bAdded = ot_api_.SmartContract_AddCallback(
        strContract,      // The contract, about to have the clause added to it.
        theSignerNymID,   // Use any Nym you wish here. (The signing at this
                          // postd::int32_t is only to cause a save.)
        strBylawName,     // Should already be on the contract. (This way we can
                          // find it.)
        strCallbackName,  // The Callback's name as referenced in the smart
                          // contract. (And the scripts...)
        strClauseName,    // The actual clause that will be triggered by the
                          // callback. (Must exist.)
        strOutput);
    if (!bAdded || !strOutput->Exists()) { return {}; }
    // Success!
    //
    return strOutput->Get();
}

// returns: the updated smart contract (or "")
auto OTAPI_Exec::SmartContract_RemoveCallback(
    const UnallocatedCString& THE_CONTRACT,   // The contract, about to have the
                                              // callback
                                              // removed from it.
    const UnallocatedCString& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                              // signing
    // at this postd::int32_t is only to cause a
    // save.)
    const UnallocatedCString& BYLAW_NAME,  // Should already be on the contract.
                                           // (This way we can find it.)
    const UnallocatedCString& CALLBACK_NAME  // The Callback's name as
                                             // referenced in the smart
                                             // contract. (And the scripts...)
) const -> UnallocatedCString
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);
    OT_VERIFY_STD_STR(BYLAW_NAME);
    OT_VERIFY_STD_STR(CALLBACK_NAME);

    const auto strContract = String::Factory(THE_CONTRACT),
               strBylawName = String::Factory(BYLAW_NAME),
               strCallbackName = String::Factory(CALLBACK_NAME);
    const auto theSignerNymID = api_.Factory().NymIDFromBase58(SIGNER_NYM_ID);
    auto strOutput = String::Factory();

    const bool bAdded = ot_api_.SmartContract_RemoveCallback(
        strContract,      // The contract, about to have the clause added to it.
        theSignerNymID,   // Use any Nym you wish here. (The signing at this
                          // postd::int32_t is only to cause a save.)
        strBylawName,     // Should already be on the contract. (This way we can
                          // find it.)
        strCallbackName,  // The Callback's name as referenced in the smart
                          // contract. (And the scripts...)
        strOutput);
    if (!bAdded || !strOutput->Exists()) { return {}; }
    // Success!
    //
    return strOutput->Get();
}

// returns: the updated smart contract (or "")
auto OTAPI_Exec::SmartContract_AddHook(
    const UnallocatedCString& THE_CONTRACT,   // The contract, about to have the
                                              // hook added to it.
    const UnallocatedCString& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                              // signing
    // at this postd::int32_t is only to cause a
    // save.)
    const UnallocatedCString& BYLAW_NAME,  // Should already be on the contract.
                                           // (This way we can find it.)
    const UnallocatedCString& HOOK_NAME,   // The Hook's name as referenced in
                                           // the smart contract. (And the
                                           // scripts...)
    const UnallocatedCString& CLAUSE_NAME) const
    -> UnallocatedCString  // The actual clause that will be
                           // triggered
// by the hook. (You can call this multiple
// times, and have multiple clauses trigger
// on the same hook.)
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);
    OT_VERIFY_STD_STR(BYLAW_NAME);
    OT_VERIFY_STD_STR(HOOK_NAME);
    OT_VERIFY_STD_STR(CLAUSE_NAME);

    const auto strContract = String::Factory(THE_CONTRACT),
               strBylawName = String::Factory(BYLAW_NAME),
               strHookName = String::Factory(HOOK_NAME),
               strClauseName = String::Factory(CLAUSE_NAME);
    const auto theSignerNymID = api_.Factory().NymIDFromBase58(SIGNER_NYM_ID);
    auto strOutput = String::Factory();

    const bool bAdded = ot_api_.SmartContract_AddHook(
        strContract,     // The contract, about to have the clause added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // postd::int32_t is only to cause a save.)
        strBylawName,    // Should already be on the contract. (This way we can
                         // find it.)
        strHookName,     // The Hook's name as referenced in the smart contract.
                         // (And the scripts...)
        strClauseName,  // The actual clause that will be triggered by the hook.
                        // (You can call this multiple times, and have multiple
                        // clauses trigger on the same hook.)
        strOutput);
    if (!bAdded || !strOutput->Exists()) { return {}; }
    // Success!
    //
    return strOutput->Get();
}

// returns: the updated smart contract (or "")
auto OTAPI_Exec::SmartContract_RemoveHook(
    const UnallocatedCString& THE_CONTRACT,   // The contract, about to have the
                                              // hook removed from it.
    const UnallocatedCString& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                              // signing
    // at this postd::int32_t is only to cause a
    // save.)
    const UnallocatedCString& BYLAW_NAME,  // Should already be on the contract.
                                           // (This way we can find it.)
    const UnallocatedCString& HOOK_NAME,   // The Hook's name as referenced in
                                           // the smart contract. (And the
                                           // scripts...)
    const UnallocatedCString& CLAUSE_NAME) const
    -> UnallocatedCString  // The actual clause that will be
                           // triggered
// by the hook. (You can call this multiple
// times, and have multiple clauses trigger
// on the same hook.)
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);
    OT_VERIFY_STD_STR(BYLAW_NAME);
    OT_VERIFY_STD_STR(HOOK_NAME);
    OT_VERIFY_STD_STR(CLAUSE_NAME);

    const auto strContract = String::Factory(THE_CONTRACT),
               strBylawName = String::Factory(BYLAW_NAME),
               strHookName = String::Factory(HOOK_NAME),
               strClauseName = String::Factory(CLAUSE_NAME);
    const auto theSignerNymID = api_.Factory().NymIDFromBase58(SIGNER_NYM_ID);
    auto strOutput = String::Factory();

    const bool bAdded = ot_api_.SmartContract_RemoveHook(
        strContract,     // The contract, about to have the clause added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // postd::int32_t is only to cause a save.)
        strBylawName,    // Should already be on the contract. (This way we can
                         // find it.)
        strHookName,     // The Hook's name as referenced in the smart contract.
                         // (And the scripts...)
        strClauseName,  // The actual clause that will be triggered by the hook.
                        // (You can call this multiple times, and have multiple
                        // clauses trigger on the same hook.)
        strOutput);
    if (!bAdded || !strOutput->Exists()) { return {}; }
    // Success!
    //
    return strOutput->Get();
}

// RETURNS: Updated version of THE_CONTRACT. (Or "".)
auto OTAPI_Exec::SmartContract_AddParty(
    const UnallocatedCString& THE_CONTRACT,   // The contract, about to have the
                                              // party added to it.
    const UnallocatedCString& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                              // signing
    // at this postd::int32_t is only to cause a
    // save.)
    const UnallocatedCString& PARTY_NYM_ID,  // Required when the smart contract
                                             // is configured to require parties
                                             // to be specified. Otherwise must
                                             // be empty.
    const UnallocatedCString& PARTY_NAME,  // The Party's NAME as referenced in
                                           // the smart contract. (And the
                                           // scripts...)
    const UnallocatedCString& AGENT_NAME) const
    -> UnallocatedCString  // An AGENT will be added by default
                           // for this
                           // party. Need Agent NAME.
// (FYI, that is basically the only option, until I code Entities and Roles.
// Until then, a party can ONLY be
// a Nym, with himself as the agent representing that same party. Nym ID is
// supplied on ConfirmParty() below.)
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);
    OT_VERIFY_STD_STR(PARTY_NAME);
    OT_VERIFY_STD_STR(AGENT_NAME);

    const auto strContract = String::Factory(THE_CONTRACT),
               strPartyName = String::Factory(PARTY_NAME),
               strAgentName = String::Factory(AGENT_NAME),
               strPartyNymID = String::Factory(PARTY_NYM_ID);
    const auto theSignerNymID = api_.Factory().NymIDFromBase58(SIGNER_NYM_ID);
    auto strOutput = String::Factory();

    const bool bAdded = ot_api_.SmartContract_AddParty(
        strContract,     // The contract, about to have the bylaw added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // postd::int32_t is only to cause a save.)
        strPartyNymID,
        strPartyName,  // The Party's NAME as referenced in the smart contract.
                       // (And the scripts...)
        strAgentName,  // An AGENT will be added by default for this party. Need
                       // Agent NAME.
        strOutput);
    if (!bAdded || !strOutput->Exists()) { return {}; }
    // Success!
    //
    return strOutput->Get();
}

// RETURNS: Updated version of THE_CONTRACT. (Or "".)
auto OTAPI_Exec::SmartContract_RemoveParty(
    const UnallocatedCString& THE_CONTRACT,   // The contract, about to have the
                                              // party added to it.
    const UnallocatedCString& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                              // signing
    // at this postd::int32_t is only to cause a
    // save.)
    const UnallocatedCString& PARTY_NAME  // The Party's NAME as referenced in
                                          // the smart contract. (And the
                                          // scripts...)
) const -> UnallocatedCString
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);
    OT_VERIFY_STD_STR(PARTY_NAME);

    const auto strContract = String::Factory(THE_CONTRACT),
               strPartyName = String::Factory(PARTY_NAME);
    const auto theSignerNymID = api_.Factory().NymIDFromBase58(SIGNER_NYM_ID);
    auto strOutput = String::Factory();

    const bool bAdded = ot_api_.SmartContract_RemoveParty(
        strContract,     // The contract, about to have the bylaw added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // postd::int32_t is only to cause a save.)
        strPartyName,  // The Party's NAME as referenced in the smart contract.
                       // (And the scripts...)
        strOutput);
    if (!bAdded || !strOutput->Exists()) { return {}; }
    // Success!
    //
    return strOutput->Get();
}

// Used when creating a theoretical smart contract (that could be used over and
// over again with different parties.)
//
// returns: the updated smart contract (or "")
auto OTAPI_Exec::SmartContract_AddAccount(
    const UnallocatedCString& THE_CONTRACT,   // The contract, about to have the
                                              // account added to it.
    const UnallocatedCString& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                              // signing
    // at this postd::int32_t is only to cause a
    // save.)
    const UnallocatedCString& PARTY_NAME,  // The Party's NAME as referenced in
                                           // the smart contract. (And the
                                           // scripts...)
    const UnallocatedCString& ACCT_NAME,  // The Account's name as referenced in
                                          // the smart contract
    const UnallocatedCString& INSTRUMENT_DEFINITION_ID) const
    -> UnallocatedCString  // Instrument Definition
// ID for the Account. (Optional.)
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);
    OT_VERIFY_STD_STR(PARTY_NAME);
    OT_VERIFY_STD_STR(ACCT_NAME);

    const auto strContract = String::Factory(THE_CONTRACT),
               strPartyName = String::Factory(PARTY_NAME),
               strAcctName = String::Factory(ACCT_NAME),
               strInstrumentDefinitionID =
                   String::Factory(INSTRUMENT_DEFINITION_ID);
    const auto theSignerNymID = api_.Factory().NymIDFromBase58(SIGNER_NYM_ID);
    auto strOutput = String::Factory();

    const bool bAdded = ot_api_.SmartContract_AddAccount(
        strContract,     // The contract, about to have the clause added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // postd::int32_t is only to cause a save.)
        strPartyName,  // The Party's NAME as referenced in the smart contract.
                       // (And the scripts...)
        strAcctName,   // The Account's name as referenced in the smart contract
        strInstrumentDefinitionID,  // Instrument Definition ID for the Account.
        strOutput);
    if (!bAdded || !strOutput->Exists()) { return {}; }
    // Success!
    //
    return strOutput->Get();
}

// returns: the updated smart contract (or "")
auto OTAPI_Exec::SmartContract_RemoveAccount(
    const UnallocatedCString& THE_CONTRACT,   // The contract, about to have the
                                              // account removed from it.
    const UnallocatedCString& SIGNER_NYM_ID,  // Use any Nym you wish here. (The
                                              // signing
    // at this postd::int32_t is only to cause a
    // save.)
    const UnallocatedCString& PARTY_NAME,  // The Party's NAME as referenced in
                                           // the smart contract. (And the
                                           // scripts...)
    const UnallocatedCString& ACCT_NAME  // The Account's name as referenced in
                                         // the smart contract
) const -> UnallocatedCString
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);
    OT_VERIFY_STD_STR(PARTY_NAME);
    OT_VERIFY_STD_STR(ACCT_NAME);

    const auto strContract = String::Factory(THE_CONTRACT),
               strPartyName = String::Factory(PARTY_NAME),
               strAcctName = String::Factory(ACCT_NAME);
    const auto theSignerNymID = api_.Factory().NymIDFromBase58(SIGNER_NYM_ID);
    auto strOutput = String::Factory();

    const bool bAdded = ot_api_.SmartContract_RemoveAccount(
        strContract,     // The contract, about to have the clause added to it.
        theSignerNymID,  // Use any Nym you wish here. (The signing at this
                         // postd::int32_t is only to cause a save.)
        strPartyName,  // The Party's NAME as referenced in the smart contract.
                       // (And the scripts...)
        strAcctName,   // The Account's name as referenced in the smart contract
        strOutput);
    if (!bAdded || !strOutput->Exists()) { return {}; }
    // Success!
    //
    return strOutput->Get();
}

// This function returns the count of how many trans#s a Nym needs in order to
// confirm as
// a specific agent for a contract. (An opening number is needed for every party
// of which
// agent is the authorizing agent, plus a closing number for every acct of which
// agent is the
// authorized agent.)
//
// Otherwise a Nym might try to confirm a smart contract and be unable to, since
// he doesn't
// have enough transaction numbers, yet he would have no way of finding out HOW
// MANY HE *DOES*
// NEED. This function allows him to find out, before confirmation, so he can
// first grab however
// many transaction#s he will need in order to confirm this smart contract.
//
auto OTAPI_Exec::SmartContract_CountNumsNeeded(
    const UnallocatedCString& THE_CONTRACT,  // The smart contract, about to be
                                             // queried by this function.
    const UnallocatedCString& AGENT_NAME) const -> std::int32_t
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(AGENT_NAME);

    const auto strContract = String::Factory(THE_CONTRACT),
               strAgentName = String::Factory(AGENT_NAME);
    return ot_api_.SmartContract_CountNumsNeeded(strContract, strAgentName);
}

// Used when taking a theoretical smart contract, and setting it up to use
// specific Nyms and accounts.
// This function sets the ACCT ID and the AGENT NAME for the acct specified by
// party name and acct name.
// Returns the updated smart contract (or "".)
//
auto OTAPI_Exec::SmartContract_ConfirmAccount(
    const UnallocatedCString& THE_CONTRACT,
    const UnallocatedCString& SIGNER_NYM_ID,
    const UnallocatedCString& PARTY_NAME,  // Should already be on the contract.
    const UnallocatedCString& ACCT_NAME,   // Should already be on the contract.
    const UnallocatedCString& AGENT_NAME,  // The agent name for this asset
                                           // account.
    const UnallocatedCString& ACCT_ID) const
    -> UnallocatedCString  // AcctID for the asset account. (For
                           // acct_name).
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_ID_STR(SIGNER_NYM_ID);
    OT_VERIFY_STD_STR(PARTY_NAME);
    OT_VERIFY_STD_STR(ACCT_NAME);
    OT_VERIFY_STD_STR(AGENT_NAME);
    OT_VERIFY_ID_STR(ACCT_ID);

    const auto strContract = String::Factory(THE_CONTRACT),
               strPartyName = String::Factory(PARTY_NAME);
    const auto strAccountID = String::Factory(ACCT_ID),
               strAcctName = String::Factory(ACCT_NAME),
               strAgentName = String::Factory(AGENT_NAME);
    const auto theSignerNymID = api_.Factory().NymIDFromBase58(SIGNER_NYM_ID);
    const auto theAcctID =
        api_.Factory().IdentifierFromBase58(strAccountID->Bytes());
    auto strOutput = String::Factory();

    const bool bConfirmed = ot_api_.SmartContract_ConfirmAccount(
        strContract,
        theSignerNymID,
        strPartyName,
        strAcctName,
        strAgentName,
        strAccountID,
        strOutput);
    if (!bConfirmed || !strOutput->Exists()) { return {}; }
    // Success!
    return strOutput->Get();
}

// Called by each Party. Pass in the smart contract obtained in the above call.
// Call OTAPI_Exec::SmartContract_ConfirmAccount() first, as much as you need
// to, THEN call this (for final signing.)
// This is the last call you make before either passing it on to another party
// to confirm, or calling OTAPI_Exec::activateSmartContract().
// Returns the updated smart contract (or "".)
auto OTAPI_Exec::SmartContract_ConfirmParty(
    const UnallocatedCString& THE_CONTRACT,  // The smart contract, about to be
                                             // changed by this function.
    const UnallocatedCString& PARTY_NAME,  // Should already be on the contract.
                                           // This way we can find it.
    const UnallocatedCString& NYM_ID,
    const UnallocatedCString& NOTARY_ID) const -> UnallocatedCString
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(PARTY_NAME);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(NOTARY_ID);

    const auto theNymID = api_.Factory().NymIDFromBase58(NYM_ID);
    const auto strContract = String::Factory(THE_CONTRACT),
               strPartyName = String::Factory(PARTY_NAME);
    auto strOutput = String::Factory();

    const bool bConfirmed = ot_api_.SmartContract_ConfirmParty(
        strContract,   // The smart contract, about to be changed by this
                       // function.
        strPartyName,  // Should already be on the contract. This way we can
                       // find it.
        theNymID,      // Nym ID for the party, the actual owner,
        api_.Factory().NotaryIDFromBase58(NOTARY_ID),
        strOutput);
    if (!bConfirmed || !strOutput->Exists()) { return {}; }
    // Success!
    return strOutput->Get();
}

auto OTAPI_Exec::Smart_AreAllPartiesConfirmed(
    const UnallocatedCString& THE_CONTRACT) const -> bool  // true or false?
{
    OT_VERIFY_STD_STR(THE_CONTRACT);

    auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().InternalSession().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed trying to load smart contract from string : ")(
            strContract.get())(".")
            .Flush();
    } else {
        const bool bConfirmed =
            pScriptable->AllPartiesHaveSupposedlyConfirmed();
        const bool bVerified =
            pScriptable->VerifyThisAgainstAllPartiesSignedCopies();
        if (!bConfirmed) {
            LogDetail()(OT_PRETTY_CLASS())(
                "Smart contract loaded up, but all ")(
                "parties are NOT confirmed.")
                .Flush();
            return false;
        } else if (bVerified) {
            // Todo security: We have confirmed that all parties have provided
            // signed copies, but we have
            // not actually verified the signatures themselves. (Though we HAVE
            // verified that their copies of
            // the contract match the main copy.)
            // The server DOES verify this before activation, but the client
            // should as well, just in case. Todo.
            // (I'd want MY client to do it...)
            //
            return true;
        }
        LogConsole()(OT_PRETTY_CLASS())(
            "Suspicious: Smart contract loaded up, and is supposedly "
            "confirmed by all parties, but failed to verify: ")(
            strContract.get())(".")
            .Flush();
    }
    return false;
}

auto OTAPI_Exec::Smart_IsPartyConfirmed(
    const UnallocatedCString& THE_CONTRACT,
    const UnallocatedCString& PARTY_NAME) const -> bool  // true
                                                         // or
                                                         // false?
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(PARTY_NAME);

    auto reason = api_.Factory().PasswordPrompt(__func__);
    const auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().InternalSession().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed trying to load smart contract from string : ")(
            strContract.get())(".")
            .Flush();
        return false;
    }

    OTParty* pParty = pScriptable->GetParty(PARTY_NAME);
    if (nullptr == pParty) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Smart contract loaded up, but failed to find a party "
            "with the name: ")(PARTY_NAME)(".")
            .Flush();
        return false;
    }

    // We found the party...
    //...is he confirmed?
    //
    if (!pParty->GetMySignedCopy().Exists()) {
        LogDetail()(OT_PRETTY_CLASS())("Smart contract loaded up, and party ")(
            PARTY_NAME)(" was found, but didn't find a signed copy of the ")(
            "agreement for that party.")
            .Flush();
        return false;
    }

    // FYI, this block comes from
    // OTScriptable::VerifyThisAgainstAllPartiesSignedCopies.
    auto pPartySignedCopy(
        api_.Factory().InternalSession().Scriptable(pParty->GetMySignedCopy()));

    if (nullptr == pPartySignedCopy) {
        const UnallocatedCString current_party_name(pParty->GetPartyName());
        LogError()(OT_PRETTY_CLASS())("Error loading party's (")(
            current_party_name)(") signed copy of agreement. Has it been "
                                "executed?")
            .Flush();
        return false;
    }

    if (!pScriptable->Compare(*pPartySignedCopy)) {
        const UnallocatedCString current_party_name(pParty->GetPartyName());
        LogError()(OT_PRETTY_CLASS())("Suspicious: Party's (")(
            current_party_name)(") signed copy of agreement doesn't match the "
                                "contract.")
            .Flush();
        return false;
    }

    // TODO Security: This function doesn't actually verify
    // the party's SIGNATURE on his signed
    // version, only that it exists and it matches the main
    // contract.
    //
    // The actual signatures are all verified by the server
    // before activation, but I'd still like the client
    // to have the option to do so as well. I can imagine
    // getting someone's signature on something (without
    // signing it yourself) and then just retaining the
    // OPTION to sign it later -- but he might not have
    // actually signed it if he knew that you hadn't either.
    // He'd want his client to tell him, if this were
    // the case. Todo.

    return true;
}

auto OTAPI_Exec::Smart_GetPartyCount(
    const UnallocatedCString& THE_CONTRACT) const -> std::int32_t
{
    OT_VERIFY_STD_STR(THE_CONTRACT);

    auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().InternalSession().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed trying to load smart contract from string: ")(
            strContract.get())(".")
            .Flush();
        return OT_ERROR;  // Error condition.
    }

    return pScriptable->GetPartyCount();
}

auto OTAPI_Exec::Smart_GetBylawCount(
    const UnallocatedCString& THE_CONTRACT) const -> std::int32_t
{
    OT_VERIFY_STD_STR(THE_CONTRACT);

    auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().InternalSession().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed trying to load smart contract from string : ")(
            strContract.get())(".")
            .Flush();
        return OT_ERROR;  // Error condition.
    }

    return pScriptable->GetBylawCount();
}

/// returns the name of the party.
auto OTAPI_Exec::Smart_GetPartyByIndex(
    const UnallocatedCString& THE_CONTRACT,
    const std::int32_t& nIndex) const -> UnallocatedCString
{
    OT_VERIFY_STD_STR(THE_CONTRACT);

    const auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().InternalSession().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed trying to load smart contract from string: ")(
            strContract.get())(".")
            .Flush();
        return {};
    }

    const std::int32_t nTempIndex = nIndex;
    OTParty* pParty = pScriptable->GetPartyByIndex(
        nTempIndex);  // has range-checking built-in.
    if (nullptr == pParty) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Smart contract loaded up, but failed to retrieve the "
            "party using index: ")(nTempIndex)(".")
            .Flush();
        return {};
    }

    return pParty->GetPartyName();
}

/// returns the name of the bylaw.
auto OTAPI_Exec::Smart_GetBylawByIndex(
    const UnallocatedCString& THE_CONTRACT,
    const std::int32_t& nIndex) const -> UnallocatedCString
{
    OT_VERIFY_STD_STR(THE_CONTRACT);

    const auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().InternalSession().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed trying to load smart contract from string : ")(
            strContract.get())(".")
            .Flush();
        return {};
    }

    const std::int32_t nTempIndex = nIndex;
    OTBylaw* pBylaw = pScriptable->GetBylawByIndex(
        nTempIndex);  // has range-checking built-in.
    if (nullptr == pBylaw) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Smart contract loaded up, but failed to retrieve the "
            "bylaw using index: ")(nTempIndex)(".")
            .Flush();
        return {};
    }

    // We found the bylaw...
    return pBylaw->GetName().Get();
}

auto OTAPI_Exec::Bylaw_GetLanguage(
    const UnallocatedCString& THE_CONTRACT,
    const UnallocatedCString& BYLAW_NAME) const -> UnallocatedCString
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(BYLAW_NAME);

    auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().InternalSession().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed trying to load smart contract from string : ")(
            strContract.get())(".")
            .Flush();
        return {};
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Smart contract loaded up, but failed to find a bylaw "
            "with the name: ")(BYLAW_NAME)(".")
            .Flush();
        return {};
    }
    // We found the bylaw...
    if (nullptr == pBylaw->GetLanguage()) { return "error_no_language"; }
    return pBylaw->GetLanguage();
}

auto OTAPI_Exec::Bylaw_GetClauseCount(
    const UnallocatedCString& THE_CONTRACT,
    const UnallocatedCString& BYLAW_NAME) const -> std::int32_t
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(BYLAW_NAME);

    auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().InternalSession().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed trying to load smart contract from string : ")(
            strContract.get())(".")
            .Flush();
        return OT_ERROR;
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Smart contract loaded up, but failed to find a bylaw "
            "with the name: ")(BYLAW_NAME)(".")
            .Flush();
        return OT_ERROR;
    }

    return pBylaw->GetClauseCount();
}

auto OTAPI_Exec::Bylaw_GetVariableCount(
    const UnallocatedCString& THE_CONTRACT,
    const UnallocatedCString& BYLAW_NAME) const -> std::int32_t
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(BYLAW_NAME);

    auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().InternalSession().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed trying to load smart contract from string : ")(
            strContract.get())(".")
            .Flush();
        return OT_ERROR;
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Smart contract loaded up, but failed to find a bylaw "
            "with the name: ")(BYLAW_NAME)(".")
            .Flush();
        return OT_ERROR;
    }

    return pBylaw->GetVariableCount();
}

auto OTAPI_Exec::Bylaw_GetHookCount(
    const UnallocatedCString& THE_CONTRACT,
    const UnallocatedCString& BYLAW_NAME) const -> std::int32_t
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(BYLAW_NAME);

    auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().InternalSession().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed trying to load smart contract from string : ")(
            strContract.get())(".")
            .Flush();
        return OT_ERROR;
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Smart contract loaded up, but failed to find a bylaw "
            "with the name: ")(BYLAW_NAME)(".")
            .Flush();
        return OT_ERROR;
    }

    return pBylaw->GetHookCount();
}

auto OTAPI_Exec::Bylaw_GetCallbackCount(
    const UnallocatedCString& THE_CONTRACT,
    const UnallocatedCString& BYLAW_NAME) const -> std::int32_t
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(BYLAW_NAME);

    auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().InternalSession().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed trying to load smart contract from string: ")(
            strContract.get())(".")
            .Flush();
        return OT_ERROR;
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Smart contract loaded up, but failed to find a bylaw "
            "with the name: ")(BYLAW_NAME)(".")
            .Flush();
        return OT_ERROR;
    }

    return pBylaw->GetCallbackCount();
}

auto OTAPI_Exec::Clause_GetNameByIndex(
    const UnallocatedCString& THE_CONTRACT,
    const UnallocatedCString& BYLAW_NAME,
    const std::int32_t& nIndex) const
    -> UnallocatedCString  // returns the name of the clause.
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(BYLAW_NAME);

    const auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().InternalSession().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed trying to load smart contract from string: ")(
            strContract.get())(".")
            .Flush();
        return {};
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Smart contract loaded up, but failed to retrieve the "
            "bylaw with name: ")(BYLAW_NAME)(".")
            .Flush();
        return {};
    }

    const std::int32_t nTempIndex = nIndex;
    OTClause* pClause = pBylaw->GetClauseByIndex(nTempIndex);
    if (nullptr == pClause) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Smart contract loaded up, and "
            "bylaw found, but failed to retrieve "
            "the clause at index: ")(nTempIndex)(".")
            .Flush();
        return {};
    }

    // Success.
    return pClause->GetName().Get();
}

auto OTAPI_Exec::Clause_GetContents(
    const UnallocatedCString& THE_CONTRACT,
    const UnallocatedCString& BYLAW_NAME,
    const UnallocatedCString& CLAUSE_NAME) const
    -> UnallocatedCString  // returns the contents of the
                           // clause.
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(BYLAW_NAME);
    OT_VERIFY_STD_STR(CLAUSE_NAME);

    const auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().InternalSession().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed trying to load smart contract from string: ")(
            strContract.get())(".")
            .Flush();
        return {};
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Smart contract loaded up, but failed to retrieve the "
            "bylaw with name: ")(BYLAW_NAME)(".")
            .Flush();
        return {};
    }

    OTClause* pClause = pBylaw->GetClause(CLAUSE_NAME);

    if (nullptr == pClause) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Smart contract loaded up, and "
            "bylaw found, but failed to retrieve "
            "the clause with name: ")(CLAUSE_NAME)(".")
            .Flush();
        return {};
    }
    // Success.
    return pClause->GetCode();
}

auto OTAPI_Exec::Variable_GetNameByIndex(
    const UnallocatedCString& THE_CONTRACT,
    const UnallocatedCString& BYLAW_NAME,
    const std::int32_t& nIndex) const
    -> UnallocatedCString  // returns the name of the variable.
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(BYLAW_NAME);

    const auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().InternalSession().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed trying to load smart contract from string: ")(
            strContract.get())(".")
            .Flush();
        return {};
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Smart contract loaded up, but failed to retrieve the "
            "bylaw with name: ")(BYLAW_NAME)(".")
            .Flush();
        return {};
    }

    const std::int32_t nTempIndex = nIndex;
    OTVariable* pVar = pBylaw->GetVariableByIndex(nTempIndex);
    if (nullptr == pVar) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Smart contract loaded up, and "
            "bylaw found, but failed to retrieve "
            "the variable at index: ")(nTempIndex)(".")
            .Flush();
        return {};
    }

    return pVar->GetName().Get();
}

auto OTAPI_Exec::Variable_GetType(
    const UnallocatedCString& THE_CONTRACT,
    const UnallocatedCString& BYLAW_NAME,
    const UnallocatedCString& VARIABLE_NAME) const
    -> UnallocatedCString  // returns the type
                           // of the variable.
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(BYLAW_NAME);
    OT_VERIFY_STD_STR(VARIABLE_NAME);

    const auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().InternalSession().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed trying to load smart contract from string: ")(
            strContract.get())(".")
            .Flush();
        return {};
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Smart contract loaded up, but failed to retrieve the "
            "bylaw with name: ")(BYLAW_NAME)(".")
            .Flush();
        return {};
    }

    OTVariable* pVar = pBylaw->GetVariable(VARIABLE_NAME);
    if (nullptr == pVar) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Smart contract loaded up, and bylaw found, but "
            "failed to retrieve the variable with name: ")(VARIABLE_NAME)(".")
            .Flush();
        return {};
    }

    if (pVar->IsInteger()) { return "integer"; }
    if (pVar->IsBool()) { return "boolean"; }
    if (pVar->IsString()) { return "string"; }
    return "error_type";
}

auto OTAPI_Exec::Variable_GetAccess(
    const UnallocatedCString& THE_CONTRACT,
    const UnallocatedCString& BYLAW_NAME,
    const UnallocatedCString& VARIABLE_NAME) const
    -> UnallocatedCString  // returns the access level of the
                           // variable.
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(BYLAW_NAME);
    OT_VERIFY_STD_STR(VARIABLE_NAME);

    const auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().InternalSession().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed trying to load smart contract from string: ")(
            strContract.get())(".")
            .Flush();
        return {};
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Smart contract loaded up, but failed to retrieve the "
            "bylaw with name: ")(BYLAW_NAME)(".")
            .Flush();
        return {};
    }

    OTVariable* pVar = pBylaw->GetVariable(VARIABLE_NAME);
    if (nullptr == pVar) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Smart contract loaded up, and bylaw found, but "
            "failed to retrieve the variable with name: ")(VARIABLE_NAME)(".")
            .Flush();
        return {};
    }

    if (pVar->IsConstant()) { return "constant"; }
    if (pVar->IsImportant()) { return "important"; }
    if (pVar->IsPersistent()) { return "persistent"; }
    return "error_access";
}

auto OTAPI_Exec::Variable_GetContents(
    const UnallocatedCString& THE_CONTRACT,
    const UnallocatedCString& BYLAW_NAME,
    const UnallocatedCString& VARIABLE_NAME) const
    -> UnallocatedCString  // returns the contents of the
                           // variable.
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(BYLAW_NAME);
    OT_VERIFY_STD_STR(VARIABLE_NAME);

    const auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().InternalSession().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed trying to load smart contract from string: ")(
            strContract.get())(".")
            .Flush();
        return {};
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Smart contract loaded up, but failed to retrieve the "
            "bylaw with name: ")(BYLAW_NAME)(".")
            .Flush();
        return {};
    }

    OTVariable* pVar = pBylaw->GetVariable(VARIABLE_NAME);
    if (nullptr == pVar) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Smart contract loaded up, and bylaw found, but "
            "failed to retrieve the variable with name: ")(VARIABLE_NAME)(".")
            .Flush();
        return {};
    }

    switch (pVar->GetType()) {
        case OTVariable::Var_String: {

            return pVar->GetValueString();
        }
        case OTVariable::Var_Integer: {

            return String::LongToString(
                static_cast<std::int64_t>(pVar->GetValueInteger()));
        }
        case OTVariable::Var_Bool: {

            return pVar->GetValueBool() ? "true" : "false";
        }
        case OTVariable::Var_Error_Type:
        default: {
            LogError()(OT_PRETTY_CLASS())("Error: Unknown variable type.")
                .Flush();

            return {};
        }
    }
}

auto OTAPI_Exec::Hook_GetNameByIndex(
    const UnallocatedCString& THE_CONTRACT,
    const UnallocatedCString& BYLAW_NAME,
    const std::int32_t& nIndex) const
    -> UnallocatedCString  // returns the name of the hook.
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(BYLAW_NAME);

    const auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().InternalSession().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed trying to load smart contract from string: ")(
            strContract.get())(".")
            .Flush();
        return {};
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Smart contract loaded up, but failed to retrieve the "
            "bylaw with name: ")(BYLAW_NAME)(".")
            .Flush();
        return {};
    }

    const std::int32_t nTempIndex = nIndex;
    return pBylaw->GetHookNameByIndex(nTempIndex);
}

/// Returns the number of clauses attached to a specific hook.
auto OTAPI_Exec::Hook_GetClauseCount(
    const UnallocatedCString& THE_CONTRACT,
    const UnallocatedCString& BYLAW_NAME,
    const UnallocatedCString& HOOK_NAME) const -> std::int32_t
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(BYLAW_NAME);
    OT_VERIFY_STD_STR(HOOK_NAME);

    const auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().InternalSession().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed trying to load smart contract from string: ")(
            strContract.get())(".")
            .Flush();
        return OT_ERROR;
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Smart contract loaded up, but failed to retrieve the "
            "bylaw with name: ")(BYLAW_NAME)(".")
            .Flush();
        return OT_ERROR;
    }

    mapOfClauses theResults;
    // Look up all clauses matching a specific hook.
    if (!pBylaw->GetHooks(HOOK_NAME, theResults)) { return OT_ERROR; }

    return static_cast<std::int32_t>(theResults.size());
}

/// Multiple clauses can trigger from the same hook.
/// Hook_GetClauseCount and Hook_GetClauseAtIndex allow you to
/// iterate through them.
/// This function returns the name for the clause at the specified index.
///
auto OTAPI_Exec::Hook_GetClauseAtIndex(
    const UnallocatedCString& THE_CONTRACT,
    const UnallocatedCString& BYLAW_NAME,
    const UnallocatedCString& HOOK_NAME,
    const std::int32_t& nIndex) const -> UnallocatedCString
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(BYLAW_NAME);
    OT_VERIFY_STD_STR(HOOK_NAME);

    const auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().InternalSession().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed trying to load smart contract from string: ")(
            strContract.get())(".")
            .Flush();
        return {};
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Smart contract loaded up, but failed to retrieve the "
            "bylaw with name: ")(BYLAW_NAME)(".")
            .Flush();
        return {};
    }

    mapOfClauses theResults;

    // Look up all clauses matching a specific hook.
    if (!pBylaw->GetHooks(HOOK_NAME, theResults)) { return {}; }

    if ((nIndex < 0) ||
        (nIndex >= static_cast<std::int64_t>(theResults.size()))) {
        return {};
    }

    std::int32_t nLoopIndex = -1;
    for (auto& it : theResults) {
        OTClause* pClause = it.second;
        OT_ASSERT(nullptr != pClause);
        ++nLoopIndex;  // on first iteration, this is now 0.

        if (nLoopIndex == nIndex) { return pClause->GetName().Get(); }
    }
    return {};
}

auto OTAPI_Exec::Callback_GetNameByIndex(
    const UnallocatedCString& THE_CONTRACT,
    const UnallocatedCString& BYLAW_NAME,
    const std::int32_t& nIndex) const
    -> UnallocatedCString  // returns the name of the callback.
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(BYLAW_NAME);

    const auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().InternalSession().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed trying to load smart contract from string: ")(
            strContract.get())(".")
            .Flush();
        return {};
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Smart contract loaded up, but failed to retrieve the "
            "bylaw with name: ")(BYLAW_NAME)(".")
            .Flush();
        return {};
    }

    const std::int32_t nTempIndex = nIndex;
    return pBylaw->GetCallbackNameByIndex(nTempIndex);
}

auto OTAPI_Exec::Callback_GetClause(
    const UnallocatedCString& THE_CONTRACT,
    const UnallocatedCString& BYLAW_NAME,
    const UnallocatedCString& CALLBACK_NAME) const
    -> UnallocatedCString  // returns name of clause attached
                           // to callback.
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(BYLAW_NAME);
    OT_VERIFY_STD_STR(CALLBACK_NAME);

    const auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().InternalSession().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed trying to load smart contract from string: ")(
            strContract.get())(".")
            .Flush();
        return {};
    }

    OTBylaw* pBylaw = pScriptable->GetBylaw(BYLAW_NAME);
    if (nullptr == pBylaw) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Smart contract loaded up, but failed to retrieve the "
            "bylaw with name: ")(BYLAW_NAME)(".")
            .Flush();
        return {};
    }

    OTClause* pClause = pBylaw->GetCallback(CALLBACK_NAME);
    if (nullptr == pClause) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Smart contract loaded up, and bylaw found, but "
            "failed to retrieve the clause for callback: ")(CALLBACK_NAME)(".")
            .Flush();
        return {};
    }

    return pClause->GetName().Get();
}

auto OTAPI_Exec::Party_GetAcctCount(
    const UnallocatedCString& THE_CONTRACT,
    const UnallocatedCString& PARTY_NAME) const -> std::int32_t
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(PARTY_NAME);

    auto strContract = String::Factory(THE_CONTRACT);
    auto pScriptable(api_.Factory().InternalSession().Scriptable(strContract));
    if (nullptr == pScriptable) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed trying to load smart contract from string: ")(
            strContract.get())(".")
            .Flush();
        return OT_ERROR;
    }

    OTParty* pParty = pScriptable->GetParty(PARTY_NAME);
    if (nullptr == pParty) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Smart contract loaded up, but failed to find a party "
            "with the name: ")(PARTY_NAME)(".")
            .Flush();
        return OT_ERROR;
    }

    return pParty->GetAccountCount();
}

auto OTAPI_Exec::Party_GetAgentCount(
    const UnallocatedCString& THE_CONTRACT,
    const UnallocatedCString& PARTY_NAME) const -> std::int32_t
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(PARTY_NAME);

    auto strContract = String::Factory(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        api_.Factory().InternalSession().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed trying to load smart contract from string: ")(
            strContract.get())(".")
            .Flush();
        return OT_ERROR;
    }

    OTParty* pParty = pScriptable->GetParty(PARTY_NAME);
    if (nullptr == pParty) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Smart contract loaded up, but failed to find a party "
            "with the name: ")(PARTY_NAME)(".")
            .Flush();
        return OT_ERROR;
    }

    return pParty->GetAgentCount();
}

// returns either NymID or Entity ID.
// (If there is one... Contract might not be
// signed yet.)
auto OTAPI_Exec::Party_GetID(
    const UnallocatedCString& THE_CONTRACT,
    const UnallocatedCString& PARTY_NAME) const -> UnallocatedCString
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(PARTY_NAME);

    auto strContract = String::Factory(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        api_.Factory().InternalSession().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed trying to load smart contract from string: ")(
            strContract.get())(".")
            .Flush();
        return {};
    }

    OTParty* pParty = pScriptable->GetParty(PARTY_NAME);
    if (nullptr == pParty) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Smart contract loaded up, but failed to find a party "
            "with the name: ")(PARTY_NAME)(".")
            .Flush();
        return {};
    }

    return pParty->GetPartyID();
}

auto OTAPI_Exec::Party_GetAcctNameByIndex(
    const UnallocatedCString& THE_CONTRACT,
    const UnallocatedCString& PARTY_NAME,
    const std::int32_t& nIndex) const
    -> UnallocatedCString  // returns the name of the clause.
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(PARTY_NAME);

    const auto strContract = String::Factory(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        api_.Factory().InternalSession().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed trying to load smart contract from string: ")(
            strContract.get())(".")
            .Flush();
        return {};
    }

    OTParty* pParty = pScriptable->GetParty(PARTY_NAME);
    if (nullptr == pParty) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Smart contract loaded up, but failed to retrieve the "
            "party with name: ")(PARTY_NAME)(".")
            .Flush();
        return {};
    }

    const std::int32_t nTempIndex = nIndex;
    OTPartyAccount* pAcct = pParty->GetAccountByIndex(nTempIndex);
    if (nullptr == pAcct) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Smart contract loaded up, and "
            "party found, but failed to retrieve "
            "the account at index: ")(nTempIndex)(".")
            .Flush();
        return {};
    }

    return pAcct->GetName().Get();
}

auto OTAPI_Exec::Party_GetAcctID(
    const UnallocatedCString& THE_CONTRACT,
    const UnallocatedCString& PARTY_NAME,
    const UnallocatedCString& ACCT_NAME) const
    -> UnallocatedCString  // returns the account ID based on the
                           // account
                           // name. (If there is one yet...)
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(PARTY_NAME);
    OT_VERIFY_STD_STR(ACCT_NAME);

    const auto strContract = String::Factory(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        api_.Factory().InternalSession().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed trying to load smart contract from string: ")(
            strContract.get())(".")
            .Flush();
        return {};
    }

    OTParty* pParty = pScriptable->GetParty(PARTY_NAME);
    if (nullptr == pParty) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Smart contract loaded up, but failed to retrieve the "
            "party with name: ")(PARTY_NAME)(".")
            .Flush();
        return {};
    }

    const OTPartyAccount* pAcct = pParty->GetAccount(ACCT_NAME);
    if (nullptr == pAcct) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Smart contract loaded up, and "
            "party found, but failed to retrieve "
            "party's account named: ")(ACCT_NAME)(".")
            .Flush();
        return {};
    }

    return pAcct->GetAcctID().Get();
}

auto OTAPI_Exec::Party_GetAcctInstrumentDefinitionID(
    const UnallocatedCString& THE_CONTRACT,
    const UnallocatedCString& PARTY_NAME,
    const UnallocatedCString& ACCT_NAME) const
    -> UnallocatedCString  // returns the instrument definition ID
                           // based on
                           // the
                           // account name.
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(PARTY_NAME);
    OT_VERIFY_STD_STR(ACCT_NAME);

    const auto strContract = String::Factory(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        api_.Factory().InternalSession().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed trying to load smart contract from string: ")(
            strContract.get())(".")
            .Flush();
    } else {
        OTParty* pParty = pScriptable->GetParty(PARTY_NAME);
        if (nullptr == pParty) {
            LogConsole()(OT_PRETTY_CLASS())(
                "Smart contract loaded up, but failed to retrieve the "
                "party with name: ")(PARTY_NAME)(".")
                .Flush();
        } else  // We found the party...
        {
            const OTPartyAccount* pAcct = pParty->GetAccount(ACCT_NAME);

            if (nullptr == pAcct) {
                LogConsole()(OT_PRETTY_CLASS())(
                    "Smart contract loaded up, and "
                    "party found, but failed to retrieve "
                    "party's account named: ")(ACCT_NAME)(".")
                    .Flush();
            } else  // We found the account...
            {
                const UnallocatedCString str_return(
                    pAcct->GetInstrumentDefinitionID().Get());  // Success.
                return str_return;
            }
        }
    }
    return {};
}

auto OTAPI_Exec::Party_GetAcctAgentName(
    const UnallocatedCString& THE_CONTRACT,
    const UnallocatedCString& PARTY_NAME,
    const UnallocatedCString& ACCT_NAME) const
    -> UnallocatedCString  // returns the authorized agent for the
                           // named
                           // account.
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(PARTY_NAME);
    OT_VERIFY_STD_STR(ACCT_NAME);

    const auto strContract = String::Factory(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        api_.Factory().InternalSession().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed trying to load smart contract from string: ")(
            strContract.get())(".")
            .Flush();
    } else {
        OTParty* pParty = pScriptable->GetParty(PARTY_NAME);
        if (nullptr == pParty) {
            LogConsole()(OT_PRETTY_CLASS())(
                "Smart contract loaded up, but failed to retrieve the "
                "party with name: ")(PARTY_NAME)(".")
                .Flush();
        } else  // We found the party...
        {
            const OTPartyAccount* pAcct = pParty->GetAccount(ACCT_NAME);

            if (nullptr == pAcct) {
                LogConsole()(OT_PRETTY_CLASS())(
                    "Smart contract loaded up, and "
                    "party found, but failed to retrieve "
                    "party's account named: ")(ACCT_NAME)(".")
                    .Flush();
            } else  // We found the account...
            {
                const UnallocatedCString str_return(
                    pAcct->GetAgentName().Get());  // Success.
                return str_return;
            }
        }
    }
    return {};
}

auto OTAPI_Exec::Party_GetAgentNameByIndex(
    const UnallocatedCString& THE_CONTRACT,
    const UnallocatedCString& PARTY_NAME,
    const std::int32_t& nIndex) const
    -> UnallocatedCString  // returns the name of the agent.
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(PARTY_NAME);

    const auto strContract = String::Factory(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        api_.Factory().InternalSession().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed trying to load smart contract from string: ")(
            strContract.get())(".")
            .Flush();
    } else {
        OTParty* pParty = pScriptable->GetParty(PARTY_NAME);
        if (nullptr == pParty) {
            LogConsole()(OT_PRETTY_CLASS())(
                "Smart contract loaded up, but failed to retrieve the "
                "party with name: ")(PARTY_NAME)(".")
                .Flush();
        } else  // We found the party...
        {
            const std::int32_t nTempIndex = nIndex;
            OTAgent* pAgent = pParty->GetAgentByIndex(nTempIndex);

            if (nullptr == pAgent) {
                LogConsole()(OT_PRETTY_CLASS())(
                    "Smart contract loaded up, and party found, but "
                    "failed to retrieve the agent at index: ")(nTempIndex)(".")
                    .Flush();
            } else  // We found the agent...
            {
                const UnallocatedCString str_name(
                    pAgent->GetName().Get());  // Success.
                return str_name;
            }
        }
    }
    return {};
}

auto OTAPI_Exec::Party_GetAgentID(
    const UnallocatedCString& THE_CONTRACT,
    const UnallocatedCString& PARTY_NAME,
    const UnallocatedCString& AGENT_NAME) const
    -> UnallocatedCString  // returns ID of the agent. (If
                           // there is one...)
{
    OT_VERIFY_STD_STR(THE_CONTRACT);
    OT_VERIFY_STD_STR(PARTY_NAME);
    OT_VERIFY_STD_STR(AGENT_NAME);

    const auto strContract = String::Factory(THE_CONTRACT);
    std::unique_ptr<OTScriptable> pScriptable(
        api_.Factory().InternalSession().Scriptable(strContract));
    if (false == bool(pScriptable)) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed trying to load smart contract from string: ")(
            strContract.get())(".")
            .Flush();
    } else {
        OTParty* pParty = pScriptable->GetParty(PARTY_NAME);
        if (nullptr == pParty) {
            LogConsole()(OT_PRETTY_CLASS())(
                "Smart contract loaded up, but failed to retrieve the "
                "party with name: ")(PARTY_NAME)(".")
                .Flush();
        } else  // We found the party...
        {
            OTAgent* pAgent = pParty->GetAgent(AGENT_NAME);

            if (nullptr == pAgent) {
                LogConsole()(OT_PRETTY_CLASS())(
                    "Smart contract loaded up, and "
                    "party found, but failed to retrieve "
                    "party's agent named: ")(AGENT_NAME)(".")
                    .Flush();
            } else  // We found the agent...
            {
                auto theAgentID = identifier::Nym{};
                if (pAgent->IsAnIndividual() && pAgent->GetNymID(theAgentID)) {
                    return theAgentID.asBase58(api_.Crypto());
                }
            }
        }
    }
    return {};
}

// IS BASKET CURRENCY ?
//
// Tells you whether or not a given instrument definition is actually a
// basket currency.
//
// returns bool (true or false aka 1 or 0.)
//
auto OTAPI_Exec::IsBasketCurrency(
    const UnallocatedCString& INSTRUMENT_DEFINITION_ID) const -> bool
{
    OT_VERIFY_ID_STR(INSTRUMENT_DEFINITION_ID);

    const auto theInstrumentDefinitionID =
        api_.Factory().UnitIDFromBase58(INSTRUMENT_DEFINITION_ID);

    if (ot_api_.IsBasketCurrency(theInstrumentDefinitionID)) {
        return true;
    } else {
        return false;
    }
}

// Get Basket Count (of backing instrument definitions.)
//
// Returns the number of instrument definitions that make up this basket.
// (Or zero.)
//
auto OTAPI_Exec::Basket_GetMemberCount(
    const UnallocatedCString& INSTRUMENT_DEFINITION_ID) const -> std::int32_t
{
    OT_VERIFY_ID_STR(INSTRUMENT_DEFINITION_ID);

    const auto theInstrumentDefinitionID =
        api_.Factory().UnitIDFromBase58(INSTRUMENT_DEFINITION_ID);

    return ot_api_.GetBasketMemberCount(theInstrumentDefinitionID);
}

// Get Asset Type of a basket's member currency, by index.
//
// (Returns a string containing Instrument Definition ID, or "").
//
auto OTAPI_Exec::Basket_GetMemberType(
    const UnallocatedCString& BASKET_INSTRUMENT_DEFINITION_ID,
    const std::int32_t& nIndex) const -> UnallocatedCString
{
    OT_VERIFY_ID_STR(BASKET_INSTRUMENT_DEFINITION_ID);
    OT_VERIFY_MIN_BOUND(nIndex, 0);

    const auto theInstrumentDefinitionID =
        api_.Factory().UnitIDFromBase58(BASKET_INSTRUMENT_DEFINITION_ID);

    auto theOutputMemberType = identifier::UnitDefinition{};

    bool bGotType = ot_api_.GetBasketMemberType(
        theInstrumentDefinitionID, nIndex, theOutputMemberType);
    if (!bGotType) { return {}; }

    return theOutputMemberType.asBase58(api_.Crypto());
}

// GET BASKET MINIMUM TRANSFER AMOUNT
//
// Returns a std::int64_t containing the minimum transfer
// amount for the entire basket.
//
// Returns OT_ERROR_AMOUNT on error.
//
// FOR EXAMPLE:
// If the basket is defined as 10 Rands == 2 Silver, 5 Gold, 8 Euro,
// then the minimum transfer amount for the basket is 10. This function
// would return a string containing "10", in that example.
//
auto OTAPI_Exec::Basket_GetMinimumTransferAmount(
    const UnallocatedCString& BASKET_INSTRUMENT_DEFINITION_ID) const -> Amount
{
    OT_VERIFY_ID_STR(BASKET_INSTRUMENT_DEFINITION_ID);

    const auto theInstrumentDefinitionID =
        api_.Factory().UnitIDFromBase58(BASKET_INSTRUMENT_DEFINITION_ID);

    const Amount lMinTransAmount =
        ot_api_.GetBasketMinimumTransferAmount(theInstrumentDefinitionID);

    if (0 >= lMinTransAmount) {
        LogError()(OT_PRETTY_CLASS())(
            "Returned 0 (or negative). Strange! What basket is "
            "this?")
            .Flush();
        return OT_ERROR_AMOUNT;
    }

    return lMinTransAmount;
}

// GET BASKET MEMBER's MINIMUM TRANSFER AMOUNT
//
// Returns a std::int64_t containing the minimum transfer
// amount for one of the member currencies in the basket.
//
// Returns OT_ERROR_AMOUNT on error.
//
// FOR EXAMPLE:
// If the basket is defined as 10 Rands == 2 Silver, 5 Gold, 8 Euro,
// then the minimum transfer amount for the member currency at index
// 0 is 2, the minimum transfer amount for the member currency at
// index 1 is 5, and the minimum transfer amount for the member
// currency at index 2 is 8.
//
auto OTAPI_Exec::Basket_GetMemberMinimumTransferAmount(
    const UnallocatedCString& BASKET_INSTRUMENT_DEFINITION_ID,
    const std::int32_t& nIndex) const -> Amount
{
    OT_VERIFY_ID_STR(BASKET_INSTRUMENT_DEFINITION_ID);
    OT_VERIFY_MIN_BOUND(nIndex, 0);

    const auto theInstrumentDefinitionID =
        api_.Factory().UnitIDFromBase58(BASKET_INSTRUMENT_DEFINITION_ID);

    Amount lMinTransAmount = ot_api_.GetBasketMemberMinimumTransferAmount(
        theInstrumentDefinitionID, nIndex);

    if (0 >= lMinTransAmount) {
        LogError()(OT_PRETTY_CLASS())(
            "Returned 0 (or negative). Strange! What basket is "
            "this?")
            .Flush();
        return OT_ERROR_AMOUNT;
    }

    return lMinTransAmount;
}

// GENERATE BASKET CREATION REQUEST
//
// (returns the basket in string form.)
//
// Call OTAPI_Exec::AddBasketCreationItem multiple times to add
// the various currencies to the basket, and then call
// OTAPI_Exec::issueBasket to send the request to the server.
//
auto OTAPI_Exec::GenerateBasketCreation(
    const UnallocatedCString& serverID,
    const UnallocatedCString& shortname,
    const UnallocatedCString& terms,
    const std::uint64_t weight,
    const display::Definition& displayDefinition,
    const Amount& redemptionIncrement,
    const VersionNumber version) const -> UnallocatedCString
{
    try {
        const auto serverContract =
            api_.Wallet().Server(api_.Factory().NotaryIDFromBase58(serverID));
        const auto basketTemplate = api_.Factory().BasketContract(
            serverContract->Nym(),
            shortname,
            terms,
            weight,
            UnitType::Unknown,
            version,
            displayDefinition,
            redemptionIncrement);

        auto serialized = proto::UnitDefinition{};
        if (false == basketTemplate->Serialize(serialized, true)) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed to serialize unit definition.")
                .Flush();
            return {};
        }
        return api_.Factory()
            .InternalSession()
            .Armored(serialized, "BASKET CONTRACT")
            ->Get();
    } catch (...) {

        return {};
    }
}

// ADD BASKET CREATION ITEM
//
// (returns the updated basket in string form.)
//
// Call OTAPI_Exec::GenerateBasketCreation first (above), then
// call this function multiple times to add the various
// currencies to the basket, and then call OTAPI_Exec::issueBasket
// to send the request to the server.
//
auto OTAPI_Exec::AddBasketCreationItem(
    const UnallocatedCString& basketTemplate,
    const UnallocatedCString& currencyID,
    const std::uint64_t& weight) const -> UnallocatedCString
{
    OT_ASSERT_MSG(
        !basketTemplate.empty(),
        "OTAPI_Exec::AddBasketCreationItem: Null basketTemplate passed "
        "in.");
    OT_ASSERT_MSG(
        !currencyID.empty(),
        "OTAPI_Exec::AddBasketCreationItem: Null currencyID passed in.");

    bool bAdded = false;
    auto contract = proto::StringToProto<proto::UnitDefinition>(
        String::Factory(basketTemplate));

    bAdded = ot_api_.AddBasketCreationItem(
        contract, String::Factory(currencyID), weight);

    if (!bAdded) { return {}; }

    return api_.Factory()
        .InternalSession()
        .Armored(contract, "BASKET CONTRACT")
        ->Get();
}

// GENERATE BASKET EXCHANGE REQUEST
//
// (Returns the new basket exchange request in string form.)
//
// Call this function first. Then call OTAPI_Exec::AddBasketExchangeItem
// multiple times, and then finally call OTAPI_Exec::exchangeBasket to
// send the request to the server.
//
auto OTAPI_Exec::GenerateBasketExchange(
    const UnallocatedCString& NOTARY_ID,
    const UnallocatedCString& NYM_ID,
    const UnallocatedCString& BASKET_INSTRUMENT_DEFINITION_ID,
    const UnallocatedCString& BASKET_ASSET_ACCT_ID,
    const std::int32_t& TRANSFER_MULTIPLE) const -> UnallocatedCString
// 1            2            3
// 5=2,3,4  OR  10=4,6,8  OR 15=6,9,12
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_ID_STR(BASKET_INSTRUMENT_DEFINITION_ID);
    OT_VERIFY_ID_STR(BASKET_ASSET_ACCT_ID);

    const auto theNymID = api_.Factory().NymIDFromBase58(NYM_ID);
    const auto theNotaryID = api_.Factory().NotaryIDFromBase58(NOTARY_ID);
    const auto theBasketInstrumentDefinitionID =
        api_.Factory().UnitIDFromBase58(BASKET_INSTRUMENT_DEFINITION_ID);
    const auto theBasketAssetAcctID =
        api_.Factory().IdentifierFromBase58(BASKET_ASSET_ACCT_ID);
    std::int32_t nTransferMultiple = 1;  // Just a default value.

    if (TRANSFER_MULTIPLE > 0) { nTransferMultiple = TRANSFER_MULTIPLE; }
    std::unique_ptr<Basket> pBasket(ot_api_.GenerateBasketExchange(
        theNotaryID,
        theNymID,
        theBasketInstrumentDefinitionID,
        theBasketAssetAcctID,
        nTransferMultiple));
    // 1            2            3
    // 5=2,3,4  OR  10=4,6,8  OR 15=6,9,12

    if (nullptr == pBasket) { return {}; }

    // At this point, I know pBasket is good (and will be cleaned up
    // automatically.)
    auto strOutput =
        String::Factory(*pBasket);  // Extract the basket to string form.
    UnallocatedCString pBuf = strOutput->Get();
    return pBuf;
}

// ADD BASKET EXCHANGE ITEM
//
// Returns the updated basket exchange request in string form.
// (Or "".)
//
// Call the above function first. Then call this one multiple
// times, and then finally call OTAPI_Exec::exchangeBasket to send
// the request to the server.
//
auto OTAPI_Exec::AddBasketExchangeItem(
    const UnallocatedCString& NOTARY_ID,
    const UnallocatedCString& NYM_ID,
    const UnallocatedCString& THE_BASKET,
    const UnallocatedCString& INSTRUMENT_DEFINITION_ID,
    const UnallocatedCString& ASSET_ACCT_ID) const -> UnallocatedCString
{
    OT_VERIFY_ID_STR(NOTARY_ID);
    OT_VERIFY_ID_STR(NYM_ID);
    OT_VERIFY_STD_STR(THE_BASKET);
    OT_VERIFY_ID_STR(INSTRUMENT_DEFINITION_ID);
    OT_VERIFY_ID_STR(ASSET_ACCT_ID);

    auto strBasket = String::Factory(THE_BASKET);
    const auto theNotaryID = api_.Factory().NotaryIDFromBase58(NOTARY_ID);
    const auto theNymID = api_.Factory().NymIDFromBase58(NYM_ID);
    const auto theInstrumentDefinitionID =
        api_.Factory().UnitIDFromBase58(INSTRUMENT_DEFINITION_ID);
    const auto theAssetAcctID =
        api_.Factory().IdentifierFromBase58(ASSET_ACCT_ID);
    auto theBasket{api_.Factory().InternalSession().Basket()};

    OT_ASSERT(false != bool(theBasket));

    bool bAdded = false;

    // todo perhaps verify the basket here, even though I already verified
    // the asset contract itself... Can't never be too sure.
    if (theBasket->LoadContractFromString(strBasket)) {
        bAdded = ot_api_.AddBasketExchangeItem(
            theNotaryID,
            theNymID,
            *theBasket,
            theInstrumentDefinitionID,
            theAssetAcctID);
    }

    if (!bAdded) { return {}; }

    auto strOutput = String::Factory(*theBasket);  // Extract the updated basket
                                                   // to string form.
    UnallocatedCString pBuf = strOutput->Get();
    return pBuf;
}

auto OTAPI_Exec::Wallet_ImportSeed(
    const UnallocatedCString& words,
    const UnallocatedCString& passphrase) const -> UnallocatedCString
{
    auto reason = api_.Factory().PasswordPrompt("Importing a BIP-39 seed");
    auto secureWords = api_.Factory().SecretFromText(words);
    auto securePassphrase = api_.Factory().SecretFromText(passphrase);

    return api_.Crypto().Seed().ImportSeed(
        secureWords,
        securePassphrase,
        crypto::SeedStyle::BIP39,
        crypto::Language::en,
        reason);
}
}  // namespace opentxs
