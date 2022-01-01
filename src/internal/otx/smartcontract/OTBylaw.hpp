// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <map>
#include <string>

#include "internal/otx/smartcontract/OTVariable.hpp"
#include "opentxs/core/String.hpp"

namespace opentxs
{
class OTClause;
class OTScript;
class OTScriptable;
class Tag;

using mapOfCallbacks = std::pmr::map<std::string, std::string>;
using mapOfClauses = std::pmr::map<std::string, OTClause*>;
using mapOfVariables = std::pmr::map<std::string, OTVariable*>;

// First is the name of some standard OT hook, like OnActivate, and Second is
// name of clause.
// It's a multimap because you might have 6 or 7 clauses that all trigger on the
// same hook.
//
using mapOfHooks = std::pmr::multimap<std::string, std::string>;

// A section of law, including its own clauses (scripts). A bylaw is kind of
// like an OT script "program", so it makes sense to be able to collect them,
// and to have them as discrete "packages".
//
class OTBylaw
{
    OTString m_strName;      // Name of this Bylaw.
    OTString m_strLanguage;  // Language that the scripts are written in, for
                             // this bylaw.

    mapOfVariables m_mapVariables;  // constant, persistant, and important
                                    // variables (strings and longs)
    mapOfClauses m_mapClauses;  // map of scripts associated with this bylaw.

    mapOfHooks m_mapHooks;  // multimap of server hooks associated with clauses.
                            // string / string
    mapOfCallbacks m_mapCallbacks;  // map of standard callbacks associated with
                                    // script clauses. string / string

    OTScriptable* m_pOwnerAgreement;  // This Bylaw is owned by an agreement
                                      // (OTScriptable-derived.)
    OTBylaw(const OTBylaw&) = delete;
    OTBylaw(OTBylaw&&) = delete;
    auto operator=(const OTBylaw&) -> OTBylaw& = delete;
    auto operator=(OTBylaw&&) -> OTBylaw& = delete;

public:
    auto GetName() const -> const String& { return m_strName; }
    auto GetLanguage() const -> const char*;
    auto AddVariable(OTVariable& theVariable) -> bool;
    auto AddVariable(
        std::string str_Name,
        std::string str_Value,
        OTVariable::OTVariable_Access theAccess = OTVariable::Var_Persistent)
        -> bool;
    auto AddVariable(
        std::string str_Name,
        std::int32_t nValue,
        OTVariable::OTVariable_Access theAccess = OTVariable::Var_Persistent)
        -> bool;
    auto AddVariable(
        std::string str_Name,
        bool bValue,
        OTVariable::OTVariable_Access theAccess = OTVariable::Var_Persistent)
        -> bool;
    auto AddClause(OTClause& theClause) -> bool;
    auto AddClause(const char* szName, const char* szCode) -> bool;
    auto AddHook(
        std::string str_HookName,
        std::string str_ClauseName) -> bool;  // name of hook such
                                              // as cron_process or
                                              // hook_activate, and
                                              // name of clause,
                                              // such as sectionA
                                              // (corresponding to
                                              // an actual script
                                              // in the clauses
                                              // map.)
    auto AddCallback(
        std::string str_CallbackName,
        std::string str_ClauseName) -> bool;  // name of
                                              // callback such
                                              // as
    // callback_party_may_execute_clause,
    // and name of clause, such as
    // custom_party_may_execute_clause
    // (corresponding to an actual script
    // in the clauses map.)

    auto RemoveVariable(std::string str_Name) -> bool;
    auto RemoveClause(std::string str_Name) -> bool;
    auto RemoveHook(std::string str_Name, std::string str_ClauseName) -> bool;
    auto RemoveCallback(std::string str_Name) -> bool;

    auto UpdateClause(std::string str_Name, std::string str_Code) -> bool;

    auto GetVariable(std::string str_Name) -> OTVariable*;  // not a
                                                            // reference,
                                                            // so you can
                                                            // pass in
                                                            // char *.
                                                            // Maybe
                                                            // that's
                                                            // bad? todo:
                                                            // research
                                                            // that.
    auto GetClause(std::string str_Name) const -> OTClause*;
    auto GetCallback(std::string str_CallbackName) -> OTClause*;
    auto GetHooks(
        std::string str_HookName,
        mapOfClauses& theResults) -> bool;  // Look up all clauses
                                            // matching a specific hook.
    auto GetVariableCount() const -> std::int32_t
    {
        return static_cast<std::int32_t>(m_mapVariables.size());
    }
    auto GetClauseCount() const -> std::int32_t
    {
        return static_cast<std::int32_t>(m_mapClauses.size());
    }
    auto GetCallbackCount() const -> std::int32_t
    {
        return static_cast<std::int32_t>(m_mapCallbacks.size());
    }
    auto GetHookCount() const -> std::int32_t
    {
        return static_cast<std::int32_t>(m_mapHooks.size());
    }
    auto GetVariableByIndex(std::int32_t nIndex) -> OTVariable*;
    auto GetClauseByIndex(std::int32_t nIndex) -> OTClause*;
    auto GetCallbackByIndex(std::int32_t nIndex) -> OTClause*;
    auto GetHookByIndex(std::int32_t nIndex) -> OTClause*;
    auto GetCallbackNameByIndex(std::int32_t nIndex) -> const std::string;
    auto GetHookNameByIndex(std::int32_t nIndex) -> const std::string;
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
    auto GetOwnerAgreement() -> OTScriptable* { return m_pOwnerAgreement; }
    void SetOwnerAgreement(OTScriptable& theOwner)
    {
        m_pOwnerAgreement = &theOwner;
    }
    OTBylaw();
    OTBylaw(const char* szName, const char* szLanguage);
    virtual ~OTBylaw();

    auto Compare(OTBylaw& rhs) -> bool;

    void Serialize(Tag& parent, bool bCalculatingID = false) const;
};

}  // namespace opentxs
