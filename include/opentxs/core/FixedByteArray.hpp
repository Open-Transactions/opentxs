// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/Version.hpp"  // IWYU pragma: associated

#include <array>
#include <cstddef>
#include <cstdint>

#include "opentxs/core/Data.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs
{
template <std::size_t N>
class FixedByteArray : virtual public Data
{
public:
    static constexpr auto payload_size_ = std::size_t{N};

    auto asHex() const -> UnallocatedCString override;
    auto asHex(alloc::Resource* alloc) const -> CString override;
    auto at(const std::size_t position) const -> const std::byte& final;
    auto begin() const -> const_iterator final;
    auto Bytes() const noexcept -> ReadView final;
    auto cbegin() const -> const_iterator final;
    auto cend() const -> const_iterator final;
    auto data() const -> const void* final;
    auto empty() const -> bool final { return false; }
    auto end() const -> const_iterator final;
    [[nodiscard]] auto Extract(
        const std::size_t amount,
        Data& output,
        const std::size_t pos = 0) const -> bool final;
    [[nodiscard]] auto Extract(std::uint8_t& output, const std::size_t pos = 0)
        const -> bool final;
    [[nodiscard]] auto Extract(std::uint16_t& output, const std::size_t pos = 0)
        const -> bool final;
    [[nodiscard]] auto Extract(std::uint32_t& output, const std::size_t pos = 0)
        const -> bool final;
    [[nodiscard]] auto Extract(std::uint64_t& output, const std::size_t pos = 0)
        const -> bool final;
    auto GetPointer() const -> const void* final;
    auto GetSize() const -> std::size_t final { return N; }
    auto IsEmpty() const -> bool final { return false; }
    auto IsNull() const -> bool final;
    auto operator==(const Data& rhs) const noexcept -> bool final;
    auto operator!=(const Data& rhs) const noexcept -> bool final;
    auto operator<(const Data& rhs) const noexcept -> bool final;
    auto operator>(const Data& rhs) const noexcept -> bool final;
    auto operator<=(const Data& rhs) const noexcept -> bool final;
    auto operator>=(const Data& rhs) const noexcept -> bool final;
    auto size() const -> std::size_t final { return N; }
    auto str() const -> UnallocatedCString override;
    auto str(alloc::Resource* alloc) const -> CString override;

    [[nodiscard]] auto Assign(const Data& source) noexcept -> bool final;
    [[nodiscard]] auto Assign(const ReadView source) noexcept -> bool final;
    [[nodiscard]] auto Assign(const void* data, const std::size_t size) noexcept
        -> bool final;
    auto at(const std::size_t position) -> std::byte& final;
    auto begin() -> iterator final;
    auto clear() noexcept -> void final;
    [[nodiscard]] auto Concatenate(const ReadView) noexcept -> bool final
    {
        return false;
    }
    [[nodiscard]] auto Concatenate(const void*, const std::size_t) noexcept
        -> bool final
    {
        return false;
    }
    auto data() -> void* final;
    [[nodiscard]] auto DecodeHex(const ReadView hex) -> bool final;
    auto end() -> iterator final;
    [[nodiscard]] auto operator+=(const Data& rhs) noexcept(false)
        -> FixedByteArray& final;
    [[nodiscard]] auto operator+=(const ReadView rhs) noexcept(false)
        -> FixedByteArray& final;
    [[nodiscard]] auto operator+=(const std::uint8_t rhs) noexcept(false)
        -> FixedByteArray& final;
    [[nodiscard]] auto operator+=(const std::uint16_t rhs) noexcept(false)
        -> FixedByteArray& final;
    [[nodiscard]] auto operator+=(const std::uint32_t rhs) noexcept(false)
        -> FixedByteArray& final;
    [[nodiscard]] auto operator+=(const std::uint64_t rhs) noexcept(false)
        -> FixedByteArray& final;
    [[nodiscard]] auto Randomize(const std::size_t size) -> bool final;
    [[nodiscard]] auto resize(const std::size_t) -> bool final { return false; }
    [[nodiscard]] auto SetSize(const std::size_t) -> bool final
    {
        return false;
    }
    auto WriteInto() noexcept -> AllocateOutput final;
    auto zeroMemory() -> void final;

    FixedByteArray() noexcept;
    /// Throws std::out_of_range if input size is incorrect
    FixedByteArray(const ReadView bytes) noexcept(false);
    FixedByteArray(const FixedByteArray& rhs) noexcept;
    auto operator=(const FixedByteArray& rhs) noexcept -> FixedByteArray&;

    ~FixedByteArray() override;

private:
    std::array<std::byte, N> data_;

    auto clone() const -> Data* override;
};

// NOTE sorry Windows users, MSVC throws an ICE if we export this symbol
extern template class OPENTXS_EXPORT_TEMPLATE FixedByteArray<32>;
}  // namespace opentxs
