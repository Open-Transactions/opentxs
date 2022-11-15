// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"               // IWYU pragma: associated
#include "core/ByteArrayPrivate.hpp"  // IWYU pragma: associated

extern "C" {
#include <sodium.h>
}

#include <boost/endian/buffers.hpp>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <functional>
#include <iterator>
#include <span>
#include <utility>

#include "internal/core/Core.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs
{
ByteArrayPrivate::ByteArrayPrivate(allocator_type alloc) noexcept
    : parent_(nullptr)
    , data_(alloc)
{
}

ByteArrayPrivate::ByteArrayPrivate(
    const void* data,
    std::size_t size,
    allocator_type alloc) noexcept
    : parent_(nullptr)
    , data_(
          static_cast<const std::byte*>(data),
          static_cast<const std::byte*>(data) + size,
          alloc)
{
}

auto ByteArrayPrivate::asHex() const -> UnallocatedCString
{
    return to_hex(reinterpret_cast<const std::byte*>(data_.data()), size());
}

auto ByteArrayPrivate::asHex(alloc::Default alloc) const -> CString
{
    return to_hex(
        reinterpret_cast<const std::byte*>(data_.data()), size(), alloc);
}

auto ByteArrayPrivate::Assign(const void* data, const std::size_t size) noexcept
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

auto ByteArrayPrivate::begin() -> iterator { return {parent_, 0_uz}; }

auto ByteArrayPrivate::cbegin() const -> const_iterator
{
    return {parent_, 0_uz};
}

auto ByteArrayPrivate::cend() const -> const_iterator
{
    return {parent_, size()};
}

auto ByteArrayPrivate::check_sub(
    const std::size_t pos,
    const std::size_t target) const -> bool
{
    return check_subset(size(), target, pos);
}

auto ByteArrayPrivate::concatenate(const Vector& data) -> void
{
    data_.insert(data_.end(), data.begin(), data.end());
}

auto ByteArrayPrivate::Concatenate(
    const void* data,
    const std::size_t size) noexcept -> bool
{
    if ((size == 0_uz) || (nullptr == data)) { return false; }

    concatenate(ByteArrayPrivate{data, size}.data_);

    return true;
}

auto ByteArrayPrivate::DecodeHex(const std::string_view hex) -> bool
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

auto ByteArrayPrivate::end() -> iterator { return {parent_, size()}; }

auto ByteArrayPrivate::Extract(
    const std::size_t amount,
    opentxs::Data& output,
    const std::size_t pos) const -> bool
{
    if (false == check_sub(pos, amount)) { return false; }

    output.Assign(&data_.at(pos), amount);

    return true;
}

auto ByteArrayPrivate::Extract(std::uint8_t& output, const std::size_t pos)
    const -> bool
{
    if (false == check_sub(pos, sizeof(output))) { return false; }

    output = std::to_integer<std::uint8_t>(data_.at(pos));

    return true;
}

auto ByteArrayPrivate::Extract(std::uint16_t& output, const std::size_t pos)
    const -> bool
{
    if (false == check_sub(pos, sizeof(output))) { return false; }

    auto temp = boost::endian::big_uint16_buf_t();
    std::memcpy(static_cast<void*>(&temp), &data_.at(pos), sizeof(temp));
    output = temp.value();

    return true;
}

auto ByteArrayPrivate::Extract(std::uint32_t& output, const std::size_t pos)
    const -> bool
{
    if (false == check_sub(pos, sizeof(output))) { return false; }

    auto temp = boost::endian::big_uint32_buf_t();
    std::memcpy(static_cast<void*>(&temp), &data_.at(pos), sizeof(temp));
    output = temp.value();

    return true;
}

auto ByteArrayPrivate::Extract(std::uint64_t& output, const std::size_t pos)
    const -> bool
{
    if (false == check_sub(pos, sizeof(output))) { return false; }

    auto temp = boost::endian::big_uint64_buf_t();
    std::memcpy(static_cast<void*>(&temp), &data_.at(pos), sizeof(temp));
    output = temp.value();

    return true;
}

auto ByteArrayPrivate::Initialize() -> void { data_.clear(); }

auto ByteArrayPrivate::IsNull() const -> bool
{
    if (data_.empty()) { return true; }

    for (const auto& byte : data_) {
        static constexpr auto null = std::byte{0x0};

        if (null != byte) { return false; }
    }

    return true;
}

auto ByteArrayPrivate::operator+=(const opentxs::Data& rhs) noexcept -> void
{
    Concatenate(rhs.data(), rhs.size());
}

auto ByteArrayPrivate::operator+=(const ReadView rhs) noexcept -> void
{
    Concatenate(rhs);
}

auto ByteArrayPrivate::operator+=(const std::uint8_t rhs) noexcept -> void
{
    data_.emplace_back(std::byte(rhs));
}

auto ByteArrayPrivate::operator+=(const std::uint16_t rhs) noexcept -> void
{
    const auto input = boost::endian::big_uint16_buf_t(rhs);
    concatenate(ByteArrayPrivate{std::addressof(input), sizeof(input)}.data_);
}

auto ByteArrayPrivate::operator+=(const std::uint32_t rhs) noexcept -> void
{
    const auto input = boost::endian::big_uint32_buf_t(rhs);
    concatenate(ByteArrayPrivate{std::addressof(input), sizeof(input)}.data_);
}

auto ByteArrayPrivate::operator+=(const std::uint64_t rhs) noexcept -> void
{
    const auto input = boost::endian::big_uint64_buf_t(rhs);
    concatenate(ByteArrayPrivate{std::addressof(input), sizeof(input)}.data_);
}

auto ByteArrayPrivate::pop_front() noexcept -> void
{
    if (false == data_.empty()) { data_.erase(data_.begin()); }
}

auto ByteArrayPrivate::Randomize(const std::size_t size) -> bool
{
    SetSize(size);

    if (size == 0_uz) { return false; }

    ::randombytes_buf(data_.data(), size);

    return true;
}

auto ByteArrayPrivate::resize(const std::size_t size) -> bool
{
    data_.resize(size);

    return true;
}

auto ByteArrayPrivate::SetSize(const std::size_t size) -> bool
{
    clear();

    if (size > 0_uz) { data_.assign(size, std::byte{}); }

    return true;
}

auto ByteArrayPrivate::WriteInto() noexcept -> Writer
{
    return {
        [this](auto size) -> WriteBuffer {
            static constexpr auto blank = std::byte{51};
            data_.clear();
            data_.assign(size, blank);

            return std::span<std::byte>{
                static_cast<std::byte*>(data_.data()), data_.size()};
        },
        [this](auto size) -> bool {
            data_.resize(size);

            return true;
        },
        get_allocator()};
}

auto ByteArrayPrivate::zeroMemory() -> void
{
    ::sodium_memzero(data_.data(), data_.size());
}
}  // namespace opentxs
