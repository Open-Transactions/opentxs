// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/script/OTVariable.hpp"

#include "opentxs/core/script/OTScript.hpp"
#include "opentxs/core/util/Common.hpp"
#include "opentxs/core/util/Tag.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"

#include <cstdint>
#include <memory>
#include <ostream>
#include <string>

#define OT_METHOD "opentxs::OTVariable::"

namespace opentxs
{

void OTVariable::Serialize(Tag& parent, bool bCalculatingID) const
{
    std::string str_access(""), str_type("");

    switch (m_Access) {
        // This cannot be changed from inside the
        // script.
        case OTVariable::Var_Constant:
            str_access = "constant";
            break;
        // This can be changed without notifying
        // the parties.
        case OTVariable::Var_Persistent:
            str_access = "persistent";
            break;
        // This cannot be changed without notifying
        // the parties.
        case OTVariable::Var_Important:
            str_access = "important";
            break;
        default:
            LogOutput(OT_METHOD)(__FUNCTION__)(": ERROR: Bad variable access.")
                .Flush();
            break;
    }

    TagPtr pTag(new Tag("variable"));

    pTag->add_attribute("name", m_strName->Get());
    pTag->add_attribute("access", str_access);

    // Notice the use of bCalculatingID. Because
    // we don't serialize the variable's value when
    // calculating the smart contract's ID.
    switch (m_Type) {
        case OTVariable::Var_String: {
            str_type = "string";
            if ((false == bCalculatingID) && (m_str_Value.size() > 0)) {
                auto strVal = String::Factory(m_str_Value.c_str());
                auto ascVal = Armored::Factory(strVal);
                pTag->add_attribute("value", "exists");
                pTag->set_text(ascVal->Get());
            } else {
                pTag->add_attribute("value", "none");
            }
        } break;
        case OTVariable::Var_Integer:
            str_type = "integer";
            pTag->add_attribute(
                "value", formatInt(bCalculatingID ? 0 : m_nValue));
            break;
        case OTVariable::Var_Bool:
            str_type = "bool";
            pTag->add_attribute(
                "value", bCalculatingID ? "false" : formatBool(m_bValue));
            break;
        default:
            LogOutput(OT_METHOD)(__FUNCTION__)(": ERROR: Bad variable type.")
                .Flush();
            break;
    }

    pTag->add_attribute("type", str_type);

    parent.add_tag(pTag);
}

// NO TYPE (YET)
OTVariable::OTVariable()
    : m_strName(String::Factory())
    , m_nValue(0)
    , m_bValue(false)
    , m_nValueBackup(0)
    , m_bValueBackup(false)
    , m_pBylaw(nullptr)
    , m_Type(OTVariable::Var_Error_Type)
    , m_Access(Var_Error_Access)
    , m_pScript(nullptr)
{
}

// STRING
OTVariable::OTVariable(
    const std::string& str_Name,
    const std::string& str_Value,
    const OTVariable_Access theAccess)
    : m_strName(String::Factory(str_Name.c_str()))
    , m_str_Value(str_Value)
    , m_nValue(0)
    , m_bValue(false)
    , m_str_ValueBackup(str_Value)
    , m_nValueBackup(0)
    , m_bValueBackup(false)
    , m_pBylaw(nullptr)
    , m_Type(OTVariable::Var_String)
    , m_Access(theAccess)
    , m_pScript(nullptr)
{
    if (m_str_Value.empty()) m_str_Value = "";
    if (m_str_ValueBackup.empty()) m_str_ValueBackup = "";
}

// INT
OTVariable::OTVariable(
    const std::string& str_Name,
    const std::int32_t nValue,
    const OTVariable_Access theAccess)
    : m_strName(String::Factory(str_Name.c_str()))
    , m_nValue(nValue)
    , m_bValue(false)
    , m_nValueBackup(nValue)
    , m_bValueBackup(false)
    , m_pBylaw(nullptr)
    , m_Type(OTVariable::Var_Integer)
    , m_Access(theAccess)
    , m_pScript(nullptr)
{
}

// BOOL
OTVariable::OTVariable(
    const std::string& str_Name,
    const bool bValue,
    const OTVariable_Access theAccess)
    : m_strName(String::Factory(str_Name.c_str()))
    , m_nValue(0)
    , m_bValue(bValue)
    , m_nValueBackup(0)
    , m_bValueBackup(bValue)
    , m_pBylaw(nullptr)
    , m_Type(OTVariable::Var_Bool)
    , m_Access(theAccess)
    , m_pScript(nullptr)
{
}

OTVariable::~OTVariable()
{
    if (nullptr != m_pScript) { m_pScript->RemoveVariable(*this); }

    m_pScript =
        nullptr;  // I wasn't the owner, it was a pointer for convenience only.
    m_pBylaw =
        nullptr;  // I wasn't the owner, it was a pointer for convenience only.
}

bool OTVariable::SetValue(const std::int32_t& nValue)
{
    if (!IsInteger()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error: This variable (")(
            m_strName)(") is not an integer.")
            .Flush();
        return false;
    }

    m_nValue = m_nValueBackup = nValue;

    return true;
}

bool OTVariable::SetValue(bool bValue)
{
    if (!IsBool()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error: This variable (")(
            m_strName)(") is not a bool.")
            .Flush();
        return false;
    }

    m_bValue = m_bValueBackup = bValue;

    return true;
}

bool OTVariable::SetValue(const std::string& str_Value)
{
    if (!IsString()) {
        LogOutput(OT_METHOD)(__FUNCTION__)(": Error: This variable (")(
            m_strName)(") is not a string.")
            .Flush();
        return false;
    }

    m_str_Value = m_str_ValueBackup = str_Value;

    if (m_str_Value.empty()) m_str_Value = "";
    if (m_str_ValueBackup.empty()) m_str_ValueBackup = "";

    return true;
}

// So you can tell if the variable has CHANGED since it was last set clean.
// (Usually you set clean just before running the script, and you check dirty
// just AFTER running the script.)
//
bool OTVariable::IsDirty() const
{
    bool bReturnVal = false;

    switch (m_Type) {
        case OTVariable::Var_String:
            if (0 != m_str_Value.compare(m_str_ValueBackup))  // If they do NOT
                // match, then it's
                // dirty.
                bReturnVal = true;
            break;
        case OTVariable::Var_Integer:
            if (m_nValue != m_nValueBackup)  // If they do NOT match, then it's
                                             // dirty.
                bReturnVal = true;
            break;
        case OTVariable::Var_Bool:
            if (m_bValue != m_bValueBackup)  // If they do NOT match, then it's
                                             // dirty.
                bReturnVal = true;
            break;
        default:
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error: Unknown type for variable: ")(m_strName)(".")
                .Flush();
            break;
    }

    return bReturnVal;
}

// Sets the variable as clean, so you can check it later and see if it's been
// changed (if it's DIRTY again.)
void OTVariable::SetAsClean()
{
    switch (m_Type) {
        case OTVariable::Var_String:
            m_str_ValueBackup = m_str_Value;  // Save a copy of the current
                                              // value, so we can check later
                                              // and see if they're different.
            break;
        case OTVariable::Var_Integer:
            m_nValueBackup = m_nValue;  // Save a copy of the current value, so
                                        // we can check later and see if they're
                                        // different.
            break;
        case OTVariable::Var_Bool:
            m_bValueBackup = m_bValue;  // Save a copy of the current value, so
                                        // we can check later and see if they're
                                        // different.
            break;
        default:
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error: Unknown type for variable: ")(m_strName)(".")
                .Flush();
            m_str_ValueBackup = m_str_Value;
            m_nValueBackup = m_nValue;
            m_bValueBackup = m_bValue;
            break;
    }
}

// If the script destructs before the variable does, it unregisters
// itself here, so the variable isn't stuck with a bad pointer.
//
void OTVariable::UnregisterScript() { m_pScript = nullptr; }

// We keep an internal script pointer here, so if we destruct,
// we can remove ourselves from the script.
//
void OTVariable::RegisterForExecution(OTScript& theScript)
{
    SetAsClean();  // so we can check for dirtiness after execution.

    const std::string str_var_name = m_strName->Get();

    theScript.AddVariable(str_var_name, *this);

    m_pScript = &theScript;  // So later, if the variable destructs, and
                             // this pointer is set, the variable can
                             // remove itself from the script.
}

// Done
bool OTVariable::Compare(OTVariable& rhs)
{
    if (!(GetName().Compare(rhs.GetName()))) {
        {
            LogNormal(OT_METHOD)(__FUNCTION__)(": Names don't match: ")(
                GetName())(" / ")(rhs.GetName())(".")
                .Flush();
        }
        return false;
    }
    if (!(GetType() == rhs.GetType())) {
        {
            LogNormal(OT_METHOD)(__FUNCTION__)(": Type doesn't match: ")(
                GetName())(".")
                .Flush();
        }
        return false;
    }
    if (!(GetAccess() == rhs.GetAccess())) {
        {
            LogNormal(OT_METHOD)(__FUNCTION__)(": Access tyes don't match: ")(
                GetName())(".")
                .Flush();
        }
        return false;
    }

    bool bMatch = false;

    switch (GetType()) {
        case OTVariable::Var_Integer:
            bMatch = (GetValueInteger() == rhs.GetValueInteger());
            break;
        case OTVariable::Var_Bool:
            bMatch = (GetValueBool() == rhs.GetValueBool());
            break;
        case OTVariable::Var_String:
            bMatch = (GetValueString().compare(rhs.GetValueString()) == 0);
            break;
        default:
            LogOutput(OT_METHOD)(__FUNCTION__)(": Unknown type in variable ")(
                m_strName)(".")
                .Flush();
            break;
    }

    return bMatch;
}

}  // namespace opentxs
