// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "util/storage/drivers/multiplex/Multiplex.hpp"  // IWYU pragma: associated

#include <atomic>
#include <cstddef>
#include <memory>

#include "TBB.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/util/Container.hpp"

namespace opentxs::storage::driver
{
auto Multiplex::Store(
    const bool isTransaction,
    const UnallocatedCString& key,
    const UnallocatedCString& value,
    const bool bucket) const -> bool
{
    OT_ASSERT(primary_plugin_);

    auto success = std::atomic_bool{false};
    tbb::parallel_for(
        tbb::blocked_range<std::size_t>{0_uz, plugins_.size()},
        [&, this](const auto& r) {
            for (auto i = r.begin(); i != r.end(); ++i) {
                const auto* plugin = plugins_[i];

                if (plugin->Store(isTransaction, key, value, bucket)) {
                    success.store(true);
                }
            }
        });

    return success.load();
}
}  // namespace opentxs::storage::driver
