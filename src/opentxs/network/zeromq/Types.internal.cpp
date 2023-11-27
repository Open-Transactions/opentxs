// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/network/zeromq/Types.internal.hpp"  // IWYU pragma: associated

#include <atomic>

#include "internal/util/P0330.hpp"
#include "opentxs/network/zeromq/message/Frame.hpp"

namespace opentxs::network::zeromq
{
auto GetBatchID() noexcept -> BatchID
{
    static auto counter = std::atomic<BatchID>{};

    return ++counter;
}

auto GetSocketID() noexcept -> SocketID
{
    static auto counter = std::atomic<SocketID>{};

    return ++counter;
}

auto operator==(std::span<const Frame> lhs, std::span<const Frame> rhs) noexcept
    -> bool
{
    const auto count = lhs.size();

    if (rhs.size() != count) { return false; }

    for (auto n = 0_uz; n < count; ++n) {
        if (lhs[n] != rhs[n]) { return false; }
    }

    return true;
}

auto operator<=>(
    std::span<const Frame> lhs,
    std::span<const Frame> rhs) noexcept -> std::strong_ordering
{
    if (auto s1 = lhs.size(), s2 = rhs.size(); s1 < s2) {

        return std::strong_ordering::less;
    } else if (s2 < s1) {

        return std::strong_ordering::greater;
    } else {
        constexpr auto same = std::strong_ordering::equal;

        for (auto n = 0_uz; n < s1; ++n) {
            if (auto val = lhs[n] <=> rhs[n]; same != val) { return val; }
        }

        return same;
    }
}
}  // namespace opentxs::network::zeromq
