// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/blockoracle/Shared.hpp"  // IWYU pragma: associated

#include "TBB.hpp"
#include "internal/util/P0330.hpp"

namespace opentxs::blockchain::node::internal
{
auto BlockOracle::Shared::check_blocks(std::span<BlockData> view) const noexcept
    -> void
{
    tbb::parallel_for(
        tbb::blocked_range<std::size_t>{0_uz, view.size()},
        [&, this](const auto& r) {
            for (auto i = r.begin(); i != r.end(); ++i) {
                check_block(view[i]);
            }
        });
}
}  // namespace opentxs::blockchain::node::internal
