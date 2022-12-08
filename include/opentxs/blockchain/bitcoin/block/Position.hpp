// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/blockchain/bitcoin/block/Types.hpp"

namespace opentxs::blockchain::bitcoin::block::script
{
enum class Position : std::uint8_t {
    Coinbase = 0,
    Input = 1,
    Output = 2,
    Redeem = 3,
};
}  // namespace opentxs::blockchain::bitcoin::block::script
