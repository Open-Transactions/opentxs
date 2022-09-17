// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                          // IWYU pragma: associated
#include "1_Internal.hpp"                        // IWYU pragma: associated
#include "internal/blockchain/ethereum/RLP.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <boost/multiprecision/cpp_int.hpp>
#include <cstdint>
#include <cstring>
#include <functional>
#include <iterator>
#include <numeric>
#include <sstream>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "opentxs/api/session/Factory.hpp"
#include "opentxs/api/session/Session.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::ethereum::rlp
{
struct Node::Constants {
    enum class Type {
        null,
        byte,
        short_string,
        long_string,
        short_list,
        long_list,
    };

    static constexpr auto direct_encode_ = std::byte{0x7f};
    static constexpr auto null_ = std::byte{0x80};
    static constexpr auto short_string_ = std::byte{0xb7};
    static constexpr auto long_string_ = std::byte{0xb8};
    static constexpr auto short_list_ = std::byte{0xc0};
    static constexpr auto long_list_ = std::byte{0xf7};
    static constexpr auto long_ = 56_uz;

    static constexpr auto to_int(std::byte in)
    {
        return std::to_integer<std::uint8_t>(in);
    }
};
}  // namespace opentxs::blockchain::ethereum::rlp

namespace opentxs::blockchain::ethereum::rlp
{
struct Node::Calculator {
    auto operator()(const Null& in) const -> std::size_t { return 1; }
    auto operator()(const Sequence& in) const -> std::size_t
    {
        const auto payload = std::accumulate(
            in.begin(),
            in.end(),
            0_uz,
            [this](const auto lhs, const auto& node) {
                return lhs + node.EncodedSize(*this);
            });

        if (0 == payload) {

            return 1;
        } else if (Const::long_ > payload) {

            return 1 + payload;
        } else {

            return 1 + calculate_encoded_size(payload) + payload;
        }
    }
    auto operator()(const String& in) const -> std::size_t
    {
        if (const auto bytes = in.size(); 0 == bytes) {

            return 1;
        } else if (1 == bytes) {

            return calculate_encoded_size(in.at(0));
        } else if (Const::long_ > bytes) {

            return 1 + bytes;
        } else {

            return 1 + calculate_encoded_size(bytes) + bytes;
        }
    }

private:
    using Const = Node::Constants;

    static constexpr auto calculate_encoded_size(std::byte in) noexcept
        -> std::size_t
    {
        if (Const::to_int(in) > Const::to_int(Const::direct_encode_)) {

            return 2;
        } else {

            return 1;
        }
    }
    static constexpr auto calculate_encoded_size(std::uint64_t in) noexcept
        -> std::size_t
    {
        if (((1ull << 8) - 1) >= in) {

            return 1;
        } else if (((1ull << 16) - 1) >= in) {

            return 2;
        } else if (((1ull << 24) - 1) >= in) {

            return 3;
        } else if (((1ull << 32) - 1) >= in) {

            return 4;
        } else if (((1ull << 40) - 1) >= in) {

            return 5;
        } else if (((1ull << 48) - 1) >= in) {

            return 6;
        } else if (((1ull << 56) - 1) >= in) {

            return 7;
        } else {

            return 8;
        }
    }
};
}  // namespace opentxs::blockchain::ethereum::rlp

namespace opentxs::blockchain::ethereum::rlp
{
struct Node::Decoder {
    auto operator()() noexcept(false) -> Node
    {
        if ((0u == remaining_) || (nullptr == in_)) {

            throw std::invalid_argument{"empty input"};
        }

        log_(OT_PRETTY_CLASS())("decoding ")(remaining_)(" bytes").Flush();
        auto out = decode(classify(*in_));

        if ((0u == substring_.size()) && (0u < remaining_)) {
            const auto error = std::to_string(remaining_)
                                   .append(" excess bytes found in input");

            throw std::invalid_argument{error};
        }

        return out;
    }

    Decoder(const api::Session& api, ReadView in) noexcept
        : api_(api)
        , log_(LogInsane())
        , in_(reinterpret_cast<InputIterator>(in.data()))
        , remaining_(in.size())
        , substring_()
    {
    }
    Decoder() = delete;
    Decoder(const Decoder&) = delete;
    Decoder(Decoder&&) = delete;
    auto operator=(const Decoder&) -> Decoder& = delete;
    auto operator=(Decoder&&) -> Decoder& = delete;

private:
    using Const = Node::Constants;
    using InputIterator = const std::byte*;

    static constexpr auto prefix_ = sizeof(std::byte);

    const api::Session& api_;
    const Log& log_;
    InputIterator in_;
    std::size_t remaining_;
    List<std::size_t> substring_;

    auto check_read(std::size_t expected) const noexcept -> bool
    {
        if (expected > remaining_) {
            log_(OT_PRETTY_CLASS())("remaining size of ")(
                remaining_)(" less than ")(expected)
                .Flush();

            return false;
        } else if ((0u < substring_.size()) && (expected > substring_.back())) {
            log_(OT_PRETTY_CLASS())("substring size of ")(
                remaining_)(" less than ")(expected)
                .Flush();

            return false;
        } else {

            return true;
        }
    }
    auto classify(const std::byte& in) const noexcept -> Const::Type
    {
        const auto val = Const::to_int(in);
        log_(OT_PRETTY_CLASS())("classifying byte ")(val).Flush();

        if (val <= Const::to_int(Const::direct_encode_)) {
            log_(OT_PRETTY_CLASS())("found direct encode marker byte").Flush();

            return Const::Type::byte;
        } else if (val == Const::to_int(Const::null_)) {
            log_(OT_PRETTY_CLASS())("found null marker byte").Flush();

            return Const::Type::null;
        } else if (val < Const::to_int(Const::long_string_)) {
            log_(OT_PRETTY_CLASS())("found short string marker byte").Flush();

            return Const::Type::short_string;
        } else if (val < Const::to_int(Const::short_list_)) {
            log_(OT_PRETTY_CLASS())("found long string marker byte").Flush();

            return Const::Type::long_string;
        } else if (val <= Const::to_int(Const::long_list_)) {
            log_(OT_PRETTY_CLASS())("found short list marker byte").Flush();

            return Const::Type::short_list;
        } else {
            log_(OT_PRETTY_CLASS())("found long list marker byte").Flush();

            return Const::Type::long_list;
        }
    }
    auto extract(const std::byte& in, const std::byte& base) const noexcept
        -> std::size_t
    {
        const auto lhs = Const::to_int(in);
        const auto rhs = Const::to_int(base);

        OT_ASSERT(lhs >= rhs);

        auto out = (lhs - rhs);
        log_(OT_PRETTY_CLASS())("marker byte (")(lhs)(") encodes size of ")(out)
            .Flush();

        return out;
    }

    auto decode(Const::Type type) noexcept(false) -> Node
    {
        using Type = Const::Type;

        switch (type) {
            case Type::null: {

                return decode_null();
            }
            case Type::byte: {

                return decode_byte();
            }
            case Type::short_string: {

                return decode_short_string();
            }
            case Type::long_string: {

                return decode_long_string();
            }
            case Type::short_list: {

                return decode_short_list();
            }
            case Type::long_list: {

                return decode_long_list();
            }
            default: {
                OT_FAIL;
            }
        }
    }
    template <typename Buffer>
    auto decode_big_endian() noexcept(false) -> std::uint64_t
    {
        auto buf = Buffer{};
        const auto bytes = sizeof(buf);

        if (false == check_read(bytes)) {
            const auto error =
                UnallocatedCString{
                    "input insufficient for encoded length. Require "}
                    .append(std::to_string(bytes))
                    .append(" bytes but only have ")
                    .append(std::to_string(remaining_))
                    .append(" bytes remaining.");

            throw std::invalid_argument{error};
        }

        std::memcpy(reinterpret_cast<void*>(&buf), in_, bytes);
        finish_read(bytes);

        return buf.value();
    }
    auto decode_byte() noexcept(false) -> Node
    {
        OT_ASSERT(check_read(prefix_));

        auto bytes = ReadView{reinterpret_cast<const char*>(in_), prefix_};
        finish_read(prefix_);
        log_(OT_PRETTY_CLASS())("decoded raw byte").Flush();

        return api_.Factory().DataFromBytes(bytes);
    }
    auto decode_length(std::size_t bytes) noexcept(false) -> std::uint64_t
    {
        namespace be = boost::endian;

        switch (bytes) {
            case 1: {

                return decode_big_endian<be::big_uint8_buf_t>();
            }
            case 2: {

                return decode_big_endian<be::big_uint16_buf_t>();
            }
            case 3: {

                return decode_big_endian<be::big_uint24_buf_t>();
            }
            case 4: {

                return decode_big_endian<be::big_uint32_buf_t>();
            }
            case 5: {

                return decode_big_endian<be::big_uint40_buf_t>();
            }
            case 6: {

                return decode_big_endian<be::big_uint48_buf_t>();
            }
            case 7: {

                return decode_big_endian<be::big_uint56_buf_t>();
            }
            case 8: {

                return decode_big_endian<be::big_uint64_buf_t>();
            }
            default: {
                OT_FAIL;
            }
        }
    }
    auto decode_list(std::size_t length) noexcept(false) -> Node
    {
        const auto level = substring_.size();
        log_(OT_PRETTY_CLASS())("level ")(level)(": decoding list of ")(
            length)(" bytes")
            .Flush();
        auto out = Sequence{};
        auto& substring = substring_.emplace_back(length);
        auto count{0};

        while (0u < substring) {
            const auto index = count++;
            log_(OT_PRETTY_CLASS())("level ")(level)(": decoding item ")(index)
                .Flush();
            out.emplace_back(operator()());
            log_(OT_PRETTY_CLASS())("level ")(level)(": item ")(
                index)(" decoded")
                .Flush();
            log_(OT_PRETTY_CLASS())("level ")(level)(": ")(
                substring)(" bytes remain in list")
                .Flush();
        }

        substring_.pop_back();
        log_(OT_PRETTY_CLASS())("level ")(
            level)(": finished decoding list of ")(length)(" bytes")
            .Flush();

        return out;
    }
    auto decode_long_list() noexcept(false) -> Node
    {
        OT_ASSERT(check_read(prefix_));

        const auto bytes = extract(*in_, Const::long_list_);
        finish_read(prefix_);

        return decode_list(convert_to_size(decode_length(bytes)));
    }
    auto decode_long_string() noexcept(false) -> Node
    {
        OT_ASSERT(check_read(prefix_));

        const auto bytes = extract(*in_, Const::short_string_);
        finish_read(prefix_);

        return decode_string(convert_to_size(decode_length(bytes)));
    }
    auto decode_null() noexcept(false) -> Node
    {
        OT_ASSERT(check_read(prefix_));

        finish_read(prefix_);
        log_(OT_PRETTY_CLASS())("decoded null byte").Flush();

        return Null{};
    }
    auto decode_short_list() noexcept(false) -> Node
    {
        OT_ASSERT(check_read(prefix_));

        const auto length = extract(*in_, Const::short_list_);
        finish_read(prefix_);

        return decode_list(length);
    }
    auto decode_short_string() noexcept(false) -> Node
    {
        OT_ASSERT(check_read(prefix_));

        const auto length = extract(*in_, Const::null_);
        finish_read(prefix_);

        return decode_string(length);
    }
    auto decode_string(std::size_t length) noexcept(false) -> Node
    {
        OT_ASSERT(0u < length);

        if (false == check_read(length)) {
            const auto error =
                UnallocatedCString{
                    "input insufficient for short string. Require "}
                    .append(std::to_string(length))
                    .append(" bytes but only have ")
                    .append(std::to_string(remaining_))
                    .append(" bytes remaining.");

            throw std::invalid_argument{error};
        }

        auto bytes = ReadView{reinterpret_cast<const char*>(in_), length};
        finish_read(length);
        log_(OT_PRETTY_CLASS())("decoded ")(length)(" byte string").Flush();

        return api_.Factory().DataFromBytes(bytes);
    }
    auto finish_read(std::size_t bytes) noexcept -> void
    {
        std::advance(in_, bytes);
        remaining_ -= bytes;

        for (auto& substring : substring_) { substring -= bytes; }
    }
};
}  // namespace opentxs::blockchain::ethereum::rlp

namespace opentxs::blockchain::ethereum::rlp
{
struct Node::Encoder {
    using OutputIterator = std::byte*;

    auto operator()(const Null&) -> bool
    {
        encode_null();

        return true;
    }
    auto operator()(const Sequence& in) -> bool
    {
        const auto payload = std::accumulate(
            in.begin(),
            in.end(),
            0_uz,
            [this](const auto lhs, const auto& rhs) {
                return lhs + rhs.EncodedSize(api_);
            });
        const auto prefix =
            make_prefix(Const::short_list_, Const::long_list_, payload);
        const auto pBytes = prefix.size();
        std::memcpy(out_, prefix.data(), pBytes);
        std::advance(out_, pBytes);
        written_ += pBytes;
        log_(OT_PRETTY_CLASS())("wrote ")(pBytes)(" prefix bytes to output")
            .Flush();
        log_(OT_PRETTY_CLASS())(written_)(" of ")(reserved_bytes_)(" written")
            .Flush();

        for (const auto& node : in) {
            if (false == node.Encode(*this)) { return false; }
        }

        return true;
    }
    auto operator()(const String& in) -> bool
    {
        const auto bytes = in.size();

        if (can_direct_encode(in)) {
            direct_encode(in);
        } else {
            const auto prefix =
                make_prefix(Const::null_, Const::short_string_, bytes);
            const auto pBytes = prefix.size();
            std::memcpy(out_, prefix.data(), pBytes);
            std::advance(out_, pBytes);
            written_ += pBytes;
            log_(OT_PRETTY_CLASS())("wrote ")(pBytes)(" prefix bytes to output")
                .Flush();
            log_(OT_PRETTY_CLASS())(written_)(" of ")(
                reserved_bytes_)(" written")
                .Flush();

            if (0u < bytes) {
                std::memcpy(out_, in.data(), bytes);
                std::advance(out_, bytes);
                written_ += bytes;
                log_(OT_PRETTY_CLASS())("wrote ")(
                    bytes)(" payload bytes to output")
                    .Flush();
                log_(OT_PRETTY_CLASS())(written_)(" of ")(
                    reserved_bytes_)(" written")
                    .Flush();
            }
        }

        return true;
    }

    Encoder(
        const api::Session& api,
        std::size_t reservedBytes,
        OutputIterator it) noexcept
        : api_(api)
        , log_(LogInsane())
        , reserved_bytes_(reservedBytes)
        , out_(it)
        , written_(0)
    {
    }
    Encoder() = delete;
    Encoder(const Encoder&) = delete;
    Encoder(Encoder&&) = delete;
    auto operator=(const Encoder&) -> Encoder& = delete;
    auto operator=(Encoder&&) -> Encoder& = delete;

private:
    using Const = Node::Constants;

    const api::Session& api_;
    const Log& log_;
    const std::size_t reserved_bytes_;
    OutputIterator out_;
    std::size_t written_;

    static auto is_null(const String& in) noexcept -> bool
    {
        if (const auto size = in.size(); size == 0u) {

            return true;
        } else {

            return false;
        }
    }
    static auto can_direct_encode(const String& in) noexcept -> bool
    {
        if (const auto size = in.size(); 0u == size) {

            return true;
        } else if (1u < size) {

            return false;
        } else {

            return Const::to_int(in.at(0)) <=
                   Const::to_int(Const::direct_encode_);
        }
    }

    auto direct_encode(const String& in) noexcept -> void
    {
        if (is_null(in)) {
            encode_null();
        } else {
            const auto& payload = in.at(0);
            const auto bytes = sizeof(payload);
            std::memcpy(out_, in.data(), bytes);
            std::advance(out_, bytes);
            written_ += bytes;
            log_(OT_PRETTY_CLASS())("wrote ")(bytes)(" payload bytes to output")
                .Flush();
            log_(OT_PRETTY_CLASS())(written_)(" of ")(
                reserved_bytes_)(" written")
                .Flush();
        }
    }
    auto encode_null() noexcept -> void
    {
        const auto& payload = Const::null_;
        const auto bytes = sizeof(payload);
        std::memcpy(out_, &payload, bytes);
        std::advance(out_, bytes);
        written_ += bytes;
        log_(OT_PRETTY_CLASS())("wrote ")(bytes)(" payload bytes to output")
            .Flush();
        log_(OT_PRETTY_CLASS())(written_)(" of ")(reserved_bytes_)(" written")
            .Flush();
    }
    auto encode_size(std::byte longVal, std::size_t in) const noexcept -> String
    {
        namespace mp = boost::multiprecision;
        const auto number = mp::checked_cpp_int{in};
        const auto hex = [&] {
            auto ss = std::stringstream{};
            ss << std::hex << std::showbase << number;

            return ss.str();
        }();
        const auto payload = api_.Factory().DataFromHex(hex);

        OT_ASSERT(9 > payload.size());

        const auto size = static_cast<std::uint8_t>(payload.size());
        const auto prefix = std::byte(Const::to_int(longVal) + size);
        auto out = api_.Factory().Data();
        out.Concatenate(&prefix, sizeof(prefix));
        out.Concatenate(payload.Bytes());

        return out;
    }
    auto make_prefix(std::byte shortVal, std::byte longVal, std::size_t bytes)
        const noexcept -> String
    {
        if (Const::long_ > bytes) {
            const auto size = static_cast<std::uint8_t>(bytes);
            const auto prefix = std::byte(Const::to_int(shortVal) + size);
            auto out = api_.Factory().Data();
            out.Concatenate(&prefix, sizeof(prefix));

            return out;
        } else {

            return encode_size(longVal, bytes);
        }
    }
};
}  // namespace opentxs::blockchain::ethereum::rlp

namespace opentxs::blockchain::ethereum::rlp
{
Node::Node() noexcept
    : data_(std::monostate{})
{
}

Node::Node(Data&& data) noexcept
    : data_(std::move(data))
{
}

Node::Node(Null&& value) noexcept
    : data_(std::move(value))
{
}

Node::Node(Sequence&& value) noexcept
    : data_(std::move(value))
{
}

Node::Node(String&& value) noexcept
    : data_(std::move(value))
{
}

Node::Node(const Node& rhs) noexcept
    : Node()
{
    operator=(rhs);
}

Node::Node(Node&& rhs) noexcept
    : Node()
{
    operator=(std::move(rhs));
}

auto Node::Decode(const api::Session& api, ReadView in) noexcept(false) -> Node
{
    auto decoder = Decoder{api, in};

    return decoder();
}

auto Node::Encode(const api::Session& api, AllocateOutput out) const noexcept
    -> bool
{
    const auto& log = LogInsane();
    const auto bytes = EncodedSize(api);

    if (0u < bytes) {
        log(OT_PRETTY_CLASS())("calculated output buffer: ")(bytes)(" bytes")
            .Flush();
    } else {
        LogError()(OT_PRETTY_CLASS())("unable to serialize invalid node")
            .Flush();

        return false;
    }

    if (out.operator bool()) {
        log(OT_PRETTY_CLASS())("output allocator is valid").Flush();
    } else {
        LogError()(OT_PRETTY_CLASS())("invalid output allocator").Flush();

        return false;
    }

    auto buffer = out(bytes);

    if (buffer.valid(bytes)) {
        log(OT_PRETTY_CLASS())(bytes)(" bytes allocated in output buffer")
            .Flush();
    } else {
        LogError()(OT_PRETTY_CLASS())("failed to allocate output buffer")
            .Flush();

        return false;
    }

    auto visitor = Encoder{api, bytes, buffer.as<std::byte>()};

    return Encode(visitor);
}

auto Node::Encode(Encoder& visitor) const noexcept -> bool
{
    return std::visit(visitor, data_);
}

auto Node::EncodedSize(const api::Session& api) const noexcept -> std::size_t
{
    static const auto visitor = Calculator{};

    return EncodedSize(visitor);
}

auto Node::EncodedSize(const Calculator& visitor) const noexcept -> std::size_t
{
    try {

        return std::visit(visitor, data_);
    } catch (...) {

        return 0;
    }
}

auto Node::operator=(const Node& rhs) noexcept -> Node&
{
    if (this != std::addressof(rhs)) { data_ = rhs.data_; }

    return *this;
}

auto Node::operator=(Node&& rhs) noexcept -> Node&
{
    if (this != std::addressof(rhs)) { data_ = std::move(rhs.data_); }

    return *this;
}

auto Node::operator==(const Node& rhs) const noexcept -> bool
{
    return data_ == rhs.data_;
}
}  // namespace opentxs::blockchain::ethereum::rlp
