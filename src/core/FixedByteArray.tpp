// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include "opentxs/core/FixedByteArray.hpp"  // IWYU pragma: associated

extern "C" {
#include <sodium.h>
}

#include <boost/endian/buffers.hpp>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <memory>
#include <stdexcept>
#include <string_view>

#include "internal/core/Core.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs
{
template <std::size_t N>
FixedByteArray<N>::FixedByteArray() noexcept
    : data_()
{
    static_assert(0 < N);
}

template <std::size_t N>
FixedByteArray<N>::FixedByteArray(const ReadView bytes) noexcept(false)
    : data_()
{
    if (false == Assign(bytes)) {
        const auto error = CString{"input size "}
                               .append(std::to_string(bytes.size()))
                               .append(" vs expected ")
                               .append(std::to_string(N))
                               .append("\n")
                               .append(PrintStackTrace());

        throw std::out_of_range{error.c_str()};
    }
}

template <std::size_t N>
FixedByteArray<N>::FixedByteArray(const FixedByteArray& rhs) noexcept
    : data_(rhs.data_)
{
}

template <std::size_t N>
auto FixedByteArray<N>::operator=(const FixedByteArray& rhs) noexcept
    -> FixedByteArray&
{
    data_ = rhs.data_;

    return *this;
}

template <std::size_t N>
auto FixedByteArray<N>::asHex() const -> UnallocatedCString
{
    return to_hex(data_.data(), N);
}

template <std::size_t N>
auto FixedByteArray<N>::asHex(alloc::Default alloc) const -> CString
{
    return to_hex(data_.data(), N, alloc);
}

template <std::size_t N>
auto FixedByteArray<N>::Assign(const ReadView rhs) noexcept -> bool
{
    if (const auto size = rhs.size(); N != size) {
        LogError()(OT_PRETTY_CLASS())("wrong input size ")(
            size)(" vs expected ")(N)
            .Flush();

        return false;
    }

    std::memcpy(data_.data(), rhs.data(), N);

    return true;
}

template <std::size_t N>
auto FixedByteArray<N>::Assign(const Data& rhs) noexcept -> bool
{
    return Assign(rhs.Bytes());
}

template <std::size_t N>
auto FixedByteArray<N>::Assign(
    const void* data,
    const std::size_t size) noexcept -> bool
{
    return Assign(ReadView{static_cast<const char*>(data), size});
}

template <std::size_t N>
auto FixedByteArray<N>::at(const std::size_t position) const -> const std::byte&
{
    return data_.at(position);
}

template <std::size_t N>
auto FixedByteArray<N>::at(const std::size_t position) -> std::byte&
{
    return data_.at(position);
}

template <std::size_t N>
auto FixedByteArray<N>::begin() const -> const_iterator
{
    return const_iterator(this, 0);
}

template <std::size_t N>
auto FixedByteArray<N>::begin() -> iterator
{
    return iterator(this, 0);
}

template <std::size_t N>
auto FixedByteArray<N>::Bytes() const noexcept -> ReadView
{
    return ReadView{reinterpret_cast<const char*>(data_.data()), data_.size()};
}

template <std::size_t N>
auto FixedByteArray<N>::cbegin() const -> const_iterator
{
    return const_iterator(this, 0);
}

template <std::size_t N>
auto FixedByteArray<N>::cend() const -> const_iterator
{
    return const_iterator(this, data_.size());
}

template <std::size_t N>
auto FixedByteArray<N>::clear() noexcept -> void
{
    zeroMemory();
}

template <std::size_t N>
auto FixedByteArray<N>::data() const -> const void*
{
    return data_.data();
}

template <std::size_t N>
auto FixedByteArray<N>::data() -> void*
{
    return data_.data();
}

template <std::size_t N>
auto FixedByteArray<N>::DecodeHex(const std::string_view hex) -> bool
{
    const auto prefix = hex.substr(0, 2);
    const auto stripped = (prefix == "0x" || prefix == "0X")
                              ? hex.substr(2, hex.size() - 2)
                              : hex;
    const auto ssize = stripped.size();

    if (0_uz == ssize) { return true; }

    if ((ssize != (2 * N)) && (ssize != ((2 * N) - 1))) {
        LogError()(OT_PRETTY_CLASS())("invalid size for input hex ")(hex.size())
            .Flush();

        return false;
    }

    using namespace std::literals;
    auto padded = std::array<char, 2 * N>{'0'};
    std::memcpy(std::next(padded.data(), ssize % 2), stripped.data(), ssize);
    auto byte = data_.begin();
    auto buf = std::array<char, 3>{'\0'};

    for (std::size_t i = 0; i < padded.size(); i += 2, ++byte) {
        std::memcpy(buf.data(), std::next(padded.data(), i), 2);
        *byte = std::byte(
            static_cast<std::uint8_t>(std::strtol(buf.data(), nullptr, 16)));
    }

    return true;
}

template <std::size_t N>
auto FixedByteArray<N>::end() const -> const_iterator
{
    return const_iterator(this, data_.size());
}

template <std::size_t N>
auto FixedByteArray<N>::end() -> iterator
{
    return iterator(this, data_.size());
}

template <std::size_t N>
auto FixedByteArray<N>::Extract(
    const std::size_t amount,
    opentxs::Data& output,
    const std::size_t pos) const -> bool
{
    if (false == check_subset(N, pos, amount)) { return false; }

    output.Assign(&data_.at(pos), amount);

    return true;
}

template <std::size_t N>
auto FixedByteArray<N>::Extract(std::uint8_t& output, const std::size_t pos)
    const -> bool
{
    if (false == check_subset(N, pos, sizeof(output))) { return false; }

    output = reinterpret_cast<const std::uint8_t&>(data_.at(pos));

    return true;
}

template <std::size_t N>
auto FixedByteArray<N>::Extract(std::uint16_t& output, const std::size_t pos)
    const -> bool
{
    if (false == check_subset(N, pos, sizeof(output))) { return false; }

    auto buf = boost::endian::big_uint16_buf_t();
    std::memcpy(static_cast<void*>(&buf), &data_.at(pos), sizeof(buf));
    output = buf.value();

    return true;
}

template <std::size_t N>
auto FixedByteArray<N>::Extract(std::uint32_t& output, const std::size_t pos)
    const -> bool
{
    if (false == check_subset(N, pos, sizeof(output))) { return false; }

    auto buf = boost::endian::big_uint32_buf_t();
    std::memcpy(static_cast<void*>(&buf), &data_.at(pos), sizeof(buf));
    output = buf.value();

    return true;
}

template <std::size_t N>
auto FixedByteArray<N>::Extract(std::uint64_t& output, const std::size_t pos)
    const -> bool
{
    if (false == check_subset(N, pos, sizeof(output))) { return false; }

    auto buf = boost::endian::big_uint64_buf_t();
    std::memcpy(static_cast<void*>(&buf), &data_.at(pos), sizeof(buf));
    output = buf.value();

    return true;
}

template <std::size_t N>
auto FixedByteArray<N>::IsNull() const -> bool
{
    if (data_.empty()) { return true; }

    for (const auto& byte : data_) {
        static constexpr auto null = std::byte{0};

        if (null != byte) { return false; }
    }

    return true;
}

template <std::size_t N>
auto FixedByteArray<N>::Randomize(const std::size_t size) -> bool
{
    if (N != size) {
        LogError()(OT_PRETTY_CLASS())("wrong input size ")(
            size)(" vs expected ")(N)
            .Flush();

        return false;
    }

    ::randombytes_buf(data_.data(), N);

    return true;
}

template <std::size_t N>
auto FixedByteArray<N>::WriteInto() noexcept -> AllocateOutput
{
    return preallocated(data_.size(), data_.data());
}

template <std::size_t N>
auto FixedByteArray<N>::zeroMemory() -> void
{
    ::sodium_memzero(data_.data(), N);
}

template <std::size_t N>
FixedByteArray<N>::~FixedByteArray() = default;
}  // namespace opentxs
