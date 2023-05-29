// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/otx/smartcontract/OTClause.hpp"  // IWYU pragma: associated

#include <memory>

#include "internal/core/Armored.hpp"
#include "internal/core/String.hpp"
#include "internal/otx/common/util/Tag.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Pimpl.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

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
    : name_(String::Factory())
    , code_(String::Factory())
    , bylaw_(nullptr)
{
}

OTClause::OTClause(const char* szName, const char* szCode)
    : name_(String::Factory())
    , code_(String::Factory())
    , bylaw_(nullptr)
{
    if (nullptr != szName) { name_->Set(szName); }
    if (nullptr != szCode) { code_->Set(szCode); }

    // Todo security:  validation on the above fields.
}

OTClause::~OTClause()
{
    // nothing to delete.

    bylaw_ =
        nullptr;  // I wasn't the owner, it was a pointer for convenience only.
}

void OTClause::SetCode(const UnallocatedCString& str_code)
{
    code_->Set(str_code.c_str());
}

auto OTClause::GetCode() const -> const char*
{
    if (code_->Exists()) { return code_->Get(); }

    return "print(\"(Empty script.)\")";  // todo hardcoding
}

void OTClause::Serialize(const api::Crypto& crypto, Tag& parent) const
{
    auto ascCode = Armored::Factory(crypto);

    if (code_->GetLength() > 2) {
        ascCode->SetString(code_);
    } else {
        LogError()(OT_PRETTY_CLASS())(
            "Empty script code in OTClause::Serialize().")
            .Flush();
    }

    TagPtr pTag(new Tag("clause", ascCode->Get()));

    pTag->add_attribute("name", name_->Get());

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

    if (!(code_->Compare(rhs.GetCode()))) {
        LogConsole()(OT_PRETTY_CLASS())(
            "Source code for interpreted script fails "
            "to match, on clause: ")(GetName())(".")
            .Flush();
        return false;
    }

    return true;
}

}  // namespace opentxs
