// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/block/Factory.hpp"  // IWYU pragma: associated

#include "opentxs/blockchain/block/Block.hpp"

namespace opentxs::factory
{
auto BlockchainBlock(
    const api::Crypto&,
    const blockchain::Type,
    const ReadView,
    alloc::Strategy alloc) noexcept -> blockchain::block::Block
{
    return {alloc.result_};
}
}  // namespace opentxs::factory
