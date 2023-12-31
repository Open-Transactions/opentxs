// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/Types.internal.hpp"  // IWYU pragma: associated

#include <string_view>

#include "opentxs/util/Log.hpp"

namespace opentxs::protobuf::inline syntax
{
auto print_error_message(
    const Log& log,
    const char* proto,
    std::string_view error,
    const std::source_location& loc) noexcept -> void
{
    log(loc)(error)(" for ")(proto).Flush();
}

auto print_error_message(
    const Log& log,
    const char* proto,
    const VersionNumber version,
    std::string_view error,
    const std::source_location& loc) noexcept -> void
{
    log(loc)("verify version ")(version)(" ")(proto)(" failed: ")(error)
        .Flush();
}

auto print_error_message(
    const Log& log,
    const char* proto,
    const VersionNumber version,
    std::string_view error,
    const long long int value,
    const std::source_location& loc) noexcept -> void
{
    log(loc)("verify version ")(version)(" ")(proto)(" failed: ")(error)("(")(
        value)(")")
        .Flush();
}

auto print_error_message(
    const Log& log,
    const char* proto,
    const VersionNumber version,
    std::string_view error,
    std::string_view value,
    const std::source_location& loc) noexcept -> void
{
    log(loc)("verify version ")(version)(" ")(proto)(" failed: ")(error)("(")(
        value)(")")
        .Flush();
}
}  // namespace opentxs::protobuf::inline syntax
