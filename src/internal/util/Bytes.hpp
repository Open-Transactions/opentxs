// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <tuple>

#pragma once

#include <array>  // IWYU pragma: keep
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <exception>
#include <span>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "internal/util/P0330.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/util/Bytes.hpp"  // IWYU pragma: keep
#include "opentxs/util/Container.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace network
{
namespace blockchain
{
namespace bitcoin
{
class CompactSize;
}  // namespace bitcoin
}  // namespace blockchain
}  // namespace network
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs
{
using RawData = UnallocatedVector<unsigned char>;

auto check_at_least(
    const std::size_t have,
    const std::size_t required,
    const std::string_view msg) noexcept(false) -> void;
auto check_at_least(
    const ReadView in,
    const std::size_t required,
    const std::string_view msg) noexcept(false) -> void;
auto check_exactly(
    const ReadView in,
    const std::size_t required,
    const std::string_view msg) noexcept(false) -> void;
auto check_finished(const ReadView in) noexcept(false) -> void;
auto check_finished(const WriteBuffer& out) noexcept(false) -> void;
auto check_finished_nonfatal(
    const ReadView in,
    const std::string_view msg) noexcept -> void;
auto check_finished_nonfatal(
    const WriteBuffer& out,
    const std::string_view msg) noexcept -> void;
auto copy(
    const ReadView in,
    WriteBuffer& out,
    const std::string_view msg) noexcept(false) -> void;
auto deserialize(
    ReadView& in,
    Writer&& out,
    std::size_t bytes,
    const std::string_view msg) noexcept(false) -> void;
auto extract_prefix(
    std::string_view& view,
    std::size_t bytes,
    const std::string_view msg) noexcept(false) -> std::string_view;
template <typename Out>
auto deserialize_object(
    ReadView& in,
    Out& out,
    const std::string_view msg) noexcept(false) -> void
{
    const auto bytes = extract_prefix(in, sizeof(out), msg);
    std::memcpy(
        reinterpret_cast<std::byte*>(std::addressof(out)),
        bytes.data(),
        bytes.size());
}
constexpr auto reader(const void* p, std::size_t s) noexcept -> ReadView
{
    return {static_cast<const char*>(p), s};
}
template <std::size_t N>
auto reader(const std::array<std::byte, N>& in) noexcept -> ReadView
{
    return {reinterpret_cast<const char*>(in.data()), N};
}
auto reserve(
    Writer&& out,
    std::size_t bytes,
    const std::string_view msg) noexcept(false) -> WriteBuffer;
auto serialize_compact_size(
    std::size_t value,
    WriteBuffer& out,
    const std::string_view msg) noexcept(false) -> void;
auto serialize_compact_size(
    const network::blockchain::bitcoin::CompactSize& value,
    WriteBuffer& out,
    const std::string_view msg) noexcept(false) -> void;
template <typename In>
auto serialize_object(
    const In& in,
    WriteBuffer& out,
    const std::string_view msg) noexcept(false) -> void
{
    constexpr auto required = sizeof(in);
    check_at_least(out.size(), required, msg);
    std::memcpy(out.data(), std::addressof(in), required);

    if (false == out.RemovePrefix(required)) {

        throw std::runtime_error{"range update error"};
    }
}
template <std::size_t N>
auto writer(std::array<std::byte, N>& in) noexcept -> Writer
{
    return preallocated(N, in.data());
}
}  // namespace opentxs

namespace opentxs
{
using namespace std::literals;

constexpr auto subtract(std::byte lhs, std::byte rhs) noexcept -> std::byte
{
    constexpr auto zero = std::byte{0};

    while (rhs != zero) {
        const auto borrow = (~lhs) & rhs;
        lhs ^= rhs;
        rhs = borrow << 1;
    }

    return lhs;
}

constexpr auto hex_nibble(std::byte in) noexcept -> std::byte
{
    static_assert(sizeof(std::byte) == sizeof(std::uint8_t));
    constexpr auto digitMin = std::byte{48};
    constexpr auto digitMax = std::byte{57};
    constexpr auto upperMin = std::byte{65};
    constexpr auto upperMax = std::byte{70};
    constexpr auto lowerMin = std::byte{97};
    constexpr auto lowerMax = std::byte{102};
    constexpr auto offset = std::byte{10};
    constexpr auto upperOffset = subtract(upperMin, offset);
    constexpr auto lowerOffset = subtract(lowerMin, offset);

    if ((digitMin <= in) && (digitMax >= in)) {

        return subtract(in, digitMin);
    } else if ((upperMin <= in) && (upperMax >= in)) {

        return subtract(in, upperOffset);
    } else if ((lowerMin <= in) && (lowerMax >= in)) {

        return subtract(in, lowerOffset);
    } else {
        std::terminate();
    }
}

constexpr auto hex_nibble(char in) noexcept -> std::byte
{
    return hex_nibble(static_cast<std::byte>(in));
}

constexpr auto decode_hex(std::byte lhs, std::byte rhs) noexcept -> std::byte
{
    return (hex_nibble(lhs) << 4) | hex_nibble(rhs);
}

constexpr auto decode_hex(char lhs, char rhs) noexcept -> std::byte
{
    return decode_hex(static_cast<std::byte>(lhs), static_cast<std::byte>(rhs));
}

constexpr auto remove_hex_prefix(std::string_view& hex)
{
    if (hex.starts_with("0x"sv) || hex.starts_with("0X"sv)) {
        hex.remove_prefix(2);
    }
}

constexpr auto decode_hex(
    std::string_view hex,
    std::span<std::byte> out) noexcept -> bool
{
    using namespace std::literals;

    remove_hex_prefix(hex);

    if (hex.empty()) { return true; }

    const auto count = hex.size();
    const auto reserved = out.size();

    if (count > (reserved * 2_uz)) { return false; }

    const auto offset = count % 2_uz;
    const auto start = reserved - ((count + offset) / 2_uz);

    for (auto i = 0_uz, o = start; i < count; ++o) {
        const auto& first = hex[i];
        auto& value = out[o];

        if ((0_uz == i) && (1_uz == offset)) {
            constexpr auto zero = std::byte{48};
            value = decode_hex(zero, static_cast<std::byte>(first));
            i += 1_uz;
        } else {
            const auto& second = hex[i + 1_uz];
            value = decode_hex(first, second);
            i += 2_uz;
        }
    }

    return true;
}

constexpr auto test_decode_hex() noexcept -> bool
{
    constexpr auto F = std::byte{0x46};
    constexpr auto f = std::byte{0x66};
    constexpr auto ff = std::byte{0xff};
    constexpr auto zero = std::byte{0x00};
    constexpr auto of = std::byte{0x0f};

    static_assert(decode_hex(f, f) == ff);
    static_assert(decode_hex(F, f) == ff);
    static_assert(decode_hex(f, F) == ff);
    static_assert(decode_hex(F, F) == ff);

    {
        constexpr auto lhs = std::byte{255};
        constexpr auto rhs = std::byte{127};
        static_assert(subtract(lhs, rhs) == std::byte{128});
    }
    {
        constexpr auto empty = std::array<std::byte, 4_uz>{};
        constexpr auto full = std::array<std::byte, 4_uz>{ff, ff, ff, ff};
        constexpr auto shorter = std::array<std::byte, 4_uz>{zero, ff, ff, ff};
        constexpr auto partial = std::array<std::byte, 4_uz>{of, ff, ff, ff};
        constexpr auto decode = [](const auto hex) {
            auto buf = std::array<std::byte, 4_uz>{};
            decode_hex(hex, buf);

            return buf;
        };
        static_assert(decode("") == empty);
        static_assert(decode("0x") == empty);
        static_assert(decode("0xffffffff") == full);
        static_assert(decode("0xffffff") == shorter);
        static_assert(decode("0xfffffff") == partial);
    }

    return true;
}
}  // namespace opentxs
