
// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <chrono>
#include <future>

namespace opentxs
{
template <typename Future>
auto IsReady(const Future& future) noexcept -> bool
{
    try {
        using enum std::future_status;
        using namespace std::literals;

        return ready == future.wait_for(0ns);
    } catch (...) {

        return false;
    }
}
}  // namespace opentxs
