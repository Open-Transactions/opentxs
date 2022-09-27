// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                            // IWYU pragma: associated
#include "1_Internal.hpp"                          // IWYU pragma: associated
#include "internal/otx/smartcontract/OTBylaw.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <memory>
#include <utility>

#include "internal/otx/common/script/OTScriptable.hpp"
#include "internal/otx/common/util/Tag.hpp"
#include "internal/otx/smartcontract/OTClause.hpp"
#include "internal/otx/smartcontract/OTVariable.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
OTBylaw::OTBylaw()
    : name_(String::Factory())
    , language_(String::Factory())
    , variables_()
    , clauses_()
    , hooks_()
    , callbacks_()
    , owner_agreement_(nullptr)
{
}

OTBylaw::OTBylaw(const char* szName, const char* szLanguage)
    : name_(String::Factory())
    , language_(String::Factory())
    , variables_()
    , clauses_()
    , hooks_()
    , callbacks_()
    , owner_agreement_(nullptr)
{
    if (nullptr != szName) {
        name_->Set(szName);
    } else {
        LogError()(OT_PRETTY_CLASS())(
            "nullptr szName passed in to OTBylaw::OTBylaw.")
            .Flush();
    }

    if (nullptr != szLanguage) {
        language_ = String::Factory(szLanguage);  // "chai", "angelscript" etc.
    } else {
        LogError()(OT_PRETTY_CLASS())(
            "nullptr szLanguage passed in to OTBylaw::OTBylaw.")
            .Flush();
    }

    const UnallocatedCString str_bylaw_name = name_->Get();
    const UnallocatedCString str_language = language_->Get();

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

    pTag->add_attribute("name", name_->Get());
    pTag->add_attribute("language", language_->Get());

    const std::uint64_t numVariables = variables_.size();
    const std::uint64_t numClauses = clauses_.size();
    const std::uint64_t numHooks = hooks_.size();
    const std::uint64_t numCallbacks = callbacks_.size();

    pTag->add_attribute("numVariables", std::to_string(numVariables));
    pTag->add_attribute("numClauses", std::to_string(numClauses));
    pTag->add_attribute("numHooks", std::to_string(numHooks));
    pTag->add_attribute("numCallbacks", std::to_string(numCallbacks));

    for (const auto& it : variables_) {
        OTVariable* pVar = it.second;
        OT_ASSERT(nullptr != pVar);
        // Variables save in a specific state during ID calculation (no matter
        // their current actual value.)
        pVar->Serialize(*pTag, bCalculatingID);
    }

    for (const auto& it : clauses_) {
        OTClause* pClause = it.second;
        OT_ASSERT(nullptr != pClause);

        pClause->Serialize(*pTag);
    }

    for (const auto& it : hooks_) {
        const UnallocatedCString& str_hook_name = it.first;
        const UnallocatedCString& str_clause_name = it.second;

        TagPtr pTagHook(new Tag("hook"));

        pTagHook->add_attribute("name", str_hook_name);
        pTagHook->add_attribute("clause", str_clause_name);

        pTag->add_tag(pTagHook);
    }

    for (const auto& it : callbacks_) {
        const UnallocatedCString& str_callback_name = it.first;
        const UnallocatedCString& str_clause_name = it.second;

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

    for (const auto& it : variables_) {
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
            } else {  // If it's not persistent (which also includes important)
                // the only other option is CONSTANT. Then why is it dirty?
                LogError()(OT_PRETTY_CLASS())(
                    "Error: Why is it that a variable "
                    "is CONSTANT, yet DIRTY at the same time?")
                    .Flush();
            }
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

    for (const auto& it : variables_) {
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
    for (auto& it : variables_) {
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
    for (auto& it : variables_) {
        const UnallocatedCString str_var_name = it.first;
        OTVariable* pVar = it.second;
        OT_ASSERT((nullptr != pVar) && (str_var_name.size() > 0));

        pVar->RegisterForExecution(theScript);
    }
}

// Done:
auto OTBylaw::Compare(OTBylaw& rhs) -> bool
{
    if ((name_->Compare(rhs.GetName())) &&
        (language_->Compare(rhs.GetLanguage()))) {
        if (GetVariableCount() != rhs.GetVariableCount()) {
            LogConsole()(OT_PRETTY_CLASS())(
                "The variable count doesn't match for "
                "bylaw: ")(name_.get())(".")
                .Flush();
            return false;
        }
        if (GetClauseCount() != rhs.GetClauseCount()) {
            LogConsole()(OT_PRETTY_CLASS())(
                "The clause count doesn't match for "
                "bylaw: ")(name_.get())(".")
                .Flush();
            return false;
        }
        if (GetHookCount() != rhs.GetHookCount()) {
            LogConsole()(OT_PRETTY_CLASS())("The hook count doesn't match for "
                                            "bylaw: ")(name_.get())(".")
                .Flush();
            return false;
        }
        if (GetCallbackCount() != rhs.GetCallbackCount()) {
            LogConsole()(OT_PRETTY_CLASS())(
                "The callback count doesn't match for "
                "bylaw: ")(name_.get())(".")
                .Flush();
            return false;
        }
        // THE COUNTS MATCH, Now let's look up each one by NAME and verify that
        // they match...

        for (const auto& it : variables_) {
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

        for (const auto& it : clauses_) {
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

        for (const auto& it : callbacks_) {
            const UnallocatedCString& str_callback_name = it.first;
            const UnallocatedCString& str_clause_name = it.second;

            OTClause* pCallbackClause = GetCallback(str_callback_name);
            OTClause* pCallbackClause2 = rhs.GetCallback(str_callback_name);

            if (nullptr == pCallbackClause) {
                LogConsole()(OT_PRETTY_CLASS())(" Failed: Callback (")(
                    str_callback_name)(") clause (")(
                    str_clause_name)(") not found on this bylaw: ")(
                    name_.get())(".")
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
                    name_.get())(".")
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

        UnallocatedSet<UnallocatedCString> theHookSet;

        // There might be MANY entries with the SAME HOOK NAME. So we add them
        // all to a SET in order to get unique keys.
        for (const auto& it : hooks_) {
            const UnallocatedCString& str_hook_name = it.first;
            theHookSet.insert(str_hook_name);
        }
        // Now we loop through all the unique hook names, and get
        // the list of clauses for EACH bylaw for THAT HOOK.
        for (const auto& it_hook : theHookSet) {
            const UnallocatedCString& str_hook_name = it_hook;

            mapOfClauses theHookClauses, theHookClauses2;

            if (!GetHooks(str_hook_name, theHookClauses) ||
                !rhs.GetHooks(str_hook_name, theHookClauses2)) {
                LogConsole()(OT_PRETTY_CLASS())("Failed finding hook (")(
                    str_hook_name)(") clauses on this bylaw or rhs bylaw: ")(
                    name_.get())(".")
                    .Flush();
                return false;
            }

            if (theHookClauses.size() != theHookClauses2.size()) {
                LogConsole()(OT_PRETTY_CLASS())("Hook (")(
                    str_hook_name)(") clauses count doesn't match between this "
                                   "bylaw and the rhs bylaw named: ")(
                    name_.get())(".")
                    .Flush();
                return false;
            }

            for (auto& it : theHookClauses) {
                const UnallocatedCString str_clause_name = it.first;
                OTClause* pClause = it.second;
                OT_ASSERT(nullptr != pClause);

                auto it_rhs = theHookClauses2.find(str_clause_name);

                if (theHookClauses2.end() == it_rhs) {
                    LogConsole()(OT_PRETTY_CLASS())(
                        "Unable to find hook clause (")(
                        str_clause_name)(") on rhs that was definitely present "
                                         "on *this. Bylaw: ")(name_.get())(".")
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

auto OTBylaw::GetCallbackNameByIndex(std::int32_t nIndex)
    -> const UnallocatedCString
{
    if ((nIndex < 0) ||
        (nIndex >= static_cast<std::int64_t>(callbacks_.size()))) {
        LogError()(OT_PRETTY_CLASS())("Index out of bounds: ")(nIndex)(".")
            .Flush();
    } else {
        std::int32_t nLoopIndex = -1;

        for (auto& it : callbacks_) {
            const UnallocatedCString& str_callback_name = it.first;
            ++nLoopIndex;  // 0 on first iteration.
            if (nLoopIndex == nIndex) { return str_callback_name; }
        }
    }
    return "";
}

auto OTBylaw::GetCallback(UnallocatedCString str_Name) -> OTClause*
{
    if (false == OTScriptable::ValidateCallbackName(str_Name)) {
        LogError()(OT_PRETTY_CLASS())("Invalid Callback name: ")(str_Name)(".")
            .Flush();
        return nullptr;
    }
    // -----------------------------------------
    auto it = callbacks_.find(str_Name);

    if (callbacks_.end() != it)  // Found it!
    {
        //      const UnallocatedCString& str_callback_name = it->first;
        const UnallocatedCString& str_clause_name = it->second;

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

auto OTBylaw::RemoveVariable(UnallocatedCString str_Name) -> bool
{
    if (!OTScriptable::ValidateVariableName(str_Name)) {
        LogError()(OT_PRETTY_CLASS())("Error: Invalid str_Name.").Flush();
        return false;
    }

    auto it = variables_.find(str_Name);

    if (variables_.end() != it)  // Found it.
    {
        OTVariable* pVar = it->second;
        OT_ASSERT(nullptr != pVar);

        variables_.erase(it);
        delete pVar;
        pVar = nullptr;
        return true;
    }

    return false;
}

auto OTBylaw::RemoveClause(UnallocatedCString str_Name) -> bool
{
    if (!OTScriptable::ValidateClauseName(str_Name)) {
        LogError()(OT_PRETTY_CLASS())("Failed: Empty or invalid str_Name.")
            .Flush();
        return false;
    }

    auto it = clauses_.find(str_Name);

    if (clauses_.end() == it) { return false; }
    // -----------------------------------
    OTClause* pClause = it->second;
    OT_ASSERT(nullptr != pClause);

    // At this point we have the clause name (str_Name)
    // so we go ahead and delete the clause itself, and
    // remove it from the map.
    //
    clauses_.erase(it);

    delete pClause;
    pClause = nullptr;
    // -----------------------------------
    // AFTER we have deleted/remove the clause (above) THEN we
    // try and remove any associated callbacks and hooks.
    // Why AFTER? Because RemoveCallback calls RemoveClause again,
    // and we don't want this call to go into an infinite recursive loop.
    //
    UnallocatedList<UnallocatedCString> listStrings;

    for (auto& cb : callbacks_) {
        const UnallocatedCString& str_callback_name = cb.first;
        const UnallocatedCString& str_clause_name = cb.second;

        if (0 == str_clause_name.compare(str_Name)) {
            listStrings.push_back(str_callback_name);
        }
    }

    while (listStrings.size() > 0) {
        const UnallocatedCString str_callback_name = listStrings.front();
        listStrings.pop_front();
        RemoveCallback(str_callback_name);
    }

    for (auto& hook : hooks_) {
        const UnallocatedCString& str_hook_name = hook.first;
        const UnallocatedCString& str_clause_name = hook.second;

        if (0 == str_clause_name.compare(str_Name)) {
            listStrings.push_back(str_hook_name);
        }
    }

    while (listStrings.size() > 0) {
        const UnallocatedCString str_hook_name = listStrings.front();
        listStrings.pop_front();
        RemoveHook(str_hook_name, str_Name);
    }

    return true;
}

auto OTBylaw::RemoveHook(
    UnallocatedCString str_Name,
    UnallocatedCString str_ClauseName) -> bool
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

    for (auto it = hooks_.begin(); it != hooks_.end();) {
        const UnallocatedCString& str_hook_name = it->first;
        const UnallocatedCString& str_clause_name = it->second;

        if ((0 == str_hook_name.compare(str_Name)) &&
            (0 == str_clause_name.compare(str_ClauseName))) {
            it = hooks_.erase(it);
            bReturnVal = true;
        } else {
            ++it;
        }
    }
    // ----------------------------------------
    return bReturnVal;
}

auto OTBylaw::RemoveCallback(UnallocatedCString str_Name) -> bool
{
    if (false == OTScriptable::ValidateCallbackName(str_Name)) {
        LogError()(OT_PRETTY_CLASS())("Invalid Callback name: ")(str_Name)(".")
            .Flush();
        return false;
    }
    // -----------------------------------------
    auto it = callbacks_.find(str_Name);

    if (callbacks_.end() != it)  // Found it!
    {
        //      const UnallocatedCString& str_callback_name = it->first;
        const UnallocatedCString& str_clause_name = it->second;

        callbacks_.erase(it);
        // -----------------------------
        // AFTER erasing the callback (above), THEN we call RemoveClause.
        // Why AFTER? Because RemoveClause calls RemoveCallback again (and
        // RemoveHooks.) So I remove the callback first since this is recursive
        // and I don't want it to recurse forever.
        //
        OTClause* pClause = GetClause(str_clause_name);

        if (nullptr != pClause) { RemoveClause(str_clause_name); }

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
    UnallocatedCString str_CallbackName,
    UnallocatedCString str_ClauseName) -> bool
{
    // Make sure it's not already there...
    //
    auto it = callbacks_.find(str_CallbackName);

    if (callbacks_.end() != it)  // It's already there. (Can't add it
                                 // twice.)
    {
        const UnallocatedCString str_existing_clause = it->second;
        LogConsole()(OT_PRETTY_CLASS())("Failed to add callback (")(
            str_CallbackName)(") to bylaw ")(name_.get())(
            ", already there as ")(str_existing_clause)(".")
            .Flush();
        return false;
    }
    // Below this point, we know the callback wasn't already there.

    if (!OTScriptable::ValidateCallbackName(str_CallbackName) ||
        !OTScriptable::ValidateClauseName(str_ClauseName)) {
        LogError()(OT_PRETTY_CLASS())("Error: Empty or invalid name (")(
            str_CallbackName)(") or clause (")(str_ClauseName)(").")
            .Flush();
    } else if (
        callbacks_.end() ==
        callbacks_.insert(
            callbacks_.begin(),
            std::pair<UnallocatedCString, UnallocatedCString>(
                str_CallbackName.c_str(), str_ClauseName.c_str()))) {
        LogError()(OT_PRETTY_CLASS())("Failed inserting to callbacks_: ")(
            str_CallbackName)(" / ")(str_ClauseName)(".")
            .Flush();
    } else {
        return true;
    }

    return false;
}

// You ARE allowed to add multiple clauses for the same hook.
// They will ALL trigger on that hook.
//
auto OTBylaw::AddHook(
    UnallocatedCString str_HookName,
    UnallocatedCString str_ClauseName) -> bool
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
    for (auto& it : hooks_) {
        const UnallocatedCString& str_hook_name = it.first;
        const UnallocatedCString& str_clause_name = it.second;

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
    if (hooks_.end() ==
        hooks_.insert(std::pair<UnallocatedCString, UnallocatedCString>(
            str_HookName.c_str(), str_ClauseName.c_str()))) {
        LogError()(OT_PRETTY_CLASS())("Failed inserting to hooks_: ")(
            str_HookName)(" / ")(str_ClauseName)(".")
            .Flush();
    } else {
        return true;
    }

    return false;
}

auto OTBylaw::GetVariable(UnallocatedCString str_var_name)
    -> OTVariable*  // not a
                    // reference,
                    // so you can
                    // pass in char
// *. Maybe that's bad? todo: research that.
{
    auto it = variables_.find(str_var_name);

    if (variables_.end() == it) { return nullptr; }

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
          (nIndex < static_cast<std::int64_t>(variables_.size())))) {
        LogError()(OT_PRETTY_CLASS())("Index out of bounds: ")(nIndex)(".")
            .Flush();
    } else {
        std::int32_t nLoopIndex = -1;

        for (auto& it : variables_) {
            OTVariable* pVar = it.second;
            OT_ASSERT(nullptr != pVar);

            ++nLoopIndex;  // 0 on first iteration.

            if (nLoopIndex == nIndex) { return pVar; }
        }
    }
    return nullptr;
}

auto OTBylaw::GetClause(UnallocatedCString str_clause_name) const -> OTClause*
{
    if (!OTScriptable::ValidateClauseName(str_clause_name)) {
        LogError()(OT_PRETTY_CLASS())("Error: Empty str_clause_name.").Flush();
        return nullptr;
    }

    auto it = clauses_.find(str_clause_name);

    if (clauses_.end() == it) { return nullptr; }

    OTClause* pClause = it->second;
    OT_ASSERT(nullptr != pClause);

    return pClause;
}

/// Get Clause pointer by Index. Returns nullptr on failure.
///
auto OTBylaw::GetClauseByIndex(std::int32_t nIndex) -> OTClause*
{
    if (!((nIndex >= 0) &&
          (nIndex < static_cast<std::int64_t>(clauses_.size())))) {
        LogError()(OT_PRETTY_CLASS())("Index out of bounds: ")(nIndex)(".")
            .Flush();
    } else {
        std::int32_t nLoopIndex = -1;

        for (auto& it : clauses_) {
            OTClause* pClause = it.second;
            OT_ASSERT(nullptr != pClause);

            ++nLoopIndex;  // 0 on first iteration.

            if (nLoopIndex == nIndex) { return pClause; }
        }
    }
    return nullptr;
}

auto OTBylaw::GetHookNameByIndex(std::int32_t nIndex)
    -> const UnallocatedCString
{
    if ((nIndex < 0) || (nIndex >= static_cast<std::int64_t>(hooks_.size()))) {
        LogError()(OT_PRETTY_CLASS())("Index out of bounds: ")(nIndex)(".")
            .Flush();
    } else {
        std::int32_t nLoopIndex = -1;

        for (auto& it : hooks_) {
            const UnallocatedCString& str_hook_name = it.first;
            ++nLoopIndex;  // 0 on first iteration.

            if (nLoopIndex == nIndex) { return str_hook_name; }
        }
    }
    return "";
}

// Returns a map of clause pointers (or not) based on the HOOK name.
// ANY clauses on the list for that hook. (There could be many for each hook.)
// "GetHooks" could have been termed,
// "GetAMapOfAllClausesRegisteredForTheHookWithName(str_HookName)
//
auto OTBylaw::GetHooks(
    UnallocatedCString str_HookName,
    mapOfClauses& theResults) -> bool
{
    if (!OTScriptable::ValidateHookName(str_HookName)) {
        LogError()(OT_PRETTY_CLASS())("Error: Invalid str_HookName.").Flush();
        return false;
    }

    bool bReturnVal = false;

    for (auto& it : hooks_) {
        const UnallocatedCString& str_hook_name = it.first;
        const UnallocatedCString& str_clause_name = it.second;

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
                if (theResults.end() !=
                    theResults.insert(
                        theResults.begin(),
                        std::pair<UnallocatedCString, OTClause*>(
                            str_clause_name, pClause))) {
                    bReturnVal = true;
                }
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
    const UnallocatedCString str_name = theVariable.GetName().Get();

    if (!OTScriptable::ValidateVariableName(str_name)) {
        LogError()(OT_PRETTY_CLASS())(
            "Failed due to invalid variable name. In Bylaw: ")(name_.get())(".")
            .Flush();
        return false;
    }

    auto it = variables_.find(str_name);

    // Make sure it's not already there...
    //
    if (variables_.end() == it)  // If it wasn't already there...
    {
        // Then insert it...
        variables_.insert(
            std::pair<UnallocatedCString, OTVariable*>(str_name, &theVariable));

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
    UnallocatedCString str_Name,
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
    UnallocatedCString str_Name,
    UnallocatedCString str_Value,
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
    UnallocatedCString str_Name,
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

auto OTBylaw::UpdateClause(
    UnallocatedCString str_Name,
    UnallocatedCString str_Code) -> bool
{
    if (!OTScriptable::ValidateClauseName(str_Name)) {
        LogError()(OT_PRETTY_CLASS())(
            "Failed due to invalid clause name. In Bylaw: ")(name_.get())(".")
            .Flush();
        return false;
    }

    auto it = clauses_.find(str_Name);

    if (clauses_.end() == it) {  // Didn't exist.
        return false;
    }
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

    const UnallocatedCString str_clause_name = theClause.GetName().Get();

    if (!OTScriptable::ValidateClauseName(str_clause_name)) {
        LogError()(OT_PRETTY_CLASS())(
            "Failed due to invalid clause name. In Bylaw: ")(name_.get())(".")
            .Flush();
        return false;
    }

    auto it = clauses_.find(str_clause_name);

    if (clauses_.end() == it)  // If it wasn't already there...
    {
        // Then insert it...
        clauses_.insert(std::pair<UnallocatedCString, OTClause*>(
            str_clause_name, &theClause));

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
    return language_->Exists() ? language_->Get()
                               : "chai";  // todo add default script to
                                          // config files. no hardcoding.
}

OTBylaw::~OTBylaw()
{
    // A Bylaw owns its clauses and variables.
    //
    while (!clauses_.empty()) {
        OTClause* pClause = clauses_.begin()->second;
        OT_ASSERT(nullptr != pClause);

        clauses_.erase(clauses_.begin());

        delete pClause;
        pClause = nullptr;
    }

    while (!variables_.empty()) {
        OTVariable* pVar = variables_.begin()->second;
        OT_ASSERT(nullptr != pVar);

        variables_.erase(variables_.begin());

        delete pVar;
        pVar = nullptr;
    }

    owner_agreement_ =
        nullptr;  // This Bylaw is owned by an agreement (OTScriptable-derived.)

    // Hooks and Callbacks are maps of UnallocatedCString to UnallocatedCString.
    //
    // (THEREFORE NO NEED TO CLEAN THEM UP HERE.)
    //
}

}  // namespace opentxs
