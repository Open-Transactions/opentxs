// Copyright (c) 2010-2022 The Open-Transactions developers
// This Source Code Form is subject to the terms of the Mozilla Public
// License, v. 2.0. If a copy of the MPL was not distributed with this
// file, You can obtain one at http://mozilla.org/MPL/2.0/.

#include "internal/blockchain/bitcoin/Bitcoin.hpp"  // IWYU pragma: associated

#include <boost/endian/buffers.hpp>
#include <cstdint>
#include <iterator>
#include <numeric>
#include <stdexcept>

#include "internal/blockchain/bitcoin/block/Input.hpp"
#include "internal/blockchain/bitcoin/block/Output.hpp"
#include "internal/core/Amount.hpp"
#include "internal/util/Bytes.hpp"
#include "internal/util/LogMacros.hpp"
#include "internal/util/P0330.hpp"
#include "opentxs/blockchain/Blockchain.hpp"
#include "opentxs/blockchain/BlockchainType.hpp"
#include "opentxs/blockchain/Types.hpp"
#include "opentxs/blockchain/bitcoin/block/Input.hpp"
#include "opentxs/blockchain/bitcoin/block/Output.hpp"
#include "opentxs/blockchain/bitcoin/block/Script.hpp"
#include "opentxs/blockchain/block/Outpoint.hpp"
#include "opentxs/core/Amount.hpp"
#include "opentxs/util/Log.hpp"
#include "opentxs/util/WriteBuffer.hpp"
#include "opentxs/util/Writer.hpp"

namespace bb = opentxs::blockchain::bitcoin;

namespace opentxs::blockchain::bitcoin::sighash
{
constexpr auto All = std::byte{0x01};
constexpr auto None = std::byte{0x02};
constexpr auto Single = std::byte{0x03};
constexpr auto Fork_ID = std::byte{0x40};
constexpr auto Anyone_Can_Pay = std::byte{0x08};

constexpr auto test_anyone_can_pay(const std::byte& rhs) noexcept -> bool
{
    return (rhs & Anyone_Can_Pay) == Anyone_Can_Pay;
}
constexpr auto test_none(const std::byte& rhs) noexcept -> bool
{
    return (rhs & Single) == None;
}
constexpr auto test_single(const std::byte& rhs) noexcept -> bool
{
    return (rhs & Single) == Single;
}
constexpr auto test_all(const std::byte& rhs) noexcept -> bool
{
    return ((rhs & Single) | All) == All;
}
}  // namespace opentxs::blockchain::bitcoin::sighash

namespace opentxs::blockchain::bitcoin
{
const auto cb = [](const auto lhs, const auto& in) -> std::size_t {
    return lhs + in.size();
};

auto Bip143Hashes::blank() noexcept -> const Hash&
{
    static const auto output = Hash{};

    return output;
}

auto Bip143Hashes::get_single(
    const std::size_t index,
    const std::size_t total,
    const SigHash& sigHash) noexcept -> std::unique_ptr<Hash>
{
    if (bitcoin::SigOption::Single != sigHash.Type()) { return nullptr; }

    if (index >= total) { return nullptr; }

    // TODO not implemented yet

    return nullptr;
}

auto Bip143Hashes::Outpoints(const SigHash sig) const noexcept -> const Hash&
{
    if (sig.AnyoneCanPay()) { return blank(); }

    return outpoints_;
}

auto Bip143Hashes::Outputs(const SigHash sig, const Hash* single) const noexcept
    -> const Hash&
{
    if (SigOption::All == sig.Type()) {

        return outputs_;
    } else if ((SigOption::Single == sig.Type()) && (nullptr != single)) {

        return *single;
    } else {

        return blank();
    }
}

auto Bip143Hashes::Preimage(
    const std::size_t index,
    const std::size_t total,
    const be::little_int32_buf_t& version,
    const be::little_uint32_buf_t& locktime,
    const SigHash& sigHash,
    const blockchain::bitcoin::block::Input& input) const noexcept(false)
    -> ByteArray
{
    const auto& outpoints = Outpoints(sigHash);
    const auto& sequences = Sequences(sigHash);
    const auto& outpoint = input.PreviousOutput();
    // TODO monotonic allocator
    const auto script =
        input.Internal().Spends().Internal().SigningSubscript({});

    OT_ASSERT(script.IsValid());

    const auto scriptBytes = script.CalculateSize();
    const auto cs = blockchain::bitcoin::CompactSize{scriptBytes};
    const auto& output = input.Internal().Spends();
    const auto value =
        be::little_int64_buf_t{output.Value().Internal().ExtractInt64()};
    const auto sequence = be::little_uint32_buf_t{input.Sequence()};
    const auto single = get_single(index, total, sigHash);
    const auto& outputs = Outputs(sigHash, single.get());
    // clang-format off
    const auto bytes =
        sizeof(version) +
        sizeof(outpoints) +
        sizeof(sequences) +
        sizeof(outpoint) +
        cs.Total() +
        sizeof(value) +
        sizeof(sequence) +
        sizeof(outputs) +
        sizeof(locktime) +
        sizeof(sigHash);
    // clang-format on
    auto preimage = ByteArray{};
    auto buf = reserve(preimage.WriteInto(), bytes, "preimage");
    serialize_object(version, buf, "version");
    copy(reader(outpoints), buf, "outpoints");
    copy(reader(sequences), buf, "sequences");
    serialize_object(outpoint, buf, "outpoint");
    serialize_compact_size(cs, buf, "script bytes");

    if (false == script.Serialize(buf.Write(scriptBytes))) {

        throw std::runtime_error{"failed to serialize script"};
    }

    serialize_object(value, buf, "value");
    serialize_object(sequence, buf, "sequence");
    copy(reader(outputs), buf, "outputs");
    serialize_object(locktime, buf, "locktime");
    serialize_object(sigHash, buf, "sigHash");

    return preimage;
}

auto Bip143Hashes::Sequences(const SigHash sig) const noexcept -> const Hash&
{
    if (sig.AnyoneCanPay() || (SigOption::All != sig.Type())) {
        return blank();
    }

    return sequences_;
}

auto EncodedInput::size() const noexcept -> std::size_t
{
    return sizeof(outpoint_) + cs_.Total() + sizeof(sequence_);
}

auto EncodedInputWitness::size() const noexcept -> std::size_t
{
    return cs_.Size() +
           std::accumulate(std::begin(items_), std::end(items_), 0_uz, cb);
}

auto EncodedOutput::size() const noexcept -> std::size_t
{
    return sizeof(value_) + cs_.Total();
}

auto EncodedTransaction::CalculateIDs(
    const api::Crypto& crypto,
    const blockchain::Type chain,
    const bool isGeneration) noexcept -> bool
{
    try {
        const auto preimage = preimages();

        if (witnesses_.empty()) {
            OT_ASSERT(false == preimage.legacy_.empty());
            OT_ASSERT(preimage.segwit_.empty());
        } else {
            OT_ASSERT(false == preimage.legacy_.empty());
            OT_ASSERT(false == preimage.segwit_.empty());
        }

        const auto txid = preimage.legacy_.Bytes();

        if (false == blockchain::TransactionHash(
                         crypto, chain, txid, txid_.WriteInto())) {

            throw std::runtime_error{"failed to calculate txid"};
        }

        if (isGeneration) {
            // NOTE BIP-141: The wtxid of coinbase transaction is assumed to be
            // 0x0000....0000
            wtxid_ = {};
        } else if (witnesses_.empty()) {
            // NOTE BIP-141: If all txins are not witness program, a
            // transaction's wtxid is equal to its txid
            wtxid_ = txid_;
        } else {
            const auto wtxid = preimage.segwit_.Bytes();

            if (false == blockchain::TransactionHash(
                             crypto, chain, wtxid, wtxid_.WriteInto())) {

                throw std::runtime_error{"failed to calculate wtxid"};
            }
        }

        return true;
    } catch (const std::exception& e) {
        LogError()(OT_PRETTY_CLASS())(e.what()).Flush();

        return false;
    }
}

auto EncodedTransaction::DefaultVersion(const blockchain::Type) noexcept
    -> std::uint32_t
{
    return 1;
}

auto EncodedTransaction::legacy_size() const noexcept -> std::size_t
{
    return sizeof(version_) + input_count_.Size() +
           std::accumulate(std::begin(inputs_), std::end(inputs_), 0_uz, cb) +
           output_count_.Size() +
           std::accumulate(std::begin(outputs_), std::end(outputs_), 0_uz, cb) +
           sizeof(lock_time_);
}

auto EncodedTransaction::preimages() const noexcept(false) -> Preimages
{
    auto out = Preimages{};
    const auto isSegwit = [this] {
        if (witnesses_.empty()) {

            return false;
        } else {
            if (false == segwit_flag_.has_value()) {

                throw std::runtime_error{"missing segwit flag"};
            }

            return true;
        }
    }();
    auto serialize = [&, this](auto& target, auto size, auto segwit) {
        auto buf = reserve(target.WriteInto(), size, "preimage");
        serialize_object(version_, buf, "version");
        static constexpr auto marker = std::byte{0x0};

        if (segwit) {
            serialize_object(marker, buf, "segwit marker");
            serialize_object(*segwit_flag_, buf, "segwit flag");
        }

        serialize_compact_size(input_count_, buf, "input count");

        for (const auto& [outpoint, cs, script, sequence] : inputs_) {
            serialize_object(outpoint, buf, "outpoint");
            serialize_compact_size(cs, buf, "script bytes");
            copy(script.Bytes(), buf, "script");
            serialize_object(sequence, buf, "sequence");
        }

        serialize_compact_size(output_count_, buf, "output count");

        for (const auto& [value, cs, script] : outputs_) {
            serialize_object(value, buf, "value");
            serialize_compact_size(cs, buf, "script bytes");
            copy(script.Bytes(), buf, "script");
        }

        if (segwit) {
            for (const auto& input : witnesses_) {
                serialize_compact_size(input.cs_, buf, "witness count");

                for (const auto& item : input.items_) {
                    serialize_compact_size(item.cs_, buf, "witness bytes");
                    copy(item.item_.Bytes(), buf, "witness");
                }
            }
        }

        serialize_object(lock_time_, buf, "locktime");
    };
    serialize(out.legacy_, legacy_size(), false);

    if (isSegwit) { serialize(out.segwit_, segwit_size(), true); }

    return out;
}

auto EncodedTransaction::segwit_size() const noexcept -> std::size_t
{
    return legacy_size() +
           (segwit_flag_.has_value()
                ? std::accumulate(
                      std::begin(witnesses_), std::end(witnesses_), 2_uz, cb)
                : 0_uz);
}

auto EncodedWitnessItem::size() const noexcept -> std::size_t
{
    return cs_.Total();
}

SigHash::SigHash(
    const blockchain::Type chain,
    const SigOption flag,
    const bool anyoneCanPay) noexcept
    : flags_(opentxs::blockchain::bitcoin::sighash::All)
    , forkid_()
{
    using namespace opentxs::blockchain::bitcoin::sighash;

    static_assert(sizeof(std::uint32_t) == sizeof(SigHash));

    static_assert(false == test_anyone_can_pay(std::byte{0x00}));
    static_assert(false == test_anyone_can_pay(std::byte{0x01}));
    static_assert(false == test_anyone_can_pay(std::byte{0x02}));
    static_assert(false == test_anyone_can_pay(std::byte{0x03}));
    static_assert(test_anyone_can_pay(std::byte{0x08}));
    static_assert(test_anyone_can_pay(std::byte{0x09}));
    static_assert(test_anyone_can_pay(std::byte{0x0a}));
    static_assert(test_anyone_can_pay(std::byte{0x0b}));

    static_assert(false == test_none(std::byte{0x00}));
    static_assert(false == test_none(std::byte{0x01}));
    static_assert(test_none(std::byte{0x02}));
    static_assert(false == test_none(std::byte{0x03}));
    static_assert(false == test_none(std::byte{0x08}));
    static_assert(false == test_none(std::byte{0x09}));
    static_assert(test_none(std::byte{0x0a}));
    static_assert(false == test_none(std::byte{0x0b}));

    static_assert(false == test_single(std::byte{0x00}));
    static_assert(false == test_single(std::byte{0x01}));
    static_assert(false == test_single(std::byte{0x02}));
    static_assert(test_single(std::byte{0x03}));
    static_assert(false == test_single(std::byte{0x08}));
    static_assert(false == test_single(std::byte{0x09}));
    static_assert(false == test_single(std::byte{0x0a}));
    static_assert(test_single(std::byte{0x0b}));

    static_assert(test_all(std::byte{0x00}));
    static_assert(test_all(std::byte{0x01}));
    static_assert(false == test_all(std::byte{0x02}));
    static_assert(false == test_all(std::byte{0x03}));
    static_assert(test_all(std::byte{0x08}));
    static_assert(test_all(std::byte{0x09}));
    static_assert(false == test_all(std::byte{0x0a}));
    static_assert(false == test_all(std::byte{0x0b}));

    switch (flag) {
        case SigOption::Single: {
            flags_ = Single;
        } break;
        case SigOption::None: {
            flags_ = None;
        } break;
        case SigOption::All:
        default: {
        }
    }

    if (anyoneCanPay) { flags_ |= Anyone_Can_Pay; }

    using enum blockchain::Type;

    switch (chain) {
        case BitcoinCash:
        case BitcoinCash_testnet3:
        case BitcoinSV:
        case BitcoinSV_testnet3:
        case eCash:
        case eCash_testnet3: {
            flags_ |= Fork_ID;
            break;
        }
        case Unknown:
        case Bitcoin:
        case Bitcoin_testnet3:
        case Ethereum_frontier:
        case Ethereum_ropsten:
        case Litecoin:
        case Litecoin_testnet4:
        case PKT:
        case PKT_testnet:
        case UnitTest:
        default: {
        }
    }
}

auto SigHash::AnyoneCanPay() const noexcept -> bool
{
    using namespace opentxs::blockchain::bitcoin::sighash;

    return test_anyone_can_pay(flags_);
}

auto SigHash::begin() const noexcept -> const std::byte*
{
    return reinterpret_cast<const std::byte*>(this);
}

auto SigHash::end() const noexcept -> const std::byte*
{
    return begin() + sizeof(*this);
}

auto SigHash::ForkID() const noexcept -> ReadView
{
    return {reinterpret_cast<const char*>(forkid_.data()), forkid_.size()};
}

auto SigHash::Type() const noexcept -> SigOption
{
    using namespace opentxs::blockchain::bitcoin::sighash;

    if (test_single(flags_)) {

        return SigOption::Single;
    } else if (test_none(flags_)) {

        return SigOption::None;
    }

    return SigOption::All;
}
}  // namespace opentxs::blockchain::bitcoin
