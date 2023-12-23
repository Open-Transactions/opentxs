// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/database/common/Peers.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <chrono>
#include <compare>
#include <ranges>
#include <utility>

#include "internal/util/storage/lmdb/Database.hpp"  // IWYU pragma: keep
#include "internal/util/storage/lmdb/Types.hpp"
#include "opentxs/api/Session.hpp"
#include "opentxs/api/Session.internal.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/identifier/Generic.hpp"  // IWYU pragma: keep

namespace opentxs::blockchain::database::common
{
auto Peers::init_chains(
    const api::Session& api,
    const Time now,
    Data& data,
    std::span<const std::pair<const Addresses*, GuardedIndex*>> chains) noexcept
    -> void
{
    auto read = [&api, &data, now](const auto& item) {
        if (api.Internal().ShuttingDown()) { return; }

        const auto& [addresses, guarded] = item;
        auto h = guarded->lock();
        auto& index = *h;

        for (const auto& id : *addresses) {
            const auto lastConnected = [&]() -> Time {
                if (auto j = data.connected_.find(id);
                    data.connected_.end() != j) {

                    return j->second;
                } else {

                    return {};
                }
            }();
            using namespace std::chrono;
            const auto interval = duration_cast<hours>(now - lastConnected);
            constexpr auto limit = 24h;

            if (interval <= limit) {
                index.known_good_.emplace(id);
            } else {
                index.untested_.emplace(id);
            }
        }
    };
    std::ranges::for_each(chains, read);
}

auto Peers::init_tables(
    const api::Session& api,
    std::span<const std::pair<Table, storage::lmdb::ReadCallback>>
        work) noexcept -> void
{
    auto read = [this, &api](const auto& item) {
        if (api.Internal().ShuttingDown()) { return; }

        const auto& [table, cb] = item;

        try {
            lmdb_.Read(table, cb, storage::lmdb::Dir::Forward);
        } catch (...) {

            return;
        }
    };
    std::ranges::for_each(work, read);
}
}  // namespace opentxs::blockchain::database::common
