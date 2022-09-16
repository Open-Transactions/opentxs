// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <string_view>

#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Allocated.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs
{
class ByteArray::Imp : virtual public opentxs::Allocated
{
public:
    // TODO make the type of parent_ variable a template parameter so that
    // Generic::Imp can inherit from this class rather than duplicating it.
    ByteArray* parent_;

    using Vector = opentxs::Vector<std::byte>;

    auto asHex() const -> UnallocatedCString;
    auto asHex(alloc::Default alloc) const -> CString;
    auto at(const std::size_t position) const -> const std::byte&
    {
        return reinterpret_cast<const std::byte&>(data_.at(position));
    }
    auto begin() const -> const_iterator;
    auto Bytes() const noexcept -> ReadView
    {
        return ReadView{
            reinterpret_cast<const char*>(data_.data()), data_.size()};
    }
    auto cbegin() const -> const_iterator;
    auto cend() const -> const_iterator;
    auto empty() const -> bool { return data_.empty(); }
    auto data() const -> const void* { return data_.data(); }
    auto end() const -> const_iterator;
    auto Extract(
        const std::size_t amount,
        opentxs::Data& output,
        const std::size_t pos) const -> bool;
    auto Extract(std::uint8_t& output, const std::size_t pos) const -> bool;
    auto Extract(std::uint16_t& output, const std::size_t pos) const -> bool;
    auto Extract(std::uint32_t& output, const std::size_t pos) const -> bool;
    auto Extract(std::uint64_t& output, const std::size_t pos) const -> bool;
    auto get_allocator() const noexcept -> allocator_type final
    {
        return data_.get_allocator();
    }
    auto IsNull() const -> bool;
    auto size() const -> std::size_t { return data_.size(); }

    auto Assign(const opentxs::Data& source) noexcept -> bool
    {
        return Assign(source.data(), source.size());
    }
    auto Assign(ReadView source) noexcept -> bool
    {
        return Assign(source.data(), source.size());
    }
    auto Assign(const void* data, const std::size_t size) noexcept -> bool;
    auto at(const std::size_t position) -> std::byte&
    {
        return reinterpret_cast<std::byte&>(data_.at(position));
    }
    auto begin() -> iterator;
    auto clear() noexcept -> void { data_.clear(); }
    auto Concatenate(const ReadView data) noexcept -> bool
    {
        return Concatenate(data.data(), data.size());
    }
    auto Concatenate(const void* data, const std::size_t size) noexcept -> bool;
    auto data() -> void* { return data_.data(); }
    auto DecodeHex(const std::string_view hex) -> bool;
    auto end() -> iterator;
    auto operator+=(const opentxs::Data& rhs) noexcept(false) -> ByteArray&;
    auto operator+=(const ReadView rhs) noexcept(false) -> ByteArray&;
    auto operator+=(const std::uint8_t rhs) noexcept(false) -> ByteArray&;
    auto operator+=(const std::uint16_t rhs) noexcept(false) -> ByteArray&;
    auto operator+=(const std::uint32_t rhs) noexcept(false) -> ByteArray&;
    auto operator+=(const std::uint64_t rhs) noexcept(false) -> ByteArray&;
    auto Randomize(const std::size_t size) -> bool;
    auto resize(const std::size_t size) -> bool;
    auto SetSize(const std::size_t size) -> bool;
    auto WriteInto() noexcept -> AllocateOutput;
    auto zeroMemory() -> void;

    Imp() = delete;
    Imp(ByteArray* parent, allocator_type alloc = {}) noexcept;
    Imp(ByteArray* parent,
        const void* data,
        std::size_t size,
        allocator_type alloc = {}) noexcept;
    Imp(const Imp& rhs) = delete;
    Imp(Imp&& rhs) = delete;
    auto operator=(const Imp& rhs) -> Imp& = delete;
    auto operator=(Imp&& rhs) -> Imp& = delete;

    ~Imp() override = default;

protected:
    Vector data_;

    auto Initialize() -> void;

private:
    auto check_sub(const std::size_t pos, const std::size_t target) const
        -> bool;
    auto concatenate(const Vector& data) -> void;
};
}  // namespace opentxs
