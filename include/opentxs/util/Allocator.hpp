// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>

#if __has_include(<memory_resource>)
#include <memory_resource>  // IWYU pragma: export
#elif __has_include(<experimental/memory_resource>)
#include <experimental/memory_resource>  // IWYU pragma: export
#else
#error polymorphic allocator support is required
#endif

#include "opentxs/Export.hpp"

namespace opentxs::alloc
{
#if __has_include(<memory_resource>)
template <typename T>
using PMR = std::pmr::polymorphic_allocator<T>;
using Resource = std::pmr::memory_resource;
#else
template <typename T>
using PMR = std::experimental::pmr::polymorphic_allocator<T>;
using Resource = std::experimental::pmr::memory_resource;
#endif
using Default = PMR<std::byte>;
auto OPENTXS_EXPORT System() noexcept -> Resource*;
auto OPENTXS_EXPORT Null() noexcept -> Resource*;

struct OPENTXS_EXPORT Strategy {
    struct work_only_t {
    };

    Default result_;  /// Allocator to use for constructing a return value
    Default work_;    /// Allocator to use for temporary values

    auto WorkOnly() const noexcept -> Strategy { return {work_, work_}; }

    Strategy() noexcept
        : Strategy{System(), System()}
    {
    }
    Strategy(Default result) noexcept
        : Strategy{result, System()}
    {
    }
    Strategy(work_only_t, Default work) noexcept
        : Strategy{work, work}
    {
    }
    Strategy(Default result, Default work) noexcept
        : result_{result}
        , work_{work}
    {
    }
    Strategy(const Strategy& rhs) noexcept = default;
    Strategy(Strategy&& rhs) noexcept = default;
    auto operator=(const Strategy& rhs) noexcept -> Strategy& = delete;
    auto operator=(Strategy&& rhs) noexcept -> Strategy& = delete;
};
}  // namespace opentxs::alloc

namespace opentxs
{
static constexpr auto WorkOnly = alloc::Strategy::work_only_t{};
}  // namespace opentxs
