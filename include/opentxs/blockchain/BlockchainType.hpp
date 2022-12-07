// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <limits>

#include "opentxs/Export.hpp"
#include "opentxs/blockchain/Types.hpp"

namespace opentxs::blockchain
{
enum class Type : TypeEnum {
    Unknown = 0,
    Bitcoin = 1,
    Bitcoin_testnet3 = 2,
    BitcoinCash = 3,
    BitcoinCash_testnet3 = 4,
    Ethereum_frontier = 5,
    Ethereum_ropsten = 6,
    Litecoin = 7,
    Litecoin_testnet4 = 8,
    PKT = 9,
    PKT_testnet = 10,
    BitcoinSV = 11,
    BitcoinSV_testnet3 = 12,
    eCash = 13,
    eCash_testnet3 = 14,
    UnitTest = std::numeric_limits<TypeEnum>::max(),
};
}  // namespace opentxs::blockchain
