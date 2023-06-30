// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/bitcoin/block/Factory.hpp"  // IWYU pragma: associated

#include "blockchain/bitcoin/block/output/OutputPrivate.hpp"
#include "internal/blockchain/token/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Output.hpp"

namespace opentxs::factory
{
auto BitcoinTransactionOutput(
    const blockchain::Type,
    const std::uint32_t,
    const opentxs::Amount&,
    blockchain::bitcoin::block::Script,
    std::optional<const blockchain::token::cashtoken::Value>,
    const UnallocatedSet<blockchain::crypto::Key>&,
    alloc::Strategy alloc) noexcept -> blockchain::bitcoin::block::Output
{
    return blockchain::bitcoin::block::OutputPrivate::Blank(alloc.result_);
}

auto BitcoinTransactionOutput(
    const blockchain::Type,
    const std::uint32_t,
    const opentxs::Amount&,
    const network::blockchain::bitcoin::CompactSize&,
    const ReadView,
    std::optional<const blockchain::token::cashtoken::Value>,
    alloc::Strategy alloc) noexcept -> blockchain::bitcoin::block::Output
{
    return blockchain::bitcoin::block::OutputPrivate::Blank(alloc.result_);
}

auto BitcoinTransactionOutput(
    const api::crypto::Blockchain&,
    const api::Factory&,
    const blockchain::Type,
    const proto::BlockchainTransactionOutput&,
    alloc::Strategy alloc) noexcept -> blockchain::bitcoin::block::Output
{
    return blockchain::bitcoin::block::OutputPrivate::Blank(alloc.result_);
}
}  // namespace opentxs::factory
