// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "ottest/fixtures/blockchain/Blocks.hpp"  // IWYU pragma: associated

#include "internal/blockchain/Params.hpp"
#include "internal/blockchain/block/Checker.hpp"
#include "opentxs/opentxs.hpp"

namespace ottest
{
auto BlockchainBlocks::CheckBlock(
    opentxs::blockchain::Type chain,
    const opentxs::blockchain::block::Hash& id,
    const opentxs::ReadView bytes) const noexcept -> bool
{
    return opentxs::blockchain::block::Checker::Check(
        ot_.Crypto(), chain, id, bytes);
}

auto BlockchainBlocks::CheckGenesisBlock(
    opentxs::blockchain::Type chain) const noexcept -> bool
{
    try {
        const auto& params = opentxs::blockchain::params::get(chain);
        const auto& block = params.GenesisBlock();
        const auto bytes = params.GenesisBlockSerialized();

        return opentxs::blockchain::block::Checker::Check(
            ot_.Crypto(), chain, block.ID(), bytes);
    } catch (...) {

        return false;
    }
}
}  // namespace ottest
