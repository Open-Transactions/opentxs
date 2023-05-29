// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/otx/smartcontract/OTVariable.hpp"  // IWYU pragma: associated

#include <cstdint>
#include <memory>

#include "internal/core/Armored.hpp"
#include "internal/core/String.hpp"
#include "internal/otx/common/util/Common.hpp"
#include "internal/otx/common/util/Tag.hpp"
#include "internal/otx/smartcontract/OTScript.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
void OTVariable::Serialize(
    const api::Crypto& crypto,
    Tag& parent,
    bool bCalculatingID) const
{
    UnallocatedCString str_access(""), str_type("");

    switch (access_) {
        // This cannot be changed from inside the script.
        case OTVariable::Var_Constant: {
            str_access = "constant";
        } break;
        // This can be changed without notifying the parties.
        case OTVariable::Var_Persistent: {
            str_access = "persistent";
        } break;
        // This cannot be changed without notifying the parties.
        case OTVariable::Var_Important: {
            str_access = "important";
        } break;
        case OTVariable::Var_Error_Access:
        default: {
            LogError()(OT_PRETTY_CLASS())("ERROR: Bad variable access.")
                .Flush();
        }
    }

    TagPtr pTag(new Tag("variable"));

    pTag->add_attribute("name", name_->Get());
    pTag->add_attribute("access", str_access);

    // Notice the use of bCalculatingID. Because
    // we don't serialize the variable's value when
    // calculating the smart contract's ID.
    switch (type_) {
        case OTVariable::Var_String: {
            str_type = "string";
            if ((false == bCalculatingID) && (string_.size() > 0)) {
                auto strVal = String::Factory(string_.c_str());
                auto ascVal = Armored::Factory(crypto, strVal);
                pTag->add_attribute("value", "exists");
                pTag->set_text(ascVal->Get());
            } else {
                pTag->add_attribute("value", "none");
            }
        } break;
        case OTVariable::Var_Integer: {
            str_type = "integer";
            pTag->add_attribute(
                "value", std::to_string(bCalculatingID ? 0 : number_));
        } break;
        case OTVariable::Var_Bool: {
            str_type = "bool";
            pTag->add_attribute(
                "value", bCalculatingID ? "false" : formatBool(bool_));
        } break;
        case OTVariable::Var_Error_Type:
        default: {
            LogError()(OT_PRETTY_CLASS())("ERROR: Bad variable type.").Flush();
        }
    }

    pTag->add_attribute("type", str_type);

    parent.add_tag(pTag);
}

// NO TYPE (YET)
OTVariable::OTVariable()
    : name_(String::Factory())
    , string_()
    , number_(0)
    , bool_(false)
    , string_backup_()
    , number_backup_(0)
    , bool_backup_(false)
    , bylaw_(nullptr)
    , type_(OTVariable::Var_Error_Type)
    , access_(Var_Error_Access)
    , script_(nullptr)
{
}

// STRING
OTVariable::OTVariable(
    const UnallocatedCString& str_Name,
    const UnallocatedCString& str_Value,
    const OTVariable_Access theAccess)
    : name_(String::Factory(str_Name.c_str()))
    , string_(str_Value)
    , number_(0)
    , bool_(false)
    , string_backup_(str_Value)
    , number_backup_(0)
    , bool_backup_(false)
    , bylaw_(nullptr)
    , type_(OTVariable::Var_String)
    , access_(theAccess)
    , script_(nullptr)
{
    if (string_.empty()) { string_ = ""; }
    if (string_backup_.empty()) { string_backup_ = ""; }
}

// INT
OTVariable::OTVariable(
    const UnallocatedCString& str_Name,
    const std::int32_t nValue,
    const OTVariable_Access theAccess)
    : name_(String::Factory(str_Name.c_str()))
    , string_()
    , number_(nValue)
    , bool_(false)
    , string_backup_()
    , number_backup_(nValue)
    , bool_backup_(false)
    , bylaw_(nullptr)
    , type_(OTVariable::Var_Integer)
    , access_(theAccess)
    , script_(nullptr)
{
}

// BOOL
OTVariable::OTVariable(
    const UnallocatedCString& str_Name,
    const bool bValue,
    const OTVariable_Access theAccess)
    : name_(String::Factory(str_Name.c_str()))
    , string_()
    , number_(0)
    , bool_(bValue)
    , string_backup_()
    , number_backup_(0)
    , bool_backup_(bValue)
    , bylaw_(nullptr)
    , type_(OTVariable::Var_Bool)
    , access_(theAccess)
    , script_(nullptr)
{
}

OTVariable::~OTVariable()
{
    if (nullptr != script_) { script_->RemoveVariable(*this); }

    script_ =
        nullptr;  // I wasn't the owner, it was a pointer for convenience only.
    bylaw_ =
        nullptr;  // I wasn't the owner, it was a pointer for convenience only.
}

auto OTVariable::SetValue(const std::int32_t& nValue) -> bool
{
    if (!IsInteger()) {
        LogError()(OT_PRETTY_CLASS())("Error: This variable (")(name_.get())(
            ") is not an integer.")
            .Flush();
        return false;
    }

    number_ = number_backup_ = nValue;

    return true;
}

auto OTVariable::SetValue(bool bValue) -> bool
{
    if (!IsBool()) {
        LogError()(OT_PRETTY_CLASS())("Error: This variable (")(name_.get())(
            ") is not a bool.")
            .Flush();
        return false;
    }

    bool_ = bool_backup_ = bValue;

    return true;
}

auto OTVariable::SetValue(const UnallocatedCString& str_Value) -> bool
{
    if (!IsString()) {
        LogError()(OT_PRETTY_CLASS())("Error: This variable (")(name_.get())(
            ") is not a string.")
            .Flush();
        return false;
    }

    string_ = string_backup_ = str_Value;

    if (string_.empty()) { string_ = ""; }
    if (string_backup_.empty()) { string_backup_ = ""; }

    return true;
}

// So you can tell if the variable has CHANGED since it was last set clean.
// (Usually you set clean just before running the script, and you check dirty
// just AFTER running the script.)
//
auto OTVariable::IsDirty() const -> bool
{
    bool bReturnVal = false;

    switch (type_) {
        case OTVariable::Var_String: {
            if (0 != string_.compare(string_backup_)) {  // If they do
                                                         // NOT
                // match, then it's
                // dirty.
                bReturnVal = true;
            }
        } break;
        case OTVariable::Var_Integer: {
            if (number_ != number_backup_) {  // If they do NOT match, then
                                              // it's dirty.
                bReturnVal = true;
            }
        } break;
        case OTVariable::Var_Bool: {
            if (bool_ != bool_backup_) {  // If they do NOT match, then
                                          // it's dirty.
                bReturnVal = true;
            }
        } break;
        case OTVariable::Var_Error_Type:
        default: {
            LogError()(OT_PRETTY_CLASS())("Error: Unknown type for variable: ")(
                name_.get())(".")
                .Flush();
        }
    }

    return bReturnVal;
}

// Sets the variable as clean, so you can check it later and see if it's been
// changed (if it's DIRTY again.)
void OTVariable::SetAsClean()
{
    switch (type_) {
        case OTVariable::Var_String: {
            string_backup_ = string_;  // Save a copy of the current
                                       // value, so we can check later
                                       // and see if they're different.
        } break;
        case OTVariable::Var_Integer: {
            number_backup_ = number_;  // Save a copy of the current value, so
                                       // we can check later and see if they're
                                       // different.
        } break;
        case OTVariable::Var_Bool: {
            bool_backup_ = bool_;  // Save a copy of the current value, so
                                   // we can check later and see if they're
                                   // different.
        } break;
        case OTVariable::Var_Error_Type:
        default: {
            LogError()(OT_PRETTY_CLASS())("Error: Unknown type for variable: ")(
                name_.get())(".")
                .Flush();
            string_backup_ = string_;
            number_backup_ = number_;
            bool_backup_ = bool_;
        }
    }
}

// If the script destructs before the variable does, it unregisters
// itself here, so the variable isn't stuck with a bad pointer.
//
void OTVariable::UnregisterScript() { script_ = nullptr; }

// We keep an internal script pointer here, so if we destruct,
// we can remove ourselves from the script.
//
void OTVariable::RegisterForExecution(OTScript& theScript)
{
    SetAsClean();  // so we can check for dirtiness after execution.

    const UnallocatedCString str_var_name = name_->Get();

    theScript.AddVariable(str_var_name, *this);

    script_ = &theScript;  // So later, if the variable destructs, and
                           // this pointer is set, the variable can
                           // remove itself from the script.
}

// Done
auto OTVariable::Compare(OTVariable& rhs) -> bool
{
    if (!(GetName().Compare(rhs.GetName()))) {
        {
            LogConsole()(OT_PRETTY_CLASS())("Names don't match: ")(GetName())(
                " / ")(rhs.GetName())(".")
                .Flush();
        }
        return false;
    }
    if (!(GetType() == rhs.GetType())) {
        {
            LogConsole()(OT_PRETTY_CLASS())("Type doesn't match: ")(GetName())(
                ".")
                .Flush();
        }
        return false;
    }
    if (!(GetAccess() == rhs.GetAccess())) {
        {
            LogConsole()(OT_PRETTY_CLASS())("Access tyes don't match: ")(
                GetName())(".")
                .Flush();
        }
        return false;
    }

    bool bMatch = false;

    switch (GetType()) {
        case OTVariable::Var_Integer: {
            bMatch = (GetValueInteger() == rhs.GetValueInteger());
        } break;
        case OTVariable::Var_Bool: {
            bMatch = (GetValueBool() == rhs.GetValueBool());
        } break;
        case OTVariable::Var_String: {
            bMatch = (GetValueString().compare(rhs.GetValueString()) == 0);
        } break;
        case OTVariable::Var_Error_Type:
        default: {
            LogError()(OT_PRETTY_CLASS())("Unknown type in variable ")(
                name_.get())(".")
                .Flush();
        }
    }

    return bMatch;
}

}  // namespace opentxs
