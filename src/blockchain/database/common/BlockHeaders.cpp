// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "blockchain/database/common/BlockHeaders.hpp"  // IWYU pragma: associated

#include <cstring>
#include <stdexcept>
#include <type_traits>
#include <utility>

#include "Proto.hpp"
#include "Proto.tpp"
#include "blockchain/database/common/Bulk.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/TSV.hpp"
#include "opentxs/blockchain/block/Header.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "serialization/protobuf/BlockchainBlockHeader.pb.h"
#include "util/LMDB.hpp"
#include "util/MappedFileStorage.hpp"

namespace opentxs::blockchain::database::common
{
BlockHeader::BlockHeader(storage::lmdb::LMDB& lmdb, Bulk& bulk) noexcept(false)
    : lmdb_(lmdb)
    , bulk_(bulk)
    , table_(Table::HeaderIndex)
{
}

auto BlockHeader::Exists(
    const opentxs::blockchain::block::Hash& hash) const noexcept -> bool
{
    return lmdb_.Exists(table_, hash.Bytes());
}

auto BlockHeader::Load(const opentxs::blockchain::block::Hash& hash) const
    noexcept(false) -> proto::BlockchainBlockHeader
{
    const auto index = [&] {
        auto out = util::IndexData{};
        auto cb = [&out](const ReadView in) {
            if (sizeof(out) != in.size()) { return; }

            std::memcpy(static_cast<void*>(&out), in.data(), in.size());
        };
        lmdb_.Load(table_, hash.Bytes(), cb);

        if (0 == out.size_) {
            throw std::out_of_range("Block header not found");
        }

        return out;
    }();

    return proto::Factory<proto::BlockchainBlockHeader>(bulk_.ReadView(index));
}

auto BlockHeader::Store(
    const opentxs::blockchain::block::Header& header) const noexcept -> bool
{
    auto tx = lmdb_.TransactionRW();
    auto lock = Lock{bulk_.Mutex()};

    if (false == store(lock, false, tx, header)) { return false; }

    if (tx.Finalize(true)) { return true; }

    LogError()(OT_PRETTY_CLASS())("Database update error").Flush();

    return false;
}

auto BlockHeader::Store(const UpdatedHeader& headers) const noexcept -> bool
{
    auto tx = lmdb_.TransactionRW();
    auto lock = Lock{bulk_.Mutex()};

    for (const auto& [hash, pair] : headers) {
        const auto& [header, newBlock] = pair;

        if (newBlock) {

            if (false == store(lock, true, tx, *header)) { return false; }
        }
    }

    if (tx.Finalize(true)) { return true; }

    LogError()(OT_PRETTY_CLASS())("Database update error").Flush();

    return false;
}

auto BlockHeader::store(
    const Lock& lock,
    bool clearLocal,
    storage::lmdb::LMDB::Transaction& pTx,
    const opentxs::blockchain::block::Header& header) const noexcept -> bool
{
    const auto& hash = header.Hash();

    try {
        const auto proto = [&] {
            auto out = opentxs::blockchain::block::Header::SerializedType{};

            if (false == header.Serialize(out)) {
                throw std::runtime_error{"Failed to serialized header"};
            }

            if (clearLocal) { out.clear_local(); }

            return out;
        }();
        const auto bytes = proto.ByteSizeLong();
        auto index = [&] {
            auto output = util::IndexData{};
            auto cb = [&output](const ReadView in) {
                if (sizeof(output) != in.size()) { return; }

                std::memcpy(static_cast<void*>(&output), in.data(), in.size());
            };
            lmdb_.Load(table_, hash.Bytes(), cb);

            return output;
        }();
        auto cb = [&](auto& tx) -> bool {
            const auto result =
                lmdb_.Store(table_, hash.Bytes(), tsv(index), tx);

            if (false == result.first) {
                LogError()(OT_PRETTY_CLASS())(
                    "Failed to update index for block header ")(hash.asHex())
                    .Flush();

                return false;
            }

            return true;
        };
        auto view = bulk_.WriteView(lock, pTx, index, std::move(cb), bytes);

        if (false == view.valid(bytes)) {
            throw std::runtime_error{
                "Failed to get write position for block header"};
        }

        return proto::write(proto, preallocated(bytes, view.data()));
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}
}  // namespace opentxs::blockchain::database::common
