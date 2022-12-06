// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/Export.hpp"
#include "opentxs/identity/wot/claim/Types.hpp"

namespace opentxs::identity::wot::claim
{
enum class Attribute : std::uint8_t {
    Error = 0,
    Active = 1,
    Primary = 2,
    Local = 3,
};
}  // namespace opentxs::identity::wot::claim
