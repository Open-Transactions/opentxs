// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#pragma once

#include <cstddef>
#include <span>
#include <string_view>
#include <tuple>

#include "internal/blockchain/block/Parser.hpp"
#include "internal/blockchain/block/Types.hpp"
#include "internal/blockchain/protocol/bitcoin/base/Bitcoin.hpp"
#include "internal/blockchain/protocol/bitcoin/base/block/Types.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Header.hpp"
#include "opentxs/blockchain/protocol/bitcoin/base/block/Types.hpp"
#include "opentxs/util/Allocator.hpp"
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
namespace block
{
class Block;
class Transaction;
}  // namespace block
}  // namespace blockchain

namespace crypto
{
class Hasher;
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
}  // namespace opentxs
// NOLINTEND(modernize-concat-nested-namespaces)

namespace opentxs::blockchain::protocol::bitcoin::base::block
{
using network::blockchain::bitcoin::CompactSize;

class ParserBase : virtual public blockchain::block::Parser
{
public:
    [[nodiscard]] auto GetHeader() const noexcept -> ReadView final
    {
        return header_view_;
    }
    [[nodiscard]] auto operator()(
        const Hash& expected,
        ReadView bytes) && noexcept -> bool final;
    [[nodiscard]] auto operator()(ReadView bytes, Hash& out) noexcept
        -> bool final;
    [[nodiscard]] auto operator()(
        const Hash& expected,
        ReadView bytes,
        blockchain::block::Block& out) && noexcept -> bool final;
    [[nodiscard]] auto operator()(
        const std::size_t position,
        const Time& time,
        ReadView bytes,
        blockchain::block::Transaction& out) && noexcept -> bool final;

    ParserBase() = delete;
    ParserBase(
        const api::Crypto& crypto,
        blockchain::Type type,
        alloc::Strategy alloc) noexcept;
    ParserBase(const ParserBase&) = delete;
    ParserBase(ParserBase&&) = delete;
    auto operator=(const ParserBase&) -> ParserBase& = delete;
    auto operator=(ParserBase&&) -> ParserBase& = delete;

    ~ParserBase() override;

protected:
    enum class Mode : bool { checking, constructing };

    const api::Crypto& crypto_;
    const blockchain::Type chain_;
    const bool cashtoken_;
    alloc::Strategy alloc_;
    ReadView data_;
    std::size_t bytes_;
    ReadView header_view_;
    Header header_;
    Vector<TransactionHash> txids_;
    Vector<TransactionHash> wtxids_;
    Vector<EncodedTransaction> transactions_;

    auto check(std::string_view message, std::size_t required) const
        noexcept(false) -> void;
    auto constructing() const noexcept { return Mode::constructing == mode_; }
    auto make_index(std::span<TransactionHash> hashes) noexcept -> TxidIndex;

    virtual auto find_payload() noexcept -> bool;
    auto get_transactions() noexcept(false) -> TransactionMap;
    auto parse_size(std::string_view message) noexcept(false) -> std::size_t;

private:
    using Data = std::tuple<
        std::size_t,
        EncodedTransaction*,
        blockchain::block::Transaction*>;

    Mode mode_;
    bool verify_hash_;
    Hash block_hash_;
    Hash merkle_root_;
    Hash witness_reserved_value_;
    Hash segwit_commitment_;
    std::size_t transaction_count_;
    bool has_segwit_commitment_;
    bool has_segwit_transactions_;
    bool has_segwit_reserved_value_;
    bool dip_2_;
    Time timestamp_;

    auto calculate_committment() const noexcept -> Hash;
    auto calculate_merkle() const noexcept -> Hash;
    auto calculate_witness() const noexcept -> Hash;
    auto compare_header_to_hash(const Hash& expected) const noexcept -> bool;
    auto compare_merkle_to_header() const noexcept -> bool;
    auto compare_segwit_to_commitment() const noexcept -> bool;
    auto get_transaction(Data data) const noexcept -> void;
    auto get_transactions(std::span<Data> data) const noexcept -> void;
    auto is_dip_2(ReadView version) const noexcept -> bool;
    auto is_segwit_tx(EncodedTransaction* out) const noexcept -> bool;

    auto calculate_hash(const ReadView header) noexcept -> bool;
    auto calculate_txids(
        bool isSegwit,
        bool isGeneration,
        bool haveWitnesses,
        opentxs::crypto::Hasher& wtxid,
        opentxs::crypto::Hasher& txid,
        EncodedTransaction* out) noexcept(false) -> void;
    auto check_dip_2() noexcept(false) -> void;
    virtual auto construct_block(blockchain::block::Block& out) noexcept
        -> bool = 0;
    auto parse(const Hash& expected, ReadView bytes) noexcept -> bool;
    auto parse_dip_2(
        opentxs::crypto::Hasher& wtxid,
        opentxs::crypto::Hasher& txid,
        EncodedTransaction* out) noexcept(false) -> void;
    auto parse_header() noexcept -> bool;
    auto parse_inputs(
        bool isSegwit,
        opentxs::crypto::Hasher& wtxid,
        opentxs::crypto::Hasher& txid,
        EncodedTransaction* out) noexcept(false) -> std::size_t;
    auto parse_locktime(
        bool isSegwit,
        opentxs::crypto::Hasher& wtxid,
        opentxs::crypto::Hasher& txid,
        EncodedTransaction* out) noexcept(false) -> void;
    auto parse_next_transaction(const bool isGeneration) noexcept -> bool;
    auto parse_outputs(
        bool isGeneration,
        bool isSegwit,
        opentxs::crypto::Hasher& wtxid,
        opentxs::crypto::Hasher& txid,
        EncodedTransaction* out) noexcept(false) -> void;
    auto parse_segwit_commitment(
        bool isGeneration,
        const ReadView script) noexcept -> bool;
    auto parse_size(
        std::string_view message,
        bool isSegwit,
        opentxs::crypto::Hasher& wtxid,
        opentxs::crypto::Hasher& txid,
        CompactSize* out) noexcept(false) -> std::size_t;
    auto parse_size_segwit(
        std::string_view message,
        opentxs::crypto::Hasher& wtxid,
        CompactSize* out) noexcept(false) -> std::size_t;
    auto parse_transactions() noexcept -> bool;
    auto parse_version(
        bool isSegwit,
        opentxs::crypto::Hasher& wtxid,
        opentxs::crypto::Hasher& txid,
        EncodedTransaction* out) noexcept(false) -> void;
    auto parse_witnesses(
        bool isGeneration,
        std::size_t count,
        opentxs::crypto::Hasher& wtxid,
        EncodedTransaction* out) noexcept(false) -> bool;
};
}  // namespace opentxs::blockchain::protocol::bitcoin::base::block
