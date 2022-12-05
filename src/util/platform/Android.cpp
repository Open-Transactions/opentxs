// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/Legacy.hpp"  // IWYU pragma: associated
#include "api/Log.hpp"     // IWYU pragma: associated

extern "C" {
#include <android/log.h>
}

#include <sstream>

#include "opentxs/util/Allocator.hpp"

namespace opentxs::api::imp
{
auto Legacy::use_dot() noexcept -> bool { return false; }

auto Log::print(
    const int level,
    const Console,
    const std::string_view text,
    const std::string_view thread) noexcept -> void
{
    const auto tag = std::stringstream{"opentxs "} << "(" << thread << ")";
    const auto nullTerminated = UnallocatedCString{text};
    const auto prio = [&] {
        // TODO ANDROID_LOG_ERROR

        switch (level) {
            case -2: {

                return ANDROID_LOG_FATAL;
            }
            case -1: {

                return ANDROID_LOG_WARN;
            }
            case 0:
            case 1: {

                return ANDROID_LOG_INFO;
            }
            case 2:
            case 3: {

                return ANDROID_LOG_VERBOSE;
            }
            case 4:
            case 5:
            default: {

                return ANDROID_LOG_DEBUG;
            }
        }
    }();
    __android_log_write(prio, tag.str().c_str(), nullTerminated.c_str());
}
}  // namespace opentxs::api::imp

// TODO after libc++ finally incorporates this into std, and after a new version
// of the ndk is released which uses that version of libc++, then this can be
// removed
namespace std::experimental::fundamentals_v1::pmr
{
auto get_default_resource() noexcept -> opentxs::alloc::Resource*
{
    return opentxs::alloc::System();
}
}  // namespace std::experimental::fundamentals_v1::pmr
