// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/util/Log.hpp"  // IWYU pragma: associated

#include <utility>

#include "util/log/Streambuf.hpp"

namespace opentxs::log
{
Stream::Stream(const std::source_location& loc, Streambuf& buf) noexcept
    : std::ostream(std::addressof(buf))
{
    buf.init(loc);
}

Stream::~Stream() { flush(); }
}  // namespace opentxs::log

namespace opentxs::log
{
auto Abort(const std::source_location& loc) noexcept -> Stream
{
    static thread_local auto buf = Streambuf{LogAbort()};

    return {loc, buf};
}

auto Console(const std::source_location& loc) noexcept -> Stream
{
    static thread_local auto buf = Streambuf{LogConsole()};

    return {loc, buf};
}

auto Debug(const std::source_location& loc) noexcept -> Stream
{
    static thread_local auto buf = Streambuf{LogDebug()};

    return {loc, buf};
}

auto Detail(const std::source_location& loc) noexcept -> Stream
{
    static thread_local auto buf = Streambuf{LogDetail()};

    return {loc, buf};
}

auto Error(const std::source_location& loc) noexcept -> Stream
{
    static thread_local auto buf = Streambuf{LogError()};

    return {loc, buf};
}

auto Insane(const std::source_location& loc) noexcept -> Stream
{
    static thread_local auto buf = Streambuf{LogInsane()};

    return {loc, buf};
}

auto Trace(const std::source_location& loc) noexcept -> Stream
{
    static thread_local auto buf = Streambuf{LogTrace()};

    return {loc, buf};
}

auto Verbose(const std::source_location& loc) noexcept -> Stream
{
    static thread_local auto buf = Streambuf{LogVerbose()};

    return {loc, buf};
}
}  // namespace opentxs::log
