// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/log/Streambuf.hpp"  // IWYU pragma: associated

#include <memory>

#include "opentxs/util/Log.hpp"

namespace opentxs::log
{
Streambuf::Streambuf(Log& log) noexcept
    : log_(log)
{
}

auto Streambuf::init(const std::source_location& loc) noexcept -> void
{
    log_(loc);
}

auto Streambuf::sync() noexcept -> int
{
    // TODO c++20
    // log_(view()).Flush();
    log_(str()).Flush();
    pubseekoff(0, std::ios::beg);

    return 0;
}
}  // namespace opentxs::log
