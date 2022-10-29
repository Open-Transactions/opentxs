// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <opentxs/opentxs.hpp>
#include <atomic>
#include <functional>
#include <string_view>

namespace ottest
{
struct OPENTXS_EXPORT Counter {
    std::atomic_int expected_{};
    std::atomic_int updated_{};
};

OPENTXS_EXPORT auto make_cb(Counter& counter, std::string_view name) noexcept
    -> std::function<void()>;
OPENTXS_EXPORT auto wait_for_counter(
    Counter& data,
    const bool hard = true) noexcept -> bool;
}  // namespace ottest
