// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/network/blockchain/Types.hpp"

namespace opentxs::network::blockchain
{
enum class Subchain : std::uint8_t {
    invalid = 0,
    primary = 1,
    testnet1 = 128,
    testnet2 = 129,
    testnet3 = 130,
    testnet4 = 131,
    testnet5 = 132,
    testnet6 = 133,
    testnet7 = 134,
    testnet8 = 135,
    testnet9 = 136,
    testnet10 = 137,
    chipnet = 252,
    scalenet = 253,
    signet = 254,
    regtest = 255,
};
}  // namespace opentxs::network::blockchain
