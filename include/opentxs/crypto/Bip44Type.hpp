// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"       // IWYU pragma: associated
#include "opentxs/crypto/Types.hpp"  // IWYU pragma: associated

namespace opentxs
{
enum class Bip44Type : Bip32Index {
    BITCOIN = 0,
    TESTNET = 1,
    LITECOIN = 2,
    DOGECOIN = 3,
    REDDCOIN = 4,
    DASH = 5,
    PEERCOIN = 6,
    NAMECOIN = 7,
    FEATHERCOIN = 8,
    COUNTERPARTY = 9,
    BLACKCOIN = 10,
    ETHER = 60,
    BITCOINCASH = 145,
    BITCOINSV = 236,
    PKT = 390,
};
}  // namespace opentxs
