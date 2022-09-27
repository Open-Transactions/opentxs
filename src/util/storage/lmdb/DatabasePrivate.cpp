// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"                           // IWYU pragma: associated
#include "1_Internal.hpp"                         // IWYU pragma: associated
#include "util/storage/lmdb/DatabasePrivate.hpp"  // IWYU pragma: associated

extern "C" {
#include <lmdb.h>
}

#include <cstddef>
#include <limits>
#include <stdexcept>
#include <utility>

#include "internal/util/LogMacros.hpp"
#include "internal/util/storage/lmdb/Transaction.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "util/FileSize.hpp"
#include "util/ScopeGuard.hpp"
#include "util/storage/lmdb/TransactionPrivate.hpp"

namespace opentxs::storage::lmdb
{
DatabasePrivate::DatabasePrivate(
    const TableNames& names,
    const std::filesystem::path& folder,
    const TablesToInit init,
    const Flags flags,
    const std::size_t extraTables) noexcept
    : names_(names)
    , env_(nullptr)
    , db_()
    , pending_()
    , write_()
{
    init_environment(folder, init.size() + extraTables, flags);
    init_tables(init);
}

auto DatabasePrivate::close_env() -> void
{
    if (nullptr != env_) {
        ::mdb_env_close(env_);
        env_ = nullptr;
    }
}

auto DatabasePrivate::Commit() const noexcept -> bool
{
    try {
        auto tx = TransactionRW();
        auto handle = pending_.lock();
        auto& pending = *handle;
        auto post = ScopeGuard{[&] { pending.clear(); }};

        for (auto& [table, mode, index, data] : pending) {
            auto dbi = db_.at(table);
            auto key = MDB_val{index.size(), const_cast<char*>(index.data())};
            auto value = MDB_val{data.size(), const_cast<char*>(data.data())};

            if (const auto rc = ::mdb_put(*tx.imp_, dbi, &key, &value, 0);
                0 != rc) {
                throw std::runtime_error{::mdb_strerror(rc)};
            } else {
                tx.SetSuccess(true);
            }
        }

        return tx.Success();
    } catch (const std::exception& e) {
        LogTrace()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto DatabasePrivate::Delete(const Table table) const noexcept -> bool
{
    auto parent = Transaction{};

    return Delete(table, parent);
}

auto DatabasePrivate::Delete(const Table table, Transaction& parent)
    const noexcept -> bool
{
    try {
        auto tx = TransactionRW(parent);
        const auto dbi = db_.at(table);

        if (const auto rc = ::mdb_drop(*tx.imp_, dbi, 0); 0 != rc) {
            throw std::runtime_error{::mdb_strerror(rc)};
        } else {
            tx.SetSuccess(true);
        }

        return tx.Success();
    } catch (const std::exception& e) {
        LogTrace()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto DatabasePrivate::Delete(const Table table, const ReadView index)
    const noexcept -> bool
{
    auto parent = Transaction{};

    return Delete(table, index, parent);
}

auto DatabasePrivate::Delete(
    const Table table,
    const ReadView index,
    Transaction& parent) const noexcept -> bool
{
    try {
        auto tx = TransactionRW(parent);
        const auto dbi = db_.at(table);
        auto key = MDB_val{index.size(), const_cast<char*>(index.data())};

        if (const auto rc = ::mdb_del(*tx.imp_, dbi, &key, nullptr); 0 != rc) {
            throw std::runtime_error{::mdb_strerror(rc)};
        } else {
            tx.SetSuccess(true);
        }

        return tx.Success();
    } catch (const std::exception& e) {
        LogTrace()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto DatabasePrivate::Delete(const Table table, const std::size_t index)
    const noexcept -> bool
{
    auto parent = Transaction{};

    return Delete(table, index, parent);
}

auto DatabasePrivate::Delete(
    const Table table,
    const std::size_t index,
    Transaction& parent) const noexcept -> bool
{
    return Delete(
        table,
        ReadView{reinterpret_cast<const char*>(&index), sizeof(index)},
        parent);
}

auto DatabasePrivate::Delete(
    const Table table,
    const std::size_t index,
    const ReadView data) const noexcept -> bool
{
    auto parent = Transaction{};

    return Delete(table, index, data, parent);
}

auto DatabasePrivate::Delete(
    const Table table,
    const std::size_t index,
    const ReadView data,
    Transaction& parent) const noexcept -> bool
{
    return Delete(
        table,
        ReadView{reinterpret_cast<const char*>(&index), sizeof(index)},
        data,
        parent);
}

auto DatabasePrivate::Delete(
    const Table table,
    const ReadView index,
    const ReadView data) const noexcept -> bool
{
    auto parent = Transaction{};

    return Delete(table, index, data, parent);
}

auto DatabasePrivate::Delete(
    const Table table,
    const ReadView index,
    const ReadView data,
    Transaction& parent) const noexcept -> bool
{
    try {
        auto tx = TransactionRW(parent);
        const auto dbi = db_.at(table);
        auto key = MDB_val{index.size(), const_cast<char*>(index.data())};
        auto value = MDB_val{data.size(), const_cast<char*>(data.data())};

        if (const auto rc = ::mdb_del(*tx.imp_, dbi, &key, &value); 0 != rc) {
            throw std::runtime_error{::mdb_strerror(rc)};
        } else {
            tx.SetSuccess(true);
        }

        return tx.Success();
    } catch (const std::exception& e) {
        LogTrace()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto DatabasePrivate::Exists(
    const Table table,
    const ReadView index,
    const ReadView item) const noexcept -> bool
{
    auto tx = TransactionRO();

    return Exists(table, index, item, tx);
}

auto DatabasePrivate::Exists(
    const Table table,
    const ReadView index,
    const ReadView item,
    Transaction& tx) const noexcept -> bool
{
    try {
        MDB_cursor* cursor{nullptr};
        auto post = ScopeGuard{[&] {
            if (nullptr != cursor) {
                ::mdb_cursor_close(cursor);
                cursor = nullptr;
            }
        }};
        const auto dbi = db_.at(table);

        if (0 != ::mdb_cursor_open(*tx.imp_, dbi, &cursor)) {
            throw std::runtime_error{"Failed to get cursor"};
        }

        auto key = MDB_val{index.size(), const_cast<char*>(index.data())};
        auto value = [&] {
            if (valid(item)) {

                return MDB_val{item.size(), const_cast<char*>(item.data())};
            } else {

                return MDB_val{};
            }
        }();

        return 0 == ::mdb_cursor_get(cursor, &key, &value, MDB_SET);
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto DatabasePrivate::init_db(const Table table, unsigned int flags) noexcept
    -> MDB_dbi
{
    MDB_txn* transaction{nullptr};
    auto status = (0 == ::mdb_txn_begin(env_, nullptr, 0, &transaction));

    OT_ASSERT(status);
    OT_ASSERT(nullptr != transaction);

    auto output = MDB_dbi{};
    status =
        0 ==
        ::mdb_dbi_open(
            transaction, names_.at(table).c_str(), MDB_CREATE | flags, &output);
    if (!static_cast<bool>(status)) {  // free memory allocated in
                                       // mdb_txn_begin - transaction
        ::mdb_txn_abort(transaction);
    }
    OT_ASSERT(status);

    ::mdb_txn_commit(transaction);  // free memory allocated in
                                    // mdb_txn_begin - transaction

    return output;
}

auto DatabasePrivate::init_environment(
    const std::filesystem::path& folder,
    const std::size_t tables,
    const Flags flags) noexcept -> void
{
    OT_ASSERT(std::numeric_limits<unsigned int>::max() >= tables);

    auto rc = ::mdb_env_create(&env_);
    bool set = 0 == rc;
    if (!set) {
        LogConsole()("failed to mdb_env_create: ")(::mdb_strerror(rc)).Flush();
        close_env();
    }

    OT_ASSERT(set);
    OT_ASSERT(nullptr != env_);

    rc = ::mdb_env_set_mapsize(env_, db_file_size());
    set = 0 == rc;
    if (!set) {
        LogConsole()("failed to set mapsize: ")(::mdb_strerror(rc)).Flush();
        close_env();
    }

    OT_ASSERT(set);

    rc = ::mdb_env_set_maxdbs(env_, static_cast<unsigned int>(tables));
    set = 0 == rc;
    if (!set) {
        LogConsole()("failed to set maxdbs: ")(::mdb_strerror(rc)).Flush();
        close_env();
    }

    OT_ASSERT(set);

    rc = ::mdb_env_set_maxreaders(env_, 1024u);
    set = 0 == rc;
    if (!set) {
        LogConsole()("failed to set maxreaders: ")(::mdb_strerror(rc)).Flush();
        close_env();
    }

    OT_ASSERT(set);

    rc = ::mdb_env_open(env_, folder.string().c_str(), flags, 0664);
    set = 0 == rc;
    if (!set) {
        LogConsole()("failed to open: ")(folder.string().c_str())(" flags: ")(
            flags)(" reason: ")(::mdb_strerror(rc))
            .Flush();
        close_env();
    }

    OT_ASSERT(set);
}

auto DatabasePrivate::init_tables(const TablesToInit init) noexcept -> void
{
    for (const auto& [table, flags] : init) {
        db_.emplace(table, init_db(table, flags));
    }
}

auto DatabasePrivate::Load(
    const Table table,
    const ReadView index,
    const Callback cb,
    const Mode multiple) const noexcept -> bool
{
    auto tx = TransactionRO();

    return Load(table, index, cb, multiple, tx);
}

auto DatabasePrivate::Load(
    const Table table,
    const ReadView index,
    const Callback cb,
    const Mode multiple,
    Transaction& tx) const noexcept -> bool
{
    try {
        MDB_cursor* cursor{nullptr};
        auto post = ScopeGuard{[&] {
            if (nullptr != cursor) {
                ::mdb_cursor_close(cursor);
                cursor = nullptr;
            }
        }};
        const auto dbi = db_.at(table);

        if (0 != ::mdb_cursor_open(*tx.imp_, dbi, &cursor)) {
            throw std::runtime_error{"Failed to get cursor"};
        }

        auto key = MDB_val{index.size(), const_cast<char*>(index.data())};
        auto value = MDB_val{};
        auto success = 0 == ::mdb_cursor_get(cursor, &key, &value, MDB_SET);

        if (success) {
            ::mdb_cursor_get(cursor, &key, &value, MDB_FIRST_DUP);

            if (0 == ::mdb_cursor_get(cursor, &key, &value, MDB_GET_CURRENT)) {
                cb({static_cast<char*>(value.mv_data), value.mv_size});
            } else {

                return false;
            }

            if (static_cast<bool>(multiple)) {
                while (0 ==
                       ::mdb_cursor_get(cursor, &key, &value, MDB_NEXT_DUP)) {
                    if (0 == ::mdb_cursor_get(
                                 cursor, &key, &value, MDB_GET_CURRENT)) {
                        cb({static_cast<char*>(value.mv_data), value.mv_size});
                    } else {

                        return false;
                    }
                }
            }
        }

        return success;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto DatabasePrivate::Queue(
    const Table table,
    const ReadView key,
    const ReadView value,
    const Mode mode) const noexcept -> bool
{
    pending_.lock()->emplace_back(NewKey{table, mode, key, value});

    return true;
}

auto DatabasePrivate::Read(
    const Table table,
    const ReadCallback cb,
    const Dir dir) const noexcept -> bool
{
    auto tx = TransactionRO();

    return read(db_.at(table), cb, dir, tx);
}

auto DatabasePrivate::Read(
    const Table table,
    const ReadCallback cb,
    const Dir dir,
    Transaction& tx) const noexcept -> bool
{
    return read(db_.at(table), cb, dir, tx);
}

auto DatabasePrivate::read(
    const MDB_dbi dbi,
    const ReadCallback cb,
    const Dir dir,
    Transaction& tx) const noexcept -> bool
{
    try {
        MDB_cursor* cursor{nullptr};
        auto post = ScopeGuard{[&] {
            if (nullptr != cursor) {
                ::mdb_cursor_close(cursor);
                cursor = nullptr;
            }
        }};

        if (0 != ::mdb_cursor_open(*tx.imp_, dbi, &cursor)) {
            throw std::runtime_error{"Failed to get cursor"};
        }

        const auto start =
            MDB_cursor_op{(Dir::Forward == dir) ? MDB_FIRST : MDB_LAST};
        const auto next =
            MDB_cursor_op{(Dir::Forward == dir) ? MDB_NEXT : MDB_PREV};
        auto again{true};
        auto key = MDB_val{};
        auto value = MDB_val{};
        auto success = 0 == ::mdb_cursor_get(cursor, &key, &value, start);

        if (success) {
            if (0 == ::mdb_cursor_get(cursor, &key, &value, MDB_GET_CURRENT)) {
                again =
                    cb({static_cast<char*>(key.mv_data), key.mv_size},
                       {static_cast<char*>(value.mv_data), value.mv_size});
            } else {

                return false;
            }

            while (again && 0 == ::mdb_cursor_get(cursor, &key, &value, next)) {
                if (0 ==
                    ::mdb_cursor_get(cursor, &key, &value, MDB_GET_CURRENT)) {
                    again =
                        cb({static_cast<char*>(key.mv_data), key.mv_size},
                           {static_cast<char*>(value.mv_data), value.mv_size});
                } else {

                    return false;
                }
            }
        } else {
            // NOTE table is empty

            return true;
        }

        return success;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto DatabasePrivate::ReadAndDelete(
    const Table table,
    const ReadCallback cb,
    Transaction& tx,
    const UnallocatedCString& message) const noexcept -> bool
{
    auto dbi = MDB_dbi{};

    if (0 != ::mdb_dbi_open(*tx.imp_, names_.at(table).c_str(), 0, &dbi)) {
        LogTrace()(OT_PRETTY_CLASS())("table does not exist").Flush();

        return false;
    }

    LogConsole()("Beginning database upgrade for ")(message).Flush();
    read(dbi, cb, Dir::Forward, tx);

    if (0 != ::mdb_drop(*tx.imp_, dbi, 1)) {
        LogError()(OT_PRETTY_CLASS())("Failed to delete table").Flush();

        return false;
    }

    LogConsole()("Finished database upgrade for ")(message).Flush();

    return true;
}

auto DatabasePrivate::ReadFrom(
    const Table table,
    const ReadView index,
    const ReadCallback cb,
    const Dir dir) const noexcept -> bool
{
    try {
        auto tx = TransactionRO();
        MDB_cursor* cursor{nullptr};
        auto post = ScopeGuard{[&] {
            if (nullptr != cursor) {
                ::mdb_cursor_close(cursor);
                cursor = nullptr;
            }
        }};
        const auto dbi = db_.at(table);

        if (0 != ::mdb_cursor_open(*tx.imp_, dbi, &cursor)) {
            throw std::runtime_error{"Failed to get cursor"};
        }

        const auto next =
            MDB_cursor_op{(Dir::Forward == dir) ? MDB_NEXT : MDB_PREV};
        auto again{true};
        auto key = MDB_val{index.size(), const_cast<char*>(index.data())};
        auto value = MDB_val{};
        auto success = 0 == ::mdb_cursor_get(cursor, &key, &value, MDB_SET);

        if (success) {
            if (0 == ::mdb_cursor_get(cursor, &key, &value, MDB_GET_CURRENT)) {
                again =
                    cb({static_cast<char*>(key.mv_data), key.mv_size},
                       {static_cast<char*>(value.mv_data), value.mv_size});
            } else {

                return false;
            }

            while (again && 0 == ::mdb_cursor_get(cursor, &key, &value, next)) {
                if (0 ==
                    ::mdb_cursor_get(cursor, &key, &value, MDB_GET_CURRENT)) {
                    again =
                        cb({static_cast<char*>(key.mv_data), key.mv_size},
                           {static_cast<char*>(value.mv_data), value.mv_size});
                } else {

                    return false;
                }
            }
        }

        return success;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto DatabasePrivate::Store(
    const Table table,
    const ReadView index,
    const ReadView data,
    const Flags flags) const noexcept -> Result
{
    auto parent = Transaction{};

    return Store(table, index, data, parent, flags);
}

auto DatabasePrivate::Store(
    const Table table,
    const ReadView index,
    const ReadView data,
    Transaction& parent,
    const Flags flags) const noexcept -> Result
{
    auto output = Result{false, MDB_LAST_ERRCODE};
    auto& [success, code] = output;
    auto tx = TransactionRW(parent);
    auto post = ScopeGuard{[&] { tx.SetSuccess(output.first); }};
    const auto dbi = db_.at(table);
    auto key = MDB_val{index.size(), const_cast<char*>(index.data())};
    auto value = MDB_val{data.size(), const_cast<char*>(data.data())};
    code = ::mdb_put(*tx.imp_, dbi, &key, &value, flags);
    success = 0 == code;

    return output;
}

auto DatabasePrivate::StoreOrUpdate(
    const Table table,
    const ReadView index,
    const UpdateCallback cb,
    const Flags flags) const noexcept -> Result
{
    auto parent = Transaction{};

    return StoreOrUpdate(table, index, cb, parent, flags);
}

auto DatabasePrivate::StoreOrUpdate(
    const Table table,
    const ReadView index,
    const UpdateCallback cb,
    Transaction& parent,
    const Flags flags) const noexcept -> Result
{
    auto output = Result{false, MDB_LAST_ERRCODE};

    try {
        if (false == bool(cb)) { throw std::runtime_error{"Invalid callback"}; }

        auto tx = TransactionRW(parent);
        MDB_cursor* cursor{nullptr};
        auto post = ScopeGuard{[&] {
            if (nullptr != cursor) {
                ::mdb_cursor_close(cursor);
                cursor = nullptr;
            }

            tx.SetSuccess(output.first);
        }};
        auto& [success, code] = output;
        const auto dbi = db_.at(table);

        if (0 != ::mdb_cursor_open(*tx.imp_, dbi, &cursor)) {
            throw std::runtime_error{"Failed to get cursor"};
        }

        auto key = MDB_val{index.size(), const_cast<char*>(index.data())};
        auto value = MDB_val{};
        const auto exists =
            0 == ::mdb_cursor_get(cursor, &key, &value, MDB_SET_KEY);
        const auto previous =
            exists
                ? ReadView{static_cast<const char*>(value.mv_data), value.mv_size}
                : ReadView{};
        const auto bytes = cb(previous);
        auto replace =
            MDB_val{bytes.size(), const_cast<std::byte*>(bytes.data())};
        code = ::mdb_put(*tx.imp_, dbi, &key, &replace, flags);
        success = 0 == code;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
    }

    return output;
}

auto DatabasePrivate::TransactionRO() const noexcept(false) -> Transaction
{
    return std::make_unique<TransactionPrivate>(env_);
}

auto DatabasePrivate::TransactionRW() const noexcept(false) -> Transaction
{
    auto parent = Transaction{};

    return TransactionRW(parent);
}

auto DatabasePrivate::TransactionRW(Transaction& parent) const noexcept(false)
    -> Transaction
{
    if (auto& write = parent.imp_->write_; false == write.operator bool()) {
        write = std::make_shared<GuardedWrite::handle>(write_.lock());
    }

    return std::make_unique<TransactionPrivate>(env_, parent);
}

DatabasePrivate::~DatabasePrivate() { close_env(); }
}  // namespace opentxs::storage::lmdb
