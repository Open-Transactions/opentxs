// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <algorithm>
#include <chrono>

#include "opentxs/Time.hpp"

namespace opentxs
{
class Backoff
{
public:
    auto test(const std::chrono::milliseconds& start) noexcept -> bool
    {
        start_interval_ = start;

        return test();
    }
    auto test() noexcept -> bool
    {
        const auto now = Clock::now();

        if (never_ == last_) {
            next_interval_ = start_interval_;
            last_ = now;
            next_ = now + next_interval_;

            return true;
        }

        if (now < next_) { return false; }

        const auto next = next_interval_ * factor_;
        next_interval_ = std::min(next, max_interval_);
        last_ = now;
        next_ = now + next_interval_;

        return true;
    }
    auto reset() noexcept -> void { last_ = never_; }

    Backoff(
        const std::chrono::milliseconds& start = 1s,
        const std::chrono::milliseconds& max = 32s,
        const unsigned int factor = 2) noexcept
        : factor_(factor)
        , max_interval_(max)
        , start_interval_(start)
        , next_interval_(start_interval_)
        , last_(never_)
        , next_(never_)
    {
    }
    Backoff(const Backoff&) = delete;
    Backoff(Backoff&&) = delete;
    auto operator=(const Backoff&) -> Backoff& = delete;
    auto operator=(Backoff&&) -> Backoff& = delete;

private:
    static constexpr auto never_ = Time{};

    const unsigned int factor_;
    const std::chrono::milliseconds max_interval_;
    std::chrono::milliseconds start_interval_;
    std::chrono::milliseconds next_interval_;
    Time last_;
    Time next_;
};
}  // namespace opentxs
