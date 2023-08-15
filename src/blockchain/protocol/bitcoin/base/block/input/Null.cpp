// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/protocol/bitcoin/base/block/Factory.hpp"  // IWYU pragma: associated

#include "blockchain/protocol/bitcoin/base/block/input/InputPrivate.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Input.hpp"

namespace opentxs::factory
{
auto BitcoinTransactionInput(
    const blockchain::Type,
    const blockchain::node::UTXO&,
    const std::optional<std::uint32_t>,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Input
{
    return blockchain::protocol::bitcoin::base::block::InputPrivate::Blank(
        alloc.result_);
}

auto BitcoinTransactionInput(
    const blockchain::Type,
    const ReadView,
    const network::blockchain::bitcoin::CompactSize&,
    const ReadView,
    const ReadView,
    const bool,
    Vector<blockchain::protocol::bitcoin::base::block::WitnessItem>,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Input
{
    return blockchain::protocol::bitcoin::base::block::InputPrivate::Blank(
        alloc.result_);
}

auto BitcoinTransactionInput(
    const api::crypto::Blockchain&,
    const api::Factory&,
    const blockchain::Type,
    const proto::BlockchainTransactionInput&,
    const bool,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Input
{
    return blockchain::protocol::bitcoin::base::block::InputPrivate::Blank(
        alloc.result_);
}
}  // namespace opentxs::factory
