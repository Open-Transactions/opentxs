// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/util/Bytes.hpp"  // IWYU pragma: associated
#include "opentxs/util/Bytes.hpp"   // IWYU pragma: associated

#include <algorithm>
#include <cstring>
#include <functional>
#include <span>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs
{
auto check_at_least(
    const std::size_t have,
    const std::size_t required,
    const std::string_view msg) noexcept(false) -> void
{
    if (have < required) {
        const auto error = UnallocatedCString{"expected "}
                               .append(std::to_string(required))
                               .append(" bytes for ")
                               .append(msg)
                               .append(" but only have ")
                               .append(std::to_string(have))
                               .append(" bytes available");

        throw std::runtime_error{error.c_str()};
    }
}

auto check_at_least(
    const ReadView in,
    const std::size_t required,
    const std::string_view msg) noexcept(false) -> void
{
    check_at_least(in.size(), required, msg);
}

auto check_exactly(
    const ReadView in,
    const std::size_t required,
    const std::string_view msg) noexcept(false) -> void
{
    if (in.size() != required) {
        const auto error = UnallocatedCString{"expected "}
                               .append(std::to_string(required))
                               .append(" bytes for ")
                               .append(msg)
                               .append(" but have ")
                               .append(std::to_string(in.size()))
                               .append(" bytes");

        throw std::runtime_error{error.c_str()};
    }
}

auto check_finished(const ReadView in) noexcept(false) -> void
{
    if (false == in.empty()) {
        const auto error = UnallocatedCString{"expected empty view but have "}
                               .append(std::to_string(in.size()))
                               .append(" bytes remaining");

        throw std::runtime_error{error.c_str()};
    }
}

auto check_finished(const WriteBuffer& out) noexcept(false) -> void
{
    if (false == out.empty()) {
        const auto error =
            UnallocatedCString{"expected empty write buffer but have "}
                .append(std::to_string(out.size()))
                .append(" bytes remaining");

        throw std::runtime_error{error.c_str()};
    }
}

auto check_finished_nonfatal(
    const ReadView in,
    const std::string_view msg) noexcept -> void
{
    if (false == in.empty()) {
        LogError()(in.size())(" unexpected bytes remaining after parsing ")(msg)
            .Flush();
    }
}

auto check_finished_nonfatal(
    const WriteBuffer& out,
    const std::string_view msg) noexcept -> void
{
    if (false == out.empty()) {
        LogError()(out.size())(" unexpected bytes remaining after parsing ")(
            msg)
            .Flush();
    }
}

auto copy(const ReadView in, Writer&& out) noexcept -> bool
{
    return copy(in, std::move(out), in.size());
}

auto copy(const ReadView in, Writer&& out, const std::size_t limit) noexcept
    -> bool
{
    if ((nullptr == in.data()) || (0_uz == in.size()) || (0_uz == limit)) {
        return true;
    }

    const auto size = std::min(in.size(), limit);
    auto write = out.Reserve(size);

    if (false == write.IsValid(size)) {
        LogError()(__func__)(": failed to allocate space").Flush();

        return false;
    }

    OT_ASSERT(size == write.size());

    std::memcpy(write.data(), in.data(), size);

    return true;
}

auto copy(
    const ReadView in,
    WriteBuffer& out,
    const std::string_view msg) noexcept(false) -> void
{
    if (false == copy(in, out.Write(in.size()))) {
        const auto error = UnallocatedCString{"failed to write "}.append(msg);

        throw std::runtime_error{error.c_str()};
    }
}

auto deserialize(
    ReadView& in,
    Writer&& out,
    std::size_t bytes,
    const std::string_view msg) noexcept(false) -> void
{
    if (copy(in, std::move(out), bytes)) {
        in.remove_prefix(bytes);
    } else {
        const auto error = UnallocatedCString{"failed to read "}.append(msg);

        throw std::runtime_error{error.c_str()};
    }
}

auto extract_prefix(
    std::string_view& view,
    std::size_t bytes,
    const std::string_view msg) noexcept(false) -> std::string_view
{
    check_at_least(view, bytes, msg);
    auto out = view.substr(0_uz, bytes);
    view.remove_prefix(bytes);

    return out;
}

auto preallocated(const std::size_t size, void* out) noexcept -> Writer
{
    return {[=](const auto in) -> WriteBuffer {
        if (in <= size) {

            return std::span<std::byte>{static_cast<std::byte*>(out), in};
        } else {
            LogError()("preallocated(): Requested ")(in)(" bytes but only ")(
                size)(" are available")
                .Flush();

            return std::span<std::byte>{};
        }
    }};
}

auto reader(const Space& in) noexcept -> ReadView
{
    return {reinterpret_cast<const char*>(in.data()), in.size()};
}

auto reader(const Vector<std::byte>& in) noexcept -> ReadView
{
    return {reinterpret_cast<const char*>(in.data()), in.size()};
}

auto reader(const UnallocatedVector<std::uint8_t>& in) noexcept -> ReadView
{
    return {reinterpret_cast<const char*>(in.data()), in.size()};
}

auto reserve(
    Writer&& destination,
    std::size_t bytes,
    const std::string_view msg) noexcept(false) -> WriteBuffer
{
    auto out = destination.Reserve(bytes);

    if ((0_uz < bytes) && (false == out.IsValid(bytes))) {
        const auto error =
            UnallocatedCString{"failed to reserve space for "}.append(msg);

        throw std::runtime_error{error.c_str()};
    }

    return out;
}

auto serialize_compact_size(
    std::size_t value,
    WriteBuffer& out,
    const std::string_view msg) noexcept(false) -> void
{
    using network::blockchain::bitcoin::CompactSize;

    serialize_compact_size(CompactSize{value}, out, msg);
}

auto serialize_compact_size(
    const network::blockchain::bitcoin::CompactSize& value,
    WriteBuffer& out,
    const std::string_view msg) noexcept(false) -> void
{
    if (false == value.Encode(out.Write(value.Size()))) {
        const auto error =
            UnallocatedCString{"failed to serialize "}.append(msg);

        throw std::runtime_error{error.c_str()};
    }
}

auto space(const std::size_t size) noexcept -> Space
{
    auto output = Space{};
    output.assign(size, std::byte{51});

    return output;
}

auto space(const std::size_t size, alloc::Strategy alloc) noexcept
    -> Vector<std::byte>
{
    auto output = Vector<std::byte>{alloc.result_};
    output.assign(size, std::byte{51});

    return output;
}

auto space(const ReadView bytes) noexcept -> Space
{
    if ((nullptr == bytes.data()) || (0_uz == bytes.size())) { return {}; }

    const auto* it = reinterpret_cast<const std::byte*>(bytes.data());

    return {it, it + bytes.size()};
}

auto space(const ReadView bytes, alloc::Strategy alloc) noexcept
    -> Vector<std::byte>
{
    using Out = Vector<std::byte>;

    if ((nullptr == bytes.data()) || (0_uz == bytes.size())) {
        return Out{alloc.result_};
    }

    const auto* it = reinterpret_cast<const std::byte*>(bytes.data());

    return Out{it, it + bytes.size(), alloc.result_};
}

auto valid(const ReadView view) noexcept -> bool
{
    return (nullptr != view.data()) && (0_uz < view.size());
}

auto writer(CString& in) noexcept -> Writer
{
    return {
        [&in](auto size) -> WriteBuffer {
            in.resize(size, 51);
            auto* out = reinterpret_cast<std::byte*>(in.data());

            return std::span<std::byte>{out, in.size()};
        },
        [&in](auto size) -> bool {
            in.resize(size);

            return true;
        }};
}

auto writer(UnallocatedCString& in) noexcept -> Writer
{
    return {
        [&in](auto size) -> WriteBuffer {
            in.resize(size, 51);
            auto* out = reinterpret_cast<std::byte*>(in.data());

            return std::span<std::byte>{out, in.size()};
        },
        [&in](auto size) -> bool {
            in.resize(size);

            return true;
        }};
}

auto writer(UnallocatedCString* protobuf) noexcept -> Writer
{
    if (nullptr == protobuf) { return {}; }

    return writer(*protobuf);
}

auto writer(Space& in) noexcept -> Writer
{
    return {
        [&in](auto size) -> WriteBuffer {
            in.resize(size, std::byte{51});
            auto* out = reinterpret_cast<std::byte*>(in.data());

            return std::span<std::byte>{out, in.size()};
        },
        [&in](auto size) -> bool {
            in.resize(size);

            return true;
        }};
}

auto writer(Vector<std::byte>& in) noexcept -> Writer
{
    return {
        [&in](auto size) -> WriteBuffer {
            in.resize(size, std::byte{51});
            auto* out = reinterpret_cast<std::byte*>(in.data());

            return std::span<std::byte>{out, in.size()};
        },
        [&in](auto size) -> bool {
            in.resize(size);

            return true;
        }};
}
}  // namespace opentxs
