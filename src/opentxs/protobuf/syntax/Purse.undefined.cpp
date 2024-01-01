// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "opentxs/protobuf/syntax/Purse.hpp"  // IWYU pragma: associated

#include <opentxs/protobuf/Purse.pb.h>  // IWYU pragma: keep

#include "opentxs/protobuf/syntax/Macros.hpp"

namespace opentxs::protobuf::inline syntax
{
using namespace std::literals;

auto version_2(const Purse& input, const Log& log) -> bool
{
    auto value = "0"s;

    return version_1(input, log, value);
}

auto version_2(const Purse& input, const Log& log, UnallocatedCString& value)
    -> bool
{
    UNDEFINED_VERSION(2);
}

auto version_3(const Purse& input, const Log& log) -> bool
{
    auto value = "0"s;

    return version_1(input, log, value);
}

auto version_3(const Purse& input, const Log& log, UnallocatedCString& value)
    -> bool
{
    UNDEFINED_VERSION(3);
}

auto version_4(const Purse& input, const Log& log) -> bool
{
    auto value = "0"s;

    return version_1(input, log, value);
}

auto version_4(const Purse& input, const Log& log, UnallocatedCString& value)
    -> bool
{
    UNDEFINED_VERSION(4);
}

auto version_5(const Purse& input, const Log& log) -> bool
{
    auto value = "0"s;

    return version_1(input, log, value);
}

auto version_5(const Purse& input, const Log& log, UnallocatedCString& value)
    -> bool
{
    UNDEFINED_VERSION(5);
}

auto version_6(const Purse& input, const Log& log) -> bool
{
    auto value = "0"s;

    return version_1(input, log, value);
}

auto version_6(const Purse& input, const Log& log, UnallocatedCString& value)
    -> bool
{
    UNDEFINED_VERSION(6);
}

auto version_7(const Purse& input, const Log& log) -> bool
{
    auto value = "0"s;

    return version_1(input, log, value);
}

auto version_7(const Purse& input, const Log& log, UnallocatedCString& value)
    -> bool
{
    UNDEFINED_VERSION(7);
}

auto version_8(const Purse& input, const Log& log) -> bool
{
    auto value = "0"s;

    return version_1(input, log, value);
}

auto version_8(const Purse& input, const Log& log, UnallocatedCString& value)
    -> bool
{
    UNDEFINED_VERSION(8);
}

auto version_9(const Purse& input, const Log& log) -> bool
{
    auto value = "0"s;

    return version_1(input, log, value);
}

auto version_9(const Purse& input, const Log& log, UnallocatedCString& value)
    -> bool
{
    UNDEFINED_VERSION(9);
}

auto version_10(const Purse& input, const Log& log) -> bool
{
    auto value = "0"s;

    return version_1(input, log, value);
}

auto version_10(const Purse& input, const Log& log, UnallocatedCString& value)
    -> bool
{
    UNDEFINED_VERSION(10);
}

auto version_11(const Purse& input, const Log& log) -> bool
{
    auto value = "0"s;

    return version_1(input, log, value);
}

auto version_11(const Purse& input, const Log& log, UnallocatedCString& value)
    -> bool
{
    UNDEFINED_VERSION(11);
}

auto version_12(const Purse& input, const Log& log) -> bool
{
    auto value = "0"s;

    return version_1(input, log, value);
}

auto version_12(const Purse& input, const Log& log, UnallocatedCString& value)
    -> bool
{
    UNDEFINED_VERSION(12);
}

auto version_13(const Purse& input, const Log& log) -> bool
{
    auto value = "0"s;

    return version_1(input, log, value);
}

auto version_13(const Purse& input, const Log& log, UnallocatedCString& value)
    -> bool
{
    UNDEFINED_VERSION(13);
}

auto version_14(const Purse& input, const Log& log) -> bool
{
    auto value = "0"s;

    return version_1(input, log, value);
}

auto version_14(const Purse& input, const Log& log, UnallocatedCString& value)
    -> bool
{
    UNDEFINED_VERSION(14);
}

auto version_15(const Purse& input, const Log& log) -> bool
{
    auto value = "0"s;

    return version_1(input, log, value);
}

auto version_15(const Purse& input, const Log& log, UnallocatedCString& value)
    -> bool
{
    UNDEFINED_VERSION(15);
}

auto version_16(const Purse& input, const Log& log) -> bool
{
    auto value = "0"s;

    return version_1(input, log, value);
}

auto version_16(const Purse& input, const Log& log, UnallocatedCString& value)
    -> bool
{
    UNDEFINED_VERSION(16);
}

auto version_17(const Purse& input, const Log& log) -> bool
{
    auto value = "0"s;

    return version_1(input, log, value);
}

auto version_17(const Purse& input, const Log& log, UnallocatedCString& value)
    -> bool
{
    UNDEFINED_VERSION(17);
}

auto version_18(const Purse& input, const Log& log) -> bool
{
    auto value = "0"s;

    return version_1(input, log, value);
}

auto version_18(const Purse& input, const Log& log, UnallocatedCString& value)
    -> bool
{
    UNDEFINED_VERSION(18);
}

auto version_19(const Purse& input, const Log& log) -> bool
{
    auto value = "0"s;

    return version_1(input, log, value);
}

auto version_19(const Purse& input, const Log& log, UnallocatedCString& value)
    -> bool
{
    UNDEFINED_VERSION(19);
}

auto version_20(const Purse& input, const Log& log) -> bool
{
    auto value = "0"s;

    return version_1(input, log, value);
}

auto version_20(const Purse& input, const Log& log, UnallocatedCString& value)
    -> bool
{
    UNDEFINED_VERSION(20);
}
}  // namespace opentxs::protobuf::inline syntax

#include "opentxs/protobuf/syntax/Macros.undefine.inc"  // IWYU pragma: keep
