// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/blockoracle/Shared.hpp"  // IWYU pragma: associated

#include <algorithm>

namespace opentxs::blockchain::node::internal
{
auto BlockOracle::Shared::check_blocks(std::span<BlockData> v) const noexcept
    -> void
{
    std::for_each(v.begin(), v.end(), [this](auto& d) { check_block(d); });
}
}  // namespace opentxs::blockchain::node::internal
