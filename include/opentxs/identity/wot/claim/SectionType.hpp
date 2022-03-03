// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"                   // IWYU pragma: associated
#include "opentxs/identity/wot/claim/Types.hpp"  // IWYU pragma: associated

#include <cstdint>

namespace opentxs::identity::wot::claim
{
enum class SectionType : std::uint8_t {
    Error = 0,
    Scope = 1,
    Identifier = 2,
    Address = 3,
    Communication = 4,
    Profile = 5,
    Relationship = 6,
    Descriptor = 7,
    Event = 8,
    Contract = 9,
    Procedure = 10,
};
}  // namespace opentxs::identity::wot::claim
