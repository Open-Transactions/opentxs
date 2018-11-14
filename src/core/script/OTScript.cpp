// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/script/OTScript.hpp"

#include "opentxs/core/script/OTParty.hpp"
#if OT_SCRIPT_CHAI
#include "opentxs/core/script/OTScriptChai.hpp"
#endif
#include "opentxs/core/script/OTVariable.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"

#include <stddef.h>
#include <map>
#include <memory>
#include <ostream>
#include <string>
#include <utility>

#define OT_METHOD "opentxs::OTScript::"

namespace opentxs
{

// A script should be "Dumb", meaning that you just stick it with its
// parties and other resources, and it EXPECTS them to be the correct
// ones.  It uses them low-level style.
//
// Any verification should be done at a higher level, in OTSmartContract.
// There, multiple parties might be loaded, as well as multiple scripts
// (clauses) and that is where the proper resources, accounts, etc are
// instantiated and validated before any use.
//
// Thus by the time you get down to OTScript, all that validation is already
// done.  The programmatic user will interact with OTSmartContract, likely,
// and not with OTScript itself.
//

// Note: any relevant assets or asset accounts are listed by their owner /
// contributor
// parties. Therefore there's no need to separately input any accounts or assets
// to
// a script, since the necessary ones are already present inside their
// respective parties.
//
// bool OTScript::ExecuteScript()
//{
//    return true;
//}

/*

 To use:

 const char * default_script_language = "chai";

 ...

 OTParty theParty(theNym);

 // (Set up theParty here, with his asset accounts, etc)
 // Then...
 //
 std::shared_ptr<OTScript> pScript = OTScript::Factory(default_script_language,
 strScript);

 if (pScript)
 {
     pScript->AddParty("mynym", &theParty);
     pScript->Execute();
 }


 MIGHT WANT TO ADD an AddParty(string, Nym) function, which automatically wraps
 the Nym in a party.
 That way you can basically treat a Nym like a party to an agreement.

 */

std::shared_ptr<OTScript> OTScriptFactory(const std::string& script_type)
{

#if OT_SCRIPT_CHAI
    // default or explicit chai script interpreter
    if (script_type == "" || script_type == "chai")  // todo no hardcoding.
    {
        std::shared_ptr<OTScript> pChaiScript(new OTScriptChai);
        return pChaiScript;
    }

    //#elif OT_USE_SCRIPT_LUA
    //  if (script_type =="lua") // todo no hardcoding.
    //  {
    //      std::shared_ptr<OTScript> pLuaScript(new OTScriptLua);
    //      return pLuaScript;
    //  }

#else
    // default no script interpreter
    if (script_type == "") {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": WARNING 1: script_type == noscript.")
            .Flush();

        std::shared_ptr<OTScript> pNoScript(new OTScript);
        return pNoScript;
    }
#endif

    LogOutput(OT_METHOD)(__FUNCTION__)(": Script language (")(script_type)(
        ") not found.")
        .Flush();

    std::shared_ptr<OTScript> retVal;
    return retVal;
}

std::shared_ptr<OTScript> OTScriptFactory(
    const std::string& script_type,
    ANDROID_UNUSED const std::string& script_contents)
{

#if OT_SCRIPT_CHAI
    // default or explicit chai script interpreter
    if (script_type == "" || script_type == "chai")  // todo no hardcoding.
    {
        std::shared_ptr<OTScript> pChaiScript(
            new OTScriptChai(script_contents));
        return pChaiScript;
    }

    //#elif OT_USE_SCRIPT_LUA
    //  if (script_type =="lua") // todo no hardcoding.
    //  {
    //      std::shared_ptr<OTScript> pLuaScript(new
    //      OTScriptLua(script_contents)); return pLuaScript;
    //  }

#else
    // default no script interpreter
    if (script_type == "") {
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": WARNING 2: script_type == noscript.")
            .Flush();

        std::shared_ptr<OTScript> pNoScript(new OTScript);
        return pNoScript;
    }
#endif

    LogOutput(OT_METHOD)(__FUNCTION__)(": Script language (")(script_type)(
        ") not found.")
        .Flush();

    std::shared_ptr<OTScript> retVal;
    return retVal;
}

OTScript::OTScript() {}

OTScript::OTScript(const String& strValue)
    : m_str_script(strValue.Get())
{
}

OTScript::OTScript(const char* new_string)
    : m_str_script(new_string)
{
}

OTScript::OTScript(const char* new_string, size_t sizeLength)
    : m_str_script(new_string, sizeLength)
{
}

OTScript::OTScript(const std::string& new_string)
    : m_str_script(new_string)
{
}

OTScript::~OTScript()
{
    // mapOfParties; // NO NEED to clean this up, since OTScript doesn't own the
    // parties.
    // See OTSmartContract, rather, for that.

    while (!m_mapVariables.empty()) {
        OTVariable* pVar = m_mapVariables.begin()->second;
        OT_ASSERT(nullptr != pVar);

        // NOTE: We're NOT going to delete pVar, since we don't own it.
        // But we ARE going to remove pVar's pointer to this script, so
        // pVar doesn't dereference a bad pointer later on.
        //
        pVar->UnregisterScript();
        m_mapVariables.erase(m_mapVariables.begin());
    }
}

void OTScript::SetScript(const String& strValue)
{
    if (strValue.Exists()) m_str_script = strValue.Get();
}

void OTScript::SetScript(const char* new_string)
{
    if (nullptr != new_string) m_str_script = new_string;
}

void OTScript::SetScript(const char* new_string, size_t sizeLength)
{
    if (nullptr != new_string) m_str_script.assign(new_string, sizeLength);
}

void OTScript::SetScript(const std::string& new_string)
{
    m_str_script = new_string;
}

// The same OTSmartContract that loads all the clauses (scripts) will
// also load all the parties, so it will call this function whenever before it
// needs to actually run a script.
//
// NOTE: OTScript does NOT take ownership of the party, since there could be
// multiple scripts (with all scripts and parties being owned by a
// OTSmartContract.)
// Therefore it's ASSUMED that the owner OTSmartContract will handle all the
// work of
// cleaning up the mess!  theParty is passed as reference to insure it already
// exists.
//
void OTScript::AddParty(std::string str_party_name, OTParty& theParty)
{
    //  typedef std::map<std::string, OTParty *> mapOfParties;

    m_mapParties.insert(
        std::pair<std::string, OTParty*>(str_party_name, &theParty));
    // We're just storing these pointers for reference value. Script doesn't
    // actually Own the
    // parties, and isn't responsible to clean them up.

    theParty.RegisterAccountsForExecution(*this);
}

void OTScript::AddAccount(std::string str_acct_name, OTPartyAccount& theAcct)
{
    m_mapAccounts.insert(
        std::pair<std::string, OTPartyAccount*>(str_acct_name, &theAcct));

    // We're just storing these pointers for reference value. Script doesn't
    // actually Own the
    // accounts, and isn't responsible to clean them up.
}

// If you want to add a variable to a script, you should probably call
// OTVariable::RegisterForExecution
// so that later if the variable destructs, it will have a pointer to the script
// and it can remove itself
// from the script's list of variables. (Instead of calling this function, which
// is lower-level.)
//
void OTScript::AddVariable(std::string str_var_name, OTVariable& theVar)
{
    //  mapOfVariables  m_mapVariables;

    m_mapVariables.insert(
        std::pair<std::string, OTVariable*>(str_var_name, &theVar));

    // We're just storing these pointers for reference value. Script doesn't
    // actually Own the
    // variables, and isn't responsible to clean them up.
}

OTVariable* OTScript::FindVariable(std::string str_var_name)
{
    auto it_var = m_mapVariables.find(str_var_name);
    return it_var != m_mapVariables.end() ? it_var->second : nullptr;
}

// If a variable is set onto a script, it sets an internal pointer to that
// script.
// Later, when the variable destructs, if that pointer is set, it removes itself
// from the script by calling this function. (Yes, this would be better with
// smart
// pointers. C++11 here we come!)
//
void OTScript::RemoveVariable(OTVariable& theVar)
{
    const std::string str_var_name = theVar.GetName().Get();
    auto it_var = m_mapVariables.find(str_var_name);

    if (it_var != m_mapVariables.end()) {
        m_mapVariables.erase(it_var);  // no need to delete the variable pointer
        // since the script doesn't own it anyway.
    }
}

bool OTScript::ExecuteScript(OTVariable*)
{
    LogOutput(OT_METHOD)(__FUNCTION__)(": Scripting has been disabled.")
        .Flush();
    return true;
}

}  // namespace opentxs
