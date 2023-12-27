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

#include "internal/blockchain/protocol/bitcoin/base/block/Types.hpp"
#include "opentxs/Time.hpp"
#include "opentxs/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Types.hpp"
#include "opentxs/blockchain/block/Types.internal.hpp"
#include "opentxs/blockchain/crypto/Types.hpp"
#include "opentxs/blockchain/node/Types.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Script.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Types.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/script/Element.hpp"  // IWYU pragma: keep
#include "opentxs/blockchain/protocol/bitcoin/base/block/script/Types.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/util/Allocator.hpp"
#include "opentxs/util/Container.hpp"

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
namespace block
{
class Block;
class BlockPrivate;
class Hash;
class Header;
class HeaderPrivate;
}  // namespace block

namespace protocol
{
namespace bitcoin
{
namespace base
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
}  // namespace base

namespace bitcoincash
{
namespace token
{
namespace cashtoken
{
struct Value;
}  // namespace cashtoken
}  // namespace token
}  // namespace bitcoincash
}  // namespace bitcoin
}  // namespace protocol
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
    blockchain::protocol::bitcoin::base::block::Transaction
        generationTransaction,
    std::uint32_t nBits,
    std::span<blockchain::protocol::bitcoin::base::block::Transaction>
        extraTransactions,
    std::int32_t version,
    AbortFunction abort,
    alloc::Strategy alloc) noexcept -> blockchain::block::Block;
auto BitcoinBlock(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const ReadView in,
    alloc::Strategy alloc) noexcept -> blockchain::block::Block;
[[nodiscard]] auto BitcoinBlock(
    const blockchain::Type chain,
    blockchain::protocol::bitcoin::base::block::Header header,
    blockchain::protocol::bitcoin::base::block::TxidIndex&& ids,
    blockchain::protocol::bitcoin::base::block::TxidIndex&& hashes,
    blockchain::protocol::bitcoin::base::block::TransactionMap&& transactions,
    std::optional<blockchain::protocol::bitcoin::base::block::CalculatedSize>&&
        size,
    alloc::Strategy alloc) noexcept -> blockchain::block::BlockPrivate*;
[[nodiscard]] auto BitcoinBlockHeader(
    const api::Crypto& crypto,
    const blockchain::block::Header& previous,
    const std::uint32_t nBits,
    const std::int32_t version,
    blockchain::block::Hash&& merkle,
    const AbortFunction abort,
    alloc::Strategy alloc) noexcept -> blockchain::block::HeaderPrivate*;
[[nodiscard]] auto BitcoinBlockHeader(
    const api::Crypto& crypto,
    const proto::BlockchainBlockHeader& serialized,
    alloc::Strategy alloc) noexcept -> blockchain::block::HeaderPrivate*;
[[nodiscard]] auto BitcoinBlockHeader(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const ReadView bytes,
    alloc::Strategy alloc) noexcept -> blockchain::block::HeaderPrivate*;
[[nodiscard]] auto BitcoinBlockHeader(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const blockchain::block::Hash& hash,
    const blockchain::block::Hash& parent,
    const blockchain::block::Height height,
    alloc::Strategy alloc) noexcept -> blockchain::block::HeaderPrivate*;
auto BitcoinScript(
    const blockchain::Type chain,
    ReadView bytes,
    const blockchain::protocol::bitcoin::base::block::script::Position role,
    const bool allowInvalidOpcodes,
    const bool mute,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Script;
auto BitcoinScript(
    const blockchain::Type chain,
    Vector<blockchain::protocol::bitcoin::base::block::script::Element>
        elements,
    const blockchain::protocol::bitcoin::base::block::script::Position role,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Script;
auto BitcoinScriptNullData(
    const blockchain::Type chain,
    std::span<const ReadView> data,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Script;
auto BitcoinScriptP2MS(
    const blockchain::Type chain,
    const std::uint8_t M,
    const std::uint8_t N,
    std::span<const opentxs::crypto::asymmetric::key::EllipticCurve*> keys,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Script;
auto BitcoinScriptP2PK(
    const blockchain::Type chain,
    const opentxs::crypto::asymmetric::key::EllipticCurve& key,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Script;
auto BitcoinScriptP2PKH(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const opentxs::crypto::asymmetric::key::EllipticCurve& key,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Script;
auto BitcoinScriptP2SH(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const blockchain::protocol::bitcoin::base::block::Script& script,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Script;
auto BitcoinScriptP2WPKH(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const opentxs::crypto::asymmetric::key::EllipticCurve& key,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Script;
auto BitcoinScriptP2WSH(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const blockchain::protocol::bitcoin::base::block::Script& script,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Script;
[[nodiscard]] auto BitcoinTransaction(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const Time& time,
    const boost::endian::little_int32_buf_t& version,
    const boost::endian::little_uint32_buf_t lockTime,
    bool segwit,
    Vector<blockchain::protocol::bitcoin::base::block::Input> inputs,
    Vector<blockchain::protocol::bitcoin::base::block::Output> outputs,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::TransactionPrivate*;
auto BitcoinTransaction(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const std::size_t position,
    const Time& time,
    ReadView native,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Transaction;
[[nodiscard]] auto BitcoinTransaction(
    const blockchain::Type chain,
    const std::size_t position,
    const Time& time,
    blockchain::protocol::bitcoin::base::EncodedTransaction&& parsed,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::TransactionPrivate*;
[[nodiscard]] auto BitcoinTransaction(
    const api::crypto::Blockchain& crypto,
    const api::Factory& factory,
    const proto::BlockchainTransaction& serialized,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::TransactionPrivate*;
[[nodiscard]] auto BitcoinTransaction(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const blockchain::block::Height height,
    std::span<blockchain::OutputBuilder> scripts,
    ReadView coinbase,
    std::int32_t version,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::TransactionPrivate*;
auto BitcoinTransactionInput(
    const blockchain::Type chain,
    const ReadView outpoint,
    const network::blockchain::bitcoin::CompactSize& cs,
    const ReadView script,
    const ReadView sequence,
    const bool isGeneration,
    Vector<blockchain::protocol::bitcoin::base::block::WitnessItem> witness,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Input;
auto BitcoinTransactionInput(
    const blockchain::Type chain,
    const blockchain::node::UTXO& spends,
    const std::optional<std::uint32_t> sequence,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Input;
auto BitcoinTransactionInput(
    const api::crypto::Blockchain& crypto,
    const api::Factory& factory,
    const blockchain::Type chain,
    const proto::BlockchainTransactionInput&,
    const bool isGeneration,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Input;
auto BitcoinTransactionOutput(
    const blockchain::Type chain,
    const std::uint32_t index,
    const opentxs::Amount& value,
    blockchain::protocol::bitcoin::base::block::Script script,
    std::optional<const blockchain::protocol::bitcoin::bitcoincash::token::
                      cashtoken::Value> cashtoken,
    const UnallocatedSet<blockchain::crypto::Key>& keys,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Output;
auto BitcoinTransactionOutput(
    const blockchain::Type chain,
    const std::uint32_t index,
    const opentxs::Amount& value,
    const network::blockchain::bitcoin::CompactSize& cs,
    const ReadView script,
    std::optional<const blockchain::protocol::bitcoin::bitcoincash::token::
                      cashtoken::Value> cashtoken,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Output;
auto BitcoinTransactionOutput(
    const api::crypto::Blockchain& crypto,
    const api::Factory& factory,
    const blockchain::Type chain,
    const proto::BlockchainTransactionOutput& in,
    alloc::Strategy alloc) noexcept
    -> blockchain::protocol::bitcoin::base::block::Output;
}  // namespace opentxs::factory
