// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <limits>

#include "opentxs/Export.hpp"
#include "opentxs/crypto/Types.hpp"

namespace opentxs::crypto
{
enum class SeedStyle : std::uint8_t {
    BIP32 = 0,
    BIP39 = 1,
    PKT = 2,
    Error = std::numeric_limits<std::uint8_t>::max(),
};
}  // namespace opentxs::crypto
