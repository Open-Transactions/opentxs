// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "internal/blockchain/bitcoin/block/Factory.hpp"  // IWYU pragma: associated

#include "blockchain/bitcoin/block/header/Header.hpp"

namespace opentxs::factory
{
auto BitcoinBlockHeader(
    const api::Crypto&,
    const blockchain::block::Header&,
    const std::uint32_t,
    const std::int32_t,
    blockchain::block::Hash&&,
    const AbortFunction) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::Header>
{
    return std::make_unique<blockchain::bitcoin::block::Header>();
}

auto BitcoinBlockHeader(
    const api::Crypto&,
    const proto::BlockchainBlockHeader&) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::Header>
{
    return std::make_unique<blockchain::bitcoin::block::Header>();
}

auto BitcoinBlockHeader(
    const api::Crypto&,
    const blockchain::Type,
    const ReadView raw) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::Header>
{
    return std::make_unique<blockchain::bitcoin::block::Header>();
}

auto BitcoinBlockHeader(
    const api::Crypto&,
    const blockchain::Type,
    const blockchain::block::Hash&,
    const blockchain::block::Hash&,
    const blockchain::block::Height) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::Header>
{
    return std::make_unique<blockchain::bitcoin::block::Header>();
}
}  // namespace opentxs::factory
