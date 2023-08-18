// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/drivers/plugin/Plugin.hpp"  // IWYU pragma: associated

#include "internal/util/LogMacros.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::storage::driver::implementation
{
auto Plugin::init_fs(std::unique_ptr<storage::Driver>& plugin) -> void
{
    LogError()(OT_PRETTY_CLASS())("Filesystem driver not compiled in.").Flush();
}

auto Plugin::init_fs_backup(const std::filesystem::path& dir) -> void
{
    LogError()(OT_PRETTY_CLASS())("Filesystem driver not compiled in.").Flush();
}
}  // namespace opentxs::storage::driver::implementation
