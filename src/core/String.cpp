// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"     // IWYU pragma: associated
#include "1_Internal.hpp"   // IWYU pragma: associated
#include "core/String.hpp"  // IWYU pragma: associated

extern "C" {
#include <sodium.h>
}

#include <cctype>
#include <cstdarg>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <map>
#include <memory_resource>
#include <sstream>
#include <string>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "internal/util/String.hpp"
#include "opentxs/core/Armored.hpp"
#include "opentxs/core/Contract.hpp"
#include "opentxs/core/NymFile.hpp"
#include "opentxs/core/crypto/Signature.hpp"
#include "opentxs/core/identifier/Generic.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Pimpl.hpp"

#define MAX_STRING_LENGTH 0x800000  // this is about 8 megs.

template class opentxs::Pimpl<opentxs::String>;

namespace opentxs
{
auto operator<<(std::ostream& os, const String& obj) -> std::ostream&
{
    os << obj.Get();
    return os;
}

auto String::Factory() -> OTString
{
    return OTString(new implementation::String());
}

auto String::Factory(const Armored& value) -> OTString
{
    return OTString(new implementation::String(value));
}

auto String::Factory(const Signature& value) -> OTString
{
    return OTString(new implementation::String(value));
}

auto String::Factory(const Contract& value) -> OTString
{
    return OTString(new implementation::String(value));
}

auto String::Factory(const Identifier& value) -> OTString
{
    return OTString(new implementation::String(value));
}

auto String::Factory(const NymFile& value) -> OTString
{
    return OTString(new implementation::String(value));
}

auto String::Factory(const char* value) -> OTString
{
    return OTString(new implementation::String(value));
}

auto String::Factory(const std::string& value) -> OTString
{
    return OTString(new implementation::String(value));
}

auto String::Factory(const char* value, std::size_t size) -> OTString
{
    return OTString(new implementation::String(value, size));
}

auto String::LongToString(const std::int64_t& lNumber) -> std::string
{
    std::string strNumber;
    std::stringstream strstream;

    strstream << lNumber;
    strstream >> strNumber;

    return strNumber;
}

auto String::replace_chars(
    const std::string& str,
    const std::string& charsFrom,
    const char& charTo) -> std::string
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

auto String::safe_strlen(const char* s, std::size_t max) -> std::size_t
{
    OT_ASSERT_MSG(
        max <= MAX_STRING_LENGTH,
        "OT_String::safe_strlen: ASSERT: "
        "max length passed in is longer "
        "than allowed.\n");

    return strnlen(s, max);
}

auto String::StringToInt(const std::string& strNumber) -> std::int32_t
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

auto String::StringToLong(const std::string& strNumber) -> std::int64_t
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

auto String::StringToUint(const std::string& strNumber) -> std::uint32_t
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

auto String::StringToUlong(const std::string& strNumber) -> std::uint64_t
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

auto String::trim(std::string& str) -> std::string&
{
    std::string whitespaces(" \t\f\v\n\r");
    std::size_t found = str.find_first_not_of(whitespaces);

    if (found != std::string::npos) { str.erase(0, found); }

    found = str.find_last_not_of(whitespaces);

    if (found != std::string::npos) { str.erase(found + 1); }

    return str;
}

auto String::UlongToString(const std::uint64_t& uNumber) -> std::string
{
    std::string strNumber;
    std::stringstream strstream;

    strstream << uNumber;
    strstream >> strNumber;

    return strNumber;
}
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

auto String::operator=(const String& rhs) -> String&
{
    Release();
    LowLevelSetStr(rhs);

    return *this;
}

auto String::operator>(const opentxs::String& s2) const -> bool
{
    auto& rhs = dynamic_cast<const String&>(s2);

    if (rhs.length_ == 0) { return (true); }
    if (length_ == 0) { return (false); }
    if (strcmp(internal_.data(), rhs.internal_.data()) <= 0) { return (false); }
    return (true);
}

auto String::operator<(const opentxs::String& s2) const -> bool
{
    auto& rhs = dynamic_cast<const String&>(s2);

    if (length_ == 0) { return (true); }
    if (rhs.length_ == 0) { return (false); }
    if (strcmp(internal_.data(), rhs.internal_.data()) >= 0) { return (false); }
    return (true);
}

auto String::operator<=(const opentxs::String& s2) const -> bool
{
    auto& rhs = dynamic_cast<const String&>(s2);

    if (length_ == 0) { return (true); }
    if (rhs.length_ == 0) { return (false); }
    if (strcmp(internal_.data(), rhs.internal_.data()) > 0) { return (false); }
    return (true);
}

auto String::operator>=(const opentxs::String& s2) const -> bool
{
    auto& rhs = dynamic_cast<const String&>(s2);

    if (rhs.length_ == 0) { return (true); }
    if (length_ == 0) { return (false); }
    if (strcmp(internal_.data(), rhs.internal_.data()) < 0) { return (false); }
    return (true);
}

auto String::operator==(const opentxs::String& s2) const -> bool
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

auto String::At(std::uint32_t lIndex, char& c) const -> bool
{
    if (lIndex < length_) {
        c = internal_.data()[lIndex];
        return true;
    } else
        return false;
}

auto String::clone() const -> String* { return new String(*this); }

// Compare is simple.  True if they match, False if they don't match.
auto String::Compare(const char* strCompare) const -> bool
{
    if (internal_.empty() || nullptr == strCompare) { return false; }

    const char* s1 = internal_.data();
    const char* s2 = strCompare;

    for (; *s1 && *s2; s1++, s2++)
        if (*s1 != *s2) return false;

    if (*s1 != *s2) return false;

    return true;
}

auto String::Compare(const opentxs::String& strCompare) const -> bool
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

    const bool bSuccess = opentxs::vformat(fmt, &vl, str_output);

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
auto String::Contains(const char* strCompare) const -> bool
{
    if (internal_.empty() || nullptr == strCompare) { return false; }

    if (strstr(internal_.data(), strCompare)) return true;

    return false;
}

auto String::Contains(const opentxs::String& strCompare) const -> bool
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
auto String::DecodeIfArmored(bool bEscapedIsAllowed) -> bool
{
    if (!Exists()) return false;

    bool bArmoredAndALSOescaped = false;  // "- -----BEGIN OT ARMORED"
    bool bArmoredButNOTescaped = false;   // "-----BEGIN OT ARMORED"

    if (Contains(OT_BEGIN_ARMORED_escaped))  // check this one first...
    {
        bArmoredAndALSOescaped = true;

        if (!bEscapedIsAllowed) {
            LogError()(OT_PRETTY_CLASS())(
                "Armored and escaped value passed in, "
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
            LogError()(OT_PRETTY_CLASS())("Error loading string contents from "
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

auto String::empty(void) const -> bool
{
    return (internal_.empty()) ? true : false;
}

auto String::Exists(void) const -> bool { return !empty(); }

void String::Format(const char* fmt, ...)
{
    va_list vl;
    va_start(vl, fmt);

    std::string str_output;

    const bool bSuccess = opentxs::vformat(fmt, &vl, str_output);

    va_end(vl);

    if (bSuccess) Set(str_output.c_str());
}

auto String::Get() const -> const char*
{
    if (internal_.empty()) {

        return empty_.c_str();
    } else {

        return internal_.data();
    }
}

auto String::GetLength(void) const -> std::uint32_t { return length_; }

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

auto String::make_string(const char* str, std::uint32_t length)
    -> std::pmr::vector<char>
{
    std::pmr::vector<char> output{};

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
auto String::MemSet(const char* pMem, std::uint32_t theSize)
    -> bool  // if theSize is
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
auto String::sgets(char* szBuffer, std::uint32_t nBufSize) -> bool
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

auto String::sgetc(void) -> char
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

auto String::ToInt() const -> std::int32_t
{
    const std::string str_number(Get());

    return StringToInt(str_number);
}

auto String::TokenizeIntoKeyValuePairs(
    std::pmr::map<std::string, std::string>& mapOutput) const -> bool
{
#if __has_include(<wordexp.h>)
    return tokenize_enhanced(mapOutput);
#else
    return tokenize_basic(mapOutput);
#endif
}

auto String::ToLong() const -> std::int64_t
{
    const std::string str_number(Get());

    return StringToLong(str_number);
}

auto String::ToUint() const -> std::uint32_t
{
    const std::string str_number(Get());

    return StringToUint(str_number);
}

auto String::ToUlong() const -> std::uint64_t
{
    const std::string str_number(Get());

    return StringToUlong(str_number);
}

auto String::WriteInto() noexcept -> AllocateOutput
{
    return [this](const auto size) {
        Release();
        auto blank = std::pmr::vector<char>{};
        blank.assign(size, 5);
        blank.push_back('\0');
        Set(blank.data());

        return WritableView{internal_.data(), GetLength()};
    };
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
        ::sodium_memzero(internal_.data(), length_);
    }
}

String::~String() { Release_String(); }
}  // namespace opentxs::implementation
