// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/type_index.hpp>
#include <cstdint>
#include <sstream>
#include <string>

#include "opentxs/protobuf/syntax/Types.internal.hpp"
#include "opentxs/util/Numbers.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Log;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::protobuf::inline syntax
{
template <typename T, typename... Args>
auto check_version(
    const T& input,
    const Log& log,
    VersionNumber minVersion,
    VersionNumber maxVersion,
    Args&&... params) noexcept -> bool
{
    if (false == input.has_version()) {
        print_error_message(
            log, input.GetTypeName().c_str(), "missing version");

        return false;
    }

    const std::uint32_t version = input.version();

    if ((version < minVersion) || (version > maxVersion)) {
        print_error_message(
            log,
            input.GetTypeName().c_str(),
            input.version(),
            "incorrect version",
            input.version());

        return false;
    }

    switch (version) {
        case 1: {
            return version_1(input, log, params...);
        }
        case 2: {
            return version_2(input, log, params...);
        }
        case 3: {
            return version_3(input, log, params...);
        }
        case 4: {
            return version_4(input, log, params...);
        }
        case 5: {
            return version_5(input, log, params...);
        }
        case 6: {
            return version_6(input, log, params...);
        }
        case 7: {
            return version_7(input, log, params...);
        }
        case 8: {
            return version_8(input, log, params...);
        }
        case 9: {
            return version_9(input, log, params...);
        }
        case 10: {
            return version_10(input, log, params...);
        }
        case 11: {
            return version_11(input, log, params...);
        }
        case 12: {
            return version_12(input, log, params...);
        }
        case 13: {
            return version_13(input, log, params...);
        }
        case 14: {
            return version_14(input, log, params...);
        }
        case 15: {
            return version_15(input, log, params...);
        }
        case 16: {
            return version_16(input, log, params...);
        }
        case 17: {
            return version_17(input, log, params...);
        }
        case 18: {
            return version_18(input, log, params...);
        }
        case 19: {
            return version_19(input, log, params...);
        }
        case 20: {
            return version_20(input, log, params...);
        }
        default: {
            print_error_message(
                log,
                input.GetTypeName().c_str(),
                input.version(),
                "unsupported version");

            return false;
        }
    }

    return true;
}

template <typename T, typename... Args>
auto check(const Log& log, const T& input, Args&&... params) noexcept -> bool
{
    if (!input.has_version()) {
        print_error_message(
            log, input.GetTypeName().c_str(), "missing version");

        return false;
    }

    const auto version = input.version();

    return check_version<T>(input, log, version, version, params...);
}

template <typename T>
auto get_protobuf_name(const T&) noexcept -> const std::string&
{
    static const auto out = [] {
        const auto name = boost::typeindex::type_id<T>().pretty_name();
        auto ss = std::stringstream{};
        ss << name;

        return ss.str();
    }();

    return out;
}
}  // namespace opentxs::protobuf::inline syntax
