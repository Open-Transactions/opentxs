// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/bitcoin/block/Factory.hpp"  // IWYU pragma: associated

#include "blockchain/block/header/HeaderPrivate.hpp"

namespace opentxs::factory
{
auto BitcoinBlockHeader(
    const api::Crypto&,
    const blockchain::block::Header&,
    const std::uint32_t,
    const std::int32_t,
    blockchain::block::Hash&&,
    const AbortFunction,
    alloc::Default alloc) noexcept -> blockchain::block::HeaderPrivate*
{
    return blockchain::block::HeaderPrivate::Blank(alloc);
}

auto BitcoinBlockHeader(
    const api::Crypto&,
    const proto::BlockchainBlockHeader&,
    alloc::Default alloc) noexcept -> blockchain::block::HeaderPrivate*
{
    return blockchain::block::HeaderPrivate::Blank(alloc);
}

auto BitcoinBlockHeader(
    const api::Crypto&,
    const blockchain::Type,
    const ReadView raw,
    alloc::Default alloc) noexcept -> blockchain::block::HeaderPrivate*
{
    return blockchain::block::HeaderPrivate::Blank(alloc);
}

auto BitcoinBlockHeader(
    const api::Crypto&,
    const blockchain::Type,
    const blockchain::block::Hash&,
    const blockchain::block::Hash&,
    const blockchain::block::Height,
    alloc::Default alloc) noexcept -> blockchain::block::HeaderPrivate*
{
    return blockchain::block::HeaderPrivate::Blank(alloc);
}
}  // namespace opentxs::factory
