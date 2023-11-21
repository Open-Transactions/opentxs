// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/api/PathsPrivate.hpp"  // IWYU pragma: associated

extern "C" {
#include <pwd.h>
#include <unistd.h>
}

#include <cerrno>
#include <cstring>

#include "opentxs/util/Log.hpp"

namespace opentxs::api::internal
{
auto PathsPrivate::get_home_platform() noexcept -> UnallocatedCString
{
    const auto* pwd = getpwuid(getuid());

    if (nullptr != pwd) {
        if (nullptr != pwd->pw_dir) { return pwd->pw_dir; }
    }

    LogConsole()("getpwuid: ")(strerror(errno)).Flush();

    return {};
}
}  // namespace opentxs::api::internal
