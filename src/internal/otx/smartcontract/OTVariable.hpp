// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "internal/core/String.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class OTBylaw;
class OTScript;
class Tag;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
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
    OTString name_;              // Name of this variable.
    UnallocatedCString string_;  // If a string, the value is stored here.
    std::int32_t number_{};      // If an integer, the value is stored here.
    bool bool_{false};           // If a bool, the value is stored here.
    UnallocatedCString string_backup_;  // If a string, the value backup is
                                        // stored here. (So we can see if it
                                        // has changed since execution)
    std::int32_t number_backup_{};  // If an integer, the value backup is stored
                                    // here.
    // (So we can see if it has changed since execution)
    bool bool_backup_{false};  // If a bool, the value backup is stored here.
                               // (So we can check for dirtiness later...)
    OTBylaw* bylaw_{nullptr};  // the Bylaw that this variable belongs to.
    OTVariable_Type type_{Var_Error_Type};  // Currently bool, std::int32_t, or
                                            // string.
    OTVariable_Access access_{Var_Error_Access};  // Determines how the
                                                  // variable is used inside
                                                  // the script.
    OTScript* script_{nullptr};  // If the variable is set onto a script, this
                                 // pointer gets set. When the variable
                                 // destructs, it will remove itself from the
                                 // script.

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
    auto IsConstant() const -> bool { return (Var_Constant == access_); }
    auto IsPersistent() const -> bool
    {
        return ((Var_Persistent == access_) || (Var_Important == access_));
    }  // important vars are persistent, too.
    auto IsImportant() const -> bool { return (Var_Important == access_); }
    void SetBylaw(OTBylaw& theBylaw) { bylaw_ = &theBylaw; }
    auto SetValue(const std::int32_t& nValue) -> bool;
    auto SetValue(bool bValue) -> bool;
    auto SetValue(const UnallocatedCString& str_Value) -> bool;

    auto GetName() const -> const String&
    {
        return name_;
    }  // variable's name as used in a script.
    auto GetType() const -> OTVariable_Type { return type_; }
    auto GetAccess() const -> OTVariable_Access { return access_; }

    auto IsInteger() const -> bool { return (Var_Integer == type_); }
    auto IsBool() const -> bool { return (Var_Bool == type_); }
    auto IsString() const -> bool { return (Var_String == type_); }

    auto CopyValueInteger() const -> std::int32_t { return number_; }
    auto CopyValueBool() const -> bool { return bool_; }
    auto CopyValueString() const -> UnallocatedCString { return string_; }

    auto GetValueInteger() -> std::int32_t& { return number_; }
    auto GetValueBool() -> bool& { return bool_; }
    auto GetValueString() -> UnallocatedCString& { return string_; }

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
    OTVariable(const OTVariable&) = delete;
    OTVariable(OTVariable&&) = delete;
    auto operator=(const OTVariable&) -> OTVariable& = delete;
    auto operator=(OTVariable&&) -> OTVariable& = delete;

    virtual ~OTVariable();

    void Serialize(Tag& parent, bool bCalculatingID = false) const;
};

}  // namespace opentxs
