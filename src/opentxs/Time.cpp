// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/Time.hpp"  // IWYU pragma: associated

#include <compare>
#include <thread>

namespace opentxs
{
auto seconds_since_epoch(Time in) noexcept -> std::optional<std::int64_t>
{
    using namespace std::chrono;

    return duration_cast<seconds>(in.time_since_epoch()).count();
}

auto seconds_since_epoch(std::int64_t in) noexcept -> std::optional<Time>
{
    using namespace std::chrono;
    constexpr auto min = seconds::min().count();
    constexpr auto max = seconds::max().count();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtautological-type-limit-compare"
    if ((in < min) || (in > max)) { return std::nullopt; }
#pragma GCC diagnostic pop

    return Time{seconds{in}};
}

auto seconds_since_epoch_unsigned(Time in) noexcept
    -> std::optional<std::uint64_t>
{
    using namespace std::chrono;
    constexpr auto min = Time{seconds{0}};

    if (in < min) { return std::nullopt; }

    return duration_cast<seconds>(in.time_since_epoch()).count();
}

auto seconds_since_epoch_unsigned(std::uint64_t in) noexcept
    -> std::optional<Time>
{
    using namespace std::chrono;
    constexpr auto max = static_cast<std::uint64_t>(seconds::max().count());

    if (in > max) { return std::nullopt; }

    return Time{seconds{in}};
}

auto sleep(const std::chrono::microseconds us) noexcept -> bool
{
    const auto start = sClock::now();
    const auto end = start + us;

    do {
        std::this_thread::yield();
        std::this_thread::sleep_for(us);
    } while (sClock::now() < end);

    return true;
}
}  // namespace opentxs
