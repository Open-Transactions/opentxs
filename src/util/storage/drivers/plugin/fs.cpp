// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/drivers/plugin/Plugin.hpp"  // IWYU pragma: associated

#include <memory>

#include "internal/util/storage/drivers/Factory.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/storage/Driver.hpp"

namespace opentxs::storage::driver::implementation
{
auto Plugin::init_fs(std::unique_ptr<storage::Driver>& plugin) -> void
{
    LogVerbose()()("Initializing primary filesystem plugin.").Flush();
    plugin = factory::StorageFSGC(crypto_, config_);
}

auto Plugin::init_fs_backup(const std::filesystem::path& dir) -> void
{
    backup_drivers_.emplace_back(
        factory::StorageFSArchive(crypto_, config_, dir, null_));
}
}  // namespace opentxs::storage::driver::implementation
