// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdarg>
#include <cstdint>
#include <iosfwd>
#include <utility>

#include "internal/util/Pimpl.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
}  // namespace api

namespace identifier
{
class Generic;
}  // namespace identifier

class Armored;
class Contract;
class NymFile;
class Signature;
class String;
class Writer;

using OTString = Pimpl<String>;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
class String
{
public:
    using List = UnallocatedList<UnallocatedCString>;
    using Map = UnallocatedMap<UnallocatedCString, UnallocatedCString>;

    static auto Factory() -> opentxs::Pimpl<opentxs::String>;
    static auto Factory(const Armored& value)
        -> opentxs::Pimpl<opentxs::String>;
    static auto Factory(const Signature& value)
        -> opentxs::Pimpl<opentxs::String>;
    static auto Factory(const Contract& value)
        -> opentxs::Pimpl<opentxs::String>;
    static auto Factory(
        const identifier::Generic& value,
        const api::Crypto& crypto) -> opentxs::Pimpl<opentxs::String>;
    static auto Factory(const NymFile& value)
        -> opentxs::Pimpl<opentxs::String>;
    static auto Factory(const char* value) -> opentxs::Pimpl<opentxs::String>;
    static auto Factory(const UnallocatedCString& value)
        -> opentxs::Pimpl<opentxs::String>;
    static auto Factory(const char* value, std::size_t size)
        -> opentxs::Pimpl<opentxs::String>;

    static auto LongToString(const std::int64_t& lNumber) -> UnallocatedCString;
    static auto replace_chars(
        const UnallocatedCString& str,
        const UnallocatedCString& charsFrom,
        const char& charTo) -> UnallocatedCString;
    static auto safe_strlen(const char* s, std::size_t max) -> std::size_t;
    static auto StringToInt(const UnallocatedCString& number) -> std::int32_t;
    static auto StringToLong(const UnallocatedCString& number) -> std::int64_t;
    static auto StringToUint(const UnallocatedCString& number) -> std::uint32_t;
    static auto StringToUlong(const UnallocatedCString& number)
        -> std::uint64_t;
    static auto trim(UnallocatedCString& str) -> UnallocatedCString&;
    static auto UlongToString(const std::uint64_t& uNumber)
        -> UnallocatedCString;

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
    virtual auto WriteToFile(std::ostream& ofs) const -> void = 0;

    virtual auto Concatenate(const String& data) -> String& = 0;
    virtual auto Concatenate(std::string_view data) -> String& = 0;
    virtual auto ConvertToUpperCase() -> void = 0;
    virtual auto DecodeIfArmored(
        const api::Crypto& crypto,
        bool escapedIsAllowed = true) -> bool = 0;
    /** For a straight-across, exact-size copy of bytes. Source not expected to
     * be null-terminated. */
    virtual auto MemSet(const char* mem, std::uint32_t size) -> bool = 0;
    virtual auto Release() -> void = 0;
    /** new_string MUST be at least nEnforcedMaxLength in size if
    nEnforcedMaxLength is passed in at all.
    That's because this function forces the null terminator at that length,
    minus 1. For example, if the max is set to 10, then the valid range is 0..9.
    Therefore 9 (10 minus 1) is where the nullptr terminator goes. */
    virtual auto Set(const char* data, std::uint32_t enforcedMaxLength = 0)
        -> void = 0;
    virtual auto Set(const String& data) -> void = 0;
    /** true  == there are more lines to read.
    false == this is the last line. Like EOF. */
    virtual auto sgets(char* buffer, std::uint32_t size) -> bool = 0;
    virtual auto sgetc() -> char = 0;
    virtual auto swap(String& rhs) -> void = 0;
    virtual auto reset() -> void = 0;
    virtual auto WriteInto() noexcept -> Writer = 0;

    String(String&& rhs) = delete;
    auto operator=(const String& rhs) -> String& = delete;
    auto operator=(String&& rhs) -> String& = delete;

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
};
}  // namespace opentxs
