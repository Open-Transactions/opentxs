// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/Log.hpp"  // IWYU pragma: associated

extern "C" {
#include <android/log.h>
}

#include <sstream>

namespace opentxs::api::imp
{
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
