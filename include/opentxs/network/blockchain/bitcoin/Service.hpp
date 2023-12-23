// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <type_traits>

#include "opentxs/network/blockchain/bitcoin/Types.hpp"  // IWYU pragma: keep

namespace opentxs::network::blockchain::bitcoin
{
enum class Service : std::underlying_type_t<Service> {
    None = 0,
    Avalanche = 1,
    BitcoinCash = 2,
    Bloom = 3,
    CompactFilters = 4,
    Graphene = 5,
    Limited = 6,
    Network = 7,
    Segwit2X = 8,
    UTXO = 9,
    WeakBlocks = 10,
    Witness = 11,
    XThin = 12,
    XThinner = 13,
};
}  // namespace opentxs::network::blockchain::bitcoin
