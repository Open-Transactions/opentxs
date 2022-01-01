// Copyright (c) 2010-2021 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <future>
#include <memory>
#include <string>
#include <vector>

#include "internal/storage/drivers/Drivers.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/Version.hpp"
#include "opentxs/crypto/key/Symmetric.hpp"
#include "opentxs/storage/Driver.hpp"
#include "opentxs/util/Bytes.hpp"

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
class Root;
}  // namespace storage

class Flag;
class String;
}  // namespace opentxs

namespace opentxs::storage::driver
{
class Multiplex final : virtual public internal::Multiplex
{
public:
    auto EmptyBucket(const bool bucket) const -> bool final;
    auto LoadFromBucket(
        const std::string& key,
        std::string& value,
        const bool bucket) const -> bool final;
    auto Load(const std::string& key, const bool checking, std::string& value)
        const -> bool final;
    auto LoadRoot() const -> std::string final;
    auto Migrate(const std::string& key, const storage::Driver& to) const
        -> bool final;
    auto Store(
        const bool isTransaction,
        const std::string& key,
        const std::string& value,
        const bool bucket) const -> bool final;
    void Store(
        const bool isTransaction,
        const std::string& key,
        const std::string& value,
        const bool bucket,
        std::promise<bool>& promise) const final;
    auto Store(
        const bool isTransaction,
        const std::string& value,
        std::string& key) const -> bool final;
    auto StoreRoot(const bool commit, const std::string& hash) const
        -> bool final;

    auto BestRoot(bool& primaryOutOfSync) -> std::string final;
    void InitBackup() final;
    void InitEncryptedBackup(crypto::key::Symmetric& key) final;
    auto Primary() -> storage::Driver& final;
    void SynchronizePlugins(
        const std::string& hash,
        const storage::Root& root,
        const bool syncPrimary) final;

    Multiplex(
        const api::Crypto& crypto,
        const api::network::Asio& asio,
        const api::session::Storage& storage,
        const Flag& primaryBucket,
        const storage::Config& config);

    ~Multiplex() final;

private:
    const api::Crypto& crypto_;
    const api::network::Asio& asio_;
    const api::session::Storage& storage_;
    const Flag& primary_bucket_;
    const storage::Config& config_;
    std::unique_ptr<storage::Plugin> primary_plugin_;
    std::pmr::vector<std::unique_ptr<storage::Plugin>> backup_plugins_;
    OTSymmetricKey null_;

    auto Cleanup() -> void;
    auto Cleanup_Multiplex() -> void;
    auto init(
        const std::string& primary,
        std::unique_ptr<storage::Plugin>& plugin) -> void;
    auto init_fs(std::unique_ptr<storage::Plugin>& plugin) -> void;
    auto init_fs_backup(const std::string& dir) -> void;
    auto init_lmdb(std::unique_ptr<storage::Plugin>& plugin) -> void;
    auto init_memdb(std::unique_ptr<storage::Plugin>& plugin) -> void;
    auto init_sqlite(std::unique_ptr<storage::Plugin>& plugin) -> void;
    auto Init_Multiplex() -> void;
    auto migrate_primary(const std::string& from, const std::string& to)
        -> void;

    Multiplex() = delete;
    Multiplex(const Multiplex&) = delete;
    Multiplex(Multiplex&&) = delete;
    auto operator=(const Multiplex&) -> Multiplex& = delete;
    auto operator=(Multiplex&&) -> Multiplex& = delete;
};
}  // namespace opentxs::storage::driver
