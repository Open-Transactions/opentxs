// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <Enums.pb.h>
#include <boost/type_index.hpp>
#include <cstddef>
#include <cstdint>
#include <sstream>
#include <string_view>

#include "internal/serialization/protobuf/Check.hpp"  // IWYU pragma: keep
#include "opentxs/util/Container.hpp"

namespace opentxs::proto
{
using namespace std::literals;

static const UnallocatedMap<std::uint32_t, UnallocatedSet<AsymmetricKeyType>>
    AsymmetricKeyAllowedTypes{
        {1, {AKEYTYPE_LEGACY, AKEYTYPE_SECP256K1, AKEYTYPE_ED25519}},
        {2, {AKEYTYPE_LEGACY, AKEYTYPE_SECP256K1, AKEYTYPE_ED25519}},
    };
static const std::size_t MIN_PLAUSIBLE_IDENTIFIER{20};
static const std::size_t MAX_PLAUSIBLE_IDENTIFIER{116};
static const std::size_t MIN_PLAUSIBLE_KEYSIZE{16};
static const std::size_t MAX_PLAUSIBLE_KEYSIZE{8192};
static const std::size_t MIN_PLAUSIBLE_SIGNATURE{32};
static const std::uint32_t MAX_VALID_PORT{65535};
static const std::size_t MAX_VALID_CONTACT_VALUE{512};
static const std::size_t MAX_LABEL_SIZE{512};
static const std::size_t MAX_TRANSACTION_MEMO_SIZE{512};
static const std::size_t MIN_PLAUSIBLE_SCRIPT{2};
static const std::size_t MAX_PLAUSIBLE_SCRIPT{1048576};
static const std::size_t MAX_PLAUSIBLE_WORK{128};

template <typename T>
auto GetProtoName(const T& in) noexcept -> std::string
{
    static const auto name = boost::typeindex::type_id<T>().pretty_name();
    auto out = std::stringstream{};
    out << name;

    return out.str();
}

void WriteLogMessage(const std::stringstream& message) noexcept;
}  // namespace opentxs::proto

#define FAIL_1(b)                                                              \
    if (false == silent) {                                                     \
        PrintErrorMessage(GetProtoName(input).c_str(), input.version(), b);    \
    }                                                                          \
                                                                               \
    return false

#define FAIL_2(b, c)                                                           \
    if (false == silent) {                                                     \
        PrintErrorMessage(GetProtoName(input).c_str(), input.version(), b, c); \
    }                                                                          \
                                                                               \
    return false

#define FAIL_4(b, c, d, e)                                                     \
    if (false == silent) {                                                     \
        std::stringstream out{};                                               \
        out << "Verify version " << input.version() << " "                     \
            << GetProtoName(input) << " failed: " << b << "(" << c << ")" << d \
            << "(" << e << ")" << std::endl;                                   \
        WriteLogMessage(out);                                                  \
    }                                                                          \
                                                                               \
    return false

#define FAIL_6(b, c, d, e, f, g)                                               \
    if (false == silent) {                                                     \
        std::stringstream out{};                                               \
        out << "Verify version " << input.version() << " "                     \
            << GetProtoName(input) << " failed: " << b << "(" << c << ")" << d \
            << "(" << e << ")" << f << "(" << g << ")" << std::endl;           \
        WriteLogMessage(out);                                                  \
    }                                                                          \
                                                                               \
    return false

#define CHECK_STRING_(a, min, max)                                             \
    if (input.has_##a() && (0 < input.a().size())) {                           \
        if ((min > input.a().size()) || (max < input.a().size())) {            \
            const auto fail =                                                  \
                CString{"invalid "sv}.append(#a).append(" size"sv);            \
            FAIL_2(fail.c_str(), input.a().size());                            \
        }                                                                      \
    }                                                                          \
    static_assert(0 < sizeof(char))  // NOTE silence -Wextra-semi-stmt

#define CHECK_SUBOBJECT_(a, b, ...)                                            \
    if (input.has_##a()) {                                                     \
        try {                                                                  \
            const bool valid##a = Check(                                       \
                input.a(),                                                     \
                b.at(input.version()).first,                                   \
                b.at(input.version()).second,                                  \
                __VA_ARGS__);                                                  \
                                                                               \
            if (false == valid##a) {                                           \
                const auto fail = CString{"invalid "sv}.append(#a);            \
                FAIL_1(fail.c_str());                                          \
            }                                                                  \
        } catch (const std::out_of_range&) {                                   \
            const auto fail = CString{"allowed "sv}.append(#a).append(         \
                " version not defined for version"sv);                         \
            FAIL_2(fail.c_str(), input.version());                             \
        }                                                                      \
    }                                                                          \
    static_assert(0 < sizeof(char))  // NOTE silence -Wextra-semi-stmt

#define CHECK_SUBOBJECTS_(a, b, ...)                                           \
    for (const auto& it : input.a()) {                                         \
        try {                                                                  \
            const bool valid##a = Check(                                       \
                it,                                                            \
                b.at(input.version()).first,                                   \
                b.at(input.version()).second,                                  \
                __VA_ARGS__);                                                  \
                                                                               \
            if (false == valid##a) {                                           \
                const auto fail = CString{"invalid "sv}.append(#a);            \
                FAIL_1(fail.c_str());                                          \
            }                                                                  \
        } catch (const std::out_of_range&) {                                   \
            const auto fail = CString{"allowed "sv}.append(#a).append(         \
                " version not defined for version"sv);                         \
            FAIL_2(fail.c_str(), input.version());                             \
        }                                                                      \
    }                                                                          \
    static_assert(0 < sizeof(char))  // NOTE silence -Wextra-semi-stmt

#define CHECK_EXCLUDED(a)                                                      \
    if (true == input.has_##a()) {                                             \
        const auto fail =                                                      \
            CString{"unexpected "sv}.append(#a).append(" present"sv);          \
        FAIL_1(fail.c_str());                                                  \
    }                                                                          \
    static_assert(0 < sizeof(char))  // NOTE silence -Wextra-semi-stmt

#define CHECK_EXISTS(a)                                                        \
    if (false == input.has_##a()) {                                            \
        const auto fail = CString{"missing "sv}.append(#a);                    \
        FAIL_1(fail.c_str());                                                  \
    }                                                                          \
    static_assert(0 < sizeof(char))  // NOTE silence -Wextra-semi-stmt

#define CHECK_EXISTS_STRING(a)                                                 \
    if ((false == input.has_##a()) || (0 == input.a().size())) {               \
        const auto fail = CString{"missing "sv}.append(#a);                    \
        FAIL_1(fail.c_str());                                                  \
    }                                                                          \
    static_assert(0 < sizeof(char))  // NOTE silence -Wextra-semi-stmt

#define CHECK_KEY(a)                                                           \
    CHECK_EXISTS_STRING(a);                                                    \
    OPTIONAL_KEY(a)

#define CHECK_HAVE(a)                                                          \
    if (0 == input.a().size()) {                                               \
        const auto fail = CString{"missing "sv}.append(#a);                    \
        FAIL_1(fail.c_str());                                                  \
    }                                                                          \
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
        if (false == valid##a) {                                               \
            const auto fail = CString{"invalid "sv}.append(#a);                \
            FAIL_2(fail, input.a());                                           \
        }                                                                      \
    } catch (const std::out_of_range&) {                                       \
        const auto fail = CString{"allowed "sv}.append(#a).append(             \
            " not defined for version"sv);                                     \
        FAIL_2(fail.c_str(), input.version());                                 \
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
        const auto fail =                                                      \
            CString{"unexpected "sv}.append(#a).append(" present"sv);          \
        FAIL_1(fail.c_str());                                                  \
    }                                                                          \
    static_assert(0 < sizeof(char))  // NOTE silence -Wextra-semi-stmt

#define CHECK_SIZE(a, b)                                                       \
    if (b != input.a().size()) {                                               \
        const auto fail =                                                      \
            CString{"unexpected "sv}.append(#a).append(" present "sv);         \
        FAIL_2(fail.c_str(), input.a().size());                                \
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
            const auto fail = CString{"invalid "sv}.append(#a);                \
            FAIL_4(fail.c_str(), input.a(), " expected "sv, b);                \
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
            const auto fail =                                                  \
                CString{"invalid "sv}.append(#a).append(" size"sv);            \
            FAIL_2(fail.c_str(), it.size());                                   \
        }                                                                      \
    }                                                                          \
    static_assert(0 < sizeof(char))  // NOTE silence -Wextra-semi-stmt

#define OPTIONAL_KEY(a)                                                        \
    CHECK_STRING_(a, MIN_PLAUSIBLE_KEYSIZE, MAX_PLAUSIBLE_KEYSIZE)

#define OPTIONAL_NAME(a) CHECK_STRING_(a, 1, MAX_VALID_CONTACT_VALUE)

#define OPTIONAL_NAMES(a)                                                      \
    for (const auto& it : input.a()) {                                         \
        if ((1 > it.size()) || (MAX_VALID_CONTACT_VALUE < it.size())) {        \
            const auto fail =                                                  \
                CString{"invalid "sv}.append(#a).append(" size"sv);            \
            FAIL_2(fail.c_str(), it.size());                                   \
        }                                                                      \
    }                                                                          \
    static_assert(0 < sizeof(char))  // NOTE silence -Wextra-semi-stmt

#define OPTIONAL_SCRIPT(a)                                                     \
    CHECK_STRING_(a, MIN_PLAUSIBLE_SCRIPT, MAX_PLAUSIBLE_SCRIPT)

#define OPTIONAL_SUBOBJECT(a, b) CHECK_SUBOBJECT_(a, b, silent)

#define OPTIONAL_SUBOBJECT_VA(a, b, ...)                                       \
    CHECK_SUBOBJECT_(a, b, silent, __VA_ARGS__)

#define OPTIONAL_SUBOBJECTS(a, b) CHECK_SUBOBJECTS_(a, b, silent)

#define OPTIONAL_SUBOBJECTS_VA(a, b, ...)                                      \
    CHECK_SUBOBJECTS_(a, b, silent, __VA_ARGS__)

#define REQUIRED_SIZE(a, b) CHECK_STRING_(a, b, b)

#define UNDEFINED_VERSION(b)                                                   \
    if (false == silent) {                                                     \
        PrintErrorMessage(                                                     \
            GetProtoName(input).c_str(),                                       \
            input.version(),                                                   \
            "undefined version",                                               \
            b);                                                                \
    }                                                                          \
                                                                               \
    return false
