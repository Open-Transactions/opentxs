// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/otx/smartcontract/OTScript.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <memory>
#include <utility>

#include "internal/core/String.hpp"
#include "internal/otx/smartcontract/Factory.hpp"
#include "internal/otx/smartcontract/OTParty.hpp"
#include "internal/otx/smartcontract/OTVariable.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::factory
{
auto OTScript(const UnallocatedCString& script_type)
    -> std::shared_ptr<opentxs::OTScript>
{
    if (script_type == "" || script_type == "chai") {

        return OTScriptChai();
    } else {
        LogError()("opentxs::factory::")(__func__)(": Script language (")(
            script_type)(") invalid not found")
            .Flush();

        return std::make_shared<opentxs::OTScript>();
    }
}

auto OTScript(
    const UnallocatedCString& script_type,
    const UnallocatedCString& script_contents)
    -> std::shared_ptr<opentxs::OTScript>
{
    if (script_type == "" || script_type == "chai") {

        return OTScriptChai(script_contents);
    } else {
        LogError()("opentxs::factory::")(__func__)(": Script language (")(
            script_type)(") invalid not found")
            .Flush();

        return std::make_shared<opentxs::OTScript>();
    }
}
}  // namespace opentxs::factory

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

OTScript::OTScript(const UnallocatedCString& new_string)
    : script_(new_string)
    , display_filename_()
    , parties_()
    , accounts_()
    , variables_()
{
}

OTScript::OTScript()
    : OTScript(UnallocatedCString{})
{
}

OTScript::OTScript(const String& strValue)
    : OTScript(UnallocatedCString{strValue.Get()})
{
}

OTScript::OTScript(const char* new_string)
    : OTScript(UnallocatedCString{new_string})
{
}

OTScript::OTScript(const char* new_string, std::size_t sizeLength)
    : OTScript(UnallocatedCString{new_string, sizeLength})
{
}

OTScript::~OTScript()
{
    // mapOfParties; // NO NEED to clean this up, since OTScript doesn't own the
    // parties.
    // See OTSmartContract, rather, for that.

    while (!variables_.empty()) {
        OTVariable* pVar = variables_.begin()->second;
        OT_ASSERT(nullptr != pVar);

        // NOTE: We're NOT going to delete pVar, since we don't own it.
        // But we ARE going to remove pVar's pointer to this script, so
        // pVar doesn't dereference a bad pointer later on.
        //
        pVar->UnregisterScript();
        variables_.erase(variables_.begin());
    }
}

void OTScript::SetScript(const String& strValue)
{
    if (strValue.Exists()) { script_ = strValue.Get(); }
}

void OTScript::SetScript(const char* new_string)
{
    if (nullptr != new_string) { script_ = new_string; }
}

void OTScript::SetScript(const char* new_string, std::size_t sizeLength)
{
    if (nullptr != new_string) { script_.assign(new_string, sizeLength); }
}

void OTScript::SetScript(const UnallocatedCString& new_string)
{
    script_ = new_string;
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
void OTScript::AddParty(UnallocatedCString str_party_name, OTParty& theParty)
{
    //  typedef UnallocatedMap<UnallocatedCString, OTParty *> mapOfParties;

    parties_.insert(
        std::pair<UnallocatedCString, OTParty*>(str_party_name, &theParty));
    // We're just storing these pointers for reference value. Script doesn't
    // actually Own the
    // parties, and isn't responsible to clean them up.

    theParty.RegisterAccountsForExecution(*this);
}

void OTScript::AddAccount(
    UnallocatedCString str_acct_name,
    OTPartyAccount& theAcct)
{
    accounts_.insert(std::pair<UnallocatedCString, OTPartyAccount*>(
        str_acct_name, &theAcct));

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
void OTScript::AddVariable(UnallocatedCString str_var_name, OTVariable& theVar)
{
    //  mapOfVariables  variables_;

    variables_.insert(
        std::pair<UnallocatedCString, OTVariable*>(str_var_name, &theVar));

    // We're just storing these pointers for reference value. Script doesn't
    // actually Own the
    // variables, and isn't responsible to clean them up.
}

auto OTScript::FindVariable(UnallocatedCString str_var_name) -> OTVariable*
{
    auto it_var = variables_.find(str_var_name);
    return it_var != variables_.end() ? it_var->second : nullptr;
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
    const UnallocatedCString str_var_name = theVar.GetName().Get();
    auto it_var = variables_.find(str_var_name);

    if (it_var != variables_.end()) {
        variables_.erase(it_var);  // no need to delete the variable pointer
        // since the script doesn't own it anyway.
    }
}

auto OTScript::ExecuteScript(OTVariable*) -> bool
{
    LogError()(OT_PRETTY_CLASS())("Scripting has been disabled.").Flush();
    return true;
}

auto OTScript::RegisterNativeScriptableCalls(OTScriptable&) noexcept -> void {}

auto OTScript::RegisterNativeSmartContractCalls(OTSmartContract&) noexcept
    -> void
{
}
}  // namespace opentxs
