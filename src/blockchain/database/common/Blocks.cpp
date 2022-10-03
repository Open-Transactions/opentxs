// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "blockchain/database/common/Blocks.hpp"  // IWYU pragma: associated

#include <stdexcept>
#include <utility>

#include "blockchain/database/common/Bulk.hpp"
#include "internal/blockchain/block/Block.hpp"
#include "internal/blockchain/database/common/Common.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/storage/file/Index.hpp"
#include "internal/util/storage/lmdb/Database.hpp"
#include "internal/util/storage/lmdb/Transaction.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/core/FixedByteArray.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"

namespace opentxs::blockchain::database::common
{
struct Blocks::Imp {
    const int table_;
    storage::lmdb::Database& lmdb_;
    Bulk& bulk_;

    auto Exists(const block::Hash& block) const noexcept -> bool
    {
        return lmdb_.Exists(table_, block.Bytes());
    }
    auto Forget(const block::Hash& block) const noexcept -> bool
    {
        return lmdb_.Delete(table_, block.Bytes());
    }
    auto Load(const block::Hash& block) const noexcept -> ReadView
    {
        try {
            auto indices = Vector<storage::file::Index>{};
            auto cb = [&indices](const auto in) {
                auto& index = indices.emplace_back();
                index.Deserialize(in);
            };
            lmdb_.Load(table_, block.Bytes(), cb);

            if (indices.empty() || indices.front().empty()) {
                throw std::runtime_error{"block not found in index"};
            }

            auto views = bulk_.Read(indices);

            OT_ASSERT(false == views.empty());

            auto view = views.front();

            if (false == valid(view)) { Forget(block); }

            return view;
        } catch (const std::exception& e) {
            LogTrace()(OT_PRETTY_CLASS())(e.what()).Flush();

            return {};
        }
    }

    auto Store(const block::Block& block) const noexcept -> bool
    {
        try {
            const auto size = block.Internal().CalculateSize();
            auto tx = lmdb_.TransactionRW();
            auto data = bulk_.Write(tx, {size});

            OT_ASSERT(false == data.empty());

            auto& [index, view] = data.front();

            if (false == block.Serialize(preallocated(size, view.data()))) {
                throw std::runtime_error{"failed to serialize block"};
            }

            const auto& id = block::Hash();
            const auto sIndex = index.Serialize();
            const auto result =
                lmdb_.Store(table_, id.Bytes(), sIndex.Bytes(), tx);

            if (result.first) {
                LogTrace()(OT_PRETTY_CLASS())("saved ")(index.ItemSize())(
                    " bytes at position ")(index.MemoryPosition())(
                    " for block ")
                    .asHex(id)
                    .Flush();
            } else {
                throw std::runtime_error{"Failed to update index for block"};
            }

            if (false == tx.Finalize(true)) {
                throw std::runtime_error{"database error"};
            }

            return true;
        } catch (const std::exception& e) {
            LogTrace()(OT_PRETTY_CLASS())(e.what()).Flush();

            return false;
        }
    }

    Imp(storage::lmdb::Database& lmdb, Bulk& bulk) noexcept
        : table_(Table::BlockIndex)
        , lmdb_(lmdb)
        , bulk_(bulk)
    {
    }
};

Blocks::Blocks(storage::lmdb::Database& lmdb, Bulk& bulk) noexcept
    : imp_(std::make_unique<Imp>(lmdb, bulk))
{
}

auto Blocks::Exists(const block::Hash& block) const noexcept -> bool
{
    return imp_->Exists(block);
}

auto Blocks::Forget(const block::Hash& block) const noexcept -> bool
{
    return imp_->Forget(block);
}

auto Blocks::Load(const block::Hash& block) const noexcept -> ReadView
{
    return imp_->Load(block);
}

auto Blocks::Store(const block::Block& block) const noexcept -> bool
{
    return imp_->Store(block);
}

Blocks::~Blocks() = default;
}  // namespace opentxs::blockchain::database::common
