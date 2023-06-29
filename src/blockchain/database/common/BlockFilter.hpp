// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <cstdint>
#include <span>
#include <tuple>

#include "internal/blockchain/database/common/Common.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/Types.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace google
{
namespace protobuf
{
class Arena;
}  // namespace protobuf
}  // namespace google

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
class Hash;
}  // namespace block

namespace database
{
namespace common
{
class Bulk;
}  // namespace common
}  // namespace database

class GCS;
}  // namespace blockchain

namespace proto
{
class GCS;
}  // namespace proto

namespace storage
{
namespace file
{
class Index;
}  // namespace file

namespace lmdb
{
class Database;
class Transaction;
}  // namespace lmdb
}  // namespace storage

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::database::common
{
class BlockFilter
{
public:
    auto ForgetCfilter(const cfilter::Type type, const ReadView blockHash)
        const noexcept -> bool;
    auto HaveCfilter(const cfilter::Type type, const ReadView blockHash)
        const noexcept -> bool;
    auto HaveCfheader(const cfilter::Type type, const ReadView blockHash)
        const noexcept -> bool;
    auto LoadCfilter(
        const cfilter::Type type,
        const ReadView blockHash,
        alloc::Strategy alloc) const noexcept -> opentxs::blockchain::GCS;
    auto LoadCfilters(
        const cfilter::Type type,
        std::span<const block::Hash> blocks,
        alloc::Strategy alloc) const noexcept -> Vector<GCS>;
    auto LoadCfilterHash(
        const cfilter::Type type,
        const ReadView blockHash,
        Writer&& filterHash) const noexcept -> bool;
    auto LoadCfheader(
        const cfilter::Type type,
        const ReadView blockHash,
        Writer&& header) const noexcept -> bool;
    auto StoreCfheaders(
        const cfilter::Type type,
        const Vector<CFHeaderParams>& headers) const noexcept -> bool;
    auto StoreCfilters(
        const cfilter::Type type,
        const Vector<CFilterParams>& filters,
        alloc::Strategy alloc) const noexcept -> bool;
    auto StoreCfilters(
        const cfilter::Type type,
        const Vector<CFHeaderParams>& headers,
        const Vector<CFilterParams>& filters,
        alloc::Strategy alloc) const noexcept -> bool;

    BlockFilter(
        const api::Session& api,
        storage::lmdb::Database& lmdb,
        Bulk& bulk) noexcept;

private:
    using Hashes = Vector<ReadView>;
    using Protos = Vector<proto::GCS*>;
    using Sizes = Vector<std::size_t>;
    using Parsed = std::tuple<Hashes, Protos, Sizes>;

    static const std::uint32_t blockchain_filter_header_version_{1};
    static const std::uint32_t blockchain_filter_headers_version_{1};
    static const std::uint32_t blockchain_filter_version_{1};
    static const std::uint32_t blockchain_filters_version_{1};

    const api::Session& api_;
    storage::lmdb::Database& lmdb_;
    Bulk& bulk_;

    static auto make_arena(unsigned long long capacity) noexcept
        -> google::protobuf::Arena;
    static auto parse(
        const Vector<CFilterParams>& filters,
        google::protobuf::Arena& arena,
        alloc::Default alloc) noexcept(false) -> Parsed;
    static auto translate_filter(const cfilter::Type type) noexcept(false)
        -> Table;
    static auto translate_header(const cfilter::Type type) noexcept(false)
        -> Table;

    auto load_cfilter_index(
        const cfilter::Type type,
        const ReadView blockHash,
        storage::file::Index& out) const noexcept(false) -> void;
    auto load_cfilter_index(
        const cfilter::Type type,
        const ReadView blockHash,
        storage::lmdb::Transaction& tx,
        storage::file::Index& out) const noexcept(false) -> void;
    auto store(
        const Parsed& parsed,
        cfilter::Type type,
        storage::lmdb::Transaction& tx) const noexcept -> bool;
    auto store_cfheaders(
        const cfilter::Type type,
        const Vector<CFHeaderParams>& headers,
        storage::lmdb::Transaction& tx) const noexcept -> bool;
};
}  // namespace opentxs::blockchain::database::common
