// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::proto::ContactItemType

#include "internal/blockchain/bitcoin/block/Factory.hpp"  // IWYU pragma: associated

#include "blockchain/bitcoin/block/transaction/TransactionPrivate.hpp"
#include "opentxs/blockchain/bitcoin/block/Transaction.hpp"

namespace opentxs::factory
{
auto BitcoinTransaction(
    const api::Crypto&,
    const blockchain::Type,
    const Time&,
    const boost::endian::little_int32_buf_t&,
    const boost::endian::little_uint32_buf_t,
    bool,
    std::span<blockchain::bitcoin::block::Input>,
    std::span<blockchain::bitcoin::block::Output>,
    alloc::Default alloc) noexcept
    -> blockchain::bitcoin::block::TransactionPrivate*
{
    return blockchain::bitcoin::block::TransactionPrivate::Blank(alloc);
}

auto BitcoinTransaction(
    const api::Crypto&,
    const blockchain::Type,
    const std::size_t,
    const Time&,
    ReadView,
    alloc::Default alloc) noexcept -> blockchain::bitcoin::block::Transaction
{
    return alloc;
}

auto BitcoinTransaction(
    const blockchain::Type,
    const std::size_t,
    const Time&,
    blockchain::bitcoin::EncodedTransaction&&,
    alloc::Default alloc) noexcept
    -> blockchain::bitcoin::block::TransactionPrivate*
{
    return blockchain::bitcoin::block::TransactionPrivate::Blank(alloc);
}

auto BitcoinTransaction(
    const api::crypto::Blockchain&,
    const api::Factory&,
    const proto::BlockchainTransaction&,
    alloc::Default alloc) noexcept
    -> blockchain::bitcoin::block::TransactionPrivate*
{
    return blockchain::bitcoin::block::TransactionPrivate::Blank(alloc);
}

auto BitcoinTransaction(
    const api::Crypto&,
    const blockchain::Type,
    const blockchain::block::Height,
    std::span<blockchain::OutputBuilder>,
    ReadView,
    std::int32_t,
    alloc::Default alloc) noexcept
    -> blockchain::bitcoin::block::TransactionPrivate*
{
    return blockchain::bitcoin::block::TransactionPrivate::Blank(alloc);
}
}  // namespace opentxs::factory
