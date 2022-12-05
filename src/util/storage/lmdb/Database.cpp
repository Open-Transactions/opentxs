// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/util/storage/lmdb/Database.hpp"  // IWYU pragma: associated

#include <cstddef>
#include <utility>

#include "internal/util/storage/lmdb/Transaction.hpp"
#include "util/storage/lmdb/DatabasePrivate.hpp"

namespace opentxs::storage::lmdb
{
Database::Database(
    const TableNames& names,
    const std::filesystem::path& folder,
    const TablesToInit init,
    const Flags flags,
    const std::size_t extraTables) noexcept
    : imp_(std::make_unique<DatabasePrivate>(
          names,
          folder,
          init,
          flags,
          extraTables))
{
}

Database::Database(Database&& rhs) noexcept
    : imp_(std::move(rhs.imp_))
{
}

auto Database::Commit() const noexcept -> bool { return imp_->Commit(); }

auto Database::Delete(const Table table) const noexcept -> bool
{
    return imp_->Delete(table);
}

auto Database::Delete(const Table table, Transaction& parent) const noexcept
    -> bool
{
    return imp_->Delete(table, parent);
}

auto Database::Delete(const Table table, const ReadView index) const noexcept
    -> bool
{
    return imp_->Delete(table, index);
}

auto Database::Delete(
    const Table table,
    const ReadView index,
    Transaction& parent) const noexcept -> bool
{
    return imp_->Delete(table, index, parent);
}

auto Database::Delete(const Table table, const std::size_t index) const noexcept
    -> bool
{
    return imp_->Delete(table, index);
}

auto Database::Delete(
    const Table table,
    const std::size_t index,
    Transaction& parent) const noexcept -> bool
{
    return imp_->Delete(table, index, parent);
}

auto Database::Delete(
    const Table table,
    const std::size_t index,
    const ReadView data) const noexcept -> bool
{
    return imp_->Delete(table, index, data);
}

auto Database::Delete(
    const Table table,
    const std::size_t index,
    const ReadView data,
    Transaction& parent) const noexcept -> bool
{
    return imp_->Delete(table, index, data, parent);
}

auto Database::Delete(
    const Table table,
    const ReadView index,
    const ReadView data) const noexcept -> bool
{
    return imp_->Delete(table, index, data);
}

auto Database::Delete(
    const Table table,
    const ReadView index,
    const ReadView data,
    Transaction& parent) const noexcept -> bool
{
    return imp_->Delete(table, index, data, parent);
}

auto Database::Exists(const Table table, const ReadView index) const noexcept
    -> bool
{
    return imp_->Exists(table, index, {});
}

auto Database::Exists(const Table table, const ReadView index, Transaction& tx)
    const noexcept -> bool
{
    return imp_->Exists(table, index, {}, tx);
}

auto Database::Exists(
    const Table table,
    const ReadView index,
    const ReadView value) const noexcept -> bool
{
    return imp_->Exists(table, index, value);
}

auto Database::Exists(
    const Table table,
    const ReadView index,
    const ReadView value,
    Transaction& tx) const noexcept -> bool
{
    return imp_->Exists(table, index, value, tx);
}

auto Database::Load(
    const Table table,
    const ReadView index,
    const Callback cb,
    const Mode multiple) const noexcept -> bool
{
    return imp_->Load(table, index, cb, multiple);
}

auto Database::Load(
    const Table table,
    const ReadView index,
    const Callback cb,
    Transaction& tx,
    const Mode multiple) const noexcept -> bool
{
    return imp_->Load(table, index, cb, multiple, tx);
}

auto Database::Load(
    const Table table,
    const std::size_t index,
    const Callback cb,
    const Mode mode) const noexcept -> bool
{
    return Load(
        table,
        ReadView{reinterpret_cast<const char*>(&index), sizeof(index)},
        cb,
        mode);
}

auto Database::Queue(
    const Table table,
    const ReadView key,
    const ReadView value,
    const Mode mode) const noexcept -> bool
{
    return imp_->Queue(table, key, value, mode);
}

auto Database::Read(const Table table, const ReadCallback cb, const Dir dir)
    const noexcept -> bool
{
    return imp_->Read(table, cb, dir);
}

auto Database::Read(
    const Table table,
    const ReadCallback cb,
    const Dir dir,
    Transaction& parent) const noexcept -> bool
{
    return imp_->Read(table, cb, dir, parent);
}

auto Database::ReadAndDelete(
    const Table table,
    const ReadCallback cb,
    Transaction& tx,
    const UnallocatedCString& message) const noexcept -> bool
{
    return imp_->ReadAndDelete(table, cb, tx, message);
}

auto Database::ReadFrom(
    const Table table,
    const ReadView index,
    const ReadCallback cb,
    const Dir dir) const noexcept -> bool
{
    return imp_->ReadFrom(table, index, cb, dir);
}

auto Database::ReadFrom(
    const Table table,
    const std::size_t index,
    const ReadCallback cb,
    const Dir dir) const noexcept -> bool
{
    return ReadFrom(
        table,
        ReadView{reinterpret_cast<const char*>(&index), sizeof(index)},
        cb,
        dir);
}

auto Database::Store(
    const Table table,
    const ReadView index,
    const ReadView data,
    const Flags flags) const noexcept -> Result
{
    return imp_->Store(table, index, data, flags);
}

auto Database::Store(
    const Table table,
    const ReadView index,
    const ReadView data,
    Transaction& parent,
    const Flags flags) const noexcept -> Result
{
    return imp_->Store(table, index, data, parent, flags);
}

auto Database::Store(
    const Table table,
    const std::size_t index,
    const ReadView data,
    const Flags flags) const noexcept -> Result
{
    return Store(
        table,
        ReadView{reinterpret_cast<const char*>(&index), sizeof(index)},
        data,
        flags);
}

auto Database::Store(
    const Table table,
    const std::size_t index,
    const ReadView data,
    Transaction& parent,
    const Flags flags) const noexcept -> Result
{
    return Store(
        table,
        ReadView{reinterpret_cast<const char*>(&index), sizeof(index)},
        data,
        parent,
        flags);
}

auto Database::StoreOrUpdate(
    const Table table,
    const ReadView index,
    const UpdateCallback cb,
    const Flags flags) const noexcept -> Result
{
    return imp_->StoreOrUpdate(table, index, cb, flags);
}

auto Database::StoreOrUpdate(
    const Table table,
    const ReadView index,
    const UpdateCallback cb,
    Transaction& parent,
    const Flags flags) const noexcept -> Result
{
    return imp_->StoreOrUpdate(table, index, cb, parent, flags);
}

auto Database::TransactionRO() const noexcept(false) -> Transaction
{
    return imp_->TransactionRO();
}

auto Database::TransactionRW() const noexcept(false) -> Transaction
{
    return imp_->TransactionRW();
}

auto Database::TransactionRW(Transaction& parent) const noexcept(false)
    -> Transaction
{
    return imp_->TransactionRW(parent);
}

Database::~Database() = default;
}  // namespace opentxs::storage::lmdb
