// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "blockchain/database/common/BlockHeaders.hpp"  // IWYU pragma: associated

#include <BlockchainBlockHeader.pb.h>
#include <cstddef>
#include <memory>
#include <stdexcept>
#include <tuple>
#include <type_traits>
#include <utility>

#include "Proto.hpp"
#include "Proto.tpp"
#include "blockchain/database/common/Bulk.hpp"
#include "internal/blockchain/block/Header.hpp"
#include "internal/blockchain/database/common/Common.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/storage/file/Index.hpp"
#include "internal/util/storage/lmdb/Database.hpp"
#include "internal/util/storage/lmdb/Transaction.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::database::common
{
BlockHeader::BlockHeader(storage::lmdb::Database& lmdb, Bulk& bulk) noexcept(
    false)
    : lmdb_(lmdb)
    , bulk_(bulk)
    , table_(Table::HeaderIndex)
{
}

auto BlockHeader::Exists(const block::Hash& hash) const noexcept -> bool
{
    return lmdb_.Exists(table_, hash.Bytes());
}

auto BlockHeader::Forget(const block::Hash& hash) const noexcept -> bool
{
    return lmdb_.Delete(table_, hash.Bytes());
}

auto BlockHeader::Load(const block::Hash& hash) const noexcept(false)
    -> proto::BlockchainBlockHeader
{
    const auto indices = [&] {
        auto out = Vector<storage::file::Index>{};
        auto cb = [&out](const auto in) {
            auto& index = out.emplace_back();
            index.Deserialize(in);
        };
        lmdb_.Load(table_, hash.Bytes(), cb);

        if (out.empty() || out.front().empty()) {
            throw std::out_of_range("Block header not found");
        }

        return out;
    }();
    const auto views = bulk_.Read(indices);

    OT_ASSERT(false == views.empty());

    const auto& bytes = views.front();

    if (false == valid(bytes)) {
        const auto error =
            CString{"failed to load serialized block header "}.append(
                hash.asHex());

        throw std::out_of_range(error.c_str());
    }

    return proto::Factory<proto::BlockchainBlockHeader>(bytes);
}

auto BlockHeader::Store(const UpdatedHeader& headers) const noexcept -> bool
{
    const auto [hashes, protos, sizes] = [&] {
        auto out = std::tuple<
            Vector<block::Hash>,
            Vector<block::internal::Header::SerializedType>,
            Vector<std::size_t>>{};
        auto& [h, p, s] = out;
        h.reserve(headers.size());
        p.reserve(headers.size());
        s.reserve(headers.size());
        h.clear();
        p.clear();
        s.clear();

        for (const auto& [hash, data] : headers) {
            const auto& [pHeader, save] = data;

            OT_ASSERT(pHeader);

            if (false == save) { continue; }

            const auto& header = *pHeader;
            h.emplace_back(hash);
            auto& proto = p.emplace_back();
            auto& size = s.emplace_back();

            if (false == header.Internal().Serialize(proto)) {
                LogError()(OT_PRETTY_CLASS())("failed to serialized header ")
                    .asHex(hash)
                    .Flush();

                return decltype(out){};
            }

            proto.clear_local();
            size = proto.ByteSizeLong();

            OT_ASSERT(0_uz < size);
        }

        return out;
    }();

    OT_ASSERT(hashes.size() == protos.size());
    OT_ASSERT(hashes.size() == sizes.size());

    auto tx = lmdb_.TransactionRW();
    auto write = bulk_.Write(tx, sizes);

    for (auto i = 0_uz; i < hashes.size(); ++i) {
        const auto& hash = hashes.at(i);
        const auto& proto = protos.at(i);
        const auto& bytes = sizes.at(i);
        auto& writeData = write.at(i);

        try {
            auto& [index, view] = writeData;
            const auto sIndex = index.Serialize();

            if (false == view.valid(bytes)) {
                throw std::runtime_error{
                    "Failed to get write position for block header"};
            }

            if (!proto::write(proto, preallocated(bytes, view.data()))) {

                throw std::runtime_error{"Failed to write block header"};
            }

            const auto result =
                lmdb_.Store(table_, hash.Bytes(), sIndex.Bytes(), tx);

            if (result.first) {
                LogTrace()(OT_PRETTY_CLASS())("saved ")(
                    bytes)(" bytes at position ")(index.MemoryPosition())(
                    " for block header ")
                    .asHex(hash)
                    .Flush();
            } else {
                throw std::runtime_error{
                    "Failed to update index for block header"};
            }
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            return false;
        }
    }

    if (tx.Finalize(true)) {

        return true;
    } else {
        LogError()(OT_PRETTY_CLASS())("Database update error").Flush();

        return false;
    }
}
}  // namespace opentxs::blockchain::database::common
