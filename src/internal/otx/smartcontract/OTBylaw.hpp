// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "internal/otx/smartcontract/OTVariable.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class OTClause;
class OTScript;
class OTScriptable;
class Tag;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
using mapOfCallbacks = UnallocatedMap<UnallocatedCString, UnallocatedCString>;
using mapOfClauses = UnallocatedMap<UnallocatedCString, OTClause*>;
using mapOfVariables = UnallocatedMap<UnallocatedCString, OTVariable*>;

// First is the name of some standard OT hook, like OnActivate, and Second is
// name of clause.
// It's a multimap because you might have 6 or 7 clauses that all trigger on the
// same hook.
//
using mapOfHooks = UnallocatedMultimap<UnallocatedCString, UnallocatedCString>;

// A section of law, including its own clauses (scripts). A bylaw is kind of
// like an OT script "program", so it makes sense to be able to collect them,
// and to have them as discrete "packages".
//
class OTBylaw
{
    OTString name_;      // Name of this Bylaw.
    OTString language_;  // Language that the scripts are written in, for
                         // this bylaw.

    mapOfVariables variables_;  // constant, persistant, and important
                                // variables (strings and longs)
    mapOfClauses clauses_;      // map of scripts associated with this bylaw.

    mapOfHooks hooks_;  // multimap of server hooks associated with clauses.
                        // string / string
    mapOfCallbacks callbacks_;  // map of standard callbacks associated with
                                // script clauses. string / string

    OTScriptable* owner_agreement_;  // This Bylaw is owned by an agreement
                                     // (OTScriptable-derived.)

public:
    auto GetName() const -> const String& { return name_; }
    auto GetLanguage() const -> const char*;
    auto AddVariable(OTVariable& theVariable) -> bool;
    auto AddVariable(
        UnallocatedCString str_Name,
        UnallocatedCString str_Value,
        OTVariable::OTVariable_Access theAccess = OTVariable::Var_Persistent)
        -> bool;
    auto AddVariable(
        UnallocatedCString str_Name,
        std::int32_t nValue,
        OTVariable::OTVariable_Access theAccess = OTVariable::Var_Persistent)
        -> bool;
    auto AddVariable(
        UnallocatedCString str_Name,
        bool bValue,
        OTVariable::OTVariable_Access theAccess = OTVariable::Var_Persistent)
        -> bool;
    auto AddClause(OTClause& theClause) -> bool;
    auto AddClause(const char* szName, const char* szCode) -> bool;
    auto AddHook(
        UnallocatedCString str_HookName,
        UnallocatedCString str_ClauseName) -> bool;  // name of hook such
                                                     // as cron_process or
                                                     // hook_activate, and
                                                     // name of clause,
                                                     // such as sectionA
                                                     // (corresponding to
                                                     // an actual script
                                                     // in the clauses
                                                     // map.)
    auto AddCallback(
        UnallocatedCString str_CallbackName,
        UnallocatedCString str_ClauseName) -> bool;  // name of
                                                     // callback such
                                                     // as
    // callback_party_may_execute_clause,
    // and name of clause, such as
    // custom_party_may_execute_clause
    // (corresponding to an actual script
    // in the clauses map.)

    auto RemoveVariable(UnallocatedCString str_Name) -> bool;
    auto RemoveClause(UnallocatedCString str_Name) -> bool;
    auto RemoveHook(
        UnallocatedCString str_Name,
        UnallocatedCString str_ClauseName) -> bool;
    auto RemoveCallback(UnallocatedCString str_Name) -> bool;

    auto UpdateClause(UnallocatedCString str_Name, UnallocatedCString str_Code)
        -> bool;

    auto GetVariable(UnallocatedCString str_Name) -> OTVariable*;  // not a
                                                                   // reference,
                                                                   // so you can
                                                                   // pass in
                                                                   // char *.
                                                                   // Maybe
                                                                   // that's
                                                                   // bad? todo:
                                                                   // research
                                                                   // that.
    auto GetClause(UnallocatedCString str_Name) const -> OTClause*;
    auto GetCallback(UnallocatedCString str_CallbackName) -> OTClause*;
    auto GetHooks(
        UnallocatedCString str_HookName,
        mapOfClauses& theResults) -> bool;  // Look up all clauses
                                            // matching a specific hook.
    auto GetVariableCount() const -> std::int32_t
    {
        return static_cast<std::int32_t>(variables_.size());
    }
    auto GetClauseCount() const -> std::int32_t
    {
        return static_cast<std::int32_t>(clauses_.size());
    }
    auto GetCallbackCount() const -> std::int32_t
    {
        return static_cast<std::int32_t>(callbacks_.size());
    }
    auto GetHookCount() const -> std::int32_t
    {
        return static_cast<std::int32_t>(hooks_.size());
    }
    auto GetVariableByIndex(std::int32_t nIndex) -> OTVariable*;
    auto GetClauseByIndex(std::int32_t nIndex) -> OTClause*;
    auto GetCallbackByIndex(std::int32_t nIndex) -> OTClause*;
    auto GetHookByIndex(std::int32_t nIndex) -> OTClause*;
    auto GetCallbackNameByIndex(std::int32_t nIndex)
        -> const UnallocatedCString;
    auto GetHookNameByIndex(std::int32_t nIndex) -> const UnallocatedCString;
    void RegisterVariablesForExecution(OTScript& theScript);
    auto IsDirty() const -> bool;  // So you can tell if any of the
                                   // persistent or important variables
                                   // have CHANGED since it was last set
                                   // clean.
    auto IsDirtyImportant() const -> bool;  // So you can tell if ONLY
                                            // the IMPORTANT variables
                                            // have CHANGED since it was
                                            // last set clean.
    void SetAsClean();  // Sets the variables as clean, so you
                        // can check
    // later and see if any have been changed (if it's
    // DIRTY again.)
    // This pointer isn't owned -- just stored for convenience.
    //
    auto GetOwnerAgreement() -> OTScriptable* { return owner_agreement_; }
    void SetOwnerAgreement(OTScriptable& theOwner)
    {
        owner_agreement_ = &theOwner;
    }

    auto Compare(OTBylaw& rhs) -> bool;

    void Serialize(Tag& parent, bool bCalculatingID = false) const;

    OTBylaw();
    OTBylaw(const char* szName, const char* szLanguage);
    OTBylaw(const OTBylaw&) = delete;
    OTBylaw(OTBylaw&&) = delete;
    auto operator=(const OTBylaw&) -> OTBylaw& = delete;
    auto operator=(OTBylaw&&) -> OTBylaw& = delete;

    virtual ~OTBylaw();
};
}  // namespace opentxs
