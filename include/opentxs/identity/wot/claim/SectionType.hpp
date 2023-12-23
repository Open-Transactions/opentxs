// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>

#include "opentxs/identity/wot/claim/Types.hpp"  // IWYU pragma: keep

namespace opentxs::identity::wot::claim
{
enum class SectionType : std::underlying_type_t<SectionType> {
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
