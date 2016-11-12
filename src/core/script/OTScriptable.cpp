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

#include "opentxs/core/script/OTScriptable.hpp"

#include "opentxs/core/Log.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/OTStringXML.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/cron/OTCronItem.hpp"
#include "opentxs/core/script/OTAgent.hpp"
#include "opentxs/core/script/OTBylaw.hpp"
#include "opentxs/core/script/OTClause.hpp"
#include "opentxs/core/script/OTParty.hpp"
#include "opentxs/core/script/OTPartyAccount.hpp"
#ifdef OT_USE_SCRIPT_CHAI
#include "opentxs/core/script/OTScriptChai.hpp"
#else
#include "opentxs/core/script/OTScript.hpp"
#endif
#include "opentxs/core/script/OTSmartContract.hpp"
#include "opentxs/core/script/OTVariable.hpp"
#include "opentxs/core/stdafx.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/Tag.hpp"

#ifdef OT_USE_SCRIPT_CHAI
#include <chaiscript/chaiscript.hpp>
#ifdef OT_USE_CHAI_STDLIB
#include <chaiscript/chaiscript_stdlib.hpp>
#endif
#endif
#include <irrxml/irrXML.hpp>
#include <cstdint>
#include <ctype.h>
#include <inttypes.h>
#include <memory>
#include <stdlib.h>
#include <string>
#include <sstream>

// CALLBACKS
//
// The server will call these callbacks, from time to time, and give you the
// opportunity to resolve its questions.

// This script is called by the server, whenever it wants to know whether a
// given party is allowed to execute a specific clause.
//
#ifndef SCRIPTABLE_CALLBACK_PARTY_MAY_EXECUTE
#define SCRIPTABLE_CALLBACK_PARTY_MAY_EXECUTE                                  \
    "callback_party_may_execute_clause"
#endif

namespace opentxs
{

OTScriptable* OTScriptable::InstantiateScriptable(const String& strInput)
{
    static char buf[45] = "";

    if (!strInput.Exists()) {
        otErr << __FUNCTION__ << ": Failure: Input string is empty.\n";
        return nullptr;
    }

    String strContract(strInput);

    if (!strContract.DecodeIfArmored(false)) // bEscapedIsAllowed=true
                                             // by default.
    {
        otErr << __FUNCTION__ << ": Input string apparently was encoded and "
                                 "then failed decoding. Contents: \n"
              << strInput << "\n";
        return nullptr;
    }

    // At this point, strContract contains the actual contents, whether they
    // were originally ascii-armored OR NOT. (And they are also now trimmed,
    // either way.)
    //
    strContract.reset(); // for sgets
    buf[0] = 0;          // probably unnecessary.
    bool bGotLine = strContract.sgets(buf, 40);

    if (!bGotLine) return nullptr;

    OTCronItem* pItem = nullptr;

    String strFirstLine(buf);
    strContract.reset(); // set the "file" pointer within this string back to
                         // index 0.

    // Now I feel pretty safe -- the string I'm examining is within
    // the first 45 characters of the beginning of the contract, and
    // it will NOT contain the escape "- " sequence. From there, if
    // it contains the proper sequence, I will instantiate that type.
    if (!strFirstLine.Exists() || strFirstLine.Contains("- -")) return nullptr;

    // There are actually two factories that load smart contracts. See
    // OTCronItem.
    //
    else if (strFirstLine.Contains(
                 "-----BEGIN SIGNED SMARTCONTRACT-----")) // this string is 36
                                                          // chars long.
    {
        pItem = new OTSmartContract();
        OT_ASSERT(nullptr != pItem);
    }

    // The string didn't match any of the options in the factory.
    if (nullptr == pItem) return nullptr;

    // Does the contract successfully load from the string passed in?
    if (pItem->LoadContractFromString(strContract))
        return pItem;
    else
        delete pItem;

    return nullptr;
}

// virtual
void OTScriptable::SetDisplayLabel(const std::string* pstrLabel)
{
    m_strLabel = (nullptr != pstrLabel) ? pstrLabel->c_str() : "";
}

// VALIDATING IDENTIFIERS IN OTSCRIPTABLE.
//
// Only alphanumerics are valid, or '_' (underscore)
//

bool OTScriptable::is_ot_namechar_invalid(char c)
{
    return !(isalnum(c) || (c == '_'));
}

// static
bool OTScriptable::ValidateName(std::string str_name)
{
    if (str_name.size() <= 0) {
        otErr << "OTScriptable::" << __FUNCTION__ << ": Name has zero size.\n";
        return false;
    }
    else if (find_if(str_name.begin(), str_name.end(),
                       is_ot_namechar_invalid) != str_name.end()) {
        otErr << "OTScriptable::" << __FUNCTION__ << ": Name fails validation testing: "
              << str_name << "\n";
        return false;
    }

    return true;
}

//static
bool OTScriptable::ValidateBylawName(std::string str_name)
{
    if (!ValidateName(str_name))
        return false;

    return true;
}

//static
bool OTScriptable::ValidatePartyName(std::string str_name)
{
    if (!ValidateName(str_name))
        return false;

    return true;
}

//static
bool OTScriptable::ValidateAgentName(std::string str_name)
{
    if (!ValidateName(str_name))
        return false;

    return true;
}

//static
bool OTScriptable::ValidateAccountName(std::string str_name)
{
    if (!ValidateName(str_name))
        return false;

    return true;
}

//static
bool OTScriptable::ValidateVariableName(std::string str_name)
{
    if (!ValidateName(str_name))
        return false;

    // This prefix is disallowed since it's reserved for clause parameter names.
    //
    if (str_name.compare(0, 6, "param_") == 0) {
        otErr << "OTScriptable::" << __FUNCTION__ << ": Invalid variable name ("
        << str_name << "). ('param_' is reserved.)\n";
        return false;
    }
    if (str_name.compare(0, 7, "return_") == 0) {
        otErr << "OTScriptable::" << __FUNCTION__ << ": Invalid variable name ("
        << str_name << "). ('return_' is reserved.)\n";
        return false;
    }

    return true;
}

//static
bool OTScriptable::ValidateClauseName(std::string str_name)
{
    if (!ValidateName(str_name))
        return false;

    // To avoid confusion, we disallow clauses beginning in cron_ or hook_ or
    // callback_
    //
    if (0 == str_name.compare(0, 5, "cron_")) // todo stop hardcoding
    {
        otOut << "OTScriptable::" << __FUNCTION__ << ": Invalid Clause name: '" << str_name << "'. name "
        "should not start with 'cron_'\n";
        return false;
    }

    if (0 == str_name.compare(0, 5, "hook_")) // todo stop hardcoding
    {
        otOut << "OTScriptable::" << __FUNCTION__ << ": Invalid Clause name: '" << str_name << "'. name "
        "should not start with 'hook_'\n";
        return false;
    }

    if (0 == str_name.compare(0, 9, "callback_")) // todo stop hardcoding
    {
        otOut << "OTScriptable::" << __FUNCTION__ << ": Invalid Clause name: '" << str_name << "'. name "
        "should not start with 'callback_'\n";
        return false;
    }

    return true;
}

//static
bool OTScriptable::ValidateHookName(std::string str_name)
{
    if (!ValidateName(str_name))
        return false;

    if ((str_name.compare(0, 5, "cron_") != 0) &&
        (str_name.compare(0, 5, "hook_") != 0))
    {
        otOut << "OTScriptable::" <<__FUNCTION__ << ": Invalid hook name: '"
        << str_name << "'. MUST begin with either 'hook_' or 'cron_'.\n";
        return false;
    }

    return true;
}

//static
bool OTScriptable::ValidateCallbackName(std::string str_name)
{
    if (!ValidateName(str_name))
        return false;

    // If the callback name DOESN'T begin with 'callback_' then it is
    // rejected.
    if (0 != str_name.compare(0, 9, "callback_"))
    {
        otOut << "OTScriptable::" << __FUNCTION__ << ": Invalid Callback name: '" << str_name <<
        "'. MUST begin with 'callback_'.\n";
        return false;
    }

    return true;
}

// OTSmartContract::RegisterOTNativeCallsWithScript OVERRIDES this, but
// also calls it.
//
void OTScriptable::RegisterOTNativeCallsWithScript(
    ANDROID_UNUSED OTScript& theScript)
{
#ifdef OT_USE_SCRIPT_CHAI
    using namespace chaiscript;

    // In the future, this will be polymorphic.
    // But for now, I'm forcing things...

    OTScriptChai* pScript = dynamic_cast<OTScriptChai*>(&theScript);

    if (nullptr != pScript) {
        OT_ASSERT(nullptr != pScript->chai)

        pScript->chai->add(fun(&OTScriptable::GetTime), "get_time");

        pScript->chai->add(fun(&OTScriptable::CanExecuteClause, this),
                           "party_may_execute_clause");
    }
    else
#endif // OT_USE_SCRIPT_CHAI
    {
        otErr << "OTScriptable::RegisterOTNativeCallsWithScript: Failed "
                 "dynamic casting OTScript to OTScriptChai \n";
    }
}

// static
std::string OTScriptable::GetTime() // Returns a string, containing seconds as
                                    // int32_t. (Time in seconds.)
{
    const time64_t CURRENT_TIME = OTTimeGetCurrentTime();
    const int64_t lTime = OTTimeGetSecondsFromTime(CURRENT_TIME);

    String strTime;
    strTime.Format("%" PRId64, lTime);
    return strTime.Get();
}

// The server calls this when it wants to know if a certain party is allowed to
// execute a specific clause.
// This function tries to answer that question by checking for a callback script
// called callback_party_may_execute_clause
// If the callback exists, then it calls that for the answer. Otherwise the
// default return value is: true
// Script coders may also call "party_may_execute_clause()" from within a
// script, which will call this function,
// which will trigger the script callback_party_may_execute_clause(), etc.
//
bool OTScriptable::CanExecuteClause(std::string str_party_name,
                                    std::string str_clause_name)
{
    OTParty* pParty = GetParty(str_party_name);
    OTClause* pClause = GetClause(str_clause_name);

    if (nullptr == pParty) {
        otOut << "OTScriptable::CanExecuteClause: Unable to find this party: "
              << (str_party_name.size() > 0 ? str_party_name.c_str() : "")
              << "\n";
        return false;
    }

    if (nullptr == pClause) {
        otOut << "OTScriptable::CanExecuteClause: Unable to find this clause: "
              << (str_clause_name.size() > 0 ? str_clause_name.c_str() : "")
              << "\n";
        return false;
    }
    // Below this point, pParty and pClause are both good.

    // ...This WILL check to see if pParty has its Opening number verified as
    // issued.
    // (If the opening number is > 0 then VerifyPartyAuthorization() is smart
    // enough to verify it.)
    //
    // To KNOW that a party has the right to even ASK the script to cancel a
    // contract, MEANS that
    // (1) The party is listed as a party on the contract. (2) The party's copy
    // of that contract
    // is signed by the authorizing agent for that party. and (3) The opening
    // transaction number for
    // that party is verified as issued for authorizing agent. (2 and 3 are both
    // performed at the same
    // time, in VerifyPartyAuthorization(), since the agent may need to be
    // loaded in order to verify
    // them.) 1 is already done by this point, as it's performed above.
    //
    // Todo: notice this code appears in CanCancelContract() (this function) as
    // well as
    // OTScriptable::CanExecuteClause.
    // Therefore I can see that THIS VERIFICATION CODE WILL GET CALLED EVERY
    // SINGLE TIME THE SCRIPT
    // CALLS ANY CLAUSE OR OT NATIVE FUNCTION.  Since technically this only
    // needs to be verified before the
    // first call, and not for EVERY call during any of a script's runs, I
    // should probably move this verification
    // higher, such as each time the OTCronItem triggers, plus each time a party
    // triggers a clause directly
    // through the API (server message). As long as those are covered, I will be
    // able to remove it from here
    // which should be a significant improvement for performance.
    // It will be at the bottom of those same functions that
    // "ClearTemporaryPointers()" should finally be called.
    //
    // Also todo:  Need to implement MOVE CONSTRUCTORS and MOVE COPY
    // CONSTRUCTORS all over the place,
    // once I'm sure C++0x build environments are available for all of the
    // various OT platforms. That should
    // be another great performance boost!
    //
    // NOTE (Above):  When it came time to compile, I realized that OTScriptable
    // has no pointer to Cron,
    // nor access to any ServerNym (unless you pass it in).  But I want this
    // function to work by merely
    // passing in 2 strings.
    //
    // SINCE THE PARTY IS VERIFIED ALREADY (WHEN THE SMART CONTRACT IS FIRST
    // ACTIVATED) THEN THIS IS
    // REDUNDANT ANYWAY.
    //
    // IF you ever need to use CanExecuteClause() in some circumstance where the
    // party has NOT been verified
    // anyway (it won't be from within some script...) then just call
    // VerifyPartyAuthorization() yourself in
    // that code, wherever you need to.

    //
    // DISALLOW parties to directly execute any clauses named similarly to
    // callbacks, hooks, or cron hooks!
    // Only allow this for normal clauses.
    //
    if (str_clause_name.compare(0, 5, "cron_") == 0) // todo stop hardcoding
    {
        otOut << "OTScriptable::CanExecuteClause: Parties may not directly "
                 "trigger clauses beginning in cron_\n";
        return false;
    }

    if (str_clause_name.compare(0, 5, "hook_") == 0) // todo stop hardcoding
    {
        otOut << "OTScriptable::CanExecuteClause: Parties may not directly "
                 "trigger clauses beginning in hook_\n";
        return false;
    }

    if (str_clause_name.compare(0, 9, "callback_") == 0) // todo stop hardcoding
    {
        otOut << "OTScriptable::CanExecuteClause: Parties may not directly "
                 "trigger clauses beginning in callback_\n";
        return false;
    }

    // IF NO CALLBACK IS PROVIDED, The default answer to this function is:
    //     YES, this party MAY run this clause!
    //
    // But... first we check to see if this OTScriptable has a clause named:
    //          "callback_party_may_execute_clause"
    // ...and if so, we ask the CALLBACK to make the decision instead. This way,
    // people can define
    // in their own scripts any rules they want about which parties may execute
    // which clauses.

    //
    const std::string str_CallbackName(SCRIPTABLE_CALLBACK_PARTY_MAY_EXECUTE);

    OTClause* pCallbackClause =
        GetCallback(str_CallbackName); // See if there is a script clause
                                       // registered for this callback.

    if (nullptr != pCallbackClause) // Found it!
    {
        otOut << "OTScriptable::CanExecuteClause: Found script for: "
              << SCRIPTABLE_CALLBACK_PARTY_MAY_EXECUTE << ". Asking...\n";

        // The function we're IN defaults to TRUE, if there's no script
        // available.
        // However, if the script is available, then our default return value
        // starts as FALSE.
        // The script itself will then have to set it to true, if that's what it
        // wants.
        //
        OTVariable param1("param_party_name", str_party_name,
                          OTVariable::Var_Constant);
        OTVariable param2("param_clause_name", str_clause_name,
                          OTVariable::Var_Constant);

        OTVariable theReturnVal("return_val", false);

        mapOfVariables theParameters;
        theParameters.insert(
            std::pair<std::string, OTVariable*>("param_party_name", &param1));
        theParameters.insert(
            std::pair<std::string, OTVariable*>("param_clause_name", &param2));

        if (false ==
            ExecuteCallback(
                *pCallbackClause, theParameters,
                theReturnVal)) // <============================================
        {
            otErr << "OTScriptable::CanExecuteClause: Error while running "
                     "callback script " << SCRIPTABLE_CALLBACK_PARTY_MAY_EXECUTE
                  << ", clause " << str_clause_name << " \n";
            return false;
        }
        else {
            otOut << "OTScriptable::CanExecuteClause: Success executing "
                     "callback script " << SCRIPTABLE_CALLBACK_PARTY_MAY_EXECUTE
                  << ", clause: " << str_clause_name << ".\n\n";

            return theReturnVal.CopyValueBool();
        }

    }
    else {
        otOut << "OTScriptable::CanExecuteClause: Unable to find script for: "
              << SCRIPTABLE_CALLBACK_PARTY_MAY_EXECUTE
              << ". Therefore, default return value is: TRUE.\n";
    }

    return true;
}

// Client-side.
// (The server could actually load all the other nyms and accounts,
// and verify their transaction numbers, yadda yadda yadda. But the
// client can only check to see that at least a signed copy is supposedly
// available for every single party.)
//
bool OTScriptable::AllPartiesHaveSupposedlyConfirmed()
{
    bool bReturnVal = !m_mapParties.empty();

    for (auto& it : m_mapParties) {
        OTParty* pParty = it.second;
        OT_ASSERT(nullptr != pParty);

        if (!(pParty->GetMySignedCopy().Exists())) return false;
    }

    return bReturnVal;
}

void OTScriptable::ClearTemporaryPointers()
{
    for (auto& it : m_mapParties) {
        OTParty* pParty = it.second;
        OT_ASSERT(nullptr != pParty);

        pParty->ClearTemporaryPointers();
    }
}

bool OTScriptable::ExecuteCallback(OTClause& theCallbackClause,
                                   mapOfVariables& theParameters,
                                   OTVariable& varReturnVal)
{
    const std::string str_clause_name = theCallbackClause.GetName().Exists()
                                            ? theCallbackClause.GetName().Get()
                                            : "";
    OT_ASSERT(OTScriptable::ValidateName(str_clause_name));

    OTBylaw* pBylaw = theCallbackClause.GetBylaw();
    OT_ASSERT(nullptr != pBylaw);

    // By this point, we have the clause we are executing as theCallbackClause,
    // and we have the Bylaw it belongs to, as pBylaw.

    const std::string str_code =
        theCallbackClause.GetCode(); // source code for the script.
    const std::string str_language =
        pBylaw->GetLanguage(); // language it's in. (Default is "chai")

    std::shared_ptr<OTScript> pScript = OTScriptFactory(str_language, str_code);

    //
    // SET UP THE NATIVE CALLS, REGISTER THE PARTIES, REGISTER THE VARIABLES,
    // AND EXECUTE THE SCRIPT.
    //
    if (pScript) {
        // Register the special server-side native OT calls we make available to
        // all scripts.
        //
        RegisterOTNativeCallsWithScript(*pScript);

        // Register all the parties with the script.
        for (auto& it : m_mapParties) {
            const std::string str_party_name = it.first;
            OTParty* pParty = it.second;
            OT_ASSERT((nullptr != pParty) && (str_party_name.size() > 0));

            pScript->AddParty(str_party_name, *pParty);
        }

        // Add the parameters...
        for (auto& it : theParameters) {
            const std::string str_var_name = it.first;
            OTVariable* pVar = it.second;
            OT_ASSERT((nullptr != pVar) && (str_var_name.size() > 0));

            pVar->RegisterForExecution(*pScript);
        }

        // Also need to loop through the Variables on pBylaw and register those
        // as well.
        //
        pBylaw->RegisterVariablesForExecution(*pScript); // This sets all the
                                                         // variables as CLEAN
                                                         // so we can check for
                                                         // dirtiness after
                                                         // execution.
        //

        SetDisplayLabel(&str_clause_name);

        pScript->SetDisplayFilename(m_strLabel.Get());

        if (!pScript->ExecuteScript(&varReturnVal)) {
            otErr << "OTScriptable::ExecuteCallback: Error while running "
                     "callback on scriptable: " << m_strLabel << "\n";
        }
        else {
            otOut << "OTScriptable::ExecuteCallback: Successfully executed "
                     "callback on scriptable: " << m_strLabel << "\n\n";
            return true;
        }
    }
    else {
        otErr << "OTScriptable::ExecuteCallback: Error instantiating script!\n";
    }

    // NOTE: Normally after calling a script, you want to check to see if any of
    // the persistent variables
    // are dirty, and if important, send a notice to the parties, save an
    // updated copy of the contract, etc.
    // WE DON'T DO THAT FOR CALLBACKS!  Why not?
    //
    // 1) It only matters if the variables change, if you are actually saving an
    // updated version of the contract.
    //    (Which is more OTCronItem / OTSmartContract, which saves an updated
    // copy of itself.) Whereas if you are
    //    NOT saving the contract with those variables in it, then why the hell
    // would you care to notify people?
    // 2) Since only OTCronItem / OTSmartContract actually save updated copies
    // of themselves, they are the only ones
    //    who will ever need to notify anyone. Not EVERY OTScriptable-derived
    // class will send notifications, but if they
    //    need to, SendNoticeToAllParties() is already available on
    // OTScriptable.
    //
    // 3) MOST IMPORTANTLY: the only time a callback is currently triggered is
    // when the script has already been activated
    //    somehow, and the only places that do that ALREADY SEND NOTICES WHEN
    // DIRTY. In fact, if a callback actually makes
    //    the scriptable dirty, IT WILL SEND NOTICES ANYWAY, since the
    // "ExecuteClauses()" function that CALLED the callback
    //    is also smart enough to send the notices already.
    //

    return false;
}

// TODO: Add a "Notice Number" to OTScriptable and OTVotingGroup. This
// increments each
// time a notice is sent to the parties, and will be passed in here as a
// parameter. The nyms
// will all store a map by NotaryID, similar to request #, and for each, a list
// of notice #s
// mapped by the transaction # for each Cron Item the Nym has open. This way the
// Nym can
// expect to see notice #1, notice #2, etc, to make sure he didn't miss one.
// They can even
// have a protocol where each notice contains a hash of the previous one, and
// the users
// (presumably using some future p2p client) can compare hashes with little
// network cost.
// (This prevents the server from sending a false notice to one party, without
// having to
// also falsify all subsequent hashes / notices, since all future hashes will
// now fail to
// match.) The hashes can also be made public if people prefer, as a way of
// "publicly
// posting" the hash of the notice ...without in any way revealing the notice
// contents.

bool OTScriptable::SendNoticeToAllParties(
    bool bSuccessMsg, Nym& theServerNym, const Identifier& theNotaryID,
    const int64_t& lNewTransactionNumber,
    // const int64_t& lInReferenceTo,
    // // Each party has its own opening trans #.
    const String& strReference, String* pstrNote, String* pstrAttachment,
    Nym* pActualNym) const
{
    bool bSuccess =
        true; // Success is defined as ALL parties receiving a notice

    for (auto& it : m_mapParties) {
        OTParty* pParty = it.second;
        OT_ASSERT(nullptr != pParty);

        // If a smart contract is being canceled, it may not have been confirmed
        // by
        // all parties. There may be some unconfirmed parties. (Meaning there is
        // no
        // NymID available for those parties.) So here, we make sure we are only
        // sending
        // the notice to parties which have been confirmed. Those parties will
        // have an
        // opening transaction number that is non-zero. So for parties with a 0
        // opening
        // number, we skip the notice (since there won't be any NymID anyway --
        // nowhere
        // to send the notice even if we tried.)
        //
        if (0 != pParty->GetOpeningTransNo()) {
            if (false ==
                pParty->SendNoticeToParty(
                    bSuccessMsg, // "success" notice? or "failure" notice?
                    theServerNym, theNotaryID, lNewTransactionNumber,
                    //                                                 lInReferenceTo,
                    // // each party has its own opening trans #.
                    strReference, pstrNote, pstrAttachment, pActualNym))
                bSuccess = false; // Notice I don't break here -- I still allow
                                  // it to try to notice ALL parties, even if
                                  // one fails.
        }
    }

    return bSuccess;
}

// So you can tell if any persistent or important variables have CHANGED since
// it was last set clean.
//
bool OTScriptable::IsDirty() const
{
    bool bIsDirty = false;

    for (const auto& it : m_mapBylaws) {
        OTBylaw* pBylaw = it.second;
        OT_ASSERT(nullptr != pBylaw);

        if (pBylaw->IsDirty()) {
            bIsDirty = true;
            break;
        }
    }

    return bIsDirty;
}

// So you can tell if ONLY the IMPORTANT variables have CHANGED since it was
// last set clean.
//
bool OTScriptable::IsDirtyImportant() const
{
    bool bIsDirty = false;

    for (const auto& it : m_mapBylaws) {
        OTBylaw* pBylaw = it.second;
        OT_ASSERT(nullptr != pBylaw);

        if (pBylaw->IsDirtyImportant()) {
            bIsDirty = true;
            break;
        }
    }

    return bIsDirty;
}

// Sets the variables as clean, so you can check later and see if any have been
// changed (if it's DIRTY again.)
//
void OTScriptable::SetAsClean()
{
    for (auto& it : m_mapBylaws) {
        OTBylaw* pBylaw = it.second;
        OT_ASSERT(nullptr != pBylaw);
        // so we can check for dirtiness later, if it's changed.
        pBylaw->SetAsClean();
    }
}

// Note: this maybe would have been more appropriate on OTSmartContract, since
// that is
// where the opening/closing numbers are actually USED. But still, they ARE
// stored HERE,
// so I might as well put the function here also. That way later on, if some new
// subclass
// uses those numbers, it will have access to count them as well.
//
// Returns 0 if this agent is not the authorizing agent for a party, and is also
// not the
// authorized agent for any party's accounts.
//
int32_t OTScriptable::GetCountTransNumsNeededForAgent(
    std::string str_agent_name) const
{
    int32_t nReturnVal = 0;

    OTAgent* pAgent = GetAgent(str_agent_name);
    if (nullptr == pAgent)
        return nReturnVal; // (Looks like there is no agent with that name.)

    // Below this point, pAgent is good, meaning str_agent_name really IS
    // a legit agent for this party. But that doesn't necessarily mean the
    // agent has to supply any opening or closing transaction #s for this
    // smart contract. That's only true if he's the AUTHORIZING agent for
    // the party (for the opening num) or the authorized agent for any of
    // party's accounts (for the closing number).  So let's add it up...
    //
    if (pAgent->IsAuthorizingAgentForParty()) // true/false whether THIS agent
                                              // is the authorizing agent for
                                              // his party.
        nReturnVal++;

    // Add the number of accounts, owned by this agent's party, that this agent
    // is the authorized agent FOR.
    //
    nReturnVal += pAgent->GetCountAuthorizedAccts();

    return nReturnVal;
}

OTPartyAccount* OTScriptable::GetPartyAccount(std::string str_acct_name) const
{
    if (!OTScriptable::ValidateName(str_acct_name)) // this logs, FYI.
    {
        otErr << "OTScriptable::GetPartyAccount:  Error: invalid name.\n";
        return nullptr;
    }

    for (auto& it : m_mapParties) {
        OTParty* pParty = it.second;
        OT_ASSERT(nullptr != pParty);
        OTPartyAccount* pAcct = pParty->GetAccount(str_acct_name);
        if (nullptr != pAcct) // found it.
            return pAcct;
    }
    return nullptr;
}

OTPartyAccount* OTScriptable::GetPartyAccountByID(
    const Identifier& theAcctID) const
{
    for (const auto& it : m_mapParties) {
        OTParty* pParty = it.second;
        OT_ASSERT(nullptr != pParty);

        OTPartyAccount* pAcct = pParty->GetAccountByID(theAcctID);

        if (nullptr != pAcct) // found it.
            return pAcct;
    }

    return nullptr;
}

OTParty* OTScriptable::FindPartyBasedOnNymIDAsAgent(const Identifier& theNymID,
                                                    OTAgent** ppAgent) const
{
    for (const auto& it : m_mapParties) {
        OTParty* pParty = it.second;
        OT_ASSERT(nullptr != pParty);

        if (pParty->HasAgentByNymID(theNymID, ppAgent)) return pParty;
    }
    return nullptr;
}

OTParty* OTScriptable::FindPartyBasedOnNymIDAsAuthAgent(
    const Identifier& theNymID, OTAgent** ppAgent) const
{
    for (auto& it : m_mapParties) {
        OTParty* pParty = it.second;
        OT_ASSERT(nullptr != pParty);

        if (pParty->HasAuthorizingAgentByNymID(theNymID, ppAgent))
            return pParty;
    }
    return nullptr;
}

OTParty* OTScriptable::FindPartyBasedOnAccountID(
    const Identifier& theAcctID, OTPartyAccount** ppPartyAccount) const
{
    for (auto& it : m_mapParties) {
        OTParty* pParty = it.second;
        OT_ASSERT(nullptr != pParty);

        if (pParty->HasAccountByID(theAcctID, ppPartyAccount)) {
            return pParty;
        }
    }
    return nullptr;
}

OTParty* OTScriptable::FindPartyBasedOnNymAsAgent(Nym& theNym,
                                                  OTAgent** ppAgent) const
{
    for (auto& it : m_mapParties) {
        OTParty* pParty = it.second;
        OT_ASSERT(nullptr != pParty);

        if (pParty->HasAgent(theNym, ppAgent)) return pParty;
    }
    return nullptr;
}

OTParty* OTScriptable::FindPartyBasedOnNymAsAuthAgent(Nym& theNym,
                                                      OTAgent** ppAgent) const
{
    for (auto& it : m_mapParties) {
        OTParty* pParty = it.second;
        OT_ASSERT(nullptr != pParty);

        if (pParty->HasAuthorizingAgent(theNym, ppAgent)) return pParty;
    }
    return nullptr;
}

OTParty* OTScriptable::FindPartyBasedOnAccount(
    Account& theAccount, OTPartyAccount** ppPartyAccount) const
{
    for (auto& it : m_mapParties) {
        OTParty* pParty = it.second;
        OT_ASSERT(nullptr != pParty);

        if (pParty->HasAccount(theAccount, ppPartyAccount)) {
            return pParty;
        }
    }
    return nullptr;
}

void OTScriptable::RetrieveNymPointers(mapOfNyms& map_Nyms_Already_Loaded)
{
    for (auto& it : m_mapParties) {
        OTParty* pParty = it.second;
        OT_ASSERT(nullptr != pParty);

        pParty->RetrieveNymPointers(map_Nyms_Already_Loaded);
    }
}

/*
 -- Load up the authorizing agent's Nym, if not already loaded. (Why? To
 verifySignature. Also, just to have
 it loaded so I don't have to load it twice in case it's needed for verifying
 one/some of the accts.) So really:
 -- Verify each party, that the authorizing agent and signature are all good. (I
 think I have this already...)
 -- Definitely during this, need to make sure that the contents of the signed
 version match the contents of the main version, for each signer.
 -- Verify that the authorizing agent actually has the opening transaction # for
 the party issued to him. (Do I have this?....)
 */

// OTScriptable::VerifyPartyAuthorization
// Similar to VerifyNymAsAgent, except it doesn't ClearTemporaryPointers.
// (If it has to Load the authorizing agent, then it will clear that one.)
// It also doesn't START with a Nym, per se, but rather starts from the Party.
// Though it still allows the possibility that the Nym is already loaded.
// I basically wrote RetrieveNymPointers() (above) so I could call it in here.
// This function is for cases where the Nym very well may NOT be loaded yet,
// BUT THAT WHETHER IT IS, OR NOT, IT *DEFINITELY* DOESN'T CLEAR OUT THE ONES
// THAT *ARE* THERE OTHER THAN WHICHEVER ONES IT IS FORCED TO LOAD ITSELF.
// (This is in contrast to VerifyNymAsAgent(), which calls
// ClearTemporaryPointers()
// religiously.)
//
// This function also verifies the opening transactions # for the party, whereas
// VerifyNymAsAgent doesn't. ACTUALLY WAIT -- There IS no opening transaction #
// except for OTCronItem-derived objects (OTSmartContract for example...)
// So perhaps that will have to go OUTSIDE of this function after all!
// Therefore I'll make a separate function for that that I can call along with
// this one. (Like how I do in OTSmartContract::CanCancelContract)
//
// No again -- Because it's in HERE that I actually have the authorizing agent
// loaded. I certainly don't want to just have to load him up again. Therefore I
// have to use a new rule:  IN THIS FUNCTION, if the party HAS an Opening
// Number,
// THEN IT MUST VERIFY. Since the default value is 0, then that should work,
// since
// I'm always verifying the number as long as one is there.
//
bool OTScriptable::VerifyPartyAuthorization(
    OTParty& theParty, // The party that supposedly is authorized for this
                       // supposedly executed agreement.
    Nym& theSignerNym, // For verifying signature on the authorizing
                       // Nym, when loading it
    const String& strNotaryID, // For verifying issued num, need the notaryID
                               // the # goes with.
    mapOfNyms* pmap_ALREADY_LOADED, // If some nyms are already
                                    // loaded, pass them here so we
                                    // don't load them twice on
                                    // accident.
    mapOfNyms* pmap_NEWLY_LOADED,   // If some nyms had to be loaded,
                                    // then they will be deleted, too.
                                    // UNLESS you pass a map here, in
                                    // which case they will instead be
    // added to this map. (But if you do
    // that, then you must delete them
    // yourself after calling this
    // function.)
    const bool bBurnTransNo) // In OTServer::VerifySmartContract(),
                             // it not only wants to verify the # is
                             // properly issued, but it additionally
                             // wants to see that it hasn't been USED
                             // yet -- AND it wants to burn it, so it
                             // can't be used again!  This bool
                             // allows you to tell the function
                             // whether or not to do that.
{
    // This function DOES assume that theParty was initially FOUND on
    // OTScriptable.
    // Meaning I don't have to verify that much if I got this far.

    const bool bNeedToCleanup = (nullptr == pmap_NEWLY_LOADED) ? true : false;

    // This party hasn't signed the contract??
    //
    if (!theParty.GetMySignedCopy().Exists()) {
        otOut << __FUNCTION__ << ": Unable to find party's signed copy of this "
                                 "contract. Has it been executed?\n";
        return false;
    }

    // By this point, we know that theParty has a signed copy of the agreement
    // (or of SOMETHING anyway) and we know that
    // we were able to find the party based on theNym as one of its agents. But
    // the signature still needs to be verified...

    // (2)
    // This step verifies that the party has been signed by its authorizing
    // agent. (Who may not be the Nym, yet might be.)
    //
    OTAgent* pAuthorizingAgent = nullptr;
    Nym* pAuthAgentsNym = nullptr;
    // In case I have to load it myself, I want it cleaned up properly.
    std::unique_ptr<Nym> theAgentNymAngel;

    // Some nyms are *already* loaded. If any were passed in, let's see if any
    // of them are
    // the authorizing agent for the contract (for this party)...
    //
    if (nullptr != pmap_ALREADY_LOADED) {
        // (So we don't load it twice, if it's there.)
        //
        mapOfNyms& map_Nyms_Already_Loaded = (*pmap_ALREADY_LOADED);

        for (auto& it : map_Nyms_Already_Loaded) {
            Nym* pNym = it.second;
            OT_ASSERT(nullptr != pNym);

            if (theParty.HasAuthorizingAgent(*pNym, &pAuthorizingAgent)) {
                pAuthAgentsNym = pNym; // Just in case
                break;
            }
        }
        // if pAuthorizingAgent != nullptr, that means the authorizing agent was
        // found among the map of nyms that were already loaded.
    }

    // Only if I had to load the authorizing agent myself, do I clear its
    // temporary pointer.
    // (Because it's going out of scope at the bottom of this function.)
    // Any other temporary pointers, I do NOT clear, but I leave them because a
    // script is
    // probably executing and may still need them.
    // (This function that we're in was expressly called by someone who didn't
    // want the
    // temporary nym pointers cleared just yet, for whatever reason.)
    //
    bool bHadToLoadItMyself = (nullptr == pAuthorizingAgent) ? true : false;

    // Still not found? Okay, let's LOAD IT UP then...
    //
    if (bHadToLoadItMyself) {
        // Of all of a party's Agents, the "authorizing agent" is the one who
        // originally activated
        // the agreement for this party (and fronted the opening trans#.) Since
        // we need to verify his
        // signature, we have to load him up.
        //
        pAuthAgentsNym =
            theParty.LoadAuthorizingAgentNym(theSignerNym, &pAuthorizingAgent);

        if (nullptr != pAuthAgentsNym) // success
        {
            OT_ASSERT(nullptr !=
                      pAuthorizingAgent); // This HAS to be set now. I
                                          // assume it henceforth.
            otLog3 << __FUNCTION__
                   << ": I just had to load "
                      "the authorizing agent's Nym for a party ("
                   << theParty.GetPartyName()
                   << "), "
                      "so I guess it wasn't already available on the list of "
                      "Nyms that were already loaded.\n";

            // Either I'm DEFINITELY cleaning it up myself, OR I'm adding it to
            // a list
            // where the CALLER can clean it up.
            //
            if (bNeedToCleanup)
                theAgentNymAngel.reset(pAuthAgentsNym); // CLEANUP!!
            else {
                const std::string str_agent_name =
                    pAuthorizingAgent->GetName().Get();

                mapOfNyms& map_Nyms_Newly_Loaded = (*pmap_NEWLY_LOADED);
                map_Nyms_Newly_Loaded.insert(
                    map_Nyms_Newly_Loaded.begin(),
                    std::pair<std::string, Nym*>(
                        str_agent_name,
                        pAuthAgentsNym)); // (Caller must clean these up.)
            }
        }
        else {
            otErr << __FUNCTION__ << ": Error: Strange, unable to load "
                                     "authorizing agent's Nym (to verify his "
                                     "signature.)\n";
            return false;
        }
    }

    // Below this point, we KNOW that pAuthorizingAgent is a good pointer and
    // will be cleaned up properly/automatically.
    // I'm not using pAuthAgentsNym directly, but pAuthorizingAgent WILL use it
    // before this function is done.

    // (3) Verify the issued number, if he has one. If this instance is
    // OTScriptable-derived, but NOT OTCronItem-derived,
    //     then that means there IS NO opening number (or closing number) since
    // we're not even on Cron. The parties just
    //     happen to store their opening number for the cases where we ARE on
    // Cron, which will probably be most cases.
    //     Therefore we CHECK TO SEE if the opening number is NONZERO -- and if
    // so, we VERIFY ISSUED on that #. That way,
    //     for cases where this IS a cron item, it will still verify the number
    // (as it should) and in other cases, it will
    //     just skip this step.
    //

    const int64_t lOpeningNo = theParty.GetOpeningTransNo();

    if (lOpeningNo > 0) // If one exists, then verify it.
    {
        if (false ==
            pAuthorizingAgent->VerifyIssuedNumber(lOpeningNo, strNotaryID)) {
            otErr << __FUNCTION__ << ": Opening trans number " << lOpeningNo
                  << " doesn't "
                     "verify for the nym listed as the authorizing agent for "
                     "party " << theParty.GetPartyName() << ".\n";
            if (bHadToLoadItMyself && bNeedToCleanup)
                pAuthorizingAgent->ClearTemporaryPointers(); // We loaded the
                                                             // Nym ourselves,
                                                             // which goes out
                                                             // of scope after
                                                             // this function.
            return false;
        }

        // The caller wants the additional verification that the number hasn't
        // been USED
        // yet -- AND the caller wants you to BURN IT HERE.
        else if (bBurnTransNo) {
            if (false ==
                pAuthorizingAgent->VerifyTransactionNumber(lOpeningNo,
                                                           strNotaryID)) {
                otErr << __FUNCTION__ << ": Opening trans number " << lOpeningNo
                      << " doesn't "
                         "verify as available for use, for the "
                         "nym listed as the authorizing agent "
                         "for party: " << theParty.GetPartyName() << ".\n";
                if (bHadToLoadItMyself && bNeedToCleanup)
                    pAuthorizingAgent->ClearTemporaryPointers(); // We loaded
                                                                 // the Nym
                                                                 // ourselves,
                                                                 // which goes
                                                                 // out of scope
                                                                 // after this
                                                                 // function.
                return false;
            }
            else // SUCCESS -- It verified as available, so let's burn it
                   // here. (So he can't use it twice. It remains issued and
                   // open until the cron item is eventually closed out for
                   // good.)
            {
                // This function also adds lOpeningNo to the agent's nym's list
                // of open cron items.
                // (OTPseudonym::GetSetOpenCronItems)
                //
                pAuthorizingAgent->RemoveTransactionNumber(
                    lOpeningNo, strNotaryID, theSignerNym, true); // bSave=true
            }
        }

    }                      // if lOpeningNo>0
    else if (bBurnTransNo) // In this case, bBurnTransNo=true, then the caller
                           // EXPECTED to burn a transaction
    {                      // num. But the number was 0! Therefore, FAILURE!
        otOut << __FUNCTION__ << ": FAILURE. On Party "
              << theParty.GetPartyName().c_str()
              << ", expected to burn a legitimate opening transaction "
                 "number, but got this instead: " << lOpeningNo << "\n";
        if (bHadToLoadItMyself && bNeedToCleanup)
            pAuthorizingAgent->ClearTemporaryPointers(); // We loaded the Nym
                                                         // ourselves, which
                                                         // goes out of scope
                                                         // after this function.
        return false;
    }

    // (4)
    // Here, we use the Authorizing Agent to verify the signature on his party's
    // version of the contract.
    // Notice: Even if the authorizing agent gets fired, we can still load his
    // Nym to verify the original signature on the
    // original contract! We should ALWAYS be able to verify our signatures!
    // Therefore, TODO: When a Nym is DELETED, it's necessary
    // to KEEP the public key on file. We may not have a Nymfile with any
    // request #s or trans# signed out, and there are may be no
    // accounts for him, but we still need that public key, for later
    // verification of the signature.
    // UNLESS... the public key ITSELF is stashed into the contract... Notice
    // that normal asset contracts already keep the keys inside,
    // so why shouldn't these? In fact I can pop the "key" value onto the
    // contract as part of the "Party-Signing" API call. Just grab the
    // public key from each one. Meanwhile the server can verify that it's
    // actually there, and refuse to process without it!!  Nice.
    // This also shows why we need to store the NymID, even if it can be
    // overridden by a Role: because you want to be able to verify
    // the original signature, no matter WHO is authorized now. Otherwise your
    // entire contract falls apart.

    OTScriptable* pPartySignedCopy =
        OTScriptable::InstantiateScriptable(theParty.GetMySignedCopy());
    std::unique_ptr<OTScriptable> theCopyAngel;

    if (nullptr == pPartySignedCopy) {
        otErr << __FUNCTION__ << ": Error loading party's signed copy of "
                                 "agreement. Has it been executed?\n";
        if (bHadToLoadItMyself && bNeedToCleanup)
            pAuthorizingAgent->ClearTemporaryPointers();
        return false;
    }
    else {
        theCopyAngel.reset(pPartySignedCopy);
    }

    const bool bSigVerified =
        pAuthorizingAgent->VerifySignature(*pPartySignedCopy);
    bool bContentsVerified = false;

    if (bSigVerified) {
        // Todo OPTIMIZE: Might move this call to a higher layer, so it gets
        // called MUCH less often but retains the same security.
        // There are several places currently in smart contracts like this.
        // Need to analyze security aspects before doing it.
        //
        bContentsVerified =
            Compare(*pPartySignedCopy); // This also compares the opening
                                        // / closing numbers, if they are
                                        // non-zero.

        if (!bContentsVerified)
            otOut << __FUNCTION__
                  << ": Though the signature verifies, the contract "
                     "signed by the party (" << theParty.GetPartyName()
                  << ") doesn't match this contract. (Failed comparison.)\n";
    }
    else
        otOut << __FUNCTION__ << ": Signature failed to verify for party: "
              << theParty.GetPartyName() << " \n";

    if (bHadToLoadItMyself && bNeedToCleanup)
        pAuthorizingAgent->ClearTemporaryPointers(); // We loaded the Nym
                                                     // ourselves, which goes
                                                     // out of scope after this
                                                     // function.
    // The party is done with it now, and we don't want it to keep pointing to
    // something that is now going out of scope.

    return bContentsVerified;
}

// Kind of replaces VerifySignature() in certain places. It still verifies
// signatures inside, but
// OTTrade and OTAgreement have a much simpler way of doing that than
// OTScriptable/OTSmartContract.
//
// This function also loads its own versions of certain nyms, when necessary,
// and cleans the pointers
// when it's done.
//
bool OTScriptable::VerifyNymAsAgent(Nym& theNym, Nym& theSignerNym,
                                    mapOfNyms* pmap_ALREADY_LOADED) const
{
    // (COmmented out) existing trades / payment plans on OT basically just have
    // this one line:
    //
    // VerifySignature(theNym)

    // NEW VERSION:
    /*
     Proves the original party DOES approve of Nym:

     1) Lookup the party for this Nym, (loop to see if Nym is listed as an agent
     by any party.)
     2) If party found, lookup authorizing agent for party, loading him if
     necessary.
     3) Use authorizing agent to verify signature on original copy of this
     contract. Each party stores their
        own copy of the agreement, therefore use that copy to verify signature,
     instead of the current version.

     This proves that the original authorizing agent for the party that lists
     Nym as an agent HAS signed the smart contract.
     (As long as that same party OWNS the account, that should be fine.)
     Obviously the best proof is to verify the ORIGINAL version of the contract
     against the PARTY'S ORIGINAL SIGNED COPY,
     and also to verify them AGAINST EACH OTHER.  The calling function should do
     this. Otherwise what if the "authorized signer"
     has been changed, and some new guy and signature are substituted? Well, I
     guess as long as he really is authorized... but
     you simply don't know what the original agreement really says unless you
     look at it.  So load it and use it to call THIS METHOD.
     */

    // (1)
    // This step verifies that theNym is at least REGISTERED as a valid agent
    // for the party. (According to the party.)
    //
    OTParty* pParty = FindPartyBasedOnNymAsAgent(theNym);

    if (nullptr == pParty) {
        otOut << "OTScriptable::VerifyNymAsAgent: Unable to find party based "
                 "on Nym as agent.\n";
        return false;
    }
    // Below this point, pParty is good.

    // This party hasn't signed the contract??
    //
    if (!pParty->GetMySignedCopy().Exists()) {
        otOut << "OTScriptable::VerifyNymAsAgent: Unable to find party's ("
              << pParty->GetPartyName()
              << ") signed copy of this contract. Has it been executed?\n";
        return false;
    }

    // By this point, we know that pParty has a signed copy of the agreement (or
    // of SOMETHING anyway) and we know that
    // we were able to find the party based on theNym as one of its agents. But
    // the signature still needs to be verified...

    // (2)
    // This step verifies that the party has been signed by its authorizing
    // agent. (Who may not be the Nym, yet might be.)
    //
    OTAgent* pAuthorizingAgent = nullptr;
    Nym* pAuthAgentsNym = nullptr;
    std::unique_ptr<Nym> theAgentNymAngel;

    // See if theNym is the authorizing agent.
    //
    if ((false == pParty->HasAuthorizingAgent(theNym, &pAuthorizingAgent)) &&
        (nullptr != pmap_ALREADY_LOADED)) {
        // No? Okay then, let's see if the authorizing agent is one of these
        // already loaded.
        // (So we don't load it twice.)
        //
        mapOfNyms& map_Nyms_Already_Loaded = (*pmap_ALREADY_LOADED);
        for (auto& it : map_Nyms_Already_Loaded) {
            Nym* pNym = it.second;
            OT_ASSERT(nullptr != pNym);

            if (pParty->HasAuthorizingAgent(*pNym, &pAuthorizingAgent)) {
                pAuthAgentsNym = pNym; // Just in case.
                break;
            }
        }
        // if pAuthorizingAgent != nullptr, that means the authorizing agent was
        // found among the map of nyms that were already loaded.
    }

    // Still not found?
    if (nullptr == pAuthorizingAgent) {
        // Of all of a party's Agents, the "authorizing agent" is the one who
        // originally activated
        // the agreement for this party (and fronted the opening trans#.) Since
        // we need to verify his
        // signature, we have to load him up.
        //
        pAuthAgentsNym =
            pParty->LoadAuthorizingAgentNym(theSignerNym, &pAuthorizingAgent);

        if (nullptr != pAuthAgentsNym) // success
        {
            OT_ASSERT(nullptr !=
                      pAuthorizingAgent); // This HAS to be set now. I
                                          // assume it henceforth.
            otLog3 << "OTScriptable::VerifyNymAsAgent: I just had to load the "
                      "authorizing agent's Nym for a party ("
                   << pParty->GetPartyName()
                   << "), so I guess it wasn't already available "
                      "on the list of Nyms that were already loaded.\n";
            theAgentNymAngel.reset(pAuthAgentsNym); // CLEANUP!!
        }
        else {
            otErr << "OTScriptable::VerifyNymAsAgent: Error: Strange, unable "
                     "to load authorizing "
                     "agent's Nym for party " << pParty->GetPartyName()
                  << " (to verify his signature.)\n";
            pParty->ClearTemporaryPointers();
            return false;
        }
    }

    // Below this point, we KNOW that pAuthorizingAgent is a good pointer and
    // will be cleaned up properly/automatically.
    // I'm not using pAuthAgentsNym directly, but pAuthorizingAgent WILL use it
    // before this function is done.

    // TODO: Verify the opening transaction # here, like I do in
    // VerifyPartyAuthorization() ??  Research first.

    // (3)
    // Here, we use the Authorizing Agent to verify the signature on his party's
    // version of the contract.
    // Notice: Even if the authorizing agent gets fired, we can still load his
    // Nym to verify the original signature on the
    // original contract! We should ALWAYS be able to verify our signatures!
    // Therefore, TODO: When a Nym is DELETED, it's necessary
    // to KEEP the public key on file. We may not have a Nymfile with any
    // request #s or trans# signed out, and there are may be no
    // accounts for him, but we still need that public key, for later
    // verification of the signature.
    // UNLESS... the public key ITSELF is stashed into the contract... Notice
    // that normal asset contracts already keep the keys inside,
    // so why shouldn't these? In fact I can pop the "key" value onto the
    // contract as part of the "Party-Signing" API call. Just grab the
    // public key from each one. Meanwhile the server can verify that it's
    // actually there, and refuse to process without it!!  Nice.
    // This also shows why we need to store the NymID, even if it can be
    // overridden by a Role: because you want to be able to verify
    // the original signature, no matter WHO is authorized now. Otherwise your
    // entire contract falls apart.

    OTScriptable* pPartySignedCopy =
        OTScriptable::InstantiateScriptable(pParty->GetMySignedCopy());
    std::unique_ptr<OTScriptable> theCopyAngel;

    if (nullptr == pPartySignedCopy) {
        otErr << "OTScriptable::VerifyNymAsAgent: Error loading party's ("
              << pParty->GetPartyName()
              << ") signed copy of agreement. Has it been executed?\n";
        pParty->ClearTemporaryPointers();
        return false;
    }
    else
        theCopyAngel.reset(pPartySignedCopy);

    const bool bSigVerified =
        pAuthorizingAgent->VerifySignature(*pPartySignedCopy);
    bool bContentsVerified = false;

    if (bSigVerified) {
        // Todo OPTIMIZE: Might move this call to a higher layer, so it gets
        // called MUCH less often but retains the same security.
        // There are several places currently in smart contracts like this.
        // Need to analyze security aspects before doing it.
        //
        bContentsVerified = Compare(*pPartySignedCopy);

        if (!bContentsVerified)
            otOut << "OTScriptable::VerifyNymAsAgent: Though the signature "
                     "verifies, the contract "
                     "signed by the party (" << pParty->GetPartyName()
                  << ") doesn't match this contract. (Failed comparison.)\n";
    }
    else
        otOut << "OTScriptable::VerifyNymAsAgent: Signature failed to verify "
                 "for party: " << pParty->GetPartyName() << " \n";

    // Todo: possibly call Compare(*pPartySignedCopy); to make sure
    // there's no funny business.
    // Well actually that HAS to happen anyway, it's just a question of whether
    // it goes here too, or only somewhere else.

    pParty->ClearTemporaryPointers(); // We loaded a Nym ourselves, which goes
                                      // out of scope after this function. The
                                      // party is done
    // with it now, and we don't want it to keep pointing to something that is
    // now going out of scope.

    return bContentsVerified;
}

// Call VerifyPartyAuthorization() first.
// Also, this function, unlike VerifyPartyAuthorization(), is able to ASSUME
// that ALL
// of the Nyms AND accounts have ALREADY been loaded into memory, AND that *this
// has
// pointers to them. That is all prepared before this function is called. That
// is why
// you don't see me passing in maps to Nyms that are already loaded -- because
// *this
// already has them all loaded.
//
// This function verifies ownership AND agency of the account, and it
// also handles closing transaction numbers when appropriate.
//
bool OTScriptable::VerifyPartyAcctAuthorization(
    OTPartyAccount& thePartyAcct, // The party is assumed to have been verified
                                  // already via VerifyPartyAuthorization()
    Nym& theSignerNym,            // For verifying signature on the authorizing
                                  // Nym.
    const String& strNotaryID,    // For verifying issued num, need the notaryID
                                  // the # goes with.
    const bool bBurnTransNo)      // In OTServer::VerifySmartContract(),
                                  // it not only wants to verify the
                                  // closing # is properly issued, but it
                                  // additionally wants to see that it
                                  // hasn't been USED yet -- AND it wants
                                  // to burn it, so it can't be used
                                  // again!  This bool allows you to tell
                                  // the function whether or not to do
                                  // that.
{
    OTParty* pParty = thePartyAcct.GetParty();

    if (nullptr == pParty) {
        otErr << "OTScriptable::" << __FUNCTION__
              << ": Unable to find party for acct: " << thePartyAcct.GetName()
              << " \n";
        return false;
    }

    OTAgent* pAuthorizedAgent = thePartyAcct.GetAuthorizedAgent();

    if (nullptr == pAuthorizedAgent) {
        otOut << "OTScriptable::" << __FUNCTION__
              << ": Unable to find authorized agent ("
              << thePartyAcct.GetAgentName()
              << ") for acct: " << thePartyAcct.GetName() << " \n";
        return false;
    }

    // BELOW THIS POINT, pParty and pAuthorizedAgent are both good pointers.
    //
    // Next, we need to verify that pParty is the proper OWNER of thePartyAcct..
    //
    // AND that pAuthorizedAgent really IS authorized to manipulate the actual
    // account itself. (Either he is listed as its actual owner, or he is an
    // agent for an entity, authorized based on a specific role, and the account
    // is owned by that entity / controlled by that role.)
    //

    // VERIFY ACCOUNT's OWNERSHIP BY PARTY
    //
    if (!thePartyAcct.VerifyOwnership()) // This will use pParty internally.
    {
        otOut << "OTScriptable::" << __FUNCTION__
              << ": Unable to verify party's (" << pParty->GetPartyName()
              << ") ownership of acct: " << thePartyAcct.GetName() << " \n";
        return false;
    }

    // VERIFY ACCOUNT's AUTHORIZED AGENT (that he has rights to manipulate the
    // account itself)
    //
    if (!thePartyAcct.VerifyAgency()) // This will use pAuthorizedAgent
                                      // internally.
    {
        otOut << "OTScriptable::" << __FUNCTION__
              << ": Unable to verify agent's ("
              << pAuthorizedAgent->GetName().Get()
              << ") rights re: acct: " << thePartyAcct.GetName() << " \n";
        return false;
    }

    //     Verify the closing number, if he has one. (If this instance is
    // OTScriptable-derived, but NOT OTCronItem-derived,
    //     then that means there IS NO closing number, since we're not even on
    // Cron.) The PartyAccts just happen
    //     to store their closing number for the cases where we ARE on Cron,
    // which will probably be most cases.
    //     Therefore we CHECK TO SEE if the closing number is NONZERO -- and if
    // so, we VERIFY ISSUED on that #. That way,
    //     for cases where this IS a cron item, it will still verify the number
    // (as it should) and in other cases, it will
    //     just skip this step.
    //
    //     Also: If bBurnTransNo is set to true, then it will force this issue,
    // since the code then DEMANDS a number
    //     be available for use.

    const int64_t lClosingNo = thePartyAcct.GetClosingTransNo();

    if (lClosingNo > 0) // If one exists, then verify it.
    {
        if (false ==
            pAuthorizedAgent->VerifyIssuedNumber(lClosingNo, strNotaryID)) {
            otOut << "OTScriptable::" << __FUNCTION__
                  << ": Closing trans number " << lClosingNo
                  << " doesn't "
                     "verify for the nym listed as the authorized agent for "
                     "account " << thePartyAcct.GetName() << ".\n";
            return false;
        }

        // The caller wants the additional verification that the number hasn't
        // been USED
        // yet -- AND the caller wants you to BURN IT HERE.
        //
        else if (bBurnTransNo) {
            if (false ==
                pAuthorizedAgent->VerifyTransactionNumber(lClosingNo,
                                                          strNotaryID)) {
                otOut << "OTScriptable::" << __FUNCTION__
                      << ": Closing trans number " << lClosingNo
                      << " doesn't "
                         "verify as available for use, for the "
                         "nym listed as the authorized agent for "
                         "acct: " << thePartyAcct.GetName() << ".\n";
                return false;
            }
            else // SUCCESS -- It verified as available, so let's burn it
                   // here. (So he can't use it twice. It remains
            { // issued and open until the cron item is eventually closed out
                // for good.)
                //
                // NOTE: This also adds lClosingNo to the agent's nym's
                // GetSetOpenCronItems.
                //
                pAuthorizedAgent->RemoveTransactionNumber(
                    lClosingNo, strNotaryID, theSignerNym, true); // bSave=true
            }
        }

    }                      // if lClosingNo>0
    else if (bBurnTransNo) // In this case, bBurnTransNo=true, then the caller
                           // EXPECTED to burn a transaction
    {                      // num. But the number was 0! Therefore, FAILURE!
        otOut << "OTScriptable::" << __FUNCTION__ << ": FAILURE. On Acct "
              << thePartyAcct.GetName()
              << ", expected to burn a legitimate closing transaction "
                 "number, but got this instead: " << lClosingNo << "\n";
        return false;
    }

    return true;
}

// Wherever you call the below function, you need to call the above function
// too, and make sure the same
// Party is the one found in both cases.
//
// AGAIN: CALL VerifyNymAsAgent() BEFORE you call this function! Otherwise you
// aren't proving nearly as much. ALWAYS call it first.
//
bool OTScriptable::VerifyNymAsAgentForAccount(Nym& theNym,
                                              Account& theAccount) const
{

    // Lookup the party via the ACCOUNT.
    //
    OTPartyAccount* pPartyAcct = nullptr;
    OTParty* pParty = FindPartyBasedOnAccount(theAccount, &pPartyAcct);

    if (nullptr == pParty) {
        OT_ASSERT(nullptr != pPartyAcct);
        otOut << "OTScriptable::VerifyNymAsAgentForAccount: Unable to find "
                 "party based on account.\n";
        return false;
    }
    // pPartyAcct is a good pointer below this point.

    // Verify ownership.
    //
    if (!pParty->VerifyOwnershipOfAccount(theAccount)) {
        otOut << "OTScriptable::VerifyNymAsAgentForAccount: pParty is not the "
                 "owner of theAccount.\n";
        pParty->ClearTemporaryPointers(); // Just in case.
        return false;
    }

    const std::string str_acct_agent_name = pPartyAcct->GetAgentName().Get();
    OTAgent* pAgent = pParty->GetAgent(str_acct_agent_name);

    // Make sure they are from the SAME PARTY.
    //
    if (nullptr == pAgent) {
        otOut << "OTScriptable::VerifyNymAsAgentForAccount: Unable to find the "
                 "right agent for this account.\n";
        pParty->ClearTemporaryPointers(); // Just in case.
        return false;
    }
    // Below this point, pPartyAcct is a good pointer, and so is pParty, as well
    // as pAgent
    // We also know that the agent's name is the same one that was listed on the
    // account as authorized.
    // (Because we used the account's agent_name to look up pAgent in the first
    // place.)

    // Make sure theNym is a valid signer for pAgent, whether representing
    // himself as in individual, or in a role for an entity.
    // This means that pAgent (looked up based on partyAcct's agent name) will
    // return true to IsValidSigner when passed theNym,
    // assuming theNym really is the agent that partyAcct was talking about.
    //
    if (!pAgent->IsValidSigner(theNym)) {
        otOut << "OTScriptable::VerifyNymAsAgentForAccount: theNym is not a "
                 "valid signer for pAgent.\n";
        pParty->ClearTemporaryPointers(); // Just in case.
        return false;
    }
    if (!pAgent->VerifyAgencyOfAccount(theAccount)) {
        otOut << "OTScriptable::VerifyNymAsAgentForAccount: theNym is not a "
                 "valid agent for theAccount.\n";
        pParty->ClearTemporaryPointers(); // Just in case.
        return false;
    }

    //    Now we know:
    // (1) theNym is agent for pParty,     (according to the party)
    // (2) theAccount is owned by pParty   (according to the party)
    // (3) theNym is agent for theAccount  (according to the party)
    // (4) theNym is valid signer for pAgent
    // (5) pParty is the actual OWNER of the account. (According to theAccount.)

    pParty->ClearTemporaryPointers(); // Just in case.

    return true;
}

// Normally I'd call this to prove OWNERSHIP and I'd ASSUME AGENCY based on
// that:
// pSourceAcct->VerifyOwner(*pSenderNym)

// However, if I'm instead supposed to accept that pSenderNym, while NOT listed
// as the "Owner" on the
// account itself, is still somehow AUTHORIZED to change it, "because the actual
// owner says that it's okay"...
// Well in that case, it's not good enough unless I prove BOTH ownership AND
// Agency. I should prove:
// -- the actual owner for the account,
// -- AND is shown to have signed the agreement with his authorizing agent,
// -- AND is shown to have set theNym as his agent, and listed theNym as
// authorized agent for that account.

/*
 Proves the original party DOES approve of Nym managing that account (assuming
 that VerifyNymAsAgentForAnyParty was called, which
 saves us from having to look up the authorizing agent for the party and verify
 his signature on the entire contract. We assume the
 contract itself is sound, and we go on to verify its term.)
 Also needs to prove that the ACCOUNT believes itself to be owned by the
 original party and was signed by the server.
 (If we have proved that ACCOUNT believes itself to be owned by the party, AND
 that the PARTY believes it has authorized Nym to
 manage that account,
 plus the previous function already proves that Nym is an agent of the PARTY and
 that the original agreement
 is sound, as signed by the party's authorizing agent.)

 AGENCY
 1) Lookup the account from among all the parties. This way we find the party.
 bool FindPartyBasedOnAccount(theAccount);
 2) Lookup the party based on the Nym as agent, and compare it to make sure they
 are the SAME PARTY.
 3) Lookup the authorized agent for that account (from that party.) This is
 different than the authorizing agent in
    the function above, which is who signed to open the contract and fronted the
 opening trans#. But in THIS case, the authorized
    agent must be available for EACH asset account, and he is the one who
 supplied the CLOSING number. Each acct has an agent
    name, so lookup that agent from pParty.
 4) Make sure theNym IS that authorized agent. Only THEN do we know that theNym
 is authorized (as far as the party is concerned.)
    Notice also that we didn't verify theNym's signature on anything since he
 hasn't actually signed anything on the contract itself.
    Notice that the ONLY REASON THAT WE KNOW FOR SURE is because
 VerifyNymAsAgent() was called before this function! Because that is
    what loads the original signer and verifies his signature. Unless that was
 done, we honestly don't even know this much! So make
    sure you call that function before this one.

 Assuming we get this far, this means the Party / Smart Contract actually
 approves of theNym as an Agent, and of theAccount as an
 account, and it also approves that Nym is authorized to manipulate that
 account.
 But it doesn't prove who OWNS that account! Nor does it prove, even if the Nym
 DOES own it, whether theNym actually authorizes
 this action. Therefore we must also prove that the owner of the account
 (according to the ACCOUNT) is the same party who authorized
 the Nym to manage that account in the smart contract.  (If the Nym IS the
 party, then he will show as the owner AND agent.)

 OWNERSHIP
 1) pParty->VerifyOwnership(theAccount)
 2) If Party owner is a Nym, then call verifyOwner on the account directly with
 that Nym (just like the old way.)
 3) BUT, if Party owner is an Entity, verify that the entity owns the account.
 4) ALSO: Outside of the smart contract, the same Nym should be able to
 manipulate that account in this same way, just
    by showing that the account is owned by an entity where that Nym has the
 RoleID in that entity that appears on the
    account. (Account should have entity AND role).

 AHH this is the difference between "this Nym may manipulate this account in
 general, due to his role" vs. "this Nym may
 manipulate this account DUE TO THE AGENCY OF THE ROLE ID stored in the
 Account."
 What's happening is, I'm proving BOTH.  The smart contract, even if you HAVE
 the role, still restricts you from doing anything
 FROM INSIDE THE SMART CONTRACT (scripts) where that Nym isn't explicitly set up
 as the authorized agent for that account. The Nym
 may STILL *normally* be able to mess with that account, if the role is set up
 that way -- he just can't do it from inside the script,
 without the additional setup inside the script itself that THIS Nym is the one
 setup on THIS account.

 Really though, inside the script itself, the only time a Nym would be set up as
 the account manager is really if the script
 owner wanted to set the ROLE as the account manager.  I don't REALLY want to
 set Frank in charge in the sales budget money.
 Rather, I want to set the SALES DIRECTOR ROLE as being in charge of the sales
 budget money, and Frank just happens to be in that
 role.  IF I FIRE HIM TOMORROW, AND PUT ANOTHER NYM IN THAT ROLE, THE CONTRACT
 SHOULD STILL FUNCTION, right? Following that...

 The account itself will store the EntityID and RoleID.

 The partyaccount (on the party) only has the "agent name" who deals with that
 account. (No nym ID so far.)

 When I lookup the partyaccount, I lookup the authorized agent. At this point,
 the agent could still be a Role, not a Nym.

 Therefore when I verify the Nym against the agent, I do it the same way as I
 might verify a nym against an account. If the
 agent is a nym repping himself, I compare the NymIDs. Else if the agent is an
 individual in a role, then I  take the Entity that
 owns that agent (the entity is hidden inside the party) and I compare the Nym's
 ID to the NymID recorded in the Entity as being
 responsible for that role based on the RoleID stored in the agent. CLearer:
 The agent has a RoleID and an Entity ID. The entity
 that owns it has a list of NymIDs, each mapped to a RoleID.  For the account
 controlled by the agent, if the Nym trying to change
 that account is actually listed in entity EntityID as being assigned role
 RoleID, where EntityID and RoleID match those in the agent,
 then Nym IS authorized to play with the account. This is verifiable for
 entities, nyms, and accounts even OUTSIDE of a smart contract,
 and doesn't rely on the smart contract having the partyaccount/authorized_agent
 set up. Those are just ADDITIONAL restrictions.
 Thus, Inside a smart contract, OT should enforce those additional restrictions
 as a layer above the verification of role/owner, etc.

 OR: Should I make it instead: That you must EITHER have Role-permission, OR
 smartcontract authorization, and EITHER WAY it allows
 you to manipulate the account?

 */

// Find the first (and hopefully the only) clause on this scriptable object,
// with a given name. (Searches ALL Bylaws on *this.)
//
OTClause* OTScriptable::GetClause(std::string str_clause_name) const
{
    if (!OTScriptable::ValidateName(str_clause_name)) // this logs, FYI.
    {
        otErr << __FUNCTION__ << ": Error: invalid name.\n";
        return nullptr;
    }

    for (auto& it : m_mapBylaws) {
        OTBylaw* pBylaw = it.second;
        OT_ASSERT(nullptr != pBylaw);

        OTClause* pClause = pBylaw->GetClause(str_clause_name);

        if (nullptr != pClause) // found it.
            return pClause;
    }

    return nullptr;
}

OTAgent* OTScriptable::GetAgent(std::string str_agent_name) const
{
    if (!OTScriptable::ValidateName(str_agent_name)) // this logs, FYI.
    {
        otErr << __FUNCTION__ << ": Error: invalid name.\n";
        return nullptr;
    }

    for (auto& it : m_mapParties) {
        OTParty* pParty = it.second;
        OT_ASSERT(nullptr != pParty);

        OTAgent* pAgent = pParty->GetAgent(str_agent_name);

        if (nullptr != pAgent) // found it.
            return pAgent;
    }

    return nullptr;
}

OTBylaw* OTScriptable::GetBylaw(std::string str_bylaw_name) const
{
    if (!OTScriptable::ValidateName(str_bylaw_name)) // this logs, FYI.
    {
        otErr << __FUNCTION__ << ": Error: invalid name.\n";
        return nullptr;
    }

    auto it = m_mapBylaws.find(str_bylaw_name);

    if (m_mapBylaws.end() == it) // Did NOT find it.
    {
        return nullptr;
    }

    OTBylaw* pBylaw = it->second;
    OT_ASSERT(nullptr != pBylaw);

    return pBylaw;
}

OTParty* OTScriptable::GetParty(std::string str_party_name) const
{
    if (!OTScriptable::ValidateName(str_party_name)) // this logs, FYI.
    {
        otErr << __FUNCTION__ << ": Error: invalid name.\n";
        return nullptr;
    }

    auto it = m_mapParties.find(str_party_name);

    if (m_mapParties.end() == it) // Did NOT find it.
    {
        return nullptr;
    }

    OTParty* pParty = it->second;
    OT_ASSERT(nullptr != pParty);

    return pParty;
}

OTParty* OTScriptable::GetPartyByIndex(int32_t nIndex) const
{
    if ((nIndex < 0) || (nIndex >= static_cast<int64_t>(m_mapParties.size()))) {
        otErr << __FUNCTION__ << ": Index out of bounds: " << nIndex << "\n";
    }
    else {

        int32_t nLoopIndex = -1; // will be 0 on first iteration.

        for (auto& it : m_mapParties) {
            OTParty* pParty = it.second;
            OT_ASSERT(nullptr != pParty);

            ++nLoopIndex; // 0 on first iteration.

            if (nLoopIndex == nIndex) return pParty;
        }
    }
    return nullptr;
}

OTBylaw* OTScriptable::GetBylawByIndex(int32_t nIndex) const
{
    if ((nIndex < 0) || (nIndex >= static_cast<int64_t>(m_mapBylaws.size()))) {
        otErr << __FUNCTION__ << ": Index out of bounds: " << nIndex << "\n";
    }
    else {

        int32_t nLoopIndex = -1; // will be 0 on first iteration.

        for (auto& it : m_mapBylaws) {
            OTBylaw* pBylaw = it.second;
            OT_ASSERT(nullptr != pBylaw);

            ++nLoopIndex; // 0 on first iteration.

            if (nLoopIndex == nIndex) return pBylaw;
        }
    }
    return nullptr;
}

// Verify the contents of THIS contract against signed copies of it that are
// stored in each Party.
//
bool OTScriptable::VerifyThisAgainstAllPartiesSignedCopies()
{
    bool bReturnVal = !m_mapParties.empty();

    // MAKE SURE ALL SIGNED COPIES ARE OF THE SAME CONTRACT.
    // Loop through ALL the parties. For whichever ones are already signed,
    // load up the signed copy and make sure it compares to the main one.
    // This is in order to make sure that I am signing the same thing that
    // everyone else signed, before I actually sign it.
    //
    for (auto& it : m_mapParties) {
        const std::string current_party_name = it.first;
        OTParty* pParty = it.second;
        OT_ASSERT(nullptr != pParty);

        if (pParty->GetMySignedCopy().Exists()) {
            OTScriptable* pPartySignedCopy =
                OTScriptable::InstantiateScriptable(pParty->GetMySignedCopy());
            std::unique_ptr<OTScriptable> theCopyAngel;

            if (nullptr == pPartySignedCopy) {
                otErr << __FUNCTION__ << ": Error loading party's ("
                      << current_party_name
                      << ") signed copy of agreement. Has it been executed?\n";
                return false;
            }
            else
                theCopyAngel.reset(pPartySignedCopy);

            if (!Compare(*pPartySignedCopy)) // <==== For all signed
                                             // copies, we compare them to
                                             // *this.
            {
                otErr << __FUNCTION__ << ": Party's (" << current_party_name
                      << ") signed copy of agreement doesn't match *this.\n";
                return false;
            }
        }
        // else nothing. (We only verify against the ones that are signed.)
    }

    return bReturnVal;
}

// Done
// Takes ownership. Client side.
// ONLY works if the party is ALREADY there. Assumes the transaction #s, etc are
// set up already.
//
// Used once all the actual parties have examined a specific smartcontract and
// have
// chosen to sign-on to it. This function looks up the theoretical party (such
// as "trustee")
// that they are trying to sign on to, compares the two, then replaces the
// theoretical one
// with the actual one. Then it signs the contract and saves a copy inside the
// party.
//
bool OTScriptable::ConfirmParty(OTParty& theParty)
{
    const std::string str_party_name = theParty.GetPartyName();

    if (!OTScriptable::ValidateName(str_party_name)) // this logs, FYI.
    {
        otErr << "OTScriptable::" << __FUNCTION__ << ":  Error: invalid name.\n";
        return false;
    }

    // MAKE SURE ALL SIGNED COPIES ARE OF THE SAME CONTRACT.
    // Loop through ALL the parties. For whichever ones are already signed,
    // load up the signed copy and make sure it compares to the main one.
    // This is in order to make sure that I am signing the same thing that
    // everyone else signed, before I actually sign it.
    //
    if (!VerifyThisAgainstAllPartiesSignedCopies())
        return false; // This already logs on failure.

    // BY THIS POINT, we know that, of all the parties who have already signed,
    // their signed copies DO match this smart contract.
    //

    //
    // Next, find the theoretical Party on this scriptable that matches the
    // actual Party that
    // was passed in. (It should already be there.) If found, replace it with
    // the one passed in.
    // Then sign the contract, save the signed version in the new party, and
    // then sign again.
    //
    // If NOT found, then we failed. (For trying to confirm a non-existent
    // party.)
    //
    auto it_delete = m_mapParties.find(str_party_name);

    if (it_delete != m_mapParties.end()) // It was already there. (Good.)
    {
        OTParty* pParty = it_delete->second;
        OT_ASSERT(nullptr != pParty);

        if (!pParty->Compare(theParty)) // Make sure my party compares to the
                                        // one it's replacing...
        {
            otOut << "OTScriptable::" << __FUNCTION__ << ": Party (" << str_party_name
                  << ") doesn't match the one it's confirming.\n";
            return false;
        }
        // else...
        m_mapParties.erase(it_delete); // Remove the theoretical party from the
                                       // map, so we can replace it with the
                                       // real one.
        delete pParty;
        pParty = nullptr; // Delete it, since I own it.

        // Careful:  This ** DOES ** TAKE OWNERSHIP!  theParty will get deleted
        // when this OTScriptable instance is.
        //
        m_mapParties.insert(
            std::pair<std::string, OTParty*>(str_party_name, &theParty));

        openingNumsInOrderOfSigning_.push_back(theParty.GetOpeningTransNo());

        theParty.SetOwnerAgreement(*this); // Now the actual party is in place,
                                           // instead of a placekeeper version
                                           // of it. There are actual Acct/Nym
                                           // IDs now, etc.

        // Sign it and save it,
        String strNewSignedCopy;
        ReleaseSignatures();
        bool bSuccess = theParty.SignContract(*this);
        if (bSuccess) {
            SaveContract();
            SaveContractRaw(strNewSignedCopy);

            // then save a copy of it inside theParty,
            //
            theParty.SetMySignedCopy(strNewSignedCopy);

            // then sign the overall thing again, so that signed copy (now
            // inside theParty) will be properly saved.
            // That way when other people verify my signature, it will be there
            // for them to verify.
            //
            ReleaseSignatures();
            theParty.SignContract(*this);
            SaveContract();

            return true;
        }

        return false;
    }
    else
        otOut << "OTScriptable::" << __FUNCTION__ << ": Failed attempt to confirm "
                 "non-existent party: " << str_party_name << " \n ";

    return false;
}

// ONLY adds it if it's not already there.
// Used during the design of the smartcontract. (Adding theoretical parties.)
//
bool OTScriptable::AddParty(OTParty& theParty)
{
    const std::string str_party_name = theParty.GetPartyName();

    if (!OTScriptable::ValidatePartyName(str_party_name)) // this logs, FYI.
    {
        otErr << "OTScriptable::" << __FUNCTION__ << ":  Error: invalid name.\n";
        return false;
    }

    if (m_mapParties.find(str_party_name) == m_mapParties.end()) {
        // Careful:  This ** DOES ** TAKE OWNERSHIP!  theParty will get deleted
        // when this OTScriptable is.
        m_mapParties.insert(
            std::pair<std::string, OTParty*>(str_party_name, &theParty));

        theParty.SetOwnerAgreement(*this);

        return true;
    }
    else
        otOut << "OTScriptable::" << __FUNCTION__ << ": Failed attempt: party already exists "
                 "on contract.\n ";

    return false;
}

bool OTScriptable::RemoveParty(std::string str_Name)
{
    if (!OTScriptable::ValidatePartyName(str_Name)) // this logs, FYI.
    {
        otErr << "OTScriptable::" << __FUNCTION__ << ":  Error: invalid name.\n";
        return false;
    }

    auto it = m_mapParties.find(str_Name);

    if (m_mapParties.end() != it) // Found it.
    {
        OTParty * pParty = it->second;
        OT_ASSERT(nullptr != pParty);

        m_mapParties.erase(it);
        delete pParty; pParty = nullptr;
        return true;
    }
    else
        otOut << "OTScriptable::" << __FUNCTION__ << ": Failed attempt: party didn't exist "
        "on contract.\n ";

    return false;
}

bool OTScriptable::RemoveBylaw(std::string str_Name)
{
    if (!OTScriptable::ValidateBylawName(str_Name)) // this logs, FYI.
    {
        otErr << "OTScriptable::" << __FUNCTION__ << ":  Error: invalid name.\n";
        return false;
    }

    auto it = m_mapBylaws.find(str_Name);

    if (m_mapBylaws.end() != it) // Found it.
    {
        OTBylaw * pBylaw = it->second;
        OT_ASSERT(nullptr != pBylaw);

        m_mapBylaws.erase(it);
        delete pBylaw; pBylaw = nullptr;
        return true;
    }
    else
        otOut << "OTScriptable::" << __FUNCTION__ << ": Failed attempt: bylaw didn't exist "
        "on contract.\n ";

    return false;
}

bool OTScriptable::AddBylaw(OTBylaw& theBylaw)
{
    const std::string str_name = theBylaw.GetName().Get();

    if (!OTScriptable::ValidateBylawName(str_name)) // this logs, FYI.
    {
        otErr << "OTScriptable::" << __FUNCTION__ << ":  Error: invalid name.\n";
        return false;
    }

    if (m_mapBylaws.find(str_name) == m_mapBylaws.end()) {
        // Careful:  This ** DOES ** TAKE OWNERSHIP!  theBylaw will get deleted
        // when this OTScriptable is.
        m_mapBylaws.insert(
            std::pair<std::string, OTBylaw*>(str_name, &theBylaw));

        theBylaw.SetOwnerAgreement(*this);

        return true;
    }
    else
        otOut << "OTScriptable::" << __FUNCTION__ << ": Failed attempt: bylaw already exists "
                 "on contract.\n ";

    return false;
}

/*
 <party name=“shareholders”
 Owner_Type=“entity”  // ONLY can be “nym” or “entity”.
 Owner_ID=“this” >   // Nym ID or Entity ID. Not known at creation.

 <agent type=“group”// could be “nym”, or “role”, or “group”.
    Nym_id=“” // In case of “nym”, this is the Nym’s ID. If “role”, this is
 NymID of employee in role.
    Role_id=“” // In case of “role”, this is the Role’s ID.
    Entity_id=“this” // same as OwnerID if ever used. Should remove.
    Group_ID=“class_A” // “class A shareholders” are the voting group that
 controls this agent.
    />

 </party>
 */

bool OTScriptable::Compare(OTScriptable& rhs) const
{
    const char* szFunc = "OTScriptable::Compare";
    //
    // UPDATE: ALL of the parties should be there, in terms of their
    // names and account names --- but the actual IDs can remain blank,
    // and the signed copies can be blank. (Those things are all verified
    // elsewhere. That's about verifying the signature and authorization,
    // versus verifying the content.)
    //
    if (GetPartyCount() != rhs.GetPartyCount()) {
        otOut << szFunc << ": The number of parties does not match.\n";
        return false;
    }
    if (GetBylawCount() != rhs.GetBylawCount()) {
        otOut << szFunc << ": The number of bylaws does not match.\n";
        return false;
    }

    for (const auto& it : m_mapBylaws) {
        const std::string str_bylaw_name = it.first;
        OTBylaw* pBylaw = it.second;
        OT_ASSERT(nullptr != pBylaw);

        OTBylaw* p2 = rhs.GetBylaw(str_bylaw_name);

        if (nullptr == p2) {
            otOut << szFunc << ": Unable to find bylaw " << str_bylaw_name
                  << " on rhs.\n";
            return false;
        }
        else if (!pBylaw->Compare(*p2)) {
            otOut << szFunc << ": Bylaws don't match: " << str_bylaw_name
                  << ".\n";
            return false;
        }
    }

    for (const auto& it : m_mapParties) {
        const std::string str_party_name = it.first;
        OTParty* pParty = it.second;
        OT_ASSERT(nullptr != pParty);

        OTParty* p2 = rhs.GetParty(str_party_name);

        if (nullptr == p2) {
            otOut << szFunc << ": Unable to find party " << str_party_name
                  << " on rhs.\n";
            return false;
        }
        else if (!pParty->Compare(*p2)) {
            otOut << szFunc << ": Parties don't match: " << str_party_name
                  << ".\n";
            return false;
        }
    }

    return true;
}

// Most contracts calculate their ID by hashing the Raw File (signatures and
// all).
// The scriptable only hashes the unsigned contents, and only with specific data
// removed.
// This way, the smart contract will produce a consistent ID even when asset
// account IDs
// or Nym IDs have been added to it (which would alter the hash of any normal
// contract
// such as an OTUnitDefinition.)
//
void OTScriptable::CalculateContractID(Identifier& newID) const
{
    // Produce a template version of the scriptable.
    OTStringXML xmlUnsigned;

    Tag tag("scriptable");

    UpdateContentsToTag(tag, true);

    std::string str_result;
    tag.output(str_result);

    xmlUnsigned.Concatenate("%s", str_result.c_str());

    newID.CalculateDigest(xmlUnsigned);
}

std::string vectorToString(const std::vector<int64_t> & v)
{
    std::stringstream ss;

    for (size_t i = 0; i < v.size(); ++i)
    {
        if(i != 0)
            ss << " ";
        ss << v[i];
    }
    return ss.str();
}

std::vector<int64_t> stringToVector(const std::string & s)
{
    std::stringstream stream(s);

    std::vector<int64_t> results;

    int64_t n;
    while(stream >> n)
    {
        results.push_back(n);
    }

    return results;
}

void OTScriptable::UpdateContentsToTag(Tag& parent, bool bCalculatingID) const
{
//    if ((!m_mapParties.empty()) || (!m_mapBylaws.empty())) {

        TagPtr pTag(new Tag("scriptableContract"));

        uint32_t sizePartyMap = m_mapParties.size();
        uint32_t sizeBylawMap = m_mapBylaws.size();

        pTag->add_attribute("specifyInstrumentDefinitionID",
                            formatBool(m_bSpecifyInstrumentDefinitionID));
        pTag->add_attribute("specifyParties", formatBool(m_bSpecifyParties));
        pTag->add_attribute("numParties", formatUint(sizePartyMap));
        pTag->add_attribute("numBylaws", formatUint(sizeBylawMap));

        const std::string str_vector = vectorToString(openingNumsInOrderOfSigning_);
        pTag->add_attribute("openingNumsInOrderOfSigning", str_vector);

        for (auto& it : m_mapParties) {
            OTParty* pParty = it.second;
            OT_ASSERT(nullptr != pParty);

            // Serialization is slightly different depending on whether
            // we are saving for real, or just generating a template
            // version of the contract in order to generate its ID.
            //
            pParty->Serialize(*pTag, bCalculatingID,
                              m_bSpecifyInstrumentDefinitionID,
                              m_bSpecifyParties);
        }

        for (auto& it : m_mapBylaws) {
            OTBylaw* pBylaw = it.second;
            OT_ASSERT(nullptr != pBylaw);

            pBylaw->Serialize(*pTag, bCalculatingID);
        }

        parent.add_tag(pTag);
//    }
}

void OTScriptable::UpdateContents() // Before transmission or serialization,
                                    // this is where the contract updates its
                                    // contents
{
    // I release this because I'm about to repopulate it.
    m_xmlUnsigned.Release();

    Tag tag("scriptable");

    UpdateContentsToTag(tag, m_bCalculatingID);

    std::string str_result;
    tag.output(str_result);

    m_xmlUnsigned.Concatenate("%s", str_result.c_str());
}

// return -1 if error, 0 if nothing, and 1 if the node was processed.
int32_t OTScriptable::ProcessXMLNode(irr::io::IrrXMLReader*& xml)
{
    int32_t nReturnVal = 0; // Unless/until I want to add Contract::Compare(),
                            // then people would be able to surreptitiously
                            // insert keys and
    //    int32_t nReturnVal = ot_super::ProcessXMLNode(xml); // conditions, and
    // entities, that passed OTScriptable::Compare() with flying colors
    //  even though they didn't really match. Therefore, here I explicitly
    // disallow loading those things.

    // Here we call the parent class first.
    // If the node is found there, or there is some error,
    // then we just return either way.  But if it comes back
    // as '0', then nothing happened, and we'll continue executing.
    //
    // -- Note: you can choose not to call the parent, if
    // you don't want to use any of those xml tags.

    //    if (nReturnVal == 1 || nReturnVal == (-1))
    //        return nReturnVal;

    const String strNodeName(xml->getNodeName());

    //    otErr << "OTScriptable::ProcessXMLNode:  strNodeName: %s \n",
    // strNodeName.Get());

    if (strNodeName.Compare("scriptableContract")) {
        const char* szFunc = "OTScriptable::ProcessXMLNode";
        const String strSpecify1 =
            xml->getAttributeValue("specifyInstrumentDefinitionID");
        const String strSpecify2 = xml->getAttributeValue("specifyParties");

        String strNumParties = xml->getAttributeValue("numParties");
        String strNumBylaws = xml->getAttributeValue("numBylaws");

        String strOpeningNumsOrderSigning = xml->getAttributeValue("openingNumsInOrderOfSigning");

        if (strOpeningNumsOrderSigning.Exists())
        {
            const std::string str_opening_nums(strOpeningNumsOrderSigning.Get());
            openingNumsInOrderOfSigning_ = stringToVector(str_opening_nums);
        }
        else
            openingNumsInOrderOfSigning_.clear();

        // These determine whether instrument definition ids and/or party owner
        // IDs
        // are used on the template for any given smart contract.
        // (Some templates are specific to certain instrument definitions, while
        // other smart contract types allow you to leave instrument definition
        // id blank
        // until the confirmation phase.)
        //
        if (strSpecify1.Compare("true"))
            m_bSpecifyInstrumentDefinitionID = true;
        if (strSpecify2.Compare("true")) m_bSpecifyParties = true;

        // Load up the Parties.
        //
        int32_t nPartyCount =
            strNumParties.Exists() ? atoi(strNumParties.Get()) : 0;
        if (nPartyCount > 0) {
            while (nPartyCount-- > 0) {
                //                xml->read(); // <==================
                if (!SkipToElement(xml)) {
                    otOut << szFunc << ": Failure: Unable to find expected "
                                       "element for party. \n";
                    return (-1);
                }

                //                otOut << "%s: Looping to load parties:
                // currently on: %s \n", szFunc, xml->getNodeName());

                if ((!strcmp("party", xml->getNodeName()))) {
                    String strName = xml->getAttributeValue(
                        "name"); // Party name (in script code)
                    String strOwnerType = xml->getAttributeValue(
                        "ownerType"); // "nym" or "entity"
                    String strOwnerID = xml->getAttributeValue(
                        "ownerID"); // Nym or Entity ID. todo security probably
                                    // make these separate variables.

                    String strOpeningTransNo = xml->getAttributeValue(
                        "openingTransNo"); // the closing #s are on the asset
                                           // accounts.

                    String strAuthAgent = xml->getAttributeValue(
                        "authorizingAgent"); // When an agent activates this
                                             // contract, it's HIS opening
                                             // trans# that's used.

                    String strNumAgents = xml->getAttributeValue(
                        "numAgents"); // number of agents on this party.
                    String strNumAccounts = xml->getAttributeValue(
                        "numAccounts"); // number of accounts for this party.

                    String strIsCopyProvided =
                        xml->getAttributeValue("signedCopyProvided");

                    bool bIsCopyProvided = false; // default

                    if (strIsCopyProvided.Compare("true"))
                        bIsCopyProvided = true;

                    int64_t lOpeningTransNo = 0;

                    if (strOpeningTransNo.Exists())
                        lOpeningTransNo = strOpeningTransNo.ToLong();
                    else
                        otErr << szFunc
                              << "s: Expected openingTransNo in party.\n";

                    OTParty* pParty = new OTParty(
                        strName.Exists() ? strName.Get() : "PARTY_ERROR_NAME",
                        strOwnerType.Compare("nym") ? true : false,
                        strOwnerID.Get(), strAuthAgent.Get());
                    OT_ASSERT(nullptr != pParty);

                    pParty->SetOpeningTransNo(
                        lOpeningTransNo); // WARNING:  NEED TO MAKE SURE pParty
                                          // IS CLEANED UP BELOW THIS POINT, IF
                                          // FAILURE!!

                    // Load up the agents.
                    //
                    int32_t nAgentCount =
                        strNumAgents.Exists() ? atoi(strNumAgents.Get()) : 0;
                    if (nAgentCount > 0) {
                        while (nAgentCount-- > 0) {
                            //                          xml->read(); //
                            // <==================
                            if (!SkipToElement(xml)) {
                                otOut << szFunc << ": Failure: Unable to find "
                                                   "expected element for "
                                                   "agent. \n";
                                delete pParty;
                                pParty = nullptr;
                                return (-1);
                            }

                            if ((xml->getNodeType() == irr::io::EXN_ELEMENT) &&
                                (!strcmp("agent", xml->getNodeName()))) {
                                String strAgentName = xml->getAttributeValue(
                                    "name"); // Agent name (if needed in script
                                             // code)
                                String strAgentRepSelf = xml->getAttributeValue(
                                    "doesAgentRepresentHimself"); // Agent
                                                                  // might
                                // also BE the
                                // party, and
                                // not just
                                // party's
                                // employee.
                                String strAgentIndividual =
                                    xml->getAttributeValue(
                                        "isAgentAnIndividual"); // Is the agent
                                                                // a voting
                                                                // group, or an
                                                                // individual
                                                                // nym? (whether
                                                                // employee or
                                                                // not)
                                String strNymID = xml->getAttributeValue(
                                    "nymID"); // Nym ID if Nym in role for
                                              // entity, or if representing
                                              // himself.
                                String strRoleID = xml->getAttributeValue(
                                    "roleID"); // Role ID if Nym in Role.
                                String strGroupName = xml->getAttributeValue(
                                    "groupName"); // Group name if voting group.
                                                  // (Relative to entity.)

                                if (!strAgentName.Exists() ||
                                    !strAgentRepSelf.Exists() ||
                                    !strAgentIndividual.Exists()) {
                                    otErr << szFunc << ": Error loading agent: "
                                                       "Either the name, or "
                                                       "one of the bool "
                                                       "variables was EMPTY.\n";
                                    delete pParty;
                                    pParty = nullptr;
                                    return (-1);
                                }

                                if (!OTScriptable::ValidateName(
                                        strAgentName.Get())) {
                                    otErr << szFunc
                                          << ": Failed loading agent due to "
                                             "Invalid name: " << strAgentName
                                          << "\n";
                                    delete pParty;
                                    pParty = nullptr;
                                    return (-1);
                                }

                                bool bRepsHimself = true; // default

                                if (strAgentRepSelf.Compare("false"))
                                    bRepsHimself = false;

                                bool bIsIndividual = true; // default

                                if (strAgentIndividual.Compare("false"))
                                    bIsIndividual = false;

                                // See if the same-named agent already exists on
                                // ANY of the OTHER PARTIES
                                // (There can only be one agent on an
                                // OTScriptable with a given name.)
                                //
                                OTAgent* pExistingAgent =
                                    GetAgent(strAgentName.Get());

                                if (nullptr != pExistingAgent) // Uh-oh, it's
                                                               // already there!
                                {
                                    otOut << szFunc
                                          << ": Error loading agent named "
                                          << strAgentName
                                          << ", since one was "
                                             "already there on party "
                                          << strName << ".\n";
                                    delete pParty;
                                    pParty = nullptr;
                                    return (-1);
                                }
                                // The AddAgent call below checks to see if it's
                                // already there, but only for the
                                // currently-loading party.
                                // Whereas the above GetAgent() call checks this
                                // OTScriptable for ALL the agents on the
                                // already-loaded parties.

                                OTAgent* pAgent = new OTAgent(
                                    bRepsHimself, bIsIndividual, strAgentName,
                                    strNymID, strRoleID, strGroupName);
                                OT_ASSERT(nullptr != pAgent);

                                if (!pParty->AddAgent(*pAgent)) {
                                    delete pAgent;
                                    pAgent = nullptr;
                                    delete pParty;
                                    pParty = nullptr;
                                    otErr
                                        << szFunc
                                        << ": Failed adding agent to party.\n";
                                    return (-1);
                                }

                                //                    xml->read(); //
                                // <==================

                                // MIGHT need to add "skip after element" here.

                                // Update: Nope.
                            }
                            else {
                                otErr << szFunc
                                      << ": Expected agent element in party.\n";
                                delete pParty;
                                pParty = nullptr;
                                return (-1); // error condition
                            }
                        } // while
                    }

                    // LOAD PARTY ACCOUNTS.
                    //
                    int32_t nAcctCount = strNumAccounts.Exists()
                                             ? atoi(strNumAccounts.Get())
                                             : 0;
                    if (nAcctCount > 0) {
                        while (nAcctCount-- > 0) {
                            if (!Contract::SkipToElement(xml)) {
                                otErr << szFunc << ": Error finding expected "
                                                   "next element for party "
                                                   "account.\n";
                                delete pParty;
                                pParty = nullptr;
                                return (-1);
                            }

                            if ((xml->getNodeType() == irr::io::EXN_ELEMENT) &&
                                (!strcmp("assetAccount", xml->getNodeName()))) {
                                String strAcctName = xml->getAttributeValue(
                                    "name"); // Acct name (if needed in script
                                             // code)
                                String strAcctID = xml->getAttributeValue(
                                    "acctID"); // Asset Acct ID
                                String strInstrumentDefinitionID =
                                    xml->getAttributeValue(
                                        "instrumentDefinitionID"); // Instrument
                                                                   // Definition
                                                                   // ID
                                String strAgentName = xml->getAttributeValue(
                                    "agentName"); // Name of agent who controls
                                                  // this account.
                                String strClosingTransNo =
                                    xml->getAttributeValue(
                                        "closingTransNo"); // the closing #s are
                                                           // on the asset
                                                           // accounts.

                                int64_t lClosingTransNo = 0;

                                if (strClosingTransNo.Exists())
                                    lClosingTransNo =
                                        strClosingTransNo.ToLong();
                                else {
                                    otErr << szFunc << ": Expected "
                                                       "closingTransNo in "
                                                       "partyaccount.\n";
                                    delete pParty;
                                    pParty = nullptr;
                                    return (-1);
                                }

                                // Missing Account ID is allowed, as well as
                                // agent name, since those things may not be
                                // decided yet.
                                //
                                if (!strAcctName.Exists() ||
                                    (m_bSpecifyInstrumentDefinitionID &&
                                     !strInstrumentDefinitionID.Exists())) {
                                    otErr << szFunc
                                          << ": Expected missing "
                                             "AcctID or InstrumentDefinitionID "
                                             "or Name or AgentName "
                                             "in partyaccount.\n";
                                    delete pParty;
                                    pParty = nullptr;
                                    return (-1);
                                }

                                // See if the same-named partyacct already
                                // exists on ANY of the OTHER PARTIES
                                // (There can only be one partyacct on an
                                // OTScriptable with a given name.)
                                //
                                OTPartyAccount* pAcct =
                                    GetPartyAccount(strAcctName.Get());

                                if (nullptr !=
                                    pAcct) // Uh-oh, it's already there!
                                {
                                    otOut << szFunc
                                          << ": Error loading partyacct named "
                                          << strAcctName
                                          << ", since one was "
                                             "already there on party "
                                          << strName << ".\n";
                                    delete pParty;
                                    pParty = nullptr;
                                    return (-1);
                                }
                                // The AddAccount call below checks to see if
                                // it's already there, but only for the
                                // currently-loading party.
                                // Whereas the above call checks this
                                // OTScriptable for all the accounts on the
                                // already-loaded parties.

                                if (false ==
                                    pParty->AddAccount(
                                        strAgentName, strAcctName, strAcctID,
                                        strInstrumentDefinitionID,
                                        lClosingTransNo)) {
                                    otErr << szFunc << ": Failed adding "
                                                       "account to party.\n";
                                    delete pParty;
                                    pParty = nullptr;
                                    return (-1);
                                }

                                // MIGHT need to add "skip after field" call
                                // here.

                                // UPdate: Nope. Not here.

                            }
                            else {
                                otErr << szFunc << ": Expected assetAccount "
                                                   "element in party.\n";
                                delete pParty;
                                pParty = nullptr;
                                return (-1); // error condition
                            }
                        } // while
                    }

                    if (bIsCopyProvided) {
                        const char* pElementExpected = "mySignedCopy";
                        String strTextExpected; // signed copy will go here.

                        if (false ==
                            Contract::LoadEncodedTextFieldByName(
                                xml, strTextExpected, pElementExpected)) {
                            otErr << szFunc << ": Expected " << pElementExpected
                                  << " element with text field.\n";
                            delete pParty;
                            pParty = nullptr;
                            return (-1); // error condition
                        }
                        // else ...

                        pParty->SetMySignedCopy(strTextExpected);
                    }

                    if (AddParty(*pParty))
                        otInfo << szFunc
                               << ": Loaded Party: " << pParty->GetPartyName()
                               << "\n";
                    else {
                        otErr << szFunc << ": Failed loading Party: "
                              << pParty->GetPartyName() << "\n";
                        delete pParty;
                        pParty = nullptr;
                        return (-1); // error condition
                    }
                }
                else {
                    otErr << szFunc << ": Expected party element.\n";
                    return (-1); // error condition
                }
            } // while
        }

        // Load up the Bylaws.
        //
        int32_t nBylawCount =
            strNumBylaws.Exists() ? atoi(strNumBylaws.Get()) : 0;
        if (nBylawCount > 0) {
            while (nBylawCount-- > 0) {
                if (!SkipToElement(xml)) {
                    otOut << szFunc << ": Failure: Unable to find expected "
                                       "element for bylaw. \n";
                    return (-1);
                }

                if (!strcmp("bylaw", xml->getNodeName())) {
                    String strName =
                        xml->getAttributeValue("name"); // bylaw name
                    String strLanguage = xml->getAttributeValue(
                        "language"); // The script language used in this bylaw.

                    String strNumVariable = xml->getAttributeValue(
                        "numVariables"); // number of variables on this bylaw.
                    String strNumClauses = xml->getAttributeValue(
                        "numClauses"); // number of clauses on this bylaw.
                    String strNumHooks = xml->getAttributeValue(
                        "numHooks"); // hooks to server events.
                    String strNumCallbacks = xml->getAttributeValue(
                        "numCallbacks"); // Callbacks the server may initiate,
                                         // when it needs answers.

                    OTBylaw* pBylaw =
                        new OTBylaw(strName.Get(), strLanguage.Get());

                    OT_ASSERT(nullptr != pBylaw);

                    // LOAD VARIABLES AND CONSTANTS.
                    //
                    int32_t nCount = strNumVariable.Exists()
                                         ? atoi(strNumVariable.Get())
                                         : 0;
                    if (nCount > 0) {
                        while (nCount-- > 0) {
                            if (!Contract::SkipToElement(xml)) {
                                otErr << szFunc << ": Error finding expected "
                                                   "next element for "
                                                   "variable.\n";
                                delete pBylaw;
                                pBylaw = nullptr;
                                return (-1);
                            }

                            if ((xml->getNodeType() == irr::io::EXN_ELEMENT) &&
                                (!strcmp("variable", xml->getNodeName()))) {
                                String strVarName = xml->getAttributeValue(
                                    "name"); // Variable name (if needed in
                                             // script code)
                                String strVarValue = xml->getAttributeValue(
                                    "value"); // Value stored in variable (If
                                              // this is "true" then a real
                                              // value is expected in a text
                                              // field below. Otherwise, it's
                                              // assumed to be a BLANK STRING.)
                                String strVarType = xml->getAttributeValue(
                                    "type"); // string or int64_t
                                String strVarAccess = xml->getAttributeValue(
                                    "access"); // constant, persistent, or
                                               // important.

                                if (!strVarName.Exists() ||
                                    !strVarType.Exists() ||
                                    !strVarAccess.Exists()) {
                                    otErr << szFunc << ": Expected missing "
                                                       "name, type, or access "
                                                       "type in variable.\n";
                                    delete pBylaw;
                                    pBylaw = nullptr;
                                    return (-1);
                                }

                                // See if the same-named variable already exists
                                // on ANY of the OTHER BYLAWS
                                // (There can only be one variable on an
                                // OTScriptable with a given name.)
                                //
                                OTVariable* pVar =
                                    GetVariable(strVarName.Get());

                                if (nullptr !=
                                    pVar) // Uh-oh, it's already there!
                                {
                                    otOut << szFunc
                                          << ": Error loading variable named "
                                          << strVarName
                                          << ", since one was "
                                             "already there on one of the "
                                             "bylaws.\n";
                                    delete pBylaw;
                                    pBylaw = nullptr;
                                    return (-1);
                                }
                                // The AddVariable call below checks to see if
                                // it's already there, but only for the
                                // currently-loading bylaw.
                                // Whereas the above call checks this
                                // OTScriptable for all the variables on the
                                // already-loaded bylaws.

                                // VARIABLE TYPE AND ACCESS TYPE
                                //
                                OTVariable::OTVariable_Type theVarType =
                                    OTVariable::Var_Error_Type;

                                if (strVarType.Compare("integer"))
                                    theVarType = OTVariable::Var_Integer;
                                else if (strVarType.Compare("string"))
                                    theVarType = OTVariable::Var_String;
                                else if (strVarType.Compare("bool"))
                                    theVarType = OTVariable::Var_Bool;
                                else
                                    otErr << szFunc << ": Bad variable type: "
                                          << strVarType << ".\n";

                                OTVariable::OTVariable_Access theVarAccess =
                                    OTVariable::Var_Error_Access;

                                if (strVarAccess.Compare("constant"))
                                    theVarAccess = OTVariable::Var_Constant;
                                else if (strVarAccess.Compare("persistent"))
                                    theVarAccess = OTVariable::Var_Persistent;
                                else if (strVarAccess.Compare("important"))
                                    theVarAccess = OTVariable::Var_Important;
                                else
                                    otErr << szFunc
                                          << ": Bad variable access type: "
                                          << strVarAccess << ".\n";

                                if ((OTVariable::Var_Error_Access ==
                                     theVarAccess) ||
                                    (OTVariable::Var_Error_Type ==
                                     theVarType)) {
                                    otErr << szFunc
                                          << ": Error loading variable to "
                                             "bylaw: bad type (" << strVarType
                                          << ") or access type ("
                                          << strVarAccess << ").\n";
                                    delete pBylaw;
                                    pBylaw = nullptr;
                                    return (-1);
                                }

                                bool bAddedVar = false;
                                const std::string str_var_name =
                                    strVarName.Get();

                                switch (theVarType) {
                                case OTVariable::Var_Integer:
                                    if (strVarValue.Exists()) {
                                        const int32_t nVarValue =
                                            atoi(strVarValue.Get());
                                        bAddedVar = pBylaw->AddVariable(
                                            str_var_name, nVarValue,
                                            theVarAccess);
                                    }
                                    else {
                                        otErr << szFunc
                                              << ": No value found for integer "
                                                 "variable: " << strVarName
                                              << "\n";
                                        delete pBylaw;
                                        pBylaw = nullptr;
                                        return (-1);
                                    }
                                    break;

                                case OTVariable::Var_Bool:
                                    if (strVarValue.Exists()) {
                                        const bool bVarValue =
                                            strVarValue.Compare("true") ? true
                                                                        : false;
                                        bAddedVar = pBylaw->AddVariable(
                                            str_var_name, bVarValue,
                                            theVarAccess);
                                    }
                                    else {
                                        otErr << szFunc
                                              << ": No value found for bool "
                                                 "variable: " << strVarName
                                              << "\n";
                                        delete pBylaw;
                                        pBylaw = nullptr;
                                        return (-1);
                                    }
                                    break;

                                case OTVariable::Var_String: {
                                    // I realized I should probably allow empty
                                    // strings.  :-P
                                    if (strVarValue.Exists() &&
                                        strVarValue.Compare("exists")) {
                                        strVarValue.Release(); // probably
                                                               // unnecessary.
                                        if (false ==
                                            Contract::LoadEncodedTextField(
                                                xml, strVarValue)) {
                                            otErr << szFunc
                                                  << ": No value found for "
                                                     "string variable: "
                                                  << strVarName << "\n";
                                            delete pBylaw;
                                            pBylaw = nullptr;
                                            return (-1);
                                        }
                                        // (else success)
                                    }
                                    else
                                        strVarValue.Release(); // Necessary. If
                                                               // it's going to
                                                               // be a blank
                                                               // string, then
                                                               // let's make
                                                               // sure.

                                    const std::string str_var_value =
                                        strVarValue.Get();
                                    bAddedVar = pBylaw->AddVariable(
                                        str_var_name, str_var_value,
                                        theVarAccess);
                                } break;
                                default:
                                    otErr << szFunc
                                          << ": Wrong variable type... "
                                             "somehow AFTER I should have "
                                             "already detected it...\n";
                                    delete pBylaw;
                                    pBylaw = nullptr;
                                    return (-1);
                                }

                                if (!bAddedVar) {
                                    otErr << szFunc << ": Failed adding "
                                                       "variable to bylaw.\n";
                                    delete pBylaw;
                                    pBylaw = nullptr;
                                    return (-1);
                                }

                            }
                            else {
                                otErr << szFunc << ": Expected variable "
                                                   "element in bylaw.\n";
                                delete pBylaw;
                                pBylaw = nullptr;
                                return (-1); // error condition
                            }
                        } // while
                    }

                    // LOAD CLAUSES
                    //
                    nCount =
                        strNumClauses.Exists() ? atoi(strNumClauses.Get()) : 0;
                    if (nCount > 0) {
                        while (nCount-- > 0) {
                            const char* pElementExpected = "clause";
                            String strTextExpected; // clause's script code
                                                    // will go here.

                            String::Map temp_MapAttributes;
                            //
                            // This map contains values we will also want, when
                            // we read the clause...
                            // (The Contract::LoadEncodedTextField call below
                            // will read all the values
                            // as specified in this map.)
                            //
                            //
                            temp_MapAttributes.insert(
                                std::pair<std::string, std::string>("name",
                                                                    ""));
                            if (!Contract::LoadEncodedTextFieldByName(
                                    xml, strTextExpected, pElementExpected,
                                    &temp_MapAttributes)) // </clause>
                            {
                                otErr << szFunc << ": Error: Expected "
                                      << pElementExpected
                                      << " element with text field.\n";
                                delete pBylaw;
                                pBylaw = nullptr;
                                return (-1); // error condition
                            }

                            // Okay we now have the script code in
                            // strTextExpected. Next, let's read the clause's
                            // NAME
                            // from the map. If it's there, and presumably some
                            // kind of harsh validation for both, then
                            // create a clause object and add to my list here.

                            auto it = temp_MapAttributes.find("name");

                            if ((it != temp_MapAttributes.end())) // We expected
                                                                  // this much.
                            {
                                std::string& str_name = it->second;

                                if (str_name.size() > 0) // SUCCESS
                                {

                                    // See if the same-named clause already
                                    // exists on ANY of the OTHER BYLAWS
                                    // (There can only be one clause on an
                                    // OTScriptable with a given name.)
                                    //
                                    OTClause* pClause =
                                        GetClause(str_name.c_str());

                                    if (nullptr !=
                                        pClause) // Uh-oh, it's already there!
                                    {
                                        otOut
                                            << szFunc
                                            << ": Error loading clause named "
                                            << str_name
                                            << ", since one was already "
                                               "there on one of the bylaws.\n";
                                        delete pBylaw;
                                        pBylaw = nullptr;
                                        return (-1);
                                    }
                                    else if (false ==
                                               pBylaw->AddClause(
                                                   str_name.c_str(),
                                                   strTextExpected.Get())) {
                                        otErr << szFunc << ": Failed adding "
                                                           "clause to bylaw.\n";
                                        delete pBylaw;
                                        pBylaw = nullptr;
                                        return (-1); // error condition
                                    }
                                }
                                // else it's empty, which is expected if nothing
                                // was there, since that's the default value
                                // that we set above for "name" in
                                // temp_MapAttributes.
                                else {
                                    otErr << szFunc
                                          << ": Expected clause name.\n";
                                    delete pBylaw;
                                    pBylaw = nullptr;
                                    return (-1); // error condition
                                }
                            }
                            else {
                                otErr << szFunc << ": Strange error: couldn't "
                                                   "find name AT ALL.\n";
                                delete pBylaw;
                                pBylaw = nullptr;
                                return (-1); // error condition
                            }
                        } // while
                    }     // if strNumClauses.Exists() && nCount > 0

                    // LOAD HOOKS.
                    //
                    nCount = strNumHooks.Exists() ? atoi(strNumHooks.Get()) : 0;
                    if (nCount > 0) {
                        while (nCount-- > 0) {
                            //                          xml->read();
                            if (!SkipToElement(xml)) {
                                otOut << szFunc << ": Failure: Unable to find "
                                                   "expected element.\n";
                                delete pBylaw;
                                pBylaw = nullptr;
                                return (-1);
                            }

                            if ((xml->getNodeType() == irr::io::EXN_ELEMENT) &&
                                (!strcmp("hook", xml->getNodeName()))) {
                                String strHookName = xml->getAttributeValue(
                                    "name"); // Name of standard hook such as
                                             // hook_activate or cron_process,
                                             // etc
                                String strClause = xml->getAttributeValue(
                                    "clause"); // Name of clause on this Bylaw
                                               // that should trigger when that
                                               // callback occurs.

                                if (!strHookName.Exists() ||
                                    !strClause.Exists()) {
                                    otErr << szFunc << ": Expected missing "
                                                       "name or clause while "
                                                       "loading hook.\n";
                                    delete pBylaw;
                                    pBylaw = nullptr;
                                    return (-1);
                                }

                                if (false ==
                                    pBylaw->AddHook(strHookName.Get(),
                                                    strClause.Get())) {
                                    otErr << szFunc
                                          << ": Failed adding hook to bylaw.\n";
                                    delete pBylaw;
                                    pBylaw = nullptr;
                                    return (-1);
                                }
                            }
                            else {
                                otErr << szFunc
                                      << ": Expected hook element in bylaw.\n";
                                delete pBylaw;
                                pBylaw = nullptr;
                                return (-1); // error condition
                            }
                        } // while
                    }

                    // LOAD CALLBACKS.
                    //
                    nCount = strNumCallbacks.Exists()
                                 ? atoi(strNumCallbacks.Get())
                                 : 0;
                    if (nCount > 0) {
                        while (nCount-- > 0) {
                            //                          xml->read();
                            if (!SkipToElement(xml)) {
                                otOut << szFunc << ": Failure: Unable to find "
                                                   "expected element.\n";
                                delete pBylaw;
                                pBylaw = nullptr;
                                return (-1);
                            }

                            if ((xml->getNodeType() == irr::io::EXN_ELEMENT) &&
                                (!strcmp("callback", xml->getNodeName()))) {
                                String strCallbackName = xml->getAttributeValue(
                                    "name"); // Name of standard callback
                                             // such as OnActivate,
                                             // OnDeactivate, etc
                                String strClause = xml->getAttributeValue(
                                    "clause"); // Name of clause on this Bylaw
                                               // that should trigger when that
                                               // hook occurs.

                                if (!strCallbackName.Exists() ||
                                    !strClause.Exists()) {
                                    otErr << szFunc
                                          << ": Expected, yet nevertheless "
                                             "missing, name or clause while "
                                             "loading "
                                             "callback for bylaw " << strName
                                          << ".\n";
                                    delete pBylaw;
                                    pBylaw = nullptr;
                                    return (-1);
                                }

                                // See if the same-named callback already exists
                                // on ANY of the OTHER BYLAWS
                                // (There can only be one clause to handle any
                                // given callback.)
                                //
                                OTClause* pClause =
                                    GetCallback(strCallbackName.Get());

                                if (nullptr !=
                                    pClause) // Uh-oh, it's already there!
                                {
                                    otOut << szFunc
                                          << ": Error loading callback "
                                          << strCallbackName
                                          << ", since one was already there on "
                                             "one of the other bylaws.\n";
                                    delete pBylaw;
                                    pBylaw = nullptr;
                                    return (-1);
                                }
                                // The below call checks to see if it's already
                                // there, but only for the currently-loading
                                // bylaw.
                                // Whereas the above call checks this
                                // OTScriptable for all the already-loaded
                                // bylaws.

                                if (false ==
                                    pBylaw->AddCallback(strCallbackName.Get(),
                                                        strClause.Get())) {
                                    otErr << szFunc
                                          << ": Failed adding callback ("
                                          << strCallbackName << ") to bylaw ("
                                          << strName << ").\n";
                                    delete pBylaw;
                                    pBylaw = nullptr;
                                    return (-1);
                                }
                            }
                            else {
                                otErr << szFunc << ": Expected callback "
                                                   "element in bylaw.\n";
                                delete pBylaw;
                                pBylaw = nullptr;
                                return (-1); // error condition
                            }
                        } // while
                    }

                    if (AddBylaw(*pBylaw)) {
                        otInfo << szFunc
                               << ": Loaded Bylaw: " << pBylaw->GetName()
                               << "\n";
                    }
                    else {
                        otErr << szFunc
                              << ": Failed loading Bylaw: " << pBylaw->GetName()
                              << "\n";
                        delete pBylaw;
                        pBylaw = nullptr;
                        return (-1); // error condition
                    }
                }
                else {
                    otErr << szFunc << ": Expected bylaw element.\n";
                    return (-1); // error condition
                }

            } // while
        }

        nReturnVal = 1;
    }

    return nReturnVal;
}

// Look up the first (and hopefully only) variable registered for a given name.
// (Across all of my Bylaws)
//
OTVariable* OTScriptable::GetVariable(std::string str_VarName)
{
    if (!OTScriptable::ValidateName(str_VarName)) // this logs, FYI.
    {
        otErr << "OTScriptable::GetVariable:  Error: invalid name.\n";
        return nullptr;
    }

    for (auto& it : m_mapBylaws) {
        OTBylaw* pBylaw = it.second;
        OT_ASSERT(nullptr != pBylaw);

        OTVariable* pVar = pBylaw->GetVariable(str_VarName);

        if (nullptr != pVar) // found it.
            return pVar;
    }

    return nullptr;
}

// Look up the first (and hopefully only) clause registered for a given
// callback.
//
OTClause* OTScriptable::GetCallback(std::string str_CallbackName)
{
    if ((false == OTScriptable::ValidateName(str_CallbackName)) ||
        (str_CallbackName.compare(0, 9, "callback_") != 0)) // this logs, FYI.
    {
        otErr << "OTScriptable::GetCallback:  Error: invalid name: "
              << str_CallbackName << "\n";
        return nullptr;
    }

    for (auto& it : m_mapBylaws) {
        OTBylaw* pBylaw = it.second;
        OT_ASSERT(nullptr != pBylaw);

        OTClause* pClause = pBylaw->GetCallback(str_CallbackName);

        if (nullptr != pClause) // found it.
            return pClause;
    }

    return nullptr;
}

// Look up all clauses matching a specific hook.
//
bool OTScriptable::GetHooks(std::string str_HookName, mapOfClauses& theResults)
{
    if (false == OTScriptable::ValidateHookName(str_HookName)) // this logs, FYI.
    {
        otErr << "OTScriptable::GetHooks:  Error: invalid name.\n";
        return false;
    }

    bool bReturnVal = false;

    for (auto& it : m_mapBylaws) {
        OTBylaw* pBylaw = it.second;
        OT_ASSERT(nullptr != pBylaw);

        // Look up all clauses matching a specific hook.
        //
        if (pBylaw->GetHooks(str_HookName, theResults)) bReturnVal = true;
    }

    return bReturnVal;
}


bool OTScriptable::arePartiesSpecified() const
{
    return m_bSpecifyParties;
}
bool OTScriptable::areAssetTypesSpecified() const
{
    return m_bSpecifyInstrumentDefinitionID;
}

void OTScriptable::specifyParties(bool bNewState)
{
    m_bSpecifyParties = bNewState;
}

void OTScriptable::specifyAssetTypes(bool bNewState)
{
    m_bSpecifyInstrumentDefinitionID = bNewState;
}

void OTScriptable::Release_Scriptable()
{
    // Go through the existing list of parties and bylaws at this point, and
    // delete them all.
    // (After all, I own them.)
    //
    while (!m_mapParties.empty()) {
        OTParty* pParty = m_mapParties.begin()->second;
        OT_ASSERT(nullptr != pParty);

        m_mapParties.erase(m_mapParties.begin());

        delete pParty;
        pParty = nullptr;
    }
    while (!m_mapBylaws.empty()) {
        OTBylaw* pBylaw = m_mapBylaws.begin()->second;
        OT_ASSERT(nullptr != pBylaw);

        m_mapBylaws.erase(m_mapBylaws.begin());

        delete pBylaw;
        pBylaw = nullptr;
    }
}

void OTScriptable::Release()
{
    Release_Scriptable();

    // If there were any dynamically allocated objects, clean them up here.

    Contract::Release(); // since I've overridden the base class, I call it
                         // now...
}

OTScriptable::OTScriptable()
    : Contract()
    , m_bCalculatingID(false)
    , // This is not serialized.
    m_bSpecifyInstrumentDefinitionID(false)
    , m_bSpecifyParties(false) // These are.
{
}

OTScriptable::~OTScriptable()
{
    Release_Scriptable();
}

} // namespace opentxs
