// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type
// IWYU pragma: no_include "opentxs/blockchain/BlockchainType.hpp"

#pragma once

#include <cstddef>
#include <memory>
#include <string_view>

#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/bitcoin/block/Types.hpp"
#include "internal/blockchain/block/Parser.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Time.hpp"
#include "opentxs/util/Types.hpp"

// NOLINTBEGIN(modernize-concat-nested-namespaces)
namespace opentxs
{
namespace api
{
class Crypto;
}  // namespace api

namespace blockchain
{
namespace bitcoin
{
namespace block
{
namespace internal
{
class Transaction;
}  // namespace internal

class Block;
class Header;
class Transaction;
}  // namespace block
}  // namespace bitcoin

namespace block
{
class Hash;
}  // namespace block
}  // namespace blockchain

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

class ByteArray;
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::bitcoin::block
{
using network::blockchain::bitcoin::CompactSize;

class ParserBase : virtual public blockchain::block::Parser
{
public:
    [[nodiscard]] auto operator()(
        const Hash& expected,
        ReadView bytes) && noexcept -> bool final;
    [[nodiscard]] auto operator()(
        ReadView bytes,
        std::shared_ptr<Block>& out) && noexcept -> bool final;
    [[nodiscard]] auto operator()(
        const std::size_t position,
        const Time& time,
        ReadView bytes,
        std::unique_ptr<bitcoin::block::internal::Transaction>& out) && noexcept
        -> bool final;

    ParserBase() = delete;
    ParserBase(const api::Crypto& crypto, blockchain::Type type) noexcept;
    ParserBase(const ParserBase&) = delete;
    ParserBase(ParserBase&&) = delete;
    auto operator=(const ParserBase&) -> ParserBase& = delete;
    auto operator=(ParserBase&&) -> ParserBase& = delete;

    ~ParserBase() override;

protected:
    enum class Mode : bool { checking, constructing };

    const api::Crypto& crypto_;
    const blockchain::Type chain_;
    ReadView data_;
    std::size_t bytes_;
    std::unique_ptr<Header> header_;
    Vector<Hash> txids_;
    Vector<EncodedTransaction> transactions_;

    auto check(std::string_view message, std::size_t required) const
        noexcept(false) -> void;
    auto constructing() const noexcept { return Mode::constructing == mode_; }

    virtual auto find_payload() noexcept -> bool;
    auto get_transactions() noexcept(false)
        -> Map<ReadView, std::shared_ptr<const block::Transaction>>;
    auto parse_size(
        std::string_view message,
        ByteArray* preimage,
        CompactSize* out) noexcept(false) -> std::size_t;

private:
    Mode mode_;
    Hash block_hash_;
    Hash merkle_root_;
    Hash witness_reserved_value_;
    Hash segwit_commitment_;
    std::size_t transaction_count_;
    Vector<Hash> wtxids_;
    bool has_segwit_commitment_;
    bool has_segwit_transactions_;
    bool has_segwit_reserved_value_;

    auto calculate_committment() const noexcept -> Hash;
    auto calculate_merkle() const noexcept -> Hash;
    auto calculate_witness() const noexcept -> Hash;
    auto compare_header_to_hash(const Hash& expected) const noexcept -> bool;
    auto compare_merkle_to_header() const noexcept -> bool;
    auto compare_segwit_to_commitment() const noexcept -> bool;
    auto is_segwit_tx(EncodedTransaction* out) const noexcept -> bool;

    auto calculate_hash(const ReadView header) noexcept -> bool;
    auto calculate_txids(
        const ReadView tx,
        bool isGeneration,
        ByteArray* preimage,
        bool haveWitnesses,
        EncodedTransaction* out) noexcept(false) -> void;
    virtual auto construct_block(std::shared_ptr<Block>& out) noexcept
        -> bool = 0;
    auto parse(
        const Hash& expected,
        ReadView bytes,
        std::shared_ptr<Block>& out) noexcept -> bool;
    auto parse_header() noexcept -> bool;
    auto parse_inputs(ByteArray* preimage, EncodedTransaction* out) noexcept(
        false) -> std::size_t;
    auto parse_locktime(ByteArray* preimage, EncodedTransaction* out) noexcept(
        false) -> void;
    auto parse_next_transaction(const bool isGeneration) noexcept -> bool;
    auto parse_outputs(
        bool isGeneration,
        ByteArray* preimage,
        EncodedTransaction* out) noexcept(false) -> void;
    auto parse_segwit_commitment(
        bool isGeneration,
        const ReadView script) noexcept -> bool;
    auto parse_transactions() noexcept -> bool;
    auto parse_version(ByteArray* preimage, EncodedTransaction* out) noexcept(
        false) -> void;
    auto parse_witnesses(
        bool isGeneration,
        std::size_t txin,
        EncodedTransaction* out) noexcept(false) -> bool;
};
}  // namespace opentxs::blockchain::bitcoin::block
