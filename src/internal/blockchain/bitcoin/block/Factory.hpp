// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <boost/endian/buffers.hpp>
#include <cstddef>
#include <cstdint>
#include <functional>
#include <optional>
#include <span>

#include "internal/blockchain/bitcoin/block/Types.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Script.hpp"
#include "opentxs/blockchain/bitcoin/block/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
namespace crypto
{
class Blockchain;
}  // namespace crypto

class Crypto;
class Factory;
}  // namespace api

namespace blockchain
{
namespace bitcoin
{
namespace block
{
class Header;
class Input;
class Output;
class Transaction;
class TransactionPrivate;
}  // namespace block

struct EncodedTransaction;
}  // namespace bitcoin

namespace block
{
class Block;
class BlockPrivate;
class Hash;
class Header;
class HeaderPrivate;
}  // namespace block
}  // namespace blockchain

namespace crypto
{
namespace asymmetric
{
namespace key
{
class EllipticCurve;
}  // namespace key
}  // namespace asymmetric
}  // namespace crypto

namespace network
{
namespace blockchain
{
namespace bitcoin
{
class CompactSize;
}  // namespace bitcoin
}  // namespace blockchain
}  // namespace network

namespace proto
{
class BlockchainBlockHeader;
class BlockchainTransaction;
class BlockchainTransactionInput;
class BlockchainTransactionOutput;
}  // namespace proto

}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::factory
{
using AbortFunction = std::function<bool()>;

auto BitcoinBlock(
    const api::Crypto& crypto,
    const blockchain::block::Header& previous,
    blockchain::bitcoin::block::Transaction generationTransaction,
    std::uint32_t nBits,
    std::span<blockchain::bitcoin::block::Transaction> extraTransactions,
    std::int32_t version,
    AbortFunction abort,
    alloc::Default alloc) noexcept -> blockchain::block::Block;
auto BitcoinBlock(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const ReadView in,
    alloc::Default alloc) noexcept -> blockchain::block::Block;
[[nodiscard]] auto BitcoinBlock(
    const blockchain::Type chain,
    blockchain::bitcoin::block::Header header,
    blockchain::bitcoin::block::TxidIndex&& ids,
    blockchain::bitcoin::block::TxidIndex&& hashes,
    blockchain::bitcoin::block::TransactionMap&& transactions,
    std::optional<blockchain::bitcoin::block::CalculatedSize>&& size,
    alloc::Default alloc) noexcept -> blockchain::block::BlockPrivate*;
[[nodiscard]] auto BitcoinBlockHeader(
    const api::Crypto& crypto,
    const blockchain::block::Header& previous,
    const std::uint32_t nBits,
    const std::int32_t version,
    blockchain::block::Hash&& merkle,
    const AbortFunction abort,
    alloc::Default alloc) noexcept -> blockchain::block::HeaderPrivate*;
[[nodiscard]] auto BitcoinBlockHeader(
    const api::Crypto& crypto,
    const proto::BlockchainBlockHeader& serialized,
    alloc::Default alloc) noexcept -> blockchain::block::HeaderPrivate*;
[[nodiscard]] auto BitcoinBlockHeader(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const ReadView bytes,
    alloc::Default alloc) noexcept -> blockchain::block::HeaderPrivate*;
[[nodiscard]] auto BitcoinBlockHeader(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const blockchain::block::Hash& hash,
    const blockchain::block::Hash& parent,
    const blockchain::block::Height height,
    alloc::Default alloc) noexcept -> blockchain::block::HeaderPrivate*;
auto BitcoinScript(
    const blockchain::Type chain,
    ReadView bytes,
    const blockchain::bitcoin::block::script::Position role,
    const bool allowInvalidOpcodes,
    const bool mute,
    alloc::Default alloc) noexcept -> blockchain::bitcoin::block::Script;
auto BitcoinScript(
    const blockchain::Type chain,
    std::span<blockchain::bitcoin::block::script::Element> elements,
    const blockchain::bitcoin::block::script::Position role,
    alloc::Default alloc) noexcept -> blockchain::bitcoin::block::Script;
auto BitcoinScriptNullData(
    const blockchain::Type chain,
    std::span<const ReadView> data,
    alloc::Default alloc) noexcept -> blockchain::bitcoin::block::Script;
auto BitcoinScriptP2MS(
    const blockchain::Type chain,
    const std::uint8_t M,
    const std::uint8_t N,
    std::span<const opentxs::crypto::asymmetric::key::EllipticCurve*> keys,
    alloc::Default alloc) noexcept -> blockchain::bitcoin::block::Script;
auto BitcoinScriptP2PK(
    const blockchain::Type chain,
    const opentxs::crypto::asymmetric::key::EllipticCurve& key,
    alloc::Default alloc) noexcept -> blockchain::bitcoin::block::Script;
auto BitcoinScriptP2PKH(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const opentxs::crypto::asymmetric::key::EllipticCurve& key,
    alloc::Default alloc) noexcept -> blockchain::bitcoin::block::Script;
auto BitcoinScriptP2SH(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const blockchain::bitcoin::block::Script& script,
    alloc::Default alloc) noexcept -> blockchain::bitcoin::block::Script;
auto BitcoinScriptP2WPKH(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const opentxs::crypto::asymmetric::key::EllipticCurve& key,
    alloc::Default alloc) noexcept -> blockchain::bitcoin::block::Script;
auto BitcoinScriptP2WSH(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const blockchain::bitcoin::block::Script& script,
    alloc::Default alloc) noexcept -> blockchain::bitcoin::block::Script;
[[nodiscard]] auto BitcoinTransaction(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const Time& time,
    const boost::endian::little_int32_buf_t& version,
    const boost::endian::little_uint32_buf_t lockTime,
    bool segwit,
    std::span<blockchain::bitcoin::block::Input> inputs,
    std::span<blockchain::bitcoin::block::Output> outputs,
    alloc::Default alloc) noexcept
    -> blockchain::bitcoin::block::TransactionPrivate*;
auto BitcoinTransaction(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const std::size_t position,
    const Time& time,
    ReadView native,
    alloc::Default alloc) noexcept -> blockchain::bitcoin::block::Transaction;
[[nodiscard]] auto BitcoinTransaction(
    const blockchain::Type chain,
    const std::size_t position,
    const Time& time,
    blockchain::bitcoin::EncodedTransaction&& parsed,
    alloc::Default alloc) noexcept
    -> blockchain::bitcoin::block::TransactionPrivate*;
[[nodiscard]] auto BitcoinTransaction(
    const api::crypto::Blockchain& crypto,
    const api::Factory& factory,
    const proto::BlockchainTransaction& serialized,
    alloc::Default alloc) noexcept
    -> blockchain::bitcoin::block::TransactionPrivate*;
[[nodiscard]] auto BitcoinTransaction(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const blockchain::block::Height height,
    std::span<blockchain::OutputBuilder> scripts,
    ReadView coinbase,
    std::int32_t version,
    alloc::Default alloc) noexcept
    -> blockchain::bitcoin::block::TransactionPrivate*;
auto BitcoinTransactionInput(
    const blockchain::Type chain,
    const ReadView outpoint,
    const network::blockchain::bitcoin::CompactSize& cs,
    const ReadView script,
    const ReadView sequence,
    const bool isGeneration,
    std::span<blockchain::bitcoin::block::WitnessItem> witness,
    alloc::Default alloc) noexcept -> blockchain::bitcoin::block::Input;
auto BitcoinTransactionInput(
    const blockchain::Type chain,
    const blockchain::node::UTXO& spends,
    const std::optional<std::uint32_t> sequence,
    alloc::Default alloc) noexcept -> blockchain::bitcoin::block::Input;
auto BitcoinTransactionInput(
    const api::crypto::Blockchain& crypto,
    const api::Factory& factory,
    const blockchain::Type chain,
    const proto::BlockchainTransactionInput&,
    const bool isGeneration,
    alloc::Default alloc) noexcept -> blockchain::bitcoin::block::Input;
auto BitcoinTransactionOutput(
    const blockchain::Type chain,
    const std::uint32_t index,
    const opentxs::Amount& value,
    blockchain::bitcoin::block::Script script,
    const UnallocatedSet<blockchain::crypto::Key>& keys,
    alloc::Default alloc) noexcept -> blockchain::bitcoin::block::Output;
auto BitcoinTransactionOutput(
    const blockchain::Type chain,
    const std::uint32_t index,
    const opentxs::Amount& value,
    const network::blockchain::bitcoin::CompactSize& cs,
    const ReadView script,
    alloc::Default alloc) noexcept -> blockchain::bitcoin::block::Output;
auto BitcoinTransactionOutput(
    const api::crypto::Blockchain& crypto,
    const api::Factory& factory,
    const blockchain::Type chain,
    const proto::BlockchainTransactionOutput& in,
    alloc::Default alloc) noexcept -> blockchain::bitcoin::block::Output;
}  // namespace opentxs::factory
