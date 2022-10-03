// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <memory>

#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class OTParty;
class OTPartyAccount;
class OTScriptable;
class OTSmartContract;
class OTVariable;
class String;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
using mapOfParties = UnallocatedMap<UnallocatedCString, OTParty*>;
using mapOfPartyAccounts = UnallocatedMap<UnallocatedCString, OTPartyAccount*>;
using mapOfVariables = UnallocatedMap<UnallocatedCString, OTVariable*>;

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
class OTScript
{
protected:
    UnallocatedCString script_;            // the script itself.
    UnallocatedCString display_filename_;  // for error handling, there is
                                           // option to set this string for
                                           // display.
    mapOfParties parties_;  // no need to clean this up. Script doesn't own
                            // the parties, just references them.
    mapOfPartyAccounts accounts_;  // no need to clean this up. Script
                                   // doesn't own the accounts, just
                                   // references them.
    mapOfVariables variables_;     // no need to clean this up. Script doesn't
                                   // own the variables, just references them.

    // List
    // Construction -- Destruction
public:
    OTScript();
    explicit OTScript(const String& strValue);
    explicit OTScript(const char* new_string);
    explicit OTScript(const UnallocatedCString& new_string);
    OTScript(const char* new_string, std::size_t sizeLength);

    virtual ~OTScript();

    virtual auto RegisterNativeScriptableCalls(OTScriptable& parent) noexcept
        -> void;
    virtual auto RegisterNativeSmartContractCalls(
        OTSmartContract& parent) noexcept -> void;
    void SetScript(const String& strValue);
    void SetScript(const char* new_string);
    void SetScript(const char* new_string, std::size_t sizeLength);
    void SetScript(const UnallocatedCString& new_string);

    void SetDisplayFilename(UnallocatedCString str_display_filename)
    {
        display_filename_ = str_display_filename;
    }

    // The same OTSmartContract that loads all the clauses (scripts) will
    // also load all the parties, so it will call this function whenever before
    // it
    // needs to actually run a script.
    //
    // NOTE: OTScript does NOT take ownership of the party, since there could be
    // multiple scripts (with all scripts and parties being owned by a
    // OTSmartContract.)
    // Therefore it's ASSUMED that the owner OTSmartContract will handle all the
    // work of
    // cleaning up the mess!  theParty is passed as reference to insure it
    // already exists.
    //
    void AddParty(UnallocatedCString str_party_name, OTParty& theParty);
    void AddAccount(UnallocatedCString str_acct_name, OTPartyAccount& theAcct);
    void AddVariable(UnallocatedCString str_var_name, OTVariable& theVar);
    auto FindVariable(UnallocatedCString str_var_name) -> OTVariable*;
    void RemoveVariable(OTVariable& theVar);

    // Note: any relevant assets or asset accounts are listed by their owner /
    // contributor
    // parties. Therefore there's no need to separately input any accounts or
    // assets to
    // a script, since the necessary ones are already present inside their
    // respective parties.

    virtual auto ExecuteScript(OTVariable* pReturnVar = nullptr) -> bool;
};
}  // namespace opentxs
