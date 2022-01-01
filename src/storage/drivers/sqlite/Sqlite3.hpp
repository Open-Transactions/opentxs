// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

extern "C" {
#include <sqlite3.h>
}

#include <cstddef>
#include <future>
#include <iosfwd>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/core/Flag.hpp"
#include "opentxs/storage/Driver.hpp"
#include "opentxs/util/Bytes.hpp"
#include "storage/Plugin.hpp"

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
class Plugin;
}  // namespace storage
}  // namespace opentxs

namespace opentxs::storage::driver
{
// SQLite3 implementation of opentxs::storage
class Sqlite3 final : public virtual implementation::Plugin,
                      public virtual storage::Driver
{
public:
    auto EmptyBucket(const bool bucket) const -> bool final;
    auto LoadFromBucket(
        const std::string& key,
        std::string& value,
        const bool bucket) const -> bool final;
    auto LoadRoot() const -> std::string final;
    auto StoreRoot(const bool commit, const std::string& hash) const
        -> bool final;

    void Cleanup() final;
    void Cleanup_Sqlite3();

    Sqlite3(
        const api::Crypto& crypto,
        const api::network::Asio& asio,
        const api::session::Storage& storage,
        const storage::Config& config,
        const Flag& bucket);

    ~Sqlite3() final;

private:
    using ot_super = Plugin;

    std::string folder_;
    mutable std::mutex transaction_lock_;
    mutable OTFlag transaction_bucket_;
    mutable std::pmr::vector<std::pair<const std::string, const std::string>>
        pending_;
    sqlite3* db_{nullptr};

    auto bind_key(
        const std::string& source,
        const std::string& key,
        const std::size_t start) const -> std::string;
    void commit(std::stringstream& sql) const;
    auto commit_transaction(const std::string& rootHash) const -> bool;
    auto Create(const std::string& tablename) const -> bool;
    auto expand_sql(sqlite3_stmt* statement) const -> std::string;
    auto GetTableName(const bool bucket) const -> std::string;
    auto Select(
        const std::string& key,
        const std::string& tablename,
        std::string& value) const -> bool;
    auto Purge(const std::string& tablename) const -> bool;
    void set_data(std::stringstream& sql) const;
    void set_root(const std::string& rootHash, std::stringstream& sql) const;
    void start_transaction(std::stringstream& sql) const;
    void store(
        const bool isTransaction,
        const std::string& key,
        const std::string& value,
        const bool bucket,
        std::promise<bool>* promise) const final;
    auto Upsert(
        const std::string& key,
        const std::string& tablename,
        const std::string& value) const -> bool;

    void Init_Sqlite3();

    Sqlite3() = delete;
    Sqlite3(const Sqlite3&) = delete;
    Sqlite3(Sqlite3&&) = delete;
    auto operator=(const Sqlite3&) -> Sqlite3& = delete;
    auto operator=(Sqlite3&&) -> Sqlite3& = delete;
};
}  // namespace opentxs::storage::driver
