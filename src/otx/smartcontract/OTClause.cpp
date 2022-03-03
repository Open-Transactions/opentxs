// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                             // IWYU pragma: associated
#include "1_Internal.hpp"                           // IWYU pragma: associated
#include "internal/otx/smartcontract/OTClause.hpp"  // IWYU pragma: associated

#include <memory>

#include "internal/otx/common/util/Tag.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"

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

void OTClause::SetCode(const UnallocatedCString& str_code)
{
    m_strCode->Set(str_code.c_str());
}

auto OTClause::GetCode() const -> const char*
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
        LogError()(OT_PRETTY_CLASS())(
            "Empty script code in OTClause::Serialize().")
            .Flush();

    TagPtr pTag(new Tag("clause", ascCode->Get()));

    pTag->add_attribute("name", m_strName->Get());

    parent.add_tag(pTag);
}

// Done
auto OTClause::Compare(const OTClause& rhs) const -> bool
{
    if (!(GetName().Compare(rhs.GetName()))) {
        LogConsole()(OT_PRETTY_CLASS())("Names don't match: ")(GetName())(
            " / ")(rhs.GetName())(".")
            .Flush();
        return false;
    }

    if (!(m_strCode->Compare(rhs.GetCode()))) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Source code for interpreted script fails "
            "to match, on clause: ")(GetName())(".")
            .Flush();
        return false;
    }

    return true;
}

}  // namespace opentxs
