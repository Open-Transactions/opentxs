// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <string_view>
#include <type_traits>

#include "opentxs/Export.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Armored;
class ByteArray;
struct HexType;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct OPENTXS_EXPORT hash<opentxs::ByteArray> {
    auto operator()(const opentxs::ByteArray& data) const noexcept
        -> std::size_t;
};
}  // namespace std

namespace opentxs
{
auto swap(ByteArray&, ByteArray&) noexcept -> void;
}  // namespace opentxs

namespace opentxs
{
class OPENTXS_EXPORT ByteArray final : virtual public Data,
                                       virtual public Allocated
{
public:
    class Imp;

    auto asHex() const -> UnallocatedCString final;
    auto asHex(alloc::Default alloc) const -> CString final;
    auto at(const std::size_t position) const -> const std::byte& final;
    auto begin() const -> const_iterator final;
    auto Bytes() const noexcept -> ReadView final;
    auto cbegin() const -> const_iterator final;
    auto cend() const -> const_iterator final;
    auto data() const -> const void* final;
    auto empty() const -> bool final;
    auto end() const -> const_iterator final;
    auto Extract(
        const std::size_t amount,
        Data& output,
        const std::size_t pos = 0) const -> bool final;
    auto Extract(std::uint8_t& output, const std::size_t pos = 0) const
        -> bool final;
    auto Extract(std::uint16_t& output, const std::size_t pos = 0) const
        -> bool final;
    auto Extract(std::uint32_t& output, const std::size_t pos = 0) const
        -> bool final;
    auto Extract(std::uint64_t& output, const std::size_t pos = 0) const
        -> bool final;
    auto get_allocator() const noexcept -> allocator_type final;
    auto IsNull() const -> bool final;
    auto size() const -> std::size_t final;

    auto Assign(const Data& source) noexcept -> bool final;
    auto Assign(const ReadView source) noexcept -> bool final;
    auto Assign(const void* data, const std::size_t size) noexcept
        -> bool final;
    auto at(const std::size_t position) -> std::byte& final;
    auto begin() -> iterator final;
    auto clear() noexcept -> void final;
    auto Concatenate(const ReadView) noexcept -> bool final;
    auto Concatenate(const void*, const std::size_t) noexcept -> bool final;
    auto data() -> void* final;
    auto DecodeHex(const ReadView hex) -> bool final;
    auto end() -> iterator final;
    auto operator+=(const Data& rhs) noexcept(false) -> ByteArray& final;
    auto operator+=(const ReadView rhs) noexcept(false) -> ByteArray& final;
    auto operator+=(const std::uint8_t rhs) noexcept(false) -> ByteArray& final;
    auto operator+=(const std::uint16_t rhs) noexcept(false)
        -> ByteArray& final;
    auto operator+=(const std::uint32_t rhs) noexcept(false)
        -> ByteArray& final;
    auto operator+=(const std::uint64_t rhs) noexcept(false)
        -> ByteArray& final;
    auto Randomize(const std::size_t size) -> bool final;
    auto resize(const std::size_t) -> bool final;
    auto SetSize(const std::size_t) -> bool final;
    auto swap(ByteArray& rhs) noexcept -> void;
    auto WriteInto() noexcept -> AllocateOutput final;
    auto zeroMemory() -> void final;

    OPENTXS_NO_EXPORT ByteArray(Imp* imp) noexcept;
    ByteArray(allocator_type alloc = {}) noexcept;
    ByteArray(std::uint8_t in, allocator_type alloc = {}) noexcept;
    /// Bytes are stored in big endian order
    ByteArray(std::uint16_t in, allocator_type alloc = {}) noexcept;
    /// Bytes are stored in big endian order
    ByteArray(std::uint32_t in, allocator_type alloc = {}) noexcept;
    /// Bytes are stored in big endian order
    ByteArray(std::uint64_t in, allocator_type alloc = {}) noexcept;
    ByteArray(const ReadView bytes, allocator_type alloc = {}) noexcept;
    // throws std::invalid_argument if bytes can not be decoded as hex
    ByteArray(
        const HexType&,
        const ReadView bytes,
        allocator_type alloc = {}) noexcept(false);
    ByteArray(const Armored& rhs, allocator_type alloc = {}) noexcept;
    ByteArray(const Data& rhs, allocator_type alloc = {}) noexcept;
    template <
        typename T,
        std::enable_if_t<std::is_trivially_copyable<T>::value, int> = 0>
    ByteArray(
        const T* data,
        std::size_t size,
        allocator_type alloc = {}) noexcept
        : ByteArray(size, data, alloc)
    {
        static_assert(sizeof(T) == sizeof(std::byte));
    }
    ByteArray(const ByteArray& rhs, allocator_type alloc = {}) noexcept;
    ByteArray(ByteArray&& rhs) noexcept;
    ByteArray(ByteArray&& rhs, allocator_type alloc) noexcept;
    auto operator=(const ByteArray& rhs) noexcept -> ByteArray&;
    auto operator=(ByteArray&& rhs) noexcept -> ByteArray&;

    ~ByteArray() final;

private:
    Imp* imp_;

    ByteArray(
        std::size_t size,
        const void* data,
        allocator_type alloc) noexcept;
};
}  // namespace opentxs
