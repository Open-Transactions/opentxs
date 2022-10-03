// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "0_stdafx.hpp"  // IWYU pragma: associated
#include "internal/blockchain/bitcoin/block/Factory.hpp"  // IWYU pragma: associated

#include "internal/blockchain/bitcoin/block/Header.hpp"
#include "internal/blockchain/bitcoin/block/Input.hpp"
#include "internal/blockchain/bitcoin/block/Inputs.hpp"
#include "internal/blockchain/bitcoin/block/Output.hpp"
#include "internal/blockchain/bitcoin/block/Outputs.hpp"
#include "internal/blockchain/bitcoin/block/Script.hpp"
#include "internal/blockchain/bitcoin/block/Transaction.hpp"
#include "opentxs/blockchain/bitcoin/block/Header.hpp"
#include "opentxs/blockchain/bitcoin/block/Input.hpp"
#include "opentxs/blockchain/bitcoin/block/Inputs.hpp"
#include "opentxs/blockchain/bitcoin/block/Output.hpp"
#include "opentxs/blockchain/bitcoin/block/Outputs.hpp"
#include "opentxs/blockchain/bitcoin/block/Script.hpp"
#include "opentxs/blockchain/bitcoin/block/Transaction.hpp"

namespace opentxs::factory
{
auto BitcoinBlock(
    const api::Crypto&,
    const blockchain::block::Header&,
    const Transaction_p,
    const std::uint32_t,
    const UnallocatedVector<Transaction_p>&,
    const std::int32_t,
    const AbortFunction) noexcept
    -> std::shared_ptr<const blockchain::bitcoin::block::Block>
{
    return BitcoinBlock();
}

auto BitcoinBlock(
    const api::Crypto&,
    const blockchain::Type,
    const ReadView) noexcept
    -> std::shared_ptr<blockchain::bitcoin::block::Block>
{
    return BitcoinBlock();
}

auto BitcoinScript(
    const blockchain::Type,
    const ReadView,
    const blockchain::bitcoin::block::Script::Position,
    const bool,
    const bool) noexcept -> std::unique_ptr<blockchain::bitcoin::block::Script>
{
    return BitcoinScript();
}

auto BitcoinScript(
    const blockchain::Type,
    blockchain::bitcoin::block::ScriptElements&&,
    const blockchain::bitcoin::block::Script::Position) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::Script>
{
    return BitcoinScript();
}

auto BitcoinScriptNullData(
    const blockchain::Type,
    const UnallocatedVector<ReadView>&) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::Script>
{
    return BitcoinScript();
}

auto BitcoinScriptP2MS(
    const blockchain::Type,
    const std::uint8_t,
    const std::uint8_t,
    const UnallocatedVector<const opentxs::crypto::key::EllipticCurve*>&
        publicKeys) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::Script>
{
    return BitcoinScript();
}

auto BitcoinScriptP2PK(
    const blockchain::Type,
    const opentxs::crypto::key::EllipticCurve&) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::Script>
{
    return BitcoinScript();
}

auto BitcoinScriptP2PKH(
    const api::Crypto&,
    const blockchain::Type,
    const opentxs::crypto::key::EllipticCurve&) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::Script>
{
    return BitcoinScript();
}

auto BitcoinScriptP2SH(
    const api::Crypto&,
    const blockchain::Type,
    const blockchain::bitcoin::block::Script&) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::Script>
{
    return BitcoinScript();
}

auto BitcoinScriptP2WPKH(
    const api::Crypto&,
    const blockchain::Type,
    const opentxs::crypto::key::EllipticCurve&) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::Script>
{
    return BitcoinScript();
}

auto BitcoinScriptP2WSH(
    const api::Crypto&,
    const blockchain::Type,
    const blockchain::bitcoin::block::Script&) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::Script>
{
    return BitcoinScript();
}

auto BitcoinTransaction(
    const api::Crypto&,
    const blockchain::Type,
    const Time&,
    const boost::endian::little_int32_buf_t&,
    const boost::endian::little_uint32_buf_t,
    bool,
    std::unique_ptr<blockchain::bitcoin::block::internal::Inputs>,
    std::unique_ptr<blockchain::bitcoin::block::internal::Outputs>) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::internal::Transaction>
{
    return BitcoinTransaction();
}

auto BitcoinTransaction(
    const api::Crypto&,
    const blockchain::Type,
    const std::size_t,
    const Time&,
    ReadView) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::internal::Transaction>
{
    return BitcoinTransaction();
}

auto BitcoinTransaction(
    const blockchain::Type,
    const std::size_t,
    const Time&,
    blockchain::bitcoin::EncodedTransaction&&) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::internal::Transaction>
{
    return BitcoinTransaction();
}

auto BitcoinTransaction(
    const api::crypto::Blockchain&,
    const api::Factory&,
    const proto::BlockchainTransaction&) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::internal::Transaction>
{
    return BitcoinTransaction();
}

auto BitcoinTransaction(
    const api::Crypto&,
    const blockchain::Type,
    const blockchain::block::Height,
    UnallocatedVector<blockchain::OutputBuilder>&&,
    const UnallocatedCString&,
    const std::int32_t) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::internal::Transaction>
{
    return BitcoinTransaction();
}

auto BitcoinTransactionInput(
    const blockchain::Type,
    const ReadView,
    const network::blockchain::bitcoin::CompactSize&,
    const ReadView,
    const ReadView,
    const bool,
    UnallocatedVector<Space>&&) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::internal::Input>
{
    return BitcoinTransactionInput();
}

auto BitcoinTransactionInput(
    const blockchain::Type,
    const UTXO&,
    const std::optional<std::uint32_t>) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::internal::Input>
{
    return BitcoinTransactionInput();
}

auto BitcoinTransactionInput(
    const api::crypto::Blockchain&,
    const api::Factory&,
    const blockchain::Type,
    const proto::BlockchainTransactionInput&,
    const bool) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::internal::Input>
{
    return BitcoinTransactionInput();
}

auto BitcoinTransactionInputs(
    UnallocatedVector<
        std::unique_ptr<blockchain::bitcoin::block::internal::Input>>&&,
    std::optional<std::size_t>) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::internal::Inputs>
{
    return BitcoinTransactionInputs();
}

auto BitcoinTransactionOutput(
    const blockchain::Type,
    const std::uint32_t,
    const blockchain::Amount&,
    std::unique_ptr<const blockchain::bitcoin::block::Script>,
    const UnallocatedSet<blockchain::crypto::Key>&) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::internal::Output>
{
    return BitcoinTransactionOutput();
}

auto BitcoinTransactionOutput(
    const blockchain::Type,
    const std::uint32_t,
    const blockchain::Amount&,
    const network::blockchain::bitcoin::CompactSize&,
    const ReadView) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::internal::Output>
{
    return BitcoinTransactionOutput();
}

auto BitcoinTransactionOutput(
    const api::crypto::Blockchain&,
    const api::Factory&,
    const blockchain::Type,
    const proto::BlockchainTransactionOutput&) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::internal::Output>
{
    return BitcoinTransactionOutput();
}

auto BitcoinTransactionOutputs(
    UnallocatedVector<
        std::unique_ptr<blockchain::bitcoin::block::internal::Output>>&&,
    std::optional<std::size_t>) noexcept
    -> std::unique_ptr<blockchain::bitcoin::block::internal::Outputs>
{
    return BitcoinTransactionOutputs();
}
}  // namespace opentxs::factory
