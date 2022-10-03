// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "blockchain/database/common/BlockFilter.hpp"  // IWYU pragma: associated

#include <BlockchainFilterHeader.pb.h>
#include <GCS.pb.h>
#include <google/protobuf/arena.h>  // IWYU pragma: keep
#include <array>
#include <cstddef>
#include <cstring>
#include <stdexcept>
#include <string_view>
#include <tuple>
#include <type_traits>
#include <utility>

#include "Proto.hpp"
#include "Proto.tpp"
#include "blockchain/database/common/Bulk.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/bitcoin/cfilter/GCS.hpp"
#include "internal/util/BoostPMR.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/Size.hpp"
#include "internal/util/storage/file/Index.hpp"
#include "internal/util/storage/lmdb/Database.hpp"
#include "internal/util/storage/lmdb/Transaction.hpp"
#include "opentxs/blockchain/bitcoin/cfilter/GCS.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "util/ByteLiterals.hpp"

namespace opentxs::blockchain::database::common
{
BlockFilter::BlockFilter(
    const api::Session& api,
    storage::lmdb::Database& lmdb,
    Bulk& bulk) noexcept
    : api_(api)
    , lmdb_(lmdb)
    , bulk_(bulk)
{
}

auto BlockFilter::ForgetCfilter(
    const cfilter::Type type,
    const ReadView blockHash) const noexcept -> bool
{
    return lmdb_.Delete(translate_filter(type), blockHash);
}

auto BlockFilter::HaveCfilter(
    const cfilter::Type type,
    const ReadView blockHash) const noexcept -> bool
{
    try {
        return lmdb_.Exists(translate_filter(type), blockHash);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto BlockFilter::HaveCfheader(
    const cfilter::Type type,
    const ReadView blockHash) const noexcept -> bool
{
    try {
        return lmdb_.Exists(translate_header(type), blockHash);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto BlockFilter::load_cfilter_index(
    const cfilter::Type type,
    const ReadView blockHash,
    storage::file::Index& out) const noexcept(false) -> void
{
    auto tx = lmdb_.TransactionRO();
    load_cfilter_index(type, blockHash, tx, out);
}

auto BlockFilter::load_cfilter_index(
    const cfilter::Type type,
    const ReadView blockHash,
    storage::lmdb::Transaction& tx,
    storage::file::Index& out) const noexcept(false) -> void
{
    auto cb = [&out](const auto in) { out.Deserialize(in); };
    lmdb_.Load(translate_filter(type), blockHash, cb, tx);

    if (out.empty()) { throw std::out_of_range("Cfilter not found"); }
}

auto BlockFilter::LoadCfilter(
    const cfilter::Type type,
    const ReadView blockHash,
    alloc::Default alloc) const noexcept -> opentxs::blockchain::GCS
{
    try {
        const auto results = LoadCfilters(type, [&] {
            auto out = Vector<block::Hash>{alloc};
            out.emplace_back(blockHash);

            return out;
        }());

        if (results.empty()) {
            throw std::out_of_range("failed to load cfilter");
        }

        return results.front();
    } catch (const std::exception& e) {
        LogVerbose()(OT_PRETTY_CLASS())(e.what()).Flush();

        return {alloc};
    }
}

auto BlockFilter::LoadCfilters(
    const cfilter::Type type,
    const Vector<block::Hash>& blocks) const noexcept -> Vector<GCS>
{
    auto alloc = blocks.get_allocator();
    auto output = Vector<GCS>{alloc};
    output.reserve(blocks.size());
    // TODO use a named constant for the cfilter scan batch size.
    constexpr auto allocBytes = (1000_uz * sizeof(storage::file::Index)) +
                                sizeof(Vector<storage::file::Index>);
    auto buf = std::array<std::byte, allocBytes>{};
    auto mr = alloc::BoostMonotonic{buf.data(), buf.size()};
    const auto indices = [&] {
        auto out = Vector<storage::file::Index>{&mr};
        out.reserve(1000_uz);
        auto tx = lmdb_.TransactionRO();

        for (const auto& hash : blocks) {
            try {
                auto& index = out.emplace_back();
                load_cfilter_index(type, hash.Bytes(), tx, index);
            } catch (const std::exception& e) {
                LogVerbose()(OT_PRETTY_CLASS())(e.what()).Flush();
                out.pop_back();

                break;
            }
        }

        return out;
    }();
    const auto views = bulk_.Read(indices);

    OT_ASSERT(views.size() == indices.size());

    for (const auto& view : views) {
        try {
            output.emplace_back(
                factory::GCS(api_, proto::Factory<proto::GCS>(view), alloc));
        } catch (const std::exception& e) {
            LogVerbose()(OT_PRETTY_CLASS())(e.what()).Flush();

            break;
        }
    }

    return output;
}

auto BlockFilter::LoadCfilterHash(
    const cfilter::Type type,
    const ReadView blockHash,
    const AllocateOutput filterHash) const noexcept -> bool
{
    auto output{false};
    auto cb = [&output, &filterHash](const auto in) {
        auto size = in.size();

        if ((nullptr == in.data()) || (0 == size)) { return; }

        auto proto =
            proto::Factory<proto::BlockchainFilterHeader>(in.data(), in.size());
        const auto& field = proto.hash();
        auto bytes = filterHash(field.size());

        if (bytes.valid(field.size())) {
            std::memcpy(bytes, field.data(), bytes);
            output = true;
        }
    };

    try {
        lmdb_.Load(translate_header(type), blockHash, cb);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
    }

    return output;
}

auto BlockFilter::LoadCfheader(
    const cfilter::Type type,
    const ReadView blockHash,
    const AllocateOutput header) const noexcept -> bool
{
    auto output{false};
    auto cb = [&output, &header](const auto in) {
        auto size = in.size();

        if ((nullptr == in.data()) || (0 == size)) { return; }

        auto proto =
            proto::Factory<proto::BlockchainFilterHeader>(in.data(), in.size());
        const auto& field = proto.header();
        auto bytes = header(field.size());

        if (bytes.valid(field.size())) {
            std::memcpy(bytes, field.data(), bytes);
            output = true;
        }
    };

    try {
        lmdb_.Load(translate_header(type), blockHash, cb);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
    }

    return output;
}

auto BlockFilter::make_arena(unsigned long long capacity) noexcept
    -> google::protobuf::Arena
{
    const auto options = [&] {
        auto out = google::protobuf::ArenaOptions{};
        out.start_block_size = convert_to_size(capacity);
        out.max_block_size = convert_to_size(capacity);

        return out;
    }();

    return google::protobuf::Arena{options};
}

auto BlockFilter::parse(
    const Vector<CFilterParams>& filters,
    google::protobuf::Arena& arena,
    alloc::Default alloc) noexcept(false) -> Parsed
{
    auto out = [&] {
        auto data = std::make_tuple(Hashes{alloc}, Protos{alloc}, Sizes{alloc});
        auto& [hashes, protos, sizes] = data;
        hashes.reserve(filters.size());
        protos.reserve(filters.size());
        sizes.reserve(filters.size());
        hashes.clear();
        protos.clear();
        sizes.clear();

        return data;
    }();
    auto& [hashes, protos, sizes] = out;

    for (const auto& [block, cfilter] : filters) {
        if (false == cfilter.IsValid()) {
            throw std::runtime_error{"invalid cfilter"};
        }

        const auto& hash = hashes.emplace_back(block.Bytes());
        auto* pProto = protos.emplace_back(
            google::protobuf::Arena::Create<proto::GCS>(std::addressof(arena)));

        if (nullptr == pProto) {
            throw std::runtime_error{"failed to allocate protobuf"};
        }

        auto& proto = *pProto;

        if (false == cfilter.Internal().Serialize(proto)) {
            throw std::runtime_error{"Failed to serialize gcs"};
        }

        const auto& size = sizes.emplace_back(proto.ByteSizeLong());

        LogInsane()(OT_PRETTY_STATIC(BlockFilter))(
            "serialized cfilter for block ")
            .asHex(hash)(" to ")(size)(" bytes")
            .Flush();
    }

    return out;
}

auto BlockFilter::store(
    const Parsed& parsed,
    cfilter::Type type,
    storage::lmdb::Transaction& tx) const noexcept -> bool
{
    const auto& [hashes, protos, sizes] = parsed;

    OT_ASSERT(hashes.size() == protos.size());
    OT_ASSERT(hashes.size() == sizes.size());

    try {
        auto write = bulk_.Write(tx, sizes);

        for (auto i = 0_uz; i < hashes.size(); ++i) {
            const auto& hash = hashes.at(i);
            const auto* proto = protos.at(i);
            const auto& bytes = sizes.at(i);
            auto& [index, view] = write.at(i);
            const auto sIndex = index.Serialize();

            if ((false == view.valid(bytes)) || (index.empty())) {
                throw std::runtime_error{
                    "Failed to get write position for cfilter"};
            }

            OT_ASSERT(nullptr != proto);

            if (!proto::write(*proto, preallocated(bytes, view.data()))) {

                throw std::runtime_error{"Failed to write cfilter"};
            }

            const auto result =
                lmdb_.Store(translate_filter(type), hash, sIndex.Bytes(), tx);

            if (result.first) {
                LogTrace()(OT_PRETTY_CLASS())("saved ")(
                    bytes)(" bytes at position ")(index.MemoryPosition())(
                    " for cfilter ")
                    .asHex(hash)
                    .Flush();
            } else {
                throw std::runtime_error{"Failed to update index for cfilter"};
            }
        }

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto BlockFilter::StoreCfheaders(
    const cfilter::Type type,
    const Vector<CFHeaderParams>& headers) const noexcept -> bool
{
    auto tx = lmdb_.TransactionRW();
    const auto output = store_cfheaders(type, headers, tx);

    return tx.Finalize(output);
}

auto BlockFilter::store_cfheaders(
    const cfilter::Type type,
    const Vector<CFHeaderParams>& headers,
    storage::lmdb::Transaction& tx) const noexcept -> bool
{
    for (const auto& [block, header, hash] : headers) {
        auto proto = proto::BlockchainFilterHeader();
        proto.set_version(1);
        proto.set_header(header.data(), header.size());
        proto.set_hash(hash.data(), hash.size());
        auto bytes = space(proto.ByteSize());
        proto.SerializeWithCachedSizesToArray(
            reinterpret_cast<std::uint8_t*>(bytes.data()));

        try {
            const auto stored = lmdb_.Store(
                translate_header(type), block.Bytes(), reader(bytes), tx);

            if (false == stored.first) { return false; }
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            return false;
        }
    }

    return true;
}

auto BlockFilter::StoreCfilters(
    const cfilter::Type type,
    const Vector<CFilterParams>& filters) const noexcept -> bool
{
    try {
        auto arena = make_arena(8_mib);
        auto upstream = alloc::StandardToBoost(alloc::System());
        auto mr =
            alloc::BoostMonotonic{static_cast<std::size_t>(4_mib), &upstream};
        auto alloc = alloc::Default{std::addressof(mr)};
        const auto parsed = parse(filters, arena, alloc);
        auto tx = lmdb_.TransactionRW();
        const auto result = store(parsed, type, tx);

        return tx.Finalize(result);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto BlockFilter::StoreCfilters(
    const cfilter::Type type,
    const Vector<CFHeaderParams>& headers,
    const Vector<CFilterParams>& filters) const noexcept -> bool
{
    try {
        if (headers.size() != filters.size()) {
            throw std::runtime_error{
                "wrong number of filters compared to headers"};
        }

        auto arena = make_arena(8_mib);
        auto upstream = alloc::StandardToBoost(alloc::System());
        auto mr =
            alloc::BoostMonotonic{static_cast<std::size_t>(4_mib), &upstream};
        auto alloc = alloc::Default{std::addressof(mr)};
        const auto parsed = parse(filters, arena, alloc);
        auto tx = lmdb_.TransactionRW();

        if (false == store_cfheaders(type, headers, tx)) {
            throw std::runtime_error{"failed to save cfheaders"};
        }

        const auto result = store(parsed, type, tx);

        return tx.Finalize(result);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto BlockFilter::translate_filter(const cfilter::Type type) noexcept(false)
    -> Table
{
    switch (type) {
        case cfilter::Type::Basic_BIP158: {
            return FilterIndexBasic;
        }
        case cfilter::Type::Basic_BCHVariant: {
            return FilterIndexBCH;
        }
        case cfilter::Type::ES: {
            return FilterIndexES;
        }
        case cfilter::Type::Unknown:
        default: {
            throw std::runtime_error("Unsupported filter type");
        }
    }
}

auto BlockFilter::translate_header(const cfilter::Type type) noexcept(false)
    -> Table
{
    switch (type) {
        case cfilter::Type::Basic_BIP158: {
            return FilterHeadersBasic;
        }
        case cfilter::Type::Basic_BCHVariant: {
            return FilterHeadersBCH;
        }
        case cfilter::Type::ES: {
            return FilterHeadersOpentxs;
        }
        case cfilter::Type::Unknown:
        default: {
            throw std::runtime_error("Unsupported filter type");
        }
    }
}
}  // namespace opentxs::blockchain::database::common
