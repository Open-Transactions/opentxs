// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"            // IWYU pragma: associated
#include "internal/util/Time.hpp"  // IWYU pragma: associated
#include "opentxs/util/Time.hpp"   // IWYU pragma: associated

#include <compare>
#include <ctime>
#include <limits>
#include <stdexcept>
#include <thread>

namespace opentxs
{
template <typename IntType>
static auto convert(IntType number) noexcept(false) -> Time
{
    static constexpr auto max = std::numeric_limits<std::time_t>::max();

    static_assert(max <= std::numeric_limits<IntType>::max());

    static constexpr auto limit = static_cast<IntType>(max);

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtautological-type-limit-compare"
    // NOTE std::size_t might be 32 bit
    if (number > limit) {

        throw std::runtime_error{"std::time_t is too small to hold this value"};
    }
#pragma GCC diagnostic pop

    return Clock::from_time_t(static_cast<std::time_t>(number));
}

auto convert_stime(std::int64_t number) noexcept(false) -> Time
{
    return convert<std::int64_t>(number);
}

auto convert_time(std::uint64_t number) noexcept(false) -> Time
{
    return convert<std::uint64_t>(number);
}

auto Sleep(const std::chrono::microseconds us) -> bool
{
    auto start = std::chrono::high_resolution_clock::now();
    auto end = start + us;

    do {
        std::this_thread::yield();
        std::this_thread::sleep_for(us);
    } while (std::chrono::high_resolution_clock::now() < end);

    return true;
}
}  // namespace opentxs
