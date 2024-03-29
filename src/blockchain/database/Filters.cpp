// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/database/Filters.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <optional>
#include <utility>

#include "blockchain/database/common/Database.hpp"
#include "internal/blockchain/Blockchain.hpp"
#include "internal/blockchain/database/Types.hpp"
#include "internal/blockchain/params/ChainData.hpp"
#include "internal/util/storage/lmdb/Database.hpp"
#include "internal/util/storage/lmdb/Transaction.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/cfilter/GCS.hpp"
#include "opentxs/blockchain/cfilter/Hash.hpp"
#include "opentxs/blockchain/cfilter/Header.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"  // IWYU pragma: keep

namespace opentxs::blockchain::database
{
Filters::Filters(
    const api::Session& api,
    const common::Database& common,
    const storage::lmdb::Database& lmdb,
    const blockchain::Type chain) noexcept
    : api_(api)
    , common_(common)
    , lmdb_(lmdb)
    , blank_position_()
    , lock_()
{
    import_genesis(chain);
}

auto Filters::CurrentHeaderTip(const cfilter::Type type) const noexcept
    -> block::Position
{
    auto output{blank_position_};
    auto cb = [this, &output](const auto in) {
        output = blockchain::internal::Deserialize(api_, in);
    };
    lmdb_.Load(
        Table::BlockFilterHeaderBest, static_cast<std::size_t>(type), cb);

    return output;
}

auto Filters::CurrentTip(const cfilter::Type type) const noexcept
    -> block::Position
{
    auto output{blank_position_};
    auto cb = [this, &output](const auto in) {
        output = blockchain::internal::Deserialize(api_, in);
    };
    lmdb_.Load(Table::BlockFilterBest, static_cast<std::size_t>(type), cb);

    return output;
}

auto Filters::HaveFilter(const cfilter::Type type, const block::Hash& block)
    const noexcept -> bool
{
    return common_.HaveFilter(type, block.Bytes());
}

auto Filters::HaveFilterHeader(
    const cfilter::Type type,
    const block::Hash& block) const noexcept -> bool
{
    return common_.HaveFilterHeader(type, block.Bytes());
}

auto Filters::import_genesis(const blockchain::Type chain) const noexcept
    -> void
{
    auto alloc = alloc::Strategy{};
    const auto& data = params::get(chain);

    for (const auto& style : data.KnownCfilterTypes()) {
        const auto needHeader =
            blank_position_.height_ == CurrentHeaderTip(style).height_;
        const auto needFilter =
            blank_position_.height_ == CurrentTip(style).height_;
        const auto& blockHash = data.GenesisHash();
        const auto& gcs = data.GenesisCfilter(api_, style);
        auto success{false};
        auto headers = Vector<CFHeaderParams>{alloc.work_};
        headers.clear();
        headers.emplace_back(
            blockHash, data.GenesisCfheader(style), gcs.Hash());
        success = common_.StoreFilterHeaders(style, headers);

        assert_true(success);

        if (needHeader) {
            success = SetHeaderTip(style, {0, blockHash});

            assert_true(success);
        }

        auto filters = Vector<database::Cfilter::CFilterParams>{alloc.work_};
        filters.clear();
        filters.emplace_back(blockHash, std::move(gcs));
        success = common_.StoreFilters(style, filters, alloc.work_);

        assert_true(success);

        if (needFilter) {
            success = SetTip(style, {0, blockHash});

            assert_true(success);
        }

        const auto loaded = LoadFilter(style, blockHash.Bytes(), alloc);

        assert_true(loaded.IsValid());
    }
}

auto Filters::LoadFilter(
    const cfilter::Type type,
    const ReadView block,
    alloc::Strategy alloc) const noexcept -> cfilter::GCS
{
    return common_.LoadFilter(type, block, alloc);
}

auto Filters::LoadFilters(
    const cfilter::Type type,
    std::span<const block::Hash> blocks,
    alloc::Strategy alloc) const noexcept -> Vector<cfilter::GCS>
{
    return common_.LoadFilters(type, blocks, alloc);
}

auto Filters::LoadFilterHash(const cfilter::Type type, const ReadView block)
    const noexcept -> cfilter::Hash
{
    auto output = cfilter::Hash{};

    if (common_.LoadFilterHash(type, block, output.WriteInto())) {

        return output;
    }

    return cfilter::Hash{};
}

auto Filters::LoadFilterHeader(const cfilter::Type type, const ReadView block)
    const noexcept -> cfilter::Header
{
    auto output = cfilter::Header{};

    if (common_.LoadFilterHeader(type, block, output.WriteInto())) {

        return output;
    }

    return {};
}

auto Filters::SetHeaderTip(
    const cfilter::Type type,
    const block::Position& position) const noexcept -> bool
{
    return lmdb_
        .Store(
            Table::BlockFilterHeaderBest,
            static_cast<std::size_t>(type),
            reader(blockchain::internal::Serialize(position)))
        .first;
}

auto Filters::SetTip(const cfilter::Type type, const block::Position& position)
    const noexcept -> bool
{
    return lmdb_
        .Store(
            Table::BlockFilterBest,
            static_cast<std::size_t>(type),
            reader(blockchain::internal::Serialize(position)))
        .first;
}

auto Filters::StoreFilters(
    const cfilter::Type type,
    const Vector<CFHeaderParams>& headers,
    const Vector<CFilterParams>& filters,
    const block::Position& tip,
    alloc::Strategy alloc) const noexcept -> bool
{
    auto output = common_.StoreFilters(type, headers, filters, alloc);

    if (false == output) {
        LogError()()("Failed to save filters").Flush();

        return false;
    }

    if (0 > tip.height_) { return true; }

    auto parentTxn = lmdb_.TransactionRW();
    output = lmdb_
                 .Store(
                     Table::BlockFilterHeaderBest,
                     static_cast<std::size_t>(type),
                     reader(blockchain::internal::Serialize(tip)),
                     parentTxn)
                 .first;

    if (false == output) {
        LogError()()("Failed to set header tip").Flush();

        return false;
    }

    output = lmdb_
                 .Store(
                     Table::BlockFilterBest,
                     static_cast<std::size_t>(type),
                     reader(blockchain::internal::Serialize(tip)),
                     parentTxn)
                 .first;

    if (false == output) {
        LogError()()("Failed to set filter tip").Flush();

        return false;
    }

    return parentTxn.Finalize(true);
}

auto Filters::StoreFilters(
    const cfilter::Type type,
    Vector<CFilterParams> filters,
    alloc::Strategy alloc) const noexcept -> bool
{
    return common_.StoreFilters(type, filters, alloc);
}

auto Filters::StoreHeaders(
    const cfilter::Type type,
    const ReadView previous,
    const Vector<CFHeaderParams> headers) const noexcept -> bool
{
    return common_.StoreFilterHeaders(type, headers);
}
}  // namespace opentxs::blockchain::database
