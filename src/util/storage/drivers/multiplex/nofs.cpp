// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "util/storage/drivers/multiplex/Multiplex.hpp"  // IWYU pragma: associated

#include "internal/util/LogMacros.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::storage::driver
{
auto Multiplex::init_fs(std::unique_ptr<storage::Plugin>& plugin) -> void
{
    LogError()(OT_PRETTY_CLASS())("Filesystem driver not compiled in.").Flush();
}

auto Multiplex::init_fs_backup(const UnallocatedCString& dir) -> void
{
    return;
}
}  // namespace opentxs::storage::driver
