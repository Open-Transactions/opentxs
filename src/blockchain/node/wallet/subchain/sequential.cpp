// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/wallet/subchain/PrehashData.hpp"  // IWYU pragma: associated

#include "internal/util/P0330.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::blockchain::node::wallet
{
auto SubchainStateData::PrehashData::Match(
    const std::string_view procedure,
    const Log& log,
    const Vector<cfilter::GCS>& cfilters,
    std::atomic_bool& atLeastOnce,
    wallet::MatchCache::Results& results,
    MatchResults& matched,
    alloc::Default monotonic) noexcept -> void
{
    for (auto n = 0_uz, stop = job_count_; n < stop; ++n) {
        match(
            procedure,
            log,
            cfilters,
            atLeastOnce,
            n,
            results,
            matched,
            monotonic);
    }
}

auto SubchainStateData::PrehashData::Prepare() noexcept -> void
{
    for (auto n = 0_uz, stop = job_count_; n < stop; ++n) { prepare(n); }
}
}  // namespace opentxs::blockchain::node::wallet
