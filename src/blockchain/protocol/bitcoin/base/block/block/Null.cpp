// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/protocol/bitcoin/base/block/Factory.hpp"  // IWYU pragma: associated

#include "blockchain/block/block/BlockPrivate.hpp"
#include "opentxs/blockchain/block/Block.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Header.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Transaction.hpp"

namespace opentxs::factory
{
auto BitcoinBlock(
    const api::Crypto&,
    const blockchain::block::Header&,
    blockchain::protocol::bitcoin::base::block::Transaction,
    const std::uint32_t,
    std::span<blockchain::protocol::bitcoin::base::block::Transaction>,
    const std::int32_t,
    const AbortFunction,
    alloc::Strategy alloc) noexcept -> blockchain::block::Block
{
    return {alloc.result_};
}

auto BitcoinBlock(
    const api::Crypto&,
    const blockchain::Type,
    const ReadView,
    alloc::Strategy alloc) noexcept -> blockchain::block::Block
{
    return {alloc.result_};
}

auto BitcoinBlock(
    const blockchain::Type,
    blockchain::protocol::bitcoin::base::block::Header,
    blockchain::protocol::bitcoin::base::block::TxidIndex&&,
    blockchain::protocol::bitcoin::base::block::TxidIndex&&,
    blockchain::protocol::bitcoin::base::block::TransactionMap&&,
    std::optional<blockchain::protocol::bitcoin::base::block::CalculatedSize>&&,
    alloc::Strategy alloc) noexcept -> blockchain::block::BlockPrivate*
{
    return blockchain::block::BlockPrivate::Blank(alloc.result_);
}
}  // namespace opentxs::factory
