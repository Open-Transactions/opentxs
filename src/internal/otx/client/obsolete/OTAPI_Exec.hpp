// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <cstdint>

#include "internal/core/contract/Unit.hpp"
#include "internal/util/Lockable.hpp"
#include "internal/util/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"
#include "opentxs/util/Time.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace session
{
class ClientPrivate;
}  // namespace session

class Session;
}  // namespace api

namespace display
{
class Definition;
}  // namespace display

class OT_API;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
class OTAPI_Exec : Lockable
{
public:
    /**

    PROPOSE PAYMENT PLAN --- Returns the payment plan in string form.

    (Called by Merchant.)

    PARAMETER NOTES:
    -- Payment Plan Delay, and Payment Plan Period, both default to 30 days (if
    you pass 0.)

    -- Payment Plan Length, and Payment Plan Max Payments, both default to 0,
    which means
    no maximum length and no maximum number of payments.

    -----------------------------------------------------------------
    FYI, the payment plan creation process (finally) is:

    1) Payment plan is written, and signed, by the recipient. (This function:
    ProposePaymentPlan)
    2) He sends it to the sender, who signs it and submits it.
    (ConfirmPaymentPlan and depositPaymentPlan)
    3) The server loads the recipient nym to verify the transaction
    number. The sender also had to burn a transaction number (to
    submit it) so now, both have verified trns#s in this way.

    ----------------------------------------------------------------------------------------

    FYI, here are all the OT library calls that are performed by this single API
    call:

    OTPaymentPlan * pPlan = new OTPaymentPlan(pAccount->GetRealNotaryID(),
    pAccount->GetInstrumentDefinitionID(),
    pAccount->GetRealAccountID(),    pAccount->GetNymID(),
    RECIPIENT_ACCT_ID, RECIPIENT_NYM_ID);

    ----------------------------------------------------------------------------------------
    From OTAgreement: (This must be called first, before the other two methods
    below can be called.)

    bool    OTAgreement::SetProposal(const identity::Nym& MERCHANT_NYM, const
    OTString& strConsideration,
    const time64_t& VALID_FROM=0, const time64_t& VALID_TO=0);

    ----------------------------------------------------------------------------------------
    (Optional initial payment):
    bool    OTPaymentPlan::SetInitialPayment(const std::int64_t& lAmount,
    time64_t
    tTimeUntilInitialPayment=0); // default: now.
    ----------------------------------------------------------------------------------------

    These two (above and below) can be called independent of each other. You can
    have an initial payment, AND/OR a payment plan.

    ----------------------------------------------------------------------------------------
    (Optional regular payments):
    bool    OTPaymentPlan::SetPaymentPlan(const std::int64_t& lPaymentAmount,
    time64_t tTimeUntilPlanStart  =OT_TIME_MONTH_IN_SECONDS, // Default: 1st
    payment in 30 days
    time64_t tBetweenPayments     =OT_TIME_MONTH_IN_SECONDS, // Default: 30
    days.
    time64_t tPlanLength=0, std::int32_t nMaxPayments=0);
    ----------------------------------------------------------------------------------------
    */
    auto ProposePaymentPlan(
        const UnallocatedCString& NOTARY_ID,
        const Time& VALID_FROM,  // Default (0 or nullptr) == current time
                                 // measured in seconds since Jan 1970.
        const Time& VALID_TO,    // Default (0 or nullptr) == no expiry /
                                 // cancel
                                 // anytime. Otherwise this is ADDED to
                                 // VALID_FROM (it's a length.)
        const UnallocatedCString& SENDER_ACCT_ID,  // Mandatory parameters.
        const UnallocatedCString& SENDER_NYM_ID,   // Both sender and recipient
                                                   // must sign before
                                                   // submitting.
        const UnallocatedCString& PLAN_CONSIDERATION,  // Like a memo.
        const UnallocatedCString& RECIPIENT_ACCT_ID,   // NOT optional.
        const UnallocatedCString& RECIPIENT_NYM_ID,    // Both sender and
                                                       // recipient must sign
                                                       // before submitting.
        const std::int64_t& INITIAL_PAYMENT_AMOUNT,    // zero or nullptr == no
                                                       // initial
                                                       // payment.
        const std::chrono::seconds& INITIAL_PAYMENT_DELAY,  // seconds from
                                                            // creation date.
                                                            // Default is zero
                                                            // or nullptr.
        const std::int64_t& PAYMENT_PLAN_AMOUNT,  // Zero or nullptr == no
                                                  // regular
                                                  // payments.
        const std::chrono::seconds& PAYMENT_PLAN_DELAY,  // No. of seconds from
                                                         // creation
        // date. Default is zero or nullptr.
        // (Causing 30 days.)
        const std::chrono::seconds& PAYMENT_PLAN_PERIOD,  // No. of seconds
                                                          // between payments.
                                                          // Default is zero or
                                                          // nullptr. (Causing
                                                          // 30 days.)
        const std::chrono::seconds& PAYMENT_PLAN_LENGTH,  // In seconds.
                                                          // Defaults to 0 or
                                                          // nullptr (no maximum
                                                          // length.)
        const std::int32_t& PAYMENT_PLAN_MAX_PAYMENTS  // integer. Defaults to 0
                                                       // or
        // nullptr (no maximum payments.)
    ) const -> UnallocatedCString;

    // The above version has too many arguments for boost::function apparently
    // (for Chaiscript.)
    // So this is a version of it that compresses those into a fewer number of
    // arguments.
    // (Then it expands them and calls the above version.)
    // See above function for more details on parameters.
    // Basically this version has ALL the same parameters, but it stuffs two or
    // three at a time into
    // a single parameter, as a comma-separated list in string form.
    //
    auto EasyProposePlan(
        const UnallocatedCString& NOTARY_ID,
        const UnallocatedCString& DATE_RANGE,  // "from,to"  Default 'from' (0
                                               // or "")
                                               // ==
                                               // NOW, and default 'to' (0 or
                                               // "") == no expiry / cancel
                                               // anytime
        const UnallocatedCString& SENDER_ACCT_ID,  // Mandatory parameters.
        const UnallocatedCString& SENDER_NYM_ID,   // Both sender and recipient
                                                   // must sign before
                                                   // submitting.
        const UnallocatedCString& PLAN_CONSIDERATION,  // Like a memo.
        const UnallocatedCString& RECIPIENT_ACCT_ID,   // NOT optional.
        const UnallocatedCString& RECIPIENT_NYM_ID,    // Both sender and
                                                       // recipient must sign
                                                       // before submitting.
        const UnallocatedCString& INITIAL_PAYMENT,  // "amount,delay"  Default
                                                    // 'amount' (0 or "") == no
                                                    // initial payment.
        // Default 'delay' (0 or nullptr) is
        // seconds from creation date.
        const UnallocatedCString& PAYMENT_PLAN,  // "amount,delay,period"
                                                 // 'amount' is a recurring
                                                 // payment. 'delay' and
        // 'period' cause 30 days if you pass 0
        // or "".
        const UnallocatedCString& PLAN_EXPIRY  // "length,number" 'length' is
                                               // maximum lifetime in seconds.
                                               // 'number' is
        // maximum number of payments in seconds.
        // 0 or "" is unlimited (for both.)
    ) const -> UnallocatedCString;

    // Called by Customer. Pass in the plan obtained in the above call.
    //
    auto ConfirmPaymentPlan(
        const UnallocatedCString& NOTARY_ID,
        const UnallocatedCString& SENDER_NYM_ID,
        const UnallocatedCString& SENDER_ACCT_ID,
        const UnallocatedCString& RECIPIENT_NYM_ID,
        const UnallocatedCString& PAYMENT_PLAN) const -> UnallocatedCString;

    // SMART CONTRACTS

    // RETURNS: the Smart Contract itself. (Or nullptr.)
    //
    auto Create_SmartContract(
        const UnallocatedCString& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                                  // (The signing at this point
                                                  // is only to cause a save.)
        const Time& VALID_FROM,  // Default (0 or nullptr) == NOW
        const Time& VALID_TO,    // Default (0 or nullptr) == no expiry /
                                 // cancel anytime
        bool SPECIFY_ASSETS,  // Asset type IDs must be provided for every named
                              // account.
        bool SPECIFY_PARTIES  // Nym IDs must be provided for every party.
    ) const -> UnallocatedCString;

    auto SmartContract_SetDates(
        const UnallocatedCString& THE_CONTRACT,   // The contract, about to have
                                                  // the dates changed on it.
        const UnallocatedCString& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                                  // (The signing at this point
                                                  // is only to cause a save.)
        const Time& VALID_FROM,  // Default (0 or nullptr) == NOW
        const Time& VALID_TO     // Default (0 or nullptr) == no expiry /
                                 // cancel
                                 // anytime
    ) const -> UnallocatedCString;

    auto Smart_ArePartiesSpecified(const UnallocatedCString& THE_CONTRACT) const
        -> bool;
    auto Smart_AreAssetTypesSpecified(
        const UnallocatedCString& THE_CONTRACT) const -> bool;

    //
    // todo: Someday add a parameter here BYLAW_LANGUAGE so that people can use
    // custom languages in their scripts. For now I have a default language, so
    // I'll just make that the default. (There's only one language right now
    // anyway.)
    //
    // returns: the updated smart contract (or nullptr)
    auto SmartContract_AddBylaw(
        const UnallocatedCString& THE_CONTRACT,   // The contract, about to have
                                                  // the bylaw added to it.
        const UnallocatedCString& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                                  // (The signing at this point
                                                  // is only to cause a save.)
        const UnallocatedCString& BYLAW_NAME  // The Bylaw's NAME as referenced
                                              // in the smart contract. (And the
                                              // scripts...)
    ) const -> UnallocatedCString;

    // returns: the updated smart contract (or nullptr)
    auto SmartContract_RemoveBylaw(
        const UnallocatedCString& THE_CONTRACT,   // The contract, about to have
                                                  // the bylaw removed from it.
        const UnallocatedCString& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                                  // (The signing at this point
                                                  // is only to cause a save.)
        const UnallocatedCString& BYLAW_NAME  // The Bylaw's NAME as referenced
                                              // in the smart contract. (And the
                                              // scripts...)
    ) const -> UnallocatedCString;

    // returns: the updated smart contract (or nullptr)
    auto SmartContract_AddClause(
        const UnallocatedCString& THE_CONTRACT,   // The contract, about to have
                                                  // the clause added to it.
        const UnallocatedCString& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                                  // (The signing at this point
                                                  // is only to cause a save.)
        const UnallocatedCString& BYLAW_NAME,     // Should already be on the
                                                  // contract. (This way we can
                                                  // find it.)
        const UnallocatedCString& CLAUSE_NAME,    // The Clause's name as
                                                  // referenced in the smart
                                                  // contract. (And the
                                                  // scripts...)
        const UnallocatedCString& SOURCE_CODE  // The actual source code for the
                                               // clause.
    ) const -> UnallocatedCString;

    // returns: the updated smart contract (or nullptr)
    auto SmartContract_UpdateClause(
        const UnallocatedCString& THE_CONTRACT,   // The contract, about to have
                                                  // the clause updated on it.
        const UnallocatedCString& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                                  // (The signing at this point
                                                  // is only to cause a save.)
        const UnallocatedCString& BYLAW_NAME,     // Should already be on the
                                                  // contract. (This way we can
                                                  // find it.)
        const UnallocatedCString& CLAUSE_NAME,    // The Clause's name as
                                                  // referenced in the smart
                                                  // contract. (And the
                                                  // scripts...)
        const UnallocatedCString& SOURCE_CODE  // The actual source code for the
                                               // clause.
    ) const -> UnallocatedCString;

    // returns: the updated smart contract (or nullptr)
    auto SmartContract_RemoveClause(
        const UnallocatedCString& THE_CONTRACT,   // The contract, about to have
                                                  // the clause removed from it.
        const UnallocatedCString& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                                  // (The signing at this point
                                                  // is only to cause a save.)
        const UnallocatedCString& BYLAW_NAME,     // Should already be on the
                                                  // contract. (This way we can
                                                  // find it.)
        const UnallocatedCString& CLAUSE_NAME     // The Clause's name as
                                                  // referenced in the smart
        // contract. (And the scripts...)
    ) const -> UnallocatedCString;

    // returns: the updated smart contract (or nullptr)
    auto SmartContract_AddVariable(
        const UnallocatedCString& THE_CONTRACT,   // The contract, about to have
                                                  // the variable added to it.
        const UnallocatedCString& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                                  // (The signing at this point
                                                  // is only to cause a save.)
        const UnallocatedCString& BYLAW_NAME,     // Should already be on the
                                                  // contract. (This way we can
                                                  // find it.)
        const UnallocatedCString& VAR_NAME,       // The Variable's name as
                                                  // referenced in the smart
        // contract. (And the scripts...)
        const UnallocatedCString& VAR_ACCESS,  // "constant", "persistent", or
                                               // "important".
        const UnallocatedCString& VAR_TYPE,    // "string", "std::int64_t", or
                                               // "bool"
        const UnallocatedCString& VAR_VALUE    // Contains a string. If type is
                                               // std::int64_t,
        // atol() will be used to convert value to
        // a std::int64_t. If type is bool, the strings
        // "true" or "false" are expected here in
        // order to convert to a bool.
    ) const -> UnallocatedCString;

    // returns: the updated smart contract (or nullptr)
    auto SmartContract_RemoveVariable(
        const UnallocatedCString& THE_CONTRACT,   // The contract, about to have
                                                  // the variable removed from
                                                  // it.
        const UnallocatedCString& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                                  // (The signing at this point
                                                  // is only to cause a save.)
        const UnallocatedCString& BYLAW_NAME,     // Should already be on the
                                                  // contract. (This way we can
                                                  // find it.)
        const UnallocatedCString& VAR_NAME  // The Variable's name as referenced
                                            // in the smart contract. (And the
                                            // scripts...)
    ) const -> UnallocatedCString;

    // returns: the updated smart contract (or nullptr)
    auto SmartContract_AddCallback(
        const UnallocatedCString& THE_CONTRACT,   // The contract, about to have
                                                  // the callback added to it.
        const UnallocatedCString& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                                  // (The signing at this point
                                                  // is only to cause a save.)
        const UnallocatedCString& BYLAW_NAME,     // Should already be on the
                                                  // contract. (This way we can
                                                  // find it.)
        const UnallocatedCString& CALLBACK_NAME,  // The Callback's name as
                                                  // referenced in the smart
                                                  // contract. (And the
                                                  // scripts...)
        const UnallocatedCString& CLAUSE_NAME  // The actual clause that will be
                                               // triggered by the callback.
                                               // (Must exist.)
    ) const -> UnallocatedCString;

    // returns: the updated smart contract (or nullptr)
    auto SmartContract_RemoveCallback(
        const UnallocatedCString& THE_CONTRACT,   // The contract, about to have
                                                  // the callback removed from
                                                  // it.
        const UnallocatedCString& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                                  // (The signing at this point
                                                  // is only to cause a save.)
        const UnallocatedCString& BYLAW_NAME,     // Should already be on the
                                                  // contract. (This way we can
                                                  // find it.)
        const UnallocatedCString& CALLBACK_NAME   // The Callback's name as
                                                  // referenced in the smart
                                                  // contract. (And the
                                                  // scripts...)
    ) const -> UnallocatedCString;

    // returns: the updated smart contract (or nullptr)
    auto SmartContract_AddHook(
        const UnallocatedCString& THE_CONTRACT,   // The contract, about to have
                                                  // the hook added to it.
        const UnallocatedCString& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                                  // (The signing at this point
                                                  // is only to cause a save.)
        const UnallocatedCString& BYLAW_NAME,     // Should already be on the
                                                  // contract. (This way we can
                                                  // find it.)
        const UnallocatedCString& HOOK_NAME,  // The Hook's name as referenced
                                              // in the smart contract. (And the
                                              // scripts...)
        const UnallocatedCString& CLAUSE_NAME  // The actual clause that will be
                                               // triggered by the hook. (You
                                               // can call
        // this multiple times, and have multiple
        // clauses trigger on the same hook.)
    ) const -> UnallocatedCString;

    // returns: the updated smart contract (or nullptr)
    auto SmartContract_RemoveHook(
        const UnallocatedCString& THE_CONTRACT,   // The contract, about to have
                                                  // the hook removed from it.
        const UnallocatedCString& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                                  // (The signing at this point
                                                  // is only to cause a save.)
        const UnallocatedCString& BYLAW_NAME,     // Should already be on the
                                                  // contract. (This way we can
                                                  // find it.)
        const UnallocatedCString& HOOK_NAME,  // The Hook's name as referenced
                                              // in the smart contract. (And the
                                              // scripts...)
        const UnallocatedCString& CLAUSE_NAME  // The actual clause that will be
                                               // triggered by the hook. (You
                                               // can call
        // this multiple times, and have multiple
        // clauses trigger on the same hook.)
    ) const -> UnallocatedCString;

    // RETURNS: Updated version of THE_CONTRACT. (Or nullptr.)
    auto SmartContract_AddParty(
        const UnallocatedCString& THE_CONTRACT,   // The contract, about to have
                                                  // the party added to it.
        const UnallocatedCString& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                                  // (The signing at this point
                                                  // is only to cause a save.)
        const UnallocatedCString& PARTY_NYM_ID,   // Required when the smart
                                                  // contract is configured to
                                                  // require parties to be
        // specified. Otherwise must be
        // empty.
        const UnallocatedCString& PARTY_NAME,  // The Party's NAME as referenced
                                               // in the smart contract. (And
                                               // the scripts...)
        const UnallocatedCString& AGENT_NAME   // An AGENT will be added by
                                               // default for this party. Need
                                               // Agent NAME.
    ) const -> UnallocatedCString;

    // RETURNS: Updated version of THE_CONTRACT. (Or nullptr.)
    auto SmartContract_RemoveParty(
        const UnallocatedCString& THE_CONTRACT,   // The contract, about to have
                                                  // the party removed from it.
        const UnallocatedCString& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                                  // (The signing at this point
                                                  // is only to cause a save.)
        const UnallocatedCString& PARTY_NAME  // The Party's NAME as referenced
                                              // in the smart contract. (And the
                                              // scripts...)
    ) const -> UnallocatedCString;

    // (FYI, that is basically the only option, until I code Entities and Roles.
    // Until then, a party can ONLY be
    // a Nym, with himself as the agent representing that same party. Nym ID is
    // supplied on ConfirmParty() below.)

    // Used when creating a theoretical smart contract (that could be used over
    // and over again with different parties.)
    //
    // returns: the updated smart contract (or nullptr)
    auto SmartContract_AddAccount(
        const UnallocatedCString& THE_CONTRACT,   // The contract, about to have
                                                  // the account added to it.
        const UnallocatedCString& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                                  // (The signing at this point
                                                  // is only to cause a save.)
        const UnallocatedCString& PARTY_NAME,  // The Party's NAME as referenced
                                               // in the smart contract. (And
                                               // the scripts...)
        const UnallocatedCString& ACCT_NAME,   // The Account's name as
                                               // referenced in the smart
                                               // contract
        const UnallocatedCString& INSTRUMENT_DEFINITION_ID  // Instrument
                                                            // Definition ID for
                                                            // the Account.
    ) const -> UnallocatedCString;

    // returns: the updated smart contract (or nullptr)
    auto SmartContract_RemoveAccount(
        const UnallocatedCString& THE_CONTRACT,  // The contract, about to have
                                                 // the account removed from it.
        const UnallocatedCString& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                                  // (The signing at this point
                                                  // is only to cause a save.)
        const UnallocatedCString& PARTY_NAME,  // The Party's NAME as referenced
                                               // in the smart contract. (And
                                               // the scripts...)
        const UnallocatedCString& ACCT_NAME  // The Account's name as referenced
                                             // in the smart contract
    ) const -> UnallocatedCString;

    /** This function returns the count of how many trans#s a Nym needs in order
    to confirm as
    // a specific agent for a contract. (An opening number is needed for every
    party of which
    // agent is the authorizing agent, plus a closing number for every acct of
    which agent is the
    // authorized agent.)
    */
    auto SmartContract_CountNumsNeeded(
        const UnallocatedCString& THE_CONTRACT,  // The smart contract, about to
                                                 // be queried by this function.
        const UnallocatedCString& AGENT_NAME) const -> std::int32_t;

    /** ----------------------------------------
    // Used when taking a theoretical smart contract, and setting it up to use
    specific Nyms and accounts. This function sets the ACCT ID for the acct
    specified by party name and acct name.
    // Returns the updated smart contract (or nullptr.)
    */
    auto SmartContract_ConfirmAccount(
        const UnallocatedCString& THE_CONTRACT,  // The smart contract, about to
                                                 // be changed by this function.
        const UnallocatedCString& SIGNER_NYM_ID,  // Use any Nym you wish here.
                                                  // (The signing at this point
                                                  // is only to cause a save.)
        const UnallocatedCString& PARTY_NAME,     // Should already be on the
                                                  // contract. (This way we can
                                                  // find it.)
        const UnallocatedCString& ACCT_NAME,      // Should already be on the
                                              // contract. (This way we can find
                                              // it.)
        const UnallocatedCString& AGENT_NAME,  // The agent name for this asset
                                               // account.
        const UnallocatedCString& ACCT_ID  // AcctID for the asset account. (For
                                           // acct_name).
    ) const -> UnallocatedCString;

    /** ----------------------------------------
    // Called by each Party. Pass in the smart contract obtained in the above
    call.
    // Call SmartContract_ConfirmAccount() first, as much as you need to.
    // Returns the updated smart contract (or nullptr.)
    */
    auto SmartContract_ConfirmParty(
        const UnallocatedCString& THE_CONTRACT,  // The smart contract, about to
                                                 // be changed by this function.
        const UnallocatedCString& PARTY_NAME,    // Should already be on the
                                               // contract. This way we can find
                                               // it.
        const UnallocatedCString& NYM_ID,  // Nym ID for the party, the actual
                                           // owner,
        const UnallocatedCString& NOTARY_ID) const -> UnallocatedCString;
    // ===> AS WELL AS for the default AGENT of that party.

    /* ----------------------------------------
    Various informational functions for the Smart Contracts.
    */

    auto Smart_AreAllPartiesConfirmed(const UnallocatedCString& THE_CONTRACT)
        const -> bool;  // true or false?
    auto Smart_GetBylawCount(const UnallocatedCString& THE_CONTRACT) const
        -> std::int32_t;
    auto Smart_GetBylawByIndex(
        const UnallocatedCString& THE_CONTRACT,
        const std::int32_t& nIndex) const
        -> UnallocatedCString;  // returns the name of the bylaw.
    auto Bylaw_GetLanguage(
        const UnallocatedCString& THE_CONTRACT,
        const UnallocatedCString& BYLAW_NAME) const -> UnallocatedCString;
    auto Bylaw_GetClauseCount(
        const UnallocatedCString& THE_CONTRACT,
        const UnallocatedCString& BYLAW_NAME) const -> std::int32_t;
    auto Clause_GetNameByIndex(
        const UnallocatedCString& THE_CONTRACT,
        const UnallocatedCString& BYLAW_NAME,
        const std::int32_t& nIndex) const
        -> UnallocatedCString;  // returns the name of the clause.
    auto Clause_GetContents(
        const UnallocatedCString& THE_CONTRACT,
        const UnallocatedCString& BYLAW_NAME,
        const UnallocatedCString& CLAUSE_NAME) const
        -> UnallocatedCString;  // returns the contents of the
                                // clause.
    auto Bylaw_GetVariableCount(
        const UnallocatedCString& THE_CONTRACT,
        const UnallocatedCString& BYLAW_NAME) const -> std::int32_t;
    auto Variable_GetNameByIndex(
        const UnallocatedCString& THE_CONTRACT,
        const UnallocatedCString& BYLAW_NAME,
        const std::int32_t& nIndex) const
        -> UnallocatedCString;  // returns the name of the variable.
    auto Variable_GetType(
        const UnallocatedCString& THE_CONTRACT,
        const UnallocatedCString& BYLAW_NAME,
        const UnallocatedCString& VARIABLE_NAME) const
        -> UnallocatedCString;  // returns the type of the
                                // variable.
    auto Variable_GetAccess(
        const UnallocatedCString& THE_CONTRACT,
        const UnallocatedCString& BYLAW_NAME,
        const UnallocatedCString& VARIABLE_NAME) const
        -> UnallocatedCString;  // returns the access level of
                                // the
                                // variable.
    auto Variable_GetContents(
        const UnallocatedCString& THE_CONTRACT,
        const UnallocatedCString& BYLAW_NAME,
        const UnallocatedCString& VARIABLE_NAME) const
        -> UnallocatedCString;  // returns the contents of the
                                // variable.
    auto Bylaw_GetHookCount(
        const UnallocatedCString& THE_CONTRACT,
        const UnallocatedCString& BYLAW_NAME) const -> std::int32_t;
    auto Hook_GetNameByIndex(
        const UnallocatedCString& THE_CONTRACT,
        const UnallocatedCString& BYLAW_NAME,
        const std::int32_t& nIndex) const
        -> UnallocatedCString;  // returns the name of the hook.
    auto Hook_GetClauseCount(
        const UnallocatedCString& THE_CONTRACT,
        const UnallocatedCString& BYLAW_NAME,
        const UnallocatedCString& HOOK_NAME) const
        -> std::int32_t;  // for iterating clauses on a
                          // hook.
    auto Hook_GetClauseAtIndex(
        const UnallocatedCString& THE_CONTRACT,
        const UnallocatedCString& BYLAW_NAME,
        const UnallocatedCString& HOOK_NAME,
        const std::int32_t& nIndex) const -> UnallocatedCString;
    auto Bylaw_GetCallbackCount(
        const UnallocatedCString& THE_CONTRACT,
        const UnallocatedCString& BYLAW_NAME) const -> std::int32_t;
    auto Callback_GetNameByIndex(
        const UnallocatedCString& THE_CONTRACT,
        const UnallocatedCString& BYLAW_NAME,
        const std::int32_t& nIndex) const
        -> UnallocatedCString;  // returns the name of the callback.
    auto Callback_GetClause(
        const UnallocatedCString& THE_CONTRACT,
        const UnallocatedCString& BYLAW_NAME,
        const UnallocatedCString& CALLBACK_NAME) const
        -> UnallocatedCString;  // returns name of clause
                                // attached to
                                // callback.
    auto Smart_GetPartyCount(const UnallocatedCString& THE_CONTRACT) const
        -> std::int32_t;
    auto Smart_GetPartyByIndex(
        const UnallocatedCString& THE_CONTRACT,
        const std::int32_t& nIndex) const
        -> UnallocatedCString;  // returns the name of the party.
    auto Smart_IsPartyConfirmed(
        const UnallocatedCString& THE_CONTRACT,
        const UnallocatedCString& PARTY_NAME) const -> bool;  // true or false?
    auto Party_GetID(
        const UnallocatedCString& THE_CONTRACT,
        const UnallocatedCString& PARTY_NAME) const
        -> UnallocatedCString;  // returns either NymID or Entity
                                // ID.
    auto Party_GetAcctCount(
        const UnallocatedCString& THE_CONTRACT,
        const UnallocatedCString& PARTY_NAME) const -> std::int32_t;
    auto Party_GetAcctNameByIndex(
        const UnallocatedCString& THE_CONTRACT,
        const UnallocatedCString& PARTY_NAME,
        const std::int32_t& nIndex) const
        -> UnallocatedCString;  // returns the name of the clause.
    auto Party_GetAcctID(
        const UnallocatedCString& THE_CONTRACT,
        const UnallocatedCString& PARTY_NAME,
        const UnallocatedCString& ACCT_NAME) const
        -> UnallocatedCString;  // returns account ID for a given
                                // acct
                                // name.
    auto Party_GetAcctInstrumentDefinitionID(
        const UnallocatedCString& THE_CONTRACT,
        const UnallocatedCString& PARTY_NAME,
        const UnallocatedCString& ACCT_NAME) const
        -> UnallocatedCString;  // returns instrument definition
                                // ID
                                // for a
                                // given acct
                                // name.
    auto Party_GetAcctAgentName(
        const UnallocatedCString& THE_CONTRACT,
        const UnallocatedCString& PARTY_NAME,
        const UnallocatedCString& ACCT_NAME) const
        -> UnallocatedCString;  // returns agent name authorized
                                // to
    // administer a given named acct. (If
    // it's set...)
    auto Party_GetAgentCount(
        const UnallocatedCString& THE_CONTRACT,
        const UnallocatedCString& PARTY_NAME) const -> std::int32_t;
    auto Party_GetAgentNameByIndex(
        const UnallocatedCString& THE_CONTRACT,
        const UnallocatedCString& PARTY_NAME,
        const std::int32_t& nIndex) const
        -> UnallocatedCString;  // returns the name of the agent.
    auto Party_GetAgentID(
        const UnallocatedCString& THE_CONTRACT,
        const UnallocatedCString& PARTY_NAME,
        const UnallocatedCString& AGENT_NAME) const
        -> UnallocatedCString;  // returns ID of the agent. (If
                                // there is
                                // one...)

    /** --------------------------------------------------------------
    // IS BASKET CURRENCY ?
    //
    // Tells you whether or not a given instrument definition is actually a
    basket
    currency.
    */
    auto IsBasketCurrency(const UnallocatedCString& INSTRUMENT_DEFINITION_ID)
        const -> bool;  // returns OT_BOOL
                        // (OT_TRUE or
                        // OT_FALSE aka 1
                        // or 0.)

    /** --------------------------------------------------------------------
    // Get Basket Count (of backing instrument definitions.)
    //
    // Returns the number of instrument definitions that make up this basket.
    // (Or zero.)
    */
    auto Basket_GetMemberCount(
        const UnallocatedCString& BASKET_INSTRUMENT_DEFINITION_ID) const
        -> std::int32_t;

    /** --------------------------------------------------------------------
    // Get Asset Type of a basket's member currency, by index.
    //
    // (Returns a string containing Instrument Definition ID, or nullptr).
    */
    auto Basket_GetMemberType(
        const UnallocatedCString& BASKET_INSTRUMENT_DEFINITION_ID,
        const std::int32_t& nIndex) const -> UnallocatedCString;

    /** ----------------------------------------------------
    // GET BASKET MINIMUM TRANSFER AMOUNT
    //
    // Returns a std::int64_t containing the minimum transfer
    // amount for the entire basket.
    //
    // FOR EXAMPLE:
    // If the basket is defined as 10 Rands == 2 Silver, 5 Gold, 8 Euro,
    // then the minimum transfer amount for the basket is 10. This function
    // would return a string containing "10", in that example.
    */
    auto Basket_GetMinimumTransferAmount(
        const UnallocatedCString& BASKET_INSTRUMENT_DEFINITION_ID) const
        -> Amount;

    /** ----------------------------------------------------
    // GET BASKET MEMBER's MINIMUM TRANSFER AMOUNT
    //
    // Returns a std::int64_t containing the minimum transfer
    // amount for one of the member currencies in the basket.
    //
    // FOR EXAMPLE:
    // If the basket is defined as 10 Rands == 2 Silver, 5 Gold, 8 Euro,
    // then the minimum transfer amount for the member currency at index
    // 0 is 2, the minimum transfer amount for the member currency at
    // index 1 is 5, and the minimum transfer amount for the member
    // currency at index 2 is 8.
    */
    auto Basket_GetMemberMinimumTransferAmount(
        const UnallocatedCString& BASKET_INSTRUMENT_DEFINITION_ID,
        const std::int32_t& nIndex) const -> Amount;

    /** ----------------------------------------------------
    // GENERATE BASKET CREATION REQUEST
    //
    // (returns the basket in string form.)
    //
    // Call AddBasketCreationItem multiple times to add
    // the various currencies to the basket, and then call
    // issueBasket to send the request to the server.
    */
    auto GenerateBasketCreation(
        const UnallocatedCString& nymID,
        const UnallocatedCString& shortname,
        const UnallocatedCString& terms,
        const std::uint64_t weight,
        const display::Definition& displayDefinition,
        const Amount& redemptionIncrement,
        const VersionNumber version = contract::Unit::DefaultVersion) const
        -> UnallocatedCString;

    /** ----------------------------------------------------
    // ADD BASKET CREATION ITEM
    //
    // (returns the updated basket in string form.)
    //
    // Call GenerateBasketCreation first (above), then
    // call this function multiple times to add the various
    // currencies to the basket, and then call issueBasket
    // to send the request to the server.
    */
    auto AddBasketCreationItem(
        const UnallocatedCString& basketTemplate,
        const UnallocatedCString& currencyID,
        const std::uint64_t& weight) const -> UnallocatedCString;

    /** ----------------------------------------------------
    // GENERATE BASKET EXCHANGE REQUEST
    //
    // (Returns the new basket exchange request in string form.)
    //
    // Call this function first. Then call AddBasketExchangeItem
    // multiple times, and then finally call exchangeBasket to
    // send the request to the server.
    */
    auto GenerateBasketExchange(
        const UnallocatedCString& NOTARY_ID,
        const UnallocatedCString& NYM_ID,
        const UnallocatedCString& BASKET_INSTRUMENT_DEFINITION_ID,
        const UnallocatedCString& BASKET_ASSET_ACCT_ID,
        const std::int32_t& TRANSFER_MULTIPLE) const -> UnallocatedCString;

    //! 1    2    3
    //! 5=2,3,4 OR 10=4,6,8 OR 15=6,9,12 Etc. (The MULTIPLE.)

    /** ----------------------------------------------------
    // ADD BASKET EXCHANGE ITEM
    //
    // Returns the updated basket exchange request in string form.
    // (Or nullptr.)
    //
    // Call the above function first. Then call this one multiple
    // times, and then finally call exchangeBasket to send
    // the request to the server.
    */
    auto AddBasketExchangeItem(
        const UnallocatedCString& NOTARY_ID,
        const UnallocatedCString& NYM_ID,
        const UnallocatedCString& THE_BASKET,
        const UnallocatedCString& INSTRUMENT_DEFINITION_ID,
        const UnallocatedCString& ASSET_ACCT_ID) const -> UnallocatedCString;

    OTAPI_Exec() = delete;
    OTAPI_Exec(const OTAPI_Exec&) = delete;
    OTAPI_Exec(OTAPI_Exec&&) = delete;
    auto operator=(const OTAPI_Exec&) -> OTAPI_Exec = delete;
    auto operator=(OTAPI_Exec&&) -> OTAPI_Exec = delete;

    ~OTAPI_Exec() override = default;

private:
    friend api::session::ClientPrivate;

    const api::Session& api_;
    const OT_API& ot_api_;
    ContextLockCallback lock_callback_;

    OTAPI_Exec(
        const api::Session& api,
        const OT_API& otapi,
        ContextLockCallback lockCallback);
};
}  // namespace opentxs
