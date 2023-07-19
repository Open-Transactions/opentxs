// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <string_view>

#include "internal/util/PMR.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
class ByteArrayPrivate : virtual public opentxs::Allocated
{
public:
    using Vector = opentxs::Vector<std::byte>;

    auto asHex() const noexcept -> UnallocatedCString;
    auto asHex(alloc::Default alloc) const noexcept -> CString;
    auto Bytes() const noexcept -> ReadView
    {
        return ReadView{reinterpret_cast<const char*>(data_.data()), size()};
    }
    auto clone(allocator_type alloc) const noexcept -> ByteArrayPrivate*
    {
        return pmr::clone(this, {alloc});
    }
    auto empty() const noexcept -> bool { return data_.empty(); }
    auto data() const noexcept -> const void* { return data_.data(); }
    auto Extract(
        const std::size_t amount,
        opentxs::Data& output,
        const std::size_t pos) const noexcept -> bool;
    auto Extract(std::uint8_t& output, const std::size_t pos) const noexcept
        -> bool;
    auto Extract(std::uint16_t& output, const std::size_t pos) const noexcept
        -> bool;
    auto Extract(std::uint32_t& output, const std::size_t pos) const noexcept
        -> bool;
    auto Extract(std::uint64_t& output, const std::size_t pos) const noexcept
        -> bool;
    auto get() const noexcept -> std::span<const std::byte> { return data_; }
    auto get_allocator() const noexcept -> allocator_type final
    {
        return data_.get_allocator();
    }
    auto IsNull() const noexcept -> bool;
    virtual auto size() const noexcept -> std::size_t { return data_.size(); }

    auto Assign(const opentxs::Data& source) noexcept -> bool
    {
        return Assign(source.data(), source.size());
    }
    auto Assign(ReadView source) noexcept -> bool
    {
        return Assign(source.data(), source.size());
    }
    virtual auto Assign(const void* data, const std::size_t size) noexcept
        -> bool;
    auto clear() noexcept -> void;
    auto Concatenate(const ReadView data) noexcept -> bool
    {
        return Concatenate(data.data(), data.size());
    }
    auto Concatenate(const void* data, const std::size_t size) noexcept -> bool;
    auto data() noexcept -> void* { return data_.data(); }
    auto DecodeHex(const std::string_view hex) noexcept -> bool;
    auto get() noexcept -> std::span<std::byte> { return data_; }
    auto get_deleter() noexcept -> delete_function override
    {
        return pmr::make_deleter(this);
    }
    auto operator+=(const opentxs::Data& rhs) noexcept -> void;
    auto operator+=(const ReadView rhs) noexcept -> void;
    auto operator+=(const std::uint8_t rhs) noexcept -> void;
    auto operator+=(const std::uint16_t rhs) noexcept -> void;
    auto operator+=(const std::uint32_t rhs) noexcept -> void;
    auto operator+=(const std::uint64_t rhs) noexcept -> void;
    auto pop_front() noexcept -> void;
    auto Randomize(const std::size_t size) noexcept -> bool;
    auto resize(const std::size_t size) noexcept -> bool;
    virtual auto WriteInto() noexcept -> Writer;

    ByteArrayPrivate() = delete;
    ByteArrayPrivate(allocator_type alloc = {}) noexcept;
    ByteArrayPrivate(
        const void* data,
        std::size_t size,
        allocator_type alloc = {}) noexcept;
    ByteArrayPrivate(
        const ByteArrayPrivate& rhs,
        allocator_type alloc) noexcept;
    ByteArrayPrivate(const ByteArrayPrivate&) = delete;
    ByteArrayPrivate(ByteArrayPrivate&& rhs) = delete;
    auto operator=(const ByteArrayPrivate& rhs) noexcept
        -> ByteArrayPrivate& = delete;
    auto operator=(ByteArrayPrivate&& rhs) noexcept
        -> ByteArrayPrivate& = delete;

    ~ByteArrayPrivate() override = default;

protected:
    Vector data_;

    auto Initialize() noexcept -> void;

private:
    auto check_sub(const std::size_t pos, const std::size_t target)
        const noexcept -> bool;
    auto concatenate(const Vector& data) noexcept -> void;
};
}  // namespace opentxs
