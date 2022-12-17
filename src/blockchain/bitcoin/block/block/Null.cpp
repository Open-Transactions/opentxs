// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/bitcoin/block/Factory.hpp"  // IWYU pragma: associated

#include "blockchain/block/block/BlockPrivate.hpp"
#include "opentxs/blockchain/bitcoin/block/Header.hpp"
#include "opentxs/blockchain/bitcoin/block/Transaction.hpp"
#include "opentxs/blockchain/block/Block.hpp"

namespace opentxs::factory
{
auto BitcoinBlock(
    const api::Crypto&,
    const blockchain::block::Header&,
    blockchain::bitcoin::block::Transaction,
    const std::uint32_t,
    std::span<blockchain::bitcoin::block::Transaction>,
    const std::int32_t,
    const AbortFunction,
    alloc::Default alloc) noexcept -> blockchain::block::Block
{
    return {alloc};
}

auto BitcoinBlock(
    const api::Crypto&,
    const blockchain::Type,
    const ReadView,
    alloc::Default alloc) noexcept -> blockchain::block::Block
{
    return {alloc};
}

auto BitcoinBlock(
    const blockchain::Type,
    blockchain::bitcoin::block::Header,
    blockchain::bitcoin::block::TxidIndex&&,
    blockchain::bitcoin::block::TxidIndex&&,
    blockchain::bitcoin::block::TransactionMap&&,
    std::optional<blockchain::bitcoin::block::CalculatedSize>&&,
    alloc::Default alloc) noexcept -> blockchain::block::BlockPrivate*
{
    return blockchain::block::BlockPrivate::Blank(alloc);
}
}  // namespace opentxs::factory
