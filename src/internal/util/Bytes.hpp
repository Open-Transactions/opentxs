// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <array>
#include <cstddef>

#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs
{
using RawData = UnallocatedVector<unsigned char>;

constexpr auto reader(const void* p, std::size_t s) noexcept -> ReadView
{
    return {static_cast<const char*>(p), s};
}

template <std::size_t N>
auto reader(const std::array<std::byte, N>& in) noexcept -> ReadView
{
    return {reinterpret_cast<const char*>(in.data()), N};
}

template <std::size_t N>
auto writer(std::array<std::byte, N>& in) noexcept -> Writer
{
    return preallocated(N, in.data());
}
}  // namespace opentxs
