// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/script/OTClause.hpp"

#include "opentxs/core/util/Tag.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"

#include <memory>
#include <ostream>
#include <string>

// ------------- OPERATIONS -------------
// Below this point, have all the actions that a party might do.
//
// (The party will internally call the appropriate agent according to its own
// rules.
// the script should not care how the party chooses its agents. At the most, the
// script
// only cares that the party has an active agent, but does not actually speak
// directly
// to said agent.)

#define OT_METHOD "opentxs::OTClause::"

namespace opentxs
{

OTClause::OTClause()
    : m_strName(String::Factory())
    , m_strCode(String::Factory())
    , m_pBylaw(nullptr)
{
}

OTClause::OTClause(const char* szName, const char* szCode)
    : m_strName(String::Factory())
    , m_strCode(String::Factory())
    , m_pBylaw(nullptr)
{
    if (nullptr != szName) { m_strName->Set(szName); }
    if (nullptr != szCode) { m_strCode->Set(szCode); }

    // Todo security:  validation on the above fields.
}

OTClause::~OTClause()
{
    // nothing to delete.

    m_pBylaw =
        nullptr;  // I wasn't the owner, it was a pointer for convenience only.
}

void OTClause::SetCode(const std::string& str_code)
{
    m_strCode->Set(str_code.c_str());
}

const char* OTClause::GetCode() const
{
    if (m_strCode->Exists()) return m_strCode->Get();

    return "print(\"(Empty script.)\")";  // todo hardcoding
}

void OTClause::Serialize(Tag& parent) const
{
    auto ascCode = Armored::Factory();

    if (m_strCode->GetLength() > 2)
        ascCode->SetString(m_strCode);
    else
        LogOutput(OT_METHOD)(__FUNCTION__)(
            ": Empty script code in OTClause::Serialize().")
            .Flush();

    TagPtr pTag(new Tag("clause", ascCode->Get()));

    pTag->add_attribute("name", m_strName->Get());

    parent.add_tag(pTag);
}

// Done
bool OTClause::Compare(const OTClause& rhs) const
{
    if (!(GetName().Compare(rhs.GetName()))) {
        LogNormal(OT_METHOD)(__FUNCTION__)(": Names don't match: ")(GetName())(
            " / ")(rhs.GetName())(".")
            .Flush();
        return false;
    }

    if (!(m_strCode->Compare(rhs.GetCode()))) {
        LogNormal(OT_METHOD)(__FUNCTION__)(
            ": Source code for interpreted script fails "
            "to match, on clause: ")(GetName())(".")
            .Flush();
        return false;
    }

    return true;
}

}  // namespace opentxs
