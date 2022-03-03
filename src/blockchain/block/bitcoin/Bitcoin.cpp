// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "internal/blockchain/block/bitcoin/Bitcoin.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <limits>
#include <stdexcept>

#include "internal/util/LogMacros.hpp"

namespace be = boost::endian;

namespace opentxs::blockchain::block::bitcoin::internal
{
auto DecodeBip34(const ReadView coinbase) noexcept -> block::Height
{
    static constexpr auto null = block::Height{-1};

    if (false == valid(coinbase)) { return null; }

    auto* i = reinterpret_cast<const std::byte*>(coinbase.data());
    const auto size = std::to_integer<std::uint8_t>(*i);
    std::advance(i, 1);
    auto buf = be::little_int64_buf_t{0};

    if ((size + 1u) > coinbase.size()) { return null; }
    if (size > sizeof(buf)) { return null; }

    std::memcpy(reinterpret_cast<std::byte*>(&buf), i, size);

    return buf.value();
}

auto EncodeBip34(block::Height height) noexcept -> Space
{
    if (std::numeric_limits<std::int8_t>::max() >= height) {
        auto buf = be::little_int8_buf_t(static_cast<std::int8_t>(height));
        auto out = space(sizeof(buf) + 1u);
        out.at(0) = std::byte{0x1};
        std::memcpy(std::next(out.data()), &buf, sizeof(buf));
        static_assert(sizeof(buf) == 1u);

        return out;
    } else if (std::numeric_limits<std::int16_t>::max() >= height) {
        auto buf = be::little_int16_buf_t(static_cast<std::int16_t>(height));
        auto out = space(sizeof(buf) + 1u);
        out.at(0) = std::byte{0x2};
        std::memcpy(std::next(out.data()), &buf, sizeof(buf));
        static_assert(sizeof(buf) == 2u);

        return out;
    } else if (8388607 >= height) {
        auto buf = be::little_int32_buf_t(static_cast<std::int32_t>(height));
        auto out = space(sizeof(buf) + 1u);
        out.at(0) = std::byte{0x3};
        std::memcpy(std::next(out.data()), &buf, 3u);
        static_assert(sizeof(buf) == 4u);

        return out;
    } else {
        OT_ASSERT(std::numeric_limits<std::int32_t>::max() >= height);

        auto buf = be::little_int32_buf_t(static_cast<std::int32_t>(height));
        auto out = space(sizeof(buf) + 1u);
        out.at(0) = std::byte{0x4};
        std::memcpy(std::next(out.data()), &buf, sizeof(buf));
        static_assert(sizeof(buf) == 4u);

        return out;
    }
}

auto Opcode(const OP opcode) noexcept(false) -> ScriptElement
{
    return {opcode, {}, {}, {}};
}

auto PushData(const ReadView in) noexcept(false) -> ScriptElement
{
    const auto size = in.size();

#pragma GCC diagnostic push
#pragma GCC diagnostic ignored "-Wtautological-type-limit-compare"
    // std::size_t might be 32 bit
    if (size > std::numeric_limits<std::uint32_t>::max()) {
        throw std::out_of_range("Too many bytes");
    }
#pragma GCC diagnostic pop

    if ((nullptr == in.data()) || (0 == size)) {
        return {OP::PUSHDATA1, {}, Space{std::byte{0x0}}, Space{}};
    }

    auto output = ScriptElement{};
    auto& [opcode, invalid, bytes, data] = output;

    if (75 >= size) {
        opcode = static_cast<OP>(static_cast<std::uint8_t>(size));
    } else if (std::numeric_limits<std::uint8_t>::max() >= size) {
        opcode = OP::PUSHDATA1;
        bytes = Space{std::byte{static_cast<std::uint8_t>(size)}};
    } else if (std::numeric_limits<std::uint16_t>::max() >= size) {
        opcode = OP::PUSHDATA2;
        const auto buf =
            be::little_uint16_buf_t{static_cast<std::uint16_t>(size)};
        bytes = space(sizeof(buf));
        std::memcpy(bytes.value().data(), &buf, sizeof(buf));
    } else {
        opcode = OP::PUSHDATA4;
        const auto buf =
            be::little_uint32_buf_t{static_cast<std::uint32_t>(size)};
        bytes = space(sizeof(buf));
        std::memcpy(bytes.value().data(), &buf, sizeof(buf));
    }

    data = space(size);
    std::memcpy(data.value().data(), in.data(), in.size());

    return output;
}
}  // namespace opentxs::blockchain::block::bitcoin::internal
