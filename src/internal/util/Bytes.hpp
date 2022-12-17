// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include <tuple>

#pragma once

#include <array>  // IWYU pragma: keep
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <string_view>
#include <utility>

#include "opentxs/util/Bytes.hpp"  // IWYU pragma: keep
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"
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
