// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/network/blockchain/bitcoin/Types.hpp"

namespace opentxs::network::blockchain::bitcoin
{
enum class Service : std::uint8_t {
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

OPENTXS_EXPORT auto print(Service) noexcept -> std::string_view;
}  // namespace opentxs::network::blockchain::bitcoin
