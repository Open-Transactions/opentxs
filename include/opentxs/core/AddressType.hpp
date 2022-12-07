// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/Export.hpp"
#include "opentxs/core/Types.hpp"

namespace opentxs
{
enum class AddressType : std::uint8_t {
    Error = 0,
    IPV4 = 1,
    IPV6 = 2,
    Onion2 = 3,
    EEP = 4,
    Inproc = 5,
};
}  // namespace opentxs
