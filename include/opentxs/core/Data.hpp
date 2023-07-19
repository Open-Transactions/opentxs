// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <compare>
#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

#include "opentxs/Export.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Data;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
OPENTXS_EXPORT auto operator==(const Data&, const Data&) noexcept -> bool;
OPENTXS_EXPORT auto operator<=>(const Data&, const Data&) noexcept
    -> std::strong_ordering;
OPENTXS_EXPORT auto to_hex(const std::byte* in, std::size_t size) noexcept
    -> UnallocatedCString;
OPENTXS_EXPORT auto to_hex(
    const std::byte* in,
    std::size_t size,
    alloc::Default alloc) noexcept -> CString;
}  // namespace opentxs

namespace opentxs
{
class OPENTXS_EXPORT Data
{
public:
    virtual auto asHex() const noexcept -> UnallocatedCString = 0;
    virtual auto asHex(alloc::Default alloc) const noexcept -> CString = 0;
    virtual auto Bytes() const noexcept -> ReadView = 0;
    virtual auto data() const noexcept -> const void* = 0;
    virtual auto empty() const noexcept -> bool = 0;
    virtual auto Extract(
        const std::size_t amount,
        Data& output,
        const std::size_t pos = 0) const noexcept -> bool = 0;
    virtual auto Extract(std::uint8_t& output, const std::size_t pos = 0)
        const noexcept -> bool = 0;
    /// Bytes are interpreted as big endian
    virtual auto Extract(std::uint16_t& output, const std::size_t pos = 0)
        const noexcept -> bool = 0;
    /// Bytes are interpreted as big endian
    virtual auto Extract(std::uint32_t& output, const std::size_t pos = 0)
        const noexcept -> bool = 0;
    /// Bytes are interpreted as big endian
    virtual auto Extract(std::uint64_t& output, const std::size_t pos = 0)
        const noexcept -> bool = 0;
    virtual auto get() const noexcept -> std::span<const std::byte> = 0;
    virtual auto IsNull() const noexcept -> bool = 0;
    virtual auto size() const noexcept -> std::size_t = 0;

    virtual auto Assign(const Data& source) noexcept -> bool = 0;
    virtual auto Assign(const ReadView source) noexcept -> bool = 0;
    virtual auto Assign(const void* data, const std::size_t size) noexcept
        -> bool = 0;
    virtual auto clear() noexcept -> void = 0;
    virtual auto Concatenate(const ReadView data) noexcept -> bool = 0;
    virtual auto Concatenate(const void* data, const std::size_t size) noexcept
        -> bool = 0;
    virtual auto data() noexcept -> void* = 0;
    virtual auto get() noexcept -> std::span<std::byte> = 0;
    virtual auto DecodeHex(const std::string_view hex) noexcept -> bool = 0;
    virtual auto Randomize(const std::size_t size) noexcept -> bool = 0;
    virtual auto resize(const std::size_t size) noexcept -> bool = 0;
    virtual auto WriteInto() noexcept -> Writer = 0;

    Data(const Data& rhs) = delete;
    Data(Data&& rhs) = delete;
    auto operator=(const Data& rhs) -> Data& = delete;
    auto operator=(Data&& rhs) -> Data& = delete;

    virtual ~Data() = default;

protected:
    Data() = default;
};
}  // namespace opentxs
