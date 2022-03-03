// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>

#include "opentxs/Version.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs
{
class String;

/** Useful for storing a UnallocatedSet of longs, serializing to/from
 * comma-separated string, And easily being able to add/remove/verify the
 * individual transaction numbers that are there. (Used by OTTransaction::blank
 * and OTTransaction::successNotice.) Also used in OTMessage, for storing lists
 * of acknowledged request numbers. */
class NumList
{
    UnallocatedSet<std::int64_t> m_setData;

    /** private for security reasons, used internally only by a function that
     * knows the string length already. if false, means the numbers were already
     * there. (At least one of them.) */
    auto Add(const char* szfNumbers) -> bool;

public:
    explicit NumList(const UnallocatedSet<std::int64_t>& theNumbers);
    explicit NumList(UnallocatedSet<std::int64_t>&& theNumbers);
    explicit NumList(const String& strNumbers);
    explicit NumList(const UnallocatedCString& strNumbers);
    explicit NumList(std::int64_t lInput);
    NumList();
    NumList(const NumList&) = default;
    NumList(NumList&&) = default;
    auto operator=(const NumList&) -> NumList& = default;
    auto operator=(NumList&&) -> NumList& = default;

    ~NumList() = default;

    /** if false, means the numbers were already there. (At least one of them.)
     */
    auto Add(const String& strNumbers) -> bool;

    /** if false, means the numbers were already there. (At least one of them.)
     */
    auto Add(const UnallocatedCString& strNumbers) -> bool;

    /** if false, means the value was already there. */
    auto Add(const std::int64_t& theValue) -> bool;

    /** if false, means the value was NOT already there. */
    auto Remove(const std::int64_t& theValue) -> bool;

    /** returns true/false (whether value is already there.) */
    auto Verify(const std::int64_t& theValue) const -> bool;

    /** if false, means the numbers were already there. (At least one of them.)
     */
    auto Add(const NumList& theNumList) -> bool;

    /** if false, means the numbers were already there. (At least one of them.)
     */
    auto Add(const UnallocatedSet<std::int64_t>& theNumbers) -> bool;

    /** if false, means the numbers were NOT already there. (At least one of
     * them.) */
    auto Remove(const UnallocatedSet<std::int64_t>& theNumbers) -> bool;

    /** True/False, based on whether values are already there. (ALL theNumbers
     * must be present.) */
    auto Verify(const UnallocatedSet<std::int64_t>& theNumbers) const -> bool;

    /** True/False, based on whether OTNumLists MATCH in COUNT and CONTENT (NOT
     * ORDER.) */
    auto Verify(const NumList& rhs) const -> bool;

    /** True/False, based on whether ANY of rhs are found in *this. */
    auto VerifyAny(const NumList& rhs) const -> bool;

    /** Verify whether ANY of the numbers on *this are found in setData. */
    auto VerifyAny(const UnallocatedSet<std::int64_t>& setData) const -> bool;
    auto Count() const -> std::int32_t;
    auto Peek(std::int64_t& lPeek) const -> bool;
    auto Pop() -> bool;

    /** Outputs the numlist as set of numbers. (To iterate OTNumList, call this,
     * then iterate the output.) returns false if the numlist was empty.*/
    auto Output(UnallocatedSet<std::int64_t>& theOutput) const -> bool;

    /** Outputs the numlist as a comma-separated string (for serialization,
     * usually.) returns false if the numlist was empty. */
    auto Output(String& strOutput) const -> bool;
    void Release();
};
}  // namespace opentxs
