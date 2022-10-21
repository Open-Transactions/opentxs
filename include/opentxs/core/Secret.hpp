// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>

#include "opentxs/Export.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

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
    auto AssignText(const ReadView source) noexcept -> bool;
    auto at(const std::size_t position) -> std::byte& final;
    auto begin() -> iterator final;
    auto clear() noexcept -> void final;
    auto Concatenate(const ReadView) noexcept -> bool final;
    auto Concatenate(const void*, const std::size_t) noexcept -> bool final;
    auto data() -> void* final;
    auto DecodeHex(const ReadView hex) -> bool final;
    auto end() -> iterator final;
    auto Randomize(const std::size_t size) -> bool final;
    auto resize(const std::size_t) -> bool final;
    auto SetSize(const std::size_t) -> bool final;
    auto swap(Secret& rhs) noexcept -> void;
    auto WriteInto() noexcept -> Writer final;
    auto WriteInto(Mode mode) noexcept -> Writer;
    auto zeroMemory() -> void final;

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
