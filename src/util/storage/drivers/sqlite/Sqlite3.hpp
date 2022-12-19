// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

extern "C" {
#include <sqlite3.h>
}

#include <cstddef>
#include <filesystem>
#include <iosfwd>
#include <mutex>
#include <utility>

#include "internal/util/Flag.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/storage/Driver.hpp"
#include "util/storage/Plugin.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace network
{
class Asio;
}  // namespace network

namespace session
{
class Storage;
}  // namespace session

class Crypto;
}  // namespace api

namespace storage
{
class Config;
}  // namespace storage
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::driver
{
// SQLite3 implementation of opentxs::storage
class Sqlite3 final : public virtual implementation::Plugin,
                      public virtual storage::Driver
{
public:
    auto EmptyBucket(const bool bucket) const -> bool final;
    auto LoadFromBucket(
        const UnallocatedCString& key,
        UnallocatedCString& value,
        const bool bucket) const -> bool final;
    auto LoadRoot() const -> UnallocatedCString final;
    auto StoreRoot(const bool commit, const UnallocatedCString& hash) const
        -> bool final;

    void Cleanup() final;
    void Cleanup_Sqlite3();

    Sqlite3(
        const api::Crypto& crypto,
        const api::network::Asio& asio,
        const api::session::Storage& storage,
        const storage::Config& config,
        const Flag& bucket);
    Sqlite3() = delete;
    Sqlite3(const Sqlite3&) = delete;
    Sqlite3(Sqlite3&&) = delete;
    auto operator=(const Sqlite3&) -> Sqlite3& = delete;
    auto operator=(Sqlite3&&) -> Sqlite3& = delete;

    ~Sqlite3() final;

private:
    using ot_super = Plugin;

    const std::filesystem::path folder_;
    mutable std::mutex transaction_lock_;
    mutable OTFlag transaction_bucket_;
    mutable UnallocatedVector<
        std::pair<const UnallocatedCString, const UnallocatedCString>>
        pending_;
    sqlite3* db_{nullptr};

    auto bind_key(
        const UnallocatedCString& source,
        const UnallocatedCString& key,
        const std::size_t start) const -> UnallocatedCString;
    void commit(std::stringstream& sql) const;
    auto commit_transaction(const UnallocatedCString& rootHash) const -> bool;
    auto Create(const UnallocatedCString& tablename) const -> bool;
    auto expand_sql(sqlite3_stmt* statement) const -> UnallocatedCString;
    auto GetTableName(const bool bucket) const -> UnallocatedCString;
    auto Select(
        const UnallocatedCString& key,
        const UnallocatedCString& tablename,
        UnallocatedCString& value) const -> bool;
    auto Purge(const UnallocatedCString& tablename) const -> bool;
    void set_data(std::stringstream& sql) const;
    void set_root(const UnallocatedCString& rootHash, std::stringstream& sql)
        const;
    void start_transaction(std::stringstream& sql) const;
    auto store(
        const bool isTransaction,
        const UnallocatedCString& key,
        const UnallocatedCString& value,
        const bool bucke) const -> bool final;
    auto Upsert(
        const UnallocatedCString& key,
        const UnallocatedCString& tablename,
        const UnallocatedCString& value) const -> bool;

    void Init_Sqlite3();
};
}  // namespace opentxs::storage::driver
