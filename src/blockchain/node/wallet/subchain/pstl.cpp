// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/wallet/subchain/PrehashData.hpp"  // IWYU pragma: associated

#include <algorithm>  // IWYU pragma: keep
#include <execution>
#include <ranges>

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
    auto match = [&, this](auto n) {
        this->match(
            procedure,
            log,
            cfilters,
            atLeastOnce,
            n,
            results,
            matched,
            monotonic);
    };
    const auto range = std::views::iota(0_uz, job_count_);
    using namespace std::execution;
    std::for_each(par, range.begin(), range.end(), match);
}

auto SubchainStateData::PrehashData::Prepare() noexcept -> void
{
    auto prepare = [&, this](auto n) { this->prepare(n); };
    using namespace std::execution;
    const auto range = std::views::iota(0_uz, job_count_);
    std::for_each(par, range.begin(), range.end(), prepare);
}
}  // namespace opentxs::blockchain::node::wallet
