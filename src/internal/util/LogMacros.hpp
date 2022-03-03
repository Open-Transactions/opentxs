// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/type_index.hpp>
#include <sstream>

#include "opentxs/util/Log.hpp"

namespace opentxs
{
template <typename T>
auto pretty_function(const char* function) noexcept -> UnallocatedCString
{
    static const auto name = boost::typeindex::type_id<T>().pretty_name();
    auto out = std::stringstream{};
    out << name << "::" << function << ": ";

    return out.str();
}

template <typename T>
auto pretty_function(T*, const char* function) noexcept -> UnallocatedCString
{
    return pretty_function<T>(function);
}
}  // namespace opentxs

#define OT_PRETTY_CLASS() opentxs::pretty_function(this, __func__)
#define OT_PRETTY_STATIC(C) opentxs::pretty_function<C>(__func__)

#define OT_TRACE                                                               \
    {                                                                          \
        ::opentxs::LogError().Trace(__FILE__, __LINE__, nullptr);              \
    };
#define OT_FAIL                                                                \
    {                                                                          \
        ::opentxs::LogError().Assert(__FILE__, __LINE__, nullptr);             \
    };
#define OT_FAIL_MSG(s)                                                         \
    {                                                                          \
        ::opentxs::LogError().Assert(__FILE__, __LINE__, (s));                 \
    };
#define OT_ASSERT(x)                                                           \
    if (false == static_cast<bool>(x)) {                                       \
        ::opentxs::LogError().Assert(__FILE__, __LINE__, nullptr);             \
    };
#define OT_ASSERT_MSG(x, s)                                                    \
    if (false == static_cast<bool>(x)) {                                       \
        ::opentxs::LogError().Assert(__FILE__, __LINE__, (s));                 \
    };

#define OT_INTERMEDIATE_FORMAT(OT_THE_ERROR_STRING)                            \
    (((OT_PRETTY_CLASS()) + UnallocatedCString(OT_THE_ERROR_STRING) +          \
      UnallocatedCString("\n"))                                                \
         .c_str())

#define OT_TO_STR_A(A) #A
#define OT_TO_STR(A) OT_TO_STR_A(A)

#define OT_ID_FORMAT(OT_ID_OBJECT)                                             \
    (((OT_PRETTY_CLASS()) + UnallocatedCString("Empty ID for '") +             \
      UnallocatedCString(OT_TO_STR(OT_ID_OBJECT)) +                            \
      UnallocatedCString(                                                      \
          "' passed in to the API (by the client application).\n"))            \
         .c_str())

#define OT_OTHER_ID_FORMAT(OT_ID_OBJECT)                                       \
    (((OT_PRETTY_CLASS()) + UnallocatedCString("Empty or invalid ID for '") +  \
      UnallocatedCString(OT_TO_STR(OT_ID_OBJECT)) +                            \
      UnallocatedCString(                                                      \
          "' passed in to the API (by the client application).\n"))            \
         .c_str())

#define OT_BOUNDS_FORMAT(OT_NUMBER)                                            \
    (((OT_PRETTY_CLASS()) + UnallocatedCString("Out-of-bounds value for '") +  \
      UnallocatedCString(OT_TO_STR(OT_NUMBER)) +                               \
      UnallocatedCString(                                                      \
          "' passed in to the API (by the client application).\n"))            \
         .c_str())

#define OT_MIN_BOUND_FORMAT(OT_NUMBER)                                         \
    (((OT_PRETTY_CLASS()) +                                                    \
      UnallocatedCString("Lower-than-minimum allowed value for '") +           \
      UnallocatedCString(OT_TO_STR(OT_NUMBER)) +                               \
      UnallocatedCString(                                                      \
          "' passed in to the API (by the client application).\n"))            \
         .c_str())

#define OT_STD_STR_FORMAT(OT_STRING_INPUT)                                     \
    (((OT_PRETTY_CLASS()) + UnallocatedCString(": Empty string for '") +       \
      UnallocatedCString(OT_TO_STR(OT_STRING_INPUT)) +                         \
      UnallocatedCString(                                                      \
          "' passed in to the API (by the client application).\n"))            \
         .c_str())

// -------------------------------------------------------
// OT_NEW_ASSERT_MSG
// This is it -- the golden banana.
//
#define OT_NEW_ASSERT_MSG(X, Z)                                                \
                                                                               \
    OT_ASSERT_MSG((X), (OT_INTERMEDIATE_FORMAT((Z))))
//
// This one is the same thing except without a message.
//
#define OT_NEW_ASSERT(X)                                                       \
    OT_ASSERT_MSG(                                                             \
        (X),                                                                   \
        (OT_INTERMEDIATE_FORMAT(("This space intentionally left blank."))))

// -------------------------------------------------------
// OT_VERIFY_OT_ID
// (Verify an opentxs Identifier object).
// Verify that the ID isn't empty, and that it contains
// a valid Opentxs ID. Otherwise, assert with a message.
//
#define OT_VERIFY_OT_ID(OT_ID_OBJECT)                                          \
                                                                               \
    OT_ASSERT_MSG((!(OT_ID_OBJECT).empty()), OT_ID_FORMAT(OT_ID_OBJECT))

// -------------------------------------------------------
// Verify that the ID isn't empty, and that it contains
// a valid Opentxs ID. Otherwise, assert with a message.
//
#define OT_VERIFY_ID_STR(STD_STR_OF_ID)                                        \
                                                                               \
    OT_ASSERT_MSG(                                                             \
        (!(STD_STR_OF_ID).empty() && (Identifier::Validate((STD_STR_OF_ID)))), \
        OT_OTHER_ID_FORMAT(STD_STR_OF_ID))

// -------------------------------------------------------
// OT_VERIFY_BOUNDS
// Bounds check a number. Usually to determine that it's
// larger than or equal to zero, and less than the size
// of some container.
//
#define OT_VERIFY_BOUNDS(                                                      \
    OT_NUMBER, OT_BOUNDS_MIN_INDEX, OT_BOUNDS_CONTAINER_SIZE)                  \
                                                                               \
    OT_ASSERT_MSG(                                                             \
        (((OT_NUMBER) >= (OT_BOUNDS_MIN_INDEX)) &&                             \
         ((OT_NUMBER) < (OT_BOUNDS_CONTAINER_SIZE))),                          \
        OT_BOUNDS_FORMAT((OT_NUMBER)))

// -------------------------------------------------------
// OT_VERIFY_MIN_BOUND
// Usually used to bounds-check a number to determine that
// it's larger than or equal to zero.
//
#define OT_VERIFY_MIN_BOUND(OT_NUMBER, OT_BOUNDS_MIN_INDEX)                    \
                                                                               \
    OT_ASSERT_MSG(                                                             \
        ((OT_NUMBER) >= (OT_BOUNDS_MIN_INDEX)),                                \
        OT_MIN_BOUND_FORMAT((OT_NUMBER)))

// -------------------------------------------------------
// OT_VERIFY_STD_STR
// Only verifies currently that the string is "not empty."
// Used for string input to the API such as a string containing
// a ledger, or a string containing a transaction, etc.
//
#define OT_VERIFY_STD_STR(OT_STRING_INPUT)                                     \
                                                                               \
    OT_ASSERT_MSG(                                                             \
        (!((OT_STRING_INPUT).empty())), OT_STD_STR_FORMAT((OT_STRING_INPUT)))
