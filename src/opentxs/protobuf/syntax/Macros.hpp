// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// NOTE #pragma once was intentionally omitted.
// This file is intended to be used along with Macros.undefine.inc and in cases
// where CMAKE_UNITY_BUILD is used this file will be included multiple times in
// the unity sources.
// Always include Macros.undefine.inc at the bottom of any file which includes
// this header.

#include <string_view>

#include "opentxs/protobuf/syntax/Constants.hpp"       // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Types.internal.hpp"  // IWYU pragma: keep
#include "opentxs/protobuf/syntax/Types.internal.tpp"  // IWYU pragma: keep
#include "opentxs/util/Log.hpp"                        // IWYU pragma: keep

namespace opentxs::protobuf::inline syntax
{
using namespace std::literals;
}  // namespace opentxs::protobuf::inline syntax

#define FAIL_1(b)                                                              \
    print_error_message(                                                       \
        log, get_protobuf_name(input).c_str(), input.version(), b);            \
                                                                               \
    return false

#define FAIL_2(b, c)                                                           \
    print_error_message(                                                       \
        log, get_protobuf_name(input).c_str(), input.version(), b, c);         \
                                                                               \
    return false

#define FAIL_4(b, c, d, e)                                                     \
    log()("verify version ")(input.version())(" ")(get_protobuf_name(input))(  \
        " failed: ")(b)("(")(c)(")")(d)("(")(e)(")")                           \
        .Flush();                                                              \
                                                                               \
    return false

#define FAIL_6(b, c, d, e, f, g)                                               \
    log()("verify version ")(input.version())(" ")(get_protobuf_name(input))(  \
        " failed: ")(b)("(")(c)(")")(d)("(")(e)(")")(f)("(")(g)(")")           \
        .Flush();                                                              \
                                                                               \
    return false

#define CHECK_STRING_(a, min, max)                                             \
    if (input.has_##a() && (0 < input.a().size())) {                           \
        if ((min > input.a().size()) || (max < input.a().size())) {            \
            FAIL_2(                                                            \
                "invalid "s.append(#a).append(" size"sv).c_str(),              \
                input.a().size());                                             \
        }                                                                      \
    }                                                                          \
    static_assert(0 < sizeof(char))  // NOTE silence -Wextra-semi-stmt

#define CHECK_SUBOBJECT_(a, b)                                                 \
    if (input.has_##a()) {                                                     \
        try {                                                                  \
            const auto valid##a = check_version(                               \
                input.a(),                                                     \
                log,                                                           \
                b.at(input.version()).first,                                   \
                b.at(input.version()).second);                                 \
                                                                               \
            if (false == valid##a) { FAIL_1("invalid "s.append(#a).c_str()); } \
        } catch (const std::out_of_range&) {                                   \
            FAIL_2(                                                            \
                "allowed "s.append(#a)                                         \
                    .append(" version not defined for version"sv)              \
                    .c_str(),                                                  \
                input.version());                                              \
        }                                                                      \
    }                                                                          \
    static_assert(0 < sizeof(char))  // NOTE silence -Wextra-semi-stmt

#define CHECK_SUBOBJECT_VA_(a, b, ...)                                         \
    if (input.has_##a()) {                                                     \
        try {                                                                  \
            const auto valid##a = check_version(                               \
                input.a(),                                                     \
                log,                                                           \
                b.at(input.version()).first,                                   \
                b.at(input.version()).second,                                  \
                __VA_ARGS__);                                                  \
                                                                               \
            if (false == valid##a) { FAIL_1("invalid "s.append(#a).c_str()); } \
        } catch (const std::out_of_range&) {                                   \
            FAIL_2(                                                            \
                "allowed "s.append(#a)                                         \
                    .append(" version not defined for version"sv)              \
                    .c_str(),                                                  \
                input.version());                                              \
        }                                                                      \
    }                                                                          \
    static_assert(0 < sizeof(char))  // NOTE silence -Wextra-semi-stmt

#define CHECK_SUBOBJECTS_(a, b)                                                \
    for (const auto& it : input.a()) {                                         \
        try {                                                                  \
            const auto valid##a = check_version(                               \
                it,                                                            \
                log,                                                           \
                b.at(input.version()).first,                                   \
                b.at(input.version()).second);                                 \
                                                                               \
            if (false == valid##a) { FAIL_1("invalid "s.append(#a).c_str()); } \
        } catch (const std::out_of_range&) {                                   \
            FAIL_2(                                                            \
                "allowed "s.append(#a)                                         \
                    .append(" version not defined for version"sv)              \
                    .c_str(),                                                  \
                input.version());                                              \
        }                                                                      \
    }                                                                          \
    static_assert(0 < sizeof(char))  // NOTE silence -Wextra-semi-stmt

#define CHECK_SUBOBJECTS_VA_(a, b, ...)                                        \
    for (const auto& it : input.a()) {                                         \
        try {                                                                  \
            const auto valid##a = check_version(                               \
                it,                                                            \
                log,                                                           \
                b.at(input.version()).first,                                   \
                b.at(input.version()).second,                                  \
                __VA_ARGS__);                                                  \
                                                                               \
            if (false == valid##a) { FAIL_1("invalid "s.append(#a).c_str()); } \
        } catch (const std::out_of_range&) {                                   \
            FAIL_2(                                                            \
                "allowed "s.append(#a)                                         \
                    .append(" version not defined for version"sv)              \
                    .c_str(),                                                  \
                input.version());                                              \
        }                                                                      \
    }                                                                          \
    static_assert(0 < sizeof(char))  // NOTE silence -Wextra-semi-stmt

#define CHECK_EXCLUDED(a)                                                      \
    if (true == input.has_##a()) {                                             \
        FAIL_1("unexpected "s.append(#a).append(" present"sv).c_str());        \
    }                                                                          \
    static_assert(0 < sizeof(char))  // NOTE silence -Wextra-semi-stmt

#define CHECK_EXISTS(a)                                                        \
    if (false == input.has_##a()) { FAIL_1("missing "s.append(#a).c_str()); }  \
    static_assert(0 < sizeof(char))  // NOTE silence -Wextra-semi-stmt

#define CHECK_EXISTS_STRING(a)                                                 \
    if ((false == input.has_##a()) || (0 == input.a().size())) {               \
        FAIL_1("missing "s.append(#a).c_str());                                \
    }                                                                          \
    static_assert(0 < sizeof(char))  // NOTE silence -Wextra-semi-stmt

#define CHECK_KEY(a)                                                           \
    CHECK_EXISTS_STRING(a);                                                    \
    OPTIONAL_KEY(a)

#define CHECK_HAVE(a)                                                          \
    if (0 == input.a().size()) { FAIL_1("missing "s.append(#a).c_str()); }     \
    static_assert(0 < sizeof(char))  // NOTE silence -Wextra-semi-stmt

#define CHECK_IDENTIFIER(a)                                                    \
    CHECK_EXISTS_STRING(a);                                                    \
    OPTIONAL_IDENTIFIER(a)

#define CHECK_IDENTIFIERS(a)                                                   \
    /* CHECK_EXISTS(a); */                                                     \
    OPTIONAL_IDENTIFIERS(a)

#define CHECK_MEMBERSHIP(a, b)                                                 \
    try {                                                                      \
        const bool valid##a = 1 == b.at(input.version()).count(input.a());     \
                                                                               \
        if (false == valid##a) { FAIL_2("invalid "s.append(#a), input.a()); }  \
    } catch (const std::out_of_range&) {                                       \
        FAIL_2(                                                                \
            "allowed "s.append(#a)                                             \
                .append(" not defined for version"sv)                          \
                .c_str(),                                                      \
            input.version());                                                  \
    }                                                                          \
    static_assert(0 < sizeof(char))  // NOTE silence -Wextra-semi-stmt

#define CHECK_NAME(a)                                                          \
    CHECK_EXISTS_STRING(a);                                                    \
    OPTIONAL_NAME(a)

#define CHECK_NAMES(a)                                                         \
    CHECK_EXISTS(a);                                                           \
    OPTIONAL_IDENTIFIERS(a)

#define CHECK_NONE(a)                                                          \
    if (0 < input.a().size()) {                                                \
        FAIL_1("unexpected "s.append(#a).append(" present"sv).c_str());        \
    }                                                                          \
    static_assert(0 < sizeof(char))  // NOTE silence -Wextra-semi-stmt

#define CHECK_SIZE(a, b)                                                       \
    if (b != input.a().size()) {                                               \
        FAIL_2(                                                                \
            "unexpected "s.append(#a).append(" present "sv).c_str(),           \
            input.a().size());                                                 \
    }                                                                          \
    static_assert(0 < sizeof(char))  // NOTE silence -Wextra-semi-stmt

#define CHECK_SCRIPT(a)                                                        \
    CHECK_EXISTS_STRING(a);                                                    \
    OPTIONAL_SCRIPT(a)

#define CHECK_SUBOBJECT(a, b)                                                  \
    CHECK_EXISTS(a);                                                           \
    OPTIONAL_SUBOBJECT(a, b)

#define CHECK_SUBOBJECT_VA(a, b, ...)                                          \
    CHECK_EXISTS(a);                                                           \
    OPTIONAL_SUBOBJECT_VA(a, b, __VA_ARGS__)

#define CHECK_SUBOBJECTS(a, b)                                                 \
    /* CHECK_HAVE(a); */                                                       \
    OPTIONAL_SUBOBJECTS(a, b)

#define CHECK_SUBOBJECTS_VA(a, b, ...)                                         \
    CHECK_HAVE(a);                                                             \
    OPTIONAL_SUBOBJECTS_VA(a, b, __VA_ARGS__)

#define CHECK_VALUE(a, b)                                                      \
    CHECK_EXISTS(a);                                                           \
    {                                                                          \
        const bool valid##a = 1 == (input.a() == b);                           \
                                                                               \
        if (false == valid##a) {                                               \
            FAIL_4(                                                            \
                "invalid "s.append(#a).c_str(), input.a(), " expected "sv, b); \
        }                                                                      \
    }                                                                          \
    static_assert(0 < sizeof(char))  // NOTE silence -Wextra-semi-stmt

#define MAX_IDENTIFIER(a) CHECK_STRING_(a, 0, MAX_PLAUSIBLE_IDENTIFIER)

#define OPTIONAL_IDENTIFIER(a)                                                 \
    CHECK_STRING_(a, MIN_PLAUSIBLE_IDENTIFIER, MAX_PLAUSIBLE_IDENTIFIER)

#define OPTIONAL_IDENTIFIERS(a)                                                \
    for (const auto& it : input.a()) {                                         \
        if ((MIN_PLAUSIBLE_IDENTIFIER > it.size()) ||                          \
            (MAX_PLAUSIBLE_IDENTIFIER < it.size())) {                          \
            FAIL_2(                                                            \
                "invalid "s.append(#a).append(" size"sv).c_str(), it.size());  \
        }                                                                      \
    }                                                                          \
    static_assert(0 < sizeof(char))  // NOTE silence -Wextra-semi-stmt

#define OPTIONAL_KEY(a)                                                        \
    CHECK_STRING_(a, MIN_PLAUSIBLE_KEYSIZE, MAX_PLAUSIBLE_KEYSIZE)

#define OPTIONAL_NAME(a) CHECK_STRING_(a, 1, MAX_VALID_CONTACT_VALUE)

#define OPTIONAL_NAMES(a)                                                      \
    for (const auto& it : input.a()) {                                         \
        if ((1 > it.size()) || (MAX_VALID_CONTACT_VALUE < it.size())) {        \
            FAIL_2(                                                            \
                "invalid "s.append(#a).append(" size"sv).c_str(), it.size());  \
        }                                                                      \
    }                                                                          \
    static_assert(0 < sizeof(char))  // NOTE silence -Wextra-semi-stmt

#define OPTIONAL_SCRIPT(a)                                                     \
    CHECK_STRING_(a, MIN_PLAUSIBLE_SCRIPT, MAX_PLAUSIBLE_SCRIPT)

#define OPTIONAL_SUBOBJECT(a, b) CHECK_SUBOBJECT_(a, b)

#define OPTIONAL_SUBOBJECT_VA(a, b, ...) CHECK_SUBOBJECT_VA_(a, b, __VA_ARGS__)

#define OPTIONAL_SUBOBJECTS(a, b) CHECK_SUBOBJECTS_(a, b)

#define OPTIONAL_SUBOBJECTS_VA(a, b, ...)                                      \
    CHECK_SUBOBJECTS_VA_(a, b, __VA_ARGS__)

#define REQUIRED_SIZE(a, b) CHECK_STRING_(a, b, b)

#define UNDEFINED_VERSION(b)                                                   \
    print_error_message(                                                       \
        log,                                                                   \
        get_protobuf_name(input).c_str(),                                      \
        input.version(),                                                       \
        "undefined version",                                                   \
        b);                                                                    \
                                                                               \
    return false
