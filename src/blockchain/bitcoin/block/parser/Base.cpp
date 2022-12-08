// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "blockchain/bitcoin/block/parser/Base.hpp"  // IWYU pragma: associated

#include <ankerl/unordered_dense.h>
#include <boost/endian/buffers.hpp>
#include <algorithm>
#include <cstdint>
#include <cstring>
#include <iterator>
#include <optional>
#include <stdexcept>
#include <utility>

#include "TBB.hpp"
#include "blockchain/bitcoin/block/transaction/TransactionPrivate.hpp"
#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/blockchain/bitcoin/block/Factory.hpp"
#include "internal/blockchain/bitcoin/block/Types.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Header.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/blockchain/block/Transaction.hpp"
#include "opentxs/blockchain/block/TransactionHash.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Bytes.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::blockchain::bitcoin::block
{
using opentxs::network::blockchain::bitcoin::DecodeCompactSize;

ParserBase::ParserBase(
    const api::Crypto& crypto,
    blockchain::Type type,
    alloc::Default alloc) noexcept
    : crypto_(crypto)
    , chain_(type)
    , alloc_(alloc)
    , data_()
    , bytes_()
    , header_view_()
    , header_(alloc_)
    , txids_(alloc_)
    , wtxids_(alloc_)
    , transactions_()
    , mode_(Mode::constructing)
    , verify_hash_(true)
    , block_hash_()
    , merkle_root_()
    , witness_reserved_value_()
    , segwit_commitment_()
    , transaction_count_()
    , has_segwit_commitment_(false)
    , has_segwit_transactions_(false)
    , has_segwit_reserved_value_(false)
    , timestamp_()
{
}

auto ParserBase::calculate_hash(const ReadView header) noexcept -> bool
{
    return BlockHash(crypto_, chain_, header, block_hash_.WriteInto());
}

auto ParserBase::calculate_committment() const noexcept -> Hash
{
    const auto data = [&] {
        auto out = ByteArray{calculate_witness()};
        out.Concatenate(witness_reserved_value_.Bytes());

        return out;
    }();

    auto out = Hash{};

    if (false == BlockHash(crypto_, chain_, data.Bytes(), out.WriteInto())) {
        LogError()(OT_PRETTY_CLASS())("failed to calculate witness committment")
            .Flush();
    }

    return out;
}

auto ParserBase::calculate_merkle() const noexcept -> Hash
{
    return CalculateMerkleValue(crypto_, chain_, txids_);
}

auto ParserBase::calculate_txids(
    const ReadView tx,
    bool isGeneration,
    ByteArray* preimage,
    bool haveWitnesses,
    EncodedTransaction* out) noexcept(false) -> void
{
    const auto segwit = nullptr != preimage;
    const auto construct = nullptr != out;
    const auto& txid = [&]() -> auto&
    {
        auto& id = txids_.emplace_back();
        const auto p = segwit ? preimage->Bytes() : tx;

        if (false ==
            blockchain::TransactionHash(crypto_, chain_, p, id.WriteInto())) {
            txids_.pop_back();

            throw std::runtime_error("failed to calculate txid");
        }

        return id;
    }
    ();
    const auto& wtxid = [&]() -> auto&
    {
        if (isGeneration) {
            // NOTE BIP-141: The wtxid of coinbase transaction is assumed to be
            // 0x0000....0000

            return wtxids_.emplace_back();
        } else if (false == haveWitnesses) {
            // NOTE BIP-141: If all txins are not witness program, a
            // transaction's wtxid is equal to its txid

            return wtxids_.emplace_back(txid);
        } else {
            auto& wid = wtxids_.emplace_back();

            if (false == blockchain::TransactionHash(
                             crypto_, chain_, tx, wid.WriteInto())) {

                throw std::runtime_error("failed to calculate wtxid");
            }

            return wid;
        }
    }
    ();

    if (construct) {
        if (false == copy(txid.Bytes(), out->txid_.WriteInto())) {
            throw std::runtime_error("failed to copy txid");
        }

        if (false == copy(wtxid.Bytes(), out->wtxid_.WriteInto())) {
            throw std::runtime_error("failed to copy wtxid");
        }
    }
}

auto ParserBase::calculate_witness() const noexcept -> Hash
{
    return CalculateMerkleValue(crypto_, chain_, wtxids_);
}

auto ParserBase::check(std::string_view message, std::size_t required) const
    noexcept(false) -> void
{
    const auto target = std::max(1_uz, required);

    if (data_.empty() || (data_.size() < target)) {
        const auto error = CString{"input too short: "}.append(message);

        throw std::runtime_error(error.c_str());
    }
}

auto ParserBase::compare_header_to_hash(const Hash& expected) const noexcept
    -> bool
{
    if (verify_hash_) {

        return expected == block_hash_;
    } else {

        return true;
    }
}

auto ParserBase::compare_merkle_to_header() const noexcept -> bool
{
    return merkle_root_ == calculate_merkle();
}

auto ParserBase::compare_segwit_to_commitment() const noexcept -> bool
{
    return segwit_commitment_ == calculate_committment();
}

auto ParserBase::find_payload() noexcept -> bool
{
    if (auto size = DecodeCompactSize(data_); size.has_value()) {
        transaction_count_ = *size;
        txids_.reserve(transaction_count_);
        wtxids_.reserve(transaction_count_);

        if (constructing()) { transactions_.reserve(transaction_count_); }

        return true;
    } else {
        LogError()(OT_PRETTY_CLASS())("failed to decode transaction count")
            .Flush();

        return false;
    }
}

auto ParserBase::get_transaction(Data data) const noexcept -> void
{
    auto& [position, encoded, out] = data;
    *out = factory::BitcoinTransaction(
        chain_, position, timestamp_, std::move(*encoded), alloc_);
}

auto ParserBase::get_transactions() noexcept(false) -> TransactionMap
{
    const auto count = transactions_.size();
    auto transactions =
        TransactionMap{count, blockchain::block::Transaction{alloc_}, alloc_};
    auto index = [&] {
        auto data = Vector<Data>{alloc_};
        data.reserve(count);
        data.clear();

        for (auto n = 0_uz; n < count; ++n) {
            auto& in = transactions_[n];
            auto& out = transactions[n];
            data.emplace_back(n, std::addressof(in), std::addressof(out));
        }

        return data;
    }();
    get_transactions(index);

    return transactions;
}

auto ParserBase::get_transactions(std::span<Data> view) const noexcept -> void
{
    tbb::parallel_for(
        tbb::blocked_range<std::size_t>{0_uz, view.size()},
        [&, this](const auto& r) {
            for (auto i = r.begin(); i != r.end(); ++i) {
                get_transaction(view[i]);
            }
        });
}

auto ParserBase::is_segwit_tx(EncodedTransaction* out) const noexcept -> bool
{
    const auto construct = (nullptr != out);
    const auto view = data_.substr(4_uz, 2_uz);
    const auto* marker = reinterpret_cast<const std::byte*>(view.data());
    const auto* flag = std::next(marker);
    static constexpr auto segwit = std::byte{0x0};
    const auto output = *marker == segwit;

    if (construct) {
        auto& dest = out->segwit_flag_;

        if (output) {
            dest.emplace(*flag);
        } else {
            dest.reset();
        }
    }

    return output;
}

auto ParserBase::make_index(std::span<TransactionHash> hashes) noexcept
    -> TxidIndex
{
    const auto count = hashes.size();
    auto out = TxidIndex{alloc_};
    out.reserve(count);
    out.clear();

    for (auto n = 0_uz; n < count; ++n) {
        auto& hash = hashes[n];
        out.try_emplace(std::move(hash), n);
    }

    return out;
}

auto ParserBase::operator()(
    const Hash& expected,
    const ReadView bytes) && noexcept -> bool
{
    mode_ = Mode::checking;
    verify_hash_ = true;

    return parse(expected, bytes);
}

auto ParserBase::operator()(ReadView bytes, Hash& out) noexcept -> bool
{
    mode_ = Mode::checking;
    verify_hash_ = false;
    static const auto id = Hash{};
    auto val = parse(id, bytes);
    out = block_hash_;

    return val;
}

auto ParserBase::operator()(
    const Hash& expected,
    ReadView bytes,
    blockchain::block::Block& out) && noexcept -> bool
{
    mode_ = Mode::constructing;
    verify_hash_ = false;

    if (parse(expected, bytes)) {
        const auto count = transactions_.size();

        OT_ASSERT(header_.IsValid());
        OT_ASSERT(count == txids_.size());
        OT_ASSERT(count == wtxids_.size());
    } else {
        LogError()(OT_PRETTY_CLASS())("invalid block").Flush();

        return false;
    }

    return construct_block(out);
}

auto ParserBase::operator()(
    const std::size_t position,
    const Time& time,
    ReadView bytes,
    blockchain::block::Transaction& out) && noexcept -> bool
{
    mode_ = Mode::constructing;
    verify_hash_ = false;
    data_ = std::move(bytes);

    try {
        const auto isGeneration = (0_uz == position);

        if (false == parse_next_transaction(isGeneration)) {
            throw std::runtime_error{"failed to parse transaction"};
        }

        OT_ASSERT(false == transactions_.empty());

        auto& encoded = transactions_.back();
        out = factory::BitcoinTransaction(
            chain_, position, time, std::move(encoded), alloc_);

        return out.IsValid();
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();
        out = {alloc_};

        return false;
    }
}

auto ParserBase::parse(const Hash& expected, ReadView bytes) noexcept -> bool
{
    data_ = std::move(bytes);
    bytes_ = data_.size();

    if (data_.empty()) {
        LogError()(OT_PRETTY_CLASS())("empty input").Flush();

        return false;
    }

    if (false == parse_header()) {
        LogError()(OT_PRETTY_CLASS())("failed to parse block header").Flush();

        return false;
    }

    if (false == compare_header_to_hash(expected)) {
        LogError()(OT_PRETTY_CLASS())(print(chain_))(
            " block header hash does not match expected value")
            .Flush();

        return false;
    }

    if (false == find_payload()) {
        LogError()(OT_PRETTY_CLASS())(print(chain_))(
            " failed to locate transactions")
            .Flush();

        return false;
    }

    if (false == parse_transactions()) {
        LogError()(OT_PRETTY_CLASS())(print(chain_))(
            " failed to parse transactions")
            .Flush();

        return false;
    }

    if (false == data_.empty()) {
        LogError()(OT_PRETTY_CLASS())(data_.size())(
            " excess bytes remain after parsing")
            .Flush();
    }

    if (false == compare_merkle_to_header()) {
        LogError()(OT_PRETTY_CLASS())(print(chain_))(
            " merkle root does not match expected value")
            .Flush();

        return false;
    }

    if (has_segwit_transactions_) {
        if (false == has_segwit_commitment_) {
            LogError()(OT_PRETTY_CLASS())(print(chain_))(
                " generation transaction does not contain segwit commitment")
                .Flush();

            return false;
        }

        if (false == has_segwit_reserved_value_) {
            LogError()(OT_PRETTY_CLASS())(print(chain_))(
                " generation transaction does not contain segwit reserved "
                "value")
                .Flush();

            return false;
        }

        if (false == compare_segwit_to_commitment()) {
            LogError()(OT_PRETTY_CLASS())(print(chain_))(
                " witness root hash does not match expected value")
                .Flush();

            return false;
        }
    }

    return true;
}

auto ParserBase::parse_header() noexcept -> bool
{
    constexpr auto header = 80_uz;
    constexpr auto merkleStart = 36_uz;

    if (data_.size() < header) {
        LogError()(OT_PRETTY_CLASS())("input does not contain a valid ")(
            print(chain_))(" block header")
            .Flush();

        return false;
    }

    header_view_ = data_.substr(0_uz, header);

    if (false == calculate_hash(header_view_)) {
        LogError()(OT_PRETTY_CLASS())("failed to calculate ")(print(chain_))(
            " block hash")
            .Flush();

        return false;
    }

    if (!merkle_root_.Assign(data_.substr(merkleStart, merkle_root_.size()))) {
        LogError()(OT_PRETTY_CLASS())("failed to extract merkle root").Flush();

        return false;
    }

    if (constructing()) {
        header_ =
            factory::BitcoinBlockHeader(crypto_, chain_, header_view_, alloc_);

        if (header_.IsValid()) {
            timestamp_ = header_.Timestamp();
        } else {
            LogError()(OT_PRETTY_CLASS())("failed to instantiate header")
                .Flush();

            return false;
        }
    }

    data_.remove_prefix(header);

    return true;
}

auto ParserBase::parse_inputs(
    ByteArray* preimage,
    EncodedTransaction* out) noexcept(false) -> std::size_t
{
    const auto hash = nullptr != preimage;
    const auto construct = nullptr != out;
    const auto txin = parse_size(
        "txin count",
        preimage,
        construct ? std::addressof(out->input_count_) : nullptr);

    for (auto j = 0_uz; j < txin; ++j) {
        auto* next = [&]() -> EncodedInput* {
            if (construct) {
                auto& dest = out->inputs_;
                dest.reserve(txin);

                return std::addressof(dest.emplace_back());
            } else {

                return nullptr;
            }
        }();
        constexpr auto outpoint = 36_uz;
        check("outpoint", outpoint);

        if (hash) { preimage->Concatenate(data_.substr(0_uz, outpoint)); }

        if (construct) {
            auto& dest = next->outpoint_;
            static_assert(sizeof(dest) == outpoint);
            std::memcpy(
                static_cast<void*>(std::addressof(dest)),
                data_.data(),
                outpoint);
        }

        data_.remove_prefix(outpoint);
        const auto script = parse_size(
            "script size",
            preimage,
            construct ? std::addressof(next->cs_) : nullptr);
        check("script", script);
        auto val = data_.substr(0_uz, script);

        if (hash) { preimage->Concatenate(val); }

        if (construct && (false == copy(val, next->script_.WriteInto()))) {
            throw std::runtime_error{"failed to copy script opcodes"};
        }

        data_.remove_prefix(script);
        constexpr auto sequence = 4_uz;
        check("sequence", sequence);
        val = data_.substr(0_uz, sequence);

        if (hash) { preimage->Concatenate(val); }

        if (construct) {
            auto& dest = next->sequence_;
            static_assert(sizeof(dest) == sequence);
            std::memcpy(
                static_cast<void*>(std::addressof(dest)),
                data_.data(),
                sequence);
        }

        data_.remove_prefix(sequence);
    }

    return txin;
}

auto ParserBase::parse_locktime(
    ByteArray* preimage,
    EncodedTransaction* out) noexcept(false) -> void
{
    const auto hash = nullptr != preimage;
    const auto construct = nullptr != out;
    constexpr auto locktime = 4_uz;
    check("lock time", locktime);

    if (hash) { preimage->Concatenate(data_.substr(0_uz, locktime)); }

    if (construct) {
        auto& dest = out->lock_time_;
        static_assert(sizeof(dest) == locktime);
        std::memcpy(
            static_cast<void*>(std::addressof(dest)), data_.data(), locktime);
    }

    data_.remove_prefix(locktime);
}

auto ParserBase::parse_next_transaction(const bool isGeneration) noexcept
    -> bool
{
    try {
        const auto minimumSize = 10_uz;

        if (data_.size() < minimumSize) {

            throw std::runtime_error{
                "input too small to be a valid transaction"};
        }

        auto* encoded = [this]() -> EncodedTransaction* {
            if (constructing()) {
                auto& next = transactions_.emplace_back();

                return std::addressof(next);
            } else {

                return nullptr;
            }
        }();
        auto segwit = [&]() -> std::optional<ByteArray> {
            if (is_segwit_tx(encoded)) {
                has_segwit_transactions_ = true;

                return ByteArray{};
            } else {

                return std::nullopt;
            }
        }();
        auto* preimage = [&]() -> ByteArray* {
            if (segwit) {

                return std::addressof(*segwit);
            } else {

                return nullptr;
            }
        }();
        auto tx{data_};
        parse_version(preimage, encoded);

        if (segwit) { data_.remove_prefix(2_uz); }

        const auto txin = parse_inputs(preimage, encoded);
        parse_outputs(isGeneration, preimage, encoded);
        const auto haveWitnesses = [&] {
            if (segwit) {

                return parse_witnesses(isGeneration, txin, encoded);
            } else {

                return false;
            }
        }();
        parse_locktime(preimage, encoded);
        tx.remove_suffix(data_.size());
        calculate_txids(tx, isGeneration, preimage, haveWitnesses, encoded);

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto ParserBase::parse_outputs(
    bool isGen,
    ByteArray* preimage,
    EncodedTransaction* out) noexcept(false) -> void
{
    const auto hash = nullptr != preimage;
    const auto construct = nullptr != out;
    const auto txout = parse_size(
        "txout count",
        preimage,
        construct ? std::addressof(out->output_count_) : nullptr);

    for (auto j = 0_uz; j < txout; ++j) {
        auto* next = [&]() -> EncodedOutput* {
            if (construct) {
                auto& dest = out->outputs_;
                dest.reserve(txout);

                return std::addressof(dest.emplace_back());
            } else {

                return nullptr;
            }
        }();
        constexpr auto value = 8_uz;
        check("value", value);
        auto val = data_.substr(0_uz, value);

        if (hash) { preimage->Concatenate(val); }

        if (construct) {
            auto& dest = next->value_;
            static_assert(sizeof(dest) == value);
            std::memcpy(
                static_cast<void*>(std::addressof(dest)), data_.data(), value);
        }

        data_.remove_prefix(value);
        const auto script = parse_size(
            "script size",
            preimage,
            construct ? std::addressof(next->cs_) : nullptr);
        check("script", script);
        val = data_.substr(0_uz, script);

        if (!parse_segwit_commitment(isGen, val)) {

            throw std::runtime_error("failed to parse segwit commitment");
        }

        if (hash) { preimage->Concatenate(val); }

        if (construct && (false == copy(val, next->script_.WriteInto()))) {
            throw std::runtime_error{"failed to copy script opcodes"};
        }

        data_.remove_prefix(script);
    }
}

auto ParserBase::parse_segwit_commitment(
    bool isGeneration,
    const ReadView script) noexcept -> bool
{
    if (false == isGeneration) { return true; }

    constexpr auto minimum = 38_uz;

    if (script.size() < minimum) { return true; }

    // NOLINTNEXTLINE(modernize-avoid-c-arrays)
    static constexpr std::uint8_t prefix[] = {
        0x6a, 0x24, 0xaa, 0x21, 0xa9, 0xed};

    if (0 == std::memcmp(prefix, script.data(), sizeof(prefix))) {
        const auto rc = segwit_commitment_.Assign(
            script.substr(sizeof(prefix), segwit_commitment_.size()));

        if (false == rc) { return false; }

        has_segwit_commitment_ = true;
    }

    return true;
}

auto ParserBase::parse_size(
    std::string_view message,
    ByteArray* preimage,
    CompactSize* val) noexcept(false) -> std::size_t
{
    const auto hash = nullptr != preimage;
    auto cs = ReadView{};

    if (auto out = DecodeCompactSize(data_, cs, val); out.has_value()) {
        if (hash) { preimage->Concatenate(cs); }

        return out.value();
    } else {
        const auto error = CString{"failed to decode: "}.append(message);

        throw std::runtime_error(error.c_str());
    }
}

auto ParserBase::parse_transactions() noexcept -> bool
{
    for (auto i = 0_uz; i < transaction_count_; ++i) {
        if (false == parse_next_transaction(0_uz == i)) {
            LogError()(OT_PRETTY_CLASS())("failed to parse transaction")
                .Flush();

            return false;
        }
    }

    return true;
}

auto ParserBase::parse_version(
    ByteArray* preimage,
    EncodedTransaction* out) noexcept(false) -> void
{
    const auto hash = nullptr != preimage;
    const auto construct = nullptr != out;
    constexpr auto version = 4_uz;
    check("version field", version);

    if (hash) { preimage->Concatenate(data_.substr(0_uz, version)); }

    if (construct) {
        auto& dest = out->version_;
        static_assert(sizeof(dest) == version);
        std::memcpy(std::addressof(dest), data_.data(), version);
    }

    data_.remove_prefix(version);
}

auto ParserBase::parse_witnesses(
    bool isGeneration,
    std::size_t txin,
    EncodedTransaction* out) noexcept(false) -> bool
{
    const auto construct = nullptr != out;
    auto haveWitnesses{false};

    if (construct) { out->witnesses_.reserve(txin); }

    for (auto j = 0_uz; j < txin; ++j) {
        auto* input = [&]() -> EncodedInputWitness* {
            if (construct) {
                auto& dest = out->witnesses_;

                return std::addressof(dest.emplace_back());
            } else {

                return nullptr;
            }
        }();
        const auto items = parse_size(
            "witness item count",
            nullptr,
            construct ? std::addressof(input->cs_) : nullptr);

        if (0_uz < items) { haveWitnesses = true; }

        for (auto k = 0_uz; k < items; ++k) {
            auto* next = [&]() -> EncodedWitnessItem* {
                if (construct) {
                    auto& dest = input->items_;
                    dest.reserve(items);

                    return std::addressof(dest.emplace_back());
                } else {

                    return nullptr;
                }
            }();
            const auto witness = parse_size(
                "witness size",
                nullptr,
                construct ? std::addressof(next->cs_) : nullptr);
            check("witness", witness);
            auto val = data_.substr(0_uz, witness);

            if (construct && (false == copy(val, next->item_.WriteInto()))) {
                throw std::runtime_error{"failed to copy witness item"};
            }

            const auto size = witness_reserved_value_.size();
            const auto witnessReservedValue =
                isGeneration && (0_uz == j) && (0_uz == k) && (size == witness);

            if (witnessReservedValue) {
                if (witness_reserved_value_.Assign(data_.substr(0_uz, size))) {
                    has_segwit_reserved_value_ = true;
                } else {

                    throw std::runtime_error(
                        "failed to assign witness reserved value");
                }
            }

            data_.remove_prefix(witness);
        }
    }

    return haveWitnesses;
}

ParserBase::~ParserBase() = default;
}  // namespace opentxs::blockchain::bitcoin::block
