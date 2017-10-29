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

#include "opentxs/core/stdafx.hpp"

#include "opentxs/core/NumList.hpp"

#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/String.hpp"

#include <cinttypes>
#include <cstdint>
#include <locale>
#include <ostream>
#include <set>
#include <string>
#include <utility>

// OTNumList (helper class.)

namespace opentxs
{

NumList::NumList(const std::set<int64_t>& theNumbers) { Add(theNumbers); }

NumList::NumList(std::set<int64_t>&& theNumbers)
    : m_setData(std::move(theNumbers))
{
}

NumList::NumList(int64_t lInput) { Add(lInput); }

// removed, security reasons.
// OTNumList::OTNumList(const char* szNumbers)
//{
//    Add(szNumbers);
//}

NumList::NumList(const String& strNumbers) { Add(strNumbers); }

NumList::NumList(const std::string& strNumbers) { Add(strNumbers); }

NumList::NumList() {}

NumList::~NumList() {}

bool NumList::Add(const String& strNumbers)  // if false, means the numbers
                                             // were already there. (At least
                                             // one of them.)
{
    return Add(strNumbers.Get());
}

bool NumList::Add(const std::string& strNumbers)  // if false, means the
                                                  // numbers were already
                                                  // there. (At least one of
                                                  // them.)
{
    return Add(strNumbers.c_str());
}

// This function is private, so you can't use it without passing an OTString.
// (For security reasons.) It takes a comma-separated list of numbers, and adds
// them to *this.
//
bool NumList::Add(const char* szNumbers)  // if false, means the numbers were
                                          // already there. (At least one of
                                          // them.)
{
    OT_ASSERT(nullptr != szNumbers);  // Should never happen.

    bool bSuccess = true;
    int64_t lNum = 0;
    const char* pChar = szNumbers;
    std::locale loc;

    // Skip any whitespace.
    while (std::isspace(*pChar, loc)) pChar++;

    bool bStartedANumber =
        false;  // During the loop, set this to true when processing a digit,
                // and set to false when anything else. That way when we go to
                // add the number to the list, and it's "0", we'll know it's a
                // real number we're supposed to add, and not just a default
                // value.

    for (;;)  // We already know it's not null, due to the assert. (So at least
              // one iteration will happen.)
    {
        if (std::isdigit(*pChar, loc)) {
            bStartedANumber = true;

            int32_t nDigit = (*pChar - '0');

            lNum *= 10;  // Move it up a decimal place.
            lNum += nDigit;
        }
        // if separator, or end of string, either way, add lNum to *this.
        else if (
            (',' == *pChar) || ('\0' == *pChar) ||
            std::isspace(*pChar, loc))  // first sign of a space, and we are
                                        // done with current number. (On to
                                        // the next.)
        {
            if ((lNum > 0) || (bStartedANumber && (0 == lNum))) {
                if (!Add(lNum))  // <=========
                {
                    bSuccess = false;  // We still go ahead and try to add them
                                       // all, and then return this sort of
                                       // status when it's all done.
                }
            }

            lNum = 0;  // reset for the next transaction number (in the
                       // comma-separated list.)
            bStartedANumber = false;  // reset
        } else {
            otErr << "OTNumList::Add: Error: Unexpected character found in "
                     "erstwhile comma-separated list of longs: "
                  << *pChar << "\n";
            bSuccess = false;
            break;
        }

        // End of the road.
        if ('\0' == *pChar) break;

        pChar++;

        // Skip any whitespace.
        while (std::isspace(*pChar, loc)) pChar++;

    }  // while

    return bSuccess;
}

bool NumList::Add(const int64_t& theValue)  // if false, means the value was
                                            // already there.
{
    auto it = m_setData.find(theValue);

    if (m_setData.end() == it)  // it's not already there, so add it.
    {
        m_setData.insert(theValue);
        return true;
    }
    return false;  // it was already there.
}

bool NumList::Peek(int64_t& lPeek) const
{
    auto it = m_setData.begin();

    if (m_setData.end() != it)  // it's there.
    {
        lPeek = *it;
        return true;
    }
    return false;
}

bool NumList::Pop()
{
    auto it = m_setData.begin();

    if (m_setData.end() != it)  // it's there.
    {
        m_setData.erase(it);
        return true;
    }
    return false;
}

bool NumList::Remove(const int64_t& theValue)  // if false, means the value was
                                               // NOT already there.
{
    auto it = m_setData.find(theValue);

    if (m_setData.end() != it)  // it's there.
    {
        m_setData.erase(it);
        return true;
    }
    return false;  // it wasn't there (so how could you remove it then?)
}

bool NumList::Verify(const int64_t& theValue) const  // returns true/false
                                                     // (whether value is
                                                     // already there.)
{
    auto it = m_setData.find(theValue);

    return (m_setData.end() == it) ? false : true;
}

// True/False, based on whether values are already there.
// (ALL theNumbersmust be present.)
// So if *this contains "3,4,5,6" and rhs contains "4,5" then match is TRUE.
//
bool NumList::Verify(const std::set<int64_t>& theNumbers) const
{
    bool bSuccess = true;

    for (const auto& it : theNumbers) {
        if (!Verify(it))  // It must have NOT already been there.
            bSuccess = false;
    }

    return bSuccess;
}

/// True/False, based on whether OTNumLists MATCH in COUNT and CONTENT (NOT
/// ORDER.)
///
bool NumList::Verify(const NumList& rhs) const
{
    // Verify they have the same number of elements.
    //
    if (Count() != rhs.Count()) return false;

    // Verify each value on *this is also found on rhs.
    //
    for (auto& it : m_setData) {
        if (!rhs.Verify(it)) return false;
    }

    return true;
}

/// True/False, based on whether ANY of the numbers in rhs are found in *this.
///
bool NumList::VerifyAny(const NumList& rhs) const
{
    return rhs.VerifyAny(m_setData);
}

/// Verify whether ANY of the numbers on *this are found in setData.
///
bool NumList::VerifyAny(const std::set<int64_t>& setData) const
{
    for (const auto& it : m_setData) {
        auto it_find = setData.find(it);

        if (it_find != setData.end())  // found a match.
            return true;
    }

    return false;
}

bool NumList::Add(const NumList& theNumList)  // if false, means the numbers
                                              // were already there. (At
                                              // least one of them.)
{
    std::set<int64_t> theOutput;
    theNumList.Output(theOutput);  // returns false if the numlist was empty.

    return Add(theOutput);
}

bool NumList::Add(const std::set<int64_t>& theNumbers)  // if false, means the
                                                        // numbers were already
                                                        // there. (At least one
                                                        // of them.)
{
    bool bSuccess = true;

    for (const auto& it : theNumbers) {
        if (!Add(it))  // It must have already been there.
            bSuccess = false;
    }

    return bSuccess;
}

bool NumList::Remove(const std::set<int64_t>& theNumbers)  // if false, means
                                                           // the numbers were
                                                           // NOT already
                                                           // there. (At least
                                                           // one of them.)
{
    bool bSuccess = true;

    for (const auto& it : theNumbers) {
        if (!Remove(it))  // It must have NOT already been there.
            bSuccess = false;
    }

    return bSuccess;
}

// Outputs the numlist as a set of numbers.
// (To iterate OTNumList, call this, then iterate the output.)
//
bool NumList::Output(std::set<int64_t>& theOutput) const  // returns false if
                                                          // the numlist was
                                                          // empty.
{
    theOutput = m_setData;

    return !m_setData.empty();
}

// Outputs the numlist as a comma-separated string (for serialization, usually.)
//
bool NumList::Output(String& strOutput) const  // returns false if the
                                               // numlist was empty.
{
    int32_t nIterationCount = 0;

    for (auto& it : m_setData) {
        nIterationCount++;

        strOutput.Concatenate(
            "%s%" PRId64,
            // If first iteration, prepend a blank string (instead of a comma.)
            // Like this:  "%" PRId64 ""
            // But for all subsequent iterations, concatenate: ",%" PRId64 ""
            (1 == nIterationCount) ? "" : ",",
            it);
    }

    return !m_setData.empty();
}

int32_t NumList::Count() const
{
    return static_cast<int32_t>(m_setData.size());
}

void NumList::Release() { m_setData.clear(); }

}  // namespace opentxs
