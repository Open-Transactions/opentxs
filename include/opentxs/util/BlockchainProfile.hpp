// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Export.hpp"
#include "opentxs/util/Types.hpp"

namespace opentxs
{
enum class BlockchainProfile : std::uint8_t {
    mobile = 0,
    desktop = 1,
    desktop_native = 2,
    server = 3,
};
}  // namespace opentxs
