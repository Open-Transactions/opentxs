// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <span>

#include "opentxs/Export.hpp"
#include "opentxs/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
class OPENTXS_EXPORT WriteBuffer
{
public:
    operator std::size_t() const noexcept { return size(); }
    operator ReadView() const noexcept { return {as<char>(), size()}; }

    template <typename DesiredType>
    auto as() const noexcept -> const DesiredType*
    {
        static_assert(sizeof(DesiredType) == sizeof(std::byte));

        return static_cast<const DesiredType*>(data());
    }
    auto data() const noexcept -> const void* { return buf_.data(); }
    auto empty() const noexcept -> bool { return buf_.empty(); }
    auto IsValid(std::size_t target) const noexcept -> bool;
    auto size() const noexcept -> std::size_t { return buf_.size(); }

    operator void*() noexcept { return data(); }
    template <typename DesiredType>
    auto as() noexcept -> DesiredType*
    {
        static_assert(sizeof(DesiredType) == sizeof(std::byte));

        return static_cast<DesiredType*>(data());
    }
    auto data() noexcept -> void* { return buf_.data(); }
    /// Removes the specified number of bytes from the front of the buffer.
    [[nodiscard]] auto RemovePrefix(std::size_t bytes) noexcept -> bool;
    /// Returns a Writer for the specified number of bytes at the beginning of
    /// the buffer and modifies the buffer to exclude that range.
    [[nodiscard]] auto Write(std::size_t bytes) noexcept -> Writer;

    WriteBuffer(std::span<std::byte>&&) noexcept;
    WriteBuffer() noexcept;
    WriteBuffer(const WriteBuffer&) = delete;
    WriteBuffer(WriteBuffer&& rhs) noexcept = default;
    auto operator=(const WriteBuffer&) -> WriteBuffer& = delete;
    auto operator=(WriteBuffer&& rhs) noexcept -> WriteBuffer&;

    ~WriteBuffer() = default;

private:
    std::span<std::byte> buf_;
};
}  // namespace opentxs
