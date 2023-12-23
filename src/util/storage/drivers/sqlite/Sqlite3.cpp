// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::storage::Driver

#include "util/storage/drivers/sqlite/Sqlite3.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <array>
#include <filesystem>
#include <functional>
#include <iterator>
#include <limits>
#include <memory>
#include <sstream>  // IWYU pragma: keep
#include <stdexcept>
#include <string_view>
#include <utility>
#include <variant>

#include "internal/util/P0330.hpp"
#include "internal/util/storage/drivers/Factory.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/FixedByteArray.hpp"  // IWYU pragma: keep
#include "opentxs/storage/Types.internal.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"  // IWYU pragma: keep
#include "opentxs/util/storage/Driver.hpp"
#include "util/storage/Config.hpp"

namespace opentxs::factory
{
auto StorageSqlite3(
    const api::Crypto& crypto,
    const storage::Config& config) noexcept -> std::unique_ptr<storage::Driver>
{
    using ReturnType = storage::driver::Sqlite3;

    try {

        return std::make_unique<ReturnType>(crypto, config);
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return {};
    }
}
}  // namespace opentxs::factory

namespace opentxs::storage::driver
{
Sqlite3::Data::Wrapper::Wrapper(const storage::Config& config) noexcept(false)
    : db_(nullptr)
{
    const auto filename = config.path_ / config.sqlite3_db_file_;
    auto rc = ::sqlite3_open_v2(
        filename.string().c_str(),
        std::addressof(db_),
        SQLITE_OPEN_READWRITE | SQLITE_OPEN_CREATE | SQLITE_OPEN_NOMUTEX,
        nullptr);
    check(rc);
}

Sqlite3::Data::Wrapper::~Wrapper() { ::sqlite3_close(db_); }
}  // namespace opentxs::storage::driver

namespace opentxs::storage::driver
{
using namespace std::literals;

Sqlite3::Data::Data(const storage::Config& config) noexcept(false)
    : config_(config)
{
    auto* db = this->db();
    auto rc = ::sqlite3_exec(
        db, "PRAGMA journal_mode=WAL;", nullptr, nullptr, nullptr);
    check(rc);
    rc = ::sqlite3_extended_result_codes(db, 1);
    check(rc);

    {
        using enum Bucket;
        auto sql = Vector<::sqlite3_stmt*>{};
        sql.reserve(3_uz);
        sql.emplace_back(create_table(left, db));
        sql.emplace_back(create_table(right, db));
        sql.emplace_back(create_table(config.sqlite3_control_table_, db));
        auto out = true;
        const auto execute = [&out, this](auto* s) {
            try {
                this->execute(s);
            } catch (const std::exception& e) {
                LogError()()(e.what()).Flush();
                out = false;
            }
        };
        std::ranges::for_each(sql, execute);
        std::ranges::for_each(sql, ::sqlite3_finalize);
    }
}

auto Sqlite3::Data::begin_transaction() const noexcept(false) -> ::sqlite3_stmt*
{
    const auto sql = [&] {
        auto out = std::stringstream{};
        start_transaction(out);

        return out.str();
    }();
    ::sqlite3_stmt* out{nullptr};
    const auto rc = ::sqlite3_prepare_v2(
        db(), sql.data(), static_cast<int>(sql.size()), &out, nullptr);
    check(rc);

    return out;
}

auto Sqlite3::Data::bind_key(
    ReadView source,
    ReadView key,
    const std::size_t start) const noexcept(false) -> UnallocatedCString
{
    assert_true(std::numeric_limits<int>::max() >= key.size());
    assert_true(std::numeric_limits<int>::max() >= start);

    sqlite3_stmt* statement{nullptr};
    auto rc = ::sqlite3_prepare_v2(
        db(),
        source.data(),
        static_cast<int>(source.size()),
        &statement,
        nullptr);
    check(rc);
    rc = ::sqlite3_bind_text(
        statement,
        static_cast<int>(start),
        key.data(),
        static_cast<int>(key.size()),
        SQLITE_STATIC);
    check(rc);
    const auto output = expand_sql(statement);
    rc = ::sqlite3_finalize(statement);
    check(rc);

    return output;
}

auto Sqlite3::Data::check(int rc, int expected) noexcept(false) -> void
{
    if (expected != rc) { throw std::runtime_error{::sqlite3_errstr(rc)}; }
}

auto Sqlite3::Data::Commit(const Hash& root, Transaction data, Bucket bucket)
    const noexcept -> bool
{
    return store(data, bucket, root);
}

auto Sqlite3::Data::commit(std::stringstream& sql) noexcept -> void
{
    sql << "COMMIT TRANSACTION;";
}

auto Sqlite3::Data::commit_transaction() const noexcept(false)
    -> ::sqlite3_stmt*
{
    const auto sql = [&] {
        auto out = std::stringstream{};
        commit(out);

        return out.str();
    }();
    ::sqlite3_stmt* out{nullptr};
    const auto rc = ::sqlite3_prepare_v2(
        db(), sql.data(), static_cast<int>(sql.size()), &out, nullptr);
    check(rc);

    return out;
}

auto Sqlite3::Data::create_table(Bucket bucket, ::sqlite3* db) const
    noexcept(false) -> ::sqlite3_stmt*
{
    return create_table(get_table_name(bucket), db);
}

auto Sqlite3::Data::create_table(std::string_view table, ::sqlite3* db) const
    noexcept(false) -> ::sqlite3_stmt*
{
    if (nullptr == db) { db = this->db(); }

    const auto sql = "CREATE TABLE IF NOT EXISTS "s.append(table).append(
        " (k TEXT PRIMARY KEY, v BLOB);");
    ::sqlite3_stmt* out{nullptr};
    auto rc = ::sqlite3_prepare_v2(
        db, sql.data(), static_cast<int>(sql.size()), &out, nullptr);
    check(rc);

    return out;
}

auto Sqlite3::Data::db() const noexcept -> ::sqlite3*
{
    static thread_local auto connection = Wrapper{config_};

    return connection;
}

auto Sqlite3::Data::drop(Bucket bucket) const noexcept(false) -> ::sqlite3_stmt*
{
    const auto sql = "DROP TABLE "s.append(get_table_name(bucket)).append(";");
    ::sqlite3_stmt* out{nullptr};
    auto rc = ::sqlite3_prepare_v2(
        db(), sql.data(), static_cast<int>(sql.size()), &out, nullptr);
    check(rc);

    return out;
}

auto Sqlite3::Data::EmptyBucket(Bucket bucket) noexcept -> bool
{
    try {
        auto sql = Vector<::sqlite3_stmt*>{};
        sql.reserve(4_uz);
        sql.emplace_back(begin_transaction());
        sql.emplace_back(drop(bucket));
        sql.emplace_back(create_table(bucket));
        sql.emplace_back(commit_transaction());
        auto out = true;
        const auto execute = [&out, this](auto* s) {
            try {
                this->execute(s);
            } catch (const std::exception& e) {
                LogError()()(e.what()).Flush();
                out = false;
            }
        };
        std::ranges::for_each(sql, execute);
        std::ranges::for_each(sql, ::sqlite3_finalize);

        return out;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return false;
    }
}

auto Sqlite3::Data::execute(sqlite3_stmt* statement) const noexcept(false)
    -> void
{
    check(::sqlite3_step(statement), SQLITE_DONE);
}

auto Sqlite3::Data::expand_sql(sqlite3_stmt* statement) noexcept
    -> UnallocatedCString
{
    auto* const sql = ::sqlite3_expanded_sql(statement);
    const auto output = UnallocatedCString{sql};
    ::sqlite3_free(sql);

    return output;
}

auto Sqlite3::Data::get_table_name(Bucket bucket) const noexcept
    -> std::string_view
{
    using enum Bucket;

    switch (bucket) {
        case left: {

            return config_.sqlite3_primary_bucket_;
        }
        case right: {

            return config_.sqlite3_secondary_bucket_;
        }
        default: {
            LogAbort()()("invalid bucket").Abort();
        }
    }
}

auto Sqlite3::Data::insert(
    std::span<const UnallocatedCString> keys,
    std::span<const ReadView> values,
    Bucket bucket) const noexcept(false) -> ::sqlite3_stmt*
{
    const auto size = static_cast<int>(keys.size());
    const auto sql = [&] {
        auto out = std::stringstream{};
        out << "INSERT OR REPLACE INTO '" << get_table_name(bucket)
            << "' (k, v) VALUES ";
        auto counter{0};

        for (auto i{0}; i < size; ++i) {
            out << "(?" << ++counter << ", ?";
            out << ++counter << ")";

            if (counter < (2 * size)) {
                out << ", ";
            } else {
                out << "; ";
            }
        }

        return out.str();
    }();
    ::sqlite3_stmt* out{nullptr};
    auto rc = ::sqlite3_prepare_v2(
        db(), sql.data(), static_cast<int>(sql.size()), &out, nullptr);
    check(rc);
    auto counter{0};

    for (auto i{0}; i < size; ++i) {
        const auto& key = keys[i];
        const auto& value = values[i];
        rc = ::sqlite3_bind_text(
            out,
            ++counter,
            key.data(),
            static_cast<int>(key.size()),
            SQLITE_STATIC);
        check(rc);
        rc = ::sqlite3_bind_blob(
            out,
            ++counter,
            value.data(),
            static_cast<int>(value.size()),
            SQLITE_STATIC);
        check(rc);
    }

    return out;
}

auto Sqlite3::Data::Load(
    const Log& logger,
    const Hash& key,
    Search order,
    Writer& value) const noexcept -> bool
{
    const auto search = [this, order]() -> std::array<std::string_view, 2> {
        using enum Bucket;
        using enum Search;

        if (ltr == order) {

            return {get_table_name(left), get_table_name(right)};
        } else {

            return {get_table_name(right), get_table_name(left)};
        }
    }();

    for (const auto& table : search) {
        if (select(std::visit(EncodedView{}, key), table, value)) {
            return true;
        }
    }

    return false;
}

auto Sqlite3::Data::LoadRoot() const noexcept -> Hash
{
    auto out = ByteArray{};
    auto writer = out.WriteInto();
    select(config_.sqlite3_root_key_, config_.sqlite3_control_table_, writer);

    return read(out.Bytes());
}

auto Sqlite3::Data::root(const Hash& hash) const noexcept(false)
    -> ::sqlite3_stmt*
{
    const auto sql = [&] {
        auto out = std::stringstream{};
        auto counter{0};
        out << "INSERT OR REPLACE INTO '" << config_.sqlite3_control_table_
            << "' (k, v) VALUES ";
        out << "(?" << ++counter << ", ?";
        out << ++counter << "); ";

        return out.str();
    }();
    ::sqlite3_stmt* out{nullptr};
    auto rc = ::sqlite3_prepare_v2(
        db(), sql.data(), static_cast<int>(sql.size()), &out, nullptr);
    check(rc);
    auto counter{0};
    const auto& key = config_.sqlite3_root_key_;
    rc = ::sqlite3_bind_text(
        out,
        ++counter,
        key.c_str(),
        static_cast<int>(key.size()),
        SQLITE_STATIC);
    check(rc);
    const auto bytes = unencoded_view(hash);
    rc = ::sqlite3_bind_blob(
        out,
        ++counter,
        bytes.data(),
        static_cast<int>(bytes.size()),
        SQLITE_STATIC);
    check(rc);

    return out;
}

auto Sqlite3::Data::select(ReadView key, ReadView tablename, Writer& value)
    const noexcept -> bool
{
    try {
        auto success = false;
        sqlite3_stmt* statement{nullptr};
        const auto query =
            "SELECT v FROM '"s.append(tablename).append("' WHERE k GLOB ?1;"sv);
        const auto sql = bind_key(query, key, 1);
        auto rc = ::sqlite3_prepare_v2(
            db(),
            sql.data(),
            static_cast<int>(sql.size()),
            &statement,
            nullptr);
        check(rc);
        auto result = ::sqlite3_step(statement);
        auto retry = 3_uz;

        while (0 < retry) {
            switch (result) {
                case SQLITE_DONE:
                case SQLITE_ROW: {
                    retry = 0_uz;
                    const auto size = ::sqlite3_column_bytes(statement, 0);
                    success = (0 < size);

                    if (success) {
                        const auto* const pResult =
                            ::sqlite3_column_blob(statement, 0);
                        const auto view = ReadView{
                            static_cast<const char*>(pResult),
                            static_cast<std::size_t>(size)};
                        // NOTE retry = 0 prevents the loop from running again
                        // NOLINTNEXTLINE(bugprone-use-after-move)
                        copy(view, std::move(value));
                    }
                } break;
                case SQLITE_BUSY: {
                    LogError()()("Busy.").Flush();
                    result = ::sqlite3_step(statement);
                    --retry;
                } break;
                default: {
                    LogError()()("Unknown error (")(result)(").").Flush();
                    result = ::sqlite3_step(statement);
                    --retry;
                }
            }
        }

        rc = ::sqlite3_finalize(statement);
        check(rc);

        return success;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return false;
    }
}

auto Sqlite3::Data::start_transaction(std::stringstream& sql) noexcept -> void
{
    sql << "BEGIN TRANSACTION; ";
}

auto Sqlite3::Data::Store(Transaction data, Bucket bucket) const noexcept
    -> bool
{
    return store(data, bucket, std::nullopt);
}

auto Sqlite3::Data::store(
    Transaction data,
    Bucket bucket,
    std::optional<Hash> root) const noexcept -> bool
{
    if (data.first.empty() && (false == root.has_value())) { return true; }

    try {
        const auto keys = [&] {
            const auto& in = data.first;
            auto out = Vector<UnallocatedCString>{};
            out.reserve(in.size());
            std::ranges::transform(in, std::back_inserter(out), to_string);

            return out;
        }();
        auto sql = Vector<::sqlite3_stmt*>{};
        sql.reserve(4_uz);
        sql.emplace_back(begin_transaction());

        if (false == keys.empty()) {
            sql.emplace_back(insert(keys, data.second, bucket));
        }

        if (root.has_value()) { sql.emplace_back(this->root(*root)); }

        sql.emplace_back(commit_transaction());
        auto out = true;
        const auto execute = [&out, this](auto* s) {
            try {
                this->execute(s);
            } catch (const std::exception& e) {
                LogError()()(e.what()).Flush();
                out = false;
            }
        };
        std::ranges::for_each(sql, execute);
        std::ranges::for_each(sql, ::sqlite3_finalize);

        return out;
    } catch (const std::exception& e) {
        LogError()()(e.what()).Flush();

        return false;
    }
}

Sqlite3::Data::~Data() = default;
}  // namespace opentxs::storage::driver

namespace opentxs::storage::driver
{
using namespace std::literals;

Sqlite3::Sqlite3(
    const api::Crypto& crypto,
    const storage::Config& config) noexcept(false)
    : Driver(crypto, config)
    , data_(config_)
{
}

auto Sqlite3::Commit(const Hash& root, Transaction data, Bucket bucket)
    const noexcept -> bool
{
    return data_.lock()->Commit(root, data, bucket);
}

auto Sqlite3::Description() const noexcept -> std::string_view
{
    return "sqlite3"sv;
}

auto Sqlite3::EmptyBucket(Bucket bucket) const noexcept -> bool
{
    return data_.lock()->EmptyBucket(bucket);
}

auto Sqlite3::Load(
    const Log& logger,
    const Hash& key,
    Search order,
    Writer& value) const noexcept -> bool
{
    return data_.lock_shared()->Load(logger, key, order, value);
}

auto Sqlite3::LoadRoot() const noexcept -> Hash
{
    return data_.lock_shared()->LoadRoot();
}

auto Sqlite3::Store(Transaction data, Bucket bucket) const noexcept -> bool
{
    return data_.lock()->Store(data, bucket);
}

Sqlite3::~Sqlite3() = default;
}  // namespace opentxs::storage::driver
