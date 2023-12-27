// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/common/Counter.hpp"  // IWYU pragma: associated

#include <opentxs/opentxs.hpp>
#include <chrono>
#include <compare>
#include <iostream>

namespace ot = opentxs;

namespace ottest
{
auto make_cb(Counter& counter, std::string_view in) noexcept
    -> std::function<void()>
{
    return [&counter, name = ot::CString{in}]() {
        auto& [expected, value] = counter;

        if (++value > expected) { std::cout << name << ": " << value << '\n'; }
    };
}

auto wait_for_counter(Counter& data, const bool hard) noexcept -> bool
{
    using namespace std::literals::chrono_literals;

    const auto limit = hard ? 300s : 30s;
    auto start = ot::Clock::now();
    auto& [expected, updated] = data;

    while ((updated < expected) && ((ot::Clock::now() - start) < limit)) {
        ot::sleep(100ms);
    }

    if (false == hard) { updated.store(expected.load()); }

    return updated >= expected;
}
}  // namespace ottest
