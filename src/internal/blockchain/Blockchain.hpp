// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

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

#include "Proto.hpp"
#include "internal/blockchain/node/Node.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/bitcoin/bloom/Types.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/FilterType.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/core/display/Definition.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "serialization/protobuf/GCS.pb.h"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
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
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace be = boost::endian;

namespace opentxs::gcs
{
auto GolombDecode(
    const std::uint32_t N,
    const std::uint8_t P,
    const Space& encoded) noexcept(false) -> UnallocatedVector<std::uint64_t>;
auto GolombEncode(
    const std::uint8_t P,
    const UnallocatedVector<std::uint64_t>& hashedSet) noexcept(false) -> Space;
auto HashToRange(
    const api::Session& api,
    const ReadView key,
    const std::uint64_t range,
    const ReadView item) noexcept(false) -> std::uint64_t;
auto HashedSetConstruct(
    const api::Session& api,
    const ReadView key,
    const std::uint32_t N,
    const std::uint32_t M,
    const UnallocatedVector<ReadView> items) noexcept(false)
    -> UnallocatedVector<std::uint64_t>;
}  // namespace opentxs::gcs

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

    BitReader(const Space& data);
    BitReader(std::uint8_t* data, int len);

private:
    OTData raw_data_;
    std::uint8_t* data_{nullptr};
    std::size_t len_{};
    std::uint64_t accum_{};
    std::size_t n_{};

    BitReader() = delete;
    BitReader(const BitReader&) = delete;
    BitReader(BitReader&&) = delete;
    auto operator=(const BitReader&) -> BitReader& = delete;
    auto operator=(BitReader&&) -> BitReader& = delete;
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

    BitWriter(Space& output);

private:
    static constexpr auto ACCUM_BITS = std::size_t{sizeof(std::uint64_t) * 8u};

    Space& output_;
    std::uint64_t accum_{};
    std::size_t n_{};

    BitWriter() = delete;
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

#if OT_BLOCKCHAIN
struct Database : virtual public node::internal::BlockDatabase,
                  virtual public node::internal::FilterDatabase,
                  virtual public node::internal::HeaderDatabase,
                  virtual public node::internal::PeerDatabase,
                  virtual public node::internal::WalletDatabase,
                  virtual public node::internal::SyncDatabase {

    ~Database() override = default;
};
#endif  // OT_BLOCKCHAIN

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
    const ReadView previous = {}) noexcept -> OTData;
auto FilterToHash(const api::Session& api, const ReadView filter) noexcept
    -> OTData;
auto FilterToHeader(
    const api::Session& api,
    const ReadView filter,
    const ReadView previous = {}) noexcept -> OTData;
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
#if OT_BLOCKCHAIN
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
    const UnallocatedVector<OTData>& elements) noexcept
    -> std::unique_ptr<blockchain::GCS>;
auto GCS(
    const api::Session& api,
    const blockchain::cfilter::Type type,
    const blockchain::block::Block& block) noexcept
    -> std::unique_ptr<blockchain::GCS>;
auto GCS(const api::Session& api, const proto::GCS& serialized) noexcept
    -> std::unique_ptr<blockchain::GCS>;
auto GCS(const api::Session& api, const ReadView serialized) noexcept
    -> std::unique_ptr<blockchain::GCS>;
auto GCS(
    const api::Session& api,
    const std::uint8_t bits,
    const std::uint32_t fpRate,
    const ReadView key,
    const std::uint32_t filterElementCount,
    const ReadView filter) noexcept -> std::unique_ptr<blockchain::GCS>;
auto GCS(
    const api::Session& api,
    const blockchain::cfilter::Type type,
    const ReadView key,
    const ReadView encoded) noexcept -> std::unique_ptr<blockchain::GCS>;
#endif  // OT_BLOCKCHAIN
auto NumericHash(const blockchain::block::Hash& hash) noexcept
    -> std::unique_ptr<blockchain::NumericHash>;
auto NumericHashNBits(const std::uint32_t nBits) noexcept
    -> std::unique_ptr<blockchain::NumericHash>;
#if OT_BLOCKCHAIN
auto Work(const UnallocatedCString& hex) -> blockchain::Work*;
auto Work(const blockchain::Type chain, const blockchain::NumericHash& target)
    -> blockchain::Work*;
#endif  // OT_BLOCKCHAIN
}  // namespace opentxs::factory
