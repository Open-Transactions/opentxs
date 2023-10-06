// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/protocol/bitcoin/base/block/Types.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <limits>
#include <optional>

#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Opcodes.hpp"  // IWYU pragma: keep
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"

namespace be = boost::endian;

namespace opentxs::blockchain::protocol::bitcoin::base::block::internal
{
auto DecodeBip34(const ReadView coinbase) noexcept -> block::Height
{
    static constexpr auto null = block::Height{-1};

    if (false == valid(coinbase)) { return null; }

    const auto* i = reinterpret_cast<const std::byte*>(coinbase.data());
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
        assert_true(std::numeric_limits<std::int32_t>::max() >= height);

        auto buf = be::little_int32_buf_t(static_cast<std::int32_t>(height));
        auto out = space(sizeof(buf) + 1u);
        out.at(0) = std::byte{0x4};
        std::memcpy(std::next(out.data()), &buf, sizeof(buf));
        static_assert(sizeof(buf) == 4u);

        return out;
    }
}

auto Opcode(const script::OP opcode) noexcept(false) -> script::Element
{
    return {opcode, {}, {}, {}};
}

auto PushData(const ReadView in) noexcept(false) -> script::Element
{
    const auto size = shorten(in.size());
    using enum script::OP;
    static constexpr auto null = char{0x0};
    using Data = script::Element::Data;

    if (false == valid(in)) {

        return {PUSHDATA1, {}, Data{&null, sizeof(null)}, {}};
    }

    auto output = script::Element{};
    auto& [opcode, invalid, bytes, data] = output;

    if (75_uz >= size) {
        opcode = static_cast<script::OP>(static_cast<std::uint8_t>(size));
    } else if (std::numeric_limits<std::uint8_t>::max() >= size) {
        opcode = PUSHDATA1;
        const auto buf = std::byte{static_cast<std::uint8_t>(size)};
        bytes = Data{{reinterpret_cast<const char*>(&buf), sizeof(buf)}};
    } else if (std::numeric_limits<std::uint16_t>::max() >= size) {
        opcode = PUSHDATA2;
        const auto buf =
            be::little_uint16_buf_t{static_cast<std::uint16_t>(size)};
        bytes = Data{{reinterpret_cast<const char*>(&buf), sizeof(buf)}};
    } else {
        opcode = PUSHDATA4;
        const auto buf =
            be::little_uint32_buf_t{static_cast<std::uint32_t>(size)};
        bytes = Data{{reinterpret_cast<const char*>(&buf), sizeof(buf)}};
    }

    data = Data{in};

    return output;
}
}  // namespace opentxs::blockchain::protocol::bitcoin::base::block::internal
