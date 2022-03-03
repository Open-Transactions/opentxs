// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                        // IWYU pragma: associated
#include "1_Internal.hpp"                      // IWYU pragma: associated
#include "api/Legacy.hpp"                      // IWYU pragma: associated
#include "api/context/Context.hpp"             // IWYU pragma: associated
#include "network/zeromq/context/Context.hpp"  // IWYU pragma: associated
#include "util/Thread.hpp"                     // IWYU pragma: associated

extern "C" {
#include <sys/resource.h>
}

#include "opentxs/util/Allocator.hpp"

namespace opentxs
{
auto SetThisThreadsPriority(ThreadPriority) noexcept -> void
{
    // TODO
}
}  // namespace opentxs

namespace opentxs::api::imp
{
auto Context::set_desired_files(::rlimit& out) noexcept -> void
{
    out.rlim_cur = OPEN_MAX;
    out.rlim_max = OPEN_MAX;
}

auto Legacy::get_suffix() noexcept -> fs::path
{
    return get_suffix("OpenTransactions");
}

auto Legacy::use_dot() noexcept -> bool { return false; }
}  // namespace opentxs::api::imp

namespace opentxs::network::zeromq::implementation
{
auto Context::max_sockets() noexcept -> int { return 10240; }
}  // namespace opentxs::network::zeromq::implementation

// TODO after libc++ finally incorporates this into std, and after Apple ships
// that version of libc++, then this can be removed
namespace std::experimental::fundamentals_v1::pmr
{
auto get_default_resource() noexcept -> opentxs::alloc::Resource*
{
    return opentxs::alloc::System();
}
}  // namespace std::experimental::fundamentals_v1::pmr
