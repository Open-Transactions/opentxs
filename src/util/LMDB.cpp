// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"    // IWYU pragma: associated
#include "1_Internal.hpp"  // IWYU pragma: associated
#include "util/LMDB.hpp"   // IWYU pragma: associated

extern "C" {
#include <lmdb.h>
}

#include <cstddef>
#include <limits>
#include <mutex>
#include <stdexcept>
#include <tuple>

#include "internal/util/LogMacros.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/util/Log.hpp"
#include "util/FileSize.hpp"
#include "util/ScopeGuard.hpp"

namespace opentxs::storage::lmdb
{
struct LMDB::Imp {
    auto Commit() const noexcept -> bool
    {
        try {
            auto lock = Lock{pending_lock_};
            auto tx = TransactionRW(nullptr);
            auto& success = tx.success_;
            auto post = ScopeGuard{[&] { pending_.clear(); }};

            for (auto& [table, mode, index, data] : pending_) {
                auto dbi = db_.at(table);
                auto key =
                    MDB_val{index.size(), const_cast<char*>(index.data())};
                auto value =
                    MDB_val{data.size(), const_cast<char*>(data.data())};

                if (const auto rc = ::mdb_put(tx, dbi, &key, &value, 0);
                    0 != rc) {
                    throw std::runtime_error{::mdb_strerror(rc)};
                } else {
                    success = true;
                }
            }

            return success;
        } catch (const std::exception& e) {
            LogTrace()(OT_PRETTY_CLASS())(e.what()).Flush();

            return false;
        }
    }
    auto Delete(const Table table, MDB_txn* parent) const noexcept -> bool
    {
        try {
            auto tx = TransactionRW(parent);
            auto& success = tx.success_;
            const auto dbi = db_.at(table);

            if (const auto rc = ::mdb_drop(tx, dbi, 0); 0 != rc) {
                throw std::runtime_error{::mdb_strerror(rc)};
            } else {
                success = true;
            }

            return success;
        } catch (const std::exception& e) {
            LogTrace()(OT_PRETTY_CLASS())(e.what()).Flush();

            return false;
        }
    }
    auto Delete(const Table table, const ReadView index, MDB_txn* parent)
        const noexcept -> bool
    {
        try {
            auto tx = TransactionRW(parent);
            auto& success = tx.success_;
            const auto dbi = db_.at(table);
            auto key = MDB_val{index.size(), const_cast<char*>(index.data())};

            if (const auto rc = ::mdb_del(tx, dbi, &key, nullptr); 0 != rc) {
                throw std::runtime_error{::mdb_strerror(rc)};
            } else {
                success = true;
            }

            return success;
        } catch (const std::exception& e) {
            LogTrace()(OT_PRETTY_CLASS())(e.what()).Flush();

            return false;
        }
    }
    auto Delete(const Table table, const std::size_t index, MDB_txn* parent)
        const noexcept -> bool
    {
        return Delete(
            table,
            ReadView{reinterpret_cast<const char*>(&index), sizeof(index)},
            parent);
    }
    auto Delete(
        const Table table,
        const std::size_t index,
        const ReadView data,
        MDB_txn* parent) const noexcept -> bool
    {
        return Delete(
            table,
            ReadView{reinterpret_cast<const char*>(&index), sizeof(index)},
            data,
            parent);
    }
    auto Delete(
        const Table table,
        const ReadView index,
        const ReadView data,
        MDB_txn* parent) const noexcept -> bool
    {
        try {
            auto tx = TransactionRW(parent);
            auto& success = tx.success_;
            const auto dbi = db_.at(table);
            auto key = MDB_val{index.size(), const_cast<char*>(index.data())};
            auto value = MDB_val{data.size(), const_cast<char*>(data.data())};

            if (const auto rc = ::mdb_del(tx, dbi, &key, &value); 0 != rc) {
                throw std::runtime_error{::mdb_strerror(rc)};
            } else {
                success = true;
            }

            return success;
        } catch (const std::exception& e) {
            LogTrace()(OT_PRETTY_CLASS())(e.what()).Flush();

            return false;
        }
    }
    auto Exists(const Table table, const ReadView index, const ReadView item)
        const noexcept -> bool
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

            if (0 != ::mdb_cursor_open(tx, dbi, &cursor)) {
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
    auto Load(
        const Table table,
        const ReadView index,
        const Callback cb,
        const Mode multiple) const noexcept -> bool
    {
        auto tx = TransactionRO();

        return Load(table, index, cb, multiple, tx);
    }
    auto Load(
        const Table table,
        const ReadView index,
        const Callback cb,
        const Mode multiple,
        MDB_txn* tx) const noexcept -> bool
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

            if (0 != ::mdb_cursor_open(tx, dbi, &cursor)) {
                throw std::runtime_error{"Failed to get cursor"};
            }

            auto key = MDB_val{index.size(), const_cast<char*>(index.data())};
            auto value = MDB_val{};
            auto success = 0 == ::mdb_cursor_get(cursor, &key, &value, MDB_SET);

            if (success) {
                ::mdb_cursor_get(cursor, &key, &value, MDB_FIRST_DUP);

                if (0 ==
                    ::mdb_cursor_get(cursor, &key, &value, MDB_GET_CURRENT)) {
                    cb({static_cast<char*>(value.mv_data), value.mv_size});
                } else {

                    return false;
                }

                if (static_cast<bool>(multiple)) {
                    while (0 == ::mdb_cursor_get(
                                    cursor, &key, &value, MDB_NEXT_DUP)) {
                        if (0 == ::mdb_cursor_get(
                                     cursor, &key, &value, MDB_GET_CURRENT)) {
                            cb({static_cast<char*>(value.mv_data),
                                value.mv_size});
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
    auto Queue(
        const Table table,
        const ReadView key,
        const ReadView value,
        const Mode mode) const noexcept -> bool
    {
        auto lock = Lock{pending_lock_};
        pending_.emplace_back(NewKey{table, mode, key, value});

        return true;
    }
    auto Read(const Table table, const ReadCallback cb, const Dir dir)
        const noexcept -> bool
    {
        return read(db_.at(table), cb, dir);
    }

    auto read(const MDB_dbi dbi, const ReadCallback cb, const Dir dir)
        const noexcept -> bool
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

            if (0 != ::mdb_cursor_open(tx, dbi, &cursor)) {
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
                if (0 ==
                    ::mdb_cursor_get(cursor, &key, &value, MDB_GET_CURRENT)) {
                    again =
                        cb({static_cast<char*>(key.mv_data), key.mv_size},
                           {static_cast<char*>(value.mv_data), value.mv_size});
                } else {

                    return false;
                }

                while (again &&
                       0 == ::mdb_cursor_get(cursor, &key, &value, next)) {
                    if (0 == ::mdb_cursor_get(
                                 cursor, &key, &value, MDB_GET_CURRENT)) {
                        again = cb(
                            {static_cast<char*>(key.mv_data), key.mv_size},
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
    auto ReadAndDelete(
        const Table table,
        const ReadCallback cb,
        MDB_txn& tx,
        const UnallocatedCString& message) const noexcept -> bool
    {
        auto dbi = MDB_dbi{};

        if (0 != ::mdb_dbi_open(&tx, names_.at(table).c_str(), 0, &dbi)) {
            LogTrace()(OT_PRETTY_CLASS())("table does not exist").Flush();

            return false;
        }

        LogConsole()("Beginning database upgrade for ")(message).Flush();
        read(dbi, cb, Dir::Forward);

        if (0 != ::mdb_drop(&tx, dbi, 1)) {
            LogError()(OT_PRETTY_CLASS())("Failed to delete table").Flush();

            return false;
        }

        LogConsole()("Finished database upgrade for ")(message).Flush();

        return true;
    }
    auto ReadFrom(
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

            if (0 != ::mdb_cursor_open(tx, dbi, &cursor)) {
                throw std::runtime_error{"Failed to get cursor"};
            }

            const auto next =
                MDB_cursor_op{(Dir::Forward == dir) ? MDB_NEXT : MDB_PREV};
            auto again{true};
            auto key = MDB_val{index.size(), const_cast<char*>(index.data())};
            auto value = MDB_val{};
            auto success = 0 == ::mdb_cursor_get(cursor, &key, &value, MDB_SET);

            if (success) {
                if (0 ==
                    ::mdb_cursor_get(cursor, &key, &value, MDB_GET_CURRENT)) {
                    again =
                        cb({static_cast<char*>(key.mv_data), key.mv_size},
                           {static_cast<char*>(value.mv_data), value.mv_size});
                } else {

                    return false;
                }

                while (again &&
                       0 == ::mdb_cursor_get(cursor, &key, &value, next)) {
                    if (0 == ::mdb_cursor_get(
                                 cursor, &key, &value, MDB_GET_CURRENT)) {
                        again = cb(
                            {static_cast<char*>(key.mv_data), key.mv_size},
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
    auto Store(
        const Table table,
        const ReadView index,
        const ReadView data,
        MDB_txn* parent,
        const Flags flags) const noexcept -> Result
    {
        auto output = Result{false, MDB_LAST_ERRCODE};
        auto& [success, code] = output;
        auto transaction = TransactionRW(parent);
        auto post = ScopeGuard{[&] { transaction.success_ = output.first; }};
        const auto dbi = db_.at(table);
        auto key = MDB_val{index.size(), const_cast<char*>(index.data())};
        auto value = MDB_val{data.size(), const_cast<char*>(data.data())};
        code = ::mdb_put(transaction, dbi, &key, &value, flags);
        success = 0 == code;

        return output;
    }
    auto StoreOrUpdate(
        const Table table,
        const ReadView index,
        const UpdateCallback cb,
        MDB_txn* parent,
        const Flags flags) const noexcept -> Result
    {
        auto output = Result{false, MDB_LAST_ERRCODE};

        try {
            if (false == bool(cb)) {
                throw std::runtime_error{"Invalid callback"};
            }

            auto tx = TransactionRW(parent);
            MDB_cursor* cursor{nullptr};
            auto post = ScopeGuard{[&] {
                if (nullptr != cursor) {
                    ::mdb_cursor_close(cursor);
                    cursor = nullptr;
                }

                tx.success_ = output.first;
            }};
            auto& [success, code] = output;
            const auto dbi = db_.at(table);

            if (0 != ::mdb_cursor_open(tx, dbi, &cursor)) {
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
            code = ::mdb_put(tx, dbi, &key, &replace, flags);
            success = 0 == code;
        } catch (const std::exception& e) {
            LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
        }

        return output;
    }
    auto TransactionRO() const noexcept(false) -> Transaction
    {
        return {env_, false, nullptr};
    }
    auto TransactionRW(MDB_txn* parent) const noexcept(false) -> Transaction
    {
        return {
            env_,
            true,
            (nullptr == parent) ? std::make_unique<Lock>(write_lock_)
                                : std::make_unique<Lock>(),
            parent};
    }

    Imp(const TableNames& names,
        const UnallocatedCString& folder,
        const TablesToInit init,
        const Flags flags,
        const std::size_t extraTables) noexcept
        : names_(names)
        , env_(nullptr)
        , db_()
        , pending_()
        , pending_lock_()
        , write_lock_()
    {
        init_environment(folder, init.size() + extraTables, flags);
        init_tables(init);
    }
    Imp(const Imp&) = delete;
    Imp(Imp&&) = delete;
    auto operator=(const Imp&) -> Imp& = delete;
    auto operator=(Imp&&) -> Imp& = delete;

    ~Imp() { close_env(); }

private:
    using NewKey =
        std::tuple<Table, Mode, UnallocatedCString, UnallocatedCString>;
    using Pending = UnallocatedVector<NewKey>;

    const TableNames& names_;
    mutable MDB_env* env_;
    mutable Databases db_;
    mutable Pending pending_;
    mutable std::mutex pending_lock_;
    mutable std::mutex write_lock_;

    auto init_db(const Table table, unsigned int flags) noexcept -> MDB_dbi
    {
        MDB_txn* transaction{nullptr};
        auto status = (0 == ::mdb_txn_begin(env_, nullptr, 0, &transaction));

        OT_ASSERT(status);
        OT_ASSERT(nullptr != transaction);

        auto output = MDB_dbi{};
        status = 0 == ::mdb_dbi_open(
                          transaction,
                          names_.at(table).c_str(),
                          MDB_CREATE | flags,
                          &output);

        if (!static_cast<bool>(status)) {  // free memory allocated in
                                           // mdb_txn_begin - transaction
            ::mdb_txn_abort(transaction);
        }
        OT_ASSERT(status);

        ::mdb_txn_commit(transaction);  // free memory allocated in
                                        // mdb_txn_begin - transaction

        return output;
    }

    auto init_environment(
        const UnallocatedCString& folder,
        const std::size_t tables,
        const Flags flags) noexcept -> void
    {
        OT_ASSERT(std::numeric_limits<unsigned int>::max() >= tables);

        auto rc = ::mdb_env_create(&env_);
        bool set = 0 == rc;
        if (!set) {
            LogConsole()("failed to mdb_env_create: ")(::mdb_strerror(rc))
                .Flush();
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
            LogConsole()("failed to set maxreaders: ")(::mdb_strerror(rc))
                .Flush();
            close_env();
        }

        OT_ASSERT(set);

        rc = ::mdb_env_open(env_, folder.c_str(), flags, 0664);
        set = 0 == rc;
        if (!set) {
            LogConsole()("failed to open: ")(folder.c_str())(" flags: ")(
                flags)(" reason: ")(::mdb_strerror(rc))
                .Flush();
            close_env();
        }

        OT_ASSERT(set);
    }

    auto init_tables(const TablesToInit init) noexcept -> void
    {
        for (const auto& [table, flags] : init) {
            db_.emplace(table, init_db(table, flags));
        }
    }

    auto close_env() -> void
    {
        if (nullptr != env_) {
            ::mdb_env_close(env_);
            env_ = nullptr;
        }
    }
};

LMDB::LMDB(
    const TableNames& names,
    const UnallocatedCString& folder,
    const TablesToInit init,
    const Flags flags,
    const std::size_t extraTables) noexcept
    : imp_(std::make_unique<Imp>(names, folder, init, flags, extraTables))
{
}

LMDB::LMDB(LMDB&& rhs) noexcept
    : imp_(std::move(rhs.imp_))
{
}

LMDB::Transaction::Transaction(
    MDB_env* env,
    const bool rw,
    std::unique_ptr<Lock> lock,
    MDB_txn* parent) noexcept(false)
    : success_(false)
    , lock_(std::move(lock))
    , ptr_(nullptr)
{
    const Flags flags = rw ? 0u : MDB_RDONLY;

    if (0 != ::mdb_txn_begin(env, parent, flags, &ptr_)) {
        throw std::runtime_error("Failed to start transaction");
    }
}

LMDB::Transaction::Transaction(Transaction&& rhs) noexcept
    : success_(rhs.success_)
    , lock_(std::move(rhs.lock_))
    , ptr_(rhs.ptr_)
{
    rhs.ptr_ = nullptr;
}

auto LMDB::Transaction::Finalize(const std::optional<bool> success) noexcept
    -> bool
{
    struct Cleanup {
        Cleanup(MDB_txn*& ptr)
            : ptr_(ptr)
        {
        }

        ~Cleanup() { ptr_ = nullptr; }

    private:
        MDB_txn*& ptr_;
    };

    if (nullptr != ptr_) {
        if (success.has_value()) { success_ = success.value(); }

        auto cleanup = Cleanup{ptr_};

        if (success_) {

            return 0 == ::mdb_txn_commit(ptr_);
        } else {
            ::mdb_txn_abort(ptr_);

            return true;
        }
    }

    return false;
}

LMDB::Transaction::~Transaction() { Finalize(); }

auto LMDB::Commit() const noexcept -> bool { return imp_->Commit(); }

auto LMDB::Delete(const Table table, MDB_txn* parent) const noexcept -> bool
{
    return imp_->Delete(table, parent);
}

auto LMDB::Delete(const Table table, const ReadView index, MDB_txn* parent)
    const noexcept -> bool
{
    return imp_->Delete(table, index, parent);
}

auto LMDB::Delete(const Table table, const std::size_t index, MDB_txn* parent)
    const noexcept -> bool
{
    return imp_->Delete(table, index, parent);
}

auto LMDB::Delete(
    const Table table,
    const std::size_t index,
    const ReadView data,
    MDB_txn* parent) const noexcept -> bool
{
    return imp_->Delete(table, index, data, parent);
}

auto LMDB::Delete(
    const Table table,
    const ReadView index,
    const ReadView data,
    MDB_txn* parent) const noexcept -> bool
{
    return imp_->Delete(table, index, data, parent);
}

auto LMDB::Exists(const Table table, const ReadView index) const noexcept
    -> bool
{
    return imp_->Exists(table, index, {});
}

auto LMDB::Exists(const Table table, const ReadView index, const ReadView value)
    const noexcept -> bool
{
    return imp_->Exists(table, index, value);
}

auto LMDB::Load(
    const Table table,
    const ReadView index,
    const Callback cb,
    const Mode multiple) const noexcept -> bool
{
    return imp_->Load(table, index, cb, multiple);
}

auto LMDB::Load(
    const Table table,
    const ReadView index,
    const Callback cb,
    Transaction& tx,
    const Mode multiple) const noexcept -> bool
{
    return imp_->Load(table, index, cb, multiple, tx);
}

auto LMDB::Load(
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

auto LMDB::Queue(
    const Table table,
    const ReadView key,
    const ReadView value,
    const Mode mode) const noexcept -> bool
{
    return imp_->Queue(table, key, value, mode);
}

auto LMDB::Read(const Table table, const ReadCallback cb, const Dir dir)
    const noexcept -> bool
{
    return imp_->Read(table, cb, dir);
}

auto LMDB::ReadAndDelete(
    const Table table,
    const ReadCallback cb,
    MDB_txn& tx,
    const UnallocatedCString& message) const noexcept -> bool
{
    return imp_->ReadAndDelete(table, cb, tx, message);
}

auto LMDB::ReadFrom(
    const Table table,
    const ReadView index,
    const ReadCallback cb,
    const Dir dir) const noexcept -> bool
{
    return imp_->ReadFrom(table, index, cb, dir);
}

auto LMDB::ReadFrom(
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

auto LMDB::Store(
    const Table table,
    const ReadView index,
    const ReadView data,
    MDB_txn* parent,
    const Flags flags) const noexcept -> Result
{
    return imp_->Store(table, index, data, parent, flags);
}

auto LMDB::Store(
    const Table table,
    const std::size_t index,
    const ReadView data,
    MDB_txn* parent,
    const Flags flags) const noexcept -> Result
{
    return Store(
        table,
        ReadView{reinterpret_cast<const char*>(&index), sizeof(index)},
        data,
        parent,
        flags);
}

auto LMDB::StoreOrUpdate(
    const Table table,
    const ReadView index,
    const UpdateCallback cb,
    MDB_txn* parent,
    const Flags flags) const noexcept -> Result
{
    return imp_->StoreOrUpdate(table, index, cb, parent, flags);
}

auto LMDB::TransactionRO() const noexcept(false) -> Transaction
{
    return imp_->TransactionRO();
}

auto LMDB::TransactionRW(MDB_txn* parent) const noexcept(false) -> Transaction
{
    return imp_->TransactionRW(parent);
}

LMDB::~LMDB() = default;
}  // namespace opentxs::storage::lmdb
