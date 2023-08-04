// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/drivers/multiplex/Multiplex.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstdint>
#include <filesystem>
#include <iterator>
#include <limits>
#include <memory>
#include <stdexcept>

#include "internal/util/Flag.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/Mutex.hpp"
#include "internal/util/P0330.hpp"
#include "internal/util/storage/drivers/Factory.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/storage/Plugin.hpp"
#include "util/storage/Config.hpp"
#include "util/storage/tree/Root.hpp"
#include "util/storage/tree/Tree.hpp"

namespace opentxs::factory
{
auto StorageMultiplex(
    const api::Crypto& crypto,
    const api::network::Asio& asio,
    const api::session::Factory& factory,
    const api::session::Storage& parent,
    const Flag& primaryBucket,
    const storage::Config& config) noexcept
    -> std::unique_ptr<storage::driver::internal::Multiplex>
{
    using ReturnType = opentxs::storage::driver::Multiplex;

    return std::make_unique<ReturnType>(
        crypto, asio, factory, parent, primaryBucket, config);
}
}  // namespace opentxs::factory

namespace opentxs::storage::driver
{
Multiplex::Multiplex(
    const api::Crypto& crypto,
    const api::network::Asio& asio,
    const api::session::Factory& factory,
    const api::session::Storage& storage,
    const Flag& primaryBucket,
    const storage::Config& config)
    : crypto_(crypto)
    , asio_(asio)
    , factory_(factory)
    , storage_(storage)
    , primary_bucket_(primaryBucket)
    , config_(config)
    , primary_plugin_()
    , backup_plugins_()
    , plugins_()
    , null_()
{
    Init_Multiplex();
}

auto Multiplex::BestRoot(bool& primaryOutOfSync) -> UnallocatedCString
{
    OT_ASSERT(primary_plugin_);

    const UnallocatedCString originalHash = primary_plugin_->LoadRoot();
    std::shared_ptr<storage::Root> bestRoot{nullptr};
    std::shared_ptr<storage::Root> localRoot{nullptr};
    UnallocatedCString bestHash{originalHash};
    std::uint64_t bestVersion{0};
    auto bucket = Flag::Factory(false);

    try {
        localRoot.reset(new storage::Root(
            asio_,
            crypto_,
            factory_,
            *this,
            bestHash,
            std::numeric_limits<std::int64_t>::max(),
            bucket));
        bestVersion = localRoot->Sequence();
        bestRoot = localRoot;
    } catch (std::runtime_error&) {
    }

    for (const auto& plugin : backup_plugins_) {
        OT_ASSERT(plugin);

        UnallocatedCString rootHash = plugin->LoadRoot();
        std::uint64_t localVersion{0};

        try {
            localRoot.reset(new storage::Root(
                asio_,
                crypto_,
                factory_,
                *this,
                rootHash,
                std::numeric_limits<std::int64_t>::max(),
                bucket));
            localVersion = localRoot->Sequence();
        } catch (std::runtime_error&) {
        }

        if (localVersion > bestVersion) {
            bestVersion = localVersion;
            bestHash = rootHash;
            bestRoot = localRoot;
        }
    }

    if (0 == bestVersion) { bestHash = ""; }

    if (originalHash != bestHash) {
        primary_plugin_->StoreRoot(false, bestHash);
        bestRoot->Save(*primary_plugin_);
        primaryOutOfSync = true;
    } else {
        primaryOutOfSync = false;
    }

    return bestHash;
}

auto Multiplex::Cleanup() -> void { Cleanup_Multiplex(); }

auto Multiplex::Cleanup_Multiplex() -> void {}

auto Multiplex::EmptyBucket(const bool bucket) const -> bool
{
    OT_ASSERT(primary_plugin_);

    for (const auto& plugin : backup_plugins_) {
        OT_ASSERT(plugin);

        plugin->EmptyBucket(bucket);
    }

    return primary_plugin_->EmptyBucket(bucket);
}

auto Multiplex::init(
    const UnallocatedCString& primary,
    std::unique_ptr<storage::Plugin>& plugin) -> void
{
    if (OT_STORAGE_PRIMARY_PLUGIN_MEMDB == primary) {
        init_memdb(plugin);
    } else if (OT_STORAGE_PRIMARY_PLUGIN_LMDB == primary) {
        init_lmdb(plugin);
    } else if (OT_STORAGE_PRIMARY_PLUGIN_SQLITE == primary) {
        init_sqlite(plugin);
    } else if (OT_STORAGE_PRIMARY_PLUGIN_FS == primary) {
        init_fs(plugin);
    }

    OT_ASSERT(plugin);
}

auto Multiplex::init_memdb(std::unique_ptr<storage::Plugin>& plugin) -> void
{
    LogVerbose()(OT_PRETTY_CLASS())("Initializing primary MemDB plugin.")
        .Flush();
    plugin = factory::StorageMemDB(
        crypto_, asio_, storage_, config_, primary_bucket_);
}

auto Multiplex::Init_Multiplex() -> void
{
    if (config_.migrate_plugin_) {
        migrate_primary(
            config_.previous_primary_plugin_, config_.primary_plugin_);
    } else {
        init(config_.primary_plugin_, primary_plugin_);
    }

    OT_ASSERT(primary_plugin_);

    // TODO init backup plugins
    plugins_.reserve(1_uz + backup_plugins_.size());
    plugins_.clear();
    plugins_.emplace_back(primary_plugin_.get());
    std::transform(
        backup_plugins_.begin(),
        backup_plugins_.end(),
        std::back_inserter(plugins_),
        [](auto& p) { return p.get(); });
}

auto Multiplex::InitBackup() -> void
{
    if (config_.fs_backup_directory_.empty()) { return; }

    init_fs_backup(config_.fs_backup_directory_.string());
}

auto Multiplex::InitEncryptedBackup(
    [[maybe_unused]] crypto::symmetric::Key& key) -> void
{
    if (config_.fs_encrypted_backup_directory_.empty()) { return; }

    init_fs_backup(config_.fs_encrypted_backup_directory_.string());
}

auto Multiplex::Load(
    const UnallocatedCString& key,
    const bool checking,
    UnallocatedCString& value) const -> bool
{
    OT_ASSERT(primary_plugin_);

    if (primary_plugin_->Load(key, checking, value)) { return true; }

    if (false == checking) {
        LogVerbose()(OT_PRETTY_CLASS())(
            "key not found by primary storage plugin.")
            .Flush();
    }

    for (const auto& plugin : backup_plugins_) {
        OT_ASSERT(plugin);

        if (plugin->Load(key, checking, value)) {
            auto notUsed = key;
            primary_plugin_->Store(false, value, notUsed);

            return true;
        }

        if (false == checking) {
            LogVerbose()(OT_PRETTY_CLASS())(
                "key not found by backup storage plugin ")
                .Flush();
        }
    }

    if (false == checking) {
        LogError()(OT_PRETTY_CLASS())("Key not found by any plugin.").Flush();
        OT_TRACE;

        throw std::runtime_error("Key not found by any plugin");
    }

    return false;
}

auto Multiplex::LoadFromBucket(
    const UnallocatedCString& key,
    UnallocatedCString& value,
    const bool bucket) const -> bool
{
    OT_ASSERT(primary_plugin_);

    if (primary_plugin_->LoadFromBucket(key, value, bucket)) { return true; }

    for (const auto& plugin : backup_plugins_) {
        OT_ASSERT(plugin);

        if (plugin->LoadFromBucket(key, value, bucket)) { return true; }
    }

    return false;
}

auto Multiplex::LoadRoot() const -> UnallocatedCString
{
    OT_ASSERT(primary_plugin_);

    UnallocatedCString root = primary_plugin_->LoadRoot();

    if (false == root.empty()) { return root; }

    for (const auto& plugin : backup_plugins_) {
        OT_ASSERT(plugin);

        root = plugin->LoadRoot();

        if (false == root.empty()) { return root; }
    }

    return root;
}

auto Multiplex::Migrate(
    const UnallocatedCString& key,
    const storage::Driver& to) const -> bool
{
    OT_ASSERT(primary_plugin_);

    if (primary_plugin_->Migrate(key, to)) { return true; }

    for (const auto& plugin : backup_plugins_) {
        OT_ASSERT(plugin);

        if (plugin->Migrate(key, to)) { return true; }
    }

    return false;
}

auto Multiplex::migrate_primary(
    const UnallocatedCString& from,
    const UnallocatedCString& to) -> void
{
    auto& old = primary_plugin_;

    init(from, old);

    OT_ASSERT(old);

    std::unique_ptr<storage::Plugin> newPlugin{nullptr};
    init(to, newPlugin);

    OT_ASSERT(newPlugin);

    const UnallocatedCString rootHash = old->LoadRoot();
    std::shared_ptr<storage::Root> root{nullptr};
    auto bucket = Flag::Factory(false);
    root.reset(new storage::Root(
        asio_,
        crypto_,
        factory_,
        *this,
        rootHash,
        std::numeric_limits<std::int64_t>::max(),
        bucket));

    OT_ASSERT(root);

    const auto& tree = root->Tree();
    const auto migrated = tree.Migrate(*newPlugin);

    if (migrated) {
        LogError()(OT_PRETTY_CLASS())(
            "Successfully migrated to new primary plugin.")
            .Flush();
    } else {
        LogError()(OT_PRETTY_CLASS())("Failed to migrate primary plugin.")
            .Flush();

        OT_FAIL;
    }

    Lock lock(root->write_lock_);
    root->save(lock, *newPlugin);
    newPlugin->StoreRoot(false, rootHash);
    old.reset(newPlugin.release());
}

auto Multiplex::Primary() -> storage::Driver&
{
    OT_ASSERT(primary_plugin_);

    return *primary_plugin_;
}

auto Multiplex::Store(
    const bool isTransaction,
    const UnallocatedCString& key,
    UnallocatedCString& value) const -> bool
{
    OT_ASSERT(primary_plugin_);

    bool output = primary_plugin_->Store(isTransaction, key, value);

    for (const auto& plugin : backup_plugins_) {
        OT_ASSERT(plugin);

        output |= plugin->Store(isTransaction, key, value);
    }

    return output;
}

auto Multiplex::StoreRoot(const bool commit, const UnallocatedCString& hash)
    const -> bool
{
    OT_ASSERT(primary_plugin_);

    for (const auto& plugin : backup_plugins_) {
        OT_ASSERT(plugin);

        plugin->StoreRoot(commit, hash);
    }

    return primary_plugin_->StoreRoot(commit, hash);
}

auto Multiplex::SynchronizePlugins(
    const UnallocatedCString& hash,
    const storage::Root& root,
    const bool syncPrimary) -> void
{
    const auto& tree = root.Tree();

    if (syncPrimary) {
        OT_ASSERT(primary_plugin_);

        LogError()(OT_PRETTY_CLASS())("Primary plugin is out of sync.").Flush();

        const auto migrated = tree.Migrate(*primary_plugin_);

        if (migrated) {
            LogError()(OT_PRETTY_CLASS())(
                "Successfully restored primary plugin from backup.")
                .Flush();
        } else {
            LogError()(OT_PRETTY_CLASS())(
                "Failed to restore primary plugin from backup.")
                .Flush();
        }
    }

    for (const auto& plugin : backup_plugins_) {
        OT_ASSERT(plugin);

        if (hash == plugin->LoadRoot()) { continue; }

        LogError()(OT_PRETTY_CLASS())(
            "Backup plugin is uninitialized or out of sync.")
            .Flush();

        if (tree.Migrate(*plugin)) {
            LogError()(OT_PRETTY_CLASS())(
                "Successfully initialized backup plugin.")
                .Flush();
        } else {
            LogError()(OT_PRETTY_CLASS())("Failed to initialize backup plugin.")
                .Flush();
        }

        if (false == root.Save(*plugin)) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed to update root index object for backup plugin.")
                .Flush();
        }

        if (false == plugin->StoreRoot(false, hash)) {
            LogError()(OT_PRETTY_CLASS())(
                "Failed to update root hash for backup plugin.")
                .Flush();
        }
    }
}

Multiplex::~Multiplex() { Cleanup_Multiplex(); }
}  // namespace opentxs::storage::driver
