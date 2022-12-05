// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/node/Job.hpp"  // IWYU pragma: associated

#include <atomic>

namespace opentxs::blockchain::download
{
auto next_job() noexcept -> JobID
{
    static auto counter = std::atomic<JobID>{-1};

    return ++counter;
}
}  // namespace opentxs::blockchain::download
