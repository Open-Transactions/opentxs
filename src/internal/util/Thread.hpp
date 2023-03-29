// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <string_view>

#include "util/ByteLiterals.hpp"

namespace opentxs
{
enum class ThreadPriority {
    Idle,
    Lowest,
    BelowNormal,
    Normal,
    AboveNormal,
    Highest,
    TimeCritical,
};  // IWYU pragma: export

constexpr auto thread_pool_stack_size_ = 8_mib;
constexpr auto thread_pool_reserve_ = 256_kib;
constexpr auto thread_pool_monotonic_ =
    thread_pool_stack_size_ - thread_pool_reserve_;

auto AdvanceToNextPageBoundry(std::size_t position) noexcept -> std::size_t;
auto IsPageAligned(std::size_t position) noexcept -> bool;
auto MaxJobs() noexcept -> unsigned int;
auto PageSize() noexcept -> std::size_t;
auto print(ThreadPriority priority) noexcept -> const char*;
auto SetThisThreadsName(const std::string_view threadname) noexcept -> void;
auto SetThisThreadsPriority(ThreadPriority priority) noexcept -> void;
}  // namespace opentxs
