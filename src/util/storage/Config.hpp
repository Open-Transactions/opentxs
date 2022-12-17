// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstdint>
#include <filesystem>
#include <functional>

#include "opentxs/util/Container.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
constexpr auto OT_STORAGE_PRIMARY_PLUGIN_SQLITE = "sqlite";
constexpr auto OT_STORAGE_PRIMARY_PLUGIN_LMDB = "lmdb";
constexpr auto OT_STORAGE_PRIMARY_PLUGIN_MEMDB = "mem";
constexpr auto OT_STORAGE_PRIMARY_PLUGIN_FS = "fs";
constexpr auto STORAGE_CONFIG_PRIMARY_PLUGIN_KEY = "primary_plugin";
constexpr auto STORAGE_CONFIG_FS_BACKUP_DIRECTORY_KEY = "fs_backup_directory";
constexpr auto STORAGE_CONFIG_FS_ENCRYPTED_BACKUP_DIRECTORY_KEY =
    "fs_encrypted_backup";

namespace api
{
class Legacy;
class Settings;
}  // namespace api

class Options;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::storage
{
using InsertCB =
    std::function<void(const UnallocatedCString&, const UnallocatedCString&)>;

class Config
{
public:
    static const UnallocatedCString default_plugin_;

    UnallocatedCString previous_primary_plugin_;
    UnallocatedCString primary_plugin_;
    bool migrate_plugin_;

    std::int64_t gc_interval_;
    std::filesystem::path path_;

    std::filesystem::path fs_primary_bucket_;
    std::filesystem::path fs_secondary_bucket_;
    std::filesystem::path fs_root_file_;
    std::filesystem::path fs_backup_directory_;
    std::filesystem::path fs_encrypted_backup_directory_;

    UnallocatedCString sqlite3_primary_bucket_;
    UnallocatedCString sqlite3_secondary_bucket_;
    UnallocatedCString sqlite3_control_table_;
    UnallocatedCString sqlite3_root_key_;
    UnallocatedCString sqlite3_db_file_;

    UnallocatedCString lmdb_primary_bucket_;
    UnallocatedCString lmdb_secondary_bucket_;
    UnallocatedCString lmdb_control_table_;
    UnallocatedCString lmdb_root_key_;

    Config(
        const api::Legacy& legacy,
        const api::Settings& options,
        const Options& cli,
        const std::filesystem::path& dataFolder) noexcept;
};
}  // namespace opentxs::storage
