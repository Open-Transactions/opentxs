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

#ifndef OPENTXS_CORE_SCRIPT_OTBYLAW_HPP
#define OPENTXS_CORE_SCRIPT_OTBYLAW_HPP

#include "opentxs/core/script/OTVariable.hpp"
#include "opentxs/core/String.hpp"

#include <stdint.h>
#include <map>
#include <string>

namespace opentxs
{

class OTClause;
class OTScript;
class OTScriptable;
class Tag;

typedef std::map<std::string, std::string> mapOfCallbacks;
typedef std::map<std::string, OTClause*> mapOfClauses;
typedef std::map<std::string, OTVariable*> mapOfVariables;

// First is the name of some standard OT hook, like OnActivate, and Second is
// name of clause.
// It's a multimap because you might have 6 or 7 clauses that all trigger on the
// same hook.
//
typedef std::multimap<std::string, std::string> mapOfHooks;

// A section of law, including its own clauses (scripts). A bylaw is kind of
// like an OT script "program", so it makes sense to be able to collect them,
// and to have them as discrete "packages".
//
class OTBylaw
{
    String m_strName;     // Name of this Bylaw.
    String m_strLanguage; // Language that the scripts are written in, for
                          // this bylaw.

    mapOfVariables m_mapVariables; // constant, persistant, and important
                                   // variables (strings and longs)
    mapOfClauses m_mapClauses;     // map of scripts associated with this bylaw.

    mapOfHooks m_mapHooks; // multimap of server hooks associated with clauses.
                           // string / string
    mapOfCallbacks m_mapCallbacks; // map of standard callbacks associated with
                                   // script clauses. string / string

    OTScriptable* m_pOwnerAgreement; // This Bylaw is owned by an agreement
                                     // (OTScriptable-derived.)
public:
    EXPORT const String& GetName() const
    {
        return m_strName;
    }
    EXPORT const char* GetLanguage() const;
    EXPORT bool AddVariable(OTVariable& theVariable);
    EXPORT bool AddVariable(
        std::string str_Name, std::string str_Value,
        OTVariable::OTVariable_Access theAccess = OTVariable::Var_Persistent);
    EXPORT bool AddVariable(
        std::string str_Name, int32_t nValue,
        OTVariable::OTVariable_Access theAccess = OTVariable::Var_Persistent);
    EXPORT bool AddVariable(
        std::string str_Name, bool bValue,
        OTVariable::OTVariable_Access theAccess = OTVariable::Var_Persistent);
    EXPORT bool AddClause(OTClause& theClause);
    EXPORT bool AddClause(const char* szName, const char* szCode);
    EXPORT bool AddHook(std::string str_HookName,
                        std::string str_ClauseName); // name of hook such
                                                     // as cron_process or
                                                     // hook_activate, and
                                                     // name of clause,
                                                     // such as sectionA
                                                     // (corresponding to
                                                     // an actual script
                                                     // in the clauses
                                                     // map.)
    EXPORT bool AddCallback(std::string str_CallbackName,
                            std::string str_ClauseName); // name of
                                                         // callback such
                                                         // as
    // callback_party_may_execute_clause,
    // and name of clause, such as
    // custom_party_may_execute_clause
    // (corresponding to an actual script
    // in the clauses map.)

    EXPORT bool RemoveVariable(std::string str_Name);
    EXPORT bool RemoveClause(std::string str_Name);
    EXPORT bool RemoveHook(std::string str_Name, std::string str_ClauseName);
    EXPORT bool RemoveCallback(std::string str_Name);

    EXPORT bool UpdateClause(std::string str_Name, std::string str_Code);

    EXPORT OTVariable* GetVariable(std::string str_Name); // not a
                                                          // reference, so
                                                          // you can pass
                                                          // in char *.
                                                          // Maybe that's
                                                          // bad? todo:
                                                          // research
                                                          // that.
    EXPORT OTClause* GetClause(std::string str_Name) const;
    EXPORT OTClause* GetCallback(std::string str_CallbackName);
    EXPORT bool GetHooks(std::string str_HookName,
                         mapOfClauses& theResults); // Look up all clauses
                                                    // matching a specific hook.
    EXPORT int32_t GetVariableCount() const
    {
        return static_cast<int32_t>(m_mapVariables.size());
    }
    EXPORT int32_t GetClauseCount() const
    {
        return static_cast<int32_t>(m_mapClauses.size());
    }
    EXPORT int32_t GetCallbackCount() const
    {
        return static_cast<int32_t>(m_mapCallbacks.size());
    }
    EXPORT int32_t GetHookCount() const
    {
        return static_cast<int32_t>(m_mapHooks.size());
    }
    EXPORT OTVariable* GetVariableByIndex(int32_t nIndex);
    EXPORT OTClause* GetClauseByIndex(int32_t nIndex);
    EXPORT OTClause* GetCallbackByIndex(int32_t nIndex);
    EXPORT OTClause* GetHookByIndex(int32_t nIndex);
    EXPORT const std::string GetCallbackNameByIndex(int32_t nIndex);
    EXPORT const std::string GetHookNameByIndex(int32_t nIndex);
    EXPORT void RegisterVariablesForExecution(OTScript& theScript);
    EXPORT bool IsDirty() const; // So you can tell if any of the persistent or
                                 // important variables have CHANGED since it
                                 // was last set clean.
    EXPORT bool IsDirtyImportant() const; // So you can tell if ONLY the
                                          // IMPORTANT variables have CHANGED
                                          // since it was last set clean.
    EXPORT void SetAsClean(); // Sets the variables as clean, so you can check
                              // later and see if any have been changed (if it's
                              // DIRTY again.)
    // This pointer isn't owned -- just stored for convenience.
    //
    EXPORT OTScriptable* GetOwnerAgreement()
    {
        return m_pOwnerAgreement;
    }
    EXPORT void SetOwnerAgreement(OTScriptable& theOwner)
    {
        m_pOwnerAgreement = &theOwner;
    }
    EXPORT OTBylaw();
    EXPORT OTBylaw(const char* szName, const char* szLanguage);
    virtual ~OTBylaw();

    EXPORT bool Compare(OTBylaw& rhs);

    EXPORT void Serialize(Tag& parent, bool bCalculatingID = false) const;
};

} // namespace opentxs

#endif // OPENTXS_CORE_SCRIPT_OTBYLAW_HPP
