// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/wallet/subchain/PrehashData.hpp"  // IWYU pragma: associated

#include <utility>

#include "TBB.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "internal/util/Thread.hpp"
#include "internal/util/alloc/Boost.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::blockchain::node::wallet
{
auto SubchainStateData::PrehashData::Match(
    const std::string_view procedure,
    const Log& log,
    const Vector<GCS>& cfilters,
    std::atomic_bool& atLeastOnce,
    wallet::MatchCache::Results& results,
    MatchResults& matched,
    alloc::Default) noexcept -> void
{
    tbb::parallel_for(
        tbb::blocked_range<std::size_t>{0_uz, job_count_},
        [&, this](const auto& r) {
            auto resource =
                alloc::BoostMonotonic(convert_to_size(thread_pool_monotonic_));
            auto temp = allocator_type{std::addressof(resource)};

            for (auto i = r.begin(); i != r.end(); ++i) {
                match(
                    procedure,
                    log,
                    cfilters,
                    atLeastOnce,
                    i,
                    results,
                    matched,
                    temp);
            }
        });
}

auto SubchainStateData::PrehashData::Prepare() noexcept -> void
{
    tbb::parallel_for(
        tbb::blocked_range<std::size_t>{0_uz, job_count_},
        [this](const auto& r) {
            for (auto i = r.begin(); i != r.end(); ++i) { prepare(i); }
        });
}
}  // namespace opentxs::blockchain::node::wallet
