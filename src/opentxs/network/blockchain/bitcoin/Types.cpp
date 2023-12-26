// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/network/blockchain/bitcoin/Types.hpp"  // IWYU pragma: associated

#include <frozen/bits/algorithms.h>
#include <frozen/unordered_map.h>
#include <functional>  // IWYU pragma: keep
#include <string_view>
#include <utility>

#include "opentxs/network/blockchain/bitcoin/Service.hpp"  // IWYU pragma: keep

namespace opentxs::network::blockchain::bitcoin
{
using namespace std::literals;

auto print(Service in) noexcept -> std::string_view
{
    using enum Service;
    static constexpr auto map =
        frozen::make_unordered_map<Service, std::string_view>({
            {None, "none"sv},
            {Avalanche, "Avalanche"sv},
            {BitcoinCash, "Bitcoin Cash"sv},
            {Bloom, "Bloom"sv},
            {CompactFilters, "Compact Filters"sv},
            {Graphene, "Graphene"sv},
            {Limited, "Limited"sv},
            {Network, "Network"sv},
            {Segwit2X, "Segwit2X"sv},
            {UTXO, "GetUTXO"sv},
            {WeakBlocks, "Weak blocks"sv},
            {Witness, "Witness"sv},
            {XThin, "XThin"sv},
            {XThinner, "XThinner"sv},
        });

    if (const auto* i = map.find(in); map.end() != i) {

        return i->second;
    } else {

        return "unknown bitcoin::Service"sv;
    }
}
}  // namespace opentxs::network::blockchain::bitcoin
