// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/database/common/Blocks.hpp"  // IWYU pragma: associated

#include <filesystem>
#include <optional>
#include <stdexcept>
#include <utility>

#include "blockchain/database/common/Bulk.hpp"
#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/database/common/Common.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/storage/file/Index.hpp"
#include "internal/util/storage/file/Mapped.hpp"
#include "internal/util/storage/lmdb/Database.hpp"
#include "internal/util/storage/lmdb/Transaction.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/core/FixedByteArray.hpp"
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
    auto Load(
        blockchain::Type chain,
        const std::span<const block::Hash> hashes,
        alloc::Default alloc,
        alloc::Default monotonic) const noexcept
        -> Vector<storage::file::Position>
    {
        const auto count = hashes.size();
        const auto indices = [&] {
            auto out = Vector<storage::file::Index>{monotonic};
            out.reserve(count);
            out.clear();

            for (const auto& id : hashes) {
                auto& index = out.emplace_back();

                try {
                    auto cb = [&](const auto in) { index.Deserialize(in); };
                    lmdb_.Load(table_, id.Bytes(), cb);

                    if (index.empty()) {
                        const auto error = CString{"block "}
                                               .append(id.asHex())
                                               .append(" not found in index");

                        throw std::runtime_error{error.c_str()};
                    }
                } catch (const std::exception& e) {
                    LogTrace()(OT_PRETTY_CLASS())(e.what()).Flush();
                }
            }

            return out;
        }();

        OT_ASSERT(indices.size() == count);

        auto views = bulk_.Read(indices, alloc);

        OT_ASSERT(views.size() == count);

        return views;
    }

    auto Store(
        const block::Hash& id,
        const ReadView bytes,
        alloc::Default monotonic) const noexcept -> storage::file::Position
    {
        try {
            const auto size = bytes.size();
            auto tx = lmdb_.TransactionRW();
            auto data = bulk_.Write(tx, {size});

            OT_ASSERT(false == data.empty());

            auto& [index, location] = data.front();
            const auto& [params, reserved] = location;
            const auto& [filename, offset] = params;

            if (reserved != size) {
                throw std::runtime_error{
                    "failed to get write position for block"};
            }

            const auto written =
                storage::file::Mapped::Write(bytes, params, monotonic);

            if (false == written) {
                throw std::runtime_error{"failed to write block"};
            }

            const auto sIndex = index.Serialize();
            const auto result =
                lmdb_.Store(table_, id.Bytes(), sIndex.Bytes(), tx);

            if (result.first) {
                LogDebug()(OT_PRETTY_CLASS())("saved ")(index.ItemSize())(
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

            return {filename, offset, size};
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

            return {};
        }
    }

    Imp(storage::lmdb::Database& lmdb, Bulk& bulk) noexcept
        : table_(Table::BlockIndex)
        , lmdb_(lmdb)
        , bulk_(bulk)
    {
        for (const auto chain : SupportedChains()) { check_genesis(chain); }
    }

private:
    auto check_genesis(blockchain::Type chain) noexcept -> void
    {
        const auto& id = params::get(chain).GenesisHash();
        auto index = storage::file::Index{};
        lmdb_.Load(
            table_, id.Bytes(), [&](const auto in) { index.Deserialize(in); });

        if (index.empty()) {
            const auto& bytes = params::get(chain).GenesisBlockSerialized();
            Store(id, bytes, {});  // TODO monotonic allocator
        }
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

auto Blocks::Load(
    blockchain::Type chain,
    const std::span<const block::Hash> hashes,
    alloc::Default alloc,
    alloc::Default monotonic) const noexcept -> Vector<storage::file::Position>
{
    return imp_->Load(chain, hashes, alloc, monotonic);
}

auto Blocks::Store(
    const block::Hash& id,
    const ReadView bytes,
    alloc::Default monotonic) const noexcept -> storage::file::Position
{
    return imp_->Store(id, bytes, monotonic);
}

Blocks::~Blocks() = default;
}  // namespace opentxs::blockchain::database::common
