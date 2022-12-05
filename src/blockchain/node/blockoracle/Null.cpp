// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/node/blockoracle/BlockOracle.hpp"  // IWYU pragma: associated

namespace opentxs::blockchain::node::internal
{
class BlockOracle::Shared
{
};

BlockOracle::BlockOracle() noexcept
    : shared_()
{
}

auto BlockOracle::DownloadQueue() const noexcept -> std::size_t { return {}; }

auto BlockOracle::Load(const block::Hash&) const noexcept -> BitcoinBlockResult
{
    return {};
}

auto BlockOracle::Load(std::span<const block::Hash>) const noexcept
    -> BitcoinBlockResults
{
    return {};
}

auto BlockOracle::Tip() const noexcept -> block::Position { return {}; }

BlockOracle::~BlockOracle() = default;
}  // namespace opentxs::blockchain::node::internal
