// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <string_view>
// IWYU pragma: no_include <typeindex>
// IWYU pragma: no_include "opentxs/util/Writer.hpp"

#pragma once

#include <array>
#include <cstddef>
#include <cstdint>
#include <span>

#include "opentxs/Export.hpp"  // IWYU pragma: keep
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

namespace opentxs
{
template <std::size_t N>
class FixedByteArray : virtual public Data
{
public:
    static constexpr auto payload_size_ = std::size_t{N};

    auto asHex() const noexcept -> UnallocatedCString override;
    auto asHex(alloc::Default alloc) const noexcept -> CString override;
    auto Bytes() const noexcept -> ReadView final;
    auto data() const noexcept -> const void* final;
    auto empty() const noexcept -> bool final { return false; }
    [[nodiscard]] auto Extract(
        const std::size_t amount,
        Data& output,
        const std::size_t pos = 0) const noexcept -> bool final;
    [[nodiscard]] auto Extract(std::uint8_t& output, const std::size_t pos = 0)
        const noexcept -> bool final;
    [[nodiscard]] auto Extract(std::uint16_t& output, const std::size_t pos = 0)
        const noexcept -> bool final;
    [[nodiscard]] auto Extract(std::uint32_t& output, const std::size_t pos = 0)
        const noexcept -> bool final;
    [[nodiscard]] auto Extract(std::uint64_t& output, const std::size_t pos = 0)
        const noexcept -> bool final;
    auto get() const noexcept -> std::span<const std::byte> final;
    auto IsNull() const noexcept -> bool final;
    auto size() const noexcept -> std::size_t final { return N; }

    [[nodiscard]] auto Assign(const Data& source) noexcept -> bool final;
    [[nodiscard]] auto Assign(const ReadView source) noexcept -> bool final;
    [[nodiscard]] auto Assign(const void* data, const std::size_t size) noexcept
        -> bool final;
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
    auto data() noexcept -> void* final;
    [[nodiscard]] auto DecodeHex(const ReadView hex) noexcept -> bool final;
    auto get() noexcept -> std::span<std::byte> final;
    [[nodiscard]] auto Randomize(const std::size_t size) noexcept -> bool final;
    [[nodiscard]] auto resize(const std::size_t) noexcept -> bool final
    {
        return false;
    }
    auto WriteInto() noexcept -> Writer final;

    FixedByteArray() noexcept;
    /// Throws std::out_of_range if input size is incorrect
    FixedByteArray(const ReadView bytes) noexcept(false);
    FixedByteArray(const FixedByteArray& rhs) noexcept;
    auto operator=(const FixedByteArray& rhs) noexcept -> FixedByteArray&;

    ~FixedByteArray() override;

private:
    std::array<std::byte, N> data_;
};

extern template class OPENTXS_IMPORT FixedByteArray<16>;
extern template class OPENTXS_IMPORT FixedByteArray<24>;
extern template class OPENTXS_IMPORT FixedByteArray<32>;
}  // namespace opentxs

namespace std
{
template <>
struct OPENTXS_EXPORT hash<opentxs::FixedByteArray<16>> {
    using is_transparent = void;
    using is_avalanching = void;

    auto operator()(const opentxs::FixedByteArray<16>& data) const noexcept
        -> std::size_t;
};
template <>
struct OPENTXS_EXPORT hash<opentxs::FixedByteArray<24>> {
    using is_transparent = void;
    using is_avalanching = void;

    auto operator()(const opentxs::FixedByteArray<24>& data) const noexcept
        -> std::size_t;
};
template <>
struct OPENTXS_EXPORT hash<opentxs::FixedByteArray<32>> {
    using is_transparent = void;
    using is_avalanching = void;

    auto operator()(const opentxs::FixedByteArray<32>& data) const noexcept
        -> std::size_t;
};
}  // namespace std
