// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"             // IWYU pragma: associated
#include "1_Internal.hpp"           // IWYU pragma: associated
#include "util/storage/Config.hpp"  // IWYU pragma: associated

#include <chrono>

#include "internal/api/Legacy.hpp"
#include "internal/util/LogMacros.hpp"
#include "opentxs/api/Settings.hpp"
#include "opentxs/core/String.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Options.hpp"
#include "opentxs/util/Pimpl.hpp"

namespace C = std::chrono;

#define STORAGE_CONFIG_KEY "storage"

namespace opentxs::storage
{
Config::Config(
    const api::Legacy& legacy,
    const api::Settings& config,
    const Options& args,
    const String& dataFolder) noexcept
    : previous_primary_plugin_([&]() -> UnallocatedCString {
        auto exists{false};
        auto value = String::Factory();
        config.Check_str(
            String::Factory(STORAGE_CONFIG_KEY),
            String::Factory(STORAGE_CONFIG_PRIMARY_PLUGIN_KEY),
            value,
            exists);

        if (exists) {

            return value->Get();
        } else {

            return {};
        }
    }())
    , primary_plugin_([&] {
        const auto cli = UnallocatedCString{args.StoragePrimaryPlugin()};
        const auto& configFile = previous_primary_plugin_;

        if (false == cli.empty()) {
            LogDetail()(OT_PRETTY_CLASS())("Using ")(
                cli)(" as primary storage plugin based on initialization "
                     "arguments")
                .Flush();

            return cli;
        }

        if (false == configFile.empty()) {
            LogDetail()(OT_PRETTY_CLASS())("Using ")(
                cli)(" as primary storage plugin based saved configuration")
                .Flush();

            return configFile;
        }

        LogDetail()(OT_PRETTY_CLASS())("Using ")(
            cli)(" as primary storage plugin")
            .Flush();

        return default_plugin_;
    }())
    , migrate_plugin_([&]() -> bool {
        const auto& previous = previous_primary_plugin_;
        const auto& current = primary_plugin_;

        if (previous != current) {
            auto notUsed{false};
            config.Set_str(
                String::Factory(STORAGE_CONFIG_KEY),
                String::Factory(STORAGE_CONFIG_PRIMARY_PLUGIN_KEY),
                String::Factory(current),
                notUsed);
            const auto migrate = false == previous.empty();

            if (migrate) {
                LogError()(OT_PRETTY_CLASS())(
                    "Migrating primary storage plugin from ")(previous)(" to ")(
                    current)
                    .Flush();
            }

            return migrate;
        } else {

            return false;
        }
    }())
    , auto_publish_nyms_([&] {
        auto output{false};
        auto notUsed{false};
        config.CheckSet_bool(
            String::Factory(STORAGE_CONFIG_KEY),
            String::Factory("auto_publish_nyms"),
            true,
            output,
            notUsed);

        return output;
    }())
    , auto_publish_servers_([&] {
        auto output{false};
        auto notUsed{false};
        config.CheckSet_bool(
            String::Factory(STORAGE_CONFIG_KEY),
            String::Factory("auto_publish_servers"),
            true,
            output,
            notUsed);

        return output;
    }())
    , auto_publish_units_([&] {
        auto output{false};
        auto notUsed{false};
        config.CheckSet_bool(
            String::Factory(STORAGE_CONFIG_KEY),
            String::Factory("auto_publish_units"),
            true,
            output,
            notUsed);

        return output;
    }())
    , gc_interval_([&] {
        auto output = std::int64_t{};
        auto notUsed{false};
        config.CheckSet_long(
            String::Factory(STORAGE_CONFIG_KEY),
            String::Factory("gc_interval"),
            0,
            output,
            notUsed);

        return output;
    }())
    , path_([&]() -> UnallocatedCString {
        auto output = String::Factory();
        auto notUsed{false};

        if (false ==
            legacy.AppendFolder(
                output, dataFolder, String::Factory(legacy.Common()))) {
            LogError()(OT_PRETTY_CLASS())("Failed to calculate storage path")
                .Flush();

            return {};
        }

        if (false == legacy.BuildFolderPath(output)) {
            LogError()(OT_PRETTY_CLASS())("Failed to construct storage path")
                .Flush();

            return {};
        }

        if (primary_plugin_ == OT_STORAGE_PRIMARY_PLUGIN_LMDB) {
            auto newPath = String::Factory();
            const auto subdir = UnallocatedCString{legacy.Common()} + "_lmdb";

            if (false ==
                legacy.AppendFolder(newPath, output, String::Factory(subdir))) {
                LogError()(OT_PRETTY_CLASS())(
                    "Failed to calculate lmdb storage path")
                    .Flush();

                return {};
            }

            if (false == legacy.BuildFolderPath(newPath)) {
                LogError()(OT_PRETTY_CLASS())(
                    "Failed to construct lmdb storage path")
                    .Flush();

                return {};
            }

            output = newPath;
        }

        config.CheckSet_str(
            String::Factory(STORAGE_CONFIG_KEY),
            String::Factory("path"),
            output,
            output,
            notUsed);

        return output->Get();
    }())
    , dht_callback_()
    , fs_primary_bucket_([&] {
        auto output = UnallocatedCString{};
        auto notUsed{false};
        config.CheckSet_str(
            String::Factory(STORAGE_CONFIG_KEY),
            String::Factory("fs_primary"),
            String::Factory("a"),
            output,
            notUsed);

        return output;
    }())
    , fs_secondary_bucket_([&] {
        auto output = UnallocatedCString{};
        auto notUsed{false};
        config.CheckSet_str(
            String::Factory(STORAGE_CONFIG_KEY),
            String::Factory("fs_secondary"),
            String::Factory("b"),
            output,
            notUsed);

        return output;
    }())
    , fs_root_file_([&] {
        auto output = UnallocatedCString{};
        auto notUsed{false};
        config.CheckSet_str(
            String::Factory(STORAGE_CONFIG_KEY),
            String::Factory("fs_root_file"),
            String::Factory("root"),
            output,
            notUsed);

        return output;
    }())
    , fs_backup_directory_([&] {
        auto output = UnallocatedCString{};
        auto notUsed{false};
        config.CheckSet_str(
            String::Factory(STORAGE_CONFIG_KEY),
            String::Factory(STORAGE_CONFIG_FS_BACKUP_DIRECTORY_KEY),
            String::Factory(""),
            output,
            notUsed);

        return output;
    }())
    , fs_encrypted_backup_directory_([&] {
        auto output = UnallocatedCString{};
        auto notUsed{false};
        config.CheckSet_str(
            String::Factory(STORAGE_CONFIG_KEY),
            String::Factory(STORAGE_CONFIG_FS_ENCRYPTED_BACKUP_DIRECTORY_KEY),
            String::Factory(""),
            output,
            notUsed);

        return output;
    }())
    , sqlite3_primary_bucket_([&] {
        auto output = UnallocatedCString{};
        auto notUsed{false};
        config.CheckSet_str(
            String::Factory(STORAGE_CONFIG_KEY),
            String::Factory("sqlite3_primary"),
            String::Factory("a"),
            output,
            notUsed);

        return output;
    }())
    , sqlite3_secondary_bucket_([&] {
        auto output = UnallocatedCString{};
        auto notUsed{false};
        config.CheckSet_str(
            String::Factory(STORAGE_CONFIG_KEY),
            String::Factory("sqlite3_secondary"),
            String::Factory("b"),
            output,
            notUsed);

        return output;
    }())
    , sqlite3_control_table_([&] {
        auto output = UnallocatedCString{};
        auto notUsed{false};
        config.CheckSet_str(
            String::Factory(STORAGE_CONFIG_KEY),
            String::Factory("sqlite3_control"),
            String::Factory("control"),
            output,
            notUsed);

        return output;
    }())
    , sqlite3_root_key_([&] {
        auto output = UnallocatedCString{};
        auto notUsed{false};
        config.CheckSet_str(
            String::Factory(STORAGE_CONFIG_KEY),
            String::Factory("sqlite3_root_key"),
            String::Factory("root"),
            output,
            notUsed);

        return output;
    }())
    , sqlite3_db_file_([&] {
        auto output = UnallocatedCString{};
        auto notUsed{false};
        config.CheckSet_str(
            String::Factory(STORAGE_CONFIG_KEY),
            String::Factory("sqlite3_db_file"),
            String::Factory("opentxs.sqlite3"),
            output,
            notUsed);

        return output;
    }())
    , lmdb_primary_bucket_([&] {
        auto output = UnallocatedCString{};
        auto notUsed{false};
        config.CheckSet_str(
            String::Factory(STORAGE_CONFIG_KEY),
            String::Factory("lmdb_primary"),
            String::Factory("a"),
            output,
            notUsed);

        return output;
    }())
    , lmdb_secondary_bucket_([&] {
        auto output = UnallocatedCString{};
        auto notUsed{false};
        config.CheckSet_str(
            String::Factory(STORAGE_CONFIG_KEY),
            String::Factory("lmdb_secondary"),
            String::Factory("b"),
            output,
            notUsed);

        return output;
    }())
    , lmdb_control_table_([&] {
        auto output = UnallocatedCString{};
        auto notUsed{false};
        config.CheckSet_str(
            String::Factory(STORAGE_CONFIG_KEY),
            String::Factory("lmdb_control"),
            String::Factory("control"),
            output,
            notUsed);

        return output;
    }())
    , lmdb_root_key_([&] {
        auto output = UnallocatedCString{};
        auto notUsed{false};
        config.CheckSet_str(
            String::Factory(STORAGE_CONFIG_KEY),
            String::Factory("lmdb_root_key"),
            String::Factory("root"),
            output,
            notUsed);

        return output;
    }())
{
    OT_ASSERT(dataFolder.Exists())

    config.Save();
}
}  // namespace opentxs::storage
