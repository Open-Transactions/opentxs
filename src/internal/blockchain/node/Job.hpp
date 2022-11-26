// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <algorithm>
#include <cstddef>
#include <cstdint>

#include "internal/util/P0330.hpp"

namespace opentxs::blockchain::download
{
using JobID = std::int64_t;
constexpr auto invalidJob = JobID{-1};

// NOTE The batch size should approximate the value appropriate for ideal load
// balancing across the number of peers which should be active, if the number of
// blocks which should be downloaded exceeds a minimum threshold. The value is
// capped at a maximum size to prevent exceeding protocol limits for inv
// requests.
constexpr auto batch_size(
    std::size_t available,
    std::size_t peers,
    std::size_t max,
    std::size_t min) noexcept -> std::size_t
{
    return std::min(
        max,
        std::min(available, std::max(min, available / std::max(peers, 1_uz))));
}
auto next_job() noexcept -> JobID;
}  // namespace opentxs::blockchain::download
