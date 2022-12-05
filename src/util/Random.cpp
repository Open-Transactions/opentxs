// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/Random.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <cstring>
#include <iterator>
#include <random>

#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs
{
auto random_bytes_non_crypto(Writer&& dest, std::size_t bytes) noexcept -> bool
{
    auto out = dest.Reserve(bytes);

    if (false == out.IsValid(bytes)) { return false; }

    static auto seed = std::random_device{};
    auto generator = std::mt19937{seed()};
    using RandType = int;
    using OutType = std::byte;
    auto rand = std::uniform_int_distribution<RandType>{};
    auto* i = out.as<OutType>();

    static_assert(sizeof(OutType) <= sizeof(RandType));

    for (auto c{0u}; c < bytes; ++c, std::advance(i, 1)) {
        const auto data = rand(generator);
        std::memcpy(i, &data, sizeof(OutType));
    }

    return true;
}
}  // namespace opentxs
