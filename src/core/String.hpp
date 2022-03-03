// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <iosfwd>

#include "opentxs/core/String.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
class Armored;
class Contract;
class Identifier;
class NymFile;
class Signature;
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::implementation
{
class String : virtual public opentxs::String
{
public:
    auto operator>(const opentxs::String& rhs) const -> bool override;
    auto operator<(const opentxs::String& rhs) const -> bool override;
    auto operator<=(const opentxs::String& rhs) const -> bool override;
    auto operator>=(const opentxs::String& rhs) const -> bool override;
    auto operator==(const opentxs::String& rhs) const -> bool override;

    auto At(std::uint32_t index, char& c) const -> bool override;
    auto Bytes() const noexcept -> ReadView final
    {
        return ReadView{Get(), internal_.size()};
    }
    auto Compare(const char* compare) const -> bool override;
    auto Compare(const opentxs::String& compare) const -> bool override;
    auto Contains(const char* compare) const -> bool override;
    auto Contains(const opentxs::String& compare) const -> bool override;
    auto empty() const -> bool override;
    auto Exists() const -> bool override;
    auto Get() const -> const char* override;
    auto GetLength() const -> std::uint32_t override;
    auto ToInt() const -> std::int32_t override;
    auto TokenizeIntoKeyValuePairs(Map& map) const -> bool override;
    auto ToLong() const -> std::int64_t override;
    auto ToUint() const -> std::uint32_t override;
    auto ToUlong() const -> std::uint64_t override;
    void WriteToFile(std::ostream& ofs) const override;

    void Concatenate(const opentxs::String& data) override;
    void ConvertToUpperCase() override;
    auto DecodeIfArmored(bool escapedIsAllowed = true) -> bool override;
    /** For a straight-across, exact-size copy of bytes. Source not expected to
     * be null-terminated. */
    auto MemSet(const char* mem, std::uint32_t size) -> bool override;
    void Release() override;
    /** new_string MUST be at least nEnforcedMaxLength in size if
    nEnforcedMaxLength is passed in at all.
    That's because this function forces the null terminator at that length,
    minus 1. For example, if the max is set to 10, then the valid range is 0..9.
    Therefore 9 (10 minus 1) is where the nullptr terminator goes. */
    void Set(const char* data, std::uint32_t enforcedMaxLength = 0) override;
    void Set(const opentxs::String& data) override;
    /** true  == there are more lines to read.
    false == this is the last line. Like EOF. */
    auto sgets(char* buffer, std::uint32_t size) -> bool override;
    auto sgetc() -> char override;
    void swap(opentxs::String& rhs) override;
    void reset() override;
    auto WriteInto() noexcept -> AllocateOutput final;

    ~String() override;

protected:
    virtual void Release_String();

    explicit String(const opentxs::Armored& value);
    explicit String(const opentxs::Signature& value);
    explicit String(const opentxs::Contract& value);
    explicit String(const opentxs::Identifier& value);
    explicit String(const opentxs::NymFile& value);
    String(const char* value);
    explicit String(const UnallocatedCString& value);
    String(const char* value, std::size_t size);
    auto operator=(const String& rhs) -> String&;
    String();
    String(const String& rhs);

private:
    friend opentxs::String;

    static const UnallocatedCString empty_;

    std::uint32_t length_{0};
    std::uint32_t position_{0};
    UnallocatedVector<char> internal_{};

    static auto make_string(const char* str, std::uint32_t length)
        -> UnallocatedVector<char>;

    auto clone() const -> String* override;
    auto tokenize_basic(Map& map) const -> bool;
    auto tokenize_enhanced(Map& map) const -> bool;

    /** Only call this right after calling Initialize() or Release(). Also, this
     * function ASSUMES the new_string pointer is good. */
    void LowLevelSet(const char* data, std::uint32_t enforcedMaxLength);
    /** You better have called Initialize() or Release() before you dare call
     * this. */
    void LowLevelSetStr(const String& buffer);
    void Initialize();
    void zeroMemory();
};
}  // namespace opentxs::implementation
