// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/database/common/BlockFilter.hpp"  // IWYU pragma: associated

#include <cstring>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "Proto.hpp"
#include "Proto.tpp"
#include "blockchain/database/common/Bulk.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/TSV.hpp"
#include "opentxs/blockchain/GCS.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "serialization/protobuf/BlockchainFilterHeader.pb.h"
#include "serialization/protobuf/GCS.pb.h"
#include "util/LMDB.hpp"
#include "util/MappedFileStorage.hpp"

namespace opentxs::blockchain::database::common
{
BlockFilter::BlockFilter(
    const api::Session& api,
    storage::lmdb::LMDB& lmdb,
    Bulk& bulk) noexcept
    : api_(api)
    , lmdb_(lmdb)
    , bulk_(bulk)
{
}

auto BlockFilter::HaveFilter(const filter::Type type, const ReadView blockHash)
    const noexcept -> bool
{
    try {
        return lmdb_.Exists(translate_filter(type), blockHash);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto BlockFilter::HaveFilterHeader(
    const filter::Type type,
    const ReadView blockHash) const noexcept -> bool
{
    try {
        return lmdb_.Exists(translate_header(type), blockHash);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto BlockFilter::LoadFilter(const filter::Type type, const ReadView blockHash)
    const noexcept -> std::unique_ptr<const opentxs::blockchain::GCS>
{
    auto output = std::unique_ptr<const opentxs::blockchain::GCS>{};

    try {
        const auto index = [&] {
            auto out = util::IndexData{};
            auto cb = [&out](const ReadView in) {
                if (sizeof(out) != in.size()) { return; }

                std::memcpy(static_cast<void*>(&out), in.data(), in.size());
            };
            lmdb_.Load(translate_filter(type), blockHash, cb);

            if (0 == out.size_) {
                throw std::out_of_range("Cfilter not found");
            }

            return out;
        }();
        output = factory::GCS(
            api_, proto::Factory<proto::GCS>(bulk_.ReadView(index)));
    } catch (const std::exception& e) {
        LogVerbose()(OT_PRETTY_CLASS())(e.what()).Flush();
    }

    return output;
}

auto BlockFilter::LoadFilterHash(
    const filter::Type type,
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

auto BlockFilter::LoadFilterHeader(
    const filter::Type type,
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

auto BlockFilter::store(
    const Lock& lock,
    storage::lmdb::LMDB::Transaction& tx,
    const ReadView blockHash,
    const filter::Type type,
    const GCS& filter) const noexcept -> bool
{
    try {
        const auto proto = [&] {
            auto out = proto::GCS{};

            if (false == filter.Serialize(out)) {
                throw std::runtime_error{"Failed to serialize gcs"};
            }

            return out;
        }();
        const auto bytes = proto.ByteSizeLong();
        const auto table = translate_filter(type);
        auto index = [&] {
            auto output = util::IndexData{};
            auto cb = [&output](const ReadView in) {
                if (sizeof(output) != in.size()) { return; }

                std::memcpy(static_cast<void*>(&output), in.data(), in.size());
            };
            lmdb_.Load(table, blockHash, cb);

            return output;
        }();
        auto cb = [&](auto& tx) -> bool {
            const auto result = lmdb_.Store(table, blockHash, tsv(index), tx);

            if (false == result.first) {
                LogError()(OT_PRETTY_CLASS())(
                    "Failed to update index for cfilter header")
                    .Flush();

                return false;
            }

            return true;
        };
        auto view = bulk_.WriteView(lock, tx, index, std::move(cb), bytes);

        if (false == view.valid(bytes)) {
            throw std::runtime_error{
                "Failed to get write position for cfilter"};
        }

        return proto::write(proto, preallocated(bytes, view.data()));
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto BlockFilter::StoreFilterHeaders(
    const filter::Type type,
    const UnallocatedVector<FilterHeader>& headers) const noexcept -> bool
{
    return StoreFilters(type, headers, {});
}

auto BlockFilter::StoreFilters(
    const filter::Type type,
    UnallocatedVector<FilterData>& filters) const noexcept -> bool
{
    return StoreFilters(type, {}, filters);
}

auto BlockFilter::StoreFilters(
    const filter::Type type,
    const UnallocatedVector<FilterHeader>& headers,
    const UnallocatedVector<FilterData>& filters) const noexcept -> bool
{
    auto tx = lmdb_.TransactionRW();
    auto lock = Lock{bulk_.Mutex()};

    for (const auto& [block, header, hash] : headers) {
        auto proto = proto::BlockchainFilterHeader();
        proto.set_version(1);
        proto.set_header(header->str());
        proto.set_hash(UnallocatedCString{hash});
        auto bytes = space(proto.ByteSize());
        proto.SerializeWithCachedSizesToArray(
            reinterpret_cast<std::uint8_t*>(bytes.data()));

        try {
            const auto stored = lmdb_.Store(
                translate_header(type), block->Bytes(), reader(bytes), tx);

            if (false == stored.first) { return false; }
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            return false;
        }
    }

    for (auto& [block, pFilter] : filters) {
        OT_ASSERT(pFilter);

        const auto& filter = *pFilter;

        if (false == store(lock, tx, block, type, filter)) { return false; }
    }

    return tx.Finalize(true);
}

auto BlockFilter::translate_filter(const filter::Type type) noexcept(false)
    -> Table
{
    switch (type) {
        case filter::Type::Basic_BIP158: {
            return FilterIndexBasic;
        }
        case filter::Type::Basic_BCHVariant: {
            return FilterIndexBCH;
        }
        case filter::Type::ES: {
            return FilterIndexES;
        }
        default: {
            throw std::runtime_error("Unsupported filter type");
        }
    }
}

auto BlockFilter::translate_header(const filter::Type type) noexcept(false)
    -> Table
{
    switch (type) {
        case filter::Type::Basic_BIP158: {
            return FilterHeadersBasic;
        }
        case filter::Type::Basic_BCHVariant: {
            return FilterHeadersBCH;
        }
        case filter::Type::ES: {
            return FilterHeadersOpentxs;
        }
        default: {
            throw std::runtime_error("Unsupported filter type");
        }
    }
}
}  // namespace opentxs::blockchain::database::common
