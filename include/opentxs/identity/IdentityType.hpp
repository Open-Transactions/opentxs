// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/Export.hpp"
#include "opentxs/identity/Types.hpp"

namespace opentxs::identity
{
enum class Type : std::uint32_t {
    invalid = 0,
    individual = 1,
    organization = 2,
    business = 3,
    government = 4,
    server = 5,
    bot = 6,
};
}  // namespace opentxs::identity
