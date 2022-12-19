// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <memory>

#include "internal/util/storage/drivers/Drivers.hpp"
#include "opentxs/crypto/symmetric/Key.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/storage/Driver.hpp"

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
class Factory;
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
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage::driver
{
class Multiplex final : virtual public internal::Multiplex
{
public:
    auto EmptyBucket(const bool bucket) const -> bool final;
    auto LoadFromBucket(
        const UnallocatedCString& key,
        UnallocatedCString& value,
        const bool bucket) const -> bool final;
    auto Load(
        const UnallocatedCString& key,
        const bool checking,
        UnallocatedCString& value) const -> bool final;
    auto LoadRoot() const -> UnallocatedCString final;
    auto Migrate(const UnallocatedCString& key, const storage::Driver& to) const
        -> bool final;
    auto Store(
        const bool isTransaction,
        const UnallocatedCString& key,
        const UnallocatedCString& value,
        const bool bucket) const -> bool final;
    auto Store(
        const bool isTransaction,
        const UnallocatedCString& value,
        UnallocatedCString& key) const -> bool final;
    auto StoreRoot(const bool commit, const UnallocatedCString& hash) const
        -> bool final;

    auto BestRoot(bool& primaryOutOfSync) -> UnallocatedCString final;
    auto InitBackup() -> void final;
    auto InitEncryptedBackup(crypto::symmetric::Key& key) -> void final;
    auto Primary() -> storage::Driver& final;
    auto SynchronizePlugins(
        const UnallocatedCString& hash,
        const storage::Root& root,
        const bool syncPrimary) -> void final;

    Multiplex(
        const api::Crypto& crypto,
        const api::network::Asio& asio,
        const api::session::Factory& factory,
        const api::session::Storage& storage,
        const Flag& primaryBucket,
        const storage::Config& config);
    Multiplex() = delete;
    Multiplex(const Multiplex&) = delete;
    Multiplex(Multiplex&&) = delete;
    auto operator=(const Multiplex&) -> Multiplex& = delete;
    auto operator=(Multiplex&&) -> Multiplex& = delete;

    ~Multiplex() final;

private:
    const api::Crypto& crypto_;
    const api::network::Asio& asio_;
    const api::session::Factory& factory_;
    const api::session::Storage& storage_;
    const Flag& primary_bucket_;
    const storage::Config& config_;
    std::unique_ptr<storage::Plugin> primary_plugin_;
    UnallocatedVector<std::unique_ptr<storage::Plugin>> backup_plugins_;
    UnallocatedVector<storage::Plugin*> plugins_;
    crypto::symmetric::Key null_;

    auto Cleanup() -> void;
    auto Cleanup_Multiplex() -> void;
    auto init(
        const UnallocatedCString& primary,
        std::unique_ptr<storage::Plugin>& plugin) -> void;
    auto init_fs(std::unique_ptr<storage::Plugin>& plugin) -> void;
    auto init_fs_backup(const UnallocatedCString& dir) -> void;
    auto init_lmdb(std::unique_ptr<storage::Plugin>& plugin) -> void;
    auto init_memdb(std::unique_ptr<storage::Plugin>& plugin) -> void;
    auto init_sqlite(std::unique_ptr<storage::Plugin>& plugin) -> void;
    auto Init_Multiplex() -> void;
    auto migrate_primary(
        const UnallocatedCString& from,
        const UnallocatedCString& to) -> void;
};
}  // namespace opentxs::storage::driver
