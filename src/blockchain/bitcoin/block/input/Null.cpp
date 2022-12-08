// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "internal/blockchain/bitcoin/block/Factory.hpp"  // IWYU pragma: associated

#include "blockchain/bitcoin/block/input/InputPrivate.hpp"
#include "opentxs/blockchain/bitcoin/block/Input.hpp"

namespace opentxs::factory
{
auto BitcoinTransactionInput(
    const blockchain::Type,
    const blockchain::node::UTXO&,
    const std::optional<std::uint32_t>,
    alloc::Default alloc) noexcept -> blockchain::bitcoin::block::Input
{
    return blockchain::bitcoin::block::InputPrivate::Blank(alloc);
}

auto BitcoinTransactionInput(
    const blockchain::Type,
    const ReadView,
    const network::blockchain::bitcoin::CompactSize&,
    const ReadView,
    const ReadView,
    const bool,
    std::span<blockchain::bitcoin::block::WitnessItem>,
    alloc::Default alloc) noexcept -> blockchain::bitcoin::block::Input
{
    return blockchain::bitcoin::block::InputPrivate::Blank(alloc);
}

auto BitcoinTransactionInput(
    const api::crypto::Blockchain&,
    const api::Factory&,
    const blockchain::Type,
    const proto::BlockchainTransactionInput&,
    const bool,
    alloc::Default alloc) noexcept -> blockchain::bitcoin::block::Input
{
    return blockchain::bitcoin::block::InputPrivate::Blank(alloc);
}
}  // namespace opentxs::factory
