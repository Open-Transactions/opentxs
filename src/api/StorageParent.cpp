// Copyright (c) 2018 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "stdafx.hpp"

#include "Internal.hpp"

#include "opentxs/api/crypto/Crypto.hpp"
#if OT_CRYPTO_WITH_BIP39
#include "opentxs/api/HDSeed.hpp"
#endif
#include "opentxs/core/Identifier.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/core/Log.hpp"
#include "opentxs/OT.hpp"

#include "StorageParent.hpp"

#define STORAGE_CONFIG_KEY "storage"

#define OT_METHOD "opentxs::api::implementation::StorageParent::"

namespace opentxs::api::implementation
{
StorageParent::StorageParent(
    const Flag& running,
    const ArgList& args,
    const api::Crypto& crypto,
    const api::Settings& config,
    const std::string& dataFolder)
    : crypto_(crypto)
    , config_(config)
    , args_(args)
    , gc_interval_(0)
    , data_folder_(dataFolder)
    , storage_config_()
    , migrate_storage_{false}
    , migrate_from_{String::Factory()}
    , primary_storage_plugin_(get_primary_storage_plugin(
          config_,
          storage_config_,
          args_,
          migrate_storage_,
          migrate_from_))
    , archive_directory_(String::Factory())
    , encrypted_directory_(String::Factory())
    , storage_(opentxs::Factory::Storage(
          running,
          crypto_,
          config_,
          data_folder_,
          primary_storage_plugin_,
          archive_directory_,
          gc_interval_,
          encrypted_directory_,
          storage_config_))
#if OT_CRYPTO_WITH_BIP39
    , storage_encryption_key_(opentxs::crypto::key::Symmetric::Factory())
#endif
{
    OT_ASSERT(storage_);
    OT_ASSERT(false == data_folder_.empty())
}

void StorageParent::init([[maybe_unused]] const api::HDSeed& seeds)
{
    if (encrypted_directory_->empty()) { return; }

#if OT_CRYPTO_WITH_BIP39
    auto seed = seeds.DefaultSeed();

    if (seed.empty()) {
        otErr << OT_METHOD << __FUNCTION__ << ": No default seed." << std::endl;
    } else {
        otErr << OT_METHOD << __FUNCTION__ << ": Default seed is: " << seed
              << std::endl;
    }

    auto rawKey = seeds.GetStorageKey(seed);

    if (false == bool(rawKey)) {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to load encryption key."
              << std::endl;
    }

    storage_encryption_key_ = crypto_.GetStorageKey(*rawKey);

    if (storage_encryption_key_.get()) {
        LogDetail(OT_METHOD)(__FUNCTION__)(": Obtained storage key ")(
            storage_encryption_key_->ID())
            .Flush();
    } else {
        otErr << OT_METHOD << __FUNCTION__ << ": Failed to load storage key "
              << seed << std::endl;
    }
#endif

    start();
}

OTString StorageParent::extract_arg(
    const std::string& name,
    const ArgList& args)
{
    const auto it = args.find(name);

    if (args.end() == it) { return String::Factory(); }

    return String::Factory(*it->second.cbegin());
}

OTString StorageParent::extract_archive_directory(const ArgList& args)
{
    return extract_arg(OPENTXS_ARG_BACKUP_DIRECTORY, args);
}

OTString StorageParent::extract_encrypted_directory(const ArgList& args)
{
    return extract_arg(OPENTXS_ARG_ENCRYPTED_DIRECTORY, args);
}

OTString StorageParent::extract_primary_storage_plugin(const ArgList& args)
{
    return extract_arg(OPENTXS_ARG_STORAGE_PLUGIN, args);
}

OTString StorageParent::get_primary_storage_plugin(
    const api::Settings& config,
    const StorageConfig& storageConfig,
    const ArgList args,
    bool& migrate,
    String& previous)
{
    const auto hardcoded = String::Factory(storageConfig.primary_plugin_);
    const auto commandLine = extract_primary_storage_plugin(args);
    auto configured = String::Factory();
    bool notUsed{false};
    config.Check_str(
        String::Factory(STORAGE_CONFIG_KEY),
        String::Factory(STORAGE_CONFIG_PRIMARY_PLUGIN_KEY),
        configured,
        notUsed);
    const auto haveConfigured = configured->Exists();
    const auto haveCommandline = commandLine->Exists();
    const bool same = (configured.get() == commandLine.get());
    if (haveCommandline) {
        if (haveConfigured && (false == same)) {
            migrate = true;
            previous.Set(configured);
            otErr << OT_METHOD << __FUNCTION__ << ": Migrating from "
                  << previous << "." << std::endl;
        }

        return commandLine;
    } else {
        if (haveConfigured) {
            LogDetail(OT_METHOD)(__FUNCTION__)(": Using config file value.")
                .Flush();

            return configured;
        } else {
            LogDetail(OT_METHOD)(__FUNCTION__)(": Using default value.")
                .Flush();

            return hardcoded;
        }
    }
}

void StorageParent::start()
{
    OT_ASSERT(storage_);

    storage_->InitBackup();

#if OT_CRYPTO_WITH_BIP39
    if (storage_encryption_key_.get()) {
        storage_->InitEncryptedBackup(storage_encryption_key_);
    }
#endif

    storage_->start();
    storage_->UpgradeNyms();
}
}  // namespace opentxs::api::implementation
