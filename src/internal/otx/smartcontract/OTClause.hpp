// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/core/String.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs
{
class OTBylaw;
class Tag;

class OTClause
{
    OTString m_strName;  // Name of this Clause.
    OTString m_strCode;  // script code.
    OTBylaw* m_pBylaw;   // the Bylaw that this clause belongs to.

    OTClause(const OTClause&) = delete;
    OTClause(OTClause&&) = delete;
    auto operator=(const OTClause&) -> OTClause& = delete;
    auto operator=(OTClause&&) -> OTClause& = delete;

public:
    void SetBylaw(OTBylaw& theBylaw) { m_pBylaw = &theBylaw; }

    auto GetName() const -> const String& { return m_strName; }

    auto GetBylaw() const -> OTBylaw* { return m_pBylaw; }

    auto GetCode() const -> const char*;

    void SetCode(const UnallocatedCString& str_code);

    auto Compare(const OTClause& rhs) const -> bool;

    OTClause();
    OTClause(const char* szName, const char* szCode);
    virtual ~OTClause();

    void Serialize(Tag& parent) const;
};
}  // namespace opentxs
