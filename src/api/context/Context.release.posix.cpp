// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "api/context/Context.hpp"  // IWYU pragma: associated

extern "C" {
#include <sys/resource.h>
}

#include <cerrno>
#include <cstring>

#include "opentxs/util/Log.hpp"

namespace opentxs::api::imp
{
auto Context::Init_CoreDump() noexcept -> void
{
    struct rlimit rlim;
    getrlimit(RLIMIT_CORE, &rlim);
    rlim.rlim_max = rlim.rlim_cur = 0;

    if (setrlimit(RLIMIT_CORE, &rlim)) {
        LogAbort()()("setrlimit (used for preventing core dumps) failed: ")(
            strerror(errno))
            .Abort();
    }
}
}  // namespace opentxs::api::imp
