// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

// IWYU pragma: no_forward_declare opentxs::blockchain::Type

#include "0_stdafx.hpp"                          // IWYU pragma: associated
#include "blockchain/bitcoin/block/Checker.hpp"  // IWYU pragma: associated

#include <algorithm>
#include <cstdint>
#include <cstring>
#include <optional>
#include <stdexcept>
#include <utility>

#include "blockchain/bitcoin/block/Block.hpp"
#include "internal/blockchain/bitcoin/Bitcoin.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/block/Hash.hpp"
#include "opentxs/core/ByteArray.hpp"
#include "opentxs/core/Data.hpp"
#include "opentxs/network/blockchain/bitcoin/CompactSize.hpp"
#include "opentxs/util/Container.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/Writer.hpp"

namespace opentxs::blockchain::bitcoin::block
{
using opentxs::network::blockchain::bitcoin::DecodeCompactSize;

Checker::Checker(const api::Crypto& crypto, blockchain::Type type) noexcept
    : data_()
    , crypto_(crypto)
    , chain_(type)
    , block_hash_()
    , merkle_root_()
    , witness_reserved_value_()
    , segwit_commitment_()
    , transaction_count_()
    , txids_()
    , wtxids_()
    , has_segwit_commitment_(false)
    , has_segwit_transactions_(false)
    , has_segwit_reserved_value_(false)
{
}

auto Checker::calculate_hash(const ReadView header) noexcept -> bool
{
    return BlockHash(crypto_, chain_, header, block_hash_.WriteInto());
}

auto Checker::calculate_committment() const noexcept -> Hash
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

auto Checker::calculate_merkle() const noexcept -> Hash
{
    using Block = implementation::Block;

    return Block::calculate_merkle_value(crypto_, chain_, txids_);
}

auto Checker::calculate_witness() const noexcept -> Hash
{
    using Block = implementation::Block;

    return Block::calculate_merkle_value(crypto_, chain_, wtxids_);
}

auto Checker::compare_header_to_hash(const Hash& expected) const noexcept
    -> bool
{
    return expected == block_hash_;
}

auto Checker::compare_merkle_to_header() const noexcept -> bool
{
    return merkle_root_ == calculate_merkle();
}

auto Checker::compare_segwit_to_commitment() const noexcept -> bool
{
    return segwit_commitment_ == calculate_committment();
}

auto Checker::find_payload() noexcept -> bool
{
    if (auto size = DecodeCompactSize(data_); size.has_value()) {
        transaction_count_ = *size;
        txids_.reserve(transaction_count_);
        wtxids_.reserve(transaction_count_);

        return true;
    } else {
        LogError()(OT_PRETTY_CLASS())("failed to decode transaction count")
            .Flush();

        return false;
    }
}

auto Checker::is_segwit_tx() const noexcept -> bool
{
    auto marker = data_.substr(4_uz, 2_uz);

    return HasSegwit(marker).has_value();
}

auto Checker::operator()(const Hash& expected, const ReadView bytes) noexcept
    -> bool
{
    data_ = std::move(bytes);

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

auto Checker::parse_header() noexcept -> bool
{
    constexpr auto header = 80_uz;
    constexpr auto merkleStart = 36_uz;

    if (data_.size() < header) {
        LogError()(OT_PRETTY_CLASS())("input does not contain a valid ")(
            print(chain_))(" block header")
            .Flush();

        return false;
    }

    if (false == calculate_hash(data_.substr(0_uz, header))) {
        LogError()(OT_PRETTY_CLASS())("failed to calculate ")(print(chain_))(
            " block hash")
            .Flush();

        return false;
    }

    if (!merkle_root_.Assign(data_.substr(merkleStart, merkle_root_.size()))) {
        LogError()(OT_PRETTY_CLASS())("failed to extract merkle root").Flush();

        return false;
    }

    data_.remove_prefix(header);

    return true;
}

auto Checker::parse_legacy_transaction(const bool isGeneration) noexcept(false)
    -> bool
{
    auto tx{data_};
    const auto check = [&](auto message, auto required = 1_uz) {
        const auto target = std::max(1_uz, required);

        if (data_.empty() || (data_.size() < target)) {
            const auto error = CString{"input too short: "}.append(message);

            throw std::runtime_error(error.c_str());
        }
    };
    const auto getSize = [&](auto message) {
        if (auto out = DecodeCompactSize(data_); out.has_value()) {

            return out.value();
        } else {
            const auto error = CString{"failed to decode: "}.append(message);

            throw std::runtime_error(error.c_str());
        }
    };
    constexpr auto version = 4_uz;
    check("version field", version);
    data_.remove_prefix(version);
    const auto txin = getSize("txin count");

    for (auto j = 0_uz; j < txin; ++j) {
        constexpr auto outpoint = 36_uz;
        check("outpoint", outpoint);
        data_.remove_prefix(outpoint);
        const auto script = getSize("script size");
        data_.remove_prefix(script);
        constexpr auto sequence = 4_uz;
        check("sequence", sequence);
        data_.remove_prefix(sequence);
    }

    const auto txout = getSize("txout count");

    for (auto j = 0_uz; j < txout; ++j) {
        constexpr auto value = 8_uz;
        check("value", value);
        data_.remove_prefix(value);
        const auto script = getSize("script size");

        if (false == parse_segwit_commitment(data_.substr(0_uz, script))) {

            throw std::runtime_error("failed to parse segwit commitment");
        }

        data_.remove_prefix(script);
    }

    constexpr auto locktime = 4_uz;
    check("lock time", locktime);
    data_.remove_prefix(locktime);
    tx.remove_suffix(data_.size());
    auto& txid = txids_.emplace_back();

    if (false == TransactionHash(crypto_, chain_, tx, txid.WriteInto())) {
        txids_.pop_back();

        throw std::runtime_error("failed to calculate txid");
    }

    wtxids_.emplace_back(txid);

    return true;
}

auto Checker::parse_next_transaction(const bool isGeneration) noexcept -> bool
{
    try {
        const auto minimumSize = 10_uz;

        if (data_.size() < minimumSize) {

            throw std::runtime_error{
                "input too small to be a valid transaction"};
        }

        if (is_segwit_tx()) {
            has_segwit_transactions_ = true;

            return parse_segwit_transaction(isGeneration);
        } else {

            return parse_legacy_transaction(isGeneration);
        }
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto Checker::parse_segwit_commitment(const ReadView script) noexcept -> bool
{
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

auto Checker::parse_segwit_transaction(const bool isGeneration) noexcept(false)
    -> bool
{
    auto tx{data_};
    auto preimage = ByteArray{};
    auto cs = ReadView{};
    const auto check = [&](auto message, auto required = 1_uz) {
        const auto target = std::max(1_uz, required);

        if (data_.empty() || (data_.size() < target)) {
            const auto error = CString{"input too short: "}.append(message);

            throw std::runtime_error(error.c_str());
        }
    };
    const auto get_size = [&](auto message, auto copy) {
        if (auto out = DecodeCompactSize(data_, cs); out.has_value()) {
            if (copy) { preimage.Concatenate(cs); }

            return out.value();
        } else {
            const auto error = CString{"failed to decode: "}.append(message);

            throw std::runtime_error(error.c_str());
        }
    };
    const auto getSize = [&](auto message) { return get_size(message, true); };
    const auto getSizeSegwit = [&](auto message) {
        return get_size(message, false);
    };
    constexpr auto version = 4_uz;
    check("version field", version);
    preimage.Concatenate(data_.substr(0_uz, version));
    data_.remove_prefix(version);
    constexpr auto segwit = 2_uz;
    data_.remove_prefix(segwit);
    const auto txin = getSize("txin count");

    for (auto j = 0_uz; j < txin; ++j) {
        constexpr auto outpoint = 36_uz;
        check("outpoint", outpoint);
        preimage.Concatenate(data_.substr(0_uz, outpoint));
        data_.remove_prefix(outpoint);
        const auto script = getSize("script size");
        check("script", script);
        preimage.Concatenate(data_.substr(0_uz, script));
        data_.remove_prefix(script);
        constexpr auto sequence = 4_uz;
        check("sequence", sequence);
        preimage.Concatenate(data_.substr(0_uz, sequence));
        data_.remove_prefix(sequence);
    }

    const auto txout = getSize("txout count");

    for (auto j = 0_uz; j < txout; ++j) {
        constexpr auto value = 8_uz;
        check("value", value);
        preimage.Concatenate(data_.substr(0_uz, value));
        data_.remove_prefix(value);
        const auto script = getSize("script size");
        check("script", script);
        const auto bytes = data_.substr(0_uz, script);

        if (false == parse_segwit_commitment(data_.substr(0_uz, script))) {

            throw std::runtime_error("failed to parse segwit commitment");
        }

        preimage.Concatenate(bytes);
        data_.remove_prefix(script);
    }

    auto haveWitnesses{false};

    for (auto j = 0_uz; j < txin; ++j) {
        const auto items = getSizeSegwit("witness item count");

        if (0_uz < items) { haveWitnesses = true; }

        for (auto k = 0_uz; k < items; ++k) {
            const auto witness = getSizeSegwit("witness size");
            check("witness", witness);
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

    constexpr auto locktime = 4_uz;
    check("lock time", locktime);
    preimage.Concatenate(data_.substr(0_uz, locktime));
    data_.remove_prefix(locktime);
    tx.remove_suffix(data_.size());
    auto& txid = txids_.emplace_back();

    if (!TransactionHash(crypto_, chain_, preimage.Bytes(), txid.WriteInto())) {
        txids_.pop_back();

        throw std::runtime_error("failed to calculate txid");
    }

    auto& wtxid = wtxids_.emplace_back();

    if (false == isGeneration) {
        if (haveWitnesses) {
            if (!TransactionHash(crypto_, chain_, tx, wtxid.WriteInto())) {
                txids_.pop_back();

                throw std::runtime_error("failed to calculate wtxid");
            }
        } else {
            wtxid = txid;
        }
    }

    return true;
}

auto Checker::parse_transactions() noexcept -> bool
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
}  // namespace opentxs::blockchain::bitcoin::block
