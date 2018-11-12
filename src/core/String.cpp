// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "opentxs/core/String.hpp"

#include "opentxs/core/crypto/OTPassword.hpp"
#include "opentxs/core/crypto/Signature.hpp"
#include "opentxs/core/util/Assert.hpp"
#include "opentxs/core/util/StringUtils.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/Identifier.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/core/Nym.hpp"

#if !(                                                                         \
    defined(_WIN32) || (defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE) ||      \
    defined(ANDROID))
#include <wordexp.h>
#endif

#include <ctype.h>
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include <cstdint>
#include <map>
#include <sstream>
#include <string>
#include <utility>

#include "String.hpp"

#define MAX_STRING_LENGTH 0x800000  // this is about 8 megs.

#define OT_METHOD "opentxs::String"

template class opentxs::Pimpl<opentxs::String>;

namespace opentxs
{
std::ostream& operator<<(std::ostream& os, const String& obj)
{
    os << obj.Get();
    return os;
}

OTString String::Factory() { return OTString(new implementation::String()); }

OTString String::Factory(const Armored& value)
{
    return OTString(new implementation::String(value));
}

OTString String::Factory(const Signature& value)
{
    return OTString(new implementation::String(value));
}

OTString String::Factory(const Contract& value)
{
    return OTString(new implementation::String(value));
}

OTString String::Factory(const Identifier& value)
{
    return OTString(new implementation::String(value));
}

OTString String::Factory(const NymFile& value)
{
    return OTString(new implementation::String(value));
}

OTString String::Factory(const char* value)
{
    return OTString(new implementation::String(value));
}

OTString String::Factory(const std::string& value)
{
    return OTString(new implementation::String(value));
}

OTString String::Factory(const char* value, std::size_t size)
{
    return OTString(new implementation::String(value, size));
}

std::string String::LongToString(const std::int64_t& lNumber)
{
    std::string strNumber;
    std::stringstream strstream;

    strstream << lNumber;
    strstream >> strNumber;

    return strNumber;
}

std::string String::replace_chars(
    const std::string& str,
    const std::string& charsFrom,
    const char& charTo)
{
    std::string l_str(str);
    std::size_t found;

    found = str.find_first_of(charsFrom);
    while (found != std::string::npos) {
        l_str[found] = charTo;
        found = str.find_first_of(charsFrom, found + 1);
    }
    return l_str;
}

#ifdef _WIN32
std::wstring String::s2ws(const std::string& s)
{
    std::int32_t len;
    std::int32_t slength = (int32_t)s.length() + 1;
    len = MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, 0, 0);
    std::wstring r(len, L'\0');
    MultiByteToWideChar(CP_ACP, 0, s.c_str(), slength, &r[0], len);
    return r;
}
#endif

std::size_t String::safe_strlen(const char* s, std::size_t max)
{
    OT_ASSERT_MSG(
        max <= MAX_STRING_LENGTH,
        "OT_String::safe_strlen: ASSERT: "
        "max length passed in is longer "
        "than allowed.\n");

    return strnlen(s, max);
}

std::int32_t String::StringToInt(const std::string& strNumber)
{
    if (strNumber.size() == 0) return 0;

    std::int32_t v = 0;
    std::size_t i = 0;

    char sign = (strNumber[0] == '-' || strNumber[0] == '+')
                    ? (++i, strNumber[0])
                    : '+';

    for (; i < strNumber.size(); ++i) {
        if (strNumber[i] < '0' || strNumber[i] > '9') break;
        v = ((v * 10) + (strNumber[i] - '0'));
    }
    return ((0 == v) ? 0 : ((sign == '-') ? -v : v));
}

std::int64_t String::StringToLong(const std::string& strNumber)
{
    if (strNumber.size() == 0) return 0;

    std::int64_t v = 0;
    std::size_t i = 0;

    char sign = (strNumber[0] == '-' || strNumber[0] == '+')
                    ? (++i, strNumber[0])
                    : '+';

    for (; i < strNumber.size(); ++i) {
        if (strNumber[i] < '0' || strNumber[i] > '9') break;
        v = ((v * 10) + (strNumber[i] - '0'));
    }
    return ((0 == v) ? 0 : ((sign == '-') ? -v : v));
}

std::uint32_t String::StringToUint(const std::string& strNumber)
{
    if (strNumber.size() == 0) return 0;

    std::uint32_t v = 0;
    std::size_t i = 0;

    for (; i < strNumber.size(); ++i) {
        if (strNumber[i] < '0' || strNumber[i] > '9') break;
        v = ((v * 10) + (strNumber[i] - '0'));
    }
    return ((0 == v) ? 0 : v);
}

std::uint64_t String::StringToUlong(const std::string& strNumber)
{
    if (strNumber.size() == 0) return 0;

    std::uint64_t v = 0;
    std::size_t i = 0;

    for (; i < strNumber.size(); ++i) {
        if (strNumber[i] < '0' || strNumber[i] > '9') break;
        v = ((v * 10) + (strNumber[i] - '0'));
    }
    return ((0 == v) ? 0 : v);
}

std::string& String::trim(std::string& str)
{
    std::string whitespaces(" \t\f\v\n\r");
    std::size_t found = str.find_first_not_of(whitespaces);

    if (found != std::string::npos) { str.erase(0, found); }

    found = str.find_last_not_of(whitespaces);

    if (found != std::string::npos) { str.erase(found + 1); }

    return str;
}

std::string String::UlongToString(const std::uint64_t& uNumber)
{
    std::string strNumber;
    std::stringstream strstream;

    strstream << uNumber;
    strstream >> strNumber;

    return strNumber;
}

bool String::vformat(const char* fmt, va_list* pvl, std::string& str_Output)
{
    OT_ASSERT(nullptr != fmt);
    OT_ASSERT(nullptr != pvl);

    std::int32_t size = 0;
    std::int32_t nsize = 0;
    char* buffer = nullptr;
    va_list args;

#ifdef _WIN32
    va_list args_2 = *pvl;  // windows only.

    args = *pvl;
    size = _vscprintf(fmt, args) + 1;
#else
    va_copy(args, *pvl);
    size = 512;
#endif

    buffer = new char[size + 100];
    OT_ASSERT(nullptr != buffer);
    OTPassword::zeroMemory(buffer, size + 100);

#ifdef _WIN32
    nsize = vsnprintf_s(buffer, size, size, fmt, args_2);
#else
    nsize = vsnprintf(buffer, size, fmt, args);
    va_end(args);
#endif

    OT_ASSERT(nsize >= 0);

    // fail -- delete buffer and try again
    // If nsize was 1024 bytes, then that would mean that it printed 1024
    // characters,
    // even though the actual string must be 1025 in length (to have room for
    // the null
    // terminator.)
    // If size, the ACTUAL buffer, was 1024 (that is, if size <= nsize) then
    // size would
    // LACK the necessary space to store the 1025th byte containing the null
    // terminator.
    // Therefore we are forced to delete the buffer and make one that is
    // nsize+1, so that
    // it will be 1025 bytes and thus have the necessary space for the
    // terminator
    //
    if (size <= nsize) {
        size = nsize + 1;
        delete[] buffer;
        buffer = new char[size + 100];
        OT_ASSERT(nullptr != buffer);
        OTPassword::zeroMemory(buffer, size + 100);

#ifdef _WIN32
        nsize = vsnprintf_s(buffer, size, size, fmt, *pvl);
        va_end(args);
        va_end(args_2);
#else
        nsize = vsnprintf(buffer, size, fmt, *pvl);
#endif

        OT_ASSERT(nsize >= 0);
    }
    OT_ASSERT(size > nsize);

    str_Output = buffer;
    delete[] buffer;
    buffer = nullptr;
    return true;
}

#ifdef _WIN32
std::string String::ws2s(const std::wstring& s)
{
    std::int32_t len;
    std::int32_t slength = (int32_t)s.length() + 1;
    len = WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, 0, 0, 0, 0);
    std::string r(len, '\0');
    WideCharToMultiByte(CP_ACP, 0, s.c_str(), slength, &r[0], len, 0, 0);
    return r;
}
#endif
}  // namespace opentxs

namespace opentxs::implementation
{
const std::string String::empty_{""};

String::String()
    : length_(0)
    , position_(0)
    , internal_()
{
}

// This constructor gets the string version of the ID passed in,
// and sets that string on this object. (For when you need a string
// version of an ID.)
String::String(const Identifier& theValue)
    : String()
{
    if (theValue.size() > 0) theValue.GetString(*this);
}

String::String(const Contract& theValue)
    : String()
{
    (const_cast<Contract&>(theValue)).SaveContractRaw(*this);
}

// This version base64-DECODES the ascii-armored string passed in,
// and then sets the decoded plaintext string onto this object.
String::String(const Armored& strValue)
    : String()
{
    if (strValue.Exists()) strValue.GetString(*this);
}

// This version base64-DECODES the ascii-armored signature that's passed in,
// and then sets the decoded plaintext signature onto this object.
// You would only do this when the signature, decoded, is normally in ASII
// form. Actually, that is regularly NOT the case--signatures are usually in
// binary form.
// But Lucre signatures, as used in this library, ARE in text form, so I
// provided this constructor to easily base64-decode them to prepare for
// loading into a bio and then a Lucre object.
String::String(const Signature& strValue)
    : String()
{
    if (strValue.Exists()) strValue.GetString(*this);
}

String::String(const NymFile& value)
    : String()
{
    value.SerializeNymFile(*this);
}

String::String(const char* new_string)
    : String()
{
    LowLevelSet(new_string, 0);
}

String::String(const char* new_string, std::size_t sizeLength)
    : String()
{
    LowLevelSet(new_string, static_cast<std::uint32_t>(sizeLength));
}

String::String(const std::string& new_string)
    : String()
{
    LowLevelSet(
        new_string.c_str(), static_cast<std::uint32_t>(new_string.length()));
}

String::String(const String& strValue)
    : String()
{
    LowLevelSetStr(strValue);
}

String& String::operator=(const String& rhs)
{
    Release();
    LowLevelSetStr(rhs);

    return *this;
}

bool String::operator>(const opentxs::String& s2) const
{
    auto& rhs = dynamic_cast<const String&>(s2);

    if (rhs.length_ == 0) { return (true); }
    if (length_ == 0) { return (false); }
    if (strcmp(internal_.data(), rhs.internal_.data()) <= 0) { return (false); }
    return (true);
}

bool String::operator<(const opentxs::String& s2) const
{
    auto& rhs = dynamic_cast<const String&>(s2);

    if (length_ == 0) { return (true); }
    if (rhs.length_ == 0) { return (false); }
    if (strcmp(internal_.data(), rhs.internal_.data()) >= 0) { return (false); }
    return (true);
}

bool String::operator<=(const opentxs::String& s2) const
{
    auto& rhs = dynamic_cast<const String&>(s2);

    if (length_ == 0) { return (true); }
    if (rhs.length_ == 0) { return (false); }
    if (strcmp(internal_.data(), rhs.internal_.data()) > 0) { return (false); }
    return (true);
}

bool String::operator>=(const opentxs::String& s2) const
{
    auto& rhs = dynamic_cast<const String&>(s2);

    if (rhs.length_ == 0) { return (true); }
    if (length_ == 0) { return (false); }
    if (strcmp(internal_.data(), rhs.internal_.data()) < 0) { return (false); }
    return (true);
}

bool String::operator==(const opentxs::String& s2) const
{
    auto& rhs = dynamic_cast<const String&>(s2);

    // If they are not the same length, return false
    if (length_ != rhs.length_) { return (false); }

    // At this point we know they are at least the same length.
    // Next--are they both 0? If they are both 0, return true
    if (length_ == 0 && rhs.length_ == 0) { return (true); }

    // At this point we have 2 identical-length strings.
    // Now we call strcmp and convert it to true or false.
    if (strcmp(internal_.data(), rhs.internal_.data()) ==
        0) {  // TODO security: use a replacement for
              // strcmp.
        return (true);
    }
    return (false);
}

bool String::At(std::uint32_t lIndex, char& c) const
{
    if (lIndex < length_) {
        c = internal_.data()[lIndex];
        return true;
    } else
        return false;
}

String* String::clone() const { return new String(*this); }

// Compare is simple.  True if they match, False if they don't match.
bool String::Compare(const char* strCompare) const
{
    if (internal_.empty() || nullptr == strCompare) { return false; }

    const char* s1 = internal_.data();
    const char* s2 = strCompare;

    for (; *s1 && *s2; s1++, s2++)
        if (*s1 != *s2) return false;

    if (*s1 != *s2) return false;

    return true;
}

bool String::Compare(const opentxs::String& strCompare) const
{
    if (internal_.empty() || !strCompare.Exists()) { return false; }

    const char* s1 = internal_.data();
    const char* s2 = strCompare.Get();

    for (; *s1 && *s1 != ' '; s1++, s2++)
        if (*s1 != *s2) return false;

    return true;
}

// append a string at the end of the current buffer.
void String::Concatenate(const char* fmt, ...)
{
    va_list vl;
    va_start(vl, fmt);

    std::string str_output;

    const bool bSuccess = String::vformat(fmt, &vl, str_output);

    va_end(vl);

    if (bSuccess) {
        const String strConcat(str_output);

        Concatenate(strConcat);
    }
}

// append a string at the end of the current buffer.
void String::Concatenate(const opentxs::String& strBuf)
{
    std::string str_output;

    if ((length_ > 0) && (false == internal_.empty()))
        str_output += internal_.data();

    if (strBuf.Exists() && (strBuf.GetLength() > 0)) str_output += strBuf.Get();

    Set(str_output.c_str());
}

// Contains is like compare.  True if the substring is there, false if not.
// I was going to return the position but then I realized I never needed it.
// Should be easy to modify if the need arises.
bool String::Contains(const char* strCompare) const
{
    if (internal_.empty() || nullptr == strCompare) { return false; }

    if (strstr(internal_.data(), strCompare)) return true;

    return false;
}

bool String::Contains(const opentxs::String& strCompare) const
{
    if (internal_.empty() || !strCompare.Exists()) { return false; }

    if (strstr(internal_.data(), strCompare.Get())) return true;

    return false;
}

void String::ConvertToUpperCase()
{
    if (internal_.data() == nullptr) { return; }

    for (char* s1 = internal_.data(); *s1; s1++) {
        *s1 = static_cast<char>(toupper(*s1));
    }
}

// If this string starts with -----BEGIN OT ARMORED...
// Then this function will load it up into an Armored (removing
// the bookends) and then decode it back into this string. This code
// has been repeated around so I'm doing this as a refactoring exercise.
//
// Return value: true  == There is a string in here that is not armored.
//                        (Whether I actually HAD to unarmor it or not... it's
//                         unarmored now.)
//               false == There was some error or the string is empty.
//
bool String::DecodeIfArmored(bool bEscapedIsAllowed)
{
    if (!Exists()) return false;

    bool bArmoredAndALSOescaped = false;  // "- -----BEGIN OT ARMORED"
    bool bArmoredButNOTescaped = false;   // "-----BEGIN OT ARMORED"

    if (Contains(OT_BEGIN_ARMORED_escaped))  // check this one first...
    {
        bArmoredAndALSOescaped = true;

        if (!bEscapedIsAllowed) {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Armored and escaped value passed in, "
                "but escaped are forbidden here. "
                "(Returning).")
                .Flush();
            return false;
        }
    } else if (Contains(OT_BEGIN_ARMORED)) {
        bArmoredButNOTescaped = true;
    }

    const bool bArmored = (bArmoredAndALSOescaped || bArmoredButNOTescaped);

    // Whether the string is armored or not, (-----BEGIN OT ARMORED)
    // either way, we'll end up with the decoded version in this variable:
    //
    std::string str_Trim;

    if (bArmored)  // it's armored, we have to decode it first.
    {
        auto ascTemp = Armored::Factory();
        if (false == (ascTemp->LoadFromString(
                         *this,
                         bArmoredAndALSOescaped,  // if it IS escaped or not,
                                                  // this variable will be true
                                                  // or false to show it.
                         // The below szOverride sub-string determines where the
                         // content starts, when loading.
                         OT_BEGIN_ARMORED)))  // Default is:       "-----BEGIN"
        // We're doing this: "-----BEGIN OT ARMORED" (Should worked for
        // escaped as well, here.)
        {
            LogOutput(OT_METHOD)(__FUNCTION__)(
                ": Error loading string contents from "
                "ascii-armored encoding. "
                "Contents: ")(Get())(".")
                .Flush();
            return false;
        } else  // success loading the actual contents out of the ascii-armored
                // version.
        {
            String strTemp(ascTemp);  // <=== ascii-decoded here.
            std::string str_temp(strTemp.Get(), strTemp.GetLength());
            str_Trim = String::trim(str_temp);  // This is the std::string for
                                                // the trim process.
        }
    } else {
        std::string str_temp(Get(), GetLength());
        str_Trim = String::trim(str_temp);  // This is the std::string for the
                                            // trim process. (Wasn't armored,
                                            // so here we use it as passed in.)
    }

    // At this point, str_Trim contains the actual contents, whether they
    // were originally ascii-armored OR NOT. (And they are also now trimmed,
    // either way.)

    Release();

    if (str_Trim.size() > 0) Set(str_Trim.c_str());

    return Exists();
}

bool String::empty(void) const { return (internal_.empty()) ? true : false; }

bool String::Exists(void) const { return !empty(); }

void String::Format(const char* fmt, ...)
{
    va_list vl;
    va_start(vl, fmt);

    std::string str_output;

    const bool bSuccess = String::vformat(fmt, &vl, str_output);

    va_end(vl);

    if (bSuccess) Set(str_output.c_str());
}

const char* String::Get() const
{
    if (internal_.empty()) {

        return empty_.c_str();
    } else {

        return internal_.data();
    }
}

std::uint32_t String::GetLength(void) const { return length_; }

void String::Initialize()
{
    length_ = 0;
    position_ = 0;

    if (false == internal_.empty()) { Release_String(); }
}

// if nEnforcedMaxLength is 10, then it will actually enforce a string at 9
// length. That is, up through index 8 (9th byte) instead of index 9 (10th
// byte.) This is because we are assuming the buffer has no more room than 10
// bytes, and thus index 9 (10th byte) MUST be reserved for the null terminating
// '\0'. Therefore, if the string is actually 10 bytes std::int64_t,
// necessitating an 11th byte for the null terminator, then you should pass 11
// here, aka OTString::GetLength()+1. That way the entire string will fit.
void String::LowLevelSet(
    const char* new_string,
    std::uint32_t nEnforcedMaxLength)
{
    OT_ASSERT(internal_.empty());  // otherwise memory leak.

    if (nullptr != new_string) {
        std::uint32_t nLength =
            (nEnforcedMaxLength > 0)
                ? static_cast<std::uint32_t>(String::safe_strlen(
                      new_string, static_cast<std::size_t>(nEnforcedMaxLength)))
                : static_cast<std::uint32_t>(String::safe_strlen(
                      new_string,
                      static_cast<std::size_t>(
                          MAX_STRING_LENGTH - 1)));  // room
                                                     // for
                                                     // \0

        // don't bother allocating memory for a 0 length string.
        if (0 == nLength) { return; }

        OT_ASSERT_MSG(
            nLength < (MAX_STRING_LENGTH - 10),
            "ASSERT: OTString::LowLevelSet: Exceeded "
            "MAX_STRING_LENGTH! (String would not have fully fit "
            "anyway--it would have been truncated here, potentially "
            "causing data corruption.)");  // 10 being a buffer.

        internal_ = make_string(new_string, nLength);

        if (false == internal_.empty()) {
            length_ = nLength;
        } else {
            length_ = 0;
        }
    }
}

void String::LowLevelSetStr(const String& strBuf)
{
    OT_ASSERT(internal_.empty());  // otherwise memory leak.

    if (strBuf.Exists()) {
        length_ = (MAX_STRING_LENGTH > strBuf.length_)
                      ? strBuf.length_
                      : (MAX_STRING_LENGTH - 1);

        OT_ASSERT_MSG(
            length_ < (MAX_STRING_LENGTH - 10),
            "ASSERT: OTString::LowLevelSetStr: Exceeded "
            "MAX_STRING_LENGTH! (String would not have fully fit "
            "anyway--it would have been truncated here, potentially "
            "causing data corruption.)");  // 10 being a buffer.

        internal_ = make_string(strBuf.internal_.data(), length_);
    }
}

std::vector<char> String::make_string(const char* str, std::uint32_t length)
{
    std::vector<char> output{};

    if ((nullptr != str) && (0 < length)) {
        auto* it = str;

        for (std::size_t i = 0; i < length; ++i, ++it) {
            output.emplace_back(*it);
        }
    }

    // INITIALIZE EXTRA BYTE OF SPACE
    //
    // If length is 10, then buffer is created with 11 elements,
    // indexed from 0 (first element) through 10 (11th element).
    //
    // Therefore str_new[length==10] is the 11th element, which was
    // the extra one created on our buffer, to store the \0 null terminator.
    //
    // This way I know I'm never cutting off data that was in the string itself.
    // Rather, I am only setting to 0 an EXTRA byte that I created myself, AFTER
    // the string's length itself.
    output.emplace_back('\0');

    return output;
}

// The source is probably NOT null-terminated.
// Size must be exact (not a max.)
bool String::MemSet(const char* pMem, std::uint32_t theSize)  // if theSize is
                                                              // 10...
{
    Release();

    if ((nullptr == pMem) || (theSize < 1)) { return true; }

    const auto* it = pMem;

    for (std::size_t i = 0; i < theSize; ++i, ++it) {
        internal_.emplace_back(*it);
    }

    internal_.emplace_back('\0');
    length_ = theSize;  // the length doesn't count the 0.

    return true;
}

void String::Release(void) { Release_String(); }

void String::Release_String(void)
{
    if (false == internal_.empty()) {
        zeroMemory();
        internal_.clear();
    }

    Initialize();
}

void String::reset(void) { position_ = 0; }

// new_string MUST be at least nEnforcedMaxLength in size if nEnforcedMaxLength
// is passed in at all. That's because this function forces the null terminator
// at that length of the string minus 1.
void String::Set(const char* new_string, std::uint32_t nEnforcedMaxLength)
{
    if (new_string == internal_.data())  // Already the same string.
        return;

    Release();

    if (nullptr == new_string) return;

    LowLevelSet(new_string, nEnforcedMaxLength);
}

void String::Set(const opentxs::String& strBuf)
{
    const auto& in = dynamic_cast<const String&>(strBuf);

    if (this == &in)  // Already the same string.
        return;

    Release();

    LowLevelSetStr(in);
}

// true  == there are more lines to read.
// false == this is the last line. Like EOF.
//
bool String::sgets(char* szBuffer, std::uint32_t nBufSize)
{
    if (nullptr == szBuffer) { return false; }

    if (position_ >= length_) return false;

    std::uint32_t lIndex = 0;
    char* pChar = internal_.data() + position_;

    // while *pChar isn't at the end of the source string,
    // and lIndex hasn't reached the end of the destination buffer,
    //
    while (0 != *pChar && (position_ < length_) &&
           lIndex < (nBufSize - 1))  // the -1 leaves room for a forced null
                                     // terminator.
    {
        // If the current character isn't a newline, then copy it...
        if ('\n' != *pChar) {
            szBuffer[lIndex] = *pChar;
            lIndex++;     // increment the buffer
            position_++;  // increment the string's internal memory of where it
                          // stopped.
            pChar++;      // increment this for convenience (could calcuate from
                          // position)
        }
        // Until we reach a newline...
        else {
            // IT'S A NEWLINE!

            szBuffer[lIndex] =
                0;  // destination buffer, this is the end of the line for him.
            position_++;  // This still moves past the newline, so the next
                          // call will get the next
                          // string.
            // lIndex does NOT increment here because we reach the end of this
            // string.
            // neither does pChar. These local variables go away since we are
            // returning.

            if (0 == *(pChar + 1))
                return false;
            else
                return true;  // there was more to read, but we stopped at the
                              // newline.
        }
    }

    // Need to add the nullptr terminator.
    szBuffer[lIndex] = 0;

    // We reached the end of the string.
    // EOF. So we return false to tell the caller not to bother calling again.
    if (0 == *pChar) { return false; }

    // Obviously if *pChar isn't at the end, then there was more to read,
    // but the buffer was full, so we return true.
    return true;
}

char String::sgetc(void)
{
    char answer;

    if (position_ >= length_) { return EOF; }
    answer = *(internal_.data() + position_);

    ++position_;

    return answer;
}

void String::swap(opentxs::String& rhs)
{
    auto& in = dynamic_cast<String&>(rhs);
    std::swap(length_, in.length_);
    std::swap(position_, in.position_);
    std::swap(internal_, in.internal_);
}

std::int32_t String::ToInt() const
{
    const std::string str_number(Get());

    return StringToInt(str_number);
}

bool String::TokenizeIntoKeyValuePairs(
    std::map<std::string, std::string>& mapOutput) const
{
#if !(                                                                         \
    defined(_WIN32) || (defined(TARGET_OS_IPHONE) && TARGET_OS_IPHONE) ||      \
    defined(ANDROID))
    // fabcy-pansy parser that allows for multiple level of quotes nesting and
    // escaped quotes
    if (!Exists()) return true;

    wordexp_t exp_result;

    exp_result.we_wordc = 0;
    exp_result.we_wordv = nullptr;
    exp_result.we_offs = 0;

    if (wordexp(Get(), &exp_result, 0))  // non-zero == failure.
    {
        otErr << "OTString::TokenizeIntoKeyValuePairs: Error calling wordexp() "
                 "(to expand user-defined script args.)\nData: "
              << *this << "\n";
        //        wordfree(&exp_result);
        return false;
    }

    if ((exp_result.we_wordc > 0) && (nullptr != exp_result.we_wordv)) {
        // wordexp tokenizes by space (as well as expands, which is why I'm
        // using it.)
        // Therefore we need to iterate through the tokens, and create a single
        // string
        // with spaces between the tokens.
        //
        for (std::uint32_t i = 0;
             (i < (exp_result.we_wordc - 1)) &&
             (exp_result.we_wordv[i] != nullptr) &&
             (exp_result.we_wordv[i + 1] !=
              nullptr);  // odd man out. Only PAIRS of strings are processed!
             i += 2) {
            const std::string str_key = exp_result.we_wordv[i];
            const std::string str_val = exp_result.we_wordv[i + 1];

            LogVerbose(OT_METHOD)(__FUNCTION__)(": Parsed: ")(str_key)(" = ")(
                str_val)
                .Flush();
            mapOutput.insert(
                std::pair<std::string, std::string>(str_key, str_val));
        }

        wordfree(&exp_result);
    }

    return true;
#else
    // simple parser that allows for one level of quotes nesting but no escaped
    // quotes
    if (!Exists()) return true;

    const char* txt = Get();
    std::string buf = txt;
    for (std::int32_t i = 0; txt[i] != 0;) {
        while (txt[i] == ' ') i++;
        std::int32_t k = i;
        std::int32_t k2 = i;
        if (txt[i] == '\'' || txt[i] == '"') {
            // quoted string
            char quote = txt[i++];
            k = i;
            while (txt[i] != quote && txt[i] != 0) i++;
            if (txt[i] != quote) {
                LogOutput(OT_METHOD)(__FUNCTION__)(": Unmatched quotes in: ")(
                    txt)(".")
                    .Flush();
                return false;
            }
            k2 = i;
            i++;
        } else {
            while (txt[i] != ' ' && txt[i] != 0) i++;
            k2 = i;
        }
        const std::string key = buf.substr(k, k2 - k);

        while (txt[i] == ' ') i++;
        std::int32_t v = i;
        std::int32_t v2 = i;
        if (txt[i] == '\'' || txt[i] == '"') {
            // quoted string
            char quote = txt[i++];
            v = i;
            while (txt[i] != quote && txt[i] != 0) i++;
            if (txt[i] != quote) {
                LogOutput(OT_METHOD)(__FUNCTION__)(": Unmatched quotes in: ")(
                    txt)(".")
                    .Flush();
                return false;
            }
            v2 = i;
            i++;
        } else {
            while (txt[i] != ' ' && txt[i] != 0) i++;
            v2 = i;
        }
        const std::string value = buf.substr(v, v2 - v);

        if (key.length() != 0 && value.length() != 0) {
            LogVerbose(OT_METHOD)(__FUNCTION__)(": Parsed: ")(key)(" = ")(value)
                .Flush();
            mapOutput.insert(std::pair<std::string, std::string>(key, value));
        }
    }
    return true;
#endif
}

std::int64_t String::ToLong() const
{
    const std::string str_number(Get());

    return StringToLong(str_number);
}

std::uint32_t String::ToUint() const
{
    const std::string str_number(Get());

    return StringToUint(str_number);
}

std::uint64_t String::ToUlong() const
{
    const std::string str_number(Get());

    return StringToUlong(str_number);
}

void String::WriteToFile(std::ostream& ofs) const
{
    if (internal_.empty()) { return; }
    char* pchar = const_cast<char*>(internal_.data());

    while (*pchar) {
        if (*pchar != '\r') ofs << *pchar;
        pchar++;
    }
}

void String::zeroMemory()
{
    if (false == internal_.empty()) {
        OTPassword::zeroMemory(internal_.data(), length_);
    }
}

String::~String() { Release_String(); }
}  // namespace opentxs::implementation
