// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/drivers/multiplex/Multiplex.hpp"  // IWYU pragma: associated

#include <memory>

#include "internal/util/LogMacros.hpp"
#include "internal/util/storage/drivers/Factory.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/storage/Plugin.hpp"

namespace opentxs::storage::driver
{
auto Multiplex::init_lmdb(std::unique_ptr<storage::Plugin>& plugin) -> void
{
    LogVerbose()(OT_PRETTY_CLASS())("Initializing primary LMDB plugin.")
        .Flush();
    plugin = factory::StorageLMDB(
        crypto_, asio_, storage_, config_, primary_bucket_);
}
}  // namespace opentxs::storage::driver
