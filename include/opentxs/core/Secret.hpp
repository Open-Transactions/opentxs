// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>

#include "opentxs/Export.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class SecretPrivate;
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
class OPENTXS_EXPORT Secret final : virtual public Data,
                                    virtual public Allocated
{
public:
    enum class Mode : bool { Mem = true, Text = false };

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
    auto AssignText(const ReadView source) noexcept -> bool;
    auto clear() noexcept -> void final;
    auto Concatenate(const ReadView) noexcept -> bool final;
    auto Concatenate(const void*, const std::size_t) noexcept -> bool final;
    auto data() noexcept -> void* final;
    auto DecodeHex(const ReadView hex) noexcept -> bool final;
    auto get() noexcept -> std::span<std::byte> final;
    auto get_deleter() noexcept -> delete_function final;
    auto Randomize(const std::size_t size) noexcept -> bool final;
    auto resize(const std::size_t) noexcept -> bool final;
    auto swap(Secret& rhs) noexcept -> void;
    auto WriteInto() noexcept -> Writer final;
    auto WriteInto(Mode mode) noexcept -> Writer;

    OPENTXS_NO_EXPORT Secret(SecretPrivate* imp) noexcept;
    Secret() = delete;
    Secret(const Secret&) noexcept;
    Secret(Secret&& rhs) noexcept;
    auto operator=(const Secret&) noexcept -> Secret&;
    auto operator=(Secret&&) noexcept -> Secret&;

    ~Secret() final;

private:
    SecretPrivate* imp_;
};
}  // namespace opentxs
