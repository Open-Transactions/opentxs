// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "internal/blockchain/bitcoin/block/Factory.hpp"  // IWYU pragma: associated

#include "blockchain/bitcoin/block/output/OutputPrivate.hpp"
#include "opentxs/blockchain/bitcoin/block/Output.hpp"

namespace opentxs::factory
{
auto BitcoinTransactionOutput(
    const blockchain::Type,
    const std::uint32_t,
    const opentxs::Amount&,
    blockchain::bitcoin::block::Script,
    const UnallocatedSet<blockchain::crypto::Key>&,
    alloc::Default alloc) noexcept -> blockchain::bitcoin::block::Output
{
    return blockchain::bitcoin::block::OutputPrivate::Blank(alloc);
}

auto BitcoinTransactionOutput(
    const blockchain::Type,
    const std::uint32_t,
    const opentxs::Amount&,
    const network::blockchain::bitcoin::CompactSize&,
    const ReadView,
    alloc::Default alloc) noexcept -> blockchain::bitcoin::block::Output
{
    return blockchain::bitcoin::block::OutputPrivate::Blank(alloc);
}

auto BitcoinTransactionOutput(
    const api::crypto::Blockchain&,
    const api::Factory&,
    const blockchain::Type,
    const proto::BlockchainTransactionOutput&,
    alloc::Default alloc) noexcept -> blockchain::bitcoin::block::Output
{
    return blockchain::bitcoin::block::OutputPrivate::Blank(alloc);
}
}  // namespace opentxs::factory
