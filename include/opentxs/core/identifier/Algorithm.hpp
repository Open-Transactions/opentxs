// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/core/identifier/Types.hpp"  // IWYU pragma: associated

#include <cstdint>

#include "opentxs/Export.hpp"

namespace opentxs::identifier
{
enum class Algorithm : std::uint8_t {
    invalid = 0,
    sha256 = 1,
    blake2b160 = 2,
    blake2b256 = 3,
};
}  // namespace opentxs::identifier
