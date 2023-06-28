// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "blockchain/node/blockoracle/Shared.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <span>

namespace opentxs::blockchain::node::internal
{
auto BlockOracle::Shared::check_blocks(std::span<BlockData> view) const noexcept
    -> void
{
    auto check = [this](auto& data) { check_block(data); };
    std::for_each(view.begin(), view.end(), check);
}
}  // namespace opentxs::blockchain::node::internal
