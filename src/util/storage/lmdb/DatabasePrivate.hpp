// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cs_plain_guarded.h>
#include <cstddef>
#include <filesystem>
#include <memory>
#include <tuple>
#include <variant>

#include "internal/util/storage/lmdb/Database.hpp"
#include "internal/util/storage/lmdb/Types.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

extern "C" {
using MDB_env = struct MDB_env;
}

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs  // NOLINT
{
// inline namespace v1
// {
namespace storage
{
namespace lmdb
{
class Transaction;
}  // namespace lmdb
}  // namespace storage
// }  // namespace v1
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::lmdb
{
// NOTE these types model the locking behavior of lmdb:
// * readers do not block writers, writers do not block readers
// * write locks can be nested
//
// WriteHandles are only constructed for write Transactions. They are shared
// pointers to allow for child Transactions to hold a copy of the same handle.
//
// If writers blocked readers or vice versa then we would use
// libguarded::shared_guarded<MDB_env*>
using GuardedWrite = libguarded::plain_guarded<std::monostate>;
using WriteHandle = std::shared_ptr<GuardedWrite::handle>;

class DatabasePrivate
{
public:
    auto Commit() const noexcept -> bool;
    auto Delete(const Table table) const noexcept -> bool;
    auto Delete(const Table table, Transaction& parent) const noexcept -> bool;
    auto Delete(const Table table, const ReadView index) const noexcept -> bool;
    auto Delete(const Table table, const ReadView index, Transaction& parent)
        const noexcept -> bool;
    auto Delete(const Table table, const std::size_t index) const noexcept
        -> bool;
    auto Delete(const Table table, const std::size_t index, Transaction& parent)
        const noexcept -> bool;
    auto Delete(const Table table, const std::size_t index, const ReadView data)
        const noexcept -> bool;
    auto Delete(
        const Table table,
        const std::size_t index,
        const ReadView data,
        Transaction& parent) const noexcept -> bool;
    auto Delete(
        const Table table,
        const ReadView index,
        const ReadView data,
        Transaction& parent) const noexcept -> bool;
    auto Delete(const Table table, const ReadView index, const ReadView data)
        const noexcept -> bool;
    auto Exists(const Table table, const ReadView index, const ReadView item)
        const noexcept -> bool;
    auto Exists(
        const Table table,
        const ReadView index,
        const ReadView item,
        Transaction& tx) const noexcept -> bool;
    auto Load(
        const Table table,
        const ReadView index,
        const Callback cb,
        const Mode multiple) const noexcept -> bool;
    auto Load(
        const Table table,
        const ReadView index,
        const Callback cb,
        const Mode multiple,
        Transaction& tx) const noexcept -> bool;
    auto Queue(
        const Table table,
        const ReadView key,
        const ReadView value,
        const Mode mode) const noexcept -> bool;
    auto Read(const Table table, const ReadCallback cb, const Dir dir)
        const noexcept -> bool;
    auto Read(
        const Table table,
        const ReadCallback cb,
        const Dir dir,
        Transaction& tx) const noexcept -> bool;
    auto ReadAndDelete(
        const Table table,
        const ReadCallback cb,
        Transaction& tx,
        const UnallocatedCString& message) const noexcept -> bool;
    auto ReadFrom(
        const Table table,
        const ReadView index,
        const ReadCallback cb,
        const Dir dir) const noexcept -> bool;
    auto Store(
        const Table table,
        const ReadView index,
        const ReadView data,
        Transaction& parent,
        const Flags flags) const noexcept -> Result;
    auto Store(
        const Table table,
        const ReadView index,
        const ReadView data,
        const Flags flags) const noexcept -> Result;
    auto StoreOrUpdate(
        const Table table,
        const ReadView index,
        const UpdateCallback cb,
        const Flags flags) const noexcept -> Result;
    auto StoreOrUpdate(
        const Table table,
        const ReadView index,
        const UpdateCallback cb,
        Transaction& parent,
        const Flags flags) const noexcept -> Result;
    auto TransactionRO() const noexcept(false) -> Transaction;
    auto TransactionRW() const noexcept(false) -> Transaction;
    auto TransactionRW(Transaction& parent) const noexcept(false)
        -> Transaction;

    DatabasePrivate(
        const TableNames& names,
        const std::filesystem::path& folder,
        const TablesToInit init,
        const Flags flags,
        const std::size_t extraTables) noexcept;
    DatabasePrivate(const DatabasePrivate&) = delete;
    DatabasePrivate(DatabasePrivate&&) = delete;
    auto operator=(const DatabasePrivate&) -> DatabasePrivate& = delete;
    auto operator=(DatabasePrivate&&) -> DatabasePrivate& = delete;

    ~DatabasePrivate();

private:
    using NewKey =
        std::tuple<Table, Mode, UnallocatedCString, UnallocatedCString>;
    using Pending = UnallocatedVector<NewKey>;
    using GuardedPending = libguarded::plain_guarded<Pending>;

    const TableNames& names_;
    mutable MDB_env* env_;
    mutable Databases db_;
    mutable GuardedPending pending_;
    mutable GuardedWrite write_;

    auto read(
        const MDB_dbi dbi,
        const ReadCallback cb,
        const Dir dir,
        Transaction& tx) const noexcept -> bool;

    auto close_env() -> void;
    auto init_db(const Table table, unsigned int flags) noexcept -> MDB_dbi;
    auto init_environment(
        const std::filesystem::path& folder,
        const std::size_t tables,
        const Flags flags) noexcept -> void;
    auto init_tables(const TablesToInit init) noexcept -> void;
};
}  // namespace opentxs::storage::lmdb
