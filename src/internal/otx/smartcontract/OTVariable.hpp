// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/core/String.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs
{
class OTBylaw;
class OTScript;
class OTVariable;
class Tag;

class OTVariable
{
public:
    enum OTVariable_Type {
        Var_String,   // UnallocatedCString
        Var_Integer,  // Integer. (For std::int64_t std::int32_t: use strings.)
        Var_Bool,     // Boolean. (True / False)
        Var_Error_Type  // should never happen.
    };

    enum OTVariable_Access {
        Var_Constant,     // Constant   -- you cannot change this value.
        Var_Persistent,   // Persistent -- changing value doesn't require notice
                          // to parties.
        Var_Important,    // Important  -- changing value requires notice to
                          // parties.
        Var_Error_Access  // should never happen.
    };

private:
    OTString m_strName;              // Name of this variable.
    UnallocatedCString m_str_Value;  // If a string, the value is stored here.
    std::int32_t m_nValue{};         // If an integer, the value is stored here.
    bool m_bValue{false};            // If a bool, the value is stored here.
    UnallocatedCString m_str_ValueBackup;  // If a string, the value backup is
                                           // stored here. (So we can see if it
                                           // has changed since execution)
    std::int32_t m_nValueBackup{};  // If an integer, the value backup is stored
                                    // here.
    // (So we can see if it has changed since execution)
    bool m_bValueBackup{false};  // If a bool, the value backup is stored here.
                                 // (So we can check for dirtiness later...)
    OTBylaw* m_pBylaw{nullptr};  // the Bylaw that this variable belongs to.
    OTVariable_Type m_Type{Var_Error_Type};  // Currently bool, std::int32_t, or
                                             // string.
    OTVariable_Access m_Access{Var_Error_Access};  // Determines how the
                                                   // variable is used inside
                                                   // the script.
    OTScript* m_pScript{nullptr};  // If the variable is set onto a script, this
                                   // pointer gets set. When the variable
                                   // destructs, it will remove itself from the
                                   // script.

    OTVariable(const OTVariable&) = delete;
    OTVariable(OTVariable&&) = delete;
    auto operator=(const OTVariable&) -> OTVariable& = delete;
    auto operator=(OTVariable&&) -> OTVariable& = delete;

public:
    void RegisterForExecution(OTScript& theScript);  // We keep an
                                                     // internal script
                                                     // pointer here, so
    // if we destruct, we
    // can remove
    // ourselves from the
    // script.
    void UnregisterScript();       // If the script destructs before
                                   // the variable does, it
                                   // unregisters itself here, so the
                                   // variable isn't stuck with a bad
                                   // pointer.
    auto IsDirty() const -> bool;  // So you can tell if the variable has
                                   // CHANGED since it was last set clean.
    void SetAsClean();  // Sets the variable as clean, so you can check it later
                        // and see if it's been changed (if it's DIRTY again.)
    auto IsConstant() const -> bool { return (Var_Constant == m_Access); }
    auto IsPersistent() const -> bool
    {
        return ((Var_Persistent == m_Access) || (Var_Important == m_Access));
    }  // important vars are persistent, too.
    auto IsImportant() const -> bool { return (Var_Important == m_Access); }
    void SetBylaw(OTBylaw& theBylaw) { m_pBylaw = &theBylaw; }
    auto SetValue(const std::int32_t& nValue) -> bool;
    auto SetValue(bool bValue) -> bool;
    auto SetValue(const UnallocatedCString& str_Value) -> bool;

    auto GetName() const -> const String&
    {
        return m_strName;
    }  // variable's name as used in a script.
    auto GetType() const -> OTVariable_Type { return m_Type; }
    auto GetAccess() const -> OTVariable_Access { return m_Access; }

    auto IsInteger() const -> bool { return (Var_Integer == m_Type); }
    auto IsBool() const -> bool { return (Var_Bool == m_Type); }
    auto IsString() const -> bool { return (Var_String == m_Type); }

    auto CopyValueInteger() const -> std::int32_t { return m_nValue; }
    auto CopyValueBool() const -> bool { return m_bValue; }
    auto CopyValueString() const -> UnallocatedCString { return m_str_Value; }

    auto GetValueInteger() -> std::int32_t& { return m_nValue; }
    auto GetValueBool() -> bool& { return m_bValue; }
    auto GetValueString() -> UnallocatedCString& { return m_str_Value; }

    auto Compare(OTVariable& rhs) -> bool;

    OTVariable();
    OTVariable(
        const UnallocatedCString& str_Name,
        const std::int32_t nValue,
        const OTVariable_Access theAccess = Var_Persistent);
    OTVariable(
        const UnallocatedCString& str_Name,
        const bool bValue,
        const OTVariable_Access theAccess = Var_Persistent);
    OTVariable(
        const UnallocatedCString& str_Name,
        const UnallocatedCString& str_Value,
        const OTVariable_Access theAccess = Var_Persistent);
    virtual ~OTVariable();

    void Serialize(Tag& parent, bool bCalculatingID = false) const;
};

}  // namespace opentxs
