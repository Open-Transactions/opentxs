// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "1_Internal.hpp"                         // IWYU pragma: associated
#include "blockchain/database/common/Blocks.hpp"  // IWYU pragma: associated

#include <cstring>
#include <map>
#include <mutex>
#include <shared_mutex>
#include <type_traits>
#include <utility>

#include "blockchain/database/common/Bulk.hpp"
#include "internal/blockchain/database/common/Common.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Log.hpp"
#include "util/LMDB.hpp"
#include "util/MappedFileStorage.hpp"

namespace opentxs::blockchain::database::common
{
template <typename Input>
auto tsv(const Input& in) noexcept -> ReadView
{
    return {reinterpret_cast<const char*>(&in), sizeof(in)};
}

struct Blocks::Imp {
    storage::lmdb::LMDB& lmdb_;
    Bulk& bulk_;
    const int table_;
    mutable std::mutex lock_;
    mutable std::pmr::map<pHash, std::shared_mutex> block_locks_;

    auto Exists(const Hash& block) const noexcept -> bool
    {
        return lmdb_.Exists(table_, block.Bytes());
    }

    auto Load(const Hash& block) const noexcept -> BlockReader
    {
        auto lock = Lock{lock_};
        auto index = util::IndexData{};
        auto cb = [&index](const auto in) {
            if (sizeof(index) != in.size()) { return; }

            std::memcpy(static_cast<void*>(&index), in.data(), in.size());
        };
        lmdb_.Load(table_, block.Bytes(), cb);

        if (0 == index.size_) {
            LogTrace()(OT_PRETTY_CLASS())("Block ")(block.asHex())(
                " not found in index")
                .Flush();

            return {};
        }

        return BlockReader{bulk_.ReadView(index), block_locks_[block]};
    }

    auto Store(const Hash& block, const std::size_t bytes) const noexcept
        -> BlockWriter
    {
        auto lock = Lock{lock_};

        if (0 == bytes) {
            LogError()(OT_PRETTY_CLASS())("Block ")(block.asHex())(
                " invalid block size")
                .Flush();

            return {};
        }

        auto index = [&] {
            auto output = util::IndexData{};
            auto cb = [&output](const auto in) {
                if (sizeof(output) != in.size()) { return; }

                std::memcpy(static_cast<void*>(&output), in.data(), in.size());
            };
            lmdb_.Load(table_, block.Bytes(), cb);

            return output;
        }();
        auto cb = [&](auto& tx) -> bool {
            const auto result =
                lmdb_.Store(table_, block.Bytes(), tsv(index), tx);

            if (false == result.first) {
                LogError()(OT_PRETTY_CLASS())(
                    "Failed to update index for block ")(block.asHex())
                    .Flush();

                return false;
            }

            return true;
        };
        auto tx = lmdb_.TransactionRW();
        auto view = bulk_.WriteView(tx, index, std::move(cb), bytes);

        if (false == view.valid()) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed to get write position for block ")(block.asHex())
                .Flush();

            return {};
        }

        if (false == tx.Finalize(true)) {
            LogError()(OT_PRETTY_CLASS())("Database error").Flush();

            return {};
        }

        return BlockWriter{std::move(view), block_locks_[block]};
    }

    Imp(storage::lmdb::LMDB& lmdb, Bulk& bulk) noexcept
        : lmdb_(lmdb)
        , bulk_(bulk)
        , table_(Table::BlockIndex)
        , lock_()
        , block_locks_()
    {
    }
};

Blocks::Blocks(storage::lmdb::LMDB& lmdb, Bulk& bulk) noexcept
    : imp_(std::make_unique<Imp>(lmdb, bulk))
{
}

auto Blocks::Exists(const Hash& block) const noexcept -> bool
{
    return imp_->Exists(block);
}

auto Blocks::Load(const Hash& block) const noexcept -> BlockReader
{
    return imp_->Load(block);
}

auto Blocks::Store(const Hash& block, const std::size_t bytes) const noexcept
    -> BlockWriter
{
    return imp_->Store(block, bytes);
}

Blocks::~Blocks() = default;
}  // namespace opentxs::blockchain::database::common
