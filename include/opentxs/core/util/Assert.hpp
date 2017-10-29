/************************************************************
 *
 *                 OPEN TRANSACTIONS
 *
 *       Financial Cryptography and Digital Cash
 *       Library, Protocol, API, Server, CLI, GUI
 *
 *       -- Anonymous Numbered Accounts.
 *       -- Untraceable Digital Cash.
 *       -- Triple-Signed Receipts.
 *       -- Cheques, Vouchers, Transfers, Inboxes.
 *       -- Basket Currencies, Markets, Payment Plans.
 *       -- Signed, XML, Ricardian-style Contracts.
 *       -- Scripted smart contracts.
 *
 *  EMAIL:
 *  fellowtraveler@opentransactions.org
 *
 *  WEBSITE:
 *  http://www.opentransactions.org/
 *
 *  -----------------------------------------------------
 *
 *   LICENSE:
 *   This Source Code Form is subject to the terms of the
 *   Mozilla Public License, v. 2.0. If a copy of the MPL
 *   was not distributed with this file, You can obtain one
 *   at http://mozilla.org/MPL/2.0/.
 *
 *   DISCLAIMER:
 *   This program is distributed in the hope that it will
 *   be useful, but WITHOUT ANY WARRANTY; without even the
 *   implied warranty of MERCHANTABILITY or FITNESS FOR A
 *   PARTICULAR PURPOSE.  See the Mozilla Public License
 *   for more details.
 *
 ************************************************************/

#ifndef OPENTXS_CORE_ASSERT_HPP
#define OPENTXS_CORE_ASSERT_HPP

#include <cstddef>
#include <exception>
#include <string>

#define OT_TRACE                                                               \
    {                                                                          \
        Assert::doAssert(__FILE__, __LINE__, nullptr);                         \
    };
#define OT_FAIL                                                                \
    {                                                                          \
        Assert::doAssert(__FILE__, __LINE__, nullptr);                         \
        std::terminate();                                                      \
    };
#define OT_FAIL_MSG(s)                                                         \
    {                                                                          \
        Assert::doAssert(__FILE__, __LINE__, (s));                             \
        std::terminate();                                                      \
    };
#define OT_ASSERT(x)                                                           \
    if (false == static_cast<bool>(x)) {                                       \
        Assert::doAssert(__FILE__, __LINE__, nullptr);                         \
        std::terminate();                                                      \
    };
#define OT_ASSERT_MSG(x, s)                                                    \
    if (false == static_cast<bool>(x)) {                                       \
        Assert::doAssert(__FILE__, __LINE__, (s));                             \
        std::terminate();                                                      \
    };

//#ifndef OT_METHOD
//#define OT_METHOD "UNKNOWN"
//#endif // OT_METHOD

#define OT_INTERMEDIATE_FORMAT(OT_THE_ERROR_STRING)                            \
    ((std::string(OT_METHOD) + std::string(__FUNCTION__) + std::string(": ") + \
      std::string(OT_THE_ERROR_STRING) + std::string("\n"))                    \
         .c_str())

#define OT_TO_STR_A(A) #A
#define OT_TO_STR(A) OT_TO_STR_A(A)
// -------------------------------------------------------

#define OT_ID_FORMAT(OT_ID_OBJECT)                                             \
    ((std::string(OT_METHOD) + std::string(__FUNCTION__) +                     \
      std::string(": Empty ID for '") + std::string(OT_TO_STR(OT_ID_OBJECT)) + \
      std::string("' passed in to the API (by the client application).\n"))    \
         .c_str())
// -------------------------------------------------------
#define OT_OTHER_ID_FORMAT(OT_ID_OBJECT)                                       \
    ((std::string(OT_METHOD) + std::string(__FUNCTION__) +                     \
      std::string(": Empty or invalid ID for '") +                             \
      std::string(OT_TO_STR(OT_ID_OBJECT)) +                                   \
      std::string("' passed in to the API (by the client application).\n"))    \
         .c_str())
// -------------------------------------------------------
#define OT_BOUNDS_FORMAT(OT_NUMBER)                                            \
    ((std::string(OT_METHOD) + std::string(__FUNCTION__) +                     \
      std::string(": Out-of-bounds value for '") +                             \
      std::string(OT_TO_STR(OT_NUMBER)) +                                      \
      std::string("' passed in to the API (by the client application).\n"))    \
         .c_str())
// -------------------------------------------------------
#define OT_MIN_BOUND_FORMAT(OT_NUMBER)                                         \
    ((std::string(OT_METHOD) + std::string(__FUNCTION__) +                     \
      std::string(": Lower-than-minimum allowed value for '") +                \
      std::string(OT_TO_STR(OT_NUMBER)) +                                      \
      std::string("' passed in to the API (by the client application).\n"))    \
         .c_str())
// -------------------------------------------------------
#define OT_STD_STR_FORMAT(OT_STRING_INPUT)                                     \
    ((std::string(OT_METHOD) + std::string(__FUNCTION__) +                     \
      std::string(": Empty string for '") +                                    \
      std::string(OT_TO_STR(OT_STRING_INPUT)) +                                \
      std::string("' passed in to the API (by the client application).\n"))    \
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
                                                                               \
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
    OT_ASSERT_MSG((!(OT_ID_OBJECT).IsEmpty()), OT_ID_FORMAT(OT_ID_OBJECT))
// -------------------------------------------------------
// Verify that the ID isn't empty, and that it contains
// a valid Opentxs ID. Otherwise, assert with a message.
//
#define OT_VERIFY_ID_STR(STD_STR_OF_ID)                                        \
                                                                               \
    OT_ASSERT_MSG(                                                             \
        (!(STD_STR_OF_ID).empty() &&                                           \
         (Identifier::validateID((STD_STR_OF_ID)))),                           \
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
// -------------------------------------------------------

class Assert
{
public:
    typedef size_t(fpt_Assert_sz_n_sz)(const char*, size_t, const char*);

private:
    fpt_Assert_sz_n_sz* m_fpt_Assert{nullptr};

    fpt_Assert_sz_n_sz(m_AssertDefault);

public:
    // if not null, must be deleted before changed.
    static Assert* s_pAssert;

    EXPORT Assert(fpt_Assert_sz_n_sz& fp1);

    EXPORT static fpt_Assert_sz_n_sz(doAssert);  // assert
};

#endif  // OPENTXS_CORE_ASSERT_HPP
