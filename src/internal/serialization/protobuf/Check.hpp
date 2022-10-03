// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <string_view>

#include "opentxs/util/Container.hpp"

namespace opentxs::proto
{
void PrintErrorMessage(const char* proto, std::string_view error) noexcept;
void PrintErrorMessage(
    const char* proto,
    const unsigned int version,
    std::string_view error) noexcept;
void PrintErrorMessage(
    const char* proto,
    const unsigned int version,
    std::string_view error) noexcept;
void PrintErrorMessage(
    const char* proto,
    const unsigned int version,
    std::string_view error,
    const long long int value) noexcept;
void PrintErrorMessage(
    const char* proto,
    const unsigned int version,
    std::string_view error,
    std::string_view value) noexcept;

template <typename T, typename... Args>
auto Check(
    const T& input,
    const std::uint32_t minVersion,
    const std::uint32_t maxVersion,
    const bool silent,
    Args&&... params) noexcept -> bool
{
    if (false == input.has_version()) {
        PrintErrorMessage("protobuf", "missing version");

        return false;
    }

    const std::uint32_t version = input.version();

    if ((version < minVersion) || (version > maxVersion)) {
        PrintErrorMessage(
            "protobuf", input.version(), "incorrect version", input.version());

        return false;
    }

    switch (version) {
        case 1: {
            return CheckProto_1(input, silent, params...);
        }
        case 2: {
            return CheckProto_2(input, silent, params...);
        }
        case 3: {
            return CheckProto_3(input, silent, params...);
        }
        case 4: {
            return CheckProto_4(input, silent, params...);
        }
        case 5: {
            return CheckProto_5(input, silent, params...);
        }
        case 6: {
            return CheckProto_6(input, silent, params...);
        }
        case 7: {
            return CheckProto_7(input, silent, params...);
        }
        case 8: {
            return CheckProto_8(input, silent, params...);
        }
        case 9: {
            return CheckProto_9(input, silent, params...);
        }
        case 10: {
            return CheckProto_10(input, silent, params...);
        }
        case 11: {
            return CheckProto_11(input, silent, params...);
        }
        case 12: {
            return CheckProto_12(input, silent, params...);
        }
        case 13: {
            return CheckProto_13(input, silent, params...);
        }
        case 14: {
            return CheckProto_14(input, silent, params...);
        }
        case 15: {
            return CheckProto_15(input, silent, params...);
        }
        case 16: {
            return CheckProto_16(input, silent, params...);
        }
        case 17: {
            return CheckProto_17(input, silent, params...);
        }
        case 18: {
            return CheckProto_18(input, silent, params...);
        }
        case 19: {
            return CheckProto_19(input, silent, params...);
        }
        case 20: {
            return CheckProto_20(input, silent, params...);
        }
        default: {
            PrintErrorMessage(
                "protobuf", input.version(), "unsupported version");

            return false;
        }
    }

    return true;
}

template <typename T, typename... Args>
auto Validate(const T& input, const bool silent, Args&&... params) noexcept
    -> bool
{

    if (!input.has_version()) {
        PrintErrorMessage("protobuf", "missing version");

        return false;
    }

    const std::uint32_t version = input.version();

    return Check<T>(input, version, version, silent, params...);
}
}  // namespace opentxs::proto
