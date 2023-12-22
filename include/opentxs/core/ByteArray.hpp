// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>
#include <type_traits>

#include "opentxs/Export.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Armored;
class ByteArray;
class ByteArrayPrivate;
class Writer;
struct HexType;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace std
{
template <>
struct OPENTXS_EXPORT hash<opentxs::ByteArray> {
    using is_transparent = void;
    using is_avalanching = void;

    auto operator()(const opentxs::ByteArray& data) const noexcept
        -> std::size_t;
};
}  // namespace std

namespace opentxs
{
auto OPENTXS_EXPORT swap(ByteArray&, ByteArray&) noexcept -> void;
}  // namespace opentxs

namespace opentxs
{
class OPENTXS_EXPORT ByteArray final : virtual public Data,
                                       virtual public Allocated
{
public:
    auto asHex() const noexcept -> UnallocatedCString final;
    auto asHex(alloc::Default alloc) const noexcept -> CString final;
    auto Bytes() const noexcept -> ReadView final;
    auto data() const noexcept -> const void* final;
    auto empty() const noexcept -> bool final;
    auto Extract(
        const std::size_t amount,
        Data& output,
        const std::size_t pos = 0) const noexcept -> bool final;
    auto Extract(std::uint8_t& output, const std::size_t pos = 0) const noexcept
        -> bool final;
    auto Extract(std::uint16_t& output, const std::size_t pos = 0)
        const noexcept -> bool final;
    auto Extract(std::uint32_t& output, const std::size_t pos = 0)
        const noexcept -> bool final;
    auto Extract(std::uint64_t& output, const std::size_t pos = 0)
        const noexcept -> bool final;
    auto get() const noexcept -> std::span<const std::byte> final;
    auto get_allocator() const noexcept -> allocator_type final;
    auto IsNull() const noexcept -> bool final;
    auto size() const noexcept -> std::size_t final;

    auto Assign(const Data& source) noexcept -> bool final;
    auto Assign(const ReadView source) noexcept -> bool final;
    auto Assign(const void* data, const std::size_t size) noexcept
        -> bool final;
    auto clear() noexcept -> void final;
    auto Concatenate(const ReadView) noexcept -> bool final;
    auto Concatenate(const void*, const std::size_t) noexcept -> bool final;
    auto data() noexcept -> void* final;
    auto DecodeHex(const ReadView hex) noexcept -> bool final;
    auto get() noexcept -> std::span<std::byte> final;
    auto get_deleter() noexcept -> delete_function final;
    auto operator+=(const Data& rhs) noexcept -> ByteArray&;
    auto operator+=(const ReadView rhs) noexcept -> ByteArray&;
    auto operator+=(const std::uint8_t rhs) noexcept -> ByteArray&;
    /// Bytes are stored in big endian order
    auto operator+=(const std::uint16_t rhs) noexcept -> ByteArray&;
    /// Bytes are stored in big endian order
    auto operator+=(const std::uint32_t rhs) noexcept -> ByteArray&;
    /// Bytes are stored in big endian order
    auto operator+=(const std::uint64_t rhs) noexcept -> ByteArray&;
    auto pop_front() noexcept -> void;
    auto Randomize(const std::size_t size) noexcept -> bool final;
    auto resize(const std::size_t) noexcept -> bool final;
    auto swap(ByteArray& rhs) noexcept -> void;
    auto WriteInto() noexcept -> Writer final;

    OPENTXS_NO_EXPORT ByteArray(ByteArrayPrivate* imp) noexcept;
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
        std::enable_if_t<std::is_trivially_copyable_v<T>, int> = 0>
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
    ByteArrayPrivate* imp_;

    ByteArray(
        std::size_t size,
        const void* data,
        allocator_type alloc) noexcept;
};
}  // namespace opentxs
