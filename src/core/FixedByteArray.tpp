// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

extern "C" {
#include <sodium.h>
}

#include <boost/endian/buffers.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <cstring>
#include <iterator>
#include <stdexcept>
#include <string_view>

#include "internal/util/P0330.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Types.internal.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs
{
template <std::size_t N>
FixedByteArray<N>::FixedByteArray(const ReadView bytes) noexcept(false)
    : FixedByteArray()
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
auto FixedByteArray<N>::asHex() const noexcept -> UnallocatedCString
{
    return to_hex(data_.data(), N);
}

template <std::size_t N>
auto FixedByteArray<N>::asHex(alloc::Default alloc) const noexcept -> CString
{
    return to_hex(data_.data(), N, alloc);
}

template <std::size_t N>
auto FixedByteArray<N>::Assign(const ReadView rhs) noexcept -> bool
{
    if (const auto size = rhs.size(); N != size) {
        LogError()()("wrong input size ")(size)(" vs expected ")(N).Flush();

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
auto FixedByteArray<N>::Bytes() const noexcept -> ReadView
{
    return ReadView{reinterpret_cast<const char*>(data_.data()), data_.size()};
}

template <std::size_t N>
auto FixedByteArray<N>::clear() noexcept -> void
{
    ::sodium_memzero(data_.data(), N);
}

template <std::size_t N>
auto FixedByteArray<N>::data() const noexcept -> const void*
{
    return data_.data();
}

template <std::size_t N>
auto FixedByteArray<N>::data() noexcept -> void*
{
    return data_.data();
}

template <std::size_t N>
auto FixedByteArray<N>::DecodeHex(const std::string_view hex) noexcept -> bool
{
    const auto prefix = hex.substr(0, 2);
    const auto stripped = (prefix == "0x" || prefix == "0X")
                              ? hex.substr(2, hex.size() - 2)
                              : hex;
    const auto ssize = stripped.size();

    if (0_uz == ssize) { return true; }

    if ((ssize != (2 * N)) && (ssize != ((2 * N) - 1))) {
        LogError()()("invalid size for input hex ")(hex.size()).Flush();

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
auto FixedByteArray<N>::Extract(
    const std::size_t amount,
    opentxs::Data& output,
    const std::size_t pos) const noexcept -> bool
{
    if (false == check_subset(N, pos, amount)) { return false; }

    output.Assign(&data_.at(pos), amount);

    return true;
}

template <std::size_t N>
auto FixedByteArray<N>::Extract(std::uint8_t& output, const std::size_t pos)
    const noexcept -> bool
{
    if (false == check_subset(N, pos, sizeof(output))) { return false; }

    output = reinterpret_cast<const std::uint8_t&>(data_.at(pos));

    return true;
}

template <std::size_t N>
auto FixedByteArray<N>::Extract(std::uint16_t& output, const std::size_t pos)
    const noexcept -> bool
{
    if (false == check_subset(N, pos, sizeof(output))) { return false; }

    auto buf = boost::endian::big_uint16_buf_t();
    std::memcpy(static_cast<void*>(&buf), &data_.at(pos), sizeof(buf));
    output = buf.value();

    return true;
}

template <std::size_t N>
auto FixedByteArray<N>::Extract(std::uint32_t& output, const std::size_t pos)
    const noexcept -> bool
{
    if (false == check_subset(N, pos, sizeof(output))) { return false; }

    auto buf = boost::endian::big_uint32_buf_t();
    std::memcpy(static_cast<void*>(&buf), &data_.at(pos), sizeof(buf));
    output = buf.value();

    return true;
}

template <std::size_t N>
auto FixedByteArray<N>::Extract(std::uint64_t& output, const std::size_t pos)
    const noexcept -> bool
{
    if (false == check_subset(N, pos, sizeof(output))) { return false; }

    auto buf = boost::endian::big_uint64_buf_t();
    std::memcpy(static_cast<void*>(&buf), &data_.at(pos), sizeof(buf));
    output = buf.value();

    return true;
}

template <std::size_t N>
auto FixedByteArray<N>::IsNull() const noexcept -> bool
{
    if (data_.empty()) { return true; }

    for (const auto& byte : data_) {
        static constexpr auto null = std::byte{0};

        if (null != byte) { return false; }
    }

    return true;
}

template <std::size_t N>
auto FixedByteArray<N>::Randomize(const std::size_t size) noexcept -> bool
{
    if (N != size) {
        LogError()()("wrong input size ")(size)(" vs expected ")(N).Flush();

        return false;
    }

    ::randombytes_buf(data_.data(), N);

    return true;
}

template <std::size_t N>
auto FixedByteArray<N>::WriteInto() noexcept -> Writer
{
    return preallocated(data_.size(), data_.data());
}
}  // namespace opentxs
