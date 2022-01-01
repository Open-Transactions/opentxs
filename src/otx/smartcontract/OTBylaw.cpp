// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                            // IWYU pragma: associated
#include "1_Internal.hpp"                          // IWYU pragma: associated
#include "internal/otx/smartcontract/OTBylaw.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <list>
#include <memory>
#include <set>
#include <string>
#include <utility>

#include "internal/otx/smartcontract/OTClause.hpp"
#include "internal/otx/smartcontract/OTVariable.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/core/script/OTScriptable.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
OTBylaw::OTBylaw()
    : m_strName(String::Factory())
    , m_strLanguage(String::Factory())
    , m_mapVariables()
    , m_mapClauses()
    , m_mapHooks()
    , m_mapCallbacks()
    , m_pOwnerAgreement(nullptr)
{
}

OTBylaw::OTBylaw(const char* szName, const char* szLanguage)
    : m_strName(String::Factory())
    , m_strLanguage(String::Factory())
    , m_mapVariables()
    , m_mapClauses()
    , m_mapHooks()
    , m_mapCallbacks()
    , m_pOwnerAgreement(nullptr)
{
    if (nullptr != szName)
        m_strName->Set(szName);
    else
        LogError()(OT_PRETTY_CLASS())(
            "nullptr szName passed in to OTBylaw::OTBylaw.")
            .Flush();

    if (nullptr != szLanguage)
        m_strLanguage =
            String::Factory(szLanguage);  // "chai", "angelscript" etc.
    else
        LogError()(OT_PRETTY_CLASS())(
            "nullptr szLanguage passed in to OTBylaw::OTBylaw.")
            .Flush();

    const std::string str_bylaw_name = m_strName->Get();
    const std::string str_language = m_strLanguage->Get();

    // Let the calling function validate these, if he doesn't want to risk an
    // ASSERT...
    //
    if (!OTScriptable::ValidateName(str_bylaw_name) ||
        !OTScriptable::ValidateName(str_language)) {
        LogError()(OT_PRETTY_CLASS())(
            "Failed validation in to OTBylaw::OTBylaw.")
            .Flush();
    }
}

void OTBylaw::Serialize(Tag& parent, bool bCalculatingID) const
{
    TagPtr pTag(new Tag("bylaw"));

    pTag->add_attribute("name", m_strName->Get());
    pTag->add_attribute("language", m_strLanguage->Get());

    const std::uint64_t numVariables = m_mapVariables.size();
    const std::uint64_t numClauses = m_mapClauses.size();
    const std::uint64_t numHooks = m_mapHooks.size();
    const std::uint64_t numCallbacks = m_mapCallbacks.size();

    pTag->add_attribute("numVariables", std::to_string(numVariables));
    pTag->add_attribute("numClauses", std::to_string(numClauses));
    pTag->add_attribute("numHooks", std::to_string(numHooks));
    pTag->add_attribute("numCallbacks", std::to_string(numCallbacks));

    for (auto& it : m_mapVariables) {
        OTVariable* pVar = it.second;
        OT_ASSERT(nullptr != pVar);
        // Variables save in a specific state during ID calculation (no matter
        // their current actual value.)
        pVar->Serialize(*pTag, bCalculatingID);
    }

    for (auto& it : m_mapClauses) {
        OTClause* pClause = it.second;
        OT_ASSERT(nullptr != pClause);

        pClause->Serialize(*pTag);
    }

    for (auto& it : m_mapHooks) {
        const std::string& str_hook_name = it.first;
        const std::string& str_clause_name = it.second;

        TagPtr pTagHook(new Tag("hook"));

        pTagHook->add_attribute("name", str_hook_name);
        pTagHook->add_attribute("clause", str_clause_name);

        pTag->add_tag(pTagHook);
    }

    for (auto& it : m_mapCallbacks) {
        const std::string& str_callback_name = it.first;
        const std::string& str_clause_name = it.second;

        TagPtr pTagCallback(new Tag("callback"));

        pTagCallback->add_attribute("name", str_callback_name);
        pTagCallback->add_attribute("clause", str_clause_name);

        pTag->add_tag(pTagCallback);
    }

    parent.add_tag(pTag);
}

// So you can tell if the persistent or important variables have CHANGED since
// it was last set clean.
//
auto OTBylaw::IsDirty() const -> bool
{
    bool bIsDirty = false;

    for (const auto& it : m_mapVariables) {
        OTVariable* pVar = it.second;
        OT_ASSERT(nullptr != pVar);

        // "Persistent" *AND* "Important" Variables are both considered
        // "persistent".
        // Important has the added distinction that notices are required when
        // important variables change.
        //
        if (pVar->IsDirty()) {
            if (pVar->IsPersistent()) {
                bIsDirty = true;
                break;
            } else  // If it's not persistent (which also includes important)
                // the only other option is CONSTANT. Then why is it dirty?
                LogError()(OT_PRETTY_CLASS())(
                    "Error: Why is it that a variable "
                    "is CONSTANT, yet DIRTY at the same time?")
                    .Flush();
        }
    }

    return bIsDirty;
}

// So you can tell if ONLY the IMPORTANT variables have changed since the last
// "set clean".
//
auto OTBylaw::IsDirtyImportant() const -> bool
{
    bool bIsDirty = false;

    for (const auto& it : m_mapVariables) {
        OTVariable* pVar = it.second;
        OT_ASSERT(nullptr != pVar);

        // "Persistent" *AND* "Important" Variables are both considered
        // "persistent".
        // But: Important has the added distinction that notices are required
        // when important variables change.
        // (So sometimes you need to know if important variables have changed,
        // so you know whether to send a notice.)
        //
        if (pVar->IsDirty() && pVar->IsImportant()) {
            bIsDirty = true;
            break;
        }
    }

    return bIsDirty;
}

// Sets the variables as clean, so you can check later and see if any have been
// changed (if it's DIRTY again.)
//
void OTBylaw::SetAsClean()
{
    for (auto& it : m_mapVariables) {
        OTVariable* pVar = it.second;
        OT_ASSERT(nullptr != pVar);

        pVar->SetAsClean();  // so we can check for dirtiness later, if it's
                             // changed.
    }
}

// Register the variables of a specific Bylaw into the Script interpreter,
// so we can execute a script.
//
void OTBylaw::RegisterVariablesForExecution(OTScript& theScript)
{
    for (auto& it : m_mapVariables) {
        const std::string str_var_name = it.first;
        OTVariable* pVar = it.second;
        OT_ASSERT((nullptr != pVar) && (str_var_name.size() > 0));

        pVar->RegisterForExecution(theScript);
    }
}

// Done:
auto OTBylaw::Compare(OTBylaw& rhs) -> bool
{
    if ((m_strName->Compare(rhs.GetName())) &&
        (m_strLanguage->Compare(rhs.GetLanguage()))) {
        if (GetVariableCount() != rhs.GetVariableCount()) {
            LogConsole()(OT_PRETTY_CLASS())(
                "The variable count doesn't match for "
                "bylaw: ")(m_strName)(".")
                .Flush();
            return false;
        }
        if (GetClauseCount() != rhs.GetClauseCount()) {
            LogConsole()(OT_PRETTY_CLASS())(
                "The clause count doesn't match for "
                "bylaw: ")(m_strName)(".")
                .Flush();
            return false;
        }
        if (GetHookCount() != rhs.GetHookCount()) {
            LogConsole()(OT_PRETTY_CLASS())("The hook count doesn't match for "
                                            "bylaw: ")(m_strName)(".")
                .Flush();
            return false;
        }
        if (GetCallbackCount() != rhs.GetCallbackCount()) {
            LogConsole()(OT_PRETTY_CLASS())(
                "The callback count doesn't match for "
                "bylaw: ")(m_strName)(".")
                .Flush();
            return false;
        }
        // THE COUNTS MATCH, Now let's look up each one by NAME and verify that
        // they match...

        for (const auto& it : m_mapVariables) {
            OTVariable* pVar = it.second;
            OT_ASSERT(nullptr != pVar);

            OTVariable* pVar2 = rhs.GetVariable(pVar->GetName().Get());

            if (nullptr == pVar2) {
                LogConsole()(OT_PRETTY_CLASS())("Failed: Variable not found: ")(
                    pVar->GetName())(".")
                    .Flush();
                return false;
            }
            if (!pVar->Compare(*pVar2)) {
                LogConsole()(OT_PRETTY_CLASS())(
                    "Failed comparison between 2 "
                    "variables named ")(pVar->GetName())(".")
                    .Flush();
                return false;
            }
        }

        for (const auto& it : m_mapClauses) {
            OTClause* pClause = it.second;
            OT_ASSERT(nullptr != pClause);

            OTClause* pClause2 = rhs.GetClause(pClause->GetName().Get());

            if (nullptr == pClause2) {
                LogConsole()(OT_PRETTY_CLASS())("Failed: Clause not found: ")(
                    pClause->GetName())(".")
                    .Flush();
                return false;
            }
            if (!pClause->Compare(*pClause2)) {
                LogConsole()(OT_PRETTY_CLASS())(
                    "Failed comparison between 2 "
                    "clauses named ")(pClause->GetName())(".")
                    .Flush();
                return false;
            }
        }

        for (const auto& it : m_mapCallbacks) {
            const std::string& str_callback_name = it.first;
            const std::string& str_clause_name = it.second;

            OTClause* pCallbackClause = GetCallback(str_callback_name);
            OTClause* pCallbackClause2 = rhs.GetCallback(str_callback_name);

            if (nullptr == pCallbackClause) {
                LogConsole()(OT_PRETTY_CLASS())(" Failed: Callback (")(
                    str_callback_name)(") clause (")(
                    str_clause_name)(") not found on this bylaw: ")(
                    m_strName)(".")
                    .Flush();
                return false;
            } else if (nullptr == pCallbackClause2) {
                LogConsole()(OT_PRETTY_CLASS())(" Failed: Callback (")(
                    str_callback_name)(") clause (")(
                    str_clause_name)(") not found on rhs bylaw: ")(
                    rhs.GetName())(".")
                    .Flush();
                return false;
            } else if (!(pCallbackClause->GetName().Compare(
                           pCallbackClause2->GetName()))) {
                LogConsole()(OT_PRETTY_CLASS())(" Failed: Callback (")(
                    str_callback_name)(") clause (")(
                    str_clause_name)(") on rhs has a different name (")(
                    pCallbackClause2->GetName())(") than *this bylaw: ")(
                    m_strName)(".")
                    .Flush();
                return false;
            }

            // OPTIMIZE: Since ALL the clauses are already compared, one-by-one,
            // in the above block, then we don't
            // actually HAVE to do a compare clause here. We just need to make
            // sure that we got them both via the same
            // name, and that the counts are the same (as already verified
            // above) and that should actually be good enough.
        }

        std::pmr::set<std::string> theHookSet;

        // There might be MANY entries with the SAME HOOK NAME. So we add them
        // all to a SET in order to get unique keys.
        for (const auto& it : m_mapHooks) {
            const std::string& str_hook_name = it.first;
            theHookSet.insert(str_hook_name);
        }
        // Now we loop through all the unique hook names, and get
        // the list of clauses for EACH bylaw for THAT HOOK.
        for (const auto& it_hook : theHookSet) {
            const std::string& str_hook_name = it_hook;

            mapOfClauses theHookClauses, theHookClauses2;

            if (!GetHooks(str_hook_name, theHookClauses) ||
                !rhs.GetHooks(str_hook_name, theHookClauses2)) {
                LogConsole()(OT_PRETTY_CLASS())("Failed finding hook (")(
                    str_hook_name)(") clauses on this bylaw or rhs bylaw: ")(
                    m_strName)(".")
                    .Flush();
                return false;
            }

            if (theHookClauses.size() != theHookClauses2.size()) {
                LogConsole()(OT_PRETTY_CLASS())("Hook (")(
                    str_hook_name)(") clauses count doesn't match between this "
                                   "bylaw and "
                                   "the rhs bylaw named: ")(m_strName)(".")
                    .Flush();
                return false;
            }

            for (auto& it : theHookClauses) {
                const std::string str_clause_name = it.first;
                OTClause* pClause = it.second;
                OT_ASSERT(nullptr != pClause);

                auto it_rhs = theHookClauses2.find(str_clause_name);

                if (theHookClauses2.end() == it_rhs) {
                    LogConsole()(OT_PRETTY_CLASS())(
                        "Unable to find hook clause (")(
                        str_clause_name)(") on rhs that was definitely present "
                                         "on "
                                         "*this. Bylaw: ")(m_strName)(".")
                        .Flush();
                    return false;
                }

                // OPTIMIZE: Since ALL the clauses are already compared,
                // one-by-one, in an above block, then we don't
                // actually HAVE to do a compare clause here. We just need to
                // make sure that we got them both via the same
                // name, and that the counts are the same (as already verified
                // above) and that should actually be good enough.
            }
        }

        return true;
    }

    return false;
}

auto OTBylaw::GetCallbackNameByIndex(std::int32_t nIndex) -> const std::string
{
    if ((nIndex < 0) ||
        (nIndex >= static_cast<std::int64_t>(m_mapCallbacks.size()))) {
        LogError()(OT_PRETTY_CLASS())("Index out of bounds: ")(nIndex)(".")
            .Flush();
    } else {
        std::int32_t nLoopIndex = -1;

        for (auto& it : m_mapCallbacks) {
            const std::string& str_callback_name = it.first;
            ++nLoopIndex;  // 0 on first iteration.
            if (nLoopIndex == nIndex) return str_callback_name;
        }
    }
    return "";
}

auto OTBylaw::GetCallback(std::string str_Name) -> OTClause*
{
    if (false == OTScriptable::ValidateCallbackName(str_Name)) {
        LogError()(OT_PRETTY_CLASS())("Invalid Callback name: ")(str_Name)(".")
            .Flush();
        return nullptr;
    }
    // -----------------------------------------
    auto it = m_mapCallbacks.find(str_Name);

    if (m_mapCallbacks.end() != it)  // Found it!
    {
        //      const std::string& str_callback_name = it->first;
        const std::string& str_clause_name = it->second;

        OTClause* pClause = GetClause(str_clause_name);

        if (nullptr != pClause)  // found it
        {
            return pClause;
        } else {
            LogConsole()(OT_PRETTY_CLASS())("Couldn't find clause (")(
                str_clause_name)(") that was registered for callback (")(
                str_Name)(")")(".")
                .Flush();
        }
    }

    return nullptr;
}

auto OTBylaw::RemoveVariable(std::string str_Name) -> bool
{
    if (!OTScriptable::ValidateVariableName(str_Name)) {
        LogError()(OT_PRETTY_CLASS())("Error: Invalid str_Name.").Flush();
        return false;
    }

    auto it = m_mapVariables.find(str_Name);

    if (m_mapVariables.end() != it)  // Found it.
    {
        OTVariable* pVar = it->second;
        OT_ASSERT(nullptr != pVar);

        m_mapVariables.erase(it);
        delete pVar;
        pVar = nullptr;
        return true;
    }

    return false;
}

auto OTBylaw::RemoveClause(std::string str_Name) -> bool
{
    if (!OTScriptable::ValidateClauseName(str_Name)) {
        LogError()(OT_PRETTY_CLASS())("Failed: Empty or invalid str_Name.")
            .Flush();
        return false;
    }

    auto it = m_mapClauses.find(str_Name);

    if (m_mapClauses.end() == it) return false;
    // -----------------------------------
    OTClause* pClause = it->second;
    OT_ASSERT(nullptr != pClause);

    // At this point we have the clause name (str_Name)
    // so we go ahead and delete the clause itself, and
    // remove it from the map.
    //
    m_mapClauses.erase(it);

    delete pClause;
    pClause = nullptr;
    // -----------------------------------
    // AFTER we have deleted/remove the clause (above) THEN we
    // try and remove any associated callbacks and hooks.
    // Why AFTER? Because RemoveCallback calls RemoveClause again,
    // and we don't want this call to go into an infinite recursive loop.
    //
    std::pmr::list<std::string> listStrings;

    for (auto& cb : m_mapCallbacks) {
        const std::string& str_callback_name = cb.first;
        const std::string& str_clause_name = cb.second;

        if (0 == str_clause_name.compare(str_Name)) {
            listStrings.push_back(str_callback_name);
        }
    }

    while (listStrings.size() > 0) {
        const std::string str_callback_name = listStrings.front();
        listStrings.pop_front();
        RemoveCallback(str_callback_name);
    }

    for (auto& hook : m_mapHooks) {
        const std::string& str_hook_name = hook.first;
        const std::string& str_clause_name = hook.second;

        if (0 == str_clause_name.compare(str_Name)) {
            listStrings.push_back(str_hook_name);
        }
    }

    while (listStrings.size() > 0) {
        const std::string str_hook_name = listStrings.front();
        listStrings.pop_front();
        RemoveHook(str_hook_name, str_Name);
    }

    return true;
}

auto OTBylaw::RemoveHook(std::string str_Name, std::string str_ClauseName)
    -> bool
{
    if (!OTScriptable::ValidateHookName(str_Name)) {
        LogError()(OT_PRETTY_CLASS())("Failed: Empty or invalid str_Name.")
            .Flush();
        return false;
    }
    if (!OTScriptable::ValidateClauseName(str_ClauseName)) {
        LogError()(OT_PRETTY_CLASS())(
            "Failed: Empty or invalid str_ClauseName.")
            .Flush();
        return false;
    }
    // ----------------------------------------
    bool bReturnVal = false;

    for (auto it = m_mapHooks.begin(); it != m_mapHooks.end();) {
        const std::string& str_hook_name = it->first;
        const std::string& str_clause_name = it->second;

        if ((0 == str_hook_name.compare(str_Name)) &&
            (0 == str_clause_name.compare(str_ClauseName))) {
            it = m_mapHooks.erase(it);
            bReturnVal = true;
        } else
            ++it;
    }
    // ----------------------------------------
    return bReturnVal;
}

auto OTBylaw::RemoveCallback(std::string str_Name) -> bool
{
    if (false == OTScriptable::ValidateCallbackName(str_Name)) {
        LogError()(OT_PRETTY_CLASS())("Invalid Callback name: ")(str_Name)(".")
            .Flush();
        return false;
    }
    // -----------------------------------------
    auto it = m_mapCallbacks.find(str_Name);

    if (m_mapCallbacks.end() != it)  // Found it!
    {
        //      const std::string& str_callback_name = it->first;
        const std::string& str_clause_name = it->second;

        m_mapCallbacks.erase(it);
        // -----------------------------
        // AFTER erasing the callback (above), THEN we call RemoveClause.
        // Why AFTER? Because RemoveClause calls RemoveCallback again (and
        // RemoveHooks.) So I remove the callback first since this is recursive
        // and I don't want it to recurse forever.
        //
        OTClause* pClause = GetClause(str_clause_name);

        if (nullptr != pClause) RemoveClause(str_clause_name);

        return true;
    }

    LogError()(OT_PRETTY_CLASS())("Failed. No such callback: ")(str_Name)(".")
        .Flush();

    return false;
}

// You are NOT allowed to add multiple callbacks for any given callback trigger.
// There can be only one clause that answers to any given callback.
//
auto OTBylaw::AddCallback(
    std::string str_CallbackName,
    std::string str_ClauseName) -> bool
{
    // Make sure it's not already there...
    //
    auto it = m_mapCallbacks.find(str_CallbackName);

    if (m_mapCallbacks.end() != it)  // It's already there. (Can't add it
                                     // twice.)
    {
        const std::string str_existing_clause = it->second;
        LogConsole()(OT_PRETTY_CLASS())("Failed to add callback (")(
            str_CallbackName)(") to bylaw ")(m_strName)(", already there as ")(
            str_existing_clause)(".")
            .Flush();
        return false;
    }
    // Below this point, we know the callback wasn't already there.

    if (!OTScriptable::ValidateCallbackName(str_CallbackName) ||
        !OTScriptable::ValidateClauseName(str_ClauseName))
        LogError()(OT_PRETTY_CLASS())("Error: Empty or invalid name (")(
            str_CallbackName)(") or clause (")(str_ClauseName)(").")
            .Flush();
    else if (
        m_mapCallbacks.end() ==
        m_mapCallbacks.insert(
            m_mapCallbacks.begin(),
            std::pair<std::string, std::string>(
                str_CallbackName.c_str(), str_ClauseName.c_str())))
        LogError()(OT_PRETTY_CLASS())("Failed inserting to m_mapCallbacks: ")(
            str_CallbackName)(" / ")(str_ClauseName)(".")
            .Flush();
    else
        return true;

    return false;
}

// You ARE allowed to add multiple clauses for the same hook.
// They will ALL trigger on that hook.
//
auto OTBylaw::AddHook(std::string str_HookName, std::string str_ClauseName)
    -> bool
{
    if (!OTScriptable::ValidateHookName(str_HookName) ||
        !OTScriptable::ValidateClauseName(str_ClauseName)) {
        LogError()(OT_PRETTY_CLASS())("Error: Invalid or empty hook name (")(
            str_HookName)(") or clause name (")(str_ClauseName)(").")
            .Flush();
        return false;
    }
    // ----------------------------------------
    // See if it already exists.
    //
    for (auto& it : m_mapHooks) {
        const std::string& str_hook_name = it.first;
        const std::string& str_clause_name = it.second;

        if ((0 == str_hook_name.compare(str_HookName)) &&
            (0 == str_clause_name.compare(str_ClauseName))) {
            LogConsole()(OT_PRETTY_CLASS())("Failed: Hook already exists: ")(
                str_HookName)(". For clause name: ")(str_ClauseName)(".")
                .Flush();
            return false;
        }
    }
    // ------------------------
    // ------------------------
    // ----------------------------------------
    if (m_mapHooks.end() ==
        m_mapHooks.insert(std::pair<std::string, std::string>(
            str_HookName.c_str(), str_ClauseName.c_str())))
        LogError()(OT_PRETTY_CLASS())("Failed inserting to m_mapHooks: ")(
            str_HookName)(" / ")(str_ClauseName)(".")
            .Flush();
    else
        return true;

    return false;
}

auto OTBylaw::GetVariable(std::string str_var_name)
    -> OTVariable*  // not a
                    // reference,
                    // so you can
                    // pass in char
// *. Maybe that's bad? todo: research that.
{
    auto it = m_mapVariables.find(str_var_name);

    if (m_mapVariables.end() == it) return nullptr;

    if (!OTScriptable::ValidateVariableName(str_var_name)) {
        LogError()(OT_PRETTY_CLASS())("Error: Invalid variable name: ")(
            str_var_name)(".")
            .Flush();
        return nullptr;
    }

    OTVariable* pVar = it->second;
    OT_ASSERT(nullptr != pVar);

    return pVar;
}

/// Get Variable pointer by Index. Returns nullptr on failure.
///
auto OTBylaw::GetVariableByIndex(std::int32_t nIndex) -> OTVariable*
{
    if (!((nIndex >= 0) &&
          (nIndex < static_cast<std::int64_t>(m_mapVariables.size())))) {
        LogError()(OT_PRETTY_CLASS())("Index out of bounds: ")(nIndex)(".")
            .Flush();
    } else {
        std::int32_t nLoopIndex = -1;

        for (auto& it : m_mapVariables) {
            OTVariable* pVar = it.second;
            OT_ASSERT(nullptr != pVar);

            ++nLoopIndex;  // 0 on first iteration.

            if (nLoopIndex == nIndex) return pVar;
        }
    }
    return nullptr;
}

auto OTBylaw::GetClause(std::string str_clause_name) const -> OTClause*
{
    if (!OTScriptable::ValidateClauseName(str_clause_name)) {
        LogError()(OT_PRETTY_CLASS())("Error: Empty str_clause_name.").Flush();
        return nullptr;
    }

    auto it = m_mapClauses.find(str_clause_name);

    if (m_mapClauses.end() == it) return nullptr;

    OTClause* pClause = it->second;
    OT_ASSERT(nullptr != pClause);

    return pClause;
}

/// Get Clause pointer by Index. Returns nullptr on failure.
///
auto OTBylaw::GetClauseByIndex(std::int32_t nIndex) -> OTClause*
{
    if (!((nIndex >= 0) &&
          (nIndex < static_cast<std::int64_t>(m_mapClauses.size())))) {
        LogError()(OT_PRETTY_CLASS())("Index out of bounds: ")(nIndex)(".")
            .Flush();
    } else {
        std::int32_t nLoopIndex = -1;

        for (auto& it : m_mapClauses) {
            OTClause* pClause = it.second;
            OT_ASSERT(nullptr != pClause);

            ++nLoopIndex;  // 0 on first iteration.

            if (nLoopIndex == nIndex) return pClause;
        }
    }
    return nullptr;
}

auto OTBylaw::GetHookNameByIndex(std::int32_t nIndex) -> const std::string
{
    if ((nIndex < 0) ||
        (nIndex >= static_cast<std::int64_t>(m_mapHooks.size()))) {
        LogError()(OT_PRETTY_CLASS())("Index out of bounds: ")(nIndex)(".")
            .Flush();
    } else {
        std::int32_t nLoopIndex = -1;

        for (auto& it : m_mapHooks) {
            const std::string& str_hook_name = it.first;
            ++nLoopIndex;  // 0 on first iteration.

            if (nLoopIndex == nIndex) return str_hook_name;
        }
    }
    return "";
}

// Returns a map of clause pointers (or not) based on the HOOK name.
// ANY clauses on the list for that hook. (There could be many for each hook.)
// "GetHooks" could have been termed,
// "GetAMapOfAllClausesRegisteredForTheHookWithName(str_HookName)
//
auto OTBylaw::GetHooks(std::string str_HookName, mapOfClauses& theResults)
    -> bool
{
    if (!OTScriptable::ValidateHookName(str_HookName)) {
        LogError()(OT_PRETTY_CLASS())("Error: Invalid str_HookName.").Flush();
        return false;
    }

    bool bReturnVal = false;

    for (auto& it : m_mapHooks) {
        const std::string& str_hook_name = it.first;
        const std::string& str_clause_name = it.second;

        // IF this entry (of a clause registered for a specific hook) MATCHES
        // the hook name passed in...
        //
        if (0 == (str_hook_name.compare(str_HookName))) {
            OTClause* pClause = GetClause(str_clause_name);

            if (nullptr != pClause)  // found it
            {
                // mapOfClauses is a map, meaning it will only allow one entry
                // per unique clause name.
                // Remember, mapOfHooks is a multimap, since there may be
                // multiple clauses registered to
                // the same hook. (Which is fine.) But what if someone registers
                // the SAME clause MULTIPLE
                // TIMES to the SAME HOOK? No need for that. So by the time the
                // clauses are inserted into
                // the result map, the duplicates are automatically weeded out.
                //
                if (theResults.end() != theResults.insert(
                                            theResults.begin(),
                                            std::pair<std::string, OTClause*>(
                                                str_clause_name, pClause)))
                    bReturnVal = true;
            } else {
                LogConsole()(OT_PRETTY_CLASS())("Couldn't find clause (")(
                    str_clause_name)(") that was registered for hook (")(
                    str_hook_name)(")")(".")
                    .Flush();
            }
        }
        // else no error, since it's normal for nothing to match.
    }

    return bReturnVal;
}

auto OTBylaw::AddVariable(OTVariable& theVariable) -> bool
{
    const std::string str_name = theVariable.GetName().Get();

    if (!OTScriptable::ValidateVariableName(str_name)) {
        LogError()(OT_PRETTY_CLASS())("Failed due to invalid variable name. "
                                      "In Bylaw: ")(m_strName)(".")
            .Flush();
        return false;
    }

    auto it = m_mapVariables.find(str_name);

    // Make sure it's not already there...
    //
    if (m_mapVariables.end() == it)  // If it wasn't already there...
    {
        // Then insert it...
        m_mapVariables.insert(
            std::pair<std::string, OTVariable*>(str_name, &theVariable));

        // Make sure it has a pointer back to me.
        theVariable.SetBylaw(*this);

        return true;
    } else {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed -- A variable was already there "
            "named: ")(str_name)(".")
            .Flush();
        return false;
    }
}

auto OTBylaw::AddVariable(
    std::string str_Name,
    bool bValue,
    OTVariable::OTVariable_Access theAccess) -> bool
{
    auto* pVar = new OTVariable(str_Name, bValue, theAccess);
    OT_ASSERT(nullptr != pVar);

    if (!AddVariable(*pVar)) {
        delete pVar;
        return false;
    }

    return true;
}

auto OTBylaw::AddVariable(
    std::string str_Name,
    std::string str_Value,
    OTVariable::OTVariable_Access theAccess) -> bool
{
    auto* pVar = new OTVariable(str_Name, str_Value, theAccess);
    OT_ASSERT(nullptr != pVar);

    if (!AddVariable(*pVar)) {
        delete pVar;
        return false;
    }

    return true;
}

auto OTBylaw::AddVariable(
    std::string str_Name,
    std::int32_t nValue,
    OTVariable::OTVariable_Access theAccess) -> bool
{
    auto* pVar = new OTVariable(str_Name, nValue, theAccess);
    OT_ASSERT(nullptr != pVar);

    if (!AddVariable(*pVar)) {
        delete pVar;
        return false;
    }

    return true;
}

auto OTBylaw::AddClause(const char* szName, const char* szCode) -> bool
{
    OT_ASSERT(nullptr != szName);
    //  OT_ASSERT(nullptr != szCode);

    // Note: name is validated in the AddClause call below.
    // (So I don't validate it here.)

    auto* pClause = new OTClause(szName, szCode);
    OT_ASSERT(nullptr != pClause);

    if (!AddClause(*pClause)) {
        delete pClause;
        return false;
    }

    return true;
}

auto OTBylaw::UpdateClause(std::string str_Name, std::string str_Code) -> bool
{
    if (!OTScriptable::ValidateClauseName(str_Name)) {
        LogError()(OT_PRETTY_CLASS())("Failed due to invalid clause name. In "
                                      "Bylaw: ")(m_strName)(".")
            .Flush();
        return false;
    }

    auto it = m_mapClauses.find(str_Name);

    if (m_mapClauses.end() == it)  // Didn't exist.
        return false;
    // -----------------------------------
    OTClause* pClause = it->second;
    OT_ASSERT(nullptr != pClause);

    pClause->SetCode(str_Code);

    return true;
}

auto OTBylaw::AddClause(OTClause& theClause) -> bool
{
    if (!theClause.GetName().Exists()) {
        LogError()(OT_PRETTY_CLASS())("Failed attempt to add a clause with a "
                                      "blank name.")
            .Flush();
        return false;
    }

    const std::string str_clause_name = theClause.GetName().Get();

    if (!OTScriptable::ValidateClauseName(str_clause_name)) {
        LogError()(OT_PRETTY_CLASS())("Failed due to invalid clause name. In "
                                      "Bylaw: ")(m_strName)(".")
            .Flush();
        return false;
    }

    auto it = m_mapClauses.find(str_clause_name);

    if (m_mapClauses.end() == it)  // If it wasn't already there...
    {
        // Then insert it...
        m_mapClauses.insert(
            std::pair<std::string, OTClause*>(str_clause_name, &theClause));

        // Make sure it has a pointer back to me.
        theClause.SetBylaw(*this);

        return true;
    } else {
        LogConsole()(OT_PRETTY_CLASS())(
            "Failed -- Clause was already there named ")(str_clause_name)(".")
            .Flush();

        return false;
    }
}

auto OTBylaw::GetLanguage() const -> const char*
{
    return m_strLanguage->Exists() ? m_strLanguage->Get()
                                   : "chai";  // todo add default script to
                                              // config files. no hardcoding.
}

OTBylaw::~OTBylaw()
{
    // A Bylaw owns its clauses and variables.
    //
    while (!m_mapClauses.empty()) {
        OTClause* pClause = m_mapClauses.begin()->second;
        OT_ASSERT(nullptr != pClause);

        m_mapClauses.erase(m_mapClauses.begin());

        delete pClause;
        pClause = nullptr;
    }

    while (!m_mapVariables.empty()) {
        OTVariable* pVar = m_mapVariables.begin()->second;
        OT_ASSERT(nullptr != pVar);

        m_mapVariables.erase(m_mapVariables.begin());

        delete pVar;
        pVar = nullptr;
    }

    m_pOwnerAgreement =
        nullptr;  // This Bylaw is owned by an agreement (OTScriptable-derived.)

    // Hooks and Callbacks are maps of std::string to std::string.
    //
    // (THEREFORE NO NEED TO CLEAN THEM UP HERE.)
    //
}

}  // namespace opentxs
