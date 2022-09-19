// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"            // IWYU pragma: associated
#include "1_Internal.hpp"          // IWYU pragma: associated
#include "core/bytearray/Imp.hpp"  // IWYU pragma: associated

extern "C" {
#include <sodium.h>
}

#include <boost/endian/buffers.hpp>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iterator>

#include "internal/core/Core.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/core/Data.hpp"

namespace opentxs
{
ByteArray::Imp::Imp(ByteArray* parent, allocator_type alloc) noexcept
    : parent_(parent)
    , data_(alloc)
{
    OT_ASSERT(nullptr != parent_);
}

ByteArray::Imp::Imp(
    ByteArray* parent,
    const void* data,
    std::size_t size,
    allocator_type alloc) noexcept
    : parent_(parent)
    , data_(
          static_cast<const std::byte*>(data),
          static_cast<const std::byte*>(data) + size,
          alloc)
{
    OT_ASSERT(nullptr != parent_);
}

auto ByteArray::Imp::asHex() const -> UnallocatedCString
{
    return to_hex(
        reinterpret_cast<const std::byte*>(data_.data()), data_.size());
}

auto ByteArray::Imp::asHex(alloc::Default alloc) const -> CString
{
    return to_hex(
        reinterpret_cast<const std::byte*>(data_.data()), data_.size(), alloc);
}

auto ByteArray::Imp::Assign(const void* data, const std::size_t size) noexcept
    -> bool
{
    auto rhs = [&] {
        if ((data == nullptr) || (size == 0_uz)) {

            return Vector{get_allocator()};
        } else {
            const auto* i = static_cast<const std::byte*>(data);

            return Vector{i, std::next(i, size), get_allocator()};
        }
    }();
    data_.swap(rhs);

    return true;
}

auto ByteArray::Imp::begin() -> iterator { return {parent_, 0}; }

auto ByteArray::Imp::cbegin() const -> const_iterator { return {parent_, 0}; }

auto ByteArray::Imp::cend() const -> const_iterator
{
    return {parent_, data_.size()};
}

auto ByteArray::Imp::check_sub(const std::size_t pos, const std::size_t target)
    const -> bool
{
    return check_subset(data_.size(), target, pos);
}

auto ByteArray::Imp::concatenate(const Vector& data) -> void
{
    data_.insert(data_.end(), data.begin(), data.end());
}

auto ByteArray::Imp::Concatenate(
    const void* data,
    const std::size_t size) noexcept -> bool
{
    if ((size == 0_uz) || (nullptr == data)) { return false; }

    auto temp = Imp{parent_, data, size};
    concatenate(temp.data_);

    return true;
}

auto ByteArray::Imp::DecodeHex(const std::string_view hex) -> bool
{
    data_.clear();

    if (hex.empty()) { return true; }

    if (2 > hex.size()) { return false; }

    const auto prefix = hex.substr(0, 2);
    const auto stripped = (prefix == "0x" || prefix == "0X")
                              ? hex.substr(2, hex.size() - 2)
                              : hex;
    using namespace std::literals;
    // TODO c++20 use ranges to prevent unnecessary copy
    const auto padded = (0 == stripped.size() % 2)
                            ? CString{stripped}
                            : CString{"0"sv}.append(stripped);

    for (std::size_t i = 0; i < padded.length(); i += 2) {
        data_.emplace_back(std::byte(static_cast<std::uint8_t>(
            strtol(padded.substr(i, 2).c_str(), nullptr, 16))));
    }

    return true;
}

auto ByteArray::Imp::end() -> iterator { return {parent_, data_.size()}; }

auto ByteArray::Imp::Extract(
    const std::size_t amount,
    opentxs::Data& output,
    const std::size_t pos) const -> bool
{
    if (false == check_sub(pos, amount)) { return false; }

    output.Assign(&data_.at(pos), amount);

    return true;
}

auto ByteArray::Imp::Extract(std::uint8_t& output, const std::size_t pos) const
    -> bool
{
    if (false == check_sub(pos, sizeof(output))) { return false; }

    output = std::to_integer<std::uint8_t>(data_.at(pos));

    return true;
}

auto ByteArray::Imp::Extract(std::uint16_t& output, const std::size_t pos) const
    -> bool
{
    if (false == check_sub(pos, sizeof(output))) { return false; }

    auto temp = boost::endian::big_uint16_buf_t();
    std::memcpy(static_cast<void*>(&temp), &data_.at(pos), sizeof(temp));
    output = temp.value();

    return true;
}

auto ByteArray::Imp::Extract(std::uint32_t& output, const std::size_t pos) const
    -> bool
{
    if (false == check_sub(pos, sizeof(output))) { return false; }

    auto temp = boost::endian::big_uint32_buf_t();
    std::memcpy(static_cast<void*>(&temp), &data_.at(pos), sizeof(temp));
    output = temp.value();

    return true;
}

auto ByteArray::Imp::Extract(std::uint64_t& output, const std::size_t pos) const
    -> bool
{
    if (false == check_sub(pos, sizeof(output))) { return false; }

    auto temp = boost::endian::big_uint64_buf_t();
    std::memcpy(static_cast<void*>(&temp), &data_.at(pos), sizeof(temp));
    output = temp.value();

    return true;
}

auto ByteArray::Imp::Initialize() -> void { data_.clear(); }

auto ByteArray::Imp::IsNull() const -> bool
{
    if (data_.empty()) { return true; }

    for (const auto& byte : data_) {
        static constexpr auto null = std::byte{0x0};

        if (null != byte) { return false; }
    }

    return true;
}

auto ByteArray::Imp::operator+=(const opentxs::Data& rhs) -> ByteArray&
{
    Concatenate(rhs.data(), rhs.size());

    return *parent_;
}

auto ByteArray::Imp::operator+=(const ReadView rhs) -> ByteArray&
{
    Concatenate(rhs);

    return *parent_;
}

auto ByteArray::Imp::operator+=(const std::uint8_t rhs) -> ByteArray&
{
    data_.emplace_back(std::byte(rhs));

    return *parent_;
}

auto ByteArray::Imp::operator+=(const std::uint16_t rhs) -> ByteArray&
{
    const auto input = boost::endian::big_uint16_buf_t(rhs);
    auto temp = Imp{parent_, &input, sizeof(input)};
    concatenate(temp.data_);

    return *parent_;
}

auto ByteArray::Imp::operator+=(const std::uint32_t rhs) -> ByteArray&
{
    const auto input = boost::endian::big_uint32_buf_t(rhs);
    auto temp = Imp{parent_, &input, sizeof(input)};
    concatenate(temp.data_);

    return *parent_;
}

auto ByteArray::Imp::operator+=(const std::uint64_t rhs) -> ByteArray&
{
    const auto input = boost::endian::big_uint64_buf_t(rhs);
    auto temp = Imp{parent_, &input, sizeof(input)};
    concatenate(temp.data_);

    return *parent_;
}

auto ByteArray::Imp::Randomize(const std::size_t size) -> bool
{
    SetSize(size);

    if (size == 0_uz) { return false; }

    ::randombytes_buf(data_.data(), size);

    return true;
}

auto ByteArray::Imp::resize(const std::size_t size) -> bool
{
    data_.resize(size);

    return true;
}

auto ByteArray::Imp::SetSize(const std::size_t size) -> bool
{
    clear();

    if (size > 0) { data_.assign(size, std::byte{}); }

    return true;
}

auto ByteArray::Imp::WriteInto() noexcept -> AllocateOutput
{
    return [this](const auto size) {
        static constexpr auto blank = std::byte{51};
        data_.clear();
        data_.assign(size, blank);

        return WritableView{data_.data(), data_.size()};
    };
}

auto ByteArray::Imp::zeroMemory() -> void
{
    ::sodium_memzero(data_.data(), data_.size());
}
}  // namespace opentxs
