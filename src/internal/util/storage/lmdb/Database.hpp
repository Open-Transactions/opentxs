// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <filesystem>
#include <functional>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <optional>
#include <tuple>
#include <utility>

#include "internal/util/Mutex.hpp"
#include "internal/util/storage/lmdb/Types.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace storage
{
namespace lmdb
{
class DatabasePrivate;
class Transaction;
}  // namespace lmdb
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::lmdb
{
class Database
{
public:
    auto Commit() const noexcept -> bool;
    auto Delete(const Table table) const noexcept -> bool;
    auto Delete(const Table table, Transaction& parent) const noexcept -> bool;
    auto Delete(const Table table, const ReadView key) const noexcept -> bool;
    auto Delete(const Table table, const ReadView key, Transaction& parent)
        const noexcept -> bool;
    auto Delete(const Table table, const std::size_t key) const noexcept
        -> bool;
    auto Delete(const Table table, const std::size_t key, Transaction& parent)
        const noexcept -> bool;
    auto Delete(const Table table, const std::size_t key, const ReadView value)
        const noexcept -> bool;
    auto Delete(
        const Table table,
        const std::size_t key,
        const ReadView value,
        Transaction& parent) const noexcept -> bool;
    auto Delete(const Table table, const ReadView key, const ReadView value)
        const noexcept -> bool;
    auto Delete(
        const Table table,
        const ReadView key,
        const ReadView value,
        Transaction& parent) const noexcept -> bool;
    auto Exists(const Table table, const ReadView key) const noexcept -> bool;
    auto Exists(const Table table, const ReadView key, Transaction& tx)
        const noexcept -> bool;
    auto Exists(const Table table, const ReadView key, const ReadView value)
        const noexcept -> bool;
    auto Exists(
        const Table table,
        const ReadView key,
        const ReadView value,
        Transaction& tx) const noexcept -> bool;
    auto Load(
        const Table table,
        const ReadView key,
        const Callback cb,
        const Mode mode = Mode::One) const noexcept -> bool;
    auto Load(
        const Table table,
        const ReadView key,
        const Callback cb,
        Transaction& tx,
        const Mode mode = Mode::One) const noexcept -> bool;
    auto Load(
        const Table table,
        const std::size_t key,
        const Callback cb,
        const Mode mode = Mode::One) const noexcept -> bool;
    auto Queue(
        const Table table,
        const ReadView key,
        const ReadView value,
        const Mode mode = Mode::One) const noexcept -> bool;
    auto Read(const Table table, const ReadCallback cb, const Dir dir)
        const noexcept -> bool;
    auto Read(
        const Table table,
        const ReadCallback cb,
        const Dir dir,
        Transaction& parent) const noexcept -> bool;
    auto ReadAndDelete(
        const Table table,
        const ReadCallback cb,
        Transaction& tx,
        const UnallocatedCString& message) const noexcept -> bool;
    auto ReadFrom(
        const Table table,
        const ReadView key,
        const ReadCallback cb,
        const Dir dir) const noexcept -> bool;
    auto ReadFrom(
        const Table table,
        const std::size_t key,
        const ReadCallback cb,
        const Dir dir) const noexcept -> bool;
    auto Store(
        const Table table,
        const ReadView key,
        const ReadView value,
        const Flags flags = 0) const noexcept -> Result;
    auto Store(
        const Table table,
        const ReadView key,
        const ReadView value,
        Transaction& parent,
        const Flags flags = 0) const noexcept -> Result;
    auto Store(
        const Table table,
        const std::size_t key,
        const ReadView value,
        const Flags flags = 0) const noexcept -> Result;
    auto Store(
        const Table table,
        const std::size_t key,
        const ReadView value,
        Transaction& parent,
        const Flags flags = 0) const noexcept -> Result;
    auto StoreOrUpdate(
        const Table table,
        const ReadView key,
        const UpdateCallback cb,
        const Flags flags = 0) const noexcept -> Result;
    auto StoreOrUpdate(
        const Table table,
        const ReadView key,
        const UpdateCallback cb,
        Transaction& parent,
        const Flags flags = 0) const noexcept -> Result;
    auto TransactionRO() const noexcept(false) -> Transaction;
    auto TransactionRW() const noexcept(false) -> Transaction;
    auto TransactionRW(Transaction& parent) const noexcept(false)
        -> Transaction;

    Database(
        const TableNames& names,
        const std::filesystem::path& folder,
        const TablesToInit init,
        const Flags flags = 0,
        const std::size_t extraTables = 0) noexcept;
    Database() = delete;
    Database(const Database&) = delete;
    // NOTE: move constructor is only defined to allow copy elision. It
    // should not be used for any other purpose.
    Database(Database&&) noexcept;
    auto operator=(const Database&) -> Database& = delete;
    auto operator=(Database&&) -> Database& = delete;

    ~Database();

private:
    std::unique_ptr<DatabasePrivate> imp_;
};
}  // namespace opentxs::storage::lmdb
