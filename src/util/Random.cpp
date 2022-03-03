// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"     // IWYU pragma: associated
#include "1_Internal.hpp"   // IWYU pragma: associated
#include "util/Random.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstring>
#include <functional>
#include <iterator>
#include <random>

namespace opentxs
{
auto random_bytes_non_crypto(AllocateOutput dest, std::size_t bytes) noexcept
    -> bool
{
    if (false == bool(dest)) { return false; }

    auto out = dest(bytes);

    if (false == out.valid(bytes)) { return false; }

    static auto seed = std::random_device{};
    auto generator = std::mt19937{seed()};
    using RandType = int;
    using OutType = std::byte;
    auto rand = std::uniform_int_distribution<RandType>{};
    auto i = static_cast<OutType*>(out.data());

    static_assert(sizeof(OutType) <= sizeof(RandType));

    for (auto c{0u}; c < bytes; ++c, std::advance(i, 1)) {
        const auto data = rand(generator);
        std::memcpy(i, &data, sizeof(OutType));
    }

    return true;
}
}  // namespace opentxs
