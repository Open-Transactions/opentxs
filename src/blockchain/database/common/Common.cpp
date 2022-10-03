// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "internal/blockchain/database/common/Common.hpp"  // IWYU pragma: associated

#include <robin_hood.h>
#include <string_view>

#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::blockchain::database::common
{
constexpr auto sync_map_ = [] {
    constexpr auto offset{65536};
    auto map = robin_hood::unordered_flat_map<Chain, SyncTableData>{};

    for (const auto& chain : opentxs::blockchain::DefinedChains()) {
        auto& [table, name] = map[chain];
        table = offset + static_cast<int>(chain);
        name = print(chain);
    }

    return map;
};

auto ChainToSyncTable(const Chain chain) noexcept(false) -> int
{
    static const auto map = sync_map_();

    return map.at(chain).first;
}

auto SyncTables() noexcept -> const UnallocatedVector<SyncTableData>&
{
    static const auto map = [] {
        auto output = UnallocatedVector<SyncTableData>{};

        for (const auto& [key, value] : sync_map_()) {
            output.emplace_back(value);
        }

        return output;
    }();

    return map;
}
}  // namespace opentxs::blockchain::database::common
