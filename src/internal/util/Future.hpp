
// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <future>

#include "opentxs/util/Time.hpp"  // IWYU pragma: keep

namespace opentxs
{
template <typename Future>
auto IsReady(const Future& future) noexcept -> bool
{
    try {
        static constexpr auto zero = 0ns;
        static constexpr auto ready = std::future_status::ready;

        return ready == future.wait_for(zero);
    } catch (...) {

        return false;
    }
}
}  // namespace opentxs
