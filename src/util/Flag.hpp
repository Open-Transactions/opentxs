// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <atomic>

#include "internal/util/Flag.hpp"

namespace opentxs::implementation
{
class Flag final : virtual public opentxs::Flag
{
public:
    operator bool() const final { return flag_.load(); }

    auto Off() -> bool final { return Set(false); }
    auto On() -> bool final { return !Set(true); }
    auto Set(const bool value) -> bool final { return flag_.exchange(value); }
    auto Toggle() -> bool final;

    Flag(const bool state);

    ~Flag() final = default;

private:
    std::atomic<bool> flag_;

    auto clone() const -> Flag* final { return new Flag(flag_.load()); }

    Flag() = delete;
    Flag(const Flag&) = delete;
    Flag(Flag&&) = delete;
    auto operator=(const Flag&) -> Flag& = delete;
    auto operator=(Flag&&) -> Flag& = delete;
};
}  // namespace opentxs::implementation
