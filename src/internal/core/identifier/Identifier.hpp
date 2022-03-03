// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/core/identifier/Notary.hpp"
#include "opentxs/core/identifier/Nym.hpp"
#include "opentxs/core/identifier/UnitDefinition.hpp"
#include "util/Blank.hpp"

namespace opentxs
{
template <>
struct make_blank<OTIdentifier> {
    static auto value(const api::Session&) -> OTIdentifier
    {
        return Identifier::Factory();
    }
};

template <>
struct make_blank<OTNymID> {
    static auto value(const api::Session&) -> OTNymID
    {
        return identifier::Nym::Factory();
    }
};

template <>
struct make_blank<OTNotaryID> {
    static auto value(const api::Session&) -> OTNotaryID
    {
        return identifier::Notary::Factory();
    }
};

template <>
struct make_blank<OTUnitID> {
    static auto value(const api::Session&) -> OTUnitID
    {
        return identifier::UnitDefinition::Factory();
    }
};
}  // namespace opentxs
