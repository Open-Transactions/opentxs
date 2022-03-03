// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstddef>

#if __has_include(<memory_resource>)
#include <memory_resource>  // IWYU pragma: export
#elif __has_include(<experimental/memory_resource>)
#include <experimental/memory_resource>  // IWYU pragma: export
#else
#error polymorphic allocator support is required
#endif

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
auto System() noexcept -> Resource*;
auto Null() noexcept -> Resource*;
}  // namespace opentxs::alloc
