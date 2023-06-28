// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/drivers/multiplex/Multiplex.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <memory>

#include "internal/util/LogMacros.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/storage/Plugin.hpp"

namespace opentxs::storage::driver
{
auto Multiplex::Store(
    const bool isTransaction,
    const UnallocatedCString& key,
    const UnallocatedCString& value,
    const bool bucket) const -> bool
{
    OT_ASSERT(primary_plugin_);

    auto success = false;
    auto store = [&](auto& plugin) {
        success |= plugin->Store(isTransaction, key, value, bucket);
    };
    std::for_each(plugins_.begin(), plugins_.end(), store);

    return success;
}
}  // namespace opentxs::storage::driver
