// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                         // IWYU pragma: associated
#include "1_Internal.hpp"                       // IWYU pragma: associated
#include "blockchain/database/common/Bulk.hpp"  // IWYU pragma: associated

#include <mutex>
#include <utility>

#include "blockchain/database/common/Database.hpp"
#include "internal/blockchain/database/common/Common.hpp"
#include "util/storage/MappedFile.hpp"

namespace opentxs::blockchain::database::common
{
struct Bulk::Imp final : private util::MappedFileStorage {
    auto Mutex() const noexcept -> std::mutex& { return lock_; }
    auto ReadView(const Lock&, const util::IndexData& index) const noexcept
        -> opentxs::ReadView
    {
        return get_read_view(index);
    }
    auto WriteView(
        const Lock&,
        storage::lmdb::Transaction& tx,
        util::IndexData& index,
        UpdateCallback&& cb,
        std::size_t size) const noexcept -> WritableView
    {
        return get_write_view(tx, index, std::move(cb), size);
    }

    Imp(storage::lmdb::Database& lmdb,
        const std::filesystem::path& path) noexcept(false)
        : MappedFileStorage(
              lmdb,
              path,
              "blk",
              Table::Config,
              static_cast<std::size_t>(common::Database::Key::NextBlockAddress))
        , lock_()
    {
    }

private:
    mutable std::mutex lock_;
};

Bulk::Bulk(
    storage::lmdb::Database& lmdb,
    const std::filesystem::path& path) noexcept(false)
    : imp_(std::make_unique<Imp>(lmdb, path))
{
}

auto Bulk::Mutex() const noexcept -> std::mutex& { return imp_->Mutex(); }

auto Bulk::ReadView(const util::IndexData& index) const noexcept
    -> opentxs::ReadView
{
    auto lock = Lock{imp_->Mutex()};

    return ReadView(lock, index);
}

auto Bulk::ReadView(const Lock& lock, const util::IndexData& index)
    const noexcept -> opentxs::ReadView
{
    return imp_->ReadView(lock, index);
}

auto Bulk::WriteView(
    storage::lmdb::Transaction& tx,
    util::IndexData& index,
    UpdateCallback&& cb,
    std::size_t size) const noexcept -> WritableView
{
    auto lock = Lock{imp_->Mutex()};

    return imp_->WriteView(lock, tx, index, std::move(cb), size);
}

auto Bulk::WriteView(
    const Lock& lock,
    storage::lmdb::Transaction& tx,
    util::IndexData& index,
    UpdateCallback&& cb,
    std::size_t size) const noexcept -> WritableView
{
    return imp_->WriteView(lock, tx, index, std::move(cb), size);
}

Bulk::~Bulk() = default;
}  // namespace opentxs::blockchain::database::common
