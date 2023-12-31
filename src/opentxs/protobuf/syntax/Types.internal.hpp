// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <source_location>
#include <string_view>
#include <utility>

#include "opentxs/util/Container.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::protobuf::inline syntax
{
// This defined a map between the version of the parent object and the (minimum,
// maximum) acceptable versions of a child object.
using VersionMap =
    UnallocatedMap<std::uint32_t, std::pair<std::uint32_t, std::uint32_t>>;

auto print_error_message(
    const Log& log,
    const char* proto,
    std::string_view error,
    const std::source_location& loc = std::source_location::current()) noexcept
    -> void;
auto print_error_message(
    const Log& log,
    const char* proto,
    const VersionNumber version,
    std::string_view error,
    const std::source_location& loc = std::source_location::current()) noexcept
    -> void;
auto print_error_message(
    const Log& log,
    const char* proto,
    const VersionNumber version,
    std::string_view error,
    const long long int value,
    const std::source_location& loc = std::source_location::current()) noexcept
    -> void;
auto print_error_message(
    const Log& log,
    const char* proto,
    const VersionNumber version,
    std::string_view error,
    std::string_view value,
    const std::source_location& loc = std::source_location::current()) noexcept
    -> void;
}  // namespace opentxs::protobuf::inline syntax
