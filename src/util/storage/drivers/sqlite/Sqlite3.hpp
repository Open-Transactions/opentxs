// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

extern "C" {
#include <sqlite3.h>
}

#include <cs_shared_guarded.h>
#include <cstddef>
#include <iosfwd>
#include <optional>
#include <shared_mutex>
#include <span>
#include <string_view>

#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Types.hpp"
#include "opentxs/util/storage/Types.hpp"
#include "util/storage/drivers/Driver.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
}  // namespace api

namespace storage
{
class Config;
}  // namespace storage

class Writer;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::driver
{
class Sqlite3 final : public virtual storage::implementation::Driver
{
public:
    auto Description() const noexcept -> std::string_view final;
    auto Load(const Log& logger, const Hash& key, Search order, Writer& value)
        const noexcept -> bool final;
    auto LoadRoot() const noexcept -> Hash final;

    auto Commit(const Hash& root, Transaction data, Bucket bucket)
        const noexcept -> bool final;
    auto EmptyBucket(Bucket bucket) const noexcept -> bool final;
    auto Store(Transaction data, Bucket bucket) const noexcept -> bool final;

    Sqlite3(const api::Crypto& crypto, const storage::Config& config) noexcept(
        false);
    Sqlite3() = delete;
    Sqlite3(const Sqlite3&) = delete;
    Sqlite3(Sqlite3&&) = delete;
    auto operator=(const Sqlite3&) -> Sqlite3& = delete;
    auto operator=(Sqlite3&&) -> Sqlite3& = delete;

    ~Sqlite3() final;

private:
    struct Data {
        auto Load(
            const Log& logger,
            const Hash& key,
            Search order,
            Writer& value) const noexcept -> bool;
        auto LoadRoot() const noexcept -> Hash;

        auto Commit(const Hash& root, Transaction data, Bucket bucket)
            const noexcept -> bool;
        auto EmptyBucket(Bucket bucket) noexcept -> bool;
        auto Store(Transaction data, Bucket bucket) const noexcept -> bool;

        Data(const storage::Config& config) noexcept(false);
        Data(const Data&) = delete;
        Data(Data&&) = delete;
        auto operator=(const Data&) -> Data& = delete;
        auto operator=(Data&&) -> Data& = delete;

        ~Data();

    private:
        struct Wrapper {
            operator ::sqlite3*() noexcept { return db_; }

            Wrapper(const storage::Config& config) noexcept(false);
            Wrapper() = delete;
            Wrapper(const Wrapper&) = delete;
            Wrapper(Wrapper&&) = delete;
            auto operator=(const Wrapper&) -> Wrapper& = delete;
            auto operator=(Wrapper&&) -> Wrapper& = delete;

            ~Wrapper();

        private:
            ::sqlite3* db_;
        };

        const storage::Config& config_;

        static auto check(int rc, int expected = SQLITE_OK) noexcept(false)
            -> void;
        static auto commit(std::stringstream& sql) noexcept -> void;
        static auto expand_sql(sqlite3_stmt* statement) noexcept
            -> UnallocatedCString;
        static auto start_transaction(std::stringstream& sql) noexcept -> void;

        auto begin_transaction() const noexcept(false) -> ::sqlite3_stmt*;
        auto bind_key(ReadView source, ReadView key, const std::size_t start)
            const noexcept(false) -> UnallocatedCString;
        auto commit_transaction() const noexcept(false) -> ::sqlite3_stmt*;
        auto create_table(Bucket bucket, ::sqlite3* db = nullptr) const
            noexcept(false) -> ::sqlite3_stmt*;
        auto create_table(std::string_view table, ::sqlite3* db = nullptr) const
            noexcept(false) -> ::sqlite3_stmt*;
        auto db() const noexcept -> ::sqlite3*;
        auto drop(Bucket bucket) const noexcept(false) -> ::sqlite3_stmt*;
        auto execute(sqlite3_stmt* statement) const noexcept(false) -> void;
        auto get_table_name(Bucket bucket) const noexcept -> std::string_view;
        auto insert(
            std::span<const UnallocatedCString> keys,
            std::span<const ReadView> values,
            Bucket bucket) const noexcept(false) -> ::sqlite3_stmt*;
        auto root(const Hash& hash) const noexcept(false) -> ::sqlite3_stmt*;
        auto select(ReadView key, ReadView tablename, Writer& value)
            const noexcept -> bool;
        auto store(Transaction data, Bucket bucket, std::optional<Hash> root)
            const noexcept -> bool;
    };

    using GuardedData = libguarded::shared_guarded<Data, std::shared_mutex>;

    mutable GuardedData data_;
};
}  // namespace opentxs::storage::driver
