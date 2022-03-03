// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <functional>
#include <iosfwd>
#include <memory>
#include <mutex>
#include <optional>
#include <tuple>
#include <utility>

#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"

extern "C" {
typedef struct MDB_env MDB_env;
typedef struct MDB_txn MDB_txn;
typedef unsigned int MDB_dbi;
}

namespace opentxs::storage::lmdb
{
using Callback = std::function<void(const ReadView data)>;
using Flags = unsigned int;
using ReadCallback =
    std::function<bool(const ReadView key, const ReadView value)>;
using Result = std::pair<bool, int>;
using Table = int;
using Databases = UnallocatedMap<Table, MDB_dbi>;
using TablesToInit = UnallocatedVector<std::pair<Table, unsigned int>>;
using TableNames = UnallocatedMap<Table, const UnallocatedCString>;
using UpdateCallback = std::function<Space(const ReadView data)>;

class LMDB
{
public:
    enum class Dir : bool { Forward = false, Backward = true };
    enum class Mode : bool { One = false, Multiple = true };

    struct Transaction {
        bool success_;

        operator MDB_txn*() noexcept { return ptr_; }

        auto Finalize(const std::optional<bool> success = {}) noexcept -> bool;

        Transaction(
            MDB_env* env,
            const bool rw,
            std::unique_ptr<Lock> lock,
            MDB_txn* parent = nullptr) noexcept(false);
        Transaction(Transaction&&) noexcept;

        ~Transaction();

    private:
        std::unique_ptr<Lock> lock_;
        MDB_txn* ptr_;

        Transaction(const Transaction&) = delete;
        auto operator=(const Transaction&) -> Transaction& = delete;
        auto operator=(Transaction&&) -> Transaction& = delete;
    };

    auto Commit() const noexcept -> bool;
    auto Delete(const Table table, MDB_txn* parent = nullptr) const noexcept
        -> bool;
    auto Delete(
        const Table table,
        const std::size_t key,
        MDB_txn* parent = nullptr) const noexcept -> bool;
    auto Delete(
        const Table table,
        const ReadView key,
        MDB_txn* parent = nullptr) const noexcept -> bool;
    auto Delete(
        const Table table,
        const std::size_t key,
        const ReadView value,
        MDB_txn* parent = nullptr) const noexcept -> bool;
    auto Delete(
        const Table table,
        const ReadView key,
        const ReadView value,
        MDB_txn* parent = nullptr) const noexcept -> bool;
    auto Exists(const Table table, const ReadView key) const noexcept -> bool;
    auto Exists(const Table table, const ReadView key, const ReadView value)
        const noexcept -> bool;
    auto Load(
        const Table table,
        const ReadView key,
        const Callback cb,
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
    auto ReadAndDelete(
        const Table table,
        const ReadCallback cb,
        MDB_txn& tx,
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
        MDB_txn* parent = nullptr,
        const Flags flags = 0) const noexcept -> Result;
    auto Store(
        const Table table,
        const std::size_t key,
        const ReadView value,
        MDB_txn* parent = nullptr,
        const Flags flags = 0) const noexcept -> Result;
    auto StoreOrUpdate(
        const Table table,
        const ReadView key,
        const UpdateCallback cb,
        MDB_txn* parent = nullptr,
        const Flags flags = 0) const noexcept -> Result;
    auto TransactionRO() const noexcept(false) -> Transaction;
    auto TransactionRW(MDB_txn* parent = nullptr) const noexcept(false)
        -> Transaction;

    LMDB(
        const TableNames& names,
        const UnallocatedCString& folder,
        const TablesToInit init,
        const Flags flags = 0,
        const std::size_t extraTables = 0)
    noexcept;
    // NOTE: move constructor is only defined to allow copy elision. It
    // should not be used for any other purpose.
    LMDB(LMDB&&) noexcept;
    ~LMDB();

private:
    struct Imp;

    std::unique_ptr<Imp> imp_;

    auto read(const MDB_dbi dbi, const ReadCallback cb, const Dir dir)
        const noexcept -> bool;

    LMDB() = delete;
    LMDB(const LMDB&) = delete;
    auto operator=(const LMDB&) -> LMDB& = delete;
    auto operator=(LMDB&&) -> LMDB& = delete;
};
}  // namespace opentxs::storage::lmdb
