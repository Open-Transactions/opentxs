// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <cstdarg>
#include <cstdint>
#include <iosfwd>
#include <list>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Pimpl.hpp"

#ifdef __GNUC__
#define ATTR_PRINTF(a, b) __attribute__((format(printf, a, b)))
#else
#define ATTR_PRINTF(a, b)
#endif

#ifdef _MSC_VER
#define PRI_SIZE "Iu"
#else
#define PRI_SIZE "zu"
#endif

namespace opentxs
{
class Armored;
class Contract;
class Identifier;
class NymFile;
class Signature;
class String;

using OTString = Pimpl<String>;
}  // namespace opentxs

namespace opentxs
{
class OPENTXS_EXPORT String
{
public:
    using List = std::pmr::list<std::string>;
    using Map = std::pmr::map<std::string, std::string>;

    static auto Factory() -> opentxs::Pimpl<opentxs::String>;
    static auto Factory(const Armored& value)
        -> opentxs::Pimpl<opentxs::String>;
    static auto Factory(const Signature& value)
        -> opentxs::Pimpl<opentxs::String>;
    static auto Factory(const Contract& value)
        -> opentxs::Pimpl<opentxs::String>;
    static auto Factory(const Identifier& value)
        -> opentxs::Pimpl<opentxs::String>;
    static auto Factory(const NymFile& value)
        -> opentxs::Pimpl<opentxs::String>;
    static auto Factory(const char* value) -> opentxs::Pimpl<opentxs::String>;
    static auto Factory(const std::string& value)
        -> opentxs::Pimpl<opentxs::String>;
    static auto Factory(const char* value, std::size_t size)
        -> opentxs::Pimpl<opentxs::String>;

    static auto LongToString(const std::int64_t& lNumber) -> std::string;
    static auto replace_chars(
        const std::string& str,
        const std::string& charsFrom,
        const char& charTo) -> std::string;
    static auto safe_strlen(const char* s, std::size_t max) -> std::size_t;
    static auto StringToInt(const std::string& number) -> std::int32_t;
    static auto StringToLong(const std::string& number) -> std::int64_t;
    static auto StringToUint(const std::string& number) -> std::uint32_t;
    static auto StringToUlong(const std::string& number) -> std::uint64_t;
    static auto trim(std::string& str) -> std::string&;
    static auto UlongToString(const std::uint64_t& uNumber) -> std::string;

    virtual auto operator>(const String& rhs) const -> bool = 0;
    virtual auto operator<(const String& rhs) const -> bool = 0;
    virtual auto operator<=(const String& rhs) const -> bool = 0;
    virtual auto operator>=(const String& rhs) const -> bool = 0;
    virtual auto operator==(const String& rhs) const -> bool = 0;

    virtual auto At(std::uint32_t index, char& c) const -> bool = 0;
    virtual auto Bytes() const noexcept -> ReadView = 0;
    virtual auto Compare(const char* compare) const -> bool = 0;
    virtual auto Compare(const String& compare) const -> bool = 0;
    virtual auto Contains(const char* compare) const -> bool = 0;
    virtual auto Contains(const String& compare) const -> bool = 0;
    virtual auto empty() const -> bool = 0;
    virtual auto Exists() const -> bool = 0;
    virtual auto Get() const -> const char* = 0;
    virtual auto GetLength() const -> std::uint32_t = 0;
    virtual auto ToInt() const -> std::int32_t = 0;
    virtual auto TokenizeIntoKeyValuePairs(Map& map) const -> bool = 0;
    virtual auto ToLong() const -> std::int64_t = 0;
    virtual auto ToUint() const -> std::uint32_t = 0;
    virtual auto ToUlong() const -> std::uint64_t = 0;
    virtual void WriteToFile(std::ostream& ofs) const = 0;

    virtual void Concatenate(const char* arg, ...) ATTR_PRINTF(2, 3) = 0;
    virtual void Concatenate(const String& data) = 0;
    virtual void ConvertToUpperCase() = 0;
    virtual auto DecodeIfArmored(bool escapedIsAllowed = true) -> bool = 0;
    virtual void Format(const char* fmt, ...) ATTR_PRINTF(2, 3) = 0;
    /** For a straight-across, exact-size copy of bytes. Source not expected to
     * be null-terminated. */
    virtual auto MemSet(const char* mem, std::uint32_t size) -> bool = 0;
    virtual void Release() = 0;
    /** new_string MUST be at least nEnforcedMaxLength in size if
    nEnforcedMaxLength is passed in at all.
    That's because this function forces the null terminator at that length,
    minus 1. For example, if the max is set to 10, then the valid range is 0..9.
    Therefore 9 (10 minus 1) is where the nullptr terminator goes. */
    virtual void Set(const char* data, std::uint32_t enforcedMaxLength = 0) = 0;
    virtual void Set(const String& data) = 0;
    /** true  == there are more lines to read.
    false == this is the last line. Like EOF. */
    virtual auto sgets(char* buffer, std::uint32_t size) -> bool = 0;
    virtual auto sgetc() -> char = 0;
    virtual void swap(String& rhs) = 0;
    virtual void reset() = 0;
    virtual auto WriteInto() noexcept -> AllocateOutput = 0;

    virtual ~String() = default;

protected:
    String() = default;

private:
    friend OTString;
    friend auto operator<<(std::ostream& os, const String& obj)
        -> std::ostream&;

#ifdef _WIN32
public:
#endif
    virtual auto clone() const -> String* = 0;
#ifdef _WIN32
private:
#endif

    String(String&& rhs) = delete;
    auto operator=(const String& rhs) -> String& = delete;
    auto operator=(String&& rhs) -> String& = delete;
};
}  // namespace opentxs
