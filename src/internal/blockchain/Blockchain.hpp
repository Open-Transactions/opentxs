// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <GCS.pb.h>
#include <boost/endian/buffers.hpp>
#include <boost/endian/conversion.hpp>
#include <array>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <iosfwd>
#include <memory>
#include <tuple>
#include <utility>

#include "internal/serialization/protobuf/Proto.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/bloom/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Hash.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Header.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/blockchain/block/Position.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Session;
}  // namespace api

namespace blockchain
{
namespace block
{
class Block;
}  // namespace block

namespace cfilter
{
class Hash;
class Header;
}  // namespace cfilter

class BloomFilter;
class GCS;
class NumericHash;
class Work;
}  // namespace blockchain

namespace display
{
class Definition;
}  // namespace display

namespace proto
{
class GCS;
}  // namespace proto

class Amount;
class ByteArray;
class Data;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace be = boost::endian;

namespace opentxs::blockchain
{
auto GetDefinition(blockchain::Type) noexcept -> const display::Definition&;
}  // namespace opentxs::blockchain

namespace opentxs::blockchain::internal
{
// Source of BitReader class:
// https://github.com/rasky/gcs/blob/master/cpp/gcs.cpp
// The license there reads:
// "This is free and unencumbered software released into the public domain."
class BitReader
{
public:
    auto eof() -> bool;
    auto read(std::size_t nbits) -> std::uint64_t;

    BitReader(const Vector<std::byte>& data);
    BitReader() = delete;
    BitReader(const BitReader&) = delete;
    BitReader(BitReader&&) = delete;
    auto operator=(const BitReader&) -> BitReader& = delete;
    auto operator=(BitReader&&) -> BitReader& = delete;

private:
    const Vector<std::byte>& raw_data_;
    const std::uint8_t* data_;
    std::size_t len_;
    std::uint64_t accum_;
    std::size_t n_;
};

// Source of BitWriter class:
// https://github.com/rasky/gcs/blob/master/cpp/gcs.cpp
// The license there reads:
// "This is free and unencumbered software released into the public domain."
class BitWriter
{
public:
    void flush();
    void write(std::size_t nbits, std::uint64_t value);

    BitWriter(Vector<std::byte>& output);
    BitWriter() = delete;

private:
    static constexpr auto ACCUM_BITS = sizeof(std::uint64_t) * 8_uz;

    Vector<std::byte>& output_;
    std::uint64_t accum_;
    std::size_t n_;
};

struct SerializedBloomFilter {
    be::little_uint32_buf_t function_count_;
    be::little_uint32_buf_t tweak_;
    be::little_uint8_buf_t flags_;

    SerializedBloomFilter(
        const std::uint32_t tweak,
        const BloomUpdateFlag update,
        const std::size_t functionCount) noexcept;
    SerializedBloomFilter() noexcept;
};

using FilterParams = std::pair<std::uint8_t, std::uint32_t>;

auto DefaultFilter(const Type type) noexcept -> cfilter::Type;
auto DecodeSerializedCfilter(const ReadView bytes) noexcept(false)
    -> std::pair<std::uint32_t, ReadView>;
auto Deserialize(const Type chain, const std::uint8_t type) noexcept
    -> cfilter::Type;
auto Deserialize(const api::Session& api, const ReadView bytes) noexcept
    -> block::Position;
auto BlockHashToFilterKey(const ReadView hash) noexcept(false) -> ReadView;
auto FilterHashToHeader(
    const api::Session& api,
    const ReadView hash,
    const ReadView previous = {}) noexcept -> cfilter::Header;
auto FilterToHash(const api::Session& api, const ReadView filter) noexcept
    -> cfilter::Hash;
auto FilterToHeader(
    const api::Session& api,
    const ReadView filter,
    const ReadView previous = {}) noexcept -> cfilter::Header;
auto Format(const Type chain, const opentxs::Amount&) noexcept
    -> UnallocatedCString;
auto GetFilterParams(const cfilter::Type type) noexcept(false) -> FilterParams;
auto Grind(const std::function<void()> function) noexcept -> void;
auto Serialize(const Type chain, const cfilter::Type type) noexcept(false)
    -> std::uint8_t;
auto Serialize(const block::Position& position) noexcept -> Space;
auto Ticker(const Type chain) noexcept -> UnallocatedCString;
}  // namespace opentxs::blockchain::internal

namespace opentxs::blockchain::script
{
}  // namespace opentxs::blockchain::script

namespace opentxs::factory
{
auto BloomFilter(
    const api::Session& api,
    const std::uint32_t tweak,
    const blockchain::BloomUpdateFlag update,
    const std::size_t targets,
    const double falsePositiveRate) -> blockchain::BloomFilter*;
auto BloomFilter(const api::Session& api, const Data& serialized)
    -> blockchain::BloomFilter*;
auto GCS(
    const api::Session& api,
    const std::uint8_t bits,
    const std::uint32_t fpRate,
    const ReadView key,
    const Vector<ByteArray>& elements,
    alloc::Default alloc) noexcept -> blockchain::GCS;
auto GCS(
    const api::Session& api,
    const blockchain::cfilter::Type type,
    const blockchain::block::Block& block,
    alloc::Default alloc) noexcept -> blockchain::GCS;
auto GCS(
    const api::Session& api,
    const proto::GCS& serialized,
    alloc::Default alloc) noexcept -> blockchain::GCS;
auto GCS(
    const api::Session& api,
    const ReadView serialized,
    alloc::Default alloc) noexcept -> blockchain::GCS;
auto GCS(
    const api::Session& api,
    const std::uint8_t bits,
    const std::uint32_t fpRate,
    const ReadView key,
    const std::uint32_t filterElementCount,
    const ReadView filter,
    alloc::Default alloc) noexcept -> blockchain::GCS;
auto GCS(
    const api::Session& api,
    const blockchain::cfilter::Type type,
    const ReadView key,
    const ReadView encoded,
    alloc::Default alloc) noexcept -> blockchain::GCS;
}  // namespace opentxs::factory
